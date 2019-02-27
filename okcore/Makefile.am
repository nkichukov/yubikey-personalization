# Copyright (c) 2008-2013 Yubico AB
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

noinst_LTLIBRARIES = libokcore.la
libokcore_la_SOURCES = okdef.h okcore.h okcore_lcl.h okcore_backend.h	\
	okcore.c okstatus.h okstatus.c oktsd.h okbzero.h
libokcore_la_LIBADD = $(LTLIBYUBIKEY) $(LTLIBUSB) @LIBUSB_LIBS@
AM_CFLAGS = $(WARN_CFLAGS)

if ENABLE_DEBUG
AM_CFLAGS += -DOK_DEBUG
endif

if BACKEND_LIBUSB_1_0
libokcore_la_SOURCES += okcore_libusb-1.0.c
AM_CFLAGS += @LIBUSB_CFLAGS@
endif

if BACKEND_LIBUSB
libokcore_la_SOURCES += okcore_libusb.c
endif

if BACKEND_OSX
libokcore_la_SOURCES += okcore_osx.c
endif

if BACKEND_WINDOWS
libokcore_la_SOURCES += okcore_windows.c
endif

if ENABLE_COV
AM_CFLAGS += --coverage
AM_LDFLAGS = --coverage
endif

EXTRA_DIST = okcore_stub.c
