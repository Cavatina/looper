#include "audio_engine.h"
#include "jack_engine.h"
#include "disk_engine.h"
#include "util/slist.h"
#include "util/debug.h"
#include "util/to_string.h"

#include <iostream>
#include <iomanip>

#include <jack/jack.h>
#include <jack/transport.h>

jack_client_t *client;
jack_port_t *output_port[2];
jack_nframes_t sample_rate;

std::list<jack_dport *> inputs;
std::list<jack_dport *> outputs;

cav::slist_mrsw<jack_dport> *g_inputs = new cav::slist_mrsw<jack_dport>();
cav::slist_mrsw<jack_dport> *g_outputs = new cav::slist_mrsw<jack_dport>();

disk_thread *diskthread;

#define MAX_CHANNELS 8
jack_default_audio_sample_t *in_buf[MAX_CHANNELS];
const size_t sample_size = sizeof(jack_default_audio_sample_t);

size_t overruns = 0;
size_t underruns = 0;

audio_engine::dport::~dport()
{
}
unsigned int audio_engine::dport::get_channels() const
{
	return channels ? channels : 1;
}

// void audio_engine::purge_input(dport *p)
// {
// 	remove_from(g_inputs, p);
// }

// void audio_engine::purge_output(dport *p)
// {
// 	remove_from(g_outputs, p);
// }

jack_dport::jack_dport(int id_, int channels_)
	: dport(id_, channels_), ports(channels_, (jack_port_t*)0),
	  rb(jack_ringbuffer_create(DEFAULT_BUFFER_SIZE * channels_)),
	  halted(false)
{
}

jack_dport::~jack_dport()
{
	std::list<playback_t *>::iterator i = play_queue.begin();
	for(; i != play_queue.end(); ++i) delete *i;
	play_queue.clear();
	std::list<recording_t *>::iterator j = rec_queue.begin();
	for(; j != rec_queue.end(); ++j) delete *j;
	rec_queue.clear();
	if(rb) jack_ringbuffer_free(rb);
}

bool jack_dport::is_playing() const
{
	return !play_queue.empty() && !halted;
}

bool jack_dport::is_playing_or_flushing(metronome *m) const
{
	return (is_playing()
		&& m->get_current_time() >= play_queue.front()->real_start)
		|| jack_ringbuffer_read_space(rb);
}

bool jack_dport::is_recording() const
{
	return !rec_queue.empty() && !halted;
}

bool jack_dport::is_recording(metronome *m) const
{
	return is_recording() && rec_queue.size()
		&& m->get_current_time() >= rec_queue.front()->start;
}

void jack_dport::start(const bbt &when, sample *sample)
{
	disk_thread::lock();
	DBG2(std::cerr<<"jack_dport::start <"
	     <<sample->get_source()<<"> @ "<<when<<std::endl);
	play_queue.push_back(new playback_t(when, sample));
	disk_thread::unlock();
	disk_thread::wake();
}

void jack_dport::stop(const bbt &when)
{
	disk_thread::lock();
	if(play_queue.size()){
		play_queue.front()->end = when;
	}
	if(rec_queue.size()){
		rec_queue.front()->end = when;
	}
	disk_thread::unlock();
}

void jack_dport::stop_now()
{
	disk_thread::lock();
	cancel_record();
	if(!play_queue.empty()){
		play_queue.front()->end = bbt();
	}
	disk_thread::unlock();
}

void jack_dport::record(const bbt &when, sample *sample)
{
	disk_thread::lock();
	DBG2(std::cerr<<"jack_dport[0x"
	     <<std::hex<<std::setw(4)<<(void*)this<<std::dec<<"]::record <"
	     <<sample->get_source()<<">"<<std::endl);
	rec_queue.push_back(new recording_t(when, sample, get_channels()));
	rec_queue.back()->open();
	disk_thread::unlock();
}

void jack_dport::input(unsigned int channel, const std::string &name)
{
	if(channel < 1 || channel > get_channels())
		throw std::runtime_error("jack_dport.input: "
					 + to_string(channel) +
					 " out of bounds.");
	disk_thread::lock();
	if(ports[channel-1]){
		jack_port_unregister(client, ports[channel-1]);
	}
	ports[channel-1] = jack_port_register(client, name.c_str(),
					      JACK_DEFAULT_AUDIO_TYPE,
					      JackPortIsInput, 0);
	DBG2(if(ports[channel-1])
	     std::cerr<<"jack_dport.input[0x"
	     <<std::hex<<std::setw(4)<<(void*)this<<std::dec
	     <<"] <"<<name<<">: Registered."
	     <<std::endl;
	     else
	     std::cerr<<"jack_dport.input <"<<name<<">: Failed to register!"
	     <<std::endl);

	disk_thread::unlock();
}

audio_engine::dport *audio_engine::add_input(int index, unsigned int channels)
{
	// TODO: Check if already existing!!

	jack_dport *r = new jack_dport(index, channels);
	g_inputs = g_inputs->push(r);

	DBG2(std::cerr<<"audio_engine.add_input[0x"
	     <<std::hex<<std::setw(4)<<(void*)r<<std::dec
	     <<"] ("<<index<<", "<<channels<<")."<<std::endl);
	return r;
}

audio_engine::dport *audio_engine::add_output(int index, unsigned int channels)
{
	// TODO: Check if already existing!!

	jack_dport *r = new jack_dport(index, channels);
	g_outputs = g_outputs->push(r);
	return r;
}

size_t jack_dport::start_offset(metronome *m, jack_nframes_t nframes)
{
//	const bbtf &now = m->get_current_time();
	// TODO: Keep a start_time in jack_dport (?)
	// Only calculate a "rounding" (frame) offset from this time.
	return 0;
}

size_t jack_dport::bytes_per_frame() const
{
	return sample_size * get_channels();
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


void process_recording(jack_dport *d, metronome *m, jack_nframes_t nframes)
{
	unsigned int i, c;
	jack_ringbuffer_t *rb = d->get_ringbuffer();
	unsigned int nports = d->get_channels();

	for(c=0; c<nports; ++c){
		in_buf[c] = (jack_default_audio_sample_t *)
			jack_port_get_buffer(d->get_port(c), nframes);
	}
	i = d->start_offset(m, nframes);
	for( ; i<nframes; ++i){
		for(c=0; c<nports; ++c){
			if(jack_ringbuffer_write(rb,
						 (char*)(in_buf[c]+i),
						 sample_size)
			   < sample_size)
				++overruns;
		}
	}
}

const size_t output_channels = 2;
const jack_nframes_t samples_per_frame = output_channels;
const size_t bytes_per_frame = samples_per_frame * sample_size;
jack_default_audio_sample_t framebuf[output_channels];
jack_default_audio_sample_t *out_buf[output_channels];

void process_playback(jack_dport *d, metronome *m, jack_nframes_t nframes)
{
	unsigned int i, c;
	jack_ringbuffer_t *rb = d->get_ringbuffer();
	if(!rb) return;

	unsigned int nports = d->get_channels();
	if(nports > output_channels) nports = output_channels; // TODO: !!
	size_t bytes_per_frame = nports * sample_size;

	i = d->start_offset(m, nframes);
	for( ; i<nframes; ++i){
		if(jack_ringbuffer_read_space(rb) >= bytes_per_frame){
			jack_ringbuffer_read(rb, (char*)framebuf,
					     bytes_per_frame);
			unsigned int o = 0;
			for(c=0; c<output_channels; ++c){
				// Mix all sources down to master outputs
				*(out_buf[c]+i) += framebuf[o];
				if(++o >= nports) o=0;
			}
		}
		else {
			++underruns;
			// TODO: Add nframes to channel offset ?
			break;
		}
	}
}

int audio_engine::process(jack_nframes_t nframes, void *arg)
{
	audio_engine *a = (audio_engine *)arg;
	metronome *m = a->get_metronome();
	if(m) m->add_frames(nframes);

	size_t c;
	for(c=0; c<output_channels; ++c){
		out_buf[c] = (jack_default_audio_sample_t *)
			jack_port_get_buffer(output_port[c], nframes);
		memset(out_buf[c], '\0', nframes*sample_size);
	}

	cav::slist_mrsw<jack_dport> *pit = g_outputs;
	while(pit && pit->data()){
		jack_dport *d = pit->data();
		if(d->is_playing_or_flushing(m)){
			process_playback(d, m, nframes);
		}
		pit = pit->next();
	}


	pit = g_inputs;
	while(pit && pit->data()){
		jack_dport *d = pit->data();
		if(d->is_recording(m)) process_recording(d, m, nframes);
		pit = pit->next();
	}

	disk_thread::wake();
	return 0;
}

void jack_shutdown(void *)
{
	std::cerr<<"jack_shutdown: No handler registered..."<<std::endl;
	exit(1);
}

audio_engine::audio_engine()
	: sfn(jack_shutdown), m(0)
{}

void audio_engine::initialize()
{
	if((client = jack_client_new(name.c_str())) == 0){
		throw init_failure("jack_client_new");
	}

	sample_rate = jack_get_sample_rate(client);
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

	if(!diskthread) diskthread = new disk_thread(m);

	if(jack_activate(client)){
		shutdown();
		throw init_failure("jack_activate");
	}

	diskthread->run();
}


void audio_engine::halt()
{
	if(diskthread){
		diskthread->halt();
		delete diskthread;
		diskthread = 0;
	}
	cav::slist_mrsw<jack_dport> *pit = g_outputs;
	while(pit && pit->data()){
		pit->data()->halt();
		pit = pit->next();
	}

	pit = g_inputs;
	while(pit && pit->data()){
		pit->data()->halt();
		pit = pit->next();
	}
}

void audio_engine::shutdown()
{
	halt();

	if(client){
		jack_client_close(client);
		client = 0;
	}

	if(underruns){
		std::cerr<<"detected # of buffer underruns: "
			 <<underruns<<std::endl;
	}
	if(overruns){
		std::cerr<<"detected # of buffer overruns: "
			 <<overruns<<std::endl;
	}
	delete g_inputs;
	delete g_outputs;
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

