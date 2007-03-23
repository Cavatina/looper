#ifndef _STRING_SPLIT_H_
#define _STRING_SPLIT_H_

#include <deque>
#include <string>

inline std::deque<int> split_int(const std::string &s)
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

inline std::deque<std::string> split_colon(const std::string &p)
{
	std::deque<std::string> n;
	std::string::const_iterator s = p.begin();
	std::string::const_iterator e = p.begin();
	while(s != p.end()){
		while(e != p.end() && *e != ':') e++;
		if(e == p.end()){
			if(s != e) n.push_back(std::string(s, e));
			break;
		}
		n.push_back(std::string(s, e));
		s = ++e;
	}
	return n;
}

#endif
