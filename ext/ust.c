/*
 * ust.c
 *
 *  Created on: Dec 5, 2014
 *      Author: francis
 */
#include <Python.h>
#include <frameobject.h>
#include <unicodeobject.h>

#include "tp.h"
#include "ust.h"
#include "encode.h"

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
    const char *name;
    switch(what) {
    case PyTrace_CALL:
        name = get_utf8(frame->f_code->co_name);
        tracepoint(python, call, name);
        break;
    case PyTrace_EXCEPTION: /* the event 'exception' is not triggered by CPython */
    case PyTrace_LINE:
        break;
    case PyTrace_RETURN:
        tracepoint(python, return);
        break;
    case PyTrace_C_CALL:
        if (PyCFunction_Check(arg)) {
            name = ((PyCFunctionObject *)arg)->m_ml->ml_name;
            tracepoint(python, c_call, name);
        }
        break;
    case PyTrace_C_EXCEPTION:
        // TODO: handle c_exception event
        break;
    case PyTrace_C_RETURN:
        tracepoint(python, c_return);
        break;
    default:
        break;
    }
    return 0;
}
