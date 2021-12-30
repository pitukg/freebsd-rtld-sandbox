/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 * Copyright 2021 Mariusz Zaborski <oshogbo@FreeBSD.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <atf-c.h>
#include <fcntl.h>
#include <stdio.h>

#include "common.h"

#define LIBHELLOWORLD_DIR   "./libhelloworld"
#define LIBHELLOWORLD_NAME  "libhelloworld.so.0"
#define LIBPRINTS_DIR       "./libprints"
#define LIBPRINTS_NAME      "libprints.so.0"

static void
setup(const atf_tc_t *tc)
{
}

ATF_TC_WITHOUT_HEAD(libhelloworld_passes);
ATF_TC_BODY(libhelloworld_passes, tc)
{

	setup(tc);
	expect_sandbox_success(LIBHELLOWORLD_DIR, LIBHELLOWORLD_NAME);
}

ATF_TC_WITHOUT_HEAD(libprints_fails);
ATF_TC_BODY(libprints_fails, tc)
{

    setup(tc);
    expect_sandbox_fail(LIBPRINTS_DIR, LIBPRINTS_NAME);
}


/* Register test cases with ATF. */
ATF_TP_ADD_TCS(tp)
{

	ATF_TP_ADD_TC(tp, libhelloworld_passes);
    ATF_TP_ADD_TC(tp, libprints_fails);

	return atf_no_error();
}
