#ifndef _LOOPER_FS_H_
#define _LOOPER_FS_H_

#include <string>
#include <iostream>

namespace fs
{
	// Create directories up to last path component ([dir]/fname)
	void mkpath(const std::string &);
	// Create each part of given directory string
	void mktree(const std::string &);

	// Open logfile for append, creating directories as needed.
	bool openlog(std::ofstream &, const std::string &);

	std::string get_directory_part(const std::string &);
	std::string get_filename_part(const std::string &);
	std::string make_absolute_path(const std::string &);
}

#endif
