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

/*
 * FIXME: This is silly... Is there a way to get the object type?
 */
void show_type(char *name, PyObject *obj)
{
    char *type;
    if (PyByteArray_Check(obj)) {
        type = "bytearray";
    } else if (PyBytes_Check(obj)) {
        type = "bytes";
    } else if (PyUnicode_Check(obj)) {
        type = "unicode";
    } else if (PyUnicode_Check(obj)) {
        type = "long";
    } else if (PyBool_Check(obj)) {
        type = "bool";
    } else if (PyFloat_Check(obj)) {
        type = "float";
    } else if (PyComplex_Check(obj)) {
        type = "complex";
    } else if (PyRange_Check(obj)) {
        type = "range";
    } else if (PyMemoryView_Check(obj)) {
        type = "memory";
    } else if (PyTuple_Check(obj)) {
        type = "tuple";
    } else if (PyList_Check(obj)) {
        type = "list";
    } else if (PyDict_Check(obj)) {
        type = "dict";
    } else if (PySet_Check(obj)) {
        type = "set";
    } else if (PyFunction_Check(obj)) {
        type = "function";
    } else if (PyCFunction_Check(obj)) {
        type = "cfunction";
    } else if (PyModule_Check(obj)) {
        type = "module";
    } else if (PyMethod_Check(obj)) {
        type = "method";
    } else if (PyInstanceMethod_Check(obj)) {
        type = "instancemethod";
    } else if (PyCapsule_CheckExact(obj)) {
        type = "capsule";
    } else if (PyTraceBack_Check(obj)) {
        type = "traceback";
    } else if (PySlice_Check(obj)) {
        type = "slice";
    } else if (PyCell_Check(obj)) {
        type = "cell";
    } else if (PySeqIter_Check(obj)) {
        type = "iter";
    } else if (PyGen_Check(obj)) {
        type = "gen";
    } else if (PyWeakref_Check(obj)) {
        type = "weakref";
    } else if (PyFrame_Check(obj)) {
        type = "frame";
    } else if (Py_None == obj) {
        type = "none";
    } else {
        type = "unkown";
    }
    printf("typeof(%s)=%s\n", name, type);
}

static PyObject*
python_ust(PyObject* self, PyObject* args)
{
    PyObject *frame, *event, *arg;
    PyFrameObject *frameobj;
    PyCFunctionObject *cfunc;
    char *frame_name, *ev_name;
    const char *cfunc_name = "none";

    if (!PyArg_UnpackTuple(args, "ust", 3, 3, &frame, &event, &arg))
        return NULL;

    if (!PyFrame_Check(frame))
        return NULL;

    /*
    show_type("frame", frame);
    show_type("event", event);
    show_type("arg", arg);
    */

    frameobj = (PyFrameObject *)frame;

    frame_name = PyBytes_AsString(PyUnicode_AsUTF8String(frameobj->f_code->co_name));
    ev_name = PyBytes_AsString(PyUnicode_AsUTF8String(event));

    if (PyCFunction_Check(arg)) {
        cfunc = (PyCFunctionObject *) arg;
        cfunc_name = cfunc->m_ml->ml_name;
    }

    printf("frame=%s event=%s cfunc=%s\n",
            frame_name, ev_name, cfunc_name);

    if (!PyFrame_Check(frame))
        return NULL;

    Py_RETURN_NONE;
}

/*
#define PyTrace_CALL 0
#define PyTrace_EXCEPTION 1
#define PyTrace_LINE 2
#define PyTrace_RETURN 3
#define PyTrace_C_CALL 4
#define PyTrace_C_EXCEPTION 5
#define PyTrace_C_RETURN 6
*/

static const char *what_names[] = {
    "call", "exception", "line", "return",
    "c_call", "c_exception", "c_return"
};

int
trace_func(PyObject *obj, PyFrameObject *frame, int what, PyObject *arg)
{
    PyObject *str;
    const char *name;
    printf("what=%s\n", what_names[what]);
    switch(what) {
    case 0:
        str = PyUnicode_AsUTF8String(frame->f_code->co_name);
        name = PyBytes_AsString(str);
        tracepoint(python, call, name);
        break;
    case 1:
        // TODO: handle exception event
    case 2:
        break;
    case 3:
        tracepoint(python, return);
        break;
    case 4:
        show_type("arg", arg);
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
        printf("test_callback=%p\n", fn);
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
    {"ust",             python_ust,     METH_VARARGS, NULL},
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

