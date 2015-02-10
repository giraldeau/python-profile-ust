/*
 * clock.c
 *
 *  Created on: Feb 10, 2015
 *      Author: francis
 */

#include <Python.h>
#include <time.h>

PyObject *
clock_gettime_wrapper(PyObject* self, PyObject* args)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return Py_BuildValue("(HH)", ts.tv_sec, ts.tv_nsec);
}
