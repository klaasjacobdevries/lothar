#ifndef LOTHAR_COMMANDS_HH
#define LOTHAR_COMMANDS_HH

#include "commands.h"
#include "connection.hh"
#include "utils.hh"

/** \file commands.hh
 *
 * These are just redeclarations of the functions in commands.h in the lothar namespace
 *
 * These are direct commands to be given to the brick. Most applications should shoose the higher level functionality
 * defined in motor.hh and sensor.hh
 *
 * These are one-to-one translations from the direct commands that define the communication with the NXT brick, to that
 * documentation is referred if further details are needed.
 *
 * These throw lothar::Error on failure.
 * if a function takes a non-const pointer as a parameter (other
 * than lothar_connection_t *), it is an output parameter. Pass NULL, if you'r not interested in this output
 */

namespace lothar
{

  /** \brief Start a program with the given name.
   */
  inline void startprogram(Connection &connection, char const *filename)
  {
    check_return(lothar_startprogram(connection, filename));
  }

  /** \brief Stop the running program
   */
  inline void stopprogram(Connection &connection)
  {
    check_return(lothar_stopprogram(connection));
  }

  /** \brief Play the sound file with the given name
   *
   * \param loop (boolean) if true, then loop indefinitely
   */
  inline void playsoundfile(Connection &connection, bool loop, char const *filename)
  {
    check_return(lothar_playsoundfile(connection, loop, filename));
  }

  /** \brief Play a constant tone
   *
   * \param frequency The frequency in Hz
   * \param duration  The duration in ms
   */
  inline void playtone(Connection &connection, uint16_t frequency, uint16_t duration)
  {
    check_return(lothar_playtone(connection, frequency, duration));
  }

  /** \brief Control an output port
   *
   * \param port           The output port
   * \param power          The power to be applied, in the range (-100, 100)
   * \param motormode      The motor mode used
   * \param regulationmode The regulation mode
   * \param turnratio      The turn ratio, range (-100,100)
   * \param runstate       The run state
   * \param tacholimit     How long to run? 0 for running forever
   */
  inline void setoutputstate(Connection &connection, 
                             output_port port, 
                             int8_t power, 
                             output_motor_mode motormode, 
                             output_regulation_mode regulationmode, 
                             int8_t turnratio, 
                             output_runstate runstate, 
                             uint32_t tacholimit)
  {
    check_return(lothar_setoutputstate(connection,
                                       port,
                                       power,
                                       motormode,
                                       regulationmode,
                                       turnratio,
                                       runstate,
                                       tacholimit));
  }

  /** \brief Configure a sensor
   *
   * \param port       The input port
   * \param sensortype The type of the sensor
   * \param sensormode What kind of data does this sensor return?
   */
  inline void setinputmode(Connection &connection,
                           input_port port,
                           sensor_type sensortype,
                           sensor_mode sensormode)
  {
    check_return(lothar_setinputmode(connection,
                                     port,
                                     sensortype,
                                     sensormode));
  }

  /** \brief Inspect a motor
   *
   * This returns the arguments that were given by lothar_setoutputstate as input, in addition:
   * \param tachocount      Count since the last motor reset
   * \param blocktachocount Position relative to the last programmed movement
   * \param rotationcount   Position relative to the last reset of the rotation sensor
   */
  inline void getoutputstate(Connection &connection,
                             output_port port,
                             int8_t *power = NULL,
                             output_motor_mode *motormode = NULL,
                             output_regulation_mode *regulationmode = NULL,
                             uint8_t *turnratio = NULL,
                             output_runstate *runstate = NULL,
                             uint32_t *tacholimit = NULL,
                             int32_t *tachocount = NULL,
                             int32_t *blocktachocount = NULL,
                             int32_t *rotationcount = NULL)
  {
    check_return(lothar_getoutputstate(connection,
                                       port,
                                       power,
                                       motormode,
                                       regulationmode,
                                       turnratio,
                                       runstate,
                                       tacholimit,
                                       tachocount,
                                       blocktachocount,
                                       rotationcount));
  }

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
  void getinputvalues(Connection &connection,
                      input_port port,
                      bool *valid = NULL,
                      bool *calibrated = NULL,
                      sensor_type *type = NULL,
                      sensor_mode *mode = NULL,
                      uint16_t *rawvalue = NULL,
                      uint16_t *normvalue = NULL,
                      int16_t *scaledvalue = NULL,
                      int16_t *calibratedvalue = NULL);

  /** \brief Reset a a scaled value
   */
  inline void resetinputscaledvalue(Connection &connection, input_port port)
  {
    check_return(lothar_resetinputscaledvalue(connection, port));
  }
  
  /** \brief Write a message to an inbox
   *
   * \param inbox the inbox (0-9)
   * \param data  the data to write
   * \param len   the size of the data, max 59
   */
  inline void messagewrite(Connection &connection, uint8_t inbox, uint8_t const *data, size_t len)
  {
    check_return(lothar_messagewrite(connection, inbox, data, len));
  }

  /** \brief Reset the motor position
   *
   * \param port     the motor port
   * \param relative (boolean) relative to last movement
   */
  inline void resetmotorposition(Connection &connection, output_port port, bool relative)
  {
    check_return(lothar_resetmotorposition(connection, port, relative));
  }

  /** \brief Get the battery level
   *
   * \param batterylevel The battery level in mV
   */
  inline void getbatterylevel(Connection &connection, uint16_t *batterylevel)
  {
    check_return(lothar_getbatterylevel(connection, batterylevel));
  }

  /** \brief Get the battery level in mV
   */
  inline uint16_t getbatterylevel(Connection &connection)
  {
    uint16_t result;
    getbatterylevel(connection, &result);
    return result;
  }

  /** \brief Stop sound playback
   */
  inline void stopsoundplayback(Connection &connection)
  {
    check_return(lothar_stopsoundplayback(connection));
  }

  /** \brief Keep alive
   */
  inline void keepalive(Connection &connection, uint32_t *sleeptime)
  {
    check_return(lothar_keepalive(connection, sleeptime));
  }

  inline uint32_t keepalive(Connection &connection)
  {
    uint32_t result;
    keepalive(connection, &result);
    return result;
  }

  /** \brief Get the number of bytes ready to receive
   *
   * \param bytesready the number of bytes ready
   */
  inline void lsgetstatus(Connection &connection, input_port port, uint8_t *bytesready)
  {
    check_return(lothar_lsgetstatus(connection, port, bytesready));
  }

  inline uint8_t lsgetstatus(Connection &connection, input_port port)
  {
    uint8_t result;
    lsgetstatus(connection, port, &result);
    return result;
  }

  /** \brief Write data to an input port
   */
  inline void lswrite(Connection &connection, input_port port, uint8_t const *txdata, uint8_t txlen, uint8_t rxlen)
  {
    check_return(lothar_lswrite(connection, port, txdata, txlen, rxlen));
  }

  /** \brief Read data from an input port
   *
   * Use lothar_lsgetstatus to determine the size rxdata should have, or use 16 as a maximum.
   */
  inline void lsread(Connection &connection, input_port port, uint8_t *rxdata, size_t bufsize, uint8_t *rxlen)
  {
    check_return(lothar_lsread(connection, port, rxdata, bufsize, rxlen));
  }

  /** \brief Get the current program name
   *
   * \param filename This will store the file name.
   * \param bufsize  The size of filename, should be 19.
   */
  inline void getcurrentprogramname(Connection &connection, char filename[19])
  {
    check_return(lothar_getcurrentprogramname(connection, filename));
  }

  /** \brief Read a message from an inbox
   */
  inline void messageread(Connection &connection, uint8_t remoteinbox, uint8_t localinbox, uint8_t remove, uint8_t data[59], uint8_t *len)
  {
    check_return(lothar_messageread(connection, remoteinbox, localinbox, remove, data, len));
  }
}

#endif // LOTHAR_COMMANDS_HH
