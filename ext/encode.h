/*
 * encode.h
 *
 *  Created on: Feb 9, 2015
 *      Author: francis
 */

#ifndef ENCODE_H_
#define ENCODE_H_

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

extern const char *get_utf8(PyObject *unicode);

#endif /* ENCODE_H_ */
