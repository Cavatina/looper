targets = looper

EXTERNAL_LIBS := jack libxml-2.0 sndfile soundtouch alsa
MODULES := lib util lib/util
CFLAGS := -O2 -ggdb -Wall -Wno-deprecated -pedantic \
	  $(shell pkg-config --libs $(EXTERNAL_LIBS)) $(EXTRACFLAGS)
CFLAGS += $(patsubst %, -I%, $(MODULES))

ALL_OBJ :=
LIBS :=
SRC := main.c

.PHONY:	all clean
all:	$(targets)

-include $(patsubst %, %/module.mk, $(MODULES))
#-include $(shell find . -mindepth 1 -maxdepth 3 -name module.mk)

OBJ := $(patsubst %.c,   %.o, $(filter %.c,$(SRC))) \
	$(patsubst %.cpp,%.o, $(filter %.cpp,$(SRC)))

ALL_OBJ += $(OBJ)

DEP := $(patsubst %.o,%.P,$(ALL_OBJ))
ALLFILES := $(SRC)

CPPFLAGS = $(CFLAGS)
LDFLAGS:= -lpthread $(shell pkg-config --libs $(EXTERNAL_LIBS)) -ggdb $(LIBS)

CC = gcc
CXX = g++
LINK.o = $(CXX)
SHELL = /bin/sh
INSTALL = install
INSTALL_PROGRAM = $(INSTALL) #-s
INSTALL_DATA = $(INSTALL)

looper: $(OBJ)
	$(LINK.o) $(OBJ) $(LDFLAGS) -o $@

%.o: %.c
	$(CC) -MMD $(CFLAGS) -c $< -o $@
	@cp $*.d $*.P; \
	 sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	     -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	 rm -f $*.d

%.o: %.cpp
	$(CXX) -MMD $(CFLAGS) -c $< -o $@
	@cp $*.d $*.P; \
	 sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	     -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	 rm -f $*.d

-include $(DEP)

clean:
	-@for file in $(ALL_OBJ) $(DEP) $(targets); do \
	   if [ -f $$file ]; then rm $$file; fi; done

mrproper: clean
	-@for dir in . $(MODULES); do \
	  rm -f $$dir/*~ $$dir/*.d.*; done

todolist:
	-@for file in $(ALLFILES); do grep -H TODO $$file; done; true

