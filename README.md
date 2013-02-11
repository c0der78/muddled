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
* [premake](http://industriousone.com/premake) for building (brew install premake on OSX)
* the startup script requires the 'daemons' ruby gem
* sqlite3
* lua

#### compiling
* run 'premake4 --help' to see a list of project configurations.  I use 'premake4 gmake' on OSX.
* run 'make' to build all libraries and binaries
* run 'config=release make' to build a release

#### todo
* more areas
* more commenting!
* more lua scripting
* expand the combat code
* questing

