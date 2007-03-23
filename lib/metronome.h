#ifndef _LOOPER_METRONOME_H_
#define _LOOPER_METRONOME_H_

#include <list>

struct bbt
{
	int32_t bar;
	int32_t beat;
	int32_t tick;

	bool operator<(const bbt &rhs){
		return (bar < rhs.bar ||
			(bar == rhs.bar &&
			 (beat < rhs.beat ||
			  (beat == rhs.beat && tick < rhs.tick))));
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
	int32_t get_beat();
	int32_t get_bar();
	int32_t get_tick();
	int32_t get_frame();

	void clear() { tempo_changes.clear(); }
	void add(const tempo &t) {
		tempo_changes.push_back(t);
		tempo_changes.sort();
	}

private:
	std::list<tempo> tempo_changes;
};

#endif
