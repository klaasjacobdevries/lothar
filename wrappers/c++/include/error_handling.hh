#ifndef LOTHAR_ERROR_HANDLING_HH
#define LOTHAR_ERROR_HANDLING_HH

#include <stdexcept>
#include "error_handling.h"

namespace lothar
{
  /** \brief Error exception
   *
   * The assumption is that when using the c++ library, the c++ way of doing things is preferred. So
   * exceptions are thrown instead of raising (sometimes global) errors.
   */
  class Error : public std::exception
  {
    int d_errorcode;
  public:
    Error() : d_errorcode(lothar_errno < 0 ? -lothar_errno : lothar_errno)
    {}
    Error(int errorcode) : d_errorcode(errorcode < 0 ? -errorcode : errorcode)
    {}

    /** \brief access to the error code
     */
    int errorcode() const
    {
      return d_errorcode;
    }

    /** \brief Return a description of the error
     */
    char const *what() const throw()
    {
      return lothar_strerror(d_errorcode);
    }
  };

  /** \brief Utility to translate non-zero return values into exceptions
   */
  inline void check_return(int e)
  {
    if(e)
      throw Error(e);
  }
}

#endif // LOTHAR_ERROR_HANDLING_HH
