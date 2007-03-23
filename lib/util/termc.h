#ifndef _TERMC_H_
#define _TERMC_H_

#include <stdlib.h>
#include <stdio.h>

#include <termios.h>
#include <string.h>
#include <unistd.h>

inline void set_keypress(bool set_flag)
{
	static struct termios stored_settings;
	if(set_flag == true){
		struct termios new_settings;
     
		tcgetattr(0,&stored_settings);
     
		new_settings = stored_settings;
     
		/* Disable canonical mode, and set buffer size to 1 byte */
		new_settings.c_lflag &= (~ICANON);
		new_settings.c_cc[VTIME] = 0;
		new_settings.c_cc[VMIN] = 1;
		
		tcsetattr(0,TCSANOW,&new_settings);
	}
	else {
		tcsetattr(0,TCSANOW,&stored_settings);
	}
}

inline void init_terminal() { set_keypress(true); }
inline void reset_terminal() { set_keypress(false); }

inline int _kbhit()
{
	fd_set fds;
	struct timeval tv;
	int retval;

	FD_ZERO(&fds);
	FD_SET(0, &fds);

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	retval = select(1, &fds, NULL, NULL, &tv);

	return (retval > 0) ? 1 : 0;
}

inline int msleep(unsigned int msec){ return usleep(msec*1000); }

#endif
