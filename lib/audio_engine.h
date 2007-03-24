#ifndef _LOOPER_AUDIO_ENGINE_H_
#define _LOOPER_AUDIO_ENGINE_H_

#include <stdexcept>
#include <string>
#include "channel.h"
#include "metronome.h"

typedef struct _jack_client jack_client_t;
typedef struct _jack_port jack_port_t;

typedef void (shutdown_fn)(void *);

class audio_engine
{
public:
	audio_engine();
	void set_name(const std::string &name_) { name = name_; }
	void set_metronome(metronome *m_) { m = m_; }
	metronome *get_metronome() { return m; }

	void initialize();
	void shutdown();

	// function to call on audioengine shutdown
	void set_shutdown(shutdown_fn *f);

	void register_channel(input_channel *);
	void register_channel(output_channel *);

	// used by looper to determine required buffering length etc.
	uint32_t get_sample_rate() const;

	struct error : public std::runtime_error
	{
		error(const std::string &s) : std::runtime_error(s) {}
	};

	struct init_failure : public error
	{
		init_failure(const std::string &n) : error("Failed to initialize jack audio library [" + n + "].") {}
	};

	struct missing_playback : public error
	{
		missing_playback() : error("Failed to find any suitable playback ports.") {}
	};

	struct missing_output : public error
	{
		missing_output(const std::string &s) : error(s) {}
	};


private:
	std::string name;

	jack_client_t *client;
	jack_port_t *output_port[2];
	unsigned long sample_rate;
	shutdown_fn *sfn;
	metronome *m;
};

#endif
