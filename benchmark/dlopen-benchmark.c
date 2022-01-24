#ifndef COMPILING_ON_ZENO
#include "../contrib/libxo/libxo/xo.h"
#include "../include/dlfcn.h"
#include "../include/stdio.h"
#include "../include/string.h"
#include "../include/sysexits.h"
#include "../lib/libpmc/pmc.h"
#else
#include <dlfcn.h>
#include <libxo/xo.h>
#include <pmc.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>
#endif

#if defined(BENCHMARK_DLOPEN)
#define DLOPEN dlopen
#elif defined(BENCHMARK_DLOPEN_SANDBOX)
#define DLOPEN dlopen_sandbox
#else
#define DLOPEN fail, one of the above symbols need to be defined
#endif

#ifndef DLOPEN_MODE
#define DLOPEN_MODE RTLD_NOW
#endif

#define NUM_COUNTERS (3U)
static const char *counter_set[] = {
	"branches",
	"branch-mispredicts",
	"unhalted-cycles"
};

#define NO_OUTER_ITERATIONS (1000U)
#define NO_INNER_ITERATIONS (100U)

#define NUM_TEST_LIBRARIES (3U)
static const char *test_library_absolute_paths[] = {
	TEST_LIBRARIES_ROOT "/libhelloworld/libhelloworld.so.0",
	TEST_LIBRARIES_ROOT "/libglobvar/libglobvar.so.0",
	"/home/gp472/ld-exploration/obj/home/gp472/ld-exploration/freebsd-src/amd64.amd64/lib/libz_nofio/libz_nofio.so.6",
	TEST_LIBRARIES_ROOT "/libprints/libprints.so.0", /* Doesn't pass sandbox policy */
	NULL
};

static pmc_id_t pmcids[NUM_COUNTERS];
static unsigned long pmc_values[NUM_TEST_LIBRARIES][NUM_COUNTERS][NO_OUTER_ITERATIONS];

static void
pmc_setup_run(void)
{
	for (int counter_index = 0; counter_index < NUM_COUNTERS; ++counter_index) {
		if (pmc_allocate(counter_set[counter_index], PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcids[counter_index], 64 * 1024) < 0) {
			xo_err(EX_OSERR, "FAIL: pmc_allocate (%s) for %s", strerror(errno), counter_set[counter_index]);
		}
		if (pmc_attach(pmcids[counter_index], 0) < 0) {
			xo_err(EX_OSERR, "FAIL: pmc_attach (%s) for %s", strerror(errno), counter_set[counter_index]);
		}
		if (pmc_write(pmcids[counter_index], 0) < 0) {
			xo_err(EX_OSERR, "FAIL: pmc_write (%s) for %s", strerror(errno), counter_set[counter_index]);
		}
	}
}

static void
pmc_teardown_run(void)
{
	for (int counter_index = 0; counter_index < NUM_COUNTERS; ++counter_index) {
		if (pmc_detach(pmcids[counter_index], 0) < 0) {
			xo_err(EX_OSERR, "FAIL: pmc_detach (%s) for %s", strerror(errno), counter_set[counter_index]);
		}
		if (pmc_release(pmcids[counter_index]) < 0) {
			xo_err(EX_OSERR, "FAIL: pmc_release (%s) for %s", strerror(errno), counter_set[counter_index]);
		}
	}
}

static __inline void
pmc_begin(void)
{
	for (int counter_index = 0; counter_index < NUM_COUNTERS; ++counter_index) {
		if (pmc_start(pmcids[counter_index]) < 0) {
			xo_err(EX_OSERR, "FAIL: pmc_start (%s) for %s", strerror(errno), counter_set[counter_index]);
		}
	}
}

static __inline void
pmc_end(void)
{
	for (int counter_index = 0; counter_index < NUM_COUNTERS; ++counter_index) {
		if (pmc_stop(pmcids[counter_index]) < 0) {
			xo_err(EX_OSERR, "FAIL: pmc_stop (%s) for %s", strerror(errno), counter_set[counter_index]);
		}
	}
}


int
main(int argc, char *argv[])
{
	/* Initialise PMC library */
	pmc_init();

	/* Run dlopen-dlclose benchmark in a loop on the set of test libraries */
	for (int testlib_no = 0; testlib_no < NUM_TEST_LIBRARIES; ++testlib_no) {

		for (int outer_it_no = 0; outer_it_no < NO_OUTER_ITERATIONS; ++outer_it_no) {

			/* Assert object is closed */
			if (NULL != DLOPEN(test_library_absolute_paths[testlib_no], RTLD_NOLOAD)) {
				fprintf(stderr, "Assert failed, object not closed between iterations\n");
			}

			/* Attach and start counters */
			pmc_setup_run();
			pmc_begin();

			for (int inner_it_no = 0; inner_it_no < NO_INNER_ITERATIONS; ++inner_it_no) {
				void *handle = DLOPEN(test_library_absolute_paths[testlib_no], DLOPEN_MODE);

				if (NULL == handle) {
					xo_err(EX_OSERR, "FAIL: dlopen (%s)", dlerror());
				}

				if (dlclose(handle) < 0) {
					xo_err(EX_OSERR, "FAIL: dlclose (%s)", dlerror());
				}
			}

			for (int counter_index = 0; counter_index < NUM_COUNTERS; ++counter_index) {
				if (pmc_read(pmcids[counter_index], &pmc_values[testlib_no][counter_index][outer_it_no]) < 0) {
					xo_err(EX_OSERR, "FAIL: pmc_read (%s) for %s", strerror(errno), counter_set[counter_index]);
				}
			}

			/* Detach counters */
			pmc_end();
			pmc_teardown_run();
		}
	}

	/* End of benchmark */
	/* Print results */
	xo_parse_args(argc, argv);
#ifdef BENCHMARK_DLOPEN
	xo_open_container("dlopen-dlclose");
#else
	xo_open_container("dlopen_sandbox-dlclose");
#endif

	xo_emit("{Lwc:Outer iterations}{:outer-iterations/%u}", NO_OUTER_ITERATIONS);
	xo_emit("{Lwc:Inner iterations}{:inner-iterations/%u}", NO_INNER_ITERATIONS);

	xo_open_list("libraries");
	for (int libno = 0; libno < NUM_TEST_LIBRARIES; ++libno) {
		xo_open_instance("libraries");
		xo_emit("{Lwc:Library path}{:library-path/%s}\n", test_library_absolute_paths[libno]);
		for (int counter_index = 0; counter_index < NUM_COUNTERS; ++counter_index) {
			xo_open_list(counter_set[counter_index]);
			for (int i = 0; i < NO_OUTER_ITERATIONS; ++i) {
				const char *const fmt = "{Lwc:%s}{l:%s/%cU}\n";
				char fmt_buf[50];
				sprintf(fmt_buf, fmt, counter_set[counter_index], counter_set[counter_index], '%');
				xo_emit(fmt_buf, pmc_values[libno][counter_index][i]);
			}
			xo_close_list(counter_set[counter_index]);
		}
		xo_close_instance("libraries");
	}
	xo_close_list("libraries");

#ifdef BENCHMARK_DLOPEN
	xo_close_container("dlopen-dlclose");
#else
	xo_close_container("dlopen_sandbox-dlclose");
#endif

	/* Finish writing out data */
	xo_finish();

	return 0;
}
