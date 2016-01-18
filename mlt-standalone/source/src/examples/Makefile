CXXFLAGS += -Wall -g $(shell pkg-config mlt++ --cflags)
CFLAGS += -Wall -g $(shell pkg-config mlt++ --cflags)
LDFLAGS += $(shell pkg-config mlt++ --libs-only-L)
LDLIBS += $(shell pkg-config mlt++ --libs-only-l)
CC=c++

all:		play play_filter simple

play:		play.o

play.o:		play.cpp

play_filter:		play_filter.o

play_filter.o:		play_filter.cpp

simple:simple.o
simple.o:simple.c


clean:
	$(RM) play play.o play_filter.o play_filter

distclean: clean

install:

uninstall:
