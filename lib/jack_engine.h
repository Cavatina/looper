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

static const int DEFAULT_BUFFER_SIZE = 0x10000;

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
	virtual ~action() {}
};

struct stop_action : public action
{
	stop_action(const bbt &when) : action(Stop, when) {}
};

struct play_action : public action
{
	play_action(const bbt &when, sample *s_)
		: action(Play, when), s(s_), sf(0) {}
	sample *s;
	SNDFILE *sf;
};

struct record_action : public action
{
	record_action(const bbt &when, sample *s_)
		: action(Record, when), s(s_), sf(0), halted(false) {}
	sample *s;
	SNDFILE *sf;
	bool halted;
};

class jack_dport : public audio_engine::dport
{
public:
	jack_dport(int id_, int channels_ = 0);
	virtual ~jack_dport();
	bool is_playing() const;
	bool is_recording() const { return recording && !halted; }
	bool is_scheduled() const { return false; }
	bool cancel_scheduled() { return false; }

	void start(const bbt &when, sample *);
	void record(const bbt &when, sample *);
	void stop(const bbt &when);
	void stop_now();

	void input(unsigned int, const std::string &);
	jack_port_t *get_port(unsigned int index) { return ports[index]; }
	jack_ringbuffer_t *get_ringbuffer() { return rb; }
	size_t start_offset(metronome *, jack_nframes_t);
	size_t bytes_per_frame() const;

	void flush();
	void buffer();
	SNDFILE *record_soundfile();
	std::string record_soundfile_name() const;
	void cancel_record();
	void halt() { halted = true; }

private:
	static SNDFILE *make_new_soundfile(const std::string &, unsigned int);

	std::list<action *> actions;
	std::vector<jack_port_t *> ports;
	jack_ringbuffer_t *rb;
	bool recording;
	bool halted;
};

#endif
