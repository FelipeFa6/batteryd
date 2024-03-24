CFLAGS = -Wall -Wextra -pedantic -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -lXft -I/usr/X11R6/include/freetype2 -pthread

PREFIX ?= /usr/local
CC ?= cc

all: batteryd

config.h: config.def.h
	cp config.def.h config.h

batteryd: batteryd.c config.h
	$(CC) batteryd.c $(CFLAGS) -o batteryd

install: batteryd
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f batteryd ${DESTDIR}${PREFIX}/bin

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/batteryd

clean:
	rm -f batteryd
	rm -f config.h

.PHONY: all install uninstall clean

