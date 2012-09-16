[muddyplains](http://c0der78.github.com/muddyplains)
===========

a multi user dungeon in c99

see [documentation](http://c0der78.github.com/muddyplains/docs)

#### features
* ANSI color
* in-game world editor
* ASCII maps (nethack style)
* lua scripting support
* sqlite data layer
* some telnet negotation support like window size
* accounts, races, classes, skills, spells, forums, help system, etc

#### purpose
I would like to see what other people could do with this.

#### requirements
* [rake](http://rake.ruby.org) (ruby)
* the startup script requires the 'daemons' ruby gem
* sqlite3

#### optional
* lua

#### compiling
* running 'rake' will compile and run unit tests
* run 'rake -T' to see a list of specific builds
* 'muddyplains' is the release build, 'muddyplainsd' is the debug build

#### todo
* more commenting!
* more lua scripting
* flesh out the combat code

