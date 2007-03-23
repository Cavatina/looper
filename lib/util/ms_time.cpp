#include "util/ms_time.h"

using namespace ms_time;

#ifdef WIN32

int datetime::gettimeofday(struct timeval *tv, struct timezone *tz)
{
	struct _timeb t;
	_ftime(&t);
	
	tv->tv_sec = t.time;
	tv->tv_usec = t.millitm * 1000;
	return 0;
}

#endif

datetime::datetime()
{
	tv.tv_sec = tv.tv_usec = 0;
}

datetime::datetime(const datetime &p)
{
	tv.tv_sec = p.tv.tv_sec;
	tv.tv_usec = p.tv.tv_usec;
}

datetime &datetime::operator=(const datetime &p)
{
	tv.tv_sec = p.tv.tv_sec;
	tv.tv_usec = p.tv.tv_usec;
	return *this;
}

datetime datetime::now()
{
	datetime t;
	gettimeofday(&t.tv, NULL);
	return t;
}

void datetime::set(millisec offset_from_now)
{
	gettimeofday(&tv, NULL);
	if(offset_from_now) *this += offset_from_now;
}

datetime &datetime::operator-=(millisec p)
{
	int sec = p / 1000;
	tv.tv_sec -= sec;
	tv.tv_usec -= (p - (sec * 1000)) * 1000;
	while(tv.tv_usec < 0){ tv.tv_sec--; tv.tv_usec += 1000000; }
	return *this;
}

datetime &datetime::operator+=(millisec p)
{
	int sec = p / 1000;
	tv.tv_sec += sec;
	tv.tv_usec += (p - (sec * 1000)) * 1000;
	while(tv.tv_usec >= 1000000){ tv.tv_sec++; tv.tv_usec -= 1000000; }
	return *this;
}

datetime &datetime::operator-=(const datetime &p)
{
	tv.tv_sec -= p.tv.tv_sec;
	tv.tv_usec -= p.tv.tv_usec;
	while(tv.tv_usec < 0){ tv.tv_sec--; tv.tv_usec += 1000000; }
	return *this;
}

datetime &datetime::operator+=(const datetime &p)
{
	tv.tv_sec += p.tv.tv_sec;
	tv.tv_usec += p.tv.tv_usec;
	while(tv.tv_usec >= 1000000){ tv.tv_sec++; tv.tv_usec -= 1000000; }
	return *this;
}

datetime::operator millisec() const
{
	return (millisec)((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}


bool datetime::equal(const datetime &rhs) const
{
	if(tv.tv_sec == rhs.tv.tv_sec && tv.tv_usec == rhs.tv.tv_usec) return true; else return false;
}

bool datetime::less(const datetime &rhs) const
{
	if((tv.tv_sec < rhs.tv.tv_sec)
	   || (tv.tv_sec == rhs.tv.tv_sec && tv.tv_usec < rhs.tv.tv_usec)) return true;
	else {
		return false;
	}
}

size_t datetime::strftime(char *s, size_t max, const char *format) const
{
	time_t t = tv.tv_sec;
	struct tm *lt = localtime(&t);
	return ::strftime(s, max, format, lt);
}

std::string datetime::strftime(const std::string &fmt, size_t max) const
{
	return strftime(fmt.c_str(), max);
}

std::string datetime::strftime(const char *format, size_t max) const
{
	if(!tv.tv_sec) return "";
	char buf[64];
	char *p = buf;
	if(max > sizeof(buf)) p = new char[max];
	time_t t = tv.tv_sec;
	struct tm *lt = localtime(&t);
	if(::strftime(p, max, format, lt) < 1) *p = 0;
	if(p == buf) return std::string(p);
	std::string out(p);
	delete [] p;
	return out;
}
