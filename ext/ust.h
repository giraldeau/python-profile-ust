/*
 * ust.h
 *
 *  Created on: Dec 5, 2014
 *      Author: francis
 */

#ifndef UST_H_
#define UST_H_

extern PyObject* enable_ust(PyObject* self, PyObject* args);
extern PyObject* disable_ust(PyObject* self, PyObject* args);
int profile_func_ust(PyObject *obj, PyFrameObject *frame, int what, PyObject *arg);

#endif /* UST_H_ */
