SRC += lib/looper.cpp lib/sample.cpp lib/bank.cpp
SRC += lib/preset.cpp lib/audio_engine.cpp lib/midi_engine.cpp
SRC += lib/disk_engine_jack.cpp
SRC += lib/metronome.cpp
SRC += lib/command.cpp
SRC += lib/util/fs.cpp
SRC += lib/util/ms_time.cpp
SRC += lib/util/debug.c

LIBS += -lxml2

lib/preset.o: CFLAGS += `pkg-config --cflags libxml-2.0`
