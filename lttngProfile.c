#include <Python.h>
#include <frameobject.h>
#include <unicodeobject.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#define TRACEPOINT_DEFINE
#include "provider.h"

/*
def trace_dispatch(frame, event, arg):
    print("trace_dispatch")
    fco = frame.f_code
    pprint.pprint(fco.co_filename)
    pprint.pprint(fco.co_firstlineno)
    pprint.pprint(fco.co_name)
    pprint.pprint(event)
    pprint.pprint(arg)
 */

static PyObject*
python_profile_ust_callback(PyObject* self, PyObject* args)
{
    PyObject *frame, *event, *arg;
    PyFrameObject *frameobj;
    char *co_name;

    /*
    <class 'frame'>
    <class 'str'>
    <class 'builtin_function_or_method'>

+    const char *filename;
+    const char *fname;
+    int lineno;
+
+    filename = PyString_AsString(f->f_code->co_filename);
+    fname = PyString_AsString(f->f_code->co_name);
+    lineno = PyCode_Addr2Line(f->f_code, f->f_lasti);
     */

    if (!PyArg_UnpackTuple(args, "ust", 3, 3, &frame, &event, &arg))
        return NULL;

    if (!PyFrame_Check(frame))
        return NULL;

    setlocale(LC_ALL, "");
    frameobj = (PyFrameObject *) frame;
    co_name = (char *) PyUnicode_DATA(frameobj->f_code->co_name);
    printf("co_name=%s (%ld)\n", co_name, strlen(co_name));

    tracepoint(python, call, co_name);
    Py_RETURN_NONE;
}

static PyMethodDef PythonProfileLttngMethods[] =
{
    {"callback", python_profile_ust_callback, METH_VARARGS, NULL},
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

