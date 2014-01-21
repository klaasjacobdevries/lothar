#ifndef PYLOTHAR_H
#define PYLOTHAR_H

#include <Python.h>
#include "lothar.h"

void pylothar_raise_exception(int error);
void pylothar_raise_exception_global(void);

PyMODINIT_FUNC pylothar_initconnection(PyObject *module);
PyMODINIT_FUNC pylothar_initmotor(PyObject *module);
PyMODINIT_FUNC pylothar_initsensor(PyObject *module);
PyMODINIT_FUNC pylothar_initscheduler(PyObject *module);
PyMODINIT_FUNC pylothar_initsteering(PyObject *module);

PyObject *pylothar_check_return(int status);

typedef struct
{
  PyObject_HEAD
  lothar_connection_t *d_connection;

  PyObject *d_custom;
} connection_t;

extern PyTypeObject pylothar_connection_type;

#endif
