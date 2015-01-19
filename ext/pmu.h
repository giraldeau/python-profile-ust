/*
 * pmu.h
 *
 *  Created on: Jan 15, 2015
 *      Author: francis
 */

#ifndef PMU_H_
#define PMU_H_

extern PyObject* enable_perf(PyObject* self, PyObject* args);
extern PyObject* disable_perf(PyObject* self, PyObject* args);
extern PyObject* ust_traceback(PyObject* self, PyObject *args);

#endif /* PMU_H_ */
