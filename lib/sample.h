#ifndef _LOOPER_SAMPLE_H_
#define _LOOPER_SAMPLE_H_

#include <string>

class sample
{
public:
	sample(const std::string &, int = 0, int = 0, int = 0);

	void set_source(const std::string &);
	// offset to where playing should start.
	// only here for housekeeping, it is the outer classes (bank)
	// that need to consider it.
	void set_offset(int);
	void set_fadein(int);
	void set_fadeout(int);

	int get_offset() const;
	int get_fadein() const;
	int get_fadeout() const;
	std::string get_source() const;

private:
	std::string source;
	int offset;
	int fadein;
	int fadeout;
};

#endif
