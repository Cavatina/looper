#include "audio_engine.h"

void audio_engine::initialize()
{
}


void audio_engine::shutdown()
{
}


// function to call on audioengine shutdown
void audio_engine::kill_bill(void *)
{
}

void audio_engine::register_channel(input_channel *)
{
}

void audio_engine::register_channel(output_channel *)
{
}

// used by looper to determine required buffering length etc.
uint32_t audio_engine::get_sample_rate() const
{
	return 0;
}

