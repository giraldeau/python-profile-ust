/*
 * pmu.h
 *
 *  Created on: Jan 15, 2015
 *      Author: francis
 */

#ifndef PMU_H_
#define PMU_H_

#include <Python.h>

struct frame {
	char *co_filename;
	char *co_name;
	size_t co_filename_len;
	size_t co_name_len;
	int lineno;
};

extern PyObject* enable_perf(PyObject* self, PyObject* args);
extern PyObject* disable_perf(PyObject* self, PyObject* args);
extern PyObject* traceback_ust(PyObject* self, PyObject *args);

#endif /* PMU_H_ */
