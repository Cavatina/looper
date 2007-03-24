#include "audio_engine.h"

#include <jack/jack.h>
#include <jack/transport.h>
#include <alsa/asoundlib.h>

#include <iostream>

int process(jack_nframes_t nframes, void *arg)
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

	jack_set_process_callback(client, process, this);
	jack_on_shutdown(client, sfn, this);
	output_port[0] = jack_port_register(client, "output_1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	output_port[1] = jack_port_register(client, "output_2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
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

void audio_engine::register_channel(input_channel *)
{
}

void audio_engine::register_channel(output_channel *)
{
}

// used by looper to determine required buffering length etc.
uint32_t audio_engine::get_sample_rate() const
{
	return 0;
}

