#include "connection.hh"

using namespace std;
using namespace lothar;

ConnectionPtr Connection::create_connection()
{
  ConnectionPtr result;
  try
  {
    result = BluetoothConnection::create();
  }
  catch(Error const &)
  {
    lothar_clear_error();
    try
    {
      result = USBConnection::create();
    }
    catch(Error const &)
    {
      lothar_clear_error();
      result = ConnectionPtr();
    }
  }
  return result;
}

USBConnection::USBConnection()
{
  lothar_connection_t *connection = lothar_connection_open_usb();
  if(!connection)
    throw Error(lothar_errno);
  else
    set_connection(connection);
}

USBConnection::USBConnection(uint16_t vendor, uint16_t product)
{
  lothar_connection_t *connection = lothar_connection_open_usb_vid_pid(vendor, product);
  if(!connection)
    throw Error(lothar_errno);
  else
    set_connection(connection);
}

BluetoothConnection::BluetoothConnection()
{
  lothar_connection_t *connection = lothar_connection_open_bluetooth();
  if(!connection)
    throw Error(lothar_errno);
  else
    set_connection(connection);
}

BluetoothConnection::BluetoothConnection(char const *address)
{
  lothar_connection_t *connection = lothar_connection_open_bluetooth_address(address);
  if (!connection)
    throw Error(lothar_errno);
  else
    set_connection(connection);
}

namespace 
{
  int read_cb(void *_connection, uint8_t *data, size_t len)
  {
    CustomConnection *connection = static_cast<CustomConnection *>(_connection);
    return connection->read(data, len);
  }

  int write_cb(void *_connection, uint8_t const *data, size_t len)
  {
    CustomConnection *connection = static_cast<CustomConnection *>(_connection);
    return connection->write(data, len);
  }

  int close_cb(void *_connection)
  {
    CustomConnection *connection = static_cast<CustomConnection *>(_connection);
    return connection->close();
  }
}

lothar_connection_vtable CustomConnection::s_vtable = 
{
  read_cb,
  write_cb,
  close_cb
};

CustomConnection::CustomConnection()
{
  lothar_connection_t *connection = lothar_connection_open_custom(&s_vtable, this);
  if(!connection)
    throw Error(lothar_errno);
  else
    set_connection(connection);
}

CustomConnection::~CustomConnection()
{
  // dangerous things may happen if the derived close() is called, prevent it.

  // this is of course hacky, we depend on assumptions about lothar_connection_close, and a derived
  // class may forget to clean up itself.

  free(get_connection());
  set_connection(NULL);
}
