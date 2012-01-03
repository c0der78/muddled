
SUBDIRS = muddyengine

bin_PROGRAMS = muddyplains

muddyplains_SOURCES = main.c command.c command.h server.c server.h client.c client.h \
	telnet.c telnet.h olc.c olc.h olc_room.c olc_exit.c olc_help.c olc_npc.c \
	update.c update.h olc_object.c olc_area.c olc_player.c olc_skill.c \
	command_info.c command_settings.c command_admin.c command_communication.c olc_engine.c \
	olc_social.c importer.c

AM_CFLAGS = @AM_CFLAGS@
	
muddyplains_LDADD = libmuddyengine.a $(AM_LDFLAGS)
	
lib_LIBRARIES = libmuddyengine.a

libmuddyengine_a_SOURCES = account.c area.c character.c color.c db.c exit.c engine.c help.c log.c lookup.c \
	nonplayer.c object.c player.c room.c script.c social.c string.c util.c flag.c \
	race.c hint.c fight.c class.c map.c explored.c skill.c grid.c buffer.c forum.c affect.c channel.c \
	hashmap.c variant.c
	
CLEANFILES = *~

indent:
	@mac2unix *.c *.h muddyengine/*.h >> /dev/null
	@dos2unix *.c *.h muddyengine/*.h >> /dev/null
	@indent -kr *.c *.h muddyengine/*.h
	@echo "Sources formatted."
