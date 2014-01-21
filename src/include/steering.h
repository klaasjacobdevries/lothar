#ifndef LOTHAR_STEERING_H
#define LOTHAR_STEERING_H

#include "connection.h"
#include "motor.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** \brief opaque data structure 
 */
struct lothar_steering_t;
typedef struct lothar_steering_t lothar_steering_t;

/** \brief Open a steering object.
 *
 * The steering object is meant for a vehicle that steers by differing the speed
 * of left and right weels. This object steers the vehicle and by dead reckoning
 * keeps track of the odometry.  
 *
 * The vehicles position (x,y) is initialized to (0,0), its orientation o, which
 * is the angle from x to y in radians is initialized to (0)
 *
 * \param left     The port the left wheel motor is connected to
 * \param right    The port the right wheel motor is connected to
 * \param radius   The radius of the wheels, in m
 * \param distance The distance between the wheels, in m
 */
lothar_steering_t *lothar_steering_open(lothar_connection_t *connection, enum lothar_output_port left, enum lothar_output_port right, double radius, double distance);

/** \brief Close the steering object
 */
int lothar_steering_close(lothar_steering_t **steering);

/** \brief Return the underlying connection
 */
int lothar_steering_connection(lothar_steering_t const *steering, lothar_connection_t **connection);

/** \brief Go forward
 *
 * \param speed The speed in m/s
 */
int lothar_steering_forward(lothar_steering_t *steering, double speed);

/** \brief Turn
 *
 * \param speed     The speed in m/s
 * \param turnspeed The speed of turning
 */
int lothar_steering_turn(lothar_steering_t *steering, double speed, double turnspeed);

/** \brief Stop
 */
int lothar_steering_stop(lothar_steering_t *steering);

/** \brief Brake
 */
int lothar_steering_brake(lothar_steering_t *steering);

/** \brief Retrieve the odemetry
 *
 * \note You may want to call lothar_steering_update() before this to
 * recalculate the odometry
 */
int lothar_steering_get_odometry(lothar_steering_t const *steering, double *x, double *y, double *o);

/** \brief Manually set the odeometry
 */
int lothar_steering_set_odometry(lothar_steering_t *steering, double x, double y, double o);

/** \brief Force a recalculation of the internal odemetry
 */
int lothar_steering_update(lothar_steering_t *steering);

/** \brief Retrieve x
 */
double lothar_steering_x(lothar_steering_t const *steering);

/** \brief Retrieve y
 */
double lothar_steering_y(lothar_steering_t const *steering);
  
/** \brief Retrieve o
 */
double lothar_steering_o(lothar_steering_t const *steering);

#ifdef __cplusplus
}
#endif

#endif
