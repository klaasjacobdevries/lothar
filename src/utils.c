#include "utils.h"

#ifdef _WIN32 // windows

#include <windows.h>

int lothar_msleep(lothar_time_t ms)
{
  Sleep((DWORD)ms);
  return 0;
}

// hackish gettimeofday implementation
static int gettimeofday(struct timeval *tp, void *tzp)
{
  FILETIME ft;
  unsigned __int64 ns;
  unsigned __int64 us;
  GetSystemTimeAsFileTime(&ft);
  ns = ft.dwLowDateTime | (((unsigned __int64)ft.dwHighDateTime) << 32); // now in 100 * ns
  us = ns / 10; // microseconds
  tp->tv_sec = (long)(us / 1000000);
  tp->tv_usec = us % 1000000;
  return 0;
}

#else // all posix compatible (Linux, BSD, OSX, other unixes)

#include <unistd.h>
#include <sys/time.h>

int lothar_msleep(lothar_time_t ms)
{
  int status;

  if((status = usleep(ms * 1000)) < 0)
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_OS_ERROR);
  
  return status;
}

#endif

// the difference between now and then, in milliseconds
static lothar_time_t timediff(struct timeval *then)
{
  struct timeval now;
  gettimeofday(&now, NULL);
  return (now.tv_sec - then->tv_sec) * 1000 + (now.tv_usec - then->tv_usec) / 1000;
}

lothar_time_t lothar_time()
{
  static struct timeval firstcall = {0, 0};

  if(firstcall.tv_sec == 0 && firstcall.tv_usec == 0)
  {
    gettimeofday(&firstcall, NULL);
    return 0;
  }

  return timediff(&firstcall);
}

void *lothar_malloc(size_t size)
{
  void *r = malloc(size);
  
  if(!r)
  {
    perror("malloc() failed");
    exit(1);
  }

  return r;
}

void *lothar_realloc(void *p, size_t size)
{
  p = realloc(p, size);

  if(!p)
  {
    perror("realloc() failed");
    exit(1);
  }

  return p;
}
