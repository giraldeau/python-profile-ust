#include <Python.h>
#include <frameobject.h>
#include <unicodeobject.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#define TRACEPOINT_DEFINE
#define TRACEPOINT_CREATE_PROBES
#include "provider.h"

int
trace_func(PyObject *obj, PyFrameObject *frame, int what, PyObject *arg)
{
    PyObject *str;
    const char *name;
    switch(what) {
    case 0:
        str = PyUnicode_AsUTF8String(frame->f_code->co_name);
        name = PyBytes_AsString(str);
        tracepoint(python, call, name);
        break;
    case 1: /* the event 'exception' is not triggered by CPython */
    case 2:
        break;
    case 3:
        tracepoint(python, return);
        break;
    case 4:
        if (PyCFunction_Check(arg)) {
            name = ((PyCFunctionObject *)arg)->m_ml->ml_name;
            tracepoint(python, c_call, name);
        }
        break;
    case 5:
        // TODO: handle c_exception event
        break;
    case 6:
        tracepoint(python, c_return);
        break;
    default:
        break;
    }
    return 0;
}

/*
 * Following code based on
 * https://docs.python.org/2.5/ext/callingPython.html
 */
static PyObject *test_callback = NULL;

static PyObject*
set_callback(PyObject *self, PyObject *args)
{
    PyObject *fn;

    if (PyArg_UnpackTuple(args, "test", 1, 1, &fn)) {
        if (!PyCallable_Check(fn)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
        }
        Py_XINCREF(fn);
        Py_XDECREF(test_callback);
        test_callback = fn;
        Py_RETURN_NONE;
    }
    return NULL;
}

static PyObject*
do_callback(PyObject* self, PyObject* args)
{
    PyObject *tuple;
    PyObject *ret;

    tuple = PyTuple_New(0);
    ret = PyEval_CallObject(test_callback, tuple);
    Py_DECREF(tuple);
    if (ret == NULL)
        return NULL;
    Py_DECREF(ret);
    Py_RETURN_NONE;
}

static PyObject*
do_raise(PyObject* self, PyObject* args)
{
    (void) self, (void) args;
    return NULL;
}

static PyObject*
enable_ust(PyObject* self, PyObject* args)
{
    (void) self, (void) args;
    PyEval_SetProfile(trace_func, NULL);
    Py_RETURN_NONE;
}

static PyObject*
disable_ust(PyObject* self, PyObject* args)
{
    (void) self, (void) args;
    PyEval_SetProfile(NULL, NULL);
    Py_RETURN_NONE;
}

static PyMethodDef PythonProfileLttngMethods[] =
{
    {"enable",          enable_ust,     METH_NOARGS,  NULL},
    {"disable",         disable_ust,    METH_NOARGS,  NULL},
    {"set_callback",    set_callback,   METH_VARARGS, NULL},
    {"do_callback",     do_callback,    METH_VARARGS, NULL},
    {"do_raise",        do_raise,       METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "lttngProfile",
        NULL,
        0,
        PythonProfileLttngMethods,
        NULL,
        NULL,
        NULL,
        NULL
};

PyMODINIT_FUNC
PyInit_lttngProfile(void)
{
    PyObject *module = PyModule_Create(&moduledef);
    return module;
}

