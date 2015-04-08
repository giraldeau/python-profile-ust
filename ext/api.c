#include <Python.h>
#include <frameobject.h>
#include <unicodeobject.h>
#include <dictobject.h>
#include <longobject.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ust.h"
#include "sampling.h"
#include "clock.h"
#include "except.h"

static PyMethodDef PythonProfileLttngMethods[] =
{
    {"enable_perf",         enable_perf,        METH_NOARGS,  NULL},
    {"disable_perf",        disable_perf,       METH_NOARGS,  NULL},
    {"enable_ust",          enable_ust,         METH_NOARGS,  NULL},
    {"disable_ust",         disable_ust,        METH_NOARGS,  NULL},
    {"set_callback",        set_callback,       METH_VARARGS, NULL},
    {"do_callback",         do_callback,        METH_VARARGS, NULL},
    {"do_raise",            do_raise,           METH_VARARGS, NULL},
    {"traceback_ust",       traceback_ust,		METH_VARARGS, NULL},
    {"is_frame_utf8",       is_frame_utf8,      METH_VARARGS, NULL},
    {"clock_gettime",       clock_gettime_wrapper,     METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "api",
        NULL,
        0,
        PythonProfileLttngMethods,
        NULL,
        NULL,
        NULL,
        NULL
};


/*
 * Module initialization copied in large part from
 * linux/tools/perf/util/python.c
 */

PyMODINIT_FUNC
PyInit_api(void)
{
    int i;
    PyObject *dict;
    PyObject *submod;
    PyObject *obj;
    PyObject *module = PyModule_Create(&moduledef);
    if (module == NULL)
        goto error;

    submod = PyModule_Create(&sampling__moduledef);
    if (submod == NULL)
        goto error;

    PyModule_AddObject(module, "sampling", (PyObject*)submod);

    /* add const to the module */
    dict = PyModule_GetDict(submod);
    if (dict == NULL)
        goto error;

    for (i = 0; perf__constants[i].name != NULL; i++) {
        obj = PyLong_FromLong(perf__constants[i].value);
        if (obj == NULL)
            goto error;
        PyDict_SetItemString(dict, perf__constants[i].name, obj);
        Py_DECREF(obj);
    }

    for (i = 0; event_status__constants[i].name != NULL; i++) {
        obj = PyLong_FromLong(event_status__constants[i].value);
        if (obj == NULL)
            goto error;
        PyDict_SetItemString(dict, event_status__constants[i].name, obj);
        Py_DECREF(obj);
    }

    /* event type */
    event_ob__type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&event_ob__type) < 0)
        goto error;

    Py_INCREF(&event_ob__type);
    PyModule_AddObject(submod, "Event", (PyObject *)&event_ob__type);

    return module;

error:
    if (PyErr_Occurred())
        PyErr_SetString(PyExc_ImportError, "linuxProfile: init failed");
    return NULL;
}
