#include "pylothar.h"
#include "steering.h"

/* creation and destruction */

typedef struct
{
  PyObject_HEAD
  lothar_steering_t *d_steering;

  PyObject *d_conn;
} steering_t;

static PyObject *steering_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  steering_t *self = (steering_t *)type->tp_alloc(type, 0);
  self->d_steering = NULL;
  self->d_conn  = NULL;

  return (PyObject *)self;
}

static int steering_init(steering_t *self, PyObject *args, PyObject *kwds)
{
  PyObject *connection;
  int portleft;
  int portright;
  double radius;
  double distance;

  static char *kwlist[] = {"connection", "leftport", "rightport", "radius", "distance", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "Oiidd", kwlist, &connection, &portleft, &portright, &radius, &distance))
    return -1;

  if(!PyObject_IsInstance(connection, (PyObject *)&pylothar_connection_type))
  {
    PyErr_BadArgument();
    return -1;
  }

  self->d_steering = lothar_steering_open(((connection_t *)connection)->d_connection, portleft, portright, radius, distance);

  if(!self->d_steering)
  {
    pylothar_raise_exception_global();
    return -1;
  }

  self->d_conn = connection;
  Py_INCREF(self->d_conn);

  return 0;
}

static void steering_dealloc(steering_t *self)
{
  if(self->d_steering)
    lothar_steering_close(&self->d_steering);

  if(self->d_conn)
  {
    Py_DECREF(self->d_conn);
  }

  self->ob_type->tp_free((PyObject *)self);
}

/* methods */

static PyObject *steering_connection(steering_t *self, PyObject *arg)
{
  Py_INCREF(self->d_conn);
  return self->d_conn;
}

static PyObject *steering_forward(steering_t *self, PyObject *arg)
{
  int status;
  status = lothar_steering_forward(self->d_steering, PyFloat_AsDouble(arg));
  return pylothar_check_return(status);
}

static PyObject *steering_turn(steering_t *self, PyObject *args, PyObject *kwds)
{
  double speed;
  double turnspeed;

  static char *kwlist[] = {"speed", "turnspeed", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "dd", kwlist, &speed, &turnspeed))
    return NULL;

  return pylothar_check_return(lothar_steering_turn(self->d_steering, speed, turnspeed));
}

static PyObject *steering_stop(steering_t *self, PyObject *arg)
{
  return pylothar_check_return(lothar_steering_stop(self->d_steering));
}

static PyObject *steering_brake(steering_t *self, PyObject *arg)
{
  return pylothar_check_return(lothar_steering_brake(self->d_steering));
}

static PyObject *steering_get_odometry(steering_t *self, PyObject *arg)
{
  double x;
  double y;
  double o;

  int status;
  PyObject *r;

  if((status = lothar_steering_get_odometry(self->d_steering, &x, &y, &o)))
  {
    pylothar_raise_exception(status);
    return NULL;
  }

  r = PyTuple_New(3);
  PyTuple_SetItem(r, 0, PyFloat_FromDouble(x));
  PyTuple_SetItem(r, 1, PyFloat_FromDouble(y));
  PyTuple_SetItem(r, 2, PyFloat_FromDouble(o));
   
  return r;
}

static PyObject *steering_set_odometry(steering_t *self, PyObject *args, PyObject *kwds)
{
  double x;
  double y;
  double o;

  static char *kwlist[] = {"x", "y", "o", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "ddd", kwlist, &x, &y, &o))
    return NULL;

  return pylothar_check_return(lothar_steering_set_odometry(self->d_steering, x, y, o));
}

static PyObject *steering_update(steering_t *self, PyObject *arg)
{
  return pylothar_check_return(lothar_steering_update(self->d_steering));
}

static PyObject *steering_x(steering_t *self, PyObject *arg)
{
  return PyFloat_FromDouble(lothar_steering_x(self->d_steering));
}

static PyObject *steering_y(steering_t *self, PyObject *arg)
{
  return PyFloat_FromDouble(lothar_steering_y(self->d_steering));
}

static PyObject *steering_o(steering_t *self, PyObject *arg)
{
  return PyFloat_FromDouble(lothar_steering_o(self->d_steering));
}

/* python stuff */

static PyMethodDef steering_methods[] = 
{
  {"connection",   (PyCFunction)steering_connection,   METH_NOARGS,                  ""},
  {"forward",      (PyCFunction)steering_forward,      METH_O,                       ""},
  {"turn",         (PyCFunction)steering_turn,         METH_VARARGS | METH_KEYWORDS, ""},
  {"stop",         (PyCFunction)steering_stop,         METH_NOARGS,                  ""},
  {"brake",        (PyCFunction)steering_brake,        METH_NOARGS,                  ""},
  {"get_odometry", (PyCFunction)steering_get_odometry, METH_NOARGS,                  ""},
  {"set_odometry", (PyCFunction)steering_set_odometry, METH_VARARGS | METH_KEYWORDS, ""},
  {"update",       (PyCFunction)steering_update,       METH_NOARGS,                  ""},
  {"x",            (PyCFunction)steering_x,            METH_NOARGS,                  ""},
  {"y",            (PyCFunction)steering_y,            METH_NOARGS,                  ""},
  {"o",            (PyCFunction)steering_o,            METH_NOARGS,                  ""},

  {NULL, NULL, 0, NULL}
};

PyTypeObject pylothar_steering_type = 
{
  PyObject_HEAD_INIT(NULL)
  0,                                        // ob_size 
  "lothar.steering",                      // tp_name
  sizeof(steering_t),                     // tp_basicsize
  0,                                        // tp_itemsize
  (destructor)steering_dealloc,           // tp_dealloc
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
  "steering",   // tp_doc
  0,                                        // tp_traverse
  0,                                        // tp_clear
  0,                                        // tp_richcompare
  0,                                        // tp_weaklistoffset
  0,                                        // tp_iter
  0,                                        // tp_iternext
  steering_methods,                       // tp_methods
  0,                                        // tp_members
  0,                                        // tp_getseters
  0,                                        // tp_base
  0,                                        // tp_dict
  0,                                        // tp_descr_get
  0,                                        // tp_descr_set
  0,                                        // tp_dictoffset
  (initproc)steering_init,                // tp_init
  0,                                        // tp_alloc
  steering_new,                           // tp_new
};

void pylothar_initsteering(PyObject *module)
{
  if(PyType_Ready(&pylothar_steering_type) < 0)
    return;

  Py_INCREF(&pylothar_steering_type);
  PyModule_AddObject(module, "steering", (PyObject *)&pylothar_steering_type);
}

