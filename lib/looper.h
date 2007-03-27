#ifndef _LOOPER_H_
#define _LOOPER_H_

#include <vector>
#include <string>
#include <deque>

#include "bank.h"
#include "metronome.h"
#include "audio_engine.h"
#include "midi_engine.h"
#include "persistent_storage.h"

//float time_beats_per_bar = 4.0;
//float time_beat_type = 4.0;
//double time_ticks_per_beat = 1920.0;
//double time_beats_per_minute = 120.0;

class looper
{
public:
	// Toggle metronome start/stop
	void toggle();

	// Start metronome, if already running: pause.
	void start();

	// Stop metronome and rewind timer to 1/1/0.
	void stop();

	void set_banks(size_t);
	void set_audio_engine(audio_engine *);
	void set_midi_engine(midi_engine *);
	void set_persistent_storage(persistent_storage *);
	void read_storage();
	void dirty(); // Mark object (or a child) as `dirty'

	audio_engine *get_audio_engine() { return audio.get(); }
	midi_engine *get_midi_engine() { return midi.get(); }
	bank *get_bank(size_t);
	size_t get_banks() const { return banks.size(); }
	metronome *get_metronome() { return &m; }

	void initialize();
	void run();
	void shutdown();

private:
	std::vector<bank *> banks;
	std::auto_ptr<persistent_storage> storage;
	std::auto_ptr<audio_engine> audio;
	std::auto_ptr<midi_engine> midi;

	metronome m;
};

#endif
