include ../../../config.mak

ifeq ($(targetos),Android)
include config.$(arch).mak
LDFLAGS += -L../../framework -lmlt -lm -landroid
else
LDFLAGS += -L../../framework -lmlt -lpthread -lm
endif

CFLAGS += -I../..


LDFLAGS += $(EXTRA_LIBS)

TARGET = ../libmlt-android$(LIBSUF)

OBJS = factory.o
OBJS += android_preview_consumer.o 


SRCS := $(OBJS:.o=.c)

all: 	$(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(SHFLAGS) -o $@ $(OBJS) $(LDFLAGS)

depend:	$(SRCS)
	$(CC) -MM $(CFLAGS) $^ 1>.depend

distclean:	clean
	rm -f .depend

clean:	
	rm -f $(OBJS) ../libmlt-android$(LIBSUF)

install: all
	install -m 755 $(TARGET) "$(DESTDIR)$(moduledir)"
	install -d "$(DESTDIR)$(mltdatadir)/android"

uninstall:
	rm "$(DESTDIR)$(moduledir)/libmlt-android$(LIBSUF)" 2> /dev/null || true
	rm -rf "$(DESTDIR)$(mltdatadir)/android"

ifneq ($(wildcard .depend),)
include .depend
endif

