#include "disk_engine.h"
#include "jack_engine.h"
#include "util/debug.h"
#include "util/slist.h"
#include <iostream>

pthread_mutex_t disk_thread_lock = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t disk_thread_lock_data_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t data_ready = PTHREAD_COND_INITIALIZER;

extern cav::slist_mrsw<jack_dport> *g_inputs;
extern cav::slist_mrsw<jack_dport> *g_outputs;

//extern std::list<jack_dport *> inputs;
//extern std::list<jack_dport *> outputs;

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

void *disk_thread::main(void *arg)
{
	disk_thread *me = (disk_thread *)arg;
	DBG2(std::cerr<<"disk_thread: Says hello and good morning!"
	     <<std::endl);

	pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	pthread_mutex_lock (&disk_thread_lock);

	while (me->running) {
		DBG2(std::cerr<<"disk_thread: I'm Awake!"<<std::endl);

		cav::slist_mrsw<jack_dport> *i = g_inputs;
		while(i && i->data()){
//			jack_dport *d = i->data();

			i = i->next();
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
