# Written by Richard Levitte <richard@levitte.org> et al.
# Copyright (c) 2008-2014 Yubico AB
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Subdirectories

SUBDIRS= okcore . tests

ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(srcdir)/okcore
AM_CFLAGS = $(WARN_CFLAGS)

AM_CFLAGS += $(libjson_CFLAGS)

# The library.

okpers_includedir=$(includedir)/okpers-1
okpers_include_HEADERS = okpers.h okpers-version.h
okpers_include_HEADERS += okcore/okstatus.h okcore/okcore.h okcore/okdef.h
okpers_include_HEADERS += okpbkdf2.h

noinst_LTLIBRARIES = libhmac.la
libhmac_la_SOURCES = hmac.c usha.c sha.h sha1.c sha224-256.c
libhmac_la_SOURCES += sha384-512.c sha-private.h
libhmac_la_CFLAGS =

lib_LTLIBRARIES = libokpers-1.la
libokpers_1_la_SOURCES = okpers.c okpers-version.c okpbkdf2.c
if JSON
libokpers_1_la_SOURCES += okpers-json.c
else
libokpers_1_la_SOURCES += okpers-nojson.c
endif
libokpers_1_la_SOURCES += okpers_lcl.h okpers-json.h okpers_lcl.c
libokpers_1_la_SOURCES += okpers-1.pc.in libokpers-1.map
libokpers_1_la_LIBADD = $(LTLIBYUBIKEY) ./okcore/libokcore.la ./libhmac.la $(libjson_LIBS)
libokpers_1_la_LDFLAGS = -no-undefined \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
EXTRA_libokpers_1_la_DEPENDENCIES = libokpers-1.map

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = okpers-1.pc

if HAVE_LD_VERSION_SCRIPT
libokpers_1_la_LDFLAGS += -Wl,--version-script=$(srcdir)/libokpers-1.map
else
libokpers_1_la_LDFLAGS += -export-symbols-regex '^(okp|ok|okds|okpers)_.*|_ok.*_errno_location'
endif

# The command line tools.

bin_PROGRAMS = okpersonalize okchalresp okinfo

okpersonalize_SOURCES = okpersonalize.c
okpersonalize_LDADD = ./libokpers-1.la
okpersonalize_LDADD += ./libokpers_args.la

noinst_LTLIBRARIES += libokpers_args.la
libokpers_args_la_SOURCES = okpers-args.c okpers-args.h
libokpers_args_la_LIBADD = libokpers-1.la $(LTLIBYUBIKEY)

okchalresp_SOURCES = okchalresp.c
okchalresp_LDADD = ./libokpers-1.la $(LTLIBYUBIKEY)

okinfo_SOURCES = okinfo.c
okinfo_LDADD = ./libokpers-1.la $(LTLIBYUBIKEY)

dist_man1_MANS = okpersonalize.1 okchalresp.1 okinfo.1
DISTCLEANFILES = $(dist_man1_MANS)
MANSOURCES = okpersonalize.1.adoc okchalresp.1.adoc okinfo.1.adoc
SUFFIXES = .1.adoc .1
.1.adoc.1:
	$(A2X) -L --format=manpage -a revdate="Version $(VERSION)" --xsltproc-opts="--nonet" $<

# Dist docs
EXTRA_DIST = doc/Compatibility.asciidoc doc/USB-Hid-Issue.asciidoc

# Dist contrib stuff.
EXTRA_DIST += contrib/README contrib/programming.sh contrib/oath-unlock-reprogram.sh contrib/draft-josefsson-yubikey-config.xml

# Windows rules.
EXTRA_DIST += okpers4win.mk okpers4mac.mk

# Dist dev tools as well, to have the tarball complete.
EXTRA_DIST += build-and-test.sh .travis.yml .gitignore

# udev rule files
EXTRA_DIST += 69-yubikey.rules 70-yubikey.rules

# dist the man sources
EXTRA_DIST += $(MANSOURCES)

udevrulesdir=@udevrulesdir@
dist_udevrules_DATA = \
        @udevrulesfile@

if ENABLE_COV
AM_CFLAGS += --coverage
AM_LDFLAGS = --coverage

cov-reset:
	rm -fr coverage
	find . -name "*.gcda" -exec rm {} \;
	lcov --directory . --zerocounters

cov-report:
	mkdir -p coverage
	lcov --compat-libtool --directory . --capture --output-file coverage/app.info
	lcov --extract coverage/app.info '*.c' --output-file coverage/app2.info
	genhtml -o coverage/ coverage/app2.info

cov:
	make cov-report

clean-local:
	make cov-reset

check:
	make cov
endif

# Release

PROJECT = yubikey-personalization

ChangeLog:
	cd $(srcdir) && git2cl > ChangeLog

check-doc-dist:
	perl -pe "s,^EXTRA_DIST = .*,EXTRA_DIST = `cd $(srcdir) && ls doc/*.asciidoc | xargs echo`," \
		 < $(srcdir)/Makefile.am > check-doc-dist.tmp
	diff -ur $(srcdir)/Makefile.am check-doc-dist.tmp || \
		(rm -f check-doc-dist.tmp; echo 'error: please update $(srcdir)/Makefile.am to include all docs'; exit 1)
	rm -f check-doc-dist.tmp
	! man --warnings ./*.1 2>&1 | grep ': warning'

release:
	@if test -z "$(KEYID)"; then \
		echo "Try this instead:"; \
		echo "  make release KEYID=[PGPKEYID]"; \
		echo "For example:"; \
		echo "  make release KEYID=2117364A"; \
		exit 1; \
	fi
	@head -3 $(srcdir)/NEWS | grep -q "Version $(VERSION) .released `date -I`" || \
		(echo 'error: You need to update date/version in $(srcdir)/NEWS, see doc/MakeRelease.wiki.'; exit 1)
	@if test ! -d "$(YUBICO_WWW_REPO)"; then \
		echo "yubico www repo not found!"; \
		echo "Make sure that YUBICO_WWW_REPO is set"; \
		exit 1; \
	fi
	rm -f $(srcdir)/ChangeLog
	make check-doc-dist ChangeLog distcheck
	gpg --detach-sign --default-key $(KEYID) $(PACKAGE)-$(VERSION).tar.gz
	gpg --verify $(PACKAGE)-$(VERSION).tar.gz.sig
	cd $(srcdir) && git push
	cd $(srcdir) && git tag -u $(KEYID) -m $(VERSION) v$(VERSION)
	cd $(srcdir) && git push --tags
	$(YUBICO_WWW_REPO)/publish $(PROJECT) $(VERSION) $(PACKAGE)-$(VERSION).tar.gz*
