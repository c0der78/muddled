.PHONY: all debug release clean

all: debug

config:
	@if ! test -e configure ; then \
		autoreconf -is; \
	fi

debug: config
	@if ! test -d debug ; then \
		mkdir debug; \
	fi;
	@if ! test -e debug/Makefile ; then \
		cd debug && ../configure --enable-debug $(CONFIG); \
	fi;
	@cd debug && $(MAKE) $(ARGS)

release: config
	@if ! test -d release ; then \
		mkdir release; \
	fi;
	@if ! test -e release/Makefile ; then \
		cd release && ../configure $(CONFIG); \
	fi;
	@cd release && $(MAKE) $(ARGS)

clean:
	@if test -d debug ; then \
		cd debug && $(MAKE) clean; \
	fi;
	@if test -d release ; then \
		cd release && $(MAKE) clean; \
	fi;

dist-clean:
	@rm -rf debug release configure
