#ifndef _LOOPER_PRESET_H_
#define _LOOPER_PRESET_H_

#include <string>

#include "persistent_storage.h"
#include "looper.h"

class preset : public persistent_storage
{
public:
	preset(const std::string &fname, looper *obj_)
		: persistent_storage(fname), obj(obj_) {}

	// Make backup of last configuration on startup
	// (and when selected in GUI? => v2),
	// then always save on change. v2: option to restore backup...

	void save();
	void read();

private:
	looper *obj;
};

#endif
