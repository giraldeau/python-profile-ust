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
#include <structmember.h>
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

/*
 * Event object type for perf event attributes
 */

int event_ob__init(PyPerfEvent *self, PyObject *args, PyObject *kwargs)
{
    struct perf_event_attr attr;
    static char *kwlist[] = {
            "type",
            "config",
            "sample_freq",
            "sample_period",
            "sample_type",
            "read_format",
            "disabled",
            "inherit",
            "pinned",
            "exclusive",
            "exclude_user",
            "exclude_kernel",
            "exclude_hv",
            "exclude_idle",
            "mmap",
            "comm",
            "freq",
            "inherit_stat",
            "enable_on_exec",
            "task",
            "watermark",
            "precise_ip",
            "mmap_data",
            "sample_id_all",
            "wakeup_events",
            "bp_type",
            "bp_addr",
            "bp_len",
            NULL
    };
    uint64_t sample_period = 0;
    uint32_t disabled = 0,
            inherit = 0,
            pinned = 0,
            exclusive = 0,
            exclude_user = 0,
            exclude_kernel = 0,
            exclude_hv = 0,
            exclude_idle = 0,
            mmap = 0,
            comm = 0,
            freq = 1,
            inherit_stat = 0,
            enable_on_exec = 0,
            task = 0,
            watermark = 0,
            precise_ip = 0,
            mmap_data = 0,
            sample_id_all = 1;
    int idx = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
            "|iKiKKiiiiiiiiiiiiiiiiiiiiiKK", kwlist,
            &attr.type, &attr.config, &attr.sample_freq,
            &sample_period, &attr.sample_type,
            &attr.read_format, &disabled, &inherit,
            &pinned, &exclusive, &exclude_user,
            &exclude_kernel, &exclude_hv, &exclude_idle,
            &mmap, &comm, &freq, &inherit_stat,
            &enable_on_exec, &task, &watermark,
            &precise_ip, &mmap_data, &sample_id_all,
            &attr.wakeup_events, &attr.bp_type,
            &attr.bp_addr, &attr.bp_len, &idx))
        return -1;

    /* union... */
    if (sample_period != 0) {
        if (attr.sample_freq != 0)
            return -1; /* FIXME: throw right exception */
        attr.sample_period = sample_period;
    }

    /* Bitfields */
    attr.disabled       = disabled;
    attr.inherit        = inherit;
    attr.pinned         = pinned;
    attr.exclusive      = exclusive;
    attr.exclude_user   = exclude_user;
    attr.exclude_kernel = exclude_kernel;
    attr.exclude_hv     = exclude_hv;
    attr.exclude_idle   = exclude_idle;
    attr.mmap           = mmap;
    attr.comm           = comm;
    attr.freq           = freq;
    attr.inherit_stat   = inherit_stat;
    attr.enable_on_exec = enable_on_exec;
    attr.task           = task;
    attr.watermark      = watermark;
    attr.precise_ip     = precise_ip;
    attr.mmap_data      = mmap_data;
    attr.sample_id_all  = sample_id_all;

    self->attr = attr;

    return 0;
}

void event_ob__dealloc(PyPerfEvent *self)
{
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyMemberDef event_ob__members[] = {
    { "type",   T_INT, offsetof(PyPerfEvent, attr.type), 0, "event type" },
    { "config",  T_LONG, offsetof(PyPerfEvent, attr.config), 0, "event config"},
    { NULL }  /* Sentinel */
};

static PyMethodDef event_ob__methods[] = {
    { .ml_name = NULL, }
};

static char event_ob__doc[] = PyDoc_STR("sampling event object");

PyTypeObject event_ob__type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name    = "sampling.Event",
    .tp_basicsize   = sizeof(struct event_ob),
    .tp_dealloc = (destructor)event_ob__dealloc,
    .tp_flags   = Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,
    .tp_doc     = event_ob__doc,
    .tp_methods = event_ob__methods,
    .tp_members = event_ob__members,
    .tp_init    = (initproc)event_ob__init,
};

/*
 * Sampling module
 */

PyObject *sampling__open(PyObject* self, PyObject* args)
{
    /* Open counter file descriptor */
    /* sys_perf_event_open(attrs, pid, cpu, group_fd, flags); */
    /* Configure fasync */
    Py_RETURN_NONE;
}

PyObject *sampling__close(PyObject* self, PyObject* args)
{
    /* Close file descriptors */
    Py_RETURN_NONE;
}

PyObject *sampling__enable(PyObject* self, PyObject* args)
{
    /* ioctl(fd, PERF_EVENT_IOC_ENABLE); */
    Py_RETURN_NONE;
}

PyObject *sampling__disable(PyObject* self, PyObject* args)
{
    /* ioctl(fd, PERF_EVENT_IOC_DISABLE); */
    Py_RETURN_NONE;
}

static PyMethodDef sampling__methods[] = {
{
        .ml_name = "open",
        .ml_meth = sampling__open,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc = PyDoc_STR("open events"),
    },
    {
        .ml_name = "close",
        .ml_meth = sampling__close,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc = PyDoc_STR("close events"),
    },
    {
        .ml_name = "enable",
        .ml_meth = sampling__enable,
        .ml_flags = METH_NOARGS,
        .ml_doc = PyDoc_STR("enable events"),
    },
    {
        .ml_name = "disable",
        .ml_meth = sampling__disable,
        .ml_flags = METH_NOARGS,
        .ml_doc = PyDoc_STR("disable events"),
    },
    { .ml_name = NULL, }
};


PyModuleDef sampling__moduledef = {
        PyModuleDef_HEAD_INIT,
        "sampling",
        NULL,
        0,
        sampling__methods,
        NULL,
        NULL,
        NULL,
        NULL
};
