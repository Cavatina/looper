#ifndef _LOOPER_DISK_ENGINE_H_
#define _LOOPER_DISK_ENGINE_H_

#include <pthread.h>
#include <jack/jack.h>
#include "metronome.h"

class disk_thread
{
public:
	disk_thread(metronome *m_);

	void run();
	void halt();
	static void *main(void *);

	// pthread helpers..
	static void lock();
	static void unlock();
	static void wake();

	metronome *get_metronome() { return metro; }

private:
	volatile bool running;
	pthread_t thread_id;
	metronome *metro;
	jack_nframes_t duration;
	jack_nframes_t rb_size;
	jack_client_t *client;
};


#endif

