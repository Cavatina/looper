#ifndef _LOOPER_PERSISTENT_STORAGE_H_
#define _LOOPER_PERSISTENT_STORAGE_H_

#include <string>

class persistent_storage
{
public:
	persistent_storage(const std::string &source_) :
		source(source_), dirty(false) {}
	virtual ~persistent_storage() {}

	const std::string &get_source() const { return source; }
	bool is_dirty() const { return dirty; }
	void mark_dirty() { dirty = true; }

	virtual void read() = 0;
	virtual void save() = 0;

private:
	std::string source;
	bool dirty;
};

#endif
