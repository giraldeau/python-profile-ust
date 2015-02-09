/*
 * encode.c
 *
 *  Created on: Feb 9, 2015
 *      Author: francis
 */
#include <Python.h>
#include <unicodeobject.h>

#include "encode.h"

/*
 * Return the UTF-8 string from the unicode object. Uses the cached string if
 * defined, or trigger the encoding. Therefore, this function is not signal
 * safe.
 *
 * Returns the char buffer, NULL if failed
 */
const char *get_utf8(PyObject *unicode)
{
    char *name = NULL;
    if (PyUnicode_Check(unicode)) {
        name = PyUnicode_UTF8(unicode);
        if (name == NULL) {
            PyUnicode_AsUTF8String(unicode);
            name = PyUnicode_UTF8(unicode);
        }
    }
    return name;
}
