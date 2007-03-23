#include "midi_engine.h"
#include "util/debug.h"

#include <iostream>

#include <alsa/asoundlib.h>

void midi_engine::initialize()
{
	if(seq_handle) return; // TODO: Also verify that we are connected?
	if(snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0)
		throw init_failed();
	snd_seq_set_client_name(seq_handle, name.c_str());
	make_input(name + "_in0");
}


void midi_engine::shutdown()
{
	if(port_in) snd_seq_delete_port(seq_handle, port_in);
	if(seq_handle) snd_seq_close(seq_handle);
}


void midi_engine::register_channel(input_channel *)
{
}

void midi_engine::register_channel(output_channel *)
{
}

void midi_engine::make_input(const std::string &name)
{
	port_in = snd_seq_create_simple_port(
		seq_handle, name.c_str(),
		SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
		SND_SEQ_PORT_TYPE_APPLICATION);
	if(port_in < 0) throw failed_input_port(name);
	DBG(std::cerr << "Initialized new MIDI input channel: "
	    << name << "(" << snd_seq_client_id(seq_handle) << ":"
	    << port_in << ")" << std::endl;)
}

/*
bool midi_engine::has_event() const
{
	return snd_seq_event_input_pending(seq_handle, 1) > 0;
// 	if(!port_in) return false;
// 	int n, i;
// 	int pfd_count = count_in;
// 	struct pollfd pfd[1];
// 	snd_seq_poll_descriptors(seq_handle, pfd, 1, POLLIN);

// 	n = poll(pfd, 1, 0);
// 	if(n == -1) return -1;
// 	return (n == 1);
}

snd_seq_event_t *midi_engine::get_event()
{
	snd_seq_event_t *ev;
	snd_seq_event_input(seq_handle, &ev);
	return ev;
}
*/
