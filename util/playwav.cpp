#include <unistd.h>
#include <iostream>
#include <memory>
#include <algorithm>

#include <sys/signal.h>

#include <jack/jack.h>
#include <jack/transport.h>
#include <sndfile.h>
#include <soundtouch/SoundTouch.h>
#include "termc.h"

typedef jack_default_audio_sample_t sample_t;
jack_client_t *client = 0;
jack_port_t *output_port[2] = {0,0};
unsigned long sample_rate;

std::string client_name = "playwav";
soundtouch::SoundTouch st;
SNDFILE *sf = 0;
SF_INFO sfi;
#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))

jack_nframes_t read_stereo_samples(sample_t *buffer1, sample_t *buffer2, jack_nframes_t nframes)
{
	float fbuf[8192];
	unsigned int ch = sfi.channels;
	unsigned int rd = 0;
	do {
		unsigned int r = 0;
		if(!st.numSamples()){
			r = sf_readf_float(sf, fbuf, ARRAYSIZE(fbuf)/ch);
			if(!r) break;
			st.putSamples(fbuf, r);
			if(r < ARRAYSIZE(fbuf)/2) st.flush(); // the END?
		}
		unsigned int cnt = std::min(nframes-rd, (unsigned int)(ARRAYSIZE(fbuf)/ch));
		unsigned int rcv = st.receiveSamples(fbuf, cnt);
		for(unsigned int i=0; i<rcv; ++i){
			buffer1[rd] = fbuf[i*2];
			buffer2[rd] = fbuf[i*2+1];
			++rd;
		}
	}
	while(rd < nframes);
	return rd;
}

int process(jack_nframes_t nframes, void *arg)
{
	sample_t *buffer1 =
		(sample_t *) jack_port_get_buffer(output_port[0], nframes);
	sample_t *buffer2 =
		(sample_t *) jack_port_get_buffer(output_port[1], nframes);

	jack_nframes_t r = read_stereo_samples(buffer1, buffer2, nframes);
//	std::cerr << "r: " << r << ", nframes: " << nframes << "\n";
	if(r < nframes){
		size_t unfilled = (nframes - r) * sizeof(sample_t);
		memset(buffer1 + r * sizeof(sample_t), '\0', unfilled);
		memset(buffer2 + r * sizeof(sample_t), '\0', unfilled);
	}

	return 0;
}

void jack_shutdown(void *arg)
{
	std::cerr << "jack_shutdown()" << std::endl;
	exit(1);
}

int sample_rate_change()
{
	std::cerr << "Wheee... I'm not ready for this!" << std::endl;
	exit(1);
}

void close_audio()
{
	if(sf){ sf_close(sf); sf=0; }
	if(client){
		jack_client_close(client);
		client=0;
	}
}

int init_audio()
{
	const char **ports;

	if((client = jack_client_new(client_name.c_str())) == 0){
		std::cerr << "jack_client_new() failed" << std::endl;
		return -1;
	}

	jack_set_process_callback(client, process, 0);
	jack_on_shutdown(client, jack_shutdown, 0);
	output_port[0] = jack_port_register(client, "output_1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	output_port[1] = jack_port_register(client, "output_2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	sample_rate = jack_get_sample_rate(client);
	if(jack_activate(client)){
		std::cerr << "jack_activate() failed" << std::endl;
		close_audio();
		return -2;
	}

	if((ports = jack_get_ports(client, NULL, NULL, JackPortIsInput|JackPortIsPhysical)) == 0){
		std::cerr << "Failed to find any suitable playback ports." << std::endl;
		close_audio();
		return -3;
	}

	int i=0;
	while(ports[i] && i<2){
		if(jack_connect(client, jack_port_name(output_port[i]), ports[i])){
			std::cerr << "Failed to connect output port " << (i+1) << " to " << ports[i] << "." << std::endl;
			free(ports);
			close_audio();
			return -4;
		}
		++i;
	}

	free(ports);

	return 0;
}

void signal_handler(int)
{
	close_audio();
	exit(1);
}

void signal_init()
{
        struct sigaction sa;
	sa.sa_handler = signal_handler;
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);               /* Capture CTRL+C            */
	sigaction(SIGTERM, &sa, NULL);              /* Capture 'kill <our_pid>'  */
}

void usage()
{
	std::cerr << "playwav test application" << std::endl;
}

int main(int argc, char *argv[])
{
	if(argc < 2){
		std::cerr << "playwav: Missing input file!" << std::endl;
		return 2;
	}

	if(init_audio() < 0) return 1;
	try {
		float tempo = 1.0f;
		sfi.format = 0;
		sf = sf_open(argv[1], SFM_READ, &sfi);
		st.setSampleRate(sfi.samplerate);
		st.setChannels(sfi.channels);
		st.setTempo(tempo);
		st.setRate((float)sfi.samplerate/sample_rate);
		std::cout << "Opened file <" << argv[1] << ">:\n"
			  << "channels = " << sfi.channels << "\n"
			  << "frames = " << sfi.frames << "\n"
			  << "samplerate = " << sfi.samplerate << "\n";

		signal_init();
		init_terminal();

		while(std::cin.good()){
			char c = std::cin.get();
			if((c|0x20) == 'q') break;
			else if((c|0x20) == 'a' || c == '-'){
				if(tempo > 0.50f) st.setTempo(tempo -= 0.01f);
			}
			else if((c|0x20) == 'z' || c == '+'){
				if(tempo < 2.00f) st.setTempo(tempo += 0.01f);
			}
		}
	}
	catch(std::exception &e){
		std::cerr << "Fatal exception: " << e.what() << std::endl;
		close_audio();
		reset_terminal();
		return 1;
	}
	close_audio();
	reset_terminal();
	return 0;
}
