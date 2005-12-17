#! /usr//bin/make

OBJS=ui_cli.o gamelogic.o c_abi.o save.o
DELIVERABLES=ui_cli libmines.a
CXXFLAGS=-O2 -g \
	-Wall \
	-Werror \
	-fstrict-aliasing \
	-funit-at-a-time \
	-pedantic \
	-W \
	-Wextra \
	-Wshadow \
	-Wreorder \
	-Wold-style-cast \
	-Woverloaded-virtual

all: ui_cli library

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) $(DELIVERABLES)

ui_cli: ui_cli.o libmines.a
	$(CXX) $(LDFLAGS) $(LOADLIBES) $^ -o $@

library: libmines.a

libmines.a: c_abi.o gamelogic.o save.o
	$(AR) rc $@ $^

ui_cli.o: ui_cli.cxx gamelogic.hxx
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

gamelogic.o: gamelogic.cxx gamelogic.hxx save.hxx
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

c_abi.o: c_abi.cxx c_abi.h gamelogic.hxx
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

save.o: save.cxx save.hxx
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

.PHONY: all library
