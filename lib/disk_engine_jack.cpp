#include "disk_engine.h"
#include "jack_engine.h"
#include "util/debug.h"
#include "util/slist.h"
#include <iostream>
#include <algorithm>

pthread_mutex_t disk_thread_lock = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t disk_thread_lock_data_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t data_ready = PTHREAD_COND_INITIALIZER;

extern cav::slist_mrsw<jack_dport> *g_inputs;
extern cav::slist_mrsw<jack_dport> *g_outputs;

//extern std::list<jack_dport *> inputs;
//extern std::list<jack_dport *> outputs;

extern jack_client_t *client;
extern jack_nframes_t sample_rate;

const size_t bitdepth = 24;

void disk_thread::lock()
{
	pthread_mutex_lock(&disk_thread_lock);
}

void disk_thread::unlock()
{
	pthread_mutex_unlock(&disk_thread_lock);
}

void disk_thread::wake()
{
	if(pthread_mutex_trylock (&disk_thread_lock) == 0){
		pthread_cond_signal (&data_ready);
		pthread_mutex_unlock (&disk_thread_lock);
	}
}

SNDFILE *jack_dport::make_new_soundfile(const std::string &name,
					unsigned int channels)
{
	SNDFILE *out;
	SF_INFO sf_info;
	int short_mask;
	memset(&sf_info, '\0', sizeof(SF_INFO));
	sf_info.samplerate = sample_rate;
	sf_info.channels = channels;
	
	switch(bitdepth){
		case 8: short_mask = SF_FORMAT_PCM_U8;
		  	break;
		case 16: short_mask = SF_FORMAT_PCM_16;
			 break;
		case 24: short_mask = SF_FORMAT_PCM_24;
			 break;
		case 32: short_mask = SF_FORMAT_PCM_32;
			 break;
		default: short_mask = SF_FORMAT_PCM_16;
			 break;
	}
	sf_info.format = SF_FORMAT_WAV|short_mask;

	out = sf_open(name.c_str(), SFM_WRITE, &sf_info);
	if(!out){
		std::cerr<<"make_new_soundfile: Failed to open <"
			 <<name<<"> ("<<sf_strerror(0)<<")"<<std::endl;
	}
	else {
		DBG2(std::cerr<<"make_new_soundfile <"
		     <<name<<">: Opened Ok."<<std::endl);
	}
	return out;
}

void jack_dport::cancel_record()
{
	std::list<action *>::iterator i = actions.begin();
	for(; i != actions.end(); ++i){
		record_action *r = dynamic_cast<record_action *>(*i);
		if(r){
			recording = false;
			DBG2(std::cerr<<"cancel_record <"
			     <<r->s->get_source()<<">: Closed."<<std::endl);
			if(r->sf){
				sf_close(r->sf);
				r->sf = 0;
			}
			delete r;
			actions.erase(i);
			return;
		}
	}
}

SNDFILE *jack_dport::record_soundfile()
{
	std::list<action *>::iterator i = actions.begin();
	for(; i != actions.end(); ++i){
		record_action *r = dynamic_cast<record_action *>(*i);
		if(r){
			if(!r->sf){
				r->sf = make_new_soundfile(r->s->get_source(),
							   get_channels());
			}
			return r->sf;
		}
	}
	return 0;
}

std::string jack_dport::record_soundfile_name() const
{
	std::list<action *>::const_iterator i = actions.begin();
	for(; i != actions.end(); ++i){
		const record_action *r = dynamic_cast<record_action *>(*i);
		if(r) return r->s->get_source();
	}
	return "";
}

void jack_dport::buffer()
{
	const size_t write_samples = 2048;//128;
	if(!is_playing()) return;

	size_t frames_avail = jack_ringbuffer_write_space(rb)
		/ bytes_per_frame();
	if(!frames_avail) return;

}

void jack_dport::flush()
{
	const size_t write_samples = 2048;//128;
	jack_default_audio_sample_t buf[write_samples];
	size_t frames_avail = jack_ringbuffer_read_space(rb)
		/ bytes_per_frame();
	if(!frames_avail) return;

	frames_avail = std::min(frames_avail, write_samples/get_channels());
	size_t r = jack_ringbuffer_read(rb, (char*)buf,
					frames_avail * bytes_per_frame())
		/ bytes_per_frame();

	SNDFILE *sf = record_soundfile();
	if(!sf) cancel_record();
	else if(sf_writef_float(sf, buf, r) != r){
		char errstr[256];
		sf_error_str(sf, errstr, sizeof(errstr) - 1);
		std::cerr<<"disk_thread.write <"<<record_soundfile_name()
			 <<">: "<<errstr<<std::endl;
		cancel_record();
		// TODO: Close file!
	}
}

void *disk_thread::main(void *arg)
{
	disk_thread *me = (disk_thread *)arg;
	DBG2(std::cerr<<"disk_thread: Says hello and good morning!"
	     <<std::endl);

	pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	pthread_mutex_lock (&disk_thread_lock);

	while(me->running){
		cav::slist_mrsw<jack_dport> *pit;
		pit = g_outputs;
		while(pit && pit->data()){
			pit->data()->buffer();
			pit = pit->next();
		}

		pit = g_inputs;
		while(pit && pit->data()){
			pit->data()->flush();
			pit = pit->next();
		}
		pthread_cond_wait (&data_ready, &disk_thread_lock);
	}
	return 0;
}

disk_thread::disk_thread()
	: running(false), thread_id(0), metro(0), client(0)
{
}

void disk_thread::run()
{
	running = true;
	if(!thread_id){
		int r = pthread_create(&thread_id, NULL,
				       disk_thread::main,
				       (void*)this);
		if(r){
			std::cerr<<"disk_thread: Failed to create!"<<std::endl;
		}
		else {
			DBG2(std::cerr<<"disk_thread: Spawned Ok."<<std::endl);
		}
	}
}

void disk_thread::halt()
{
	running = false;
	pthread_join(thread_id, NULL);
	thread_id = 0;
}
