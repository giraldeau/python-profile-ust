/*
 * callstack.c
 *
 *  Created on: Dec 5, 2014
 *      Author: francis
 */

#include <Python.h>

PyObject*
enable_callstack(PyObject* self, PyObject* args)
{
    (void) self, (void) args;
    PyEval_SetProfile(NULL, NULL);
    Py_RETURN_NONE;
}

PyObject*
disable_callstack(PyObject* self, PyObject* args)
{
    (void) self, (void) args;
    PyEval_SetProfile(NULL, NULL);
    Py_RETURN_NONE;
}
