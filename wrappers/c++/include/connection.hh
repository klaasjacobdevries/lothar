#ifndef LOTHAR_CONNECTION_HH
#define LOTHAR_CONNECTION_HH

#include "connection.h"
#include "utils.hh"

namespace lothar
{
  class Connection;
  typedef lothar::shared_ptr<Connection> ConnectionPtr;
  
  /** \brief Base class for connections.
   *
   * You shouldn't use this directly, use USBConnection, BluetoothConnection or CustomConnection instead
   */
  class Connection : public no_copy
  {
    lothar_connection_t *d_connection;

  protected:
    Connection() : d_connection(NULL)
    {}

    /** \brief Return the underlying lothar_connection_t
     */
    lothar_connection_t *get_connection()
    {
      return d_connection;
    }

    /** \brief Return the underlying lothar_connection_t
     */
    lothar_connection_t const *get_connection() const
    {
      return d_connection;
    }
    
    void set_connection(lothar_connection_t *connection)
    {
      d_connection = connection;
    }

  public:
    virtual ~Connection()
    {
      lothar_connection_close(&d_connection);
    }

    operator lothar_connection_t const *() const
    {
      return get_connection();
    }

    operator lothar_connection_t *()
    {
      return get_connection();
    }

    /** \brief Try to automatically open a connection.
     *
     * \returns A pointer to a bluetooth connection if possible, if not a pointer to a usb connection if possible. NULL if everything fails.
     */
    static ConnectionPtr create_connection();
  };

  /** \brief USB connection
   */
  class USBConnection : public Connection
  {
  protected:
    USBConnection();
    USBConnection(uint16_t vendor, uint16_t product);

  public:
    /** \brief Open a connection with the default vendor and product id 
     *
     * \throws Error if the connection failed.
     */
    static ConnectionPtr create()
    {
      return ConnectionPtr(new USBConnection);
    }
    /** \brief Open a connection with the given vendor and product id
     *
     * \throws Error if the connection failed.
     */
    static ConnectionPtr create(uint16_t vendor, uint16_t product)
    {
      return ConnectionPtr(new USBConnection(vendor, product));
    }
  };

  /** \brief Bluetooth connection
   */
  class BluetoothConnection : public Connection
  {
  protected:
    BluetoothConnection();
    BluetoothConnection(char const *address);
  public:
    /** \brief Open a connection with the default address
     *
     * \throws Error if the connection failed.
     */
    static ConnectionPtr create()
    {
      return ConnectionPtr(new BluetoothConnection);
    }

    /** \brief Open a connection with a custom address
     *
     * \throws Error if the connection failed.
     */
    static ConnectionPtr create(char const *address)
    {
      return ConnectionPtr(new BluetoothConnection(address));
    }
  };

  /** \brief Interface class to create your own custom connections
   */
  class CustomConnection : public Connection
  {
    static lothar_connection_vtable s_vtable;

  public:
    /** \brief constructor
     *
     * Note that this will not open the connection. Derived classes are responsible for opening the connection
     * themselves, prior to calling read or write.
     *
     * \throws Error if creating the connection failed.
     */
    CustomConnection();
    
    /** \brief Destructor
     *
     * \note Important to realize when deriving from this class. close() won't be called by the destructor, as it is a dangerous thing to call virtual functions from destructors. If you need cleanup code, don't rely on close being called automatically, do it yourself!
     */
    ~CustomConnection();
    
    /** \brief Overwrite this for reading data
     *
     * \returns the number of bytes read.
     */
    virtual int read(uint8_t *data, size_t len) = 0;

    /** \brief Overwrite this for writing data
     *
     * \returns the number of bytes written
     */
    virtual int write(uint8_t const *data, size_t len) = 0;

    /** \brief Overwrite this to close the connection
     */
    virtual int close() = 0;
  };
};

#endif // LOTHAR_CONNECTION_HH
