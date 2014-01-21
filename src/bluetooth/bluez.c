#include "utils.h"

/* Backend for the Linux bluez bluetooth stack */

#ifdef HAVE_BLUEZ

#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

// returns true if address looks like an address (i.e. 00:16:53:12:5C:67) instead of a friendly name
static int is_address(char const *address)
{
  size_t i;
  size_t s = strlen(address);

  if(s != 17)
    return 0;

  for(i = 0; i < s - 2; i += 3)
  {
    if(!isxdigit(address[i]) ||
       !isxdigit(address[i + 1]) ||
       address[i + 2] != ':')
      return 0;
  }

  return 1;
}

static int find_address(char const *name, char address[18])
{
  int sock;
  int devid;
  int count;
  int i;
  inquiry_info *info = NULL;
  char rname[255]; 

  devid = hci_get_route(NULL);
  if(devid < 0)
    return -1;

  sock = hci_open_dev(devid);
  if(sock < 0)
    return -1;

  info = (inquiry_info *)malloc(255 * sizeof(inquiry_info ));
  count = hci_inquiry(devid, 8, sizeof(info) / sizeof(inquiry_info), NULL, &info, IREQ_CACHE_FLUSH);
  if(count < 0)
  {
    LOTHAR_DEBUG("Why does this keep failing? %d %d %s\n", count, errno, strerror(errno));
    free(info);
    close(sock);
    return -1;
  }

  for(i = 0; i < count; ++i)
  {
    memset(rname, 0, sizeof(rname));
    if(hci_read_remote_name(sock, &info[i].bdaddr, sizeof(rname), rname, 0) == 0)
    {
      if(strcmp(name, rname) == 0)
      {
        ba2str(&info[i].bdaddr, address);
        free(info);
        close(sock);
        return 0;
      }
    }
  }
  free(info);
  close(sock);
  return -1; 
}

void *lothar_bt_new(char const *a_address)
{
  struct sockaddr_rc addr = {0};
  int status;

  int *sock = (int *)lothar_malloc(sizeof(int));
  char address[18];

  LOTHAR_DEBUG("connecting to %s\n", a_address);

  if(!is_address(a_address))
  {
    LOTHAR_DEBUG("address is probably a friendly name, looking it up\n");
    if(find_address(a_address, address) < 0)
    {
      LOTHAR_ERROR(LOTHAR_ERROR_OS_ERROR);
      free(sock);
      return NULL;
    }
    LOTHAR_DEBUG("found address %s\n", address);
  }
  else
    strcpy(address, a_address);

  *sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

  if(*sock < 0)
  {
    LOTHAR_ERROR(LOTHAR_ERROR_OS_ERROR);
    free(sock);
    return NULL;
  }
 
  int yes = 1;
  setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  addr.rc_family = AF_BLUETOOTH;
  addr.rc_channel = (uint8_t) 1;
  str2ba(address, &addr.rc_bdaddr);

  status = connect(*sock, (struct sockaddr *)&addr, sizeof(addr) );
  if (status < 0)
  {
    LOTHAR_ERROR(LOTHAR_ERROR_OS_ERROR);
    close(*sock);
    free(sock);
    return NULL;
  }

  return sock;
}

int lothar_bt_backend_write(void *connection, uint8_t const *data, size_t len)
{
  int *sock = (int *)connection;
  int status = send(*sock, data, len, 0);

  if(status <= 0)
    LOTHAR_ERROR(LOTHAR_ERROR_OS_ERROR);

  return status;
}

int lothar_bt_backend_read(void *connection, uint8_t *data, size_t len)
{
  int *sock = (int *)connection;
  int status = recv(*sock, data, len, 0);
  
  if(status <= 0)
    LOTHAR_ERROR(LOTHAR_ERROR_OS_ERROR);
  
  return status;
}

int lothar_bt_close(void *connection)
{
  int *sock = (int *)connection;
  int status = close(*sock);
  free(sock);

  if(status < 0)
    LOTHAR_ERROR(LOTHAR_ERROR_OS_ERROR);

  return status;
}

#endif
