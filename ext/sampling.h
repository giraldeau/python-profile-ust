/*
 * sampling.h
 *
 *  Created on: Jan 15, 2015
 *      Author: francis
 */

#ifndef SAMPLING_H_
#define SAMPLING_H_

#include "perf.h"

struct frame {
	char *co_filename;
	char *co_name;
	size_t co_filename_len;
	size_t co_name_len;
	int lineno;
};

typedef struct event_ob {
    PyObject_HEAD;
    struct perf_event_attr attr;
} PyPerfEvent;

extern PyTypeObject event_ob__type;
extern PyModuleDef sampling__moduledef;

extern PyObject* enable_perf(PyObject* self, PyObject* args);
extern PyObject* disable_perf(PyObject* self, PyObject* args);
extern PyObject* traceback_ust(PyObject* self, PyObject *args);
extern PyObject* is_frame_utf8(PyObject* self, PyObject *args);

#endif /* SAMPLING_H_ */
