/*
 * perf.c
 *
 *  Created on: Mar 26, 2015
 *      Author: francis
 */

/*
 * Parts copied from linux/tools/perf to avoid dependency.
 */

#include <stdlib.h>
#include <linux/perf_event.h>

#include "perf.h"

struct perf_decl perf__constants[] = {
    { "TYPE_HARDWARE",   PERF_TYPE_HARDWARE },
    { "TYPE_SOFTWARE",   PERF_TYPE_SOFTWARE },
    { "TYPE_TRACEPOINT", PERF_TYPE_TRACEPOINT },
    { "TYPE_HW_CACHE",   PERF_TYPE_HW_CACHE },
    { "TYPE_RAW",        PERF_TYPE_RAW },
    { "TYPE_BREAKPOINT", PERF_TYPE_BREAKPOINT },

    { "COUNT_HW_CPU_CYCLES",      PERF_COUNT_HW_CPU_CYCLES },
    { "COUNT_HW_INSTRUCTIONS",    PERF_COUNT_HW_INSTRUCTIONS },
    { "COUNT_HW_CACHE_REFERENCES",    PERF_COUNT_HW_CACHE_REFERENCES },
    { "COUNT_HW_CACHE_MISSES",    PERF_COUNT_HW_CACHE_MISSES },
    { "COUNT_HW_BRANCH_INSTRUCTIONS", PERF_COUNT_HW_BRANCH_INSTRUCTIONS },
    { "COUNT_HW_BRANCH_MISSES",   PERF_COUNT_HW_BRANCH_MISSES },
    { "COUNT_HW_BUS_CYCLES",      PERF_COUNT_HW_BUS_CYCLES },
    { "COUNT_HW_CACHE_L1D",       PERF_COUNT_HW_CACHE_L1D },
    { "COUNT_HW_CACHE_L1I",       PERF_COUNT_HW_CACHE_L1I },
    { "COUNT_HW_CACHE_LL",        PERF_COUNT_HW_CACHE_LL },
    { "COUNT_HW_CACHE_DTLB",      PERF_COUNT_HW_CACHE_DTLB },
    { "COUNT_HW_CACHE_ITLB",      PERF_COUNT_HW_CACHE_ITLB },
    { "COUNT_HW_CACHE_BPU",       PERF_COUNT_HW_CACHE_BPU },
    { "COUNT_HW_CACHE_OP_READ",   PERF_COUNT_HW_CACHE_OP_READ },
    { "COUNT_HW_CACHE_OP_WRITE",      PERF_COUNT_HW_CACHE_OP_WRITE },
    { "COUNT_HW_CACHE_OP_PREFETCH",   PERF_COUNT_HW_CACHE_OP_PREFETCH },
    { "COUNT_HW_CACHE_RESULT_ACCESS", PERF_COUNT_HW_CACHE_RESULT_ACCESS },
    { "COUNT_HW_CACHE_RESULT_MISS",   PERF_COUNT_HW_CACHE_RESULT_MISS },

    { "COUNT_HW_STALLED_CYCLES_FRONTEND",     PERF_COUNT_HW_STALLED_CYCLES_FRONTEND },
    { "COUNT_HW_STALLED_CYCLES_BACKEND",      PERF_COUNT_HW_STALLED_CYCLES_BACKEND },

    { "COUNT_SW_CPU_CLOCK",        PERF_COUNT_SW_CPU_CLOCK },
    { "COUNT_SW_TASK_CLOCK",       PERF_COUNT_SW_TASK_CLOCK },
    { "COUNT_SW_PAGE_FAULTS",      PERF_COUNT_SW_PAGE_FAULTS },
    { "COUNT_SW_CONTEXT_SWITCHES", PERF_COUNT_SW_CONTEXT_SWITCHES },
    { "COUNT_SW_CPU_MIGRATIONS",   PERF_COUNT_SW_CPU_MIGRATIONS },
    { "COUNT_SW_PAGE_FAULTS_MIN",  PERF_COUNT_SW_PAGE_FAULTS_MIN },
    { "COUNT_SW_PAGE_FAULTS_MAJ",  PERF_COUNT_SW_PAGE_FAULTS_MAJ },
    { "COUNT_SW_ALIGNMENT_FAULTS", PERF_COUNT_SW_ALIGNMENT_FAULTS },
    { "COUNT_SW_EMULATION_FAULTS", PERF_COUNT_SW_EMULATION_FAULTS },
    { "COUNT_SW_DUMMY",            PERF_COUNT_SW_DUMMY },

    { "SAMPLE_IP",        PERF_SAMPLE_IP },
    { "SAMPLE_TID",       PERF_SAMPLE_TID },
    { "SAMPLE_TIME",      PERF_SAMPLE_TIME },
    { "SAMPLE_ADDR",      PERF_SAMPLE_ADDR },
    { "SAMPLE_READ",      PERF_SAMPLE_READ },
    { "SAMPLE_CALLCHAIN", PERF_SAMPLE_CALLCHAIN },
    { "SAMPLE_ID",        PERF_SAMPLE_ID },
    { "SAMPLE_CPU",       PERF_SAMPLE_CPU },
    { "SAMPLE_PERIOD",    PERF_SAMPLE_PERIOD },
    { "SAMPLE_STREAM_ID", PERF_SAMPLE_STREAM_ID },
    { "SAMPLE_RAW",       PERF_SAMPLE_RAW },

    { "FORMAT_TOTAL_TIME_ENABLED", PERF_FORMAT_TOTAL_TIME_ENABLED },
    { "FORMAT_TOTAL_TIME_RUNNING", PERF_FORMAT_TOTAL_TIME_RUNNING },
    { "FORMAT_ID",             PERF_FORMAT_ID },
    { "FORMAT_GROUP",          PERF_FORMAT_GROUP },

    { "RECORD_MMAP",       PERF_RECORD_MMAP },
    { "RECORD_LOST",       PERF_RECORD_LOST },
    { "RECORD_COMM",       PERF_RECORD_COMM },
    { "RECORD_EXIT",       PERF_RECORD_EXIT },
    { "RECORD_THROTTLE",   PERF_RECORD_THROTTLE },
    { "RECORD_UNTHROTTLE", PERF_RECORD_UNTHROTTLE },
    { "RECORD_FORK",       PERF_RECORD_FORK },
    { "RECORD_READ",       PERF_RECORD_READ },
    { "RECORD_SAMPLE",     PERF_RECORD_SAMPLE },
    { .name = NULL, },
};