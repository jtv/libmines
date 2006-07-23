#! /usr/bin/make

OBJS=ui_cli.o ui_web.o
DELIVERABLES=ui_cli ui_web

LOADLIBES += -lmines

# Ensure that standard C++ libraries are linked in, even in C...
CC=$(CXX)

%.o: %.cxx
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

all: $(DELIVERABLES)

clean:
	$(RM) $(OBJS) mines.savedgame

distclean: clean
	$(RM) $(DELIVERABLES)


.PHONY: all clean distclean

