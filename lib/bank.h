#ifndef _LOOPER_BANK_H_
#define _LOOPER_BANK_H_

#include <vector>
#include <string>
#include <deque>

#include "channel.h"
#include "sample.h"
#include "metronome.h"

//float time_beats_per_bar = 4.0;
//float time_beat_type = 4.0;
//double time_ticks_per_beat = 1920.0;
//double time_beats_per_minute = 120.0;

class bank
{
public:
	// loop() : Schedule playing in a endless loop, until called again.
	//          If recording, stop and schedule loop;
	//          If called again when still scheduled, cancel schedule.
	void loop();

	// play() : Increment scheduled play count.
	//          If recording, stop record and schedule for playing.
	void play();

	// stop() : Stop playing or looping immediately.
	//          If recording, discard recorded sample.
	void stop();

	// record() : Schedule recording (and stop playing)
	//            If called while recording, schedule stop record.
	//            (Without scheduling for playback)
	void record();

	// play_once() : Schedule playback one time.
	void play_once();

	// cycle_samples() : Cycle between bank samples.
	void cycle_samples();

	// discard() : Discard current sample.
	void discard();

	void set_name(const std::string &name_);
	std::string get_name() const { return name; }

	void set_index(size_t index_) { index = index_; }
	// Implicit by samples? => no, decided by input channels,
	// but should allow samples with less channels.
	unsigned short get_channels() const;
	void add_channel(channel *);
	void add_sample(sample *);

	sample *get_sample(unsigned short);
	sample *get_current_sample();
	unsigned short get_sample_index(sample *) const;
	unsigned short get_sample_count() const;
	void set_sample_index(sample *, unsigned short);
	void remove_sample(sample *);

	bool is_playing_or_scheduled() const;
	bool is_recording() const;

	void process_recorded_channels();

	size_t get_audio(unsigned short channel, void *buf, size_t);
	uint32_t get_current_sample_frame();

private:
	void finalize_recording();

	std::deque<sample *> samples;
	std::deque<channel *> channels;

	std::string name;
	size_t index;

	bool recording;
	bool playing;
	std::auto_ptr<bbt> scheduled_record;
	std::auto_ptr<bbt> scheduled_play;
	std::auto_ptr<bbt> scheduled_stop;

	int loops_to_play;
	unsigned short current_sample;
};

#endif
