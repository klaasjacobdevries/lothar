#ifndef LOTHAR_SENSOR_HH
#define LOTHAR_SENSOR_HH

#include "sensor.h"
#include "utils.hh"
#include "connection.hh"

namespace lothar
{
  /** \brief Base class for sensors.
   *
   * Though there is nothing wrong with using this directly, you should really
   * use on of the derived types
   */
  class Sensor : public no_copy
  {
    ConnectionPtr    d_connection;
    lothar_sensor_t *d_sensor;

  protected:
    /** \brief Reset the type
     *
     * Though I guess you can quickly replug the sensor, the only normal use of this
     * is switching between different color-sensor-types.
     * As the only legitimate usage for this is in the derived ColorSensor class, this is declared protected
     */
    void reset(sensor_type type)
    {
      check_return(lothar_sensor_reset(*this, type));
    }

    
  public:
    /** \brief Open the sensor of the specified type 
     *
     * \param port The port the sensor is connected to
     * \param type The type of sensor
     */
    Sensor(ConnectionPtr &connection, input_port port, sensor_type type) : d_connection(connection), d_sensor(lothar_sensor_open(*connection, port, type))
    {
      if(!d_sensor)
	throw Error();
    }

    ~Sensor()
    {
      if(d_sensor)
	check_return(lothar_sensor_close(&d_sensor));
    }

    /** \brief Access the underlying lothar_sensor_t *
     */
    operator lothar_sensor_t const *() const
    {
      return d_sensor;
    }
    
    /** \brief Access the underlying lothar_sensor_t *
     */
    operator lothar_sensor_t *()
    {
      return d_sensor;
    }
    
    /** \brief Return the port that this sensor is connected to
     */
    input_port port() const;

    /** \brief The type of the sensor
     */
    sensor_type type() const;

    /** \brief Return the underlying connection
     */
    ConnectionPtr const &connection() const
    {
      return d_connection;
    }

    /** \brief Return the underlying connection
     */
    ConnectionPtr &connection()
    {
      return d_connection;
    }

    /** \brief Put the sensor to sleep. 
     *
     * The next call to value() will automatically restart the sensor.
     */
    void stop()
    {
      check_return(lothar_sensor_stop(*this));
    }
    
    /** \brief get the value of the sensor 
     */
    uint16_t value();
    uint16_t val()
    {
      return value();
    }
    
    /** \brief The minimum value that the sensor can return.
     */
    uint16_t minimum() const;
    uint16_t min() const
    {
      return minimum();
    }
    
    /** \brief The maximum value that the sensor can return.
     */
    uint16_t maximum() const;
    uint16_t max() const
    {
      return maximum();
    }
  };

  /** \brief Switch sensor
   */
  class SwitchSensor : public Sensor
  {
  public:
    SwitchSensor(ConnectionPtr &connection, input_port port) : Sensor(connection, port, SENSOR_SWITCH)
    {}

    /** \brief Returns true if the switch is pressed
     */
    bool on()
    {
      return value() > minimum();
    }
  };
  
  /** \brief UltraSound sensor
   */
  class UltraSoundSensor : public Sensor
  {
  public:
    UltraSoundSensor(ConnectionPtr &connection, input_port port) : Sensor(connection, port, SENSOR_LOWSPEED_9V)
    {}

    uint16_t distance()
    {
      return value();
    }
  };

  /** \brief Color sensor
   *
   * This is the generic color sensor, meant for switching between different modes. You may also want to use the
   * specialized color sensors
   */
  class ColorSensor : public Sensor
  {
  public:
    /** \brief Map modes to color sensor types
     */
    typedef enum {MODE_NONE  = SENSOR_COLORNONE,
                  MODE_RED   = SENSOR_COLORRED,
                  MODE_GREEN = SENSOR_COLORGREEN,
                  MODE_BLUE  = SENSOR_COLORBLUE,
                  MODE_FULL  = SENSOR_COLORFULL} color_mode;

    /** \brief Constructor
     *
     * \param mode a color mode.
     */
    ColorSensor(ConnectionPtr &connection, input_port port, color_mode mode = MODE_NONE) : Sensor(connection, port, static_cast<sensor_type>(mode))
    {}

    /** \brief Get the color mode used.
     */
    color_mode get_mode() const
    {
      return static_cast<color_mode>(type());
    }

    /** \brief Set the color mode to use
     */
    void set_mode(color_mode mode)
    {
      reset(static_cast<sensor_type>(mode));
    }
        
    /** \brief Test wether a sensor type is a valid color sensor type
     */
    static bool is_colortype(sensor_type type)
    {
      return
        type == SENSOR_COLORRED ||
        type == SENSOR_COLORGREEN ||
        type == SENSOR_COLORBLUE ||
        type == SENSOR_COLORFULL ||
        type == SENSOR_COLORNONE;
    }
  };

  /** \brief Full Color sensor
   *
   * This really does nothing for you except return a lothar::color for value
   */
  class FullColorSensor : public ColorSensor
  {
  public:
    FullColorSensor(ConnectionPtr &connection, input_port port) : ColorSensor(connection, port, MODE_FULL)
    {}

    lothar::color color()
    {
      return static_cast<lothar::color>(value());
    }
  };

  /** \brief Light sensor
   */
  class LightSensor : public Sensor
  {
  public:
    /** \brief Constructor
     *
     * \param active Wether this is an active or inactive light sensor
     */
    LightSensor(ConnectionPtr &connection, input_port port, bool active) : Sensor(connection, port, active ? SENSOR_LIGHT_ACTIVE : SENSOR_LIGHT_INACTIVE)
    {}

    /** \brief Wether the sensor is active
     */
    bool is_active() const
    {
      return type() == SENSOR_LIGHT_ACTIVE;
    }

    /** \brief (de)active the light
     */
    void set_active(bool active)
    {
      reset(active ? SENSOR_LIGHT_ACTIVE : SENSOR_LIGHT_INACTIVE);
    }
  };
}

#endif // LOTHAR_SENSOR_HH
