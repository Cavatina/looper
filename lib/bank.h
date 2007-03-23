#ifndef _LOOPER_BANK_H_
#define _LOOPER_BANK_H_

#include <vector>
#include <string>
#include <deque>

#include "channel.h"
#include "sample.h"

//float time_beats_per_bar = 4.0;
//float time_beat_type = 4.0;
//double time_ticks_per_beat = 1920.0;
//double time_beats_per_minute = 120.0;

class bank
{
public:
	void play();
	void stop();
	void record();
	void stop_record_start_play();
	void stop_record_discard();
	void play_increment_count();
	void cycle_samples();
	void play_times(unsigned short);


	void set_name(const std::string &name_);
	std::string get_name() const { return name; }

	void set_index(size_t index_) { index = index_; }
	// Implicit by samples? => no, decided by input channels,
	// but should allow samples with less channels.
	void set_channels(unsigned short);
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

	unsigned short loops_to_play;
	std::string name;
	size_t index;
};

#endif
