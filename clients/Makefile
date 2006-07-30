#! /usr/bin/make

OBJS=ui_cli.o ui_web.o
DELIVERABLES=ui_cli ui_web

LOADLIBES += -lmines -lstdc++

%.o: %.cxx
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

all: $(DELIVERABLES)
	for i in $(DELIVERABLES) ; do strip "$$i" ; done

clean:
	$(RM) $(OBJS) mines.savedgame

distclean: clean
	$(RM) $(DELIVERABLES)


.PHONY: all clean distclean

