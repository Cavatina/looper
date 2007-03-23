#ifndef _LOOPER_PRESET_H_
#define _LOOPER_PRESET_H_

#include <string>
#include <stdexcept>

#include "persistent_storage.h"
#include "looper.h"

class preset : public persistent_storage
{
public:
	preset(const std::string &fname, looper *obj_)
		: persistent_storage(fname),
		  obj(obj_),
		  has_backup(false)
		{}

	// Make backup of last configuration on startup
	// (and when selected in GUI? => v2),
	// then always save on change. v2: option to restore backup...

	void save();
	void read();
	void make_backup();

	struct file_error : std::runtime_error
	{
		file_error(const preset *p)
			: std::runtime_error(p->get_source()
					     + ": failed to open preset.")
			{}
	};

	struct format_error : std::runtime_error
	{
		format_error(const preset *p)
			: std::runtime_error(p->get_source()
					     + ": Invalid preset format.")
			{}
	};

	looper *get_looper() { return obj; }

private:
	looper *obj;
	bool has_backup;
};

#endif
