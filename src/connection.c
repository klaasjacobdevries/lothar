#include "connection.h"
#include "error_handling.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// defined in bluetooth backend

extern void *lothar_bt_new(char const *address);
extern int lothar_bt_backend_write(void *connection, uint8_t const *data, size_t len);
extern int lothar_bt_backend_read(void *connection, uint8_t *data, size_t len);
extern int lothar_bt_close(void *connection);

// defined in usb backend

extern void *lothar_usb_new(uint16_t vendor, uint16_t product);
extern int lothar_usb_write(void *connection, uint8_t const *data, size_t len);
extern int lothar_usb_read(void *connection, uint8_t *data, size_t len);
extern int lothar_usb_close(void *connection);

// useful for any kind of bluetooth backend

static int lothar_bt_write(void *connection, uint8_t const *data, size_t len)
{
  // copying instead of sending two messages is probably better, since buffering and bandwith is probalby more a limitation then cpu and memory
  uint8_t *buf = (uint8_t *)lothar_malloc(len + 2);
  size_t w = 0;

  // set the size, the nxt expects little endian
  buf[0] = len;
  buf[1] = len >> 8;

  memcpy(buf + 2, data, len);

  while(w < len + 2)
  {
    int _w = lothar_bt_backend_write(connection, buf + w, len + 2 - w);
    
    if(_w <= 0)
    {
      free(buf);
      return -1;
    }
    
    w += _w;
  }
  
  free(buf);
  
  return len;
}

static int lothar_bt_read(void *connection, uint8_t *data, size_t len)
{
  uint8_t buf[2];
  uint16_t size;
  int status;
  int tr;

  if((status = lothar_bt_backend_read(connection, buf, 2)) && status != 2)
  {
    if(status > 0) // _some_ reading was done
      LOTHAR_ERROR(LOTHAR_ERROR_NXT_READ_ERROR);
    return -1;
  }
  
  // nxt gives us little endian
  size = lothar_nxttohs(buf);

  if(size > len)
  {
    tr = lothar_bt_backend_read(connection, data, size);

    // lets try to be robust and read what we can, i.e. don't leave it in the buffer
    size -= tr;
    while(size)
    {
      tr = lothar_bt_backend_read(connection, buf, 2);
      if(tr <= 0)
	break;

      size -= tr;
    }

    status = len; // well, in a way, it _did_ succeed
  }
  else if(len > size)
  {
    status = lothar_bt_backend_read(connection, data, size);
  }
  else
    status = lothar_bt_backend_read(connection, data, len);

  return status;
}

struct lothar_connection_t
{
  lothar_connection_vtable const *vtable;
  void *connection;
};

static lothar_connection_vtable usb_vtable = 
{
  lothar_usb_read,
  lothar_usb_write,
  lothar_usb_close
};

static lothar_connection_vtable bt_vtable = 
{
  lothar_bt_read,
  lothar_bt_write,
  lothar_bt_close
};

lothar_connection_t *lothar_connection_open_usb_vid_pid(uint16_t vendor, uint16_t product)
{
  lothar_connection_t *connection = (lothar_connection_t *)lothar_malloc(sizeof(lothar_connection_t));

  connection->connection = lothar_usb_new(vendor, product);

  if(!connection->connection)
  {
    free(connection);
    return NULL;
  }

  connection->vtable = &usb_vtable;

  return connection;
}

lothar_connection_t *lothar_connection_open_usb()
{
  return lothar_connection_open_usb_vid_pid(USB_VENDOR_LEGO, USB_PRODUCT_NXT);
}

lothar_connection_t *lothar_connection_open_bluetooth_address(char const *address)
{
  lothar_connection_t *connection = (lothar_connection_t *)lothar_malloc(sizeof(lothar_connection_t));

  assert(address);

  connection->connection = lothar_bt_new(address);
  if(!connection->connection)
  {
    free(connection);
    return NULL;
  }

  connection->vtable = &bt_vtable;

  return connection;
}

lothar_connection_t *lothar_connection_open_bluetooth()
{
  return lothar_connection_open_bluetooth_address(BLUETOOTH_NXT_ADDRESS);
}

lothar_connection_t *lothar_connection_open()
{
  lothar_connection_t *connection;
  
  LOTHAR_INFO("attempting bluetooth connection...\n");

  connection = lothar_connection_open_bluetooth();
  if(!connection)
  {
    LOTHAR_INFO("failed, attempting usb connection...\n");

    LOTHAR_ERROR(LOTHAR_ERROR_OKAY); // clear errno
    connection = lothar_connection_open_usb();

    if(!connection)
    {
      LOTHAR_INFO("failed!\n");
      return NULL;
    }
  }
  LOTHAR_INFO("success!\n");

  return connection;
}

lothar_connection_t *lothar_connection_open_custom(lothar_connection_vtable const *vtable,
                                                   void *private_data)
{
  lothar_connection_t *connection = (lothar_connection_t *)lothar_malloc(sizeof(lothar_connection_t));

  assert(vtable);

  connection->vtable = vtable;
  connection->connection = private_data;
  
  return connection;
}

int lothar_connection_close(lothar_connection_t **connection)
{
  int ret;

  if(!connection || !(*connection))
    return 0;

  ret = (*connection)->vtable->close((*connection)->connection);

  free(*connection);
  *connection = NULL;

  return ret;
}

int lothar_connection_write(lothar_connection_t *c, uint8_t const *data, size_t len)
{
  if(!c)
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_ENTITY_CLOSED);

  return c->vtable->write(c->connection, data, len);
}

int lothar_connection_read(lothar_connection_t *c, uint8_t *data, size_t len)
{
  if(!c)
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_ENTITY_CLOSED);

  return c->vtable->read(c->connection, data, len);
}
