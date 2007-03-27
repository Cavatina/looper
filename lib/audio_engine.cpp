#include "audio_engine.h"
#include "bank.h"

#include <jack/jack.h>
#include <jack/transport.h>
#include <jack/ringbuffer.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <sndfile.h>

#include <iostream>

jack_client_t *client;
jack_port_t *output_port[2];

typedef struct _thread_info {
	pthread_t thread_id;
	metronome *metro;
	jack_nframes_t duration;
	jack_nframes_t rb_size;
	jack_client_t *client;
	volatile bool can_capture;
	volatile bool can_process;
} thread_info_t;

pthread_mutex_t disk_thread_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t disk_thread_lock_data_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t data_ready = PTHREAD_COND_INITIALIZER;

struct action
{
	enum ACTION {
		Play,
		Record,
		Stop
	} atype;
	bbt when;

	action() : atype(action::Stop) {}
	action(action::ACTION type_, const bbt &when_)
		: atype(type_), when(when_) {}
};

struct stop_action : public action
{
	stop_action(const bbt &when) : action(Stop, when) {}
};

struct play_action : public action
{
	play_action(const bbt &when, sample *s_)
		: action(Play, when), s(s_) {}
	sample *s;
	SNDFILE *sf;
};

struct record_action : public action
{
	record_action(const bbt &when, sample *s_)
		: action(Record, when), s(s_) {}
	sample *s;
	SNDFILE *sf;
};

audio_engine::dport::~dport()
{
}

class jack_dport : public audio_engine::dport
{
public:
	jack_dport(int id_, int channels_ = 0)
		: dport(id_, channels_), ports(channels_, (jack_port_t*)0) {}
	virtual ~jack_dport() {}
	bool is_playing() const;
	bool is_recording() const { return false; }
	bool is_scheduled() const { return false; }
	bool cancel_scheduled() { return false; }

	void start(const bbt &when, sample *);
	void record(const bbt &when, sample *);
	void stop(const bbt &when);

	void input(int, const std::string &);

private:
	std::list<action *> actions;
	std::vector<jack_port_t *> ports;
	jack_ringbuffer_t *rb;
};

bool jack_dport::is_playing() const
{
	return false;
}

void jack_dport::start(const bbt &when, sample *sample)
{
	pthread_mutex_lock(&disk_thread_lock);
	actions.push_back(new play_action(when, sample));
	pthread_mutex_unlock(&disk_thread_lock);
}

void jack_dport::stop(const bbt &when)
{
	pthread_mutex_lock(&disk_thread_lock);
	actions.push_back(new stop_action(when));
	pthread_mutex_unlock(&disk_thread_lock);
}

void jack_dport::record(const bbt &when, sample *sample)
{
	pthread_mutex_lock(&disk_thread_lock);
	actions.push_back(new record_action(when, sample));
	pthread_mutex_unlock(&disk_thread_lock);
}

void jack_dport::input(int channel, const std::string &name)
{
	if(channel < 1 || channel > get_channels())
		throw std::runtime_error("jack_dport::input: channel out of bounds");
	pthread_mutex_lock(&disk_thread_lock);
	if(ports[channel-1]){
		jack_port_unregister(client, ports[channel-1]);
	}
	ports[channel-1] = jack_port_register(client, name.c_str(),
					      JACK_DEFAULT_AUDIO_TYPE,
					      JackPortIsInput, 0);
	pthread_mutex_unlock(&disk_thread_lock);
}

std::list<jack_dport *> inputs;
std::list<jack_dport *> outputs;

audio_engine::dport *audio_engine::add_input(int index, unsigned int channels)
{
	// TODO: Check if already existing!!

	pthread_mutex_lock(&disk_thread_lock);
	jack_dport *r = new jack_dport(index, channels);
	inputs.push_back(r);
	pthread_mutex_unlock(&disk_thread_lock);
	return r;
}

audio_engine::dport *audio_engine::add_output(int index, unsigned int channels)
{
	// TODO: Check if already existing!!

	pthread_mutex_lock(&disk_thread_lock);
	jack_dport *r = new jack_dport(index, channels);
	outputs.push_back(r);
	pthread_mutex_unlock(&disk_thread_lock);
	return r;
}

void audio_engine::connect(const std::string &p1, const std::string &p2)
{
	jack_connect(client, p1.c_str(), p2.c_str());
}



/*
  reading_channel:
    actions: Play, Play|Play, Play|Stop, Play|Stop|Play

  writing_channel:
    actions: Record, Record|Stop


  Start playing:
  * Schedule channel_action<Play>
  * A little time before <when>, fill up ring buffer
    => How to do xfade if already running

  * process() thread decides when to start emptying [play] ring buffer
  * process() thread decides when to start placing data into [rec] ring buffer

    channel_action: if size()>1, do crossfade

*/


struct reading_channel
{
	bank *b;
	unsigned int channel_id; // unique
	volatile bool is_reading;
	SNDFILE *sf;
	unsigned int channels;
	std::vector<jack_port_t *> ports;
	jack_ringbuffer_t *rb;
};

struct writing_channel
{
	bank *b;
	unsigned int channel_id; // unique
	volatile bool is_writing;
	SNDFILE *sf;
	unsigned int channels;
	int bitdepth;
	std::vector<jack_port_t *> ports;
	jack_ringbuffer_t *rb;
};


int audio_engine::process(jack_nframes_t nframes, void *arg)
{
	audio_engine *a = (audio_engine *)arg;
	if(a && a->get_metronome()) a->get_metronome()->add_frames(nframes);
	return 0;
}

void jack_shutdown(void *)
{
	std::cerr<<"jack_shutdown: No shutdown handler registered..."<<std::endl;
	exit(1);
}

audio_engine::audio_engine()
	: sfn(jack_shutdown)
{}

void audio_engine::initialize()
{
	if((client = jack_client_new(name.c_str())) == 0){
		throw init_failure("jack_client_new");
	}

	jack_set_process_callback(client, audio_engine::process, this);
	jack_on_shutdown(client, sfn, this);
	output_port[0] = jack_port_register(client,
					    "master/out_1",
					    JACK_DEFAULT_AUDIO_TYPE,
					    JackPortIsOutput, 0);
	output_port[1] = jack_port_register(client,
					    "master/out_2",
					    JACK_DEFAULT_AUDIO_TYPE,
					    JackPortIsOutput, 0);

	sample_rate = jack_get_sample_rate(client);
	if(jack_activate(client)){
		shutdown();
		throw init_failure("jack_activate");
	}
}


void audio_engine::shutdown()
{
	if(client){
		jack_client_close(client);
		client=0;
	}
}


// function to call on audioengine shutdown
void audio_engine::set_shutdown(shutdown_fn *f)
{
	sfn = f;
}

// used by looper to determine required buffering length etc.
uint32_t audio_engine::get_sample_rate() const
{
	return 0;
}

