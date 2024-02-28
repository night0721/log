CC=gcc

VERSION = 1.0
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

CFLAGS = -std=gnu11 -O0 -Wall

SRC = noted.c config.h
OBJ = ${SRC:.c=.o}

.c.o:
	${CC} -c ${CFLAGS} $<

noted: ${OBJ}
	${CC} -o $@ ${OBJ}
	strip noted

clean:
	rm -rf noted

version:
	sed -i 's/#define VERSION "[^"]*"/#define VERSION "${VERSION}"/' config.h

dist: version noted
	mkdir -p noted-${VERSION}
	cp -R LICENSE README.md noted.1 noted noted-${VERSION}
	tar -cf noted-${VERSION}.tar noted-${VERSION}
	gzip noted-${VERSION}.tar
	rm -rf noted-${VERSION}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f noted ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/noted
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < noted.1 > ${DESTDIR}${MANPREFIX}/man1/noted.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/noted.1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/noted\
		${DESTDIR}${MANPREFIX}/man1/noted.1
all: noted

.PHONY: all clean dist install uninstall noted
