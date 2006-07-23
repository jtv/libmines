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

%.o: %.cxx
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

gamelogic.o: gamelogic.cxx save.hxx

c_abi.o: c_abi.cxx

save.o: save.cxx save.hxx

.PHONY: all library

