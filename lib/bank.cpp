#include "bank.h"
#include "metronome.h"
#include "looper.h"
#include <fcntl.h>

#include <stdexcept>
#include <iostream>

#include "util/debug.h"
#include "util/to_string.h"
#include "util/string_split.h"
#include "util/fs.h"

extern looper app;

void bank::loop()
{
	if(!audio_play) return;
	if(looping){
		if(audio_play->is_playing()){
			looping = playing = false;
			audio_play->stop(m->next_bar());
		}
		else if(!audio_play->cancel_scheduled()){
			looping = playing = false;
			audio_play->stop(m->next_bar());
		}
	}
	else if(has_samples()){
		if(recording){
			audio_rec->stop(m->next_bar());
			recording = false;
		}
		looping = playing = true;
		audio_play->start(m->next_bar(), get_current_sample());
	}
}

void bank::play()
{
	if(has_samples() && audio_play){
		audio_play->start(m->next_bar(), get_current_sample());
		playing = true;
	}
}

void bank::stop()
{
	if(audio_rec) audio_rec->stop_now();
	if(audio_play) audio_play->stop_now();
	looping = playing = recording = false;
}

void bank::record()
{
	if(!audio_rec) return;
	if(playing){
		audio_play->stop(m->next_bar());
		playing = false;
	}
	if(recording){
		audio_rec->stop(m->next_bar());
	}
	else {
		samples.push_front(new sample(new_sample_name(),
					      sample_offset,
					      sample_fade_in,
					      sample_fade_out));
		audio_rec->record(m->next_bar(), samples.front());
		app.dirty();
	}
	recording = !recording;
}

void bank::play_once()
{
	if(!playing && audio_play){
		playing = true;
		audio_play->start(m->next_bar(), get_current_sample());
	}
}

void bank::cycle_samples()
{
	// TODO: Fade out/in to ensure no noise...
	sample *s = samples.front();
	samples.push_back(s);
	samples.pop_front();

	// TODO: Schedule playing offset from previous bar (?)
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

bank::~bank()
{
	std::deque<sample *>::iterator i = samples.begin();
	for(; i != samples.end(); ++i) delete *i;
	samples.clear();
}

std::string bank::channel_name(unsigned int index) const
{
	return name + "/in_" + to_string(index);
}

std::string bank::new_sample_name()
{
	int n = samples.size() + 1;
	if(!samples.empty()){
		std::deque<int> s = split_int(samples.front()->get_source());
		if(!s.empty() && s.back() > n) n = s.back();
	}

	for(int max = n + 1000; n < max; ++n){
		std::string fname = "samples/" + name + "_" +
			to_string(n) + ".wav";
		int fd = open(fname.c_str(), O_RDONLY);
		if(fd == -1){
			fs::mkpath(fname);
			return fname;
		}
		close(fd);
	}
	char buf[64] = "samples/";
	strcpy(buf+8, name.c_str());
	strcat(buf+8, ".wav.XXXXXX");
	int i = mkstemp(buf);
	if(i < 0)
		throw std::runtime_error("Failed to find unique .wav name.");
	close(i);
	return std::string(buf);
}

void bank::set_name(const std::string &name_)
{
	name = name_;
}

// Implicit by samples? => no, decided by input channels,
// but should allow samples with less channels.
unsigned short bank::get_channels() const
{
	if(audio_rec) return audio_rec->get_channels();
	else return 1;
}

void bank::add_sample(sample *s)
{
	samples.push_back(s);
}

sample *bank::get_sample(unsigned short index)
{
	if(!index) return 0;
	std::deque<sample *>::iterator i = samples.begin();
	for(; i != samples.end(); ++i) if(!--index) return *i;
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
	return (playing || looping
		|| audio_play->is_playing() || audio_play->is_scheduled());
}

bool bank::is_recording() const
{
	return recording;
}

void bank::process_recorded_channels()
{
}

void bank::finalize_recording()
{
}

void bank::set_audio_channels(audio_engine::dport *play,
			      audio_engine::dport *rec)
{
	audio_play = play;
	audio_rec = rec;
}
