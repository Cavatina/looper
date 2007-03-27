#ifndef _LOOPER_COMMAND_H_
#define _LOOPER_COMMAND_H_

#include <string>
#include <stdexcept>

class bank;
class metronome;
class looper;
class command;

command *command_parse(looper *, const std::string &);

class command
{
public:
	command(const std::string &cmd_) : cmd(cmd_) {}
	virtual ~command() {}

	virtual void execute() = 0;

	struct error : public std::runtime_error
	{
		error(const std::string &c) :
			std::runtime_error("[" + c + "]: Command error."),
			cmd(c)
			{}
		error(const std::string &c, const std::string &e) :
			std::runtime_error("[" + c + "]: " + e), cmd(c)
			{}
		virtual ~error() throw() {}
		const std::string &get_command() const { return cmd; }
	private:
		std::string cmd;
	};

	struct illegal_command : public error
	{
		illegal_command(const std::string &c) :
			error(c,  "Illegal command.") {}
	};

	struct invalid_object : public error
	{
		invalid_object(const std::string &c) :
			error(c,  "Nonexisting object.") {}
	};

	struct invalid_index : public error
	{
		invalid_index(const std::string &c) :
			error(c,  "Object index out of bounds.") {}
	};

	struct invalid_method : public error
	{
		invalid_method(const std::string &c) :
			error(c, "Invalid method.") {}
	};

protected:
	std::string cmd;
};


typedef void (bank::*bank_member_fn)();

class bank_command : public command
{
public:
	bank_command(looper *, const std::string &,
		     size_t, const std::string &);

	void execute();

private:
	size_t index;
	bank *obj;
	bank_member_fn fn;
	std::string method;
};

typedef void (looper::*looper_member_fn)();

class looper_command : public command
{
public:
	looper_command(looper *, const std::string &,
			  const std::string &);

	void execute();

private:
	looper *obj;
	looper_member_fn fn;
	std::string method;
};


#endif
