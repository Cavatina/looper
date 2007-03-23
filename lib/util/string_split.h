#ifndef _STRING_SPLIT_H_
#define _STRING_SPLIT_H_

#include <deque>
#include <string>

inline std::deque<int> split(const std::string &s)
{
	std::deque<int> n;
	const char *p = s.c_str();
	while(*p != 0){
		while(*p != 0 && !(*p >= '0' && *p <= '9'))
			if(*p++ == '-' && *p >= '0' && *p <= '9'){
				--p;
				break;
			}
		if(*p == 0) break;
		n.push_back(std::atoi(p));
		if(*p == '-') p++;
		while(*p >= '0' && *p <= '9') p++;
	}
	return n;
}

#endif
