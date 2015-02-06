/*
 * pmu.c
 *
 *  Created on: Jan 15, 2015
 *      Author: francis
 */
#include <Python.h>
#include <frameobject.h>
#include <unicodeobject.h>
#include <bytesobject.h>
#include <string.h>
#include <libperfuser.h>
#include <pthread.h>

#include "tp.h"
#include "pmu.h"

#define DEPTH_MAX 100
static __thread struct frame tsf[DEPTH_MAX];

static void handle_signal(void *data)
{
    traceback_ust(NULL, NULL);
}

static struct perfuser conf = {
    .signum = SIGUSR1,
    .sample = handle_signal,
    .data = NULL,
};

PyObject*
enable_perf(PyObject* self, PyObject* args)
{
    int ret = perfuser_register(&conf);
    printf("perfuser_register=%d\n", ret);
    Py_RETURN_NONE;
}

PyObject*
disable_perf(PyObject* self, PyObject* args)
{
    int ret = perfuser_unregister();
    printf("perfuser_unregister=%d\n", ret);
    Py_RETURN_NONE;
}

static inline void
populate_utf8(PyObject *unicode, char **str, size_t *len)
{
    *str = PyUnicode_UTF8(unicode);
    *len = PyUnicode_UTF8_LENGTH(unicode);
}

PyObject *
traceback_ust(PyObject* self, PyObject* args)
{
    PyFrameObject *frame;
    size_t depth = 0;

    frame = PyEval_GetFrame();
    while (frame != NULL && depth < DEPTH_MAX) {
        if (!(PyFrame_Check(frame) &&
              frame->f_code != NULL && PyCode_Check(frame->f_code) &&
              frame->f_code->co_filename != NULL && PyUnicode_Check(frame->f_code->co_filename) &&
              frame->f_code->co_name != NULL && PyUnicode_Check(frame->f_code->co_name))) {
            break;
        }
        struct frame *item = &tsf[depth];
        memset(item, 0, sizeof(*item));

        populate_utf8(frame->f_code->co_filename, &item->co_filename, &item->co_filename_len);
        populate_utf8(frame->f_code->co_name, &item->co_name, &item->co_name_len);
        item->lineno = PyFrame_GetLineNumber(frame);
        frame = frame->f_back;
        depth++;
    }
    tracepoint(python, traceback, tsf, depth);
    Py_RETURN_NONE;
}

static inline int
is_utf8(PyObject *unicode)
{
    printf("PyUnicode_Check       = %d\n", PyUnicode_Check(unicode));
    printf("PyUnicode_UTF8        = %p\n", PyUnicode_UTF8(unicode));
    printf("PyUnicode_UTF8_LENGTH = %lu\n", PyUnicode_UTF8_LENGTH(unicode));
    printf("PyUnicode_UTF8        = %s\n", PyUnicode_UTF8(unicode));

    printf("PyUnicode_STR         = %s\n", PyBytes_AsString(PyUnicode_AsUTF8String(unicode)));
    printf("PyUnicode_KIND        = %d\n", PyUnicode_KIND(unicode));
    printf("PyUnicode_IS_COMPACT  = %d\n", PyUnicode_IS_COMPACT(unicode));
    printf("PyUnicode_DATA        = %p\n", PyUnicode_DATA(unicode));
    printf("PyUnicode_DATA        = %s\n", PyUnicode_DATA(unicode));
    printf("\n");

    return PyUnicode_Check(unicode) &&
            PyUnicode_UTF8(unicode) != NULL &&
            PyUnicode_UTF8_LENGTH(unicode) > 0;
}

PyObject *
is_frame_utf8(PyObject* self, PyObject* args)
{
    PyFrameObject *frame = PyEval_GetFrame();
    while (frame != NULL) {
        printf("before\n");
        is_utf8(frame->f_code->co_name);
        PyUnicode_AsUTF8(frame->f_code->co_name);
        printf("after\n");
        is_utf8(frame->f_code->co_name);
        /*
        if (!(PyFrame_Check(frame)
                && is_utf8(frame->f_code->co_filename)
                && is_utf8(frame->f_code->co_name))) {
            Py_RETURN_FALSE;
        }
        */
        frame = frame->f_back;
    }
    Py_RETURN_TRUE;
}
