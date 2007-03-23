#include "command.h"

#include <deque>
#include <iostream>

#include "looper.h"
#include "bank.h"
#include "util/string_split.h"
#include "util/debug.h"

struct bm
{
	bm(const std::string &method_, bank_member_fn fn_)
		: method(method_), fn(fn_) {}
	std::string method;
	bank_member_fn fn;
	// int args; // TODO: support : arguments
};


static struct bm bank_methods[] =
{
	bm("play", &bank::play),
	bm("stop", &bank::stop),
	bm("record", &bank::record),
	bm("stop-record-start-play", &bank::stop_record_start_play),
	bm("stop-record-discard", &bank::stop_record_discard),
	bm("play-increment-count", &bank::play_increment_count),
	bm("cycle-samples", &bank::cycle_samples),
};

#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))
#define CALL_MEMBER_FN(object,ptrToMember)  ((object)->*(ptrToMember))

command *command_parse(looper *l, const std::string &cmd)
{
	std::deque<std::string> s = split_colon(cmd);
	if(s.size() != 3) throw command::illegal_command(cmd);

	if(s[0] == "bank")
		return new bank_command(l, cmd, atoi(s[1].c_str()), s[2]);
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
	DBG2(std::cerr<<"execute bank:"<<index<<":"<<method<<std::endl);
	CALL_MEMBER_FN(obj, fn)();
}
