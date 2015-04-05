
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src src/server tests resources

webdatadir = $(localstatedir)/www

ARCHIVE = muddled_web.tar.lz

dist_webdata_DATA = $(ARCHIVE)

$(ARCHIVE):
	tar --exclude={Makefile.*,doc,.git*} --lzma -cvf $(ARCHIVE) -C $(srcdir)/web .

install-data-hook:
	cd $(DESTDIR)$(webdatadir); \
	$(TAR) --lzma --exclude=._* -tf $(ARCHIVE) > uninstall_manifest.txt; \
	$(TAR) --lzma --no-same-owner -xf $(ARCHIVE); \
	rm -f $(ARCHIVE); \
	cat uninstall_manifest.txt | sed --expression='s/^\|$$/"/g' | xargs chmod a=rX,u+w

uninstall-local:
	cd $(DESTDIR)$(webdatadir); \
	cat uninstall_manifest.txt | sed --expression='s/ /\\ /g' | xargs rm -f; \
	rm -f uninstall_manifest.txt
