#include "error_handling.h"
#include <string.h>

int lothar_errno = LOTHAR_ERROR_OKAY;

char const *lothar_strerror(int error)
{
  if(error < 0)
    error = -error;

  switch(error)
  {
  case LOTHAR_ERROR_OKAY:
    return "everything's fine";
      
  case LOTHAR_ERROR_OS_ERROR:
    return strerror(errno);

  case LOTHAR_ERROR_BUFFER_TOO_SMALL:
    return "buffer too small";

  case LOTHAR_ERROR_INVALID_ARGUMENT:
    return "invalid argument";

  case LOTHAR_ERROR_FILENAME_TOO_LONG:
    return "filename too long";

  case LOTHAR_ERROR_ENTITY_CLOSED:
    return "operation attempted on a closed entity";

  case LOTHAR_ERROR_TIMEOUT:
    return "operation timed out";

  case LOTHAR_ERROR_USB_NOT_AVAILABLE:
    return "compiled without usb support";

  case LOTHAR_ERROR_USB_CANNOT_CREATE:
    return "cannot establish usb connection";

  case LOTHAR_ERROR_BLUETOOTH_NOT_AVAILABLE:
    return "compiled without bluetooth support";

  case LOTHAR_ERROR_BLUETOOTH_CANNOT_CREATE:
    return "cannot establish bluetooth connection";

  case LOTHAR_ERROR_NXT_READ_ERROR:
    return "could not read from NXT";
    
  case LOTHAR_ERROR_NXT_WRITE_ERROR:
    return "could not write to NXT";

  case LOTHAR_ERROR_SENSOR_NOT_AVAILABLE:
    return "sensor type not supported (yet?)";
    
  case LOTHAR_ERROR_PENDING_COMMUNICATION_IN_PROGRESS:
    return "pending communication transaction in progress";

  case LOTHAR_ERROR_MAILBOX_QUEUE_EMPTY:
    return "specified mailbox queue is empty";

  case LOTHAR_ERROR_REQUEST_FAILED:  
    return "request failed (i.e. specified file not found)";
    
  case LOTHAR_ERROR_UNKOWN_COMMAND_OPCODE:
    return "unknown command opcode";

  case LOTHAR_ERROR_INSANE_PACKET:
    return "insane packet";

  case LOTHAR_ERROR_OUT_OF_RANGE_VALUES:
    return "data contains out-of-range values";

  case LOTHAR_ERROR_BUS_ERROR:
    return "communication bus error";

  case LOTHAR_ERROR_BUFFER_OUT_OF_MEMORY:
    return "no free memory in communication buffer";

  case LOTHAR_ERROR_CONNECTION_NOT_VALID:
    return "specified channel/connection is not valid";

  case LOTHAR_ERROR_CONNECTION_NOT_CONFIGURED:
    return "specified channel/connection not configured or busy";

  case LOTHAR_ERROR_NO_ACTIVE_PROGRAM:
    return "no active program";

  case LOTHAR_ERROR_ILLEGAL_SIZE:
    return "illegal size specified";

  case LOTHAR_ERROR_ILLEGAL_MAILBOX_QUEUE:
    return "illegal mailbox queue ID specified";

  case LOTHAR_ERROR_INVALID_FIELD:
    return "attempted to access invalid field of a structure";

  case LOTHAR_ERROR_BAD_INPUT_OUTPUT:
    return "bad input or output specified";
    
  case LOTHAR_ERROR_INSUFFICIENT_MEMORY:
    return "insufficient memory available";

  case LOTHAR_ERROR_BAD_ARGUMENTS:
    return "bad arguments";

  case LOTHAR_ERROR_UNKOWN_ERROR:
  default:
    break;
  }
  
  return "unkown error";
}

