#include "utils.h"

#ifdef DUMMY_BLUETOOTH

// no bluetooth, using dummies

void *lothar_bt_new(char const *address)
{
  LOTHAR_ERROR(LOTHAR_ERROR_BLUETOOTH_NOT_AVAILABLE);
  return NULL;
}

int lothar_bt_backend_write(void *connection, uint8_t const *data, size_t len)
{
  LOTHAR_ERROR(LOTHAR_ERROR_BLUETOOTH_NOT_AVAILABLE);
  return -1;
}

int lothar_bt_backend_read(void *connection, uint8_t *data, size_t len)
{
  LOTHAR_ERROR(LOTHAR_ERROR_BLUETOOTH_NOT_AVAILABLE);
  return -1;
}

int lothar_bt_close(void *connection)
{
  LOTHAR_ERROR(LOTHAR_ERROR_BLUETOOTH_NOT_AVAILABLE);
  return -1;
}

#endif
