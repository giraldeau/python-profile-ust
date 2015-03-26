#include <Python.h>
#include <frameobject.h>
#include <unicodeobject.h>

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

PyMODINIT_FUNC
PyInit_api(void)
{
    PyObject *module = PyModule_Create(&moduledef);
    return module;
}

