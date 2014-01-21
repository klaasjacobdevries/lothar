#include "pylothar.h"

/* creation and destruction */

typedef struct
{
  PyObject_HEAD
  lothar_sensor_t *d_sensor;

  PyObject *d_conn;
  
} sensor_t;

static PyObject *sensor_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  sensor_t *self = (sensor_t *)type->tp_alloc(type, 0);
  self->d_sensor = NULL;
  self->d_conn   = NULL;

  return (PyObject *)self;
}

static int sensor_init(sensor_t *self, PyObject *args, PyObject *kwds)
{
  PyObject *connection;
  int port;
  int type;

  static char *kwlist[] = {"connection", "port", "type", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "Oii", kwlist, &connection, &port, &type))
    return -1;

  if(!PyObject_IsInstance(connection, (PyObject *)&pylothar_connection_type))
  {
    PyErr_BadArgument();
    return -1;
  }

  self->d_sensor = lothar_sensor_open(((connection_t *)connection)->d_connection, port, type);

  if(!self->d_sensor)
  {
    pylothar_raise_exception_global();
    return -1;
  }

  self->d_conn = connection;
  Py_INCREF(self->d_conn);

  return 0;
}

static void sensor_dealloc(sensor_t *self)
{
  if(self->d_sensor)
    lothar_sensor_close(&self->d_sensor);

  if(self->d_conn)
  {
    Py_DECREF(self->d_conn);
  }

  self->ob_type->tp_free((PyObject *)self);
}

/* methods */

static PyObject *sensor_port(sensor_t *self, PyObject *arg)
{
  enum lothar_input_port port;
  int status;
  if((status = lothar_sensor_port(self->d_sensor, &port)))
  {
    pylothar_raise_exception(status);
    return NULL;
  }

  return PyInt_FromLong(port);
}

static PyObject *sensor_connection(sensor_t *self, PyObject *arg)
{
  Py_INCREF(self->d_conn);
  return self->d_conn;
}

static PyObject *sensor_type(sensor_t *self, PyObject *arg)
{
  int status;
  enum lothar_sensor_type type;
  
  if((status = lothar_sensor_type(self->d_sensor, &type)))
  {
    pylothar_raise_exception(status);
    return NULL;
  }

  return PyInt_FromLong(type);
}

static PyObject *sensor_reset(sensor_t *self, PyObject *arg)
{
  if(!PyInt_Check(arg))
  {
    PyErr_BadArgument();
    return NULL;
  }

  return pylothar_check_return(lothar_sensor_reset(self->d_sensor, PyInt_AsLong(arg)));
}

static PyObject *sensor_stop(sensor_t *self, PyObject *arg)
{
  return pylothar_check_return(lothar_sensor_stop(self->d_sensor));
}

static PyObject *sensor_value(sensor_t *self, PyObject *arg)
{
  uint16_t val;
  int status = lothar_sensor_value(self->d_sensor, &val);

  if(status)
  {
    pylothar_raise_exception(status);
    return NULL;
  }

  return PyInt_FromLong(val);
}

static PyObject *sensor_minimum(sensor_t *self, PyObject *arg)
{
  uint16_t min;
  int status = lothar_sensor_minimum(self->d_sensor, &min);

  if(status)
  {
    pylothar_raise_exception(status);
    return NULL;
  }

  return PyInt_FromLong(min);
}

static PyObject *sensor_maximum(sensor_t *self, PyObject *arg)
{
  uint16_t max;
  int status = lothar_sensor_maximum(self->d_sensor, &max);

  if(status)
  {
    pylothar_raise_exception(status);
    return NULL;
  }

  return PyInt_FromLong(max);
}

static PyObject *sensor_val(sensor_t *self, PyObject *arg)
{
  int32_t status = lothar_sensor_val(self->d_sensor);
  
  if(status < 0)
  {
    pylothar_raise_exception(-status);
    return NULL;
  }

  return PyInt_FromLong(status);
}

static PyObject *sensor_min(sensor_t *self, PyObject *arg)
{
  int32_t status = lothar_sensor_min(self->d_sensor);
  
  if(status < 0)
  {
    pylothar_raise_exception(-status);
    return NULL;
  }

  return PyInt_FromLong(status);
}

static PyObject *sensor_max(sensor_t *self, PyObject *arg)
{
  int32_t status = lothar_sensor_max(self->d_sensor);
  
  if(status < 0)
  {
    pylothar_raise_exception(-status);
    return NULL;
  }

  return PyInt_FromLong(status);
}

/* python stuff */

static PyMethodDef sensor_methods[] = 
{
  {"connection", (PyCFunction)sensor_connection, METH_NOARGS, ""},
  {"port",       (PyCFunction)sensor_port,       METH_NOARGS, ""},
  {"type",       (PyCFunction)sensor_type,       METH_NOARGS, ""},
  {"reset",      (PyCFunction)sensor_reset,      METH_O,      ""},
  {"stop",       (PyCFunction)sensor_stop,       METH_NOARGS, ""},
  {"value",      (PyCFunction)sensor_value,      METH_NOARGS, ""},
  {"minimum",    (PyCFunction)sensor_minimum,    METH_NOARGS, ""},
  {"maximum",    (PyCFunction)sensor_maximum,    METH_NOARGS, ""},
  {"val",        (PyCFunction)sensor_val,        METH_NOARGS, ""},
  {"min",        (PyCFunction)sensor_min,        METH_NOARGS, ""},
  {"max",        (PyCFunction)sensor_max,        METH_NOARGS, ""},

  {NULL, NULL, 0, NULL}
};

PyTypeObject pylothar_sensor_type = 
{
  PyObject_HEAD_INIT(NULL)
  0,                                        // ob_size 
  "lothar.sensor",                      // tp_name
  sizeof(sensor_t),                     // tp_basicsize
  0,                                        // tp_itemsize
  (destructor)sensor_dealloc,           // tp_dealloc
  0,                                        // tp_print
  0,                                        // tp_getattr
  0,                                        // tp_setattr
  0,                                        // tp_compare
  0,                                        // tp_repr
  0,                                        // tp_as_number
  0,                                        // tp_as_sequence
  0,                                        // tp_as_mapping
  0,                                        // tp_hash
  0,                                        // tp_call
  0,                                        // tp_str
  0,                                        // tp_getattro
  0,                                        // tp_setattro
  0,                                        // tp_as_buffer
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
  "sensor",   // tp_doc
  0,                                        // tp_traverse
  0,                                        // tp_clear
  0,                                        // tp_richcompare
  0,                                        // tp_weaklistoffset
  0,                                        // tp_iter
  0,                                        // tp_iternext
  sensor_methods,                       // tp_methods
  0,                                        // tp_members
  0,                                        // tp_getseters
  0,                                        // tp_base
  0,                                        // tp_dict
  0,                                        // tp_descr_get
  0,                                        // tp_descr_set
  0,                                        // tp_dictoffset
  (initproc)sensor_init,                // tp_init
  0,                                        // tp_alloc
  sensor_new,                           // tp_new
};

void pylothar_initsensor(PyObject *module)
{
  if(PyType_Ready(&pylothar_sensor_type) < 0)
    return;

  Py_INCREF(&pylothar_sensor_type);
  PyModule_AddObject(module, "sensor", (PyObject *)&pylothar_sensor_type);
}

