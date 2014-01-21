#include "utils.h"

// backend for the libusb, the older 0.1 version

#ifdef HAVE_LIBUSB_0_1

#include <usb.h>

void *lothar_usb_new(uint16_t vendor, uint16_t product)
{
  struct usb_bus *bus;
  usb_dev_handle *result = NULL;
  static int init = 0;
  static int interface = 0;

  if(!init)
  {
    usb_init();
    usb_find_busses();
    usb_find_devices();
    init = 1;
  }

  for(bus = usb_get_busses(); bus; bus = bus->next)
  {
    struct usb_device *dev = NULL;
    for(dev = bus->devices; dev; dev = dev->next)
    {
      if(dev->descriptor.idVendor == vendor && dev->descriptor.idProduct == product)
      {
	result = usb_open(dev);
	break;
      }
    }

    if(result) // break out of nested loop
      break;
  }

  if(!result || usb_claim_interface(result, interface++)) // nothing found, or error claiming
  {
    LOTHAR_ERROR(LOTHAR_ERROR_USB_CANNOT_CREATE);
    return NULL;
  }

  return result;  
}

int lothar_usb_write(void *connection, uint8_t const  *data, size_t len)
{
  int status = usb_bulk_write((usb_dev_handle *)connection, USB_ENDPOINT_OUT | 1, (char *)data, len, 0);
  if(status < 0)
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_NXT_WRITE_ERROR);
  return status;
}

int lothar_usb_read(void *connection, uint8_t *data, size_t len)
{
  int status = usb_bulk_read((usb_dev_handle *)connection, USB_ENDPOINT_IN | 2, (char *)data, len, 0);
  if(status < 0)
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_NXT_READ_ERROR);
  return status;
}

int lothar_usb_close(void *connection)
{
  if(usb_close((usb_dev_handle *)connection))
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_UNKOWN_ERROR);
  return 0;
}

#endif
