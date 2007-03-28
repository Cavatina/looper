#ifndef _LOOPER__JACK_ENGINE_H_
#define _LOOPER__JACK_ENGINE_H_

#include <list>
#include <vector>

#include "audio_engine.h"
#include "metronome.h"

#include <jack/jack.h>
#include <jack/ringbuffer.h>
#include <alsa/asoundlib.h>
#include <sndfile.h>

static const int DEFAULT_BUFFER_SIZE = 16384;

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

class jack_dport : public audio_engine::dport
{
public:
	jack_dport(int id_, int channels_ = 0);
	virtual ~jack_dport();
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

#endif
