#ifndef LOTHAR_MOTOR_HH
#define LOTHAR_MOTOR_HH

#include "motor.h"
#include "utils.hh"
#include "connection.hh"

namespace lothar
{
  /** \brief Motor class
   *
   * This class holds on to an instance of the shared_ptr that is used to create
   * it, thus ensuring the connection is not destroyed before this class is.
   */
  class Motor : public no_copy
  {
    ConnectionPtr   d_connection;
    lothar_motor_t *d_motor;

  public:
    /** \brief Constructor
     *
     * \param connection The connection used
     * \param port       The output port that the motor is connected to
     * \throws Error if the creation failed.
     */
    Motor(ConnectionPtr &connection, output_port port) : d_connection(connection), d_motor(lothar_motor_open(*connection, port))
    {      
      if(!d_motor)
	throw Error();
    }

    /** \brief Destructor
     */
    ~Motor()
    {
      if(d_motor)
	check_return(lothar_motor_close(&d_motor));
    }

    /** \brief Access the underlying lothar_motor_t *
     */
    operator lothar_motor_t const *() const
    {
      return d_motor;
    }
    
    /** \brief Access the underlying lothar_motor_t *
     */
    operator lothar_motor_t *()
    {
      return d_motor;
    }
    
    /** \brief Return the port that this motor is connected to
     */
    output_port port() const;

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

    /** \brief turn the motor on
     *
     * Note that this function returns inmediatly, leaving the motor running
     * (i.e. this does not block)
     *
     * \param power the power level, between -100 and 100
     */
    void run(int8_t power)
    {
      check_return(lothar_motor_run(*this, power));
    }

    /** \brief run for a given number of milliseconds
     *
     * This blocks until the running is done
     *
     * \param power The power level between -100 and 100
     * \param ms    The time in ms
     */
    void run_block(int8_t power, time_t ms)
    {
      check_return(lothar_motor_run_block(*this, power, ms));
    }

    /** \brief Turn the motor by the given number of degrees.
     *
     * Specify power as a negative number to turn backwards.  Note that 'turning
     * forwards' means the negative (clockwise direction) (no, I did not think
     * of this myself, lego did).  (this does not block)
     *
     * Note that, espessialy with higher power levels, this may overshoot, which
     * may cause a turn not to be executed for example, I order a turn of 90
     * degrees, followed by a turn of 10 degrees. What may happen is
     * this. First, 90 degrees is ordered, but in reallity 105 degrees is
     * turned. Now the second call does nothing, as the total 100 degrees are
     * already done
     *
     * use lothar_motor_turn_block if you don't want this.
     *
     * \param power   The power level (-100,100)
     * \param degrees The number of degrees to turn.
     */
    void turn(int8_t power, uint32_t degrees)
    {
      check_return(lothar_motor_turn(*this, power, degrees));
    }

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
     * \param margin  The margin, set to small but nonzero to prevent blocking
     * \param poll    The polling interval in ms
     * \param timeout The timeout, specify 0 for infinite.
     */
    void turn_block(int8_t power, uint32_t degrees, uint32_t margin = 0, time_t poll = 10, time_t timeout = 0)
    {
      check_return(lothar_motor_turn_block(*this, power, degrees, margin, poll, timeout));
    }

    /** \brief Do a given number of rotations, non blocking.
     */
    void rotate(int8_t power, float nturns)
    {
      check_return(lothar_motor_rotate(*this, power, nturns));
    }
  
    /** \brief Do a given number of rotations, blocking.
     *
     * \param power   The power level (-100, 100)
     * \param nturns  The number of rotations
     * \param margin  The margin, set to small but nonzero to prevent blocking
     * \param poll    The polling interval (ms)
     * \param timeout The timeout (ms). Pass 0 for infinite
     */
    void rotate_block(int8_t power, float nturns, float margin = 0, time_t poll = 10, time_t timeout = 0)
    {
      check_return(lothar_motor_rotate_block(*this, power, nturns, margin, poll, timeout));
    }
    
    /** \brief Stop the motor. 
     *
     * Note that this just stops, this is not an actual brake.
     */
    void stop()
    {
      check_return(lothar_motor_stop(*this));
    }
    
    /** \brief Actual brake.
     */
    void brake()
    {
      check_return(lothar_motor_brake(*this));
    }
    
    /** \brief Reset the internal degree counter
     *
     * \param relative (boolean) if the relative counter is meant
     */
    void reset(bool relative)
    {
      check_return(lothar_motor_reset(*this, relative));
    }
  
    /** \brief Get the motor turn position 
     *
     * \param degrees  The number of degrees.
     * \param relative (boolean) whether absolute or relative position is meant
     */
    int32_t degrees(bool relative);
    
    /** \brief Get the power level 
     */
    int8_t power();
  };
  
  inline void sync(Motor &motor1, Motor &motor2, bool reset = true)
  {
    check_return(lothar_motor_sync(motor1, motor2, reset));
  }

  inline void unsync(Motor &motor1, Motor &motor2)
  {
    check_return(lothar_motor_unsync(motor1, motor2));
  }
}

#endif // LOTHAR_MOTOR_HH
