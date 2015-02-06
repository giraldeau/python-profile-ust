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

PyObject *
traceback_ust(PyObject* self, PyObject* args)
{
    PyFrameObject *frame;
    PyObject *co_filename, *co_name;
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

        co_filename = frame->f_code->co_filename;
        co_name = frame->f_code->co_name;
        item->co_filename= PyUnicode_DATA(co_filename);
        item->co_filename_len = PyUnicode_GET_DATA_SIZE(co_filename);
        item->co_name = PyUnicode_DATA(co_name);
        item->co_name_len = PyUnicode_GET_DATA_SIZE(co_name);
        item->lineno = PyFrame_GetLineNumber(frame);
        frame = frame->f_back;
        depth++;
    }
    tracepoint(python, traceback, tsf, depth);
    Py_RETURN_NONE;
}