/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1986, 1988, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 * Copyright (c) 2011 Konstantin Belousov <kib@FreeBSD.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include "debug.h"
#include "rtld_sandbox.h"

#include <machine/elf.h>
#include <string.h>

typedef struct Struct_AllowedSymbol {
    const char *name;
    unsigned char type;
} AllowedSymbol;

static const AllowedSymbol allowed_symbols[] = {
        {"__cxa_finalize", STT_FUNC},       /* Weak symbol defined in libc
                                               calling (C++) dtors */
        {"_Jv_RegisterClasses", STT_FUNC},  /* Weak symbol defined in libc
                                               for GNU Java compiler */
        {"malloc", STT_FUNC},
        {"free", STT_FUNC},
        {"memcpy", STT_FUNC},
        {"memset", STT_FUNC},
        {"", 0U}
        // TODO find out list of allowed symbols
};

#if 0
static const char *allowed_lib_prefixes[] = {
        "libc.so",
        "libm.so", /* C maths library */
};
#endif


bool check_symbol_allowed_in_sandbox(const char *name, unsigned char type)
{
    for (const AllowedSymbol *allowed_sym = allowed_symbols;
         allowed_sym->name[0] != '\0'; ++allowed_sym) {
        if (!strcmp(name, allowed_sym->name) && type == allowed_sym->type)
            return true;
    }

    rtld_printf("Symbol %s of type %d not allowed in sandbox\n", name, type);

    return false;
}
