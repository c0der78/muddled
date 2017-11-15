#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([muddled], [0.1], [c0der78@gmail.com])
AC_CONFIG_SRCDIR([src/engine.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE

LT_INIT

AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_CHECK_PROG(TAR, tar, tar)

AC_SUBST(X_CFLAGS, "-std=c99 -pedantic -Wall -Werror -D_XOPEN_SOURCE=700")

# Checks for libraries.
AC_CHECK_LIB(websockets, lws_create_context)
AC_CHECK_LIB(sqlite3, sqlite3_open)
AC_CHECK_LIB(crypt, crypt)
AC_CHECK_LIB(m, sqrt)

# Checks for header files.
AC_CHECK_HEADERS([stdbool.h arpa/inet.h fcntl.h inttypes.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([atexit gethostbyaddr gettimeofday inet_ntoa isascii memmove memset memcpy strcat strncat strlen pow regcomp select socket sqrt strcasecmp strchr strerror strrchr strstr strtoul])

AC_CONFIG_FILES([Makefile src/Makefile src/server/Makefile tests/Makefile resources/Makefile])

AC_OUTPUT
