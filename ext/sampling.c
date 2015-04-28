/*
 * sampling.c
 *
 *  Created on: Jan 15, 2015
 *      Author: francis
 */

#define _GNU_SOURCE
#include <Python.h>
#include <bytesobject.h>
#include <frameobject.h>
#include <listobject.h>
#include <structmember.h>
#include <unicodeobject.h>

#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <stropts.h>
#include <unistd.h>

#include <sys/select.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

#include "tp.h"
#include "sampling.h"
#include "encode.h"

#define DEPTH_MAX 100

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
static void traceback_full(PyFrameObject *frame);
static inline PyFrameObject *get_top_frame(void);

/*
 * Signal handler
 */

static int hits = 0;

PyPerfEvent *xev = NULL;

static void handle_sigio(int signo, siginfo_t *info, void *data)
{
    ACCESS_ONCE(hits) = hits + 1;
    traceback_full(get_top_frame());
    if (xev) {
        ioctl(xev->fd, PERF_EVENT_IOC_REFRESH, 1);
    }
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

static size_t __do_traceback(PyFrameObject *frame, struct frame *tsf, int max)
{
    size_t depth = 0;
    while (frame != NULL && depth < max) {
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
    return depth;
}

static void traceback_full(PyFrameObject *frame)
{
    void *addr[DEPTH_MAX];
    struct frame tsf[DEPTH_MAX];
    size_t unw_depth = 0;
    size_t depth = 0;

    depth = __do_traceback(frame, tsf, DEPTH_MAX);
    assert(depth <= DEPTH_MAX);
    unw_depth = unw_backtrace(addr, DEPTH_MAX);
    assert(unw_depth <= DEPTH_MAX);
    tracepoint(python, traceback_full, addr, unw_depth, tsf, depth);
}

PyObject *
do_traceback(PyObject* self, PyObject* args)
{
    struct frame tsf[DEPTH_MAX];
    size_t depth = 0;

    depth = __do_traceback(PyEval_GetFrame(), tsf, DEPTH_MAX);
    tracepoint(python, traceback, tsf, depth);
    Py_RETURN_NONE;
}

PyObject *
do_unwind(PyObject* self, PyObject* args)
{
    void *addr[DEPTH_MAX];
    size_t unw_depth = 0;

    unw_depth = unw_backtrace((void **)&addr, DEPTH_MAX);
    assert(unw_depth <= DEPTH_MAX);
    tracepoint(python, unwind, addr, unw_depth);
    Py_RETURN_NONE;
}

PyObject *
do_traceback_full(PyObject* self, PyObject* args)
{
    traceback_full(PyEval_GetFrame());
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
    struct perf_event_attr attr = {
        .size = sizeof(struct perf_event_attr),
    };
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
        if (attr.sample_freq != 0) {
            PyErr_SetString(PyExc_AttributeError, "Event frequency or period required, not both");
            return -1;
        }
        if (freq != 0) {
            PyErr_SetString(PyExc_AttributeError, "sample_period set requires freq equal to zero");
            return -1;
        }
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
    self->status = EVENT_STATUS_CLOSED;
    self->fd = -1;

    return 0;
}

void event_ob__dealloc(PyPerfEvent *self)
{
    Py_TYPE(self)->tp_free((PyObject*)self);
}

PyObject *event_ob__read(PyPerfEvent* self, PyObject* args)
{
    uint64_t val = 0;
    if (self->status == EVENT_STATUS_OPENED) {
        if (read(self->fd, &val, sizeof(val)) < 0) {
            PyErr_SetString(PyExc_IOError, "failed to read perf event counter");
            return NULL;
        }
    }
    return Py_BuildValue("K", val);
}

static PyMemberDef event_ob__members[] = {
    { "type",   T_INT,  offsetof(PyPerfEvent, attr.type),   0, "event type" },
    { "config", T_LONG, offsetof(PyPerfEvent, attr.config), 0, "event config"},
    { "status", T_INT,  offsetof(PyPerfEvent, status),      0, "event status"},
    { "fd",     T_INT,  offsetof(PyPerfEvent, fd),          0, "event file descriptor"},
    { NULL }  /* Sentinel */
};

static PyMethodDef event_ob__methods[] = {
    {
        .ml_name = "read",
        .ml_meth = (PyCFunction)event_ob__read,
        .ml_flags = METH_NOARGS,
        .ml_doc = PyDoc_STR("read counter value"),
    },
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

#define PyPerfEvent_CheckExact(op) (Py_TYPE(op) == &event_ob__type)

/*
 * Sampling module
 */

struct event_status_decl event_status__constants[] = {
    { "EVENT_STATUS_OPENED", EVENT_STATUS_OPENED },
    { "EVENT_STATUS_CLOSED", EVENT_STATUS_CLOSED },
    { "EVENT_STATUS_FAILED", EVENT_STATUS_FAILED },
    { .name = NULL },
};

static PyObject *evseq;

static pid_t gettid(void)
{
    return syscall(SYS_gettid);
}

static long sys_perf_event_open(struct perf_event_attr *attr,
                    pid_t pid, int cpu, int group_fd,
                    unsigned long flags)
{
  return syscall(__NR_perf_event_open, attr, pid, cpu,
                 group_fd, flags);
}

static int sampling_do_close(PyObject *obj)
{
    PyPerfEvent *ev;
    if (!PyPerfEvent_CheckExact(obj))
        return -1;
    ev = (PyPerfEvent *)obj;
    if (ev->status == EVENT_STATUS_OPENED) {
        close(ev->fd);
        ev->status = EVENT_STATUS_CLOSED;
    }
    return 0;
}

static int sampling_do_open(PyObject *obj)
{
    int tid;
    int flags;
    PyPerfEvent *ev;
    struct f_owner_ex fown;
    struct perf_event_attr attr;

    if (!PyPerfEvent_CheckExact(obj))
        return -1;
    ev = (PyPerfEvent *)obj;
    tid = gettid();

    /* Open the perf event */
    attr = ev->attr;
    attr.disabled = 0; /* do not start the event yet */
    attr.watermark = 1;
    attr.wakeup_events = 1;
    ev->fd = sys_perf_event_open(&attr, tid, -1, -1, 0);
    if (ev->fd < 0) {
        ev->status = EVENT_STATUS_FAILED;
        return -1;
    }

    /* Configure fasync */
    fown.type = F_OWNER_TID;
    fown.pid = tid;
    if (fcntl(ev->fd, F_SETOWN_EX, &fown) < 0)
        goto fail;

    flags = fcntl(ev->fd, F_GETFL);
    if (fcntl(ev->fd, F_SETFL, flags | FASYNC | O_ASYNC) < 0)
        goto fail;

    ev->status = EVENT_STATUS_OPENED;
    xev = ev;
    __sync_synchronize();
    ioctl(xev->fd, PERF_EVENT_IOC_REFRESH, 1);
    ioctl(xev->fd, PERF_EVENT_IOC_ENABLE, 0);
    return 0;

fail:
    close(ev->fd);
    ev->fd = -1;
    ev->status = EVENT_STATUS_FAILED;
    PyErr_SetString(PyExc_RuntimeError, "fcntl() failed");
    return -1;
}

static int sampling_setup_sighandler(void *action, int check)
{
    struct sigaction sigact, oldsigact;
    sigset_t set;

    /* check if the signal is blocked */
    if (check) {
        sigemptyset(&set);
        pthread_sigmask(SIG_BLOCK, NULL, &set);
        if (sigismember(&set, SIGIO)) {
            PyErr_SetString(PyExc_RuntimeError, "signal SIGIO is blocked");
            return -1;
        }
    }

    /* install handler */
    sigact.sa_sigaction = action;
    sigact.sa_flags = SA_SIGINFO;
    if (sigaction(SIGIO, &sigact, &oldsigact) < 0) {
        PyErr_SetString(PyExc_RuntimeError, "sigaction() failed");
        return -1;
    }
    if (check && !(oldsigact.sa_sigaction == (void *)SIG_DFL ||
            oldsigact.sa_sigaction == (void *)SIG_IGN)) {
        PyErr_Warn(PyExc_RuntimeWarning, "perf sampling overwrites SIGIO handler");
    }
    return 0;
}

PyObject *sampling__close(PyObject* self, PyObject* args);

PyObject *sampling__open(PyObject* self, PyObject* args)
{
    int i;
    int len;
    PyObject *obj = NULL;
    PyObject *seq = NULL;


    if (!PyArg_ParseTuple(args, "O:events", &obj))
        Py_RETURN_NONE;

    if (PyPerfEvent_CheckExact(obj)) {
        obj = PyTuple_Pack(1, obj);
    } else {
        Py_INCREF(obj);
    }
    if (!(seq = PySequence_Fast(obj, "expected a sequence")))
        return NULL;

    sampling__close(self, NULL);

    if (sampling_setup_sighandler(handle_sigio, 1) < 0)
        return NULL;

    /*
     * FIXME: use the first fd as the group_fd, control tracing with it.
     */
    len = PySequence_Size(obj);
    for (i = 0; i < len; i++) {
        PyObject *ev = PySequence_Fast_GET_ITEM(seq, i);
        sampling_do_open(ev);
    }
    Py_DECREF(seq);
    assert(seq->ob_refcnt > 0);
    evseq = seq;
    __sync_synchronize();

    Py_RETURN_NONE;
}

PyObject *sampling__close(PyObject* self, PyObject* args)
{
    int i;
    int len;
    PyObject *ev;

    if (!evseq)
        Py_RETURN_NONE;
    len = PySequence_Size(evseq);
    for (i = 0; i < len; i++) {
        ev = PySequence_Fast_GET_ITEM(evseq, i);
        sampling_do_close(ev);
    }
    sampling_setup_sighandler(SIG_DFL, 0);
    Py_DECREF(evseq);
    evseq = NULL;
    Py_RETURN_NONE;
}

int sampling__change_state(int op)
{
    int i;
    int len;
    PyObject *obj;

    if (!evseq)
        return 0;
    if (op != PERF_EVENT_IOC_ENABLE && op != PERF_EVENT_IOC_DISABLE) {
        PyErr_SetString(PyExc_RuntimeError, "Unkown ioctl");
        return -1;
    }
    len = PySequence_Size(evseq);
    for (i = 0; i < len; i++) {
        obj = PySequence_Fast_GET_ITEM(evseq, i);
        if (PyPerfEvent_CheckExact(obj)) {
            PyPerfEvent *ev = (PyPerfEvent *)obj;
            if (ev->status == EVENT_STATUS_OPENED) {
                ioctl(ev->fd, op, 0);
            }
        }
    }
    return 0;
}

PyObject *sampling__enable(PyObject* self, PyObject* args)
{
    if (sampling__change_state(PERF_EVENT_IOC_ENABLE) < 0)
        return NULL;
    Py_RETURN_NONE;
}

PyObject *sampling__disable(PyObject* self, PyObject* args)
{
    if (sampling__change_state(PERF_EVENT_IOC_DISABLE) < 0)
        return NULL;
    Py_RETURN_NONE;
}

PyObject *sampling__hits(PyObject* self, PyObject* args)
{
    return Py_BuildValue("i", hits);
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
    {
        .ml_name = "hits",
        .ml_meth = sampling__hits,
        .ml_flags = METH_NOARGS,
        .ml_doc = PyDoc_STR("signal handler hits"),
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
