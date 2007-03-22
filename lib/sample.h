#ifndef _LOOPER_SAMPLE_H_
#define _LOOPER_SAMPLE_H_

#include <string>

class sample
{
public:
	void set_source(const std::string &);
	// offset to where playing should start.
	// only here for housekeeping, it is the outer classes (bank)
	// that need to consider it.
	void set_offset(int32_t);

	// todo: per-sample fade-in and -out values?

	int32_t get_offset() const;
	const std::string & get_source() const;

	// pre-buffer xxxx bytes => or own class?
	void prebuffer_from(int32_t frame);
	void discard_buffer();

	size_t get_channel_data(int32_t from_frame,
				short unsigned int channel,
				void *buf,
				size_t frames);

private:
	std::string source;
};

#endif
