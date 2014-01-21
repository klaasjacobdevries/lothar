#include "pylothar.h"

/* creation and destruction */

typedef struct
{
  PyObject_HEAD
  lothar_motor_t *d_motor;

  PyObject *d_conn;
} motor_t;

static PyObject *motor_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  motor_t *self = (motor_t *)type->tp_alloc(type, 0);
  self->d_motor = NULL;
  self->d_conn  = NULL;

  return (PyObject *)self;
}

static int motor_init(motor_t *self, PyObject *args, PyObject *kwds)
{
  PyObject *connection;
  int port;

  static char *kwlist[] = {"connection", "port", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "Oi", kwlist, &connection, &port))
    return -1;

  if(!PyObject_IsInstance(connection, (PyObject *)&pylothar_connection_type))
  {
    PyErr_BadArgument();
    return -1;
  }

  self->d_motor = lothar_motor_open(((connection_t *)connection)->d_connection, port);

  if(!self->d_motor)
  {
    pylothar_raise_exception_global();
    return -1;
  }

  self->d_conn = connection;
  Py_INCREF(self->d_conn);

  return 0;
}

static void motor_dealloc(motor_t *self)
{
  if(self->d_motor)
    lothar_motor_close(&self->d_motor);

  if(self->d_conn)
  {
    Py_DECREF(self->d_conn);
  }

  self->ob_type->tp_free((PyObject *)self);
}

static inline int istrue(PyObject *o, int d)
{
  return o ? PyObject_IsTrue(o) : d;
}

/* methods */

static PyObject *motor_port(motor_t *self, PyObject *arg)
{
  enum lothar_output_port port;
  int status;
  if((status = lothar_motor_port(self->d_motor, &port)))
  {
    pylothar_raise_exception(status);
    return NULL;
  }

  return PyInt_FromLong(port);
}

static PyObject *motor_connection(motor_t *self, PyObject *arg)
{
  Py_INCREF(self->d_conn);
  return self->d_conn;
}

static PyObject *motor_run(motor_t *self, PyObject *args, PyObject *kwds)
{
  int power;
  unsigned ms = 0;
  PyObject *block = NULL;
  int status;

  static char *kwlist[] = {"power", "ms", "block", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "i|IO", kwlist, &power, &ms, &block))
    return NULL;

  if(ms > 0 || istrue(block, 0))
    status = lothar_motor_run_block(self->d_motor, power, ms);
  else
    status = lothar_motor_run(self->d_motor, power);

  return pylothar_check_return(status);
}

static PyObject *motor_turn(motor_t *self, PyObject *args, PyObject *kwds)
{
  int power;
  unsigned degrees;
  unsigned margin = 0;
  unsigned poll = 0;
  unsigned timeout = 0;
  PyObject *block = NULL;
  int status;

  static char *kwlist[] = {"power", "degrees", "margin", "poll", "timeout", "block", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "iI|IIIO", kwlist, &power, &degrees, &margin, &poll, &timeout, &block))
    return NULL;

  if(istrue(block, 0))
    status = lothar_motor_turn_block(self->d_motor, power, degrees, margin, poll, timeout);
  else
    status = lothar_motor_turn(self->d_motor, power, degrees);

  return pylothar_check_return(status);
}

static PyObject *motor_rotate(motor_t *self, PyObject *args, PyObject *kwds)
{
  int power;
  float nturns;
  float margin = 0;
  unsigned poll = 0;
  unsigned timeout = 0;
  PyObject *block = NULL;
  int status;

  static char *kwlist[] = {"power", "nturns", "margin", "poll", "timeout", "block", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "if|fIIO", kwlist, &power, &nturns, &margin, &poll, &timeout, &block))
    return NULL;

  if(istrue(block, 0))
    status = lothar_motor_rotate_block(self->d_motor, power, nturns, margin, poll, timeout);
  else
    status = lothar_motor_rotate(self->d_motor, power, nturns);

  return pylothar_check_return(status);
}

static PyObject *motor_stop(motor_t *self, PyObject *arg)
{
  return pylothar_check_return(lothar_motor_stop(self->d_motor));
}

static PyObject *motor_brake(motor_t *self, PyObject *arg)
{
  return pylothar_check_return(lothar_motor_brake(self->d_motor));
}

static PyObject *motor_reset(motor_t *self, PyObject *arg)
{
  return pylothar_check_return(lothar_motor_reset(self->d_motor, PyObject_IsTrue(arg)));
}

static PyObject *motor_degrees(motor_t *self, PyObject *arg)
{
  int32_t degrees = 0;
  int status;

  if((status = lothar_motor_degrees(self->d_motor, &degrees, PyObject_IsTrue(arg))))
  {
    pylothar_raise_exception(status);
    return NULL;
  }
   
  return PyInt_FromLong(degrees);
}

static PyObject *motor_power(motor_t *self, PyObject *arg)
{
  int8_t power;
  int status;

  if((status = lothar_motor_power(self->d_motor, &power)))
  {
    pylothar_raise_exception(status);
    return NULL;
  }

  return PyInt_FromLong(power);
}

PyTypeObject pylothar_motor_type; // forward decleration

PyObject *motor_sync(PyObject *self, PyObject *args, PyObject *kwds)
{
  PyObject *m1 = NULL;
  PyObject *m2 = NULL;
  PyObject *r  = NULL;
  
  static char *kwlist[] = {"motor1", "motor2", "reset", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "OO|O", kwlist, &m1, &m2, &r))
    return NULL;

  if(!PyObject_IsInstance(m1, (PyObject *)&pylothar_motor_type) ||
     !PyObject_IsInstance(m1, (PyObject *)&pylothar_motor_type))
  {
    PyErr_BadArgument();
    return NULL;
  }
  
  return pylothar_check_return(lothar_motor_sync(((motor_t *)m1)->d_motor, ((motor_t *)m2)->d_motor, istrue(r, 1)));
}

PyObject *motor_unsync(PyObject *self, PyObject *args)
{
  PyObject *m1 = NULL;
  PyObject *m2 = NULL;
  
  if(!PyArg_ParseTuple(args, "OO", &m1, &m2))
    return NULL;

  if(!PyObject_IsInstance(m1, (PyObject *)&pylothar_motor_type) ||
     !PyObject_IsInstance(m1, (PyObject *)&pylothar_motor_type))
  {
    PyErr_BadArgument();
    return NULL;
  }
  
  return pylothar_check_return(lothar_motor_unsync(((motor_t *)m1)->d_motor, ((motor_t *)m2)->d_motor));
}

/* python stuff */

static PyMethodDef motor_methods[] = 
{
  {"port",       (PyCFunction)motor_port,       METH_NOARGS,                  ""},
  {"connection", (PyCFunction)motor_connection, METH_NOARGS,                  ""},
  {"run",        (PyCFunction)motor_run,        METH_VARARGS | METH_KEYWORDS, ""},
  {"turn",       (PyCFunction)motor_turn,       METH_VARARGS | METH_KEYWORDS, ""},
  {"rotate",     (PyCFunction)motor_rotate,     METH_VARARGS | METH_KEYWORDS, ""},
  {"stop",       (PyCFunction)motor_stop,       METH_NOARGS,                  ""},
  {"brake",      (PyCFunction)motor_brake,      METH_NOARGS,                  ""},
  {"reset",      (PyCFunction)motor_reset,      METH_O,                       ""},
  {"degrees",    (PyCFunction)motor_degrees,    METH_O,                       ""},
  {"power",      (PyCFunction)motor_power,      METH_NOARGS,                  ""},

  {NULL, NULL, 0, NULL}
};

PyTypeObject pylothar_motor_type = 
{
  PyObject_HEAD_INIT(NULL)
  0,                                        // ob_size 
  "lothar.motor",                      // tp_name
  sizeof(motor_t),                     // tp_basicsize
  0,                                        // tp_itemsize
  (destructor)motor_dealloc,           // tp_dealloc
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
  "motor",   // tp_doc
  0,                                        // tp_traverse
  0,                                        // tp_clear
  0,                                        // tp_richcompare
  0,                                        // tp_weaklistoffset
  0,                                        // tp_iter
  0,                                        // tp_iternext
  motor_methods,                       // tp_methods
  0,                                        // tp_members
  0,                                        // tp_getseters
  0,                                        // tp_base
  0,                                        // tp_dict
  0,                                        // tp_descr_get
  0,                                        // tp_descr_set
  0,                                        // tp_dictoffset
  (initproc)motor_init,                // tp_init
  0,                                        // tp_alloc
  motor_new,                           // tp_new
};

void pylothar_initmotor(PyObject *module)
{
  if(PyType_Ready(&pylothar_motor_type) < 0)
    return;

  Py_INCREF(&pylothar_motor_type);
  PyModule_AddObject(module, "motor", (PyObject *)&pylothar_motor_type);
}

