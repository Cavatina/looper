#ifndef _LOOPER_MIDI_ENGINE_H_
#define _LOOPER_MIDI_ENGINE_H_

#include <stdexcept>
#include <string>
#include "channel.h"

typedef struct _snd_seq snd_seq_t;

class midi_engine
{
public:
	void set_name(const std::string &name_) { name = name_; }
	void initialize();
	void shutdown();

	void register_channel(input_channel *);
	void register_channel(output_channel *);

	// used by looper to determine required buffering length etc.
	uint32_t get_sample_rate() const;

	struct error : public std::runtime_error
	{
		error(const std::string &s) : std::runtime_error(s) {}
	};

	struct init_failed : public error
	{
		init_failed() : error("Failed to initialize alsa sequencer.\n") {}
	};

	struct failed_input_port : public error
	{
		failed_input_port(const std::string &s) : error("Could not make MIDI input port [" + s + "].") {}
	};


private:
	void make_input(const std::string &);

	std::string name;
	int port_in;
	snd_seq_t *seq_handle;
};

#endif
