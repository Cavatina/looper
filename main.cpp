#include <stdexcept>
#include <iostream>

#include <stdlib.h>
#include <sys/signal.h>

#include "looper.h"
#include "preset.h"

looper app;

void signal_init();

int main(int argc, char *argv[])
{
	const char *fname = 0;
	if(argc > 1) fname = argv[1];
	else {
		std::cerr <<
			"Missing argument <inputfile>." << std::endl <<
			"To create a new preset, simply enter a "
			"nonexisting .looper file as argument."
			  << std::endl;
		exit(1);
	}

	// Register signals etc.
	signal_init();

	try {
		app.set_midi_engine(new midi_engine());
		app.set_audio_engine(new audio_engine());
		app.set_persistent_storage(new preset(fname, &app));
		app.initialize();
		app.run();
	}
	catch(std::exception &e) {
		std::cerr << "Fatal exception: " << e.what() << std::endl;
		app.shutdown();
	}
	exit(0);
}

// -- Utility functions...

void signal_handler(int signum)
{
	if(signum == SIGPIPE) { // Ignore
		std::cerr << "Ignored SIGPIPE." << std::endl;
	}
	else if(signum == SIGTTIN) { // Ignore
		std::cerr << "Ignored SIGTTIN." << std::endl;
	}
	else if(signum == SIGINT){
		app.shutdown();
		exit(1);
	}
	else if(signum == SIGUSR1){
		app.read_storage();
	}
	else {
		std::cerr << "Unhandled signal " << signum << ", exiting." << std::endl;
		try { app.shutdown(); }
		catch(...) {}
		exit(1);
	}
}

void signal_init()
{
        struct sigaction sa;
	sa.sa_handler = signal_handler;
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);               /* Capture CTRL+C            */
	sigaction(SIGTERM, &sa, NULL);              /* Capture 'kill <our_pid>'  */
	sigaction(SIGHUP,  &sa, NULL);              /* Capture "Hangup"          */
	sigaction(SIGILL,  &sa, NULL);
	sigaction(SIGFPE,  &sa, NULL);
	sigaction(SIGPIPE,  &sa, NULL);
	sigaction(SIGTTIN,  &sa, NULL);
	sigaction(SIGTTOU,  &sa, NULL);
}
