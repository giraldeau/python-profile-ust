/*
 * pmu.h
 *
 *  Created on: Jan 15, 2015
 *      Author: francis
 */

#ifndef PMU_H_
#define PMU_H_

#include <Python.h>
#include <unicodeobject.h>

#define _PyUnicode_CHECK(op) PyUnicode_Check(op)
/*
 * Macros copied from cpython/Objects/unicodeobject.c
 */
#define _PyUnicode_UTF8(op)                             \
    (((PyCompactUnicodeObject*)(op))->utf8)
#define PyUnicode_UTF8(op)                              \
    (assert(_PyUnicode_CHECK(op)),                      \
     assert(PyUnicode_IS_READY(op)),                    \
     PyUnicode_IS_COMPACT_ASCII(op) ?                   \
         ((char*)((PyASCIIObject*)(op) + 1)) :          \
         _PyUnicode_UTF8(op))
#define _PyUnicode_UTF8_LENGTH(op)                      \
    (((PyCompactUnicodeObject*)(op))->utf8_length)
#define PyUnicode_UTF8_LENGTH(op)                       \
    (assert(_PyUnicode_CHECK(op)),                      \
     assert(PyUnicode_IS_READY(op)),                    \
     PyUnicode_IS_COMPACT_ASCII(op) ?                   \
         ((PyASCIIObject*)(op))->length :               \
         _PyUnicode_UTF8_LENGTH(op))

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
extern PyObject* is_frame_utf8(PyObject* self, PyObject *args);

#endif /* PMU_H_ */
