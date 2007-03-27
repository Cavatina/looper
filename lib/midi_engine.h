#ifndef _LOOPER_MIDI_ENGINE_H_
#define _LOOPER_MIDI_ENGINE_H_

#include <stdexcept>
#include <string>
#include <list>
#include "command.h"

typedef struct _snd_seq snd_seq_t;

class midi_handler
{
public:
	midi_handler(command *cmd_)
		: cmd(cmd_), controller(-1), note(-1), param(-1)
		{}
	~midi_handler() { delete cmd; }

	void set_controller(char c) { controller = c; }
	void set_note(char n) { note = n; }
	void set_param(char p) { param = p; }
	char get_controller() const { return controller; }
	char get_note() const { return note; }
	char get_param() const { return param; }
	bool filter_controller() const { return controller >= 0; }
	bool filter_note() const { return note >= 0; }
	bool filter_param() const { return param >= 0; }

	void execute() { cmd->execute(); }
private:
	command *cmd;
	char controller;
	char note;
	char param;
};

class midi_engine
{
public:
	midi_engine() : channel(-1) {}
	void set_name(const std::string &name_) { name = name_; }
	void initialize();
	void shutdown();
	void dispatch();
	bool has_event() const;
	void input_connect(const std::string &);
	void set_channel(char ch) { channel = ch; }
	void add(midi_handler *);
	void clear_handlers();
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
	std::list<midi_handler *> handlers;
	char channel;
};

#endif
