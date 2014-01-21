#include "pylothar.h"

/* creation and destruction */

typedef struct
{
  PyObject_HEAD
  lothar_scheduler_t *d_scheduler;
} scheduler_t;

static PyObject *scheduler_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  scheduler_t *self = (scheduler_t *)type->tp_alloc(type, 0);
  self->d_scheduler = NULL;

  return (PyObject *)self;
}

static int scheduler_init(scheduler_t *self, PyObject *args, PyObject *kwds)
{
  self->d_scheduler = lothar_scheduler_create();

  return 0;
}

static void scheduler_dealloc(scheduler_t *self)
{
  if(self->d_scheduler)
    lothar_scheduler_destroy(&self->d_scheduler);

  self->ob_type->tp_free((PyObject *)self);
}


typedef struct
{
  PyObject *scheduler;
  PyObject *callable;
  PyObject *args;
  PyObject *kwds;
} callback_t;

static void function_cb(callback_t *cb)
{
  PyObject *ret = PyObject_Call(cb->callable, cb->args, cb->kwds);

  if(ret)
  {
    Py_DECREF(ret);
  }

  if(PyErr_Occurred()) // explicitely break from the scheduler if an exception was raised
  {
    LOTHAR_DEBUG("exception raised in python code\n");
    lothar_scheduler_stop(((scheduler_t *)cb->scheduler)->d_scheduler);
  }
}

static void cleanup_cb(callback_t *cb)
{
  if(cb->scheduler)
  {
    Py_DECREF(cb->scheduler);
  }
  if(cb->callable)
  {
    Py_DECREF(cb->callable);
  }
  if(cb->args)
  {
    Py_DECREF(cb->args);
  }
  if(cb->kwds)
  {
    Py_DECREF(cb->kwds);
  }
  free(cb);
}

static callback_t *new_cb(PyObject *scheduler, PyObject *callable, PyObject *args, PyObject *kwds)
{
  callback_t *cb;

  if(!PyCallable_Check(callable))
  {
    PyErr_BadArgument();
    return NULL;
  }

  if(args && !PyTuple_Check(args))
  {
    PyErr_BadArgument();
    return NULL;
  }

  if(kwds && !PyDict_Check(kwds))
  {
    PyErr_BadArgument();
    return NULL;
  }

  cb = (callback_t *)lothar_malloc(sizeof(callback_t));
  cb->scheduler = scheduler;
  cb->callable = callable;
  cb->args = args;
  cb->kwds = kwds;

  Py_INCREF(cb->scheduler);
  Py_INCREF(cb->callable);

  if(cb->args)
    Py_INCREF(cb->args);
  else
    cb->args = PyTuple_New(0);

  if(cb->kwds)
    Py_INCREF(cb->kwds);

  return cb;
}

static PyObject *scheduler_add(scheduler_t *self, PyObject *args, PyObject *kwds)
{
  PyObject *function;
  unsigned arrival = 0;
  unsigned estimate = 0;
  unsigned nice = 10;
  PyObject *cb_args = NULL;
  PyObject *cb_kwds = NULL;
  callback_t *cb;

  static char *kwlist[] = {"function", "arrival_time", "estimate", "nice", "function_args", "function_kwds", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "O|IIIOO", kwlist, &function, &arrival, &estimate, &nice, &cb_args, &cb_kwds))
    return NULL;

  if(!(cb = new_cb((PyObject *)self, function, cb_args, cb_kwds)))
    return NULL;

  return pylothar_check_return(lothar_scheduler_add(self->d_scheduler, (void (*)(void *))function_cb, (void (*)(void *))cleanup_cb, cb, arrival, estimate, nice));
}

static PyObject *scheduler_add_interval(scheduler_t *self, PyObject *args, PyObject *kwds)
{
  PyObject *function;
  unsigned interval;
  unsigned ntimes = 0;
  unsigned arrival = 0;
  unsigned estimate = 0;
  unsigned nice = 10;
  PyObject *cb_args = NULL;
  PyObject *cb_kwds = NULL;
  callback_t *cb;

  static char *kwlist[] = {"function", "interval", "ntimes", "arrival_time", "estimate", "nice", "function_args", "function_kwds", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "OI|IIIIOO", kwlist, &function, &interval, &ntimes, &arrival, &estimate, &nice, &cb_args, &cb_kwds))
    return NULL;

  if(!(cb = new_cb((PyObject *)self, function, cb_args, cb_kwds)))
    return NULL;

  return pylothar_check_return(lothar_scheduler_add_interval(self->d_scheduler, (void (*)(void *))function_cb, (void (*)(void *))cleanup_cb, cb, arrival, estimate, nice, interval, ntimes));
}

static PyObject *scheduler_empty(scheduler_t *self, PyObject *arg)
{
  int status, isempty;
  if((status = lothar_scheduler_empty(self->d_scheduler, &isempty)))
  {
    pylothar_raise_exception(status);
    return NULL;
  }

  return PyBool_FromLong(isempty);
}

static PyObject *scheduler_size(scheduler_t *self, PyObject *arg)
{
  int status;
  size_t size;
  if((status = lothar_scheduler_size(self->d_scheduler, &size)))
  {
    pylothar_raise_exception(status);
    return NULL;
  }

  return PyInt_FromLong(size);
}

static PyObject *scheduler_peek(scheduler_t *self, PyObject *arg)
{
  int status;
  lothar_time_t next;
  if((status = lothar_scheduler_peek(self->d_scheduler, &next)))
  {
    pylothar_raise_exception(status);
    return NULL;
  }

  return PyInt_FromLong((long)next);
}

static PyObject *scheduler_run_single(scheduler_t *self, PyObject *arg)
{
  return pylothar_check_return(lothar_scheduler_run_single(self->d_scheduler));
}

static PyObject *scheduler_run(scheduler_t *self, PyObject *arg)
{
  return pylothar_check_return(lothar_scheduler_run(self->d_scheduler));
}

static PyObject *scheduler_stop(scheduler_t *self, PyObject *arg)
{
  return pylothar_check_return(lothar_scheduler_stop(self->d_scheduler));
}

static PyObject *scheduler_done(scheduler_t *self, PyObject *arg)
{
  int status = lothar_scheduler_done(self->d_scheduler);

  if(status < 0)
  {
    pylothar_raise_exception(-status);
    return NULL;
  }

  return PyBool_FromLong(status);
}

static PyObject *scheduler_next(scheduler_t *self, PyObject *arg)
{
  lothar_time_t n;

  LOTHAR_ERROR(LOTHAR_ERROR_OKAY);
  n = lothar_scheduler_next(self->d_scheduler);

  if(lothar_errno != LOTHAR_ERROR_OKAY)
  {
    pylothar_raise_exception_global();
    return NULL;
  }

  return PyInt_FromLong((long)n);
}

static PyMethodDef scheduler_methods[] = 
{
  {"add",          (PyCFunction)scheduler_add,          METH_VARARGS | METH_KEYWORDS, ""},
  {"add_interval", (PyCFunction)scheduler_add_interval, METH_VARARGS | METH_KEYWORDS, ""},
  {"empty",        (PyCFunction)scheduler_empty,        METH_NOARGS,                  ""},
  {"size",         (PyCFunction)scheduler_size,         METH_NOARGS,                  ""},
  {"peek",         (PyCFunction)scheduler_peek,         METH_NOARGS,                  ""},
  {"run",          (PyCFunction)scheduler_run,          METH_NOARGS,                  ""},
  {"run_single",   (PyCFunction)scheduler_run_single,   METH_NOARGS,                  ""},
  {"stop",         (PyCFunction)scheduler_stop,         METH_NOARGS,                  ""},
  {"done",         (PyCFunction)scheduler_done,         METH_NOARGS,                  ""},
  {"next",         (PyCFunction)scheduler_next,         METH_NOARGS,                  ""},

  {NULL, NULL, 0, NULL}
};

PyTypeObject pylothar_scheduler_type = 
{
  PyObject_HEAD_INIT(NULL)
  0,                                        // ob_size 
  "lothar.scheduler",                      // tp_name
  sizeof(scheduler_t),                     // tp_basicsize
  0,                                        // tp_itemsize
  (destructor)scheduler_dealloc,           // tp_dealloc
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
  "scheduler",   // tp_doc
  0,                                        // tp_traverse
  0,                                        // tp_clear
  0,                                        // tp_richcompare
  0,                                        // tp_weaklistoffset
  0,                                        // tp_iter
  0,                                        // tp_iternext
  scheduler_methods,                       // tp_methods
  0,                                        // tp_members
  0,                                        // tp_getseters
  0,                                        // tp_base
  0,                                        // tp_dict
  0,                                        // tp_descr_get
  0,                                        // tp_descr_set
  0,                                        // tp_dictoffset
  (initproc)scheduler_init,                // tp_init
  0,                                        // tp_alloc
  scheduler_new,                           // tp_new
};


PyMODINIT_FUNC pylothar_initscheduler(PyObject *module)
{
  if(PyType_Ready(&pylothar_scheduler_type) < 0)
    return;

  Py_INCREF(&pylothar_scheduler_type);
  PyModule_AddObject(module, "scheduler", (PyObject *)&pylothar_scheduler_type);
}


