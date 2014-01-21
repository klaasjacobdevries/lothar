#include "motor.h"
#include "commands.h"

#define IS_VALID(m) { if(!m) { LOTHAR_FAIL("invalid motor\n"); LOTHAR_RETURN_ERROR(LOTHAR_ERROR_ENTITY_CLOSED);} }

struct lothar_motor_t
{
  lothar_connection_t *d_connection;
  enum lothar_output_port d_port;
  enum lothar_output_regulation_mode d_regulation;
};

lothar_motor_t *lothar_motor_open(lothar_connection_t *connection, enum lothar_output_port port)
{
  lothar_motor_t *result;

  switch(port)
  {
  case OUTPUT_A:
  case OUTPUT_B:
  case OUTPUT_C:
    break; // okay

  default:
    LOTHAR_ERROR(LOTHAR_ERROR_INVALID_ARGUMENT);
    return NULL;
  }

  result = (lothar_motor_t *)lothar_malloc(sizeof(lothar_motor_t));

  result->d_connection = connection;
  result->d_port       = port;
  result->d_regulation = REGULATION_MODE_SPEED;

  return result;
}

int lothar_motor_close(lothar_motor_t **motor)
{
  int status;
  IS_VALID(*motor);

  status = lothar_motor_stop(*motor);

  free(*motor);
  *motor = NULL;

  return status;
}

int lothar_motor_connection(lothar_motor_t const *motor, lothar_connection_t **connection)
{
  IS_VALID(motor);

  if(connection)
    *connection = motor->d_connection;

  return 0;
}

int lothar_motor_port(lothar_motor_t const *motor, enum lothar_output_port *port)
{
  IS_VALID(motor);
  
  if(port)
    *port = motor->d_port;

  return 0;
}

int lothar_motor_run(lothar_motor_t *motor, int8_t power)
{
  IS_VALID(motor);

  return lothar_setoutputstate(motor->d_connection, 
			       motor->d_port, 
			       CLAMP(power, -100, 100),
			       MOTOR_MODE_MOTORON | MOTOR_MODE_BRAKE | MOTOR_MODE_REGULATED,
			       motor->d_regulation,
			       0,
			       RUNSTATE_RUNNING,
			       0);
}

int lothar_motor_run_block(lothar_motor_t *motor, int8_t power, lothar_time_t ms)
{
  int status;

  IS_VALID(motor);
  
  if((status = lothar_motor_run(motor, power)) < 0)
    return status;

  if((status = lothar_msleep(ms)) < 0)
    return status;

  return lothar_motor_brake(motor);
}

int lothar_motor_turn(lothar_motor_t *motor, int8_t power, uint32_t degrees)
{  
  IS_VALID(motor);

  return lothar_setoutputstate(motor->d_connection, 
			       motor->d_port, 
			       CLAMP(power, -100, 100),
                               MOTOR_MODE_MOTORON | MOTOR_MODE_BRAKE | MOTOR_MODE_REGULATED,
			       motor->d_regulation,
			       0,
			       RUNSTATE_RUNNING,
			       degrees);
}

int lothar_motor_turn_block(lothar_motor_t *motor, int8_t power, uint32_t degrees, uint32_t margin, lothar_time_t poll, lothar_time_t timeout)
{
  int status;
  int32_t target_degrees;
  lothar_time_t timer = lothar_timer(NULL);

  IS_VALID(motor);

  LOTHAR_DEBUG("turn power=%d, degrees=%d, margin=%d, poll=%d, timeout=%d\n", (int)power, (int)degrees, (int)margin, (int)poll, (int)timeout);

  if(margin >= degrees)
  {
    LOTHAR_WARN("The margin is larger that the absolute value (%d vs %d), this does not make sense, returning inmediately.\n", margin, degrees);
    return 0;
  }

  if((status = lothar_motor_degrees(motor, &target_degrees, 0)) < 0)
    return status;

  if(power > 0)
    target_degrees += (degrees - margin);
  else if(power < 0)
    target_degrees -= (degrees - margin);
  else
    return 0;

  if((status = lothar_motor_turn(motor, power, degrees)) < 0)
    return status;

  while(!timeout || lothar_timer(&timer) < timeout)
  {
    int32_t current_degrees;
    if((status = lothar_motor_degrees(motor, &current_degrees, 0)) < 0)
      return status;

    if((power > 0 && current_degrees >= target_degrees) || (power < 0 && current_degrees <= target_degrees))
      return lothar_motor_brake(motor);

    if(poll)
    {
      if((status = lothar_msleep(poll)) < 0)
	return status;
    }
  }

  // timed out
  lothar_motor_brake(motor);
 
  LOTHAR_RETURN_ERROR(LOTHAR_ERROR_TIMEOUT);
}

int lothar_motor_stop(lothar_motor_t *motor)
{
  IS_VALID(motor);

  return lothar_setoutputstate(motor->d_connection, 
			       motor->d_port,
			       0,
			       0,
			       REGULATION_MODE_IDLE,
			       0,
			       RUNSTATE_IDLE,
			       0);
}

int lothar_motor_brake(lothar_motor_t *motor)
{
  IS_VALID(motor);

  return lothar_setoutputstate(motor->d_connection,
			       motor->d_port,
			       0,
			       MOTOR_MODE_MOTORON | MOTOR_MODE_BRAKE | MOTOR_MODE_REGULATED, 
			       motor->d_regulation,
			       0,
			       RUNSTATE_RUNNING,
			       0);
}

int lothar_motor_reset(lothar_motor_t *motor, uint8_t relative)
{
  IS_VALID(motor);

  return lothar_resetmotorposition(motor->d_connection, motor->d_port, relative);
}

int lothar_motor_degrees(lothar_motor_t *motor, int32_t *degrees, uint8_t relative)
{
  IS_VALID(motor);

  return lothar_getoutputstate(motor->d_connection,
			       motor->d_port,
			       NULL, NULL, NULL, NULL, NULL, NULL, NULL,
			       relative ? degrees : NULL,
			       relative ? NULL : degrees);
}

int lothar_motor_power(lothar_motor_t *motor, int8_t *power)
{
  IS_VALID(motor);

  return lothar_getoutputstate(motor->d_connection,
			       motor->d_port,
			       power,
			       NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

int lothar_motor_sync(lothar_motor_t *motor1, lothar_motor_t *motor2, uint8_t reset)
{
  IS_VALID(motor1);
  IS_VALID(motor2);
  
  if(reset)
  {
    lothar_motor_reset(motor1, 0);
    lothar_motor_reset(motor2, 0);
    lothar_motor_reset(motor1, 1);
    lothar_motor_reset(motor2, 1);
  }

  motor1->d_regulation = REGULATION_MODE_MOTOR_SYNC;
  motor2->d_regulation = REGULATION_MODE_MOTOR_SYNC;

  return 0;
}

int lothar_motor_unsync(lothar_motor_t *motor1, lothar_motor_t *motor2)
{
  IS_VALID(motor1);
  IS_VALID(motor2);

  motor1->d_regulation = REGULATION_MODE_SPEED;
  motor2->d_regulation = REGULATION_MODE_SPEED;

  return 0;
}
