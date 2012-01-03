AC_INIT(muddyplains, 1.0.1, c0der78@gmail.com, muddyplains, arg3.net/muddyplains)

AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE(muddyplains,1.0.1)

AC_CONFIG_HEADER([src/config.h])

AC_PREFIX_DEFAULT([`pwd`])

AC_CANONICAL_HOST

case "${host_os}" in
	darwin*)
	AM_CFLAGS="$AM_CFLAGS -arch x86_64 -Wshorten-64-to-32"
	;;
esac

AC_PROG_CXX

AC_PROG_INSTALL

AC_PROG_LIBTOOL

AC_PATH_PROGS(INDENT,[astyle indent])

AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug], [specify you are building with debugging on]), AM_CFLAGS="$AM_CFLAGS -std=gnu99 -ggdb3 -Wall -Werror -Wno-uninitialized -O0 -DDEBUG", AM_CFLAGS="$AM_CFLAGS -std=gnu99 -O3")

AC_ARG_WITH(local-path, AC_HELP_STRING([--with-local-path], [specify a local path to look for development files]))

if test "x$with_local_path" != "x"; then
	AC_MSG_NOTICE([Using local path $with_local_path])
	AM_CFLAGS="$AM_CFLAGS -I$with_local_path/include"
	AM_LDFLAGS="$AM_LDFLAGS -L$with_local_path/lib"
	# for the ac_check_libs
	LDFLAGS="$LDFLAGS -L$with_local_path/lib"
fi

ACREBOOTFILE=$ac_default_prefix/var/run/reboot.tmp

AC_DEFINE_UNQUOTED(REBOOT_FILE, "$ACREBOOTFILE", [Defines the temporary file used in reboots])

ACSQLITEFILE=$ac_default_prefix/share/muddyplains/muddyplains.db3

AC_DEFINE_UNQUOTED(SQLITE3_FILE, "$ACSQLITEFILE", [Defines the sqlite3 file used for the database])

ACLOGDIR=$ac_default_prefix/var/log

AC_DEFINE_UNQUOTED(LOG_DIR, "$ACLOGDIR", [Defines the log directory])

AC_HEADER_TIME
AC_CHECK_LIB(m, sqrt)
AC_CHECK_LIB(dl, dlopen)
AC_CHECK_LIB(crypt, crypt)
AC_CHECK_LIB(sqlite3, sqlite3_open)
AC_CHECK_LIB(lua5.1, lua_call)
AC_CHECK_LIB(check, suite_create)

if test "x$ac_cv_lib_sqlite3_sqlite3_open" == xno; then
	AC_MSG_ERROR([Unable to locate sqlite3 library])
fi

if test "x$ac_cv_lib5_1_lua_lua_call" == xno; then
	AC_MSG_ERROR([Unable to find liblua, scripting will be unavailable])
fi

if test "x$ac_cv_lib_check_suite_create" == x; then
	AC_MSG_WARN([Unable to find libcheck, unit testing will be unvailable])
fi

AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_LDFLAGS)

######################################################################
# DOXYGEN SUPPORT
######################################################################

DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)

DX_INIT_DOXYGEN([$PACKAGE_NAME],[doxygen.cfg])

######################################################################

AC_OUTPUT([Makefile src/Makefile src/muddyengine/Makefile tests/Makefile var/Makefile var/run/Makefile var/log/Makefile])

