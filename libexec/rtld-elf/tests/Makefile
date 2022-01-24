# $FreeBSD$

SUBDIR+=	libhelloworld libprints libglobvar libpythagoras target dlopen_sandbox_target

SUBDIR_DEPEND_target=	libhelloworld libprints libglobvar libpythagoras

ATF_TESTS_C=	ld_library_pathfds
ATF_TESTS_C+=	ld_preload_fds
ATF_TESTS_C+=	ld_sandbox_checks

.for t in ${ATF_TESTS_C}
SRCS.$t=	$t.c common.c
.endfor

WARNS?=		3

.include <bsd.test.mk>
