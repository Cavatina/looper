#include "fs.h"
#include <iostream>
#include <fstream>

#include <fcntl.h>

#ifdef WIN32
#include <direct.h>
#include <io.h>
#ifdef _MSC_VER
typedef int mode_t;
#endif
inline int mkdir(const char *pathname, mode_t) { return _mkdir(pathname); }
#else
#include <sys/types.h>
#include <sys/stat.h>
#include<unistd.h>
#endif

using namespace fs;
using namespace std;

void fs::mkpath(const std::string &name)
{
	string::size_type s;
	if((s = name.find_last_of("/")) != string::npos)
		mktree(name.substr(0, s));
	else if((s = name.find_last_of("\\")) != string::npos)
		mktree(name.substr(0, s));
}

void fs::mktree(const std::string &name)
{
	if(!name.size()) return;
	int fd = open(name.c_str(), O_RDONLY);
	if(fd != -1){
		close(fd);
		return;
	}

	std::string::const_iterator s = name.begin();
	for(;;++s){
		if(*s == '/' || *s == '\\' || s == name.end()){
			string dir(name.begin(), s);
			if(dir.size()) mkdir(dir.c_str(), 0777);
			if(s == name.end()) break;
		}
	}
}

bool fs::openlog(std::ofstream &log, const std::string &name)
{
	if(!name.size()) return false;
	log.open(name.c_str(), std::ios::out | std::ios::app);
	if(!log.is_open()){
		fs::mkpath(name);
		log.open(name.c_str(), std::ios::out | std::ios::app);
	}

	return log.is_open();
}

std::string fs::get_directory_part(const std::string &path)
{
	string::size_type s;
	if((s = path.find_last_of("/")) != string::npos)
		return path.substr(0, s);
	else if((s = path.find_last_of("\\")) != string::npos)
		return path.substr(0, s);
	else
		return path;
}

std::string fs::get_filename_part(const std::string &path)
{
	string::size_type s;
	if((s = path.find_last_of("/")) != string::npos)
		return path.substr(s+1);
	else if((s = path.find_last_of("\\")) != string::npos)
		return path.substr(s+1);
	else
		return path;
}

std::string fs::make_absolute_path(const std::string &path)
{
	std::string out;
	if(path.size() && path[0] == '/') return path;

	// TODO: Support DOS paths...

	char buf[4096];
	if(getcwd(buf, sizeof(buf))){
		return std::string(buf) + "/" + path;
	}
	else return path;
}
