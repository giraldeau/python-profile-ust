/*
 * ust.c
 *
 *  Created on: Dec 5, 2014
 *      Author: francis
 */
#include <Python.h>
#include <frameobject.h>
#include <unicodeobject.h>

#define TRACEPOINT_DEFINE
#define TRACEPOINT_CREATE_PROBES
#include "provider.h"

#include "ust.h"

PyObject*
enable_ust(PyObject* self, PyObject* args)
{
    (void) self, (void) args;
    PyEval_SetProfile(profile_func_ust, NULL);
    Py_RETURN_NONE;
}

PyObject*
disable_ust(PyObject* self, PyObject* args)
{
    (void) self, (void) args;
    PyEval_SetProfile(NULL, NULL);
    Py_RETURN_NONE;
}

int
profile_func_ust(PyObject *obj, PyFrameObject *frame, int what, PyObject *arg)
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
