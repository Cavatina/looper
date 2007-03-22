#ifndef _LOOPER_JACK_ENGINE_H_
#define _LOOPER_JACK_ENGINE_H_

#include <string>
#include "channel.h"

class jack_engine
{
public:
	void set_name(const std::string &);
	void initialize();
	void shutdown();

	// function to call on audioengine shutdown
	void kill_bill(void *);

	void register_channel(input_channel *);
	void register_channel(output_channel *);

	// used by looper to determine required buffering length etc.
	uint32_t get_sample_rate() const;
};

#endif
