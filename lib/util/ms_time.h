#ifndef _MS_TIME_H_
#define _MS_TIME_H_

typedef long millisec;	// Time, measured in milliseconds

#include <string>
#include <time.h>

#ifdef WIN32
#include <sys/timeb.h>
#include <winsock2.h>
#else
#include <sys/time.h>
#endif

namespace ms_time
{
	class datetime
	{
	public:
		datetime();
		datetime(const datetime &p);
		~datetime() {}

		static datetime now();

		void set(millisec offset_from_now = 0);

		datetime &operator=(const datetime &p);

		datetime &operator-=(millisec p);
		datetime &operator+=(millisec p);

		datetime &operator-=(const datetime &p);
		datetime &operator+=(const datetime &p);

		operator millisec() const;

		bool equal(const datetime &rhs) const;
		bool less(const datetime &rhs) const;

		size_t strftime(char *s, size_t max, const char *format) const;
		std::string strftime(const char *format, size_t max=64) const;
		std::string strftime(const std::string &, size_t max=64) const;
	private:
		struct timeval tv;
#ifdef WIN32
		static int gettimeofday(struct timeval *tv,
					struct timezone *tz);
#endif
	};

	inline bool operator==(const datetime &lhs, const datetime &rhs)
	{
		return lhs.equal(rhs);
	}

	inline bool operator!=(const datetime &lhs, const datetime &rhs)
	{
		return !(lhs == rhs);
	}

	inline bool operator<(const datetime &lhs, const datetime &rhs)
	{
		return lhs.less(rhs);
	}

	inline bool operator>(const datetime &lhs, const datetime &rhs)
	{
		return rhs < lhs;
	}

	inline bool operator<=(const datetime &lhs, const datetime &rhs)
	{
		return !(lhs > rhs);
	}

	inline bool operator>=(const datetime &lhs, const datetime &rhs)
	{
		return rhs <= lhs;
	}

	inline datetime operator+(const datetime &t1, const datetime &t2)
	{
		datetime out = t1;
		out += t2;
		return out;
	}

	inline datetime operator-(const datetime &t1, const datetime &t2)
	{
		datetime out = t1;
		out -= t2;
		return out;
	}
}
#endif
