#ifndef LOTHAR_UTILS_HH
#define LOTHAR_UTILS_HH

#include "utils.h"
#include "error_handling.hh"
#include <algorithm>

// a bit dirty, but avoid all tr1/c++11/whatever trickery by redeclaring shared_ptr in the lothar namespace
#ifdef HAVE_SHARED_PTR

#include <memory>

namespace lothar
{
  template <typename T>
  class shared_ptr : public std::shared_ptr<T>
  {
  public:
    explicit shared_ptr(T *p = 0) : std::shared_ptr<T>(p)
    {}
  };
}

#else // HAVE_SHARED_PTR  need the tr1 version

// microsoft has std::tr1::shared_ptr in <memory> by default, others (i.e. gcc) in <tr1/memory>
#if _MSC_VER
#include <memory>
#else
#include <tr1/memory>
#endif

namespace lothar
{
  template <typename T>
  class shared_ptr : public std::tr1::shared_ptr<T>
  {
  public:
    explicit shared_ptr(T *p = 0) : std::tr1::shared_ptr<T>(p)
    {}
  };
}

#endif // HAVE_SHARED_PTR

namespace lothar
{
  /** \brief return the value, but no smaller than min, no greater than max
   */
  template <typename T>
  T const &clamp(T const &val, T const &min, T const &max)
  {
    return (val < min ? min : (val > max ? max : val));
  }

  /** \brief The minimum value of a and b
   */
  template <typename T>
  T const &min(T const &a, T const &b)
  {
    return std::min<T>(a, b);
  }

  /** \brief The maximum value of a and b
   */
  template <typename T>
  T const &max(T const &a, T const  &b)
  {
    return std::max<T>(a, b);
  }

  /** \brief Convert degrees to radians
   */
  template <typename T1, typename T2>
  T1 degtorad(T2 const &deg)
  {
    return static_cast<T1>(deg * M_PI / 180.0);
  }
  
  /** \brief Convert radians to degrees
   */
  template <typename T1, typename T2>
  T1 radtodeg(T2 const &rad)
  {
    T2 t = static_cast<T2>(rad * 180.0 / M_PI);
	return static_cast<T1>(t);
  }

  /** \brief Convert a short from host endianness to NXT (small) endianness
   *
   * \param val a 32 bit integer
   * \param buf a 2 byte buffer to hold the result
   */
  inline void htonxts(uint16_t val, uint8_t buf[2])
  {
    lothar_htonxts(val, buf);
  }

  /** \brief Convert a short from NXT (small) endianness to host endianness
   *
   * \param buf a 2 byte buffer received from the NXT
   * \returns the value in host endianness
   */
  inline uint16_t nxttohs(uint8_t const buf[2])
  {
    return lothar_nxttohs(buf);
  }
  
  /** \brief Convert a long from host endianness to NXT (small) endianness
   *
   * \param val a 32 bit integer
   * \param buf a 4 byte buffer to hold the result
   */
  inline void htonxtl(uint32_t val, uint8_t buf[4])
  {
    lothar_htonxtl(val, buf);
  }
  
  /** \brief Convert a long from NXT (small) endianness to host endianness
   *
   * \param buf a 4 byte buffer received from the NXT
   * \returns the value in host endianness
   */
  inline uint32_t nxttohl(uint8_t const buf[4])
  {
    return lothar_nxttohl(buf);
  }
  
  /** \brief The output ports (A-C, ALL)
   */
  typedef enum lothar_output_port output_port;

  /** \brief The input ports (1-4)
   */
  typedef enum lothar_input_port input_port;

  /** \brief The motor modes
   */
  typedef enum lothar_output_motor_mode output_motor_mode;

  /** \brief The motor regulation modes
   */
  typedef enum lothar_output_regulation_mode output_regulation_mode;

  /** \brief The motor runstates
   */
  typedef enum lothar_output_runstate output_runstate;

  /** \brief Varius types of sensors
   */
  typedef enum lothar_sensor_type sensor_type;

  /** \brief varius input modes */
  typedef enum lothar_sensor_mode sensor_mode;

  /** \brief enum for the SENSOR_COLORFULL detection mode 
   */ 
  typedef enum lothar_color color;

  /** \brief redaclaration in lothar namespace
   */
  typedef lothar_time_t time_t;

  /** \brief sleep for the specified number of milliseconds 
   */
  inline void msleep(lothar_time_t ms)
  {
    check_return(lothar_msleep(ms));
  }

  /** \brief simple timer, returns the number of milliseconds since the last time this function was called (0 if this is
   * the first time this function is called)
   */
  inline time_t time()
  {
    return lothar_time();
  }

  /** \brief slightly more advanced timer.
   *
   * use timer(NULL) to create a new timer, and pass the return value to the next calls to get the number of
   * milliseconds since the creation time
   */
  inline time_t timer(time_t *timer)
  {
    return lothar_timer(timer);
  }

  /** \brief base class for classes that shouldn't allow copying
   */
  class no_copy
  {
    // private and not implemented
    no_copy(no_copy const &);
    no_copy &operator=(no_copy const &);
  public:
    no_copy()
    {}
  };
}

#endif // LOTHAR_UTILS_HH
