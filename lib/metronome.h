#ifndef _LOOPER_METRONOME_H_
#define _LOOPER_METRONOME_H_

#include <list>
#include <iostream>
#include "util/to_string.h"

struct bbt
{
	bbt() : bar(1), beat(1), tick(0) {}
	bbt(uint32_t bar_, uint32_t beat_, uint32_t tick_)
		: bar(bar_), beat(beat_), tick(tick_) {}

	uint32_t bar;
	uint32_t beat;
	uint32_t tick;

	bool operator>(const bbt &rhs) const { return !operator<=(rhs); }
	bool operator<=(const bbt &rhs) const { return !rhs.operator<(*this); }
	bool operator>=(const bbt &rhs) const { return !operator<(rhs); }
	bool operator<(const bbt &rhs) const {
		return (bar < rhs.bar ||
			(bar == rhs.bar &&
			 (beat < rhs.beat ||
			  (beat == rhs.beat && tick < rhs.tick))));
	}
	std::string to_string() const {
		return std::string(::to_string(bar)+"/"+
				   ::to_string(beat)+"/"+
				   ::to_string(tick));
	}
};

struct bbtf : public bbt
{
	bbtf() : bbt(), frame(0) {}
	bbtf(const bbt &src) : bbt(src), frame(0) {}

	uint32_t frame;

	bool operator>=(const bbt &rhs) const { return !operator<(rhs); }
	bool operator<(const bbtf &rhs) const {
		return (bar < rhs.bar ||
			(bar == rhs.bar && frame < rhs.frame));
	}
	std::string to_string() const {
		return std::string(::to_string(bar)+"/"+
				   ::to_string(beat)+"/"+
				   ::to_string(tick)+":"+
				   ::to_string(frame));
	}
	void reset() {
		bar = beat = 1;
		tick = frame = 0;
	}
};

inline std::ostream &operator <<(std::ostream &os, const bbt &obj)
{
	os << obj.to_string();
	return os;
}

inline std::ostream &operator <<(std::ostream &os, const bbtf &obj)
{
	os << obj.to_string();
	return os;
}

struct tempo
{
	tempo(const bbt &when_, int32_t bpm_, int32_t bpb_, int32_t notetype_)
		: when(when_), bpm(bpm_), beatsperbar(bpb_),
		  notetype(notetype_)
		{}

	bbt when;
	int32_t bpm;
	int32_t beatsperbar;
	int32_t notetype;

	bool operator<(const tempo &rhs){ return when < rhs.when; }
};

class metronome
{
public:
	metronome();

	void start();
	void stop();
	void toggle();

	int32_t get_beat();
	int32_t get_bar();
	int32_t get_tick();
	int32_t get_frame();

	void set_framerate(uint32_t);
	void clear();
	void add(const tempo &t);

	void add_frames(uint32_t);
	bbtf &get_current_time() { return current_time; }
	const tempo *get_current_tempo();
	const tempo *get_tempo_at(const bbt &) const;
	bool is_running() const { return running; }
	uint32_t get_frames_per_bar(const tempo *) const;
	uint32_t frames_to_beat(const tempo *, uint32_t);
	uint32_t frames_to_tick(const tempo *, uint32_t);

	bbt now() const { return current_time; }
	bbt next_bar() const;
	bbt end_bar(const bbt &, uint32_t) const;

private:
	std::list<tempo> tempo_changes;
	std::list<tempo>::const_iterator tempo_it;
	bool running;
	bbtf current_time;
	uint32_t framerate;
};

#endif
