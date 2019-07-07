set(HEADER_FILES account.h
affect.h
area.h
buffer.h
cdecl.h
channel.h
character.h
class.h
color.h
connection.h
db.h
engine.h
exit.h
explored.h
fight.h
flag.h
forum.h
grid.h
gsn.h
hashmap.h
help.h
hint.h
log.h
lookup.h
macro.h
map.h
nonplayer.h
object.h
player.h
private.h
race.h
room.h
script.h
skill.h
social.h
spellfun.h
str.h
typedef.h
util.h
variant.h)

set(SOURCE_FILES
account.c
affect.c
area.c
buffer.c
channel.c
character.c
class.c
color.c
db.c
engine.c
exit.c
explored.c
fight.c
flag.c
forum.c
grid.c
hashmap.c
help.c
hint.c
log.c
lookup.c
map.c
nonplayer.c
object.c
player.c
race.c
room.c
script.c
skill.c
social.c
str.c
util.c
variant.c)

add_library(muddle ${SOURCE_FILES})

target_link_libraries(muddle m sqlite3 crypt lua5.3)

add_subdirectory(server)

install(TARGETS muddle LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


