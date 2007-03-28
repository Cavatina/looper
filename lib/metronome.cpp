#include "metronome.h"

#include "debug.h"
#include <iostream>

metronome::metronome() : running(false), framerate(96000)
{
}

void metronome::start() // pause if already running
{
	// Should round time up to nearest beat
	running = !running;
	DBG2(std::cerr<<"framerate: "<<framerate
	     <<", frames_per_bar: "<<get_frames_per_bar(get_current_tempo())
	     <<", beats_per_bar: "<<get_current_tempo()->beatsperbar
	     <<std::endl);
}

void metronome::stop()
{
	running = false;
	current_time.reset();
	tempo_it = tempo_changes.begin();
}

void metronome::toggle()
{
	if(running) stop();
	else start();
}

void metronome::set_framerate(uint32_t framerate_)
{
	framerate = framerate_;
}

void metronome::clear()
{
	tempo_changes.clear();
	tempo_it = tempo_changes.end();
}

void metronome::add(const tempo &t)
{
	tempo_changes.push_back(t);
	tempo_changes.sort();
	tempo_it = tempo_changes.begin();
}

tempo *metronome::get_current_tempo()
{
	static tempo default_tempo(bbt(1, 1, 0), 120, 4, 4);
	if(tempo_it == tempo_changes.end()){
		return &default_tempo;
	}
	std::list<tempo>::iterator j = tempo_it; ++j;
	if(j != tempo_changes.end() && j->when < current_time) tempo_it=j;
	return &*tempo_it;
}

bbt metronome::next_bar() const
{
	return bbt(current_time.bar+1, 1, 0);
}

uint32_t metronome::get_frames_per_bar(const tempo *t)
{
	return (uint32_t)(framerate * 60 * t->beatsperbar / (double)t->bpm);
}

uint32_t metronome::frames_to_beat(const tempo *t, uint32_t frames)
{
	return 1+frames*t->beatsperbar/get_frames_per_bar(t);
}

uint32_t metronome::frames_to_tick(const tempo *t, uint32_t frames)
{
	return frames*t->beatsperbar/get_frames_per_bar(t)/960;
}

void metronome::add_frames(uint32_t delta)
{
	if(!running) return;
	current_time.frame += delta;
	for(;framerate;){
		tempo *t = get_current_tempo();
		if(current_time.frame >= get_frames_per_bar(t)){
			++current_time.bar;
			current_time.beat = 1;
			current_time.tick = 0;
			current_time.frame -= get_frames_per_bar(t);
			continue;
		}
		current_time.beat = frames_to_beat(t, current_time.frame);
		current_time.tick = frames_to_tick(t, current_time.frame);
		break;
	}
}
