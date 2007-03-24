#include "bank.h"

#include <iostream>

#include "util/debug.h"

void bank::loop()
{
}

void bank::play()
{
}

void bank::stop()
{
}

void bank::record()
{
}

void bank::play_once()
{
}

void bank::cycle_samples()
{
	// TODO: Fade out/in to ensure no noise...
	sample *s = samples.front();
	samples.push_back(s);
	samples.pop_front();
}

void bank::discard()
{
	if(samples.empty()) return;
	sample *s = samples.front();
	samples.pop_front();
	// Wait 1 sec so that the jack thread does not try and play a
	// deleted object...
	sleep(1);
	delete s;
}

void bank::set_name(const std::string &name_)
{
	name = name_;
}

// Implicit by samples? => no, decided by input channels,
// but should allow samples with less channels.
unsigned short bank::get_channels() const
{
	return channels.size() || 1;
}

void bank::add_sample(sample *s)
{
	samples.push_back(s);
}

void bank::add_channel(channel *c)
{
	channels.push_back(c);
}

sample *bank::get_sample(unsigned short index)
{
	if(index >= 1 && index <= get_sample_count()) samples[index-1];
	return 0;
}

sample *bank::get_current_sample()
{
	if(samples.empty()) return 0;
	else return samples.front();
}

unsigned short bank::get_sample_index(sample *s) const
{
	std::deque<sample *>::const_iterator i = samples.begin();
	unsigned short n = 1;
	for(; i != samples.end(); ++i, ++n) if(s == *i) return n;
	return 0;
}

unsigned short bank::get_sample_count() const
{
	return samples.size();
}

void bank::set_sample_index(sample *, unsigned short)
{
}

void bank::remove_sample(sample *)
{
}

bool bank::is_playing_or_scheduled() const
{
	return (playing || scheduled_play.get());
}

bool bank::is_recording() const
{
	return recording;
}

void bank::process_recorded_channels()
{
}

size_t bank::get_audio(unsigned short, void *, size_t)
{
	return 0;
}

uint32_t bank::get_current_sample_frame()
{
	return 0;
}

void bank::finalize_recording()
{
}
