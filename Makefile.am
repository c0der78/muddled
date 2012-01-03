AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src tests var

dist_pkgdata_DATA = muddyplains.db3

CLEANFILES = *~

# DOXYGEN SUPPORT
include $(top_srcdir)/aminclude.am

# ensure the distribution of the doxygen configuration file
EXTRA_DIST = doxygen.cfg $(dist_pkgdata_DATA)

indent:
	cd src && $(MAKE) indent
