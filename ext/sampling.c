/*
 * sampling.c
 *
 *  Created on: Jan 15, 2015
 *      Author: francis
 */
#include <Python.h>
#include <frameobject.h>
#include <unicodeobject.h>
#include <bytesobject.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <fcntl.h>

#include "tp.h"
#include "sampling.h"
#include "encode.h"

#define DEPTH_MAX 100
static struct frame tsf[DEPTH_MAX];

/*
define the list of available events: copy it from perf
group -> list of event types (type,config)
open one file per event, group them
configure the signal, mmap the buffers
read event types since last read, record types or'd and count
use select to know which fds to read
 */


/*
 * We don't use TLS here, seems to possible to access a thread data from
 * another thread, nor with pthread_getspecific().
 *
 * Instead, we use a counter for the number of enable/disable. If the gen
 * variable is lower than the current ref variable, then the frame may not be
 * valid.
 */

static PyFrameObject *top;
static int gen;
static int ref = 0;

/* forward definition for handle_signal */
static void do_traceback_ust(PyFrameObject *frame);
static inline PyFrameObject *get_top_frame(void);

/*
 * Signal handler
 */

static void handle_signal(void *data)
{
    do_traceback_ust(get_top_frame());
}

/*
 * Function callback for signal safety:
 *  - encode ahead the unicode strings to UTF-8
 *  - keep a pointer to the top-frame
 */

static inline void
set_top_frame(PyFrameObject *frame)
{
    /* make sure the top frame is assigned before the generation is set */
    top = frame;
    __sync_synchronize();
    if (gen != ref) {
        gen = ref;
    }
}

static inline PyFrameObject *
get_top_frame(void)
{
    return (gen == ref) ? top : NULL;
}

static int
function_handler(PyObject *obj, PyFrameObject *frame, int what, PyObject *arg)
{
    if (what == PyTrace_CALL) {
        get_utf8(frame->f_code->co_name);
        get_utf8(frame->f_code->co_filename);
        set_top_frame(frame);
    } else if (what == PyTrace_RETURN) {
        set_top_frame(frame->f_back); // does the frame is the about-to-return one?
    }
    return 0;
}

static void
warm_frames(void)
{
    PyThreadState *tstate;
    PyFrameObject *frame;

    /* warm the current interpreter */
    tstate = PyThreadState_Get();
    while (tstate != NULL) {
        frame = _PyThreadState_GetFrame(tstate);
        while (frame != NULL) {
            get_utf8(frame->f_code->co_name);
            get_utf8(frame->f_code->co_filename);
            frame = frame->f_back;
        }
        tstate = PyThreadState_Next(tstate);
    }
}

/* FIXME: report error */
PyObject*
enable_perf(PyObject* self, PyObject* args)
{
    (void) self, (void) args;

    ref += 1;
    PyEval_SetProfile(function_handler, NULL);
    warm_frames();
    Py_RETURN_NONE;
}

PyObject*
disable_perf(PyObject* self, PyObject* args)
{
    (void) self, (void) args;

    PyEval_SetProfile(NULL, NULL);
    Py_RETURN_NONE;
}

static inline void
populate_utf8(PyObject *unicode, char **str, size_t *len)
{
    *str = PyUnicode_UTF8(unicode);
    *len = PyUnicode_UTF8_LENGTH(unicode);
}

static void do_traceback_ust(PyFrameObject *frame)
{
    size_t depth = 0;
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
}

PyObject *
traceback_ust(PyObject* self, PyObject* args)
{
    do_traceback_ust(PyEval_GetFrame());
    Py_RETURN_NONE;
}

static inline int
is_utf8(PyObject *unicode)
{
    return PyUnicode_Check(unicode) &&
            PyUnicode_UTF8(unicode) != NULL &&
            PyUnicode_UTF8_LENGTH(unicode) > 0;
}

PyObject *
is_frame_utf8(PyObject* self, PyObject* args)
{
    PyFrameObject *frame = PyEval_GetFrame();
    while (frame != NULL) {
        if (!(PyFrame_Check(frame)
                && is_utf8(frame->f_code->co_filename)
                && is_utf8(frame->f_code->co_name))) {
            Py_RETURN_FALSE;
        }
        frame = frame->f_back;
    }
    Py_RETURN_TRUE;
}

int event_ob__init(PyPerfEvent *self, PyObject *args, PyObject *kwds)
{
    printf("event_ob__init\n");
    return 0;
}

PyObject *event_ob__open(PyObject *self, PyObject *args)
{
    printf("event_ob__open\n");
    Py_RETURN_NONE;
}

void event_ob__delete(PyPerfEvent *self)
{
    printf("event_ob__delete\n");
}

static PyMethodDef event_ob__methods[] = {
    {
        .ml_name  = "open",
        .ml_meth  = (PyCFunction)event_ob__open,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc   = PyDoc_STR("open the sampling event")
    },
    { .ml_name = NULL, }
};

static char event_ob__doc[] = PyDoc_STR("sampling event object");

PyTypeObject event_ob__type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name    = "sampling.event",
    .tp_basicsize   = sizeof(struct event_ob),
    .tp_dealloc = (destructor)event_ob__delete,
    .tp_flags   = Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,
    .tp_doc     = event_ob__doc,
    .tp_methods = event_ob__methods,
    .tp_init    = (initproc)event_ob__init,
};
