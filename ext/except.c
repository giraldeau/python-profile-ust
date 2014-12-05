/*
 * except.c
 *
 *  Created on: Dec 5, 2014
 *      Author: francis
 */

#include <Python.h>

/*
 * Following code based on
 * https://docs.python.org/2.5/ext/callingPython.html
 */
static PyObject *test_callback = NULL;

PyObject*
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

PyObject*
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

PyObject*
do_raise(PyObject* self, PyObject* args)
{
    (void) self, (void) args;
    return NULL;
}
