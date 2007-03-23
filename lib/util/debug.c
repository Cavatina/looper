#include "debug.h"

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

/*
	_assert()
	Called when an assertion fails, in debug mode.
*/

void _assert(const char *expr, const char *file, unsigned int line)
{
	fflush(NULL);
	fprintf(stderr, "File %s, line %u: Assertion (%s) failed\n",
		file, line, expr);
	fflush(stderr);
	exit(-1);
}
