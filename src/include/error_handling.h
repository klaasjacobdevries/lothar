#ifndef LOTHAR_ERROR_HANDLING_H
#define LOTHAR_ERROR_HANDLING_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef DEBUG


#define LOTHAR_DEBUG(...) { fprintf(stderr, "file %s line %i: ", __FILE__, __LINE__); fprintf(stderr, __VA_ARGS__); }

#define LOTHAR_INFO(...) { fprintf(stderr, "info: "); LOTHAR_DEBUG(__VA_ARGS__); }

#define LOTHAR_WARN(...) { fprintf(stderr, "warning: "); LOTHAR_DEBUG(__VA_ARGS__); }

#define LOTHAR_FAIL(...) { LOTHAR_DEBUG(__VA_ARGS__); exit(1); }

#define LOTHAR_ERR(...) { fprintf(stderr, "error: "); LOTHAR_FAIL(__VA_ARGS__); }

#define LOTHAR_ASSERT(cond, ...) { if(!(cond)) LOTHAR_FAIL(__VA_ARGS__); }
 

#else // DEBUG


#define LOTHAR_DEBUG(...)

#define LOTHAR_INFO(...) { printf("info: "); printf(__VA_ARGS__); }

#define LOTHAR_WARN(...) { fprintf(stderr, "warning: "); fprintf(stderr, __VA_ARGS__); }

#define LOTHAR_ERR(...) { fprintf(stderr, "error: "); fprintf(stderr, __VA_ARGS__); exit(1); }

#define LOTHAR_FAIL(...)

#define LOTHAR_ASSERT(cond, ...)


#endif // DEBUG

#define LOTHAR_IF_DEBUG(cond, ...) { if((cond)) LOTHAR_DEBUG(__VA_ARGS__); }
#define LOTHAR_IF_INFO(cond, ...) { if((cond)) LOTHAR_INFO(__VA_ARGS__); }
#define LOTHAR_IF_WARN(cond, ...) { if((cond)) LOTHAR_WARN(__VA_ARGS__); }
#define LOTHAR_IF_ERR(cond, ...) { if((cond)) LOTHAR_ERR(__VA_ARGS__); }

/** \brief Varius error codes
 */
enum lothar_error
{
  /** everything is okay */
  LOTHAR_ERROR_OKAY = 0, 

  /** error caused by the operating system, check errno for details */
  LOTHAR_ERROR_OS_ERROR,  

  LOTHAR_ERROR_BUFFER_TOO_SMALL,
  
  LOTHAR_ERROR_INVALID_ARGUMENT,
  LOTHAR_ERROR_FILENAME_TOO_LONG,

  LOTHAR_ERROR_USB_NOT_AVAILABLE,
  /** error establishing usb connection */
  LOTHAR_ERROR_USB_CANNOT_CREATE,
  LOTHAR_ERROR_BLUETOOTH_NOT_AVAILABLE,
  LOTHAR_ERROR_BLUETOOTH_CANNOT_CREATE,

  /** read or write on an already closed connection/entity? */
  LOTHAR_ERROR_ENTITY_CLOSED, 
  /** operation timed out */
  LOTHAR_ERROR_TIMEOUT, 

  /** reading from NXT failed */
  LOTHAR_ERROR_NXT_READ_ERROR,  
  /** writing to NXT failed */
  LOTHAR_ERROR_NXT_WRITE_ERROR, 

  /** we don't support this type of sensor (yet?) */
  LOTHAR_ERROR_SENSOR_NOT_AVAILABLE, 

  /** error codes that may be received from the brick */
  LOTHAR_ERROR_PENDING_COMMUNICATION_IN_PROGRESS = 0x20,
  LOTHAR_ERROR_MAILBOX_QUEUE_EMPTY               = 0x40,
  LOTHAR_ERROR_REQUEST_FAILED                    = 0xBD,
  LOTHAR_ERROR_UNKOWN_COMMAND_OPCODE             = 0xBE,
  LOTHAR_ERROR_INSANE_PACKET                     = 0xBF,
  LOTHAR_ERROR_OUT_OF_RANGE_VALUES               = 0xC0,
  LOTHAR_ERROR_BUS_ERROR                         = 0xDD,
  LOTHAR_ERROR_BUFFER_OUT_OF_MEMORY              = 0xDE,
  LOTHAR_ERROR_CONNECTION_NOT_VALID              = 0xDF,
  LOTHAR_ERROR_CONNECTION_NOT_CONFIGURED         = 0xE0,
  LOTHAR_ERROR_NO_ACTIVE_PROGRAM                 = 0xEC,
  LOTHAR_ERROR_ILLEGAL_SIZE                      = 0xED,
  LOTHAR_ERROR_ILLEGAL_MAILBOX_QUEUE             = 0xEE,
  LOTHAR_ERROR_INVALID_FIELD                     = 0xEF,
  LOTHAR_ERROR_BAD_INPUT_OUTPUT                  = 0xF0,
  LOTHAR_ERROR_INSUFFICIENT_MEMORY               = 0xFB,
  LOTHAR_ERROR_BAD_ARGUMENTS                     = 0xFF,

  LOTHAR_ERROR_UNKOWN_ERROR // what happened?
};

/** \brief the global error variable 
 *
 * It is preferred to use a local variable to hold the error.
 */
extern int lothar_errno;

/** \brief clear the global error
 */
static inline void lothar_clear_error(void)
{
  lothar_errno = 0;
}

/** \brief returns the string representation of the error 
 */
char const *lothar_strerror(int error);

/** \brief global version of lothar_strerror
 */
static inline char const *lothar_strerror_global(void)
{
  return lothar_strerror(lothar_errno);
}

static inline void lothar_perror(char const *msg, int error)
{
  fprintf(stderr, "%s: %s\n", msg, lothar_strerror(error));
}

static inline void lothar_perror_global(char const *msg)
{
  lothar_perror(msg, lothar_errno);
}

#define LOTHAR_ERROR(error) \
{ \
  if(error) \
    LOTHAR_WARN("file %s line %d\n\terror raised: (%d) %s\n", __FILE__, __LINE__, error, lothar_strerror(error)); \
  lothar_errno = (error < 0 ? -error : error); \
}

#define LOTHAR_RETURN_ERROR(error) { LOTHAR_ERROR(error); return -error; }

#ifdef __cplusplus
}
#endif

#endif

