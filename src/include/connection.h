#ifndef LOTHAR_CONNECTION_H
#define LOTHAR_CONNECTION_H

#include <stdint.h>
#include "utils.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** \brief opaque data structure describing a connection (either bluetooth or usb) 
 */
struct lothar_connection_t;
typedef struct lothar_connection_t lothar_connection_t;

/// vendor id of Lego
#define USB_VENDOR_LEGO       0x0694  
/// product id of the NXT
#define USB_PRODUCT_NXT       0x0002  
/// default bluethooth address
#define BLUETOOTH_NXT_ADDRESS "NXT"   

/** \brief open a new connection over usb, given a vendor and product id 
 */
lothar_connection_t *lothar_connection_open_usb_vid_pid(uint16_t vendor, uint16_t product);

/** \brief open a new connection over usb using the standard vendor and product id of the NXT 
 */
lothar_connection_t *lothar_connection_open_usb(void);

/** \brief open a new bluetooth connection, by address, i.e. "00:16:53:12:5c:67", or "NXT" 
 */
lothar_connection_t *lothar_connection_open_bluetooth_address(char const *address);

/** \brief open a new bluetooth connection by the default bluetooth address 
 */
lothar_connection_t *lothar_connection_open_bluetooth(void);

/** \brief For your convienience, open a new connection by trying whatever works. 
 *
 * This first tries the default bluetooth address, then the default usb vendor/product, and if all fails returns NULL
 */
lothar_connection_t *lothar_connection_open(void);


/** \brief A vtable for connections, which provides the ability to create custom connections.
 */
typedef struct
{
  int (*read)(void *private_data, uint8_t *buf, size_t count);
  int (*write)(void *private_data, uint8_t const *buf, size_t count);
  int (*close)(void *private_data);
} lothar_connection_vtable;

/** \brief Custom opener for connections
 *
 * Open a connection by providing custom vtable.
 * \param vtable       a structure with the proper callback functions. The caller should ensure this 
 *                     will remain alive for the duration of the connection.
 * \param private_data a pointer to be passed as the first argument to the callbacks
 */
lothar_connection_t *lothar_connection_open_custom(lothar_connection_vtable const *vtable,
                                                   void *private_data);

/** \brief Closes a connection and frees the data structure 
 */
int lothar_connection_close(lothar_connection_t **connection);

/** \brief Writes data over a connection 
 *
 * \returns 0 on succes, and tries to send all bytes
 */
int lothar_connection_write(lothar_connection_t *connection, uint8_t const *data, size_t len);

/** \brief Reads data from a connection
 *
 * \returns 0 on success, tries to read all bytes
 */
int lothar_connection_read(lothar_connection_t *connection, uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif
