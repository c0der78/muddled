
check_PROGRAMS = muddled_test

muddled_test_SOURCES = check_account.c check_area.c check_character.c check_color.c check_db.c check_engine.c check_explored.c check_flags.c check_hashmap.c check_object.c check_string.c

muddled_test_CFLAGS = $(MUDDLED_TEST_CFLAGS) -I@srcdir@/../src @X_CFLAGS@

muddled_test_LDADD = ../src/libmuddled.la -lcheck $(LIBS)

check: muddled_test
	./muddled_test
