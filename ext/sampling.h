/*
 * sampling.h
 *
 *  Created on: Jan 15, 2015
 *      Author: francis
 */

#ifndef SAMPLING_H_
#define SAMPLING_H_

#include "perf.h"

#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))

struct frame {
	char *co_filename;
	char *co_name;
	size_t co_filename_len;
	size_t co_name_len;
	int lineno;
};

enum event_status {
    EVENT_STATUS_OPENED = 42,
    EVENT_STATUS_CLOSED,
    EVENT_STATUS_FAILED,
};

struct event_status_decl {
    char *name;
    int value;
};
extern struct event_status_decl event_status__constants[];

typedef struct event_ob {
    PyObject_HEAD;
    struct perf_event_attr attr;
    int fd;
    enum event_status status;
} PyPerfEvent;

extern PyTypeObject event_ob__type;
extern PyModuleDef sampling__moduledef;

extern PyObject* enable_perf(PyObject* self, PyObject* args);
extern PyObject* disable_perf(PyObject* self, PyObject* args);
extern PyObject* do_traceback(PyObject* self, PyObject *args);
extern PyObject* do_unwind(PyObject* self, PyObject *args);
extern PyObject* do_traceback_full(PyObject* self, PyObject *args);
extern PyObject* is_frame_utf8(PyObject* self, PyObject *args);

#endif /* SAMPLING_H_ */
