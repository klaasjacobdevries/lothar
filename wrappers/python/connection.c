#include "pylothar.h"
#include "error_handling.h"

/* creation and destruction */

static PyObject *connection_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  connection_t *self = (connection_t *)type->tp_alloc(type, 0);
  self->d_connection = NULL;
  self->d_custom = NULL;

  return (PyObject *)self;
}

static int readcb(void *private, uint8_t *buf, size_t len)
{
  connection_t *c = (connection_t *)private;
  PyObject *object = c->d_custom;

  PyObject *data = PyObject_CallMethod(object, "read", "I", len);
  int size;

  if(data && PyByteArray_Check(data))
  {
    size = PyByteArray_Size(data);
    memcpy(buf, PyByteArray_AsString(data), MIN(len, size));
  }
  else if(data && PyString_Check(data))
  {
    size = PyString_Size(data);
    memcpy(buf, PyString_AsString(data), MIN(len, size)); 
  }
  else
  {
    LOTHAR_FAIL("read callback failed\n");
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_NXT_READ_ERROR);
  }

  Py_DECREF(data);
  return size;
}

static int writecb(void *private, uint8_t const *buf, size_t len)
{
  connection_t *c = (connection_t *)private;
  PyObject *object = c->d_custom;
  int size;

  PyObject *data = PyObject_CallMethod(object, "write", "s#", (char const *)buf, len);

  if(!data || !PyInt_Check(data))
  {
    LOTHAR_FAIL("write failed\n");
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_NXT_WRITE_ERROR);
  }

  size = PyInt_AsLong(data);
  Py_DECREF(data);
  return size;
}

static int closecb(void *private)
{
  connection_t *c = (connection_t *)private;
  PyObject *object = c->d_custom;
  int size;

  PyObject *data = PyObject_CallMethod(object, "close", "");
  
  if(!data || !PyInt_Check(data))
  {
    LOTHAR_FAIL("close failed\n");
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_NXT_WRITE_ERROR);
  }

  size = PyInt_AsLong(data);
  Py_DECREF(data);
  return size;
}

static lothar_connection_vtable py_vtable = 
{
  readcb,
  writecb,
  closecb
};

static inline int checkmethod(PyObject *obj, char const *method)
{
  PyObject *m;
  int r;

  if(!PyObject_HasAttrString(obj, method))
    return 0;

  m = PyObject_GetAttrString(obj, method);
  
  r = PyCallable_Check(m);
  Py_DECREF(m);
  return r;
}

static int connection_init(connection_t *self, PyObject *args, PyObject *kwds)
{
  PyObject *do_bt = NULL;
  PyObject *do_usb = NULL;

  char const *address = BLUETOOTH_NXT_ADDRESS;
  unsigned vid = USB_VENDOR_LEGO;
  unsigned pid = USB_PRODUCT_NXT;

  PyObject *custom = NULL;

  static char *kwlist[] = {"bluetooth", "usb", "address", "vid", "pid", "custom", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "|OOsIIO", kwlist, &do_bt, &do_usb, &address, &vid, &pid, &custom))
    return -1;

  self->d_connection = NULL;

  // as usb-connections are typically faster to set up we attempt that first  
  if(do_usb && PyObject_IsTrue(do_usb))
  {
    LOTHAR_INFO("attempting usb connection to %d:%d\n", vid, pid);
    self->d_connection = lothar_connection_open_usb_vid_pid(vid, pid);
  }

  if(self->d_connection == NULL && do_bt && PyObject_IsTrue(do_bt))
  {
    LOTHAR_INFO("attemping bluetooth connection to %s\n", address);
    self->d_connection = lothar_connection_open_bluetooth_address(address);
  }  
  else if(custom)
  {
    LOTHAR_INFO("attempting custom connection\n");

    if(!checkmethod(custom, "read") ||
       !checkmethod(custom, "write") ||
       !checkmethod(custom, "close"))
    {
      LOTHAR_ERROR(LOTHAR_ERROR_BAD_ARGUMENTS);
      pylothar_raise_exception_global();
      return -1;
    }

    self->d_custom = custom;
    Py_INCREF(self->d_custom);

    self->d_connection = lothar_connection_open_custom(&py_vtable, self);
  }
  
  if(!self->d_connection)
  {
    LOTHAR_ERROR(LOTHAR_ERROR_BAD_ARGUMENTS);
    LOTHAR_WARN("no usable connection to attempt\n");

    pylothar_raise_exception_global();
    return -1;
  }

  return 0;
}

static void connection_dealloc(connection_t *self)
{
  if(self->d_connection)
    lothar_connection_close(&self->d_connection);

  if(self->d_custom)
  { // braces, bacause of ambiguous else (macro expansion)
    Py_DECREF(self->d_custom);
  }

  self->ob_type->tp_free((PyObject *)self);
}

/* read/write */

static PyObject *connection_write(connection_t *self, PyObject *arg)
{
  char const *data;
  int size;
  
  if(!PyByteArray_Check(arg))
  {
    PyErr_BadArgument();
    return NULL;
  }

  size = PyByteArray_Size(arg);
  data = PyByteArray_AsString(arg);

  return pylothar_check_return(lothar_connection_write(self->d_connection, (uint8_t const *)data, size));
}

static PyObject *connection_read(connection_t *self, PyObject *arg)
{
  unsigned size;
  int status;
  uint8_t *buf;
  PyObject *result;

  if(!PyInt_Check(arg))
  {
    PyErr_BadArgument();
    return NULL;
  }
  
  size = PyInt_AsUnsignedLongMask(arg);

  buf = (uint8_t *)lothar_malloc(size);

  if((status = lothar_connection_read(self->d_connection, buf, size)))
  {
    free(buf);
    pylothar_raise_exception(status);
    return NULL;
  }

  result = PyByteArray_FromStringAndSize((char *)buf, size);

  free(buf);
  
  return result;
}

/* the direct commands */

static PyObject *connection_startprogram(connection_t *self, PyObject *arg)
{
  char const *filename;

  if(!PyString_Check(arg))
  {
    PyErr_BadArgument();
    return NULL;
  }

  filename = PyString_AsString(arg);
    
  return pylothar_check_return(lothar_startprogram(self->d_connection, filename));
}

static PyObject *connection_stopprogram(connection_t *self, PyObject *arg)
{
  return pylothar_check_return(lothar_stopprogram(self->d_connection));
}

static PyObject *connection_playsoundfile(connection_t *self, PyObject *args, PyObject *kwds)
{
  PyObject *loop;
  char const *filename;

  static char *kwlist[] = {"loop", "filename", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "Os", kwlist, &loop, &filename))
    return NULL;

  return pylothar_check_return(lothar_playsoundfile(self->d_connection, PyObject_IsTrue(loop), filename));
}

static PyObject *connection_playtone(connection_t *self, PyObject *args, PyObject *kwds)
{
  unsigned frequency;
  unsigned duration;
  
  static char *kwlist[] = {"frequency", "duration", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "II", kwlist, &frequency, &duration))
    return NULL;

  return pylothar_check_return(lothar_playtone(self->d_connection, frequency, duration));
}

static PyObject *connection_setoutputstate(connection_t *self, PyObject *args, PyObject *kwds)
{
  int port, power, mode, rmode, turn_ratio, rstate, tacholimit;

  static char *kwlist[] = {"port", "power", "motormode", "regulationmode", "turnratio", "runstate", "tacholimit", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "iiiiiii|O", kwlist, &port, &power, &mode, &rmode, &turn_ratio, &rstate, &tacholimit))
    return NULL;

  return pylothar_check_return(lothar_setoutputstate(self->d_connection, port, power, mode, rmode, turn_ratio, rstate, tacholimit));
}

static PyObject *connection_setinputmode(connection_t *self, PyObject *args, PyObject *kwds)
{
  int port, sensortype, sensormode;

  static char *kwlist[] = {"port", "sensortype", "sensormode", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "iii|O", kwlist, &port, &sensortype, &sensormode))
    return NULL;

  return pylothar_check_return(lothar_setinputmode(self->d_connection, port, sensortype, sensormode));
}

static PyObject *connection_getoutputstate(connection_t *self, PyObject *arg)
{
  int port;
  int8_t power;
  enum lothar_output_motor_mode motormode;
  enum lothar_output_regulation_mode regulationmode;
  uint8_t turnratio;
  enum lothar_output_runstate runstate;
  uint32_t tacholimit;
  int32_t tachocount;
  int32_t blocktachocount;
  int32_t rotationcount;
  int status;

  PyObject *result;

  if(!PyInt_Check(arg))
  {
    PyErr_BadArgument();
    return NULL;
  }

  port = PyInt_AsLong(arg);

  if((status = lothar_getoutputstate(self->d_connection, port, &power, &motormode, &regulationmode, &turnratio, &runstate, &tacholimit, &tachocount, &blocktachocount, &rotationcount)) == 0)
  {
    result = PyTuple_New(9);
    
    PyTuple_SetItem(result, 0, PyInt_FromLong(power));
    PyTuple_SetItem(result, 1, PyInt_FromLong(motormode));
    PyTuple_SetItem(result, 2, PyInt_FromLong(regulationmode));
    PyTuple_SetItem(result, 3, PyInt_FromLong(turnratio));
    PyTuple_SetItem(result, 4, PyInt_FromLong(runstate));
    PyTuple_SetItem(result, 5, PyInt_FromLong(tacholimit));
    PyTuple_SetItem(result, 6, PyInt_FromLong(tachocount));
    PyTuple_SetItem(result, 7, PyInt_FromLong(blocktachocount));
    PyTuple_SetItem(result, 8, PyInt_FromLong(rotationcount));

    return result;
  }

  return pylothar_check_return(status);
}

static PyObject *connection_getinputvalues(connection_t *self, PyObject *arg)
{
  int port;
  uint8_t valid;
  uint8_t calibrated;
  enum lothar_sensor_type type;
  enum lothar_sensor_mode mode;
  uint16_t rawvalue;
  uint16_t normvalue;
  int16_t scaledvalue;
  int16_t calibratedvalue;

  int status;
  PyObject *result;

  if(!PyInt_Check(arg))
  {
    PyErr_BadArgument();
    return NULL;
  }

  port = PyInt_AsLong(arg);

  if((status = lothar_getinputvalues(self->d_connection, port, &valid, &calibrated, &type, &mode, &rawvalue, &normvalue, &scaledvalue, &calibratedvalue)) == 0)
  {
    result = PyTuple_New(8);
    
    PyTuple_SetItem(result, 0, PyBool_FromLong(valid));
    PyTuple_SetItem(result, 1, PyBool_FromLong(calibrated));
    PyTuple_SetItem(result, 2, PyInt_FromLong(type));
    PyTuple_SetItem(result, 3, PyInt_FromLong(mode));
    PyTuple_SetItem(result, 4, PyInt_FromLong(rawvalue));
    PyTuple_SetItem(result, 5, PyInt_FromLong(normvalue));
    PyTuple_SetItem(result, 6, PyInt_FromLong(scaledvalue));
    PyTuple_SetItem(result, 7, PyInt_FromLong(calibratedvalue));
    
    return result;
  }

  return pylothar_check_return(status);
}

static PyObject *connection_resetinputscaledvalue(connection_t *self, PyObject *arg)
{
  int port;

  if(!PyInt_Check(arg))
  {
    PyErr_BadArgument();
    return NULL;
  }

  port = PyInt_AsLong(arg);

  return pylothar_check_return(lothar_resetinputscaledvalue(self->d_connection, port));
}

static PyObject *connection_messagewrite(connection_t *self, PyObject *args, PyObject *kwds)
{
  int inbox;
  char const *data;
  int size;

  static char *kwlist[] = {"inbox", "data", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "is#", kwlist, &inbox, &data, &size))
    return NULL;

  return pylothar_check_return(lothar_messagewrite(self->d_connection, inbox, (uint8_t const *)data, size));
}

static PyObject *connection_resetmotorposition(connection_t *self, PyObject *args, PyObject *kwds)
{
  int port;
  PyObject *rel;

  static char *kwlist[] = {"port", "relative", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "iO", kwlist, &port, &rel))
    return NULL;
  
  return pylothar_check_return(lothar_resetmotorposition(self->d_connection, port, PyObject_IsTrue(rel)));
}

static PyObject *connection_getbatterylevel(connection_t *self, PyObject *arg)
{
  uint16_t level;
  int status;

  if((status = lothar_getbatterylevel(self->d_connection, &level)) == 0)
    return PyInt_FromSize_t(level);

  return pylothar_check_return(status);
}

static PyObject *connection_stopsoundplayback(connection_t *self, PyObject *arg)
{
  return pylothar_check_return(lothar_stopsoundplayback(self->d_connection));
}

static PyObject *connection_keepalive(connection_t *self, PyObject *arg)
{
  uint32_t sleep;
  int status;

  if((status = lothar_keepalive(self->d_connection, &sleep)) == 0)
    return PyInt_FromLong(sleep);

  return pylothar_check_return(status);
}

static PyObject *connection_lsgetstatus(connection_t *self, PyObject *arg)
{
  uint8_t bytesready;
  int status;
  int port;

  port = PyInt_AsLong(arg);

  if((status = lothar_lsgetstatus(self->d_connection, port, &bytesready)) == 0)
    return PyInt_FromLong(bytesready);

  return pylothar_check_return(status);
}

static PyObject *connection_lswrite(connection_t *self, PyObject *args, PyObject *kwds)
{
  int port;
  char const *txdata;
  int txlen;
  unsigned rxlen;

  static char *kwlist[] = {"port", "txdata", "rxlen", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "is#I", kwlist, &port, &txdata, &txlen, &rxlen))
    return NULL;

  return pylothar_check_return(lothar_lswrite(self->d_connection, port, (uint8_t const *)txdata, txlen, rxlen));
}

static PyObject *connection_lsread(connection_t *self, PyObject *arg)
{
  int port;
  uint8_t rxdata[16];
  uint8_t rxlen;
  int status;

  if(!PyInt_Check(arg))
  {
    PyErr_BadArgument();
    return NULL;
  }

  port = PyInt_AsLong(arg);

  if((status = lothar_lsread(self->d_connection, port, rxdata, sizeof(rxdata), &rxlen)) == 0)
    return PyByteArray_FromStringAndSize((char *)rxdata, rxlen);
  
  return pylothar_check_return(status);
}

static PyObject *connection_getcurrentprogramname(connection_t *self, PyObject *arg)
{
  char filename[19];
  int status;

  if((status = lothar_getcurrentprogramname(self->d_connection, filename)) == 0)
    return PyString_FromString(filename);

  return pylothar_check_return(status);
}

static PyObject *connection_messageread(connection_t *self, PyObject *args, PyObject *kwds)
{
  uint8_t data[59];
  uint8_t len;
  int remoteinbox, localinbox;
  PyObject *remove;
  int status;

  static char *kwlist[] = {"remoteinbox", "localinbox", "remove", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "iiO", kwlist, &remoteinbox, &localinbox, &remove))
    return NULL;

  if((status = lothar_messageread(self->d_connection, remoteinbox, localinbox, PyObject_IsTrue(remove), data, &len)) == 0)
    return PyByteArray_FromStringAndSize((char const *)data, len);

  return pylothar_check_return(status);
}

/* python stuff */

static PyMethodDef connection_methods[] = 
{
  {"write", (PyCFunction)connection_write, METH_O, ""},
  {"read",  (PyCFunction)connection_read,  METH_O, ""},

  {"startprogram",          (PyCFunction)connection_startprogram,          METH_O,                       ""},
  {"stopprogram",           (PyCFunction)connection_stopprogram,           METH_NOARGS,                  ""},
  {"playsoundfile",         (PyCFunction)connection_playsoundfile,         METH_VARARGS | METH_KEYWORDS, ""},
  {"playtone",              (PyCFunction)connection_playtone,              METH_VARARGS | METH_KEYWORDS, ""},
  {"setoutputstate",        (PyCFunction)connection_setoutputstate,        METH_VARARGS | METH_KEYWORDS, ""},
  {"setinputmode",          (PyCFunction)connection_setinputmode,          METH_VARARGS | METH_KEYWORDS, ""},
  {"getoutputstate",        (PyCFunction)connection_getoutputstate,        METH_O,                       ""},
  {"getinputvalues",        (PyCFunction)connection_getinputvalues,        METH_O,                       ""},
  {"resetinputscaledvalue", (PyCFunction)connection_resetinputscaledvalue, METH_O,                       ""},
  {"messagewrite",          (PyCFunction)connection_messagewrite,          METH_VARARGS | METH_KEYWORDS, ""},
  {"resetmotorposition",    (PyCFunction)connection_resetmotorposition,    METH_VARARGS | METH_KEYWORDS, ""},
  {"getbatterylevel",       (PyCFunction)connection_getbatterylevel,       METH_NOARGS,                  ""},
  {"stopsoundplayback",     (PyCFunction)connection_stopsoundplayback,     METH_NOARGS,                  ""},
  {"keepalive",             (PyCFunction)connection_keepalive,             METH_NOARGS,                  ""},
  {"lsgetstatus",           (PyCFunction)connection_lsgetstatus,           METH_O,                       ""},
  {"lswrite",               (PyCFunction)connection_lswrite,               METH_VARARGS | METH_KEYWORDS, ""},
  {"lsread",                (PyCFunction)connection_lsread,                METH_O,                       ""},
  {"getcurrentprogramname", (PyCFunction)connection_getcurrentprogramname, METH_NOARGS,                  ""},
  {"messageread",           (PyCFunction)connection_messageread,           METH_VARARGS | METH_KEYWORDS, ""},

  {NULL, NULL, 0, NULL}
};

PyTypeObject pylothar_connection_type = 
{
  PyObject_HEAD_INIT(NULL)
  0,                                        // ob_size 
  "lothar.connection",                      // tp_name
  sizeof(connection_t),                     // tp_basicsize
  0,                                        // tp_itemsize
  (destructor)connection_dealloc,           // tp_dealloc
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
  "Lothar (usb or bluetooth) connection",   // tp_doc
  0,                                        // tp_traverse
  0,                                        // tp_clear
  0,                                        // tp_richcompare
  0,                                        // tp_weaklistoffset
  0,                                        // tp_iter
  0,                                        // tp_iternext
  connection_methods,                       // tp_methods
  0,                                        // tp_members
  0,                                        // tp_getseters
  0,                                        // tp_base
  0,                                        // tp_dict
  0,                                        // tp_descr_get
  0,                                        // tp_descr_set
  0,                                        // tp_dictoffset
  (initproc)connection_init,                // tp_init
  0,                                        // tp_alloc
  connection_new,                           // tp_new
};

void pylothar_initconnection(PyObject *module)
{
  if(PyType_Ready(&pylothar_connection_type) < 0)
    return;

  Py_INCREF(&pylothar_connection_type);
  PyModule_AddObject(module, "connection", (PyObject *)&pylothar_connection_type);
}
