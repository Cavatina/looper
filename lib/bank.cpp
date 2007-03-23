#include "bank.h"


void bank::play()
{
}


void bank::stop()
{
}

void bank::record()
{
}

void bank::stop_record_start_play()
{
}

void bank::stop_record_discard()
{
}

void bank::play_increment_count()
{
}

void bank::cycle_samples()
{
}

void bank::play_times(unsigned short)
{
}

void bank::set_name(const std::string &)
{
}

// Implicit by samples? => no, decided by input channels,
// but should allow samples with less channels.
void bank::set_channels(unsigned short)
{
}

void bank::add_sample(sample *s)
{
	samples.push_back(s);
}

void bank::add_channel(channel *c)
{
	channels.push_back(c);
}

sample *bank::get_sample(unsigned short)
{
	return 0;
}

sample *bank::get_current_sample()
{
	return 0;
}

unsigned short bank::get_sample_index(sample *) const
{
	return 0;
}

void bank::set_sample_index(sample *, unsigned short)
{
}

void bank::remove_sample(sample *)
{
}

bool bank::is_playing_or_scheduled() const
{
	return false;
}
bool bank::is_recording() const
{
	return false;
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
