# Written by Simon Josefsson <simon@josefsson.org>.
# Copyright (c) 2010-2014 Yubico AB
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

LIBYUBIKEYVERSION=1.13
LIBJSONVERSION=0.11
PROJECT=yubikey-personalization
PACKAGE=okpers

all: usage okpers4win32 okpers4win64

.PHONY: usage
usage:
	@if test -z "$(VERSION)" || test -z "$(PGPKEYID)"; then \
		echo "Try this instead:"; \
		echo "  make PGPKEYID=[PGPKEYID] VERSION=[VERSION]"; \
		echo "For example:"; \
		echo "  make PGPKEYID=2117364A VERSION=1.6.0"; \
		exit 1; \
	fi

okpers4win:
	rm -rf tmp && mkdir tmp && cd tmp && \
	mkdir -p root/licenses && \
	cp ../json-c-$(LIBJSONVERSION) . \
		||	wget --no-check-certificate https://s3.amazonaws.com/json-c_releases/releases/json-c-$(LIBJSONVERSION).tar.gz && \
	tar xfa json-c-$(LIBJSONVERSION).tar.gz && \
	cd json-c-$(LIBJSONVERSION) && \
	CFLAGS="-Wno-error" ac_cv_func_realloc_0_nonnull=yes ac_cv_func_malloc_0_nonnull=yes ./configure --host=$(HOST) --build=x86_64-unknown-linux-gnu --prefix=$(PWD)/tmp/root && \
	make install && \
	cp COPYING $(PWD)/tmp/root/licenses/json-c.txt && \
	cd .. && \
	cp ../libyubikey-$(LIBYUBIKEYVERSION).tar.gz . \
		||	wget https://developers.yubico.com/yubico-c/releases/libyubikey-$(LIBYUBIKEYVERSION).tar.gz && \
	tar xfa libyubikey-$(LIBYUBIKEYVERSION).tar.gz && \
	cd libyubikey-$(LIBYUBIKEYVERSION) && \
	./configure --host=$(HOST) --build=x86_64-unknown-linux-gnu --prefix=$(PWD)/tmp/root && \
	make install $(CHECK) && \
	cp COPYING $(PWD)/tmp/root/licenses/libyubikey.txt && \
	cd .. && \
	cp ../okpers-$(VERSION).tar.gz . \
		|| wget https://github.com/trustcrypto/yubikey-personalization/releases/download/v$(VERSION)/okpers-$(VERSION).tar.gz && \
	tar xfa okpers-$(VERSION).tar.gz && \
	cd okpers-$(VERSION)/ && \
	PKG_CONFIG_PATH=$(PWD)/tmp/root/lib/pkgconfig lt_cv_deplibs_check_method=pass_all ./configure --host=$(HOST) --build=x86_64-unknown-linux-gnu --prefix=$(PWD)/tmp/root LDFLAGS=-L$(PWD)/tmp/root/lib CPPFLAGS=-I$(PWD)/tmp/root/include && \
	make install $(CHECK) && \
	rm $(PWD)/tmp/root/lib/*.la && \
	rm -rf $(PWD)/tmp/root/lib/pkgconfig && \
	cp COPYING $(PWD)/tmp/root/licenses/yubikey-personalization.txt && \
	cd .. && \
	cd root && \
	zip -r ../../okpers-$(VERSION)-win$(ARCH).zip *

okpers4win32:
	$(MAKE) -f okpers4win.mk okpers4win ARCH=32 HOST=i686-w64-mingw32 CHECK=check

okpers4win64:
	$(MAKE) -f okpers4win.mk okpers4win ARCH=64 HOST=x86_64-w64-mingw32 CHECK=check

upload-okpers4win:
	@if test ! -d "$(YUBICO_WWW_REPO)"; then \
		echo "yubico www repo not found!"; \
		echo "Make sure that YUBICO_WWW_REPO is set"; \
		exit 1; \
	fi
	gpg --detach-sign --default-key $(PGPKEYID) \
		$(PACKAGE)-$(VERSION)-win$(BITS).zip
	gpg --verify $(PACKAGE)-$(VERSION)-win$(BITS).zip.sig
	$(YUBICO_WWW_REPO)/publish $(PROJECT) $(VERSION) $(PACKAGE)-$(VERSION)-win${BITS}.zip*

upload-okpers4win32:
	$(MAKE) -f okpers4win.mk upload-okpers4win BITS=32

upload-okpers4win64:
	$(MAKE) -f okpers4win.mk upload-okpers4win BITS=64
