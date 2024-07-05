.POSIX:
.SUFFIXES:

CC = cc
VERSION = 1.0
TARGET = ccc
MANPAGE = $(TARGET).1
CONF = config.h
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man/man1

# Flags
CFLAGS = -O3 -march=native -mtune=native -pipe -s -flto -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600

SRC = $(TARGET).c

$(TARGET): $(SRC) $(CONF)
	$(CC) $(SRC) -o $@ $(CFLAGS)

dist: install
	mkdir -p $(TARGET)-$(VERSION)
	cp -R README.md $(MANPAGE) $(TARGET) $(TARGET)-$(VERSION)
	tar -cf $(TARGET)-$(VERSION).tar $(TARGET)-$(VERSION)
	gzip $(TARGET)-$(VERSION).tar
	rm -rf $(TARGET)-$(VERSION)

install: $(TARGET)
	mkdir -p $(DESTDIR)$(BINDIR)
	mkdir -p $(DESTDIR)$(MANDIR)
	cp -p $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	chmod 755 $(DESTDIR)$(BINDIR)/$(TARGET)
	sed "s/VERSION/${VERSION}/g" < $(MANPAGE) > $(DESTDIR)/$(MANDIR)/$(MANPAGE)
	chmod 644 $(DESTDIR)$(MANDIR)/$(MANPAGE)

uninstall:
	$(RM) $(DESTDIR)$(BINDIR)/$(TARGET)
	$(RM) $(DESTDIR)$(MANDIR)/$(MANPAGE)

clean:
	$(RM) $(TARGET)

all: $(TARGET)

.PHONY: all dist install uninstall clean
