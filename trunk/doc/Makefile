#! /usr/bin/make

all:
	if which doxygen >/dev/null ; then cd ../include ; doxygen ../doc/Doxyfile ; fi

clean:

distclean: clean
	$(RM) -r html

.PHONY: all clean distclean

