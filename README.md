libmines - https://github.com/jtv/libmines

Welcome to libmines, a reusable library for building Minesweeper games.

Introduction
------------

This package is for programmers who want to write their own version of the
famous little puzzle game.  (If you think you know the game from soup to nuts
and it's boring, do read on).  Building libmines requires a somewhat Unix-like
system and the GNU version of "make"--it may be installed on your system as
"gmake".

Everybody's got their own version of Minesweeper.  All of them are essentially
the same, with different user interfaces--often just so they can use different
user-interface toolkits.

Mostly these versions are "good enough."  But the way we programmers like to do
things, once everyone agrees what something should do at its core, we call it
"well-understood" and separate the core from the parts that everyone wants to do
differently.  That's what libmines is: it provides the heart of the game, the
"game logic," with some room for variation, so you can build your own new user
interface without having to write the actual puzzle part all over again.  Of
course libmines is Free Software (or Open Source, as some prefer to say) so feel
free to change it and give it to your friends.  Better yet, once you know what
you want changed in the library to suit your own project, share your wishes and
changes with the author so everyone can benefit!

Doing things this way makes the programming easier for everyone.  It also makes
for cleaner code.  It's a great way to learn programming, from making one's
first steps with a programming language to practicing one's software-engineering
skills.  And perhaps most of all, it makes it easy to do really new stuff with
the game!

A very basic text-based command line client is included, which is meant mostly
for testing.  But there's also a simple CGI program: set up your web server to
provide access to it, and you can play the game in your browser through the
Internet.  An example is running on pqxx.org--see the main development page.

Those sample user interfaces aren't great, so here's your chance.  Perhaps you
can be the one to write a much better one.  Or be the first to build an online
multiplayer version.  Or figure out new ways to present the playing field.  If
you want to do it all in 3D, well, we can talk.


Boring!
-------

Your average Minesweeper gets boring fairly quickly.  You may even think of it
as mindless--one visitor found the project page by searching the Internet for 
"Minesweeper and other mindless games."  Well, with libmines, it doesn't have to
be.

Most Minesweeper games work like this: you click on a mineless square to reveal
it.  The square will then show the number of mines neighbouring it.  If there
are no nearby mines, all surrounding squares will also be revealed, and if some
of those also show zeroes, their neighbours will be revealed and so on.  Then
you go on to the next obviously mineless square, until the game finally becomes
harder.  A large portion of the mistakes come from accidentally clicking in the
wrong place, clicking with the wrong mouse button, or simply boredom and
drifting attention.  Yes, the game becomes mindless pretty quickly that way.

That's why libmines goes a little further.  You can specify various levels of
intelligence.  Level zero doesn't "auto-reveal" anything at all, just what you
click on.  Level one does it for squares that aren't near mines, like the other
versions of the game.  Level two is smarter: it will also reveal squares that
are near mines, as long as all those mines have already been accounted for.  And
conversely, it also recognizes the situation where all remaining neighbours of a
field must be mines.  What remains are the interesting situations where you need
to do some actual thinking.  This way, the game is over much sooner--or you can
play much bigger fields with more mines.  You can do the thinking, and leave the
dumb work to the library.

Another way to make things interesting again is to vary the rules a bit.  You
can show squares outside the actual playing fields, so the player can see what
squares are safe to click on as starting points.  That eliminates the silly
guessing at the beginning of the game, as you look for a usable starting point.
You can let the user mark possible mines with flags, as most versions of the
game do, or you can make it a fatal mistake to get this wrong, or you can do a
bit of both.  Players can have multiple lives if you want.  If you would like to
do really exotic things like add or remove mines during the game, or make them
move around, they may not be possible yet but we can add them.


Writing your own
----------------

The native language for libmines is C++.  If that is the language you want to
write your game in, that's easy.  If it's not, there is also an API in C that,
with a few small exceptions, supports all of the native features.  Most if not
all other programming languages offer ways to call C functions, so this should
be enough to support user interfaces in just about any language out there.

Saving games is also possible, of course.  The file format consists of a few
lines describing the playing field, followed by a compact base64-encoded binary
representation of the field itself (two bits per square: "is mined" and "has
been revealed").  The data is written to a memory buffer provided by you; if you
want to add data of your own, you can write that to file before or after the
buffer's contents.

The saved-game feature was designed for performance.  That may seem silly: how
often do you want to save a game, and how long can it take?  We could have made
things slower and more flexible, but doing that yourself shouldn't be hard.  The
fast, compact format lets you go to other extremes: the included web user
interface saves every game to a file after every move, and reloads it every time
the field is displayed.  That will scale to enormous playing fields and huge
numbers of simultaneous games.  Future versions could optimize it even further
by storing games in shared memory.  Or someone might want to write a version for
mobile phones or other small devices, and keep game state in a tiny bit of
non-volatile memory.

To start using libmines in C++, take a look at the source files with names
ending in ".hxx".  These headers define the C++ API.  The C interface is defined
in a header called c_abi.h.

For more easily readable documentation on how to use libmines in your own
program, see the documentation in the "doc" directory.  If you would like to
have this documentation generated in other formats such as LaTeX, ensure that
the documentation extractor "doxygen" is installed on your system; edit the
configuration file doc/Doxyfile accordingly, then run "make".

