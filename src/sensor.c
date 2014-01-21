#include "sensor.h"
#include "commands.h"

#define IS_VALID(s) { if(!s) { LOTHAR_FAIL("invalid sensor\n"); LOTHAR_RETURN_ERROR(LOTHAR_ERROR_ENTITY_CLOSED); } }

struct lothar_sensor_t
{
  lothar_connection_t *d_connection;
  enum lothar_input_port d_port;

  enum lothar_sensor_type d_type;
  enum lothar_sensor_type d_last; // the last valid type, stored for restarting after stop
  enum lothar_sensor_mode d_mode;
 
  // min and max values
  uint16_t d_min;
  uint16_t d_max;

  // callbacks
  int (*d_start)(lothar_sensor_t *sensor);
  int (*d_value)(lothar_sensor_t *sensor, uint8_t *valid, uint16_t *value);
};


static int value_touch(lothar_sensor_t *sensor, uint8_t *valid, uint16_t *value)
{
  return lothar_getinputvalues(sensor->d_connection, sensor->d_port, valid, NULL, NULL, NULL, NULL, NULL, (int16_t *)value, NULL);
}

static int value_color(lothar_sensor_t *sensor, uint8_t *valid, uint16_t *value)
{
  return lothar_getinputvalues(sensor->d_connection, sensor->d_port, valid, NULL, NULL, NULL, NULL, value, NULL, NULL);
}

static int value_colorfull(lothar_sensor_t *sensor, uint8_t *valid, uint16_t *value)
{
  return lothar_getinputvalues(sensor->d_connection, sensor->d_port, valid, NULL, NULL, NULL, NULL, NULL, (int16_t *)value, NULL);
}

static int value_light(lothar_sensor_t *sensor, uint8_t *valid, uint16_t *value)
{
  return lothar_getinputvalues(sensor->d_connection, sensor->d_port, valid, NULL, NULL, NULL, NULL, value, NULL, NULL);
}

static int value_ultrasound(lothar_sensor_t *sensor, uint8_t *valid, uint16_t *value)
{
  int status;
  uint8_t v;

  if((status = lothar_lsgetstatus(sensor->d_connection, sensor->d_port, valid)) < 0)
    return status;

  if(*valid)
  {
    if((status = lothar_lsread(sensor->d_connection, sensor->d_port, &v, 1, NULL)) < 0)
      return status;

    if(value)
      *value = v;
  }
  return 0;
}

// the start methods assume a valid sensor->d_last (bit of an ugly design to get start_color right)

static int start_touch(lothar_sensor_t *sensor)
{
  int status;
  if((status = lothar_setinputmode(sensor->d_connection, sensor->d_port, sensor->d_last, SENSOR_MODE_BOOLEANMODE)) < 0)
    return status;

  sensor->d_type = sensor->d_last;
  sensor->d_mode = SENSOR_MODE_BOOLEANMODE;

  sensor->d_min = 0;
  sensor->d_max = 1;

  sensor->d_value = value_touch;

  return 0;
}

static int start_color(lothar_sensor_t *sensor)
{
  int status;
  if((status = lothar_setinputmode(sensor->d_connection, sensor->d_port, sensor->d_last, SENSOR_MODE_RAWMODE)) < 0)
    return status;

  sensor->d_type = sensor->d_last;
  sensor->d_mode = SENSOR_MODE_RAWMODE;

  switch(sensor->d_type)
  {
  case SENSOR_COLORRED:
  case SENSOR_COLORGREEN:
  case SENSOR_COLORBLUE:
  case SENSOR_COLORNONE:
    sensor->d_min = 0;
    sensor->d_max = 1023;
    sensor->d_value = value_color;
    break;

  case SENSOR_COLORFULL:
    sensor->d_min = COLOR_BLACK;
    sensor->d_max = COLOR_WHITE;
    sensor->d_value = value_colorfull;
    break;

  default: // what happened?
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_INVALID_ARGUMENT);
  }

  return 0;
}

static int start_light(lothar_sensor_t *sensor)
{
  int status;
  if((status = lothar_setinputmode(sensor->d_connection, sensor->d_port, sensor->d_last, SENSOR_MODE_RAWMODE)) < 0)
    return status;

  sensor->d_type = sensor->d_last;
  sensor->d_mode = SENSOR_MODE_RAWMODE;

  sensor->d_min = 0;
  sensor->d_max = 1023;
  sensor->d_value = value_light;
  
  return 0;
}

static int start_ultrasound(lothar_sensor_t *sensor)
{
  int status;
  int i;
  if((status = lothar_setinputmode(sensor->d_connection, sensor->d_port, SENSOR_LOWSPEED_9V, SENSOR_MODE_RAWMODE)) < 0)
    return status;

  sensor->d_type = sensor->d_last;
  sensor->d_min = 0;
  sensor->d_max = 255;
  sensor->d_value = value_ultrasound;
  
  if((status = lothar_msleep(1000)) < 0) // let the device 'power up'
    return status;

  for(i = 0; i < 10; ++i)
  {
    if((status = lothar_sensor_value(sensor, NULL)) != LOTHAR_ERROR_CONNECTION_NOT_CONFIGURED)
      return status;
    
    LOTHAR_ERROR(LOTHAR_ERROR_OKAY);

    if((status = lothar_msleep((i + 1) * 10)) < 0) // give the device time to power up
      return status;
  }

  if((status = lothar_sensor_value(sensor, NULL)) != LOTHAR_ERROR_CONNECTION_NOT_CONFIGURED)
    return status;
  
  LOTHAR_RETURN_ERROR(LOTHAR_ERROR_TIMEOUT);
}

lothar_sensor_t *lothar_sensor_open(lothar_connection_t *connection, enum lothar_input_port port, enum lothar_sensor_type type)
{
  lothar_sensor_t *result;

  switch(port)
  {
  case INPUT_1:
  case INPUT_2:
  case INPUT_3:
  case INPUT_4:
    break;

  default:
    LOTHAR_ERROR(LOTHAR_ERROR_INVALID_ARGUMENT);
    return NULL;
  }

  result = (lothar_sensor_t *)lothar_malloc(sizeof(lothar_sensor_t));

  result->d_connection = connection;
  result->d_port = port;

  if(lothar_sensor_reset(result, type))
  {
    free(result);
    return NULL;
  }

  return result;
}

int lothar_sensor_close(lothar_sensor_t **sensor)
{
  int status;

  IS_VALID(*sensor);

  status = lothar_sensor_stop(*sensor);

  free(*sensor);
  *sensor = NULL;

  return status;
}

int lothar_sensor_connection(lothar_sensor_t const *sensor, lothar_connection_t **connection)
{
  IS_VALID(sensor);

  if(connection)
    *connection = sensor->d_connection;

  return 0;
}

int lothar_sensor_port(lothar_sensor_t const *sensor, enum lothar_input_port *port)
{
  IS_VALID(sensor);
  
  if(port)
    *port = sensor->d_port;

  return 0;
}

int lothar_sensor_type(lothar_sensor_t const *sensor, enum lothar_sensor_type *type)
{
  IS_VALID(sensor);

  if(type)
    *type = sensor->d_last;

  return 0;
}

int lothar_sensor_reset(lothar_sensor_t *sensor, enum lothar_sensor_type type)
{
  IS_VALID(sensor);

  switch(type)
  {
  case SENSOR_SWITCH:
    sensor->d_last = type;
    sensor->d_start = start_touch;
    break;
    
  case SENSOR_LOWSPEED_9V:
    sensor->d_last = type;
    sensor->d_start = start_ultrasound;
    break;

  case SENSOR_COLORFULL:
  case SENSOR_COLORRED:
  case SENSOR_COLORGREEN:
  case SENSOR_COLORBLUE:
  case SENSOR_COLORNONE:
    sensor->d_last = type;
    sensor->d_start = start_color;
    break;

  case SENSOR_LIGHT_ACTIVE:
  case SENSOR_LIGHT_INACTIVE:
    sensor->d_last = type;
    sensor->d_start = start_light;
    break;

  default:
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_SENSOR_NOT_AVAILABLE);
  }

  return sensor->d_start(sensor);
}

int lothar_sensor_stop(lothar_sensor_t *sensor)
{
  IS_VALID(sensor);

  sensor->d_type = SENSOR_NO_SENSOR;
  sensor->d_min = 0;
  sensor->d_max = 0;

  return lothar_setinputmode(sensor->d_connection, sensor->d_port, SENSOR_NO_SENSOR, SENSOR_MODE_RAWMODE);
}

int lothar_sensor_value(lothar_sensor_t *sensor, uint16_t *value)
{
  uint8_t valid = 0;
  unsigned i;
  int status;

  IS_VALID(sensor);

  if(sensor->d_type == SENSOR_NO_SENSOR && (status = sensor->d_start(sensor))) // restart the sensor
    return status;

  if(sensor->d_type == SENSOR_LOWSPEED_9V && (status = lothar_lswrite(sensor->d_connection, sensor->d_port, (uint8_t *)"\x02\x42", 2, 1))) // ultrasound messes up my generic design :(
    return status;

  for(i = 0; i < 10; ++i)
  {
    if((status = sensor->d_value(sensor, &valid, value)) < 0)
      return status;

    if(valid)
      return 0;
    
    if((status = lothar_msleep((i + 1) * 10)) < 0) // give the device time to power up
      return status;
  }

  if((status = sensor->d_value(sensor, &valid, value)))
    return status;

  LOTHAR_RETURN_ERROR(LOTHAR_ERROR_TIMEOUT);
}

int lothar_sensor_minimum(lothar_sensor_t const *sensor, uint16_t *min)
{
  IS_VALID(sensor);

  if(min)
    *min = sensor->d_min;

  return 0;
}

int lothar_sensor_maximum(lothar_sensor_t const *sensor, uint16_t *max)
{
  IS_VALID(sensor);

  if(max)
    *max = sensor->d_max;

  return 0;
}

int32_t lothar_sensor_val(lothar_sensor_t *sensor)
{
  uint16_t val = 0;
  int status = lothar_sensor_value(sensor, &val);
  return status < 0 ? status : val;
}

int32_t lothar_sensor_min(lothar_sensor_t const *sensor)
{
  uint16_t min = 0;
  int status = lothar_sensor_minimum(sensor, &min);
  return status < 0 ? status : min;
}

int32_t lothar_sensor_max(lothar_sensor_t const *sensor)
{
  uint16_t max = 0;
  int status = lothar_sensor_maximum(sensor, &max);
  return status < 0 ? status : max;
}

