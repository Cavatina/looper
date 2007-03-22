#include "looper.h"

void looper::set_banks(size_t new_size)
{
	
}

void looper::set_persistent_storage(persistent_storage *s)
{
	storage.reset(s);
}

void looper::set_audio_engine(audio_engine *e)
{
	audio.reset(e);
}

void looper::read_storage()
{
	if(storage.get()) storage->read();
}

void looper::dirty()
{
	if(storage.get()) storage->mark_dirty();
}

bank *looper::get_bank(size_t index)
{
	if(index > 0 && index <= banks.size()){
		return banks[index];
	}
	else {
		return 0;
	}
}

void looper::initialize()
{

}

void looper::run()
{

}

void looper::shutdown()
{
	storage.reset(0); // Force a save() if dirty...
	audio.reset(0);
}
