#ifndef _LOOPER_AUDIO_ENGINE_H_
#define _LOOPER_AUDIO_ENGINE_H_

#include <stdexcept>
#include <string>
#include "metronome.h"
#include "sample.h"

typedef uint32_t jack_nframes_t;

typedef void (shutdown_fn)(void *);

// TODO: Should really be a singleton...
class audio_engine
{
public:
	audio_engine();
	void set_name(const std::string &name_) { name = name_; }
	void set_metronome(metronome *m_) { m = m_; }
	metronome *get_metronome() { return m; }

	void initialize();
	void halt();
	void shutdown();

	// function to call on audioengine shutdown
	void set_shutdown(shutdown_fn *f);

	void connect(const std::string &, const std::string &);

	// used by looper to determine required buffering length etc.
	uint32_t get_sample_rate() const;

	struct error : public std::runtime_error
	{
		error(const std::string &s) : std::runtime_error(s) {}
	};

	struct init_failure : public error
	{
		init_failure(const std::string &n)
			: error("Failed to initialize jack audio library ["
				+ n + "].") {}
	};

	struct missing_playback : public error
	{
		missing_playback()
			: error("Failed to find any suitable playback ports.")
			{}
	};

	struct missing_output : public error
	{
		missing_output(const std::string &s) : error(s) {}
	};

	class dport
	{
	public:
		dport(int id_, unsigned int channels_ = 0)
			: id(id_), channels(channels_)
			{}
		virtual ~dport();
		std::string get_name() const { return name; }
		int get_id() const { return id; }
		unsigned int get_channels() const;

		virtual bool is_playing() const { return false; }
		virtual bool is_recording() const { return false; }
		virtual bool is_scheduled() const { return false; }
		virtual bool cancel_scheduled() { return false; }

		virtual void start(const bbt &when, sample *) = 0;
		virtual void record(const bbt &when, sample *) = 0;
		virtual void stop(const bbt &when) = 0;
		virtual void stop_now() = 0;

		virtual void input(unsigned int, const std::string &) = 0;

	private:
		int id;
		unsigned int channels;
		std::string name;
	};

	dport *add_input(int, unsigned int);
	dport *add_output(int, unsigned int);

	static void purge_input(dport *);
	static void purge_output(dport *);

private:
	static int process(jack_nframes_t, void *);
	std::string name;

	shutdown_fn *sfn;
	metronome *m;
};

#endif
