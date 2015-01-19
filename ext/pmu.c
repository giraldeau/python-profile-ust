/*
 * pmu.c
 *
 *  Created on: Jan 15, 2015
 *      Author: francis
 */
#include <Python.h>
#include <frameobject.h>
#include <unicodeobject.h>

#include "tp.h"
#include "pmu.h"

PyObject*
enable_perf(PyObject* self, PyObject* args)
{
    Py_RETURN_NONE;
}

PyObject*
disable_perf(PyObject* self, PyObject* args)
{
    Py_RETURN_NONE;
}

PyObject *
ust_traceback(PyObject* self, PyObject* args)
{
	PyFrameObject *frame;
	PyCodeObject *code;
	PyObject *str;
	const char *name;
	int lineno;
	unsigned int depth;

	printf("ust_traceback entry\n");
	frame = PyEval_GetFrame();
	printf("frame=%p\n", frame);
	while (frame != NULL) {
		if (!PyFrame_Check(frame))
			break;
		printf("frame %d\n", depth);
		printf("f_lasti %d\n", frame->f_lasti);
		code = frame->f_code;
		if (code != NULL) {
			if (code->co_filename && PyUnicode_Check(code->co_filename)) {
				str = PyUnicode_AsUTF8String(code->co_filename);
				name = PyBytes_AsString(str);
				lineno = PyFrame_GetLineNumber(frame);
				printf("co_filename %s:%d\n", name, lineno);
			}
			if (code->co_name != NULL && PyUnicode_Check(code->co_name)) {
				str = PyUnicode_AsUTF8String(code->co_name);
				name = PyBytes_AsString(str);
				printf("co_name %s\n", name);
			}
		}
		frame = frame->f_back;
		depth++;
	}
	printf("ust_traceback exit\n");
	Py_RETURN_NONE;
}
