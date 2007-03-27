#include "command.h"

#include <deque>
#include <iostream>

#include "looper.h"
#include "bank.h"
#include "util/string_split.h"
#include "util/debug.h"

template <class T> class lookup_method
{
public:
	typedef void (T::*member_fn)();
	lookup_method(const std::string &method_, member_fn fn_)
		: method(method_), fn(fn_) {}

	std::string method;
	member_fn fn;
};

typedef lookup_method<bank> bm;
typedef lookup_method<looper> mm;

static bm bank_methods[] =
{
	bm("loop", &bank::loop),
	bm("play", &bank::play),
	bm("stop", &bank::stop),
	bm("record", &bank::record),
	bm("play-once", &bank::play_once),
	bm("cycle-samples", &bank::cycle_samples),
	bm("discard", &bank::discard),
};

static mm looper_methods[] =
{
	mm("start", &looper::start),
	mm("stop", &looper::stop),
	mm("toggle", &looper::toggle)
};


#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))
#define CALL_MEMBER_FN(object,ptrToMember)  ((object)->*(ptrToMember))

command *command_parse(looper *l, const std::string &cmd)
{
	std::deque<std::string> s = split_colon(cmd);
	if(s.size() < 1) throw command::illegal_command(cmd);

	if(s[0] == "bank"){
		if(s.size() != 3) throw command::illegal_command(cmd);
		return new bank_command(l, cmd, atoi(s[1].c_str()), s[2]);
	}
	else if(s[0] == "metronome" || s[0] == "looper"){
		if(s.size() != 2) throw command::illegal_command(cmd);
		return new looper_command(l, cmd, s[1]);
	}
	else throw command::invalid_object(cmd);
}

static bank *get_bank(looper *l, const std::string &cmd, size_t index)
{
	if(index < 1 || index > l->get_banks())
		throw command::invalid_index(cmd);
	return l->get_bank(index);
}

static bank_member_fn get_bank_method(const std::string &cmd,
				      const std::string &met)
{
	for(size_t i=0; i<ARRAYSIZE(bank_methods); ++i){
		if(met == bank_methods[i].method){
			return bank_methods[i].fn;
		}
	}
	throw command::invalid_method(cmd);
}

bank_command::bank_command(looper *l,
			   const std::string &cmd,
			   size_t index_,
			   const std::string &met)
	: command(cmd), index(index_), obj(get_bank(l, cmd, index_)),
	  fn(get_bank_method(cmd, met)), method(met)
{}

void bank_command::execute()
{
	DBG2(std::cerr<<"command: execute ["<<cmd<<"]"<<std::endl);
	CALL_MEMBER_FN(obj, fn)();
}

static looper_member_fn get_looper_method(const std::string &cmd,
					  const std::string &met)
{
	for(size_t i=0; i<ARRAYSIZE(looper_methods); ++i){
		if(met == looper_methods[i].method){
			return looper_methods[i].fn;
		}
	}
	throw command::invalid_method(cmd);
}

looper_command::looper_command(looper *l,
			       const std::string &cmd,
			       const std::string &met)
	: command(cmd), obj(l),
	  fn(get_looper_method(cmd, met)), method(met)
{}

void looper_command::execute()
{
	DBG2(std::cerr<<"command: execute ["<<cmd<<"]"<<std::endl);
	CALL_MEMBER_FN(obj, fn)();
}
