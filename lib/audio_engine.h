#ifndef _LOOPER_AUDIO_ENGINE_H_
#define _LOOPER_AUDIO_ENGINE_H_

#include <string>
#include "channel.h"

class audio_engine
{
public:
	void set_name(const std::string &name_) { name = name_; }
	void initialize();
	void shutdown();

	// function to call on audioengine shutdown
	void kill_bill(void *);

	void register_channel(input_channel *);
	void register_channel(output_channel *);

	// used by looper to determine required buffering length etc.
	uint32_t get_sample_rate() const;

private:
	std::string name;
};

#endif
