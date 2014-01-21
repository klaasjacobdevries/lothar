#ifndef LOTHAR_COMMANDS_H
#define LOTHAR_COMMANDS_H

#include "connection.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** \file commands.h 
 *
 * These are direct commands to be given to the brick. Most applications should shoose the higher level functionality
 * defined in motor.h and sensor.h
 *
 * These are one-to-one translations from the direct commands that define the communication with the NXT brick, to that
 * documentation is referred if further details are needed.
 *
 * These return 0 on success and a negative error code on failure (see error_handling.h for the meaning of these errors,
 * the error code is the negative of the lothar_error) if a function takes a non-const pointer as a parameter (other
 * than lothar_connection_t *), it is an output parameter. Pass NULL, if you'r not interested in this output
 */

/** \brief Start a program with the given name.
 */
int lothar_startprogram(lothar_connection_t *connection, char const *filename);

/** \brief Stop the running program
 */
int lothar_stopprogram(lothar_connection_t *connection);

/** \brief Play the sound file with the given name
 *
 * \param loop (boolean) if true, then loop indefinitely
 */
int lothar_playsoundfile(lothar_connection_t *connection, uint8_t loop, char const *filename);

/** \brief Play a constant tone
 *
 * \param frequency The frequency in Hz
 * \param duration  The duration in ms
 */
int lothar_playtone(lothar_connection_t *connection, uint16_t frequency, uint16_t duration);

/** \brief Control an output port
 *
 * \param port           The output port
 * \param power          The power to be applied, in the range (-100, 100)
 * \param motormode      The motor mode used
 * \param regulationmode The regulation mode
 * \param turnration     The turn ratio, range (-100,100)
 * \param runstate       The run state
 * \param tacholimit     How long to run? 0 for running forever
 */
int lothar_setoutputstate(lothar_connection_t *connection, 
			  enum lothar_output_port port, 
			  int8_t power, 
			  enum lothar_output_motor_mode motormode, 
			  enum lothar_output_regulation_mode regulationmode, 
			  int8_t turnratio, 
			  enum lothar_output_runstate runstate, 
			  uint32_t tacholimit);

/** \brief Configure a sensor
 *
 * \param port       The input port
 * \param sensortype The type of the sensor
 * \param sensormode What kind of data does this sensor return?
 */
int lothar_setinputmode(lothar_connection_t *connection,
			 enum lothar_input_port port,
			 enum lothar_sensor_type sensortype,
			 enum lothar_sensor_mode sensormode);

/** \brief Inspect a motor
 *
 * This returns the arguments that were given by lothar_setoutputstate as input, in addition:
 * \param tachocount      Count since the last motor reset
 * \param blocktachocount Position relative to the last programmed movement
 * \param rotationcount   Position relative to the last reset of the rotation sensor
 */
int lothar_getoutputstate(lothar_connection_t *connection,
			  enum lothar_output_port port,
			  int8_t *power,
			  enum lothar_output_motor_mode *motormode,
			  enum lothar_output_regulation_mode *regulationmode,
			  uint8_t *turnratio,
			  enum lothar_output_runstate *runstate,
			  uint32_t *tacholimit,
			  int32_t *tachocount,
			  int32_t *blocktachocount,
			  int32_t *rotationcount);

/** \brief Read a sensor
 *
 * \param port            The input port
 * \param valid           (boolean) true if the data can be considered valid
 * \param calibrated      (boolean) true if a calibration file can be used
 * \param type            The sensor type
 * \param mode            The sensor mode
 * \param rawvalue        The raw, sensor-dependent, value
 * \param normvalue       Normalized value, range (0,1023)
 * \param scaledvalue     The mode-dependent scaled value
 * \param calibratedvalue Currently not used
 */
int lothar_getinputvalues(lothar_connection_t *connection,
			  enum lothar_input_port port,
			  uint8_t *valid,
			  uint8_t *calibrated,
			  enum lothar_sensor_type *type,
			  enum lothar_sensor_mode *mode,
			  uint16_t *rawvalue,
			  uint16_t *normvalue,
			  int16_t *scaledvalue,
			  int16_t *calibratedvalue);

/** \brief Reset a a scaled value
 */
int lothar_resetinputscaledvalue(lothar_connection_t *connection, enum lothar_input_port port);

/** \brief Write a message to an inbox
 *
 * \param inbox the inbox (0-9)
 * \param data  the data to write
 * \param len   the size of the data, max 59
 */
int lothar_messagewrite(lothar_connection_t *connection, uint8_t inbox, uint8_t const *data, size_t len);

/** \brief Reset the motor position
 *
 * \param port     the motor port
 * \param relative (boolean) relative to last movement
 */
int lothar_resetmotorposition(lothar_connection_t *connection, enum lothar_output_port port, uint8_t relative);

/** \brief Get the battery level
 *
 * \param batterylevel The battery level in mV
 */
int lothar_getbatterylevel(lothar_connection_t *connection, uint16_t *batterylevel);

/** \brief Stop sound playback
 */
int lothar_stopsoundplayback(lothar_connection_t *connection);

/** \brief Keep alive
 */
int lothar_keepalive(lothar_connection_t *connection, uint32_t *sleeptime);

/** \brief Get the number of bytes ready to receive
 *
 * \param bytesready the number of bytes ready
 */
int lothar_lsgetstatus(lothar_connection_t *connection, enum lothar_input_port port, uint8_t *bytesready);

/** \brief Write data to an input port
 */
int lothar_lswrite(lothar_connection_t *connection, enum lothar_input_port port, uint8_t const *txdata, uint8_t txlen, uint8_t rxlen);

/** \brief Read data from an input port
 *
 * Use lothar_lsgetstatus to determine the size rxdata should have, or use 16 as a maximum.
 */
int lothar_lsread(lothar_connection_t *connection, enum lothar_input_port port, uint8_t *rxdata, size_t bufsize, uint8_t *rxlen);

/** \brief Get the current program name
 *
 * \param filename This will store the file name.
 */
int lothar_getcurrentprogramname(lothar_connection_t *connection, char filename[19]);

/** \brief Read a message from an inbox
 */
int lothar_messageread(lothar_connection_t *connection, uint8_t remoteinbox, uint8_t localinbox, uint8_t remove, uint8_t data[59], uint8_t *len);

#ifdef __cplusplus
}
#endif

#endif // LOTHAR_COMMANDS_H
