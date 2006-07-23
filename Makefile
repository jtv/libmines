#! /usr/bin/make

CXXFLAGS=-O2 -g \
	-Wall \
	-Werror \
	-fstrict-aliasing \
	-pedantic \
	-W \
	-Wshadow \
	-Wreorder \
	-Wold-style-cast \
	-Woverloaded-virtual
CFLAGS=-O2 -g \
	-Wall \
	-Werror \
	-fstrict-aliasing \
	-pedantic \
	-W \
	-Wshadow

# TODO: Rebuild all when include/*.h* changes
all: library clients doc

clean:
	make -C src clean
	make -C clients clean
	make -C doc clean

distclean: clean
	make -C src distclean
	make -C clients distclean
	make -C doc distclean

library:
	make -C src CXXFLAGS="$(CXXFLAGS)" CFLAGS="$(CFLAGS)" CPPFLAGS="$(CPPFLAGS) -I../include" LDFLAGS="$(LDFLAGS) -L../src"

clients:
	make -C clients CXXFLAGS="$(CXXFLAGS)" CFLAGS="$(CFLAGS)" CPPFLAGS="$(CPPFLAGS) -I../include" LDFLAGS="$(LDFLAGS) -L../src"

doc:
	make -C doc

.PHONY: all clean distclean library clients doc

