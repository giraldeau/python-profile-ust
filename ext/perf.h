/*
 * perf.h
 *
 *  Created on: Mar 26, 2015
 *      Author: francis
 */

#ifndef PERF_H_
#define PERF_H_

struct perf_decl {
    const char *name;
    int     value;
};

extern struct perf_decl perf__constants[];

#endif /* PERF_H_ */
