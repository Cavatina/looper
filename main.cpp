#include <stdlib.h>
#include <stdexcept>
#include <iostream>

#include "looper.h"
#include "preset.h"

looper app;

void sigusr1()
{
	app.read_storage();
}

void sigkill()
{
	app.shutdown();
}

int main(int argc, char *argv[])
{
	const char *fname = 0;
	if(argc > 1) fname = argv[1];
	else {
		std::cerr << "Missing argument <inputfile>" << std::endl;
		exit(1);
	}

	// Register signals etc.


	try {
		app.set_audio_engine(new audio_engine());
		app.set_persistent_storage(new preset(fname, &app));
		app.initialize();
		app.run();
	}
	catch(std::exception &e) {
		std::cerr << "Fatal exception: " << e.what() << std::endl;
	}
	exit(0);
}
