#include "pylothar.h"

static PyObject *pylothar_error;

void pylothar_raise_exception(int error)
{
  PyErr_SetString(pylothar_error, lothar_strerror(error));
}

void pylothar_raise_exception_global(void)
{
  pylothar_raise_exception(lothar_errno);
}

PyObject *pylothar_check_return(int status)
{
  if(status < 0)
  {
    pylothar_raise_exception(status);
    return NULL;
  }

  Py_RETURN_NONE;
}

static PyObject *utils_msleep(PyObject *self, PyObject *arg)
{
  if(!PyInt_Check(arg))
  {
    PyErr_BadArgument();
    return NULL;
  }
  
  return pylothar_check_return(lothar_msleep(PyInt_AsLong(arg)));  
}

static PyObject *utils_time(PyObject *self, PyObject *arg)
{
  return PyInt_FromLong((long)lothar_time());
}

static PyObject *utils_timer(PyObject *self, PyObject *args)
{
  PyObject *a = NULL;
  lothar_time_t t;

  if(!PyArg_ParseTuple(args, "|O", &a))
    return NULL;

  if(a)
  {
    if(!PyInt_Check(a))
    {
      PyErr_BadArgument();
      return NULL;
    }

    t = PyInt_AsLong(a);
    return PyInt_FromLong((long)lothar_timer(&t));
  }
  
  return PyInt_FromLong((long)lothar_timer(NULL));
}

PyObject *motor_sync(PyObject *self, PyObject *args, PyObject *kwds); // defined in motor.c
PyObject *motor_unsync(PyObject *self, PyObject *args); // defined in motor.c

static PyMethodDef module_methods[] = 
{
  {"msleep", (PyCFunction)utils_msleep, METH_O,       ""},
  {"time",   (PyCFunction)utils_time,   METH_NOARGS,  ""},
  {"timer",  (PyCFunction)utils_timer,  METH_VARARGS, ""},

  {"motor_sync",   (PyCFunction)motor_sync,   METH_VARARGS | METH_KEYWORDS, ""},
  {"motor_unsync", (PyCFunction)motor_unsync, METH_VARARGS, ""},
  {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initlothar(void)
{
  PyObject *module = Py_InitModule3("lothar", module_methods, "Lothar NXT communcation");

  if(!module)
    return;

  pylothar_error = PyErr_NewException("lothar.error", NULL, NULL);
  Py_INCREF(pylothar_error);

  PyModule_AddIntConstant(module, "OUTPUT_A", OUTPUT_A);
  PyModule_AddIntConstant(module, "OUTPUT_B", OUTPUT_B);
  PyModule_AddIntConstant(module, "OUTPUT_C", OUTPUT_C);

  PyModule_AddIntConstant(module, "INPUT_1", INPUT_1);
  PyModule_AddIntConstant(module, "INPUT_2", INPUT_2);
  PyModule_AddIntConstant(module, "INPUT_3", INPUT_3);
  PyModule_AddIntConstant(module, "INPUT_4", INPUT_4);

  /* add some constants */

  PyModule_AddIntConstant(module, "USB_VENDOR_LEGO", USB_VENDOR_LEGO);
  PyModule_AddIntConstant(module, "USB_PRODUCT_NXT", USB_PRODUCT_NXT);

  PyModule_AddStringConstant(module, "BLUETOOTH_NXT_ADDRESS", BLUETOOTH_NXT_ADDRESS);

  PyModule_AddIntConstant(module, "MOTOR_MODE_MOTORON",   MOTOR_MODE_MOTORON);
  PyModule_AddIntConstant(module, "MOTOR_MODE_BRAKE",     MOTOR_MODE_BRAKE);
  PyModule_AddIntConstant(module, "MOTOR_MODE_REGULATED", MOTOR_MODE_REGULATED);
  
  PyModule_AddIntConstant(module, "REGULATION_MODE_IDLE",       REGULATION_MODE_IDLE);
  PyModule_AddIntConstant(module, "REGULATION_MODE_SPEED",      REGULATION_MODE_SPEED);
  PyModule_AddIntConstant(module, "REGULATION_MODE_MOTOR_SYNC", REGULATION_MODE_MOTOR_SYNC);

  PyModule_AddIntConstant(module, "RUNSTATE_IDLE",     RUNSTATE_IDLE);
  PyModule_AddIntConstant(module, "RUNSTATE_RAMPUP",   RUNSTATE_RAMPUP);
  PyModule_AddIntConstant(module, "RUNSTATE_RUNNING",  RUNSTATE_RUNNING);
  PyModule_AddIntConstant(module, "RUNSTATE_RAMPDOWN", RUNSTATE_RAMPDOWN);

  PyModule_AddIntConstant(module, "SENSOR_NO_SENSOR",          SENSOR_NO_SENSOR);
  PyModule_AddIntConstant(module, "SENSOR_SWITCH",             SENSOR_SWITCH);
  PyModule_AddIntConstant(module, "SENSOR_TEMPERATURE",        SENSOR_TEMPERATURE);
  PyModule_AddIntConstant(module, "SENSOR_REFLECTION",         SENSOR_REFLECTION);
  PyModule_AddIntConstant(module, "SENSOR_ANGLE",              SENSOR_ANGLE);
  PyModule_AddIntConstant(module, "SENSOR_LIGHT_ACTIVE",       SENSOR_LIGHT_ACTIVE);
  PyModule_AddIntConstant(module, "SENSOR_LIGHT_INACTIVE",     SENSOR_LIGHT_INACTIVE);
  PyModule_AddIntConstant(module, "SENSOR_SOUND_DB",           SENSOR_SOUND_DB);
  PyModule_AddIntConstant(module, "SENSOR_SOUND_DBA",          SENSOR_SOUND_DBA);
  PyModule_AddIntConstant(module, "SENSOR_CUSTOM",             SENSOR_CUSTOM);
  PyModule_AddIntConstant(module, "SENSOR_LOWSPEED",           SENSOR_LOWSPEED);
  PyModule_AddIntConstant(module, "SENSOR_LOWSPEED_9V",        SENSOR_LOWSPEED_9V);
  PyModule_AddIntConstant(module, "SENSOR_HIGHSPEED",          SENSOR_HIGHSPEED);
  PyModule_AddIntConstant(module, "SENSOR_COLORFULL",          SENSOR_COLORFULL);
  PyModule_AddIntConstant(module, "SENSOR_COLORRED",           SENSOR_COLORRED);
  PyModule_AddIntConstant(module, "SENSOR_COLORGREEN",         SENSOR_COLORGREEN);
  PyModule_AddIntConstant(module, "SENSOR_COLORBLUE",          SENSOR_COLORBLUE);
  PyModule_AddIntConstant(module, "SENSOR_COLORNONE",          SENSOR_COLORNONE);
  PyModule_AddIntConstant(module, "SENSOR_NO_OF_SENSOR_TYPES", SENSOR_NO_OF_SENSOR_TYPES);

  PyModule_AddIntConstant(module, "SENSOR_MODE_RAWMODE",          SENSOR_MODE_RAWMODE);
  PyModule_AddIntConstant(module, "SENSOR_MODE_BOOLEANMODE",      SENSOR_MODE_BOOLEANMODE);
  PyModule_AddIntConstant(module, "SENSOR_MODE_TRANSITIONMODE",   SENSOR_MODE_TRANSITIONMODE);
  PyModule_AddIntConstant(module, "SENSOR_MODE_PERIODCOUNTMODE",  SENSOR_MODE_PERIODCOUNTMODE);
  PyModule_AddIntConstant(module, "SENSOR_MODE_PCTFULLSCALEMODE", SENSOR_MODE_PCTFULLSCALEMODE);
  PyModule_AddIntConstant(module, "SENSOR_MODE_CELSIUSMODE",      SENSOR_MODE_CELSIUSMODE);
  PyModule_AddIntConstant(module, "SENSOR_MODE_FAHRENHEITMODE",   SENSOR_MODE_FAHRENHEITMODE);
  PyModule_AddIntConstant(module, "SENSOR_MODE_ANGLESTEPMODE",    SENSOR_MODE_ANGLESTEPMODE);
  PyModule_AddIntConstant(module, "SENSOR_MODE_SLOPEMASK",        SENSOR_MODE_SLOPEMASK);
  PyModule_AddIntConstant(module, "SENSOR_MODE_MODEMASK",         SENSOR_MODE_MODEMASK);

  PyModule_AddIntConstant(module, "COLOR_BLACK",  COLOR_BLACK);
  PyModule_AddIntConstant(module, "COLOR_BLUE",   COLOR_BLUE);
  PyModule_AddIntConstant(module, "COLOR_GREEN",  COLOR_GREEN);
  PyModule_AddIntConstant(module, "COLOR_YELLOW", COLOR_YELLOW);
  PyModule_AddIntConstant(module, "COLOR_RED",    COLOR_RED);
  PyModule_AddIntConstant(module, "COLOR_WHITE",  COLOR_WHITE);

  pylothar_initconnection(module);
  pylothar_initmotor(module);
  pylothar_initsensor(module);
  pylothar_initscheduler(module);
  pylothar_initsteering(module);
}

