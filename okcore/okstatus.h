/* -*- mode:C; c-file-style: "bsd" -*- */
/*
 * Written by Richard Levitte <richard@levitte.org>
 * Copyright (c) 2008-2013 Yubico AB
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef	__OKSTATUS_H_INCLUDED__
#define	__OKSTATUS_H_INCLUDED__

#include <okcore.h>

# ifdef __cplusplus
extern "C" {
# endif

/* Allocate and free status structures */
extern OK_STATUS *okds_alloc(void);
extern void okds_free(OK_STATUS *st);

/* Return static status structure, to be used for quick checks.
   USE WITH CAUTION, as this is a SHARED OBJECT. */
extern OK_STATUS *okds_static(void);

/* Accessor functions */
extern int okds_version_major(const OK_STATUS *st);
extern int okds_version_minor(const OK_STATUS *st);
extern int okds_version_build(const OK_STATUS *st);
extern int okds_pgm_seq(const OK_STATUS *st);
extern int okds_touch_level(const OK_STATUS *st);

# ifdef __cplusplus
}
# endif

#endif /* __OKSTATUS_H_INCLUDED__ */
