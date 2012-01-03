
TESTS = check_engine check_server
check_PROGRAMS = check_engine check_server

check_engine_SOURCES = check_engine.c check_engine.h check_db.c check_account.c \
	check_area.c check_character.c check_color.c check_flags.c check_explored.c \
	check_string.c check_object.c check_hashmap.c
	
check_engine_LDADD = $(top_builddir)/src/libmuddyengine.la

check_server_SOURCES = check_server.c check_server.h check_commands.c check_update.c

check_server_LDADD = $(top_builddir)/src/libmuddyengine.la $(top_builddir)/src/command.o \
	$(top_builddir)/src/client.o $(top_builddir)/src/telnet.o $(top_builddir)/src/olc_room.o \
	$(top_builddir)/src/server.o $(top_builddir)/src/olc_help.o $(top_builddir)/src/olc.o \
	$(top_builddir)/src/olc_exit.o $(top_builddir)/src/olc_npc.o \
	$(top_builddir)/src/olc_area.o $(top_builddir)/src/olc_object.o $(top_builddir)/src/olc_player.o \
	$(top_builddir)/src/olc_social.o $(top_builddir)/src/command_admin.o $(top_builddir)/src/command_communication.o \
	$(top_builddir)/src/command_settings.o $(top_builddir)/src/command_info.o $(top_builddir)/src/channel.o \
	$(top_builddir)/src/olc_skill.o $(top_builddir)/src/update.o  $(top_builddir)/src/olc_engine.o \
	$(top_builddir)/src/importer.o

	
CLEANFILES = *~

