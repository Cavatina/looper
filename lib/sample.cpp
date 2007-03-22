#include "sample.h"

void sample::set_source(const std::string &)
{

}

void sample::set_offset(ssize_t)
{

}

void sample::prebuffer_from(int32_t frame)
{
}

void sample::discard_buffer()
{
}

size_t sample::get_channel_data(int32_t from_frame,
				short unsigned int channel,
				void *buf,
				size_t frames)
{
	return 0;
}
