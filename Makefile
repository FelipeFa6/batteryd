# See LICENSE file for copyright and license details.

CFLAGS = -Wall -Wextra -pedantic -pthread

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

