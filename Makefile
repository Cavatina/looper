targets := looper

MODULES := lib util
CFLAGS := -O2 -ggdb -Wall -pedantic $(EXTRACFLAGS)
CFLAGS += $(patsubst %, -I%, $(MODULES))

ALL_OBJ :=
LIBS :=
SRC := main.c

-include $(patsubst %, %/module.mk, $(MODULES))
#-include $(shell find . -mindepth 1 -maxdepth 3 -name module.mk)

OBJ := $(patsubst %.c,   %.o, $(filter %.c,$(SRC))) \
	$(patsubst %.cpp,%.o, $(filter %.cpp,$(SRC)))

ALL_OBJ += $(OBJ)

DEP := $(patsubst %.o,%.d,$(ALL_OBJ))

CPPFLAGS = $(CFLAGS)
LDFLAGS:=`pkg-config --cflags --libs jack` \
	 -lsndfile -lSoundTouch -lasound -ggdb $(LIBS)

CC = gcc
CXX = g++
LINK.o = $(CXX)
SHELL = /bin/sh
INSTALL = install
INSTALL_PROGRAM = $(INSTALL) #-s
INSTALL_DATA = $(INSTALL)

.PHONY:	all clean
all:	$(targets)

looper: $(OBJ)
	@$(LINK.o) $(LDFLAGS) $(OBJ) -o $@

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	@$(CXX) $(CFLAGS) -c $< -o $@

-include $(DEP)

%.d: %.cpp
	@set -e; rm -f $@; \
	 $(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	 sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$*

clean:
	-@for file in $(ALL_OBJ) $(DEP) $(targets); do \
	   if [ -f $$file ]; then rm $$file; fi; done

mrproper: clean
	-@for dir in . $(MODULES); do \
	  rm -f $$dir/*~ $$dir/*.d.*; done

todolist:
	-@for file in $(ALLFILES); do grep -H TODO $$file; done; true

