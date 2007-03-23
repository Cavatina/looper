SRC += lib/looper.cpp lib/sample.cpp lib/bank.cpp lib/channel.cpp
SRC += lib/preset.cpp lib/audio_engine.cpp lib/midi_engine.cpp
SRC += lib/metronome.cpp

LIBS += -lxml2

lib/preset.o: CFLAGS += `pkg-config --cflags libxml-2.0`
