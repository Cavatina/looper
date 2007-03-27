#include "sample.h"

sample::sample(const std::string &source_,
	       int offset_, int fadein_, int fadeout_)
	: source(source_), offset(offset_), fadein(fadein_), fadeout(fadeout_)
{
}

void sample::set_source(const std::string &source_)
{
	source = source_;
}

std::string sample::get_source() const
{
	return source;
}

void sample::set_offset(int offset_)
{
	offset = offset_;
}

int sample::get_offset() const
{
	return offset;
}

void sample::set_fadein(int fadein_)
{
	fadein = fadein_;
}

int sample::get_fadein() const
{
	return fadein;
}

void sample::set_fadeout(int fadeout_)
{
	fadeout = fadeout_;
}

int sample::get_fadeout() const
{
	return fadeout;
}
