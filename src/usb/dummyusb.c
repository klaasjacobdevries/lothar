#include "utils.h"

// dummy usb connection, only to return proper error messages

#ifdef DUMMY_USB

// no usb library specified, using dummies

void *lothar_usb_new(uint16_t p, uint16_t v)
{
  LOTHAR_ERROR(LOTHAR_ERROR_USB_NOT_AVAILABLE);
  return NULL;
}

int lothar_usb_write(void *c, uint8_t const *d, size_t l)
{
  LOTHAR_ERROR(LOTHAR_ERROR_USB_NOT_AVAILABLE);
  return -1;
}

int lothar_usb_read(void *c, uint8_t *d, size_t l)
{
  LOTHAR_ERROR(LOTHAR_ERROR_USB_NOT_AVAILABLE);
  return -1;
}

int lothar_usb_close(void *c)
{
  LOTHAR_ERROR(LOTHAR_ERROR_USB_NOT_AVAILABLE);
  return -1;
}

#endif
