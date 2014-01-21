#ifndef LOTHAR_STEERING_HH
#define LOTHAR_STEERING_HH

#include "connection.hh"
#include "utils.hh"
#include "steering.h"

namespace lothar
{
  /** \brief A steering object.
   *
   * The steering object is meant for a vehicle that steers by differing the
   * speed of left and right weels. This object steers the vehicle and by dead
   * reckoning keeps track of the odometry.
   */
  class Steering : public no_copy
  {
    ConnectionPtr      d_connection;
    lothar_steering_t *d_steering;
    
  public:
    /** \brief Constructor
     *
     * The vehicles position (x,y) is initialized to (0,0), its orientation o,
     * which is the angle from x to y in radians is initialized to (0)
     *
     * \param left     The port the left wheel motor is connected to
     * \param right    The port the right wheel motor is connected to
     * \param radius   The radius of the wheels, in m
     * \param distance The distance between the wheels, in m
     * \throws Error if the construction failed.
     */
    Steering(ConnectionPtr &connection, output_port left, output_port right, double radius, double distance) : d_connection(connection), d_steering(lothar_steering_open(*connection, left, right, radius, distance))
    {
      if(!d_steering)
	throw Error();
    }

    /** \brief Destructor
     */
    ~Steering()
    {
      if(d_steering)
	check_return(lothar_steering_close(&d_steering));
    }

    /** \brief Access the underlying lothar_steering_t *
     */
    operator lothar_steering_t const *() const
    {
      return d_steering;
    }
    
    /** \brief Access the underlying lothar_steering_t *
     */
    operator lothar_steering_t *()
    {
      return d_steering;
    }
    
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

    /** \brief Go forward
     *
     * \param speed The speed in m/s
     */
    void forward(double speed)
    {
      check_return(lothar_steering_forward(*this, speed));
    }

    /** \brief Turn
     *
     * \param speed     The speed in m/s
     * \param turnspeed The speed of turning
     */
    void turn(double speed, double turnspeed)
    {
      check_return(lothar_steering_turn(*this, speed, turnspeed));
    }

    /** \brief Stop
     */
    void stop()
    {
      check_return(lothar_steering_stop(*this));
    }

    /** \brief Brake
     */
    void brake()
    {
      check_return(lothar_steering_brake(*this));
    }

    /** \brief Retrieve the odemetry
     *
     * \note You may want to call update() before this to recalculate the
     * odometry
     */
    void get_odometry(double &x, double &y, double &o) const
    {
      check_return(lothar_steering_get_odometry(*this, &x, &y, &o));
    }

    /** \brief Manually set the odeometry
     */
    void set_odometry(double x, double y, double o)
    {
      check_return(lothar_steering_set_odometry(*this, x, y, o));
    }

    /** \brief Force a recalculation of the internal odemetry
     */
    void update()
    {
      check_return(lothar_steering_update(*this));
    }

    /** \brief Retrieve x
     */
    double x() const
    {
      return lothar_steering_x(*this);
    }

    /** \brief Retrieve y
     */
    double y() const
    {
      return lothar_steering_y(*this);
    }
    
    /** \brief Retrieve o
     */
    double o() const
    {
      return lothar_steering_o(*this);
    }
  };
}

#endif // LOTHAR_STEERING_HH
