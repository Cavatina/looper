#include "looper.h"

#include <iostream>
#include <stdexcept>

#include "command.h"

void looper::set_banks(size_t new_size)
{
	while(new_size > banks.size()){
		banks.push_back(new bank());
	}
	if(new_size < banks.size()){
		for(size_t i=new_size; i<banks.size(); ++i) delete banks[i];
		banks.resize(new_size);
	}
}

void looper::set_persistent_storage(persistent_storage *s)
{
	storage.reset(s);
}

void looper::set_audio_engine(audio_engine *e)
{
	audio.reset(e);
}

void looper::set_midi_engine(midi_engine *e)
{
	midi.reset(e);
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
		return banks[index-1];
	}
	else {
		return 0;
	}
}

void looper::initialize()
{
	read_storage();
}

void looper::run()
{
	while(1){
		try {
			command *c = command_parse(this, "bank:1:stop");
			c->execute();
			delete c;
		}
		catch(const command::error &e){
			std::cerr << e.what() << std::endl;
		}
		sleep(1);
	}
}

void looper::shutdown()
{
	set_banks(0);
	storage.reset(0); // Force a save() if dirty...
	audio.reset(0);
	midi.reset(0);
}
