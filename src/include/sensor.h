#ifndef LOTHAR_SENSOR_H
#define LOTHAR_SENSOR_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "connection.h"

struct lothar_sensor_t;
typedef struct lothar_sensor_t lothar_sensor_t;

/** \brief Open the sensor of the specified type 
 *
 * \param port The port the sensor is connected to
 * \param type The type of sensor
 */
lothar_sensor_t *lothar_sensor_open(lothar_connection_t *connection, enum lothar_input_port port, enum lothar_sensor_type type);

/** \brief close the sensor 
 */
int lothar_sensor_close(lothar_sensor_t **sensor);

/** \brief get the underlying connection 
 */
int lothar_sensor_connection(lothar_sensor_t const *sensor, lothar_connection_t **connection);

/** \brief Return the port that this sensor is connected to
 */
int lothar_sensor_port(lothar_sensor_t const *sensor, enum lothar_input_port *port);

/** \brief The type of the sensor
 */
int lothar_sensor_type(lothar_sensor_t const *sensor, enum lothar_sensor_type *type);

/** \brief Reset the type
 *
 * Though I guess you can quickly replug the sensor, the only normal use of this
 * is switching between different color-sensor-types.
 */
int lothar_sensor_reset(lothar_sensor_t *sensor, enum lothar_sensor_type type);

/** \brief Put the sensor to sleep. 
 *
 * The next call to lothar_sensor_value() will automatically restart the sensor
 */
int lothar_sensor_stop(lothar_sensor_t *sensor);

/** \brief get the value of the sensor 
 */
int lothar_sensor_value(lothar_sensor_t *sensor, uint16_t *value);

/** \brief The minimum value that the sensor can return.
 */
int lothar_sensor_minimum(lothar_sensor_t const *sensor, uint16_t *min);

/** \brief The maximum value that the sensor can return.
 */
int lothar_sensor_maximum(lothar_sensor_t const *sensor, uint16_t *max);

/* these are a slight breach in consistency, but for your convienience (return -lothar_errno on error, and the value of the sensor/min/max on success)*/
int lothar_sensor_val(lothar_sensor_t *sensor);
int lothar_sensor_min(lothar_sensor_t const *sensor);
int lothar_sensor_max(lothar_sensor_t const *sensor);

#ifdef __cplusplus
}
#endif

#endif
