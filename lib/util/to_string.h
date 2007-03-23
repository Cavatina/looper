#ifndef _UTIL_TO_STRING_H_
#define _UTIL_TO_STRING_H_

#include <string>
#include <iostream>
#include <sstream>

template<class type> inline std::string to_string(const type &value)
{
	std::ostringstream out;
	out << value;
	return out.str();
}

#endif
