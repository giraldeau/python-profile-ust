#include <Python.h>
#include <frameobject.h>
#include <unicodeobject.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ust.h"
#include "callstack.h"
#include "except.h"

static PyMethodDef PythonProfileLttngMethods[] =
{
    {"enable_callstack",    enable_callstack,   METH_NOARGS,  NULL},
    {"disable_callstack",   disable_callstack,  METH_NOARGS,  NULL},
    {"enable_ust",          enable_ust,         METH_NOARGS,  NULL},
    {"disable_ust",         disable_ust,        METH_NOARGS,  NULL},
    {"set_callback",        set_callback,       METH_VARARGS, NULL},
    {"do_callback",         do_callback,        METH_VARARGS, NULL},
    {"do_raise",            do_raise,           METH_VARARGS, NULL},
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

