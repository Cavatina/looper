#include "audio_engine.h"
#include "jack_engine.h"
#include "disk_engine.h"
#include "util/slist.h"
#include "util/debug.h"

#include <iostream>

#include <jack/transport.h>

jack_client_t *client;
jack_port_t *output_port[2];
jack_nframes_t sample_rate;

std::list<jack_dport *> inputs;
std::list<jack_dport *> outputs;

cav::slist_mrsw<jack_dport> *g_inputs = new cav::slist_mrsw<jack_dport>();
cav::slist_mrsw<jack_dport> *g_outputs = new cav::slist_mrsw<jack_dport>();

disk_thread *diskthread;

audio_engine::dport::~dport()
{
}

void audio_engine::purge_input(dport *p)
{
	std::list<jack_dport *>::iterator i = inputs.begin();
	for(;i != inputs.end(); ++i) if(p == *i) break;
	if(i != inputs.end()){
		disk_thread::lock();
		inputs.erase(i);
		delete *i;
		disk_thread::unlock();
	}
}

void audio_engine::purge_output(dport *p)
{
	std::list<jack_dport *>::iterator i = outputs.begin();
	for(;i != outputs.end(); ++i) if(p == *i) break;
	if(i != outputs.end()){
		disk_thread::lock();
		outputs.erase(i);
		delete *i;
		disk_thread::unlock();
	}
}

jack_dport::jack_dport(int id_, int channels_)
	: dport(id_, channels_), ports(channels_, (jack_port_t*)0),
	  rb(jack_ringbuffer_create(DEFAULT_BUFFER_SIZE * channels_))
{
}

jack_dport::~jack_dport()
{
	if(rb) jack_ringbuffer_free(rb);
}

bool jack_dport::is_playing() const
{
	return false;
}

void jack_dport::start(const bbt &when, sample *sample)
{
	disk_thread::lock();
	actions.push_back(new play_action(when, sample));
	disk_thread::unlock();
	disk_thread::wake();
}

void jack_dport::stop(const bbt &when)
{
	disk_thread::lock();
	actions.push_back(new stop_action(when));
	disk_thread::unlock();
}

void jack_dport::record(const bbt &when, sample *sample)
{
	disk_thread::lock();
	actions.push_back(new record_action(when, sample));
	disk_thread::unlock();
}

void jack_dport::input(int channel, const std::string &name)
{
	if(channel < 1 || channel > get_channels())
		throw std::runtime_error("jack_dport::input: out of bounds");
	disk_thread::lock();
	if(ports[channel-1]){
		jack_port_unregister(client, ports[channel-1]);
	}
	ports[channel-1] = jack_port_register(client, name.c_str(),
					      JACK_DEFAULT_AUDIO_TYPE,
					      JackPortIsInput, 0);
	DBG2(if(ports[channel-1])
	     std::cerr<<"jack_dport.input ["<<name<<"]: Registered."
	     <<std::endl;
	     else
	     std::cerr<<"jack_dport.input ["<<name<<"]: Failed to register!"
	     <<std::endl);

	disk_thread::unlock();
}

audio_engine::dport *audio_engine::add_input(int index, unsigned int channels)
{
	// TODO: Check if already existing!!

	jack_dport *r = new jack_dport(index, channels);
	g_inputs = g_inputs->push(r);
	return r;
}

audio_engine::dport *audio_engine::add_output(int index, unsigned int channels)
{
	// TODO: Check if already existing!!

	jack_dport *r = new jack_dport(index, channels);
	g_outputs = g_outputs->push(r);
	return r;
}

void audio_engine::connect(const std::string &p1, const std::string &p2)
{
	std::string conn[2];
	conn[0] = p1;
	conn[1] = p2;
	for(unsigned int i = 0; i<2; ++i){
		if(conn[i].find(':') == std::string::npos){
			conn[i] = name + ":" + conn[i];
		}
	}
	if(jack_connect(client, conn[0].c_str(), conn[1].c_str()) != 0){
		std::cerr<<"audio_engine.connect ["<<conn[0]<<" -> "<<conn[1]
			 <<"]: Failed!"<<std::endl;
	}
	else {
		DBG2(std::cerr<<"audio_engine.connect ["<<conn[0]<<" -> "
		     <<conn[1]<<"]: Ok."<<std::endl);
	}
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


int audio_engine::process(jack_nframes_t nframes, void *arg)
{
	audio_engine *a = (audio_engine *)arg;
	if(a && a->get_metronome()) a->get_metronome()->add_frames(nframes);
	return 0;
}

void jack_shutdown(void *)
{
	std::cerr<<"jack_shutdown: No handler registered..."<<std::endl;
	exit(1);
}

audio_engine::audio_engine()
	: sample_rate(0), sfn(jack_shutdown), m(0)
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

	if(!diskthread) diskthread = new disk_thread();

	sample_rate = jack_get_sample_rate(client);
	if(jack_activate(client)){
		shutdown();
		throw init_failure("jack_activate");
	}

	diskthread->run();
}


void audio_engine::shutdown()
{
	if(client){
		jack_client_close(client);
		client = 0;
	}

	if(diskthread){
		diskthread->halt();
		delete diskthread;
		diskthread = 0;
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
	return sample_rate;
}

