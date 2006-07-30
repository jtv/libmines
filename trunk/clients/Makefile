#! /usr/bin/make

OBJS=ui_cli.o ui_web.o
DELIVERABLES=ui_cli ui_web

LOADLIBES += -lmines -lstdc++

%.o: %.cxx
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

all: $(DELIVERABLES)

clean:
	$(RM) $(OBJS) mines.savedgame

distclean: clean
	$(RM) $(DELIVERABLES)


ui_web: ui_web.o

.PHONY: all clean distclean

