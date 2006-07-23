#! /usr/bin/make

OBJS=gamelogic.o c_abi.o save.o
DELIVERABLES=libmines.a

all: $(DELIVERABLES)

clean:
	$(RM) $(OBJS) mines.savedgame

distclean: clean
	$(RM) $(DELIVERABLES)

library: libmines.a

libmines.a: c_abi.o gamelogic.o save.o
	$(AR) rc $@ $^

ui_cli.o: ui_cli.cxx gamelogic.hxx
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

ui_web.o: ui_web.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

gamelogic.o: gamelogic.cxx gamelogic.hxx save.hxx
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

c_abi.o: c_abi.cxx gamelogic.hxx
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

save.o: save.cxx save.hxx
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

.PHONY: all library

