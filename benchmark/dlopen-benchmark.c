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

//#ifndef MODIFIED_SYSROOT
//void *dlopen_sandbox(const char *name, int mode);
//#endif
//
//typedef void *(*dlopen_fptr)(const char *, int);

#if defined(BENCHMARK_DLOPEN)
#define DLOPEN  dlopen
#elif defined(BENCHMARK_DLOPEN_SANDBOX)
#define DLOPEN  dlopen_sandbox
#else
#define DLOPEN  fail, one of the above symbols need to be defined
#endif

#ifndef DLOPEN_MODE
#define DLOPEN_MODE RTLD_NOW
#endif

#define NO_OUTER_ITERATIONS (10U)
#define NO_INNER_ITERATIONS (10U)

#define NUM_TEST_LIBRARIES  (2U)
const char *test_library_absolute_paths[] =  {
        TEST_LIBRARIES_ROOT "/libhelloworld/libhelloworld.so.0",
        NULL,
        TEST_LIBRARIES_ROOT "/libprints/libprints.so.0",
        NULL
};

unsigned long cycle_counts[NUM_TEST_LIBRARIES][NO_OUTER_ITERATIONS];


int main(int argc, char *argv[]) {
    pmc_id_t pmcid;

    /* Initialise PMC library */
    pmc_init();

    /* Allocate a PMC counter for the CPU cycles */
    if (pmc_allocate("unhalted-cycles", PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcid, 64*1024) < 0) {
        xo_err(EX_OSERR, "FAIL: pmc_allocate cycles (%s)", strerror(errno));
        return 1;
    }

    /* Attach PMC counter to current process */
    if (pmc_attach(pmcid, 0) < 0) {
        xo_err(EX_OSERR, "FAIL: pmc_attach cycles (%s)", strerror(errno));
        return 2;
    }

    /* Start the PMC counter */
    if (pmc_start(pmcid) < 0) {
        xo_err(EX_OSERR, "FAIL: pmc_start cycles (%s)", strerror(errno));
        return 3;
    }


    /* Run dlopen-dlclose benchmark in a loop on the set of test libraries */
    for (int testlib_no = 0; NULL != test_library_absolute_paths[testlib_no]; ++testlib_no) {

        for (int outer_it_no = 0; outer_it_no < NO_OUTER_ITERATIONS; ++outer_it_no) {

            unsigned long cnt_prev_val;
            if (pmc_read(pmcid, &cnt_prev_val) < 0) {
                xo_err(EX_OSERR, "FAIL: pmc_read (%s)", strerror(errno));
                return 4;
            }

            for (int inner_it_no = 0; inner_it_no < NO_INNER_ITERATIONS; ++inner_it_no) {
                void *handle = DLOPEN(test_library_absolute_paths[testlib_no], DLOPEN_MODE);

                if (NULL == handle) {
                    xo_err(EX_OSERR, "FAIL: dlopen (%s)", dlerror());
                    return 5;
                }

                if (dlclose(handle) < 0) {
                    xo_err(EX_OSERR, "FAIL: dlclose (%s)", dlerror());
                    return 6;
                }
            }

            unsigned long cnt_curr_val;
            if (pmc_read(pmcid, &cnt_curr_val) < 0) {
                xo_err(EX_OSERR, "FAIL: pmc_read (%s)", strerror(errno));
                return 7;
            }

            cycle_counts[testlib_no][outer_it_no] = cnt_curr_val - cnt_prev_val;

            /* Assert object is closed */
            if (NULL != DLOPEN(test_library_absolute_paths[testlib_no], RTLD_NOLOAD)) {
                fprintf(stderr, "Assert failed, object not closed between iterations\n");
            }
        }

    }

    /* End of benchmark */
    /* Stop the PMC counter */
    if (pmc_stop(pmcid) < 0) {
        xo_err(EX_OSERR, "FAIL: pmc_stop (%s)", strerror(errno));
        return 8;
    }

    /* Print results */
    xo_parse_args(argc, argv);
#ifdef BENCHMARK_DLOPEN
    xo_open_container("dlopen-dlclose");
#else
    xo_open_container("dlopen_sandbox-dlclose");
#endif

    xo_open_list("libraries");
    for (int libno = 0; libno < NUM_TEST_LIBRARIES; ++libno) {
        xo_open_instance("libraries");
        xo_emit("{Lwc:Library path}{:library-path/%s}\n", test_library_absolute_paths[libno]);
        for (int i = 0; i < NO_OUTER_ITERATIONS; ++i) {
            xo_emit("{Lwc:Unhalted cycles}{l:unhalted-cycles/%U}\n", cycle_counts[libno][i]);
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
