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
#include "encode.h"

#define DEPTH_MAX 100
static __thread struct frame tsf[DEPTH_MAX];

/*
 * We don't use TLS here, seems to possible to access a thread data from
 * another thread, nor with pthread_getspecific().
 *
 * Instead, we use a counter for the number of enable/disable. If the gen
 * variable is lower than the current ref variable, then the frame may not be
 * valid.
 */

static __thread PyFrameObject *top;
static __thread int gen;
static int ref = 0;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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

static struct perfuser conf = {
    .signum = SIGUSR1,
    .sample = handle_signal,
    .data = NULL,
};


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
    int ret;

    pthread_mutex_lock(&lock);
    ref += 1;
    PyEval_SetProfile(function_handler, NULL);
    warm_frames();
    ret = perfuser_register(&conf);
    printf("perfuser_register=%d\n", ret);
    if (ret < 0) {
        PyEval_SetProfile(NULL, NULL);
    }
    pthread_mutex_unlock(&lock);
    Py_RETURN_NONE;
}

PyObject*
disable_perf(PyObject* self, PyObject* args)
{
    (void) self, (void) args;

    pthread_mutex_lock(&lock);
    int ret = perfuser_unregister();
    printf("perfuser_unregister=%d\n", ret);
    PyEval_SetProfile(NULL, NULL);
    pthread_mutex_unlock(&lock);
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
