#!/usr/bin/make -f

PACKAGE := recovery-web

prefix ?= /usr/local
exec_prefix ?= $(prefix)
datarootdir ?= $(prefix)/share
datadir ?= $(datarootdir)
sysconfdir ?= $(prefix)/etc
pkgdatadir ?= $(datadir)/$(PACKAGE)

SRC_ROOT := htdocs
DST_ROOT := $(pkgdatadir)
SRC_DIRS := $(SRC_ROOT) $(SRC_ROOT)/include $(SRC_ROOT)/images
DST_DIRS := $(patsubst $(SRC_ROOT)%, $(DST_ROOT)%, $(SRC_DIRS))
EXT_644 := css inc jpeg js png
EXT_755 := dhtml

FILES_644 := $(patsubst $(SRC_ROOT)/%, %, $(foreach ext, $(EXT_644), $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.$(ext)))))
FILES_755 := $(patsubst $(SRC_ROOT)/%, %, $(foreach ext, $(EXT_755), $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.$(ext)))))

default: lighttpd.conf

lighttpd.conf: lighttpd.conf.in
	sed -e 's,@pkgdatadir@,$(pkgdatadir),g' < $^ > $@

install:
	install -d $(DESTDIR)$(sysconfdir)/lighttpd.d
	install -m 644 lighttpd.conf $(DESTDIR)$(sysconfdir)/lighttpd.d/$(PACKAGE).conf
	for dir in $(DST_DIRS); do install -d $(DESTDIR)$$dir; done
	for file in $(FILES_644); do install -m 644 $(SRC_ROOT)/$$file $(DESTDIR)$(DST_ROOT)/$$file; done
	for file in $(FILES_755); do install -m 755 $(SRC_ROOT)/$$file $(DESTDIR)$(DST_ROOT)/$$file; done
