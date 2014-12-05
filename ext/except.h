/*
 * except.h
 *
 *  Created on: Dec 5, 2014
 *      Author: francis
 */

#ifndef EXCEPT_H_
#define EXCEPT_H_

extern PyObject* set_callback(PyObject *self, PyObject *args);
extern PyObject* do_callback(PyObject* self, PyObject* args);
extern PyObject* do_raise(PyObject* self, PyObject* args);

#endif /* EXCEPT_H_ */
