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


void midi_engine::add(midi_handler *h)
{
	handlers.push_back(h);
}

void midi_engine::clear_handlers()
{
	std::list<midi_handler *>::iterator i = handlers.begin();
	for(; i != handlers.end(); ++i) delete *i;
	handlers.clear();
}

void midi_engine::input_connect(const std::string &adr)
{
	snd_seq_port_subscribe_t *s;
	snd_seq_addr_t sender, dest;

	if(snd_seq_parse_address(seq_handle, &sender, adr.c_str()) < 0){
		DBG(std::cerr<<"midi_engine.input_connect ["<<adr
		    <<"]: Invalid source address."<<std::endl);
		return;
	}

	if(snd_seq_parse_address(seq_handle, &dest, name.c_str()) < 0){
		DBG(std::cerr<<"midi_engine.input_connect ["<<name
		    <<"]: Failed to connect to ourselves."
		    <<std::endl);
		return;
	}

	snd_seq_port_subscribe_alloca(&s);
	snd_seq_port_subscribe_set_sender(s, &sender);
	snd_seq_port_subscribe_set_dest(s, &dest);

	if(snd_seq_get_port_subscription(seq_handle, s) == 0){
		DBG(std::cerr<<"midi_engine.input_connect ["<<name
		    <<"]: Failed to connect to ourselves."
		    <<std::endl);
		return;
	}

	if(snd_seq_subscribe_port(seq_handle, s) < 0){
		DBG2(std::cerr<<"midi_engine.input_connect ["<<adr
		     <<"]: Already connected."
		     <<std::endl);
		return;
	}

	DBG2(std::cerr<<"midi_engine.input_connect ["<<adr
	     <<" -> " <<name<<"]: Connected."
	     <<std::endl);
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

bool midi_engine::has_event() const
{
	return snd_seq_event_input_pending(seq_handle, 1) > 0;
}

static bool match(std::list<midi_handler *>::iterator i,
		  const std::list<midi_handler *>::iterator &e,
		  snd_seq_event_t &ev)
{
	while(i != e){
		midi_handler *h = *i;
		switch(ev.type){
		case SND_SEQ_EVENT_CONTROLLER:
			if(h->filter_controller()
			   && ev.data.control.value == h->get_controller()
			   && (!h->filter_param() ||
			       ev.data.control.param == (unsigned int)h->get_param())){
				h->execute();
				return true;
			}
			break;
		case SND_SEQ_EVENT_NOTEON:
			if(h->filter_note()
			   && ev.data.note.note == h->get_note()){
				h->execute();
				return true;
			}
		}
		++i;
	}
	return false;
}

void midi_engine::dispatch()
{
	while(snd_seq_event_input_pending(seq_handle, 1) > 0){
		snd_seq_event_t *ev;
		snd_seq_event_input(seq_handle, &ev);

		if(channel >= 0){
			// Filter event channel
			switch(ev->type){
			case SND_SEQ_EVENT_CONTROLLER:
			case SND_SEQ_EVENT_PITCHBEND:
			case SND_SEQ_EVENT_NOTEON:
			case SND_SEQ_EVENT_NOTEOFF:
				if(ev->data.control.channel != channel)
					return;
			}
		}

		snd_seq_event_t out;
		memcpy(&out, ev, sizeof(snd_seq_event_t));
		if(!match(handlers.begin(), handlers.end(), out)){
			if(out.type >= SND_SEQ_EVENT_NOTE &&
			   out.type < SND_SEQ_EVENT_CONTROLLER){
				DBG2(std::cerr<<"midi_engine: Ignored note #"
				     <<(int)out.data.note.note
				     <<" ("
				     <<(int)out.data.note.velocity
				     <<") on channel "
				     <<(int)out.data.note.channel
				     <<std::endl);
			}
			else if(out.type >= SND_SEQ_EVENT_CONTROLLER &&
				out.type < SND_SEQ_EVENT_SONGPOS){
				DBG2(std::cerr<<"midi_engine: Ignored ctrl #"
				     <<(int)out.data.control.value
				     <<" ("
				     <<(int)out.data.control.param
				     <<") on channel "
				     <<(int)out.data.control.channel
				     <<std::endl);
			}
			else {
				DBG2(std::cerr<<"midi_engine: Ignored message."
				     <<std::endl);
			}
		}
	}
}
