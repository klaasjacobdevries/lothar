#include "utils.h"

/* Backend for windows bluetooth stack */

#ifdef HAVE_WIN32_BLUETOOTH

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2bth.h>
#include <ws2tcpip.h>

#define QUERYFLAGS (LUP_RETURN_NAME | LUP_CONTAINERS | LUP_RETURN_ADDR | LUP_FLUSHCACHE | LUP_RETURN_TYPE | LUP_RETURN_BLOB | LUP_RES_SERVICE)

#define LOTHAR_WSA_ERROR { LOTHAR_DEBUG("last wsa error %d\n", WSAGetLastError()); LOTHAR_ERROR(LOTHAR_ERROR_OS_ERROR); }

void *lothar_bt_new(char const *address)
{
  static int firstcall = 1;
  static WSADATA wsadata;

  SOCKADDR_BTH addr = {0};
  int status;
  SOCKET * sock;

  if(firstcall)
  {
    WORD version = MAKEWORD(2, 2);
    if(WSAStartup(version, &wsadata))
	  {
       LOTHAR_WSA_ERROR;
       return NULL;
    }
    firstcall = 0;
  }
       
  sock = (SOCKET *)lothar_malloc(sizeof(SOCKET));
  LOTHAR_DEBUG("connection to %s\n", address);
  *sock = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
  if(*sock < 0)
  {
    LOTHAR_WSA_ERROR;
    free(sock);
    return NULL;
  }
  // todo: lookup
  status = connect(*sock, (struct sockaddr *)&addr, sizeof(addr) );
  if (status < 0)
  {
    LOTHAR_WSA_ERROR;
    shutdown(*sock, SD_BOTH);
    free(sock);
    return NULL;
  }

  return sock;
}

int lothar_bt_backend_write(void *connection, uint8_t const *data, size_t len)
{
  SOCKET *sock = (int *)connection;
  int status = send(*sock, data, len, 0);

  if(status <= 0)
    LOTHAR_WSA_ERROR;

  return status;
}

int lothar_bt_backend_read(void *connection, uint8_t *data, size_t len)
{
  SOCKET *sock = (int *)connection;
  int status = recv(*sock, data, len, 0);
  
  if(status <= 0)
    LOTHAR_WSA_ERROR;
  
  return status;
}

int lothar_bt_close(void *connection)
{
  SOCKET *sock = (int *)connection;
  int status = shutdown(*sock, SD_BOTH);
  free(sock);

  if(status < 0)
    LOTHAR_WSA_ERROR;

  return status;
}

#endif // HAVE_WIN32_BLUETOOTH
