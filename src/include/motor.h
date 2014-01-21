#ifndef LOTHAR_MOTOR_H
#define LOTHAR_MOTOR_H

#include "connection.h"
#include "config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** \brief opaque data structure describing a motor 
 */
struct lothar_motor_t;
typedef struct lothar_motor_t lothar_motor_t;
  
/* open a motor connection */
lothar_motor_t *lothar_motor_open(lothar_connection_t *connection, enum lothar_output_port port);

/* 'close' the motor (note that the caller will still have to close the connection) */
int lothar_motor_close(lothar_motor_t **motor);

/* return the underlying connection this motor is created with */
int lothar_motor_connection(lothar_motor_t const *motor, lothar_connection_t **connection);

/* return the port of this motor */
int lothar_motor_port(lothar_motor_t const *motor, enum lothar_output_port *port);

/** \brief turn the motor on
 *
 * Note that this function returns inmediatly, leaving the motor running
 * (i.e. this does not block)
 *
 * \param power the power level, between -100 and 100
 */
int lothar_motor_run(lothar_motor_t *motor, int8_t power);

/** \brief run for a given number of milliseconds
 *
 * This blocks until the running is done
 *
 * \param power The power level between -100 and 100
 * \param ms    The time in ms
 */
int lothar_motor_run_block(lothar_motor_t *motor, int8_t power, lothar_time_t ms);

/** \brief Turn the motor by the given number of degrees.
 *
 * Specify power as a negative number to turn backwards.  Note that 'turning
 * forwards' means the negative (clockwise direction) (no, I did not think of
 * this myself, lego did).  (this does not block)
 *
 * Note that, espessialy with higher power levels, this may overshoot, which may
 * cause a turn not to be executed for example, I order a turn of 90 degrees,
 * followed by a turn of 10 degrees. What may happen is this. First, 90 degrees
 * is ordered, but in reallity 105 degrees is turned. Now the second call does
 * nothing, as the total 100 degrees are already done
 *
 * use lothar_motor_turn_block if you don't want this.
 *
 * \param power   The power level (-100,100)
 * \param degrees The number of degrees to turn.
 */
int lothar_motor_turn(lothar_motor_t *motor, int8_t power, uint32_t degrees);

/** \brief Turn the motor by the given number of degrees, then brake. 
 *
 * Note that this blocks, the function does not return before the action is
 * completed the poll parameter specifies how often (interval in milliseconds to
 * test whether the end condition is met.  Good values depend on your power
 * level and need for accuracy, but I guess arround 10 is a good number.  Pass 0
 * to keep testing all the time.
 *
 * \param power   The power level (-100, 100)
 * \param degrees The number of degrees to turn
 * \param margin  The margin for the degrees. If 0, this function may block while just 1 or 2 more degrees to go.
 * \param poll    The polling interval in ms
 * \param timeout The timeout, specify 0 for infinite.
 */
int lothar_motor_turn_block(lothar_motor_t *motor, int8_t power, uint32_t degrees, uint32_t margin, lothar_time_t poll /* ms */, lothar_time_t timout /* ms */);

/** \brief Do a given number of rotations, non blocking.
 */
static inline int lothar_motor_rotate(lothar_motor_t *motor, int8_t power, float nturns)
{
  return lothar_motor_turn(motor, power, (uint32_t)(nturns * 360));
}

/** \brief Do a given number of rotations, blocking.
 *
 * \param power   The power level (-100, 100)
 * \param nturns  The number of rotations
 * \param margin  The margin, as a fraction of nturns
 * \param poll    The polling interval (ms)
 * \param timeout The timeout (ms). Pass 0 for infinite
 */
static inline int lothar_motor_rotate_block(lothar_motor_t *motor, int8_t power, float nturns, float margin, lothar_time_t poll /* ms */, lothar_time_t timeout /* ms */)
{
  return lothar_motor_turn_block(motor, power, (uint32_t)(nturns * 360), (uint32_t)(nturns * 360 * margin), poll, timeout);
}

/** \brief Stop the motor. 
 *
 * Note that this just stops, this is not an actual brake.
 */
int lothar_motor_stop(lothar_motor_t *motor);

/** \brief Actual brake.
 */
int lothar_motor_brake(lothar_motor_t *motor);

/** \brief Reset the internal degree counter
 *
 * \param relative (boolean) if the relative counter is meant
 */
int lothar_motor_reset(lothar_motor_t *motor, uint8_t relative);

/* some basic diagnostices. Use lothar_getoutputstatus if you need more detailled information */

/** \brief Get the motor turn position 
 *
 * \param degrees  The number of degrees.
 * \param relative (boolean) whether absolute or relative position is meant
 */
int lothar_motor_degrees(lothar_motor_t *motor, int32_t *degrees, uint8_t relative);

/** \brief Get the power level 
 */
int lothar_motor_power(lothar_motor_t *motor, int8_t *power);

/** \brief Synchronized two motors.
 * \param reset If true the internal degree counters of the motors will be reset.
 *              This will prevent any current differenes from affecting future behavior.
 */
  int lothar_motor_sync(lothar_motor_t *motor1, lothar_motor_t *motor2, uint8_t reset);

/** \brief Unsyncs two motors
 */
int lothar_motor_unsync(lothar_motor_t *motor1, lothar_motor_t *motor2);

#ifdef __cplusplus
}
#endif

#endif
