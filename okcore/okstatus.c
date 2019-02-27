/* -*- mode:C; c-file-style: "bsd" -*- */
/*
 * Written by Richard Levitte <richard@levitte.org>
 * Copyright (c) 2008-2012 Yubico AB
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

#include "okcore_lcl.h"
#include "okdef.h"
#include "okstatus.h"

OK_STATUS *okds_alloc(void)
{
	OK_STATUS *st = malloc(sizeof(OK_STATUS));
	if (!st) {
		ok_errno = OK_ENOMEM;
	}
	return st;
}

void okds_free(OK_STATUS *st)
{
	free(st);
}

OK_STATUS *okds_static(void)
{
	static OK_STATUS st;
	return &st;
}

extern int okds_version_major(const OK_STATUS *st)
{
	if (st)
		return st->versionMajor;
	ok_errno = OK_ENOSTATUS;
	return 0;
}
extern int okds_version_minor(const OK_STATUS *st)
{
	if (st)
		return st->versionMinor;
	ok_errno = OK_ENOSTATUS;
	return 0;
}
extern int okds_version_build(const OK_STATUS *st)
{
	if (st)
		return st->versionBuild;
	ok_errno = OK_ENOSTATUS;
	return 0;
}
extern int okds_pgm_seq(const OK_STATUS *st)
{
	if (st)
		return st->pgmSeq;
	ok_errno = OK_ENOSTATUS;
	return 0;
}
extern int okds_touch_level(const OK_STATUS *st)
{
	if (st)
		return st->touchLevel;
	ok_errno = OK_ENOSTATUS;
	return 0;
}
