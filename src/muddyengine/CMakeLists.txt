
add_library(muddyengine engine.c account.c affect.c area.c buffer.c channel.c character.c class.c color.c db.c exit.c explored.c fight.c flag.c forum.c grid.c hashmap.c help.c hint.c log.c lookup.c map.c nonplayer.c object.c player.c race.c room.c script.c skill.c social.c string.c util.c variant.c)

install(TARGETS muddyengine DESTINATION lib)
install(FILES account.h affect.h area.h buffer.h channel.h character.h class.h color.h connection.h db.h engine.h exit.h explored.h fight.h flag.h forum.h grid.h gsn.h hashmap.h help.h hint.h log.h lookup.h macro.h map.h nonplayer.h object.h player.h race.h room.h script.h skill.h social.h spellfun.h string.h util.h variant.h DESTINATION include)

add_dependencies(muddyplains muddyengine)

include_directories(..)