#include "utils.h"

/* the libusb-1.0 usb backend */

#if defined(HAVE_LIBUSB) || defined (HAVE_LIBUSBX)

#ifdef HAVE_LIBUSBX
#include <libusbx-1.0/libusb.h>
#else
#include <libusb-1.0/libusb.h>
#endif

// return the usb connection by vendor and product
void *lothar_usb_new(uint16_t vendor, uint16_t product)
{
  static int firstcall = 0;
  static int interface = 0; // number of connected devices
  libusb_device_handle *handle;
  
  if(!firstcall)
  {
    libusb_init(NULL);
    firstcall = 1;
  }

  handle = libusb_open_device_with_vid_pid(NULL, vendor, product);

  if(handle)
    libusb_claim_interface(handle, interface++);
  else
    LOTHAR_ERROR(LOTHAR_ERROR_USB_CANNOT_CREATE);

  return handle;
}

int lothar_usb_write(void *connection, uint8_t const *data, size_t len)
{
  int w;
  if(libusb_bulk_transfer((libusb_device_handle *)connection, LIBUSB_ENDPOINT_OUT | 1, (uint8_t *)data, len, &w, 0))
  {
    LOTHAR_ERROR(LOTHAR_ERROR_NXT_WRITE_ERROR);
    return -1;
  }

  return w;
}

int lothar_usb_read(void *connection, uint8_t *data, size_t len)
{
  int r;

  if(libusb_bulk_transfer((libusb_device_handle *)connection, LIBUSB_ENDPOINT_IN | 2, data, len, &r, 0))
  {
    LOTHAR_ERROR(LOTHAR_ERROR_NXT_READ_ERROR);
    return -1;
  }
  
  return r;
}

int lothar_usb_close(void *connection)
{
  libusb_close((libusb_device_handle *)connection);
  return 0;
}

#elif defined(LIBUSB_0_1)

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

#endif // defined(HAVE_LIBUSB) || defined (HAVE_LIBUSBX)
