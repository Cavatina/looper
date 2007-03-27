#ifndef _LOOPER_METRONOME_H_
#define _LOOPER_METRONOME_H_

#include <list>

struct bbt
{
	bbt() : bar(1), beat(1), tick(0) {}
	bbt(uint32_t bar_, uint32_t beat_, uint32_t tick_)
		: bar(bar_), beat(beat_), tick(tick_) {}

	uint32_t bar;
	uint32_t beat;
	uint32_t tick;

	bool operator<(const bbt &rhs){
		return (bar < rhs.bar ||
			(bar == rhs.bar &&
			 (beat < rhs.beat ||
			  (beat == rhs.beat && tick < rhs.tick))));
	}
};

struct bbtf : public bbt
{
	uint32_t frame;

	bool operator<(const bbtf &rhs){
		return (bar < rhs.bar ||
			(bar == rhs.bar && frame < rhs.frame));
	}
	void reset() {
		bar = beat = 1;
		tick = frame = 0;
	}
};

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
	tempo *get_current_tempo();
	bool is_running() const { return running; }
	uint32_t get_frames_per_bar(const tempo *);
	uint32_t frames_to_beat(const tempo *, uint32_t);
	uint32_t frames_to_tick(const tempo *, uint32_t);

	bbt now() const { return current_time; }
	bbt next_bar() const;

private:
	std::list<tempo> tempo_changes;
	std::list<tempo>::iterator tempo_it;
	bool running;
	bbtf current_time;
	uint32_t framerate;
};

#endif
