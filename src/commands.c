#include "commands.h"
#include "connection.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>

/* command definitions */

#define STARTPROGRAM          0x00
#define STOPPROGRAM           0x01
#define PLAYSOUNDFILE         0x02
#define PLAYTONE              0x03
#define SETOUTPUTSTATE        0x04
#define SETINPUTMODE          0x05
#define GETOUTPUTSTATE        0x06
#define GETINPUTVALUES        0x07
#define RESETINPUTSCALEDVALUE 0x08
#define MESSAGEWRITE          0x09
#define RESETMOTORPOSITION    0x0A
#define GETBATTERYLEVEL       0x0B
#define STOPSOUNDPLAYBACK     0x0C
#define KEEPALIVE             0x0D
#define LSGETSTATUS           0x0E
#define LSWRITE               0x0F
#define LSREAD                0x10
#define GETCURRENTPROGRAMNAME 0x11
#define MESSAGEREAD           0x13

/* utilities */

#define RESPONSE    0x00
#define NO_RESPONSE 0x80

// from here, everything returns 0 on success, lothar_errno on failure

static int send(lothar_connection_t *connection, uint8_t response_required, uint8_t command, uint8_t const *arguments, size_t len)
{
  uint8_t *buf = (uint8_t *)lothar_malloc(len + 2);
  int status;
  
  buf[0] = response_required;
  buf[1] = command;

  if(arguments && len)
    memcpy(buf + 2, arguments, len);

  status = lothar_connection_write(connection, buf, len + 2);
  free(buf);

  return status == len + 2 ? 0 : -lothar_errno;
}

static int recv(lothar_connection_t *connection, uint8_t command, uint8_t *buf, size_t len)
{
  int r = lothar_connection_read(connection, buf, len);
  if(r != len)
    return -lothar_errno;

  if(len < 3 || buf[0] != 0x02 || buf[1] != command)
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_NXT_READ_ERROR);
  
  /* explicitly ignore a pending communication error on lsgetstatus */
  if(command == LSGETSTATUS && buf[2] == LOTHAR_ERROR_PENDING_COMMUNICATION_IN_PROGRESS)
  {
    buf[3] = 0;
    return 0;
  }

  if (buf[2])
  {
    int e = buf[2]; // this is only to surpress a compiler warning (comparison always true due to range of data type)
    LOTHAR_RETURN_ERROR(e);
  }
  return 0;
}

/* commands: */

/* startprogram */

int lothar_startprogram(lothar_connection_t *connection, char const *filename)
{
  size_t fs;

  if(!filename)
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_INVALID_ARGUMENT);
 
  fs = strlen(filename);

  if(fs > 18) // this is the maximum file name size
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_FILENAME_TOO_LONG);
  
  return send(connection, NO_RESPONSE, STARTPROGRAM, (uint8_t const *)filename, fs + 1);
}

/* stopprogram */

int lothar_stopprogram(lothar_connection_t *connection)
{
  return -send(connection, NO_RESPONSE, STOPPROGRAM, NULL, 0);
}

/* playsoundfile */

int lothar_playsoundfile(lothar_connection_t *connection, uint8_t loop, char const *filename)
{
  int status;
  size_t fs;
  uint8_t *buf;

  if(!filename)
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_INVALID_ARGUMENT);

  fs = strlen(filename) + 1; // include terminating \0
  
  if(fs > 19) // this is the maximum file name size
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_FILENAME_TOO_LONG);

  buf = (uint8_t *)lothar_malloc(fs + 2);
  buf[0] = loop ? 0x01 : 0x00;
  memcpy(buf + 1, filename, fs + 1);

  status = send(connection, NO_RESPONSE, PLAYSOUNDFILE, buf, fs + 1);
  
  free(buf);

  return status;
}

/* playtone */

int lothar_playtone(lothar_connection_t *connection, uint16_t frequency, uint16_t duration)
{
  uint8_t buf[4];

  lothar_htonxts(CLAMP(frequency, 200, 14000), buf);
  lothar_htonxts(duration, (buf + 2));

  return send(connection, NO_RESPONSE, PLAYTONE, buf, 4);
}

/* setoutputstate */

int lothar_setoutputstate(lothar_connection_t *connection, 
			  enum lothar_output_port port, 
			  int8_t power, 
			  enum lothar_output_motor_mode mode, 
			  enum lothar_output_regulation_mode rmode, 
			  int8_t turn_ratio, 
			  enum lothar_output_runstate rstate, 
			  uint32_t tacholimit)
{
  uint8_t buf[10];

  buf[0] = port;
  buf[1] = CLAMP(power, -100, 100);
  buf[2] = mode;
  buf[3] = rmode;
  buf[4] = CLAMP(turn_ratio, -100, 100);
  buf[5] = rstate;
  lothar_htonxtl(tacholimit, (buf + 6));
  
  return send(connection, NO_RESPONSE, SETOUTPUTSTATE, buf, 10);
}

/* setinputmode */

int lothar_setinputmode(lothar_connection_t *connection,
			enum lothar_input_port port,
			enum lothar_sensor_type stype,
			enum lothar_sensor_mode smode)
{
  uint8_t buf[3] = {port, stype, smode};
  return send(connection, NO_RESPONSE, SETINPUTMODE, buf, 3);
}

/* getoutputstate */

int lothar_getoutputstate(lothar_connection_t *connection,
			  enum lothar_output_port port,
			  int8_t *power,
			  enum lothar_output_motor_mode *mode,
			  enum lothar_output_regulation_mode *rmode,
			  uint8_t *turn_ratio,
			  enum lothar_output_runstate *runstate,
			  uint32_t *tacholimit,
			  int32_t *tachocount,
			  int32_t *blocktachocount,
			  int32_t *rotationcount)
{
  int status;
  uint8_t buf[25];

  buf[0] = port; // quick n dirty

  if((status = send(connection, RESPONSE, GETOUTPUTSTATE, buf, 1)) < 0)
    return status;
  
  if((status = recv(connection, GETOUTPUTSTATE, buf, 25)) < 0)
    return status;

  if(buf[3] != port) // huh???
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_UNKOWN_ERROR);

  if(power) 
    *power = buf[4];
  if(mode)  
    *mode = buf[5];
  if(rmode)
    *rmode = buf[6];
  if(turn_ratio)
    *turn_ratio = buf[7];
  if(runstate)
    *runstate = buf[8];
  if(tacholimit)
    *tacholimit = lothar_nxttohl((buf + 9));
  if(tachocount)
    *tachocount = lothar_nxttohl((buf + 13));
  if(blocktachocount)
    *blocktachocount = lothar_nxttohl((buf + 17));
  if(rotationcount)
    *rotationcount = lothar_nxttohl((buf + 21));

  return status;
}

/* getinputvalues */

int lothar_getinputvalues(lothar_connection_t *connection,
			  enum lothar_input_port port,
			  uint8_t *valid,
			  uint8_t *calibrated,
			  enum lothar_sensor_type *type,
			  enum lothar_sensor_mode *mode,
			  uint16_t *raw_value,
			  uint16_t *norm_value,
			  int16_t *scaled_value,
			  int16_t *calibrated_value)
{
  int status;
  uint8_t buf[16];

  buf[0] = port;

  if((status = send(connection, RESPONSE, GETINPUTVALUES, buf, 1)) < 0)
    return status;

  if((status = recv(connection, GETINPUTVALUES, buf, 16)) < 0)
    return status;

  if(buf[3] != port) // ???
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_UNKOWN_ERROR);

  if(valid)
    *valid = buf[4];
  if(calibrated)
    *calibrated = buf[5];
  if(type)
    *type = buf[6];
  if(mode)
    *mode = buf[7];
  if(raw_value)
    *raw_value = lothar_nxttohs((buf + 8));
  if(norm_value)
    *norm_value = lothar_nxttohs((buf + 10));
  if(scaled_value)
    *scaled_value = lothar_nxttohs((buf + 12));
  if(calibrated_value)
    *calibrated_value = lothar_nxttohs((buf + 14));
    
  return status;
}

/* resetinputscaledvalue */

int lothar_resetinputscaledvalue(lothar_connection_t *connection, enum lothar_input_port port)
{
  uint8_t p = port;
  return send(connection, NO_RESPONSE, RESETINPUTSCALEDVALUE, &p, 1);
}

/* messagewrite */

int lothar_messagewrite(lothar_connection_t *connection, uint8_t inbox, uint8_t const *data, size_t len)
{
  uint8_t *buf;
  int status;

  if(inbox > 9 || len > 59)
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_INVALID_ARGUMENT);

  buf = (uint8_t *)lothar_malloc(len + 2);

  buf[0] = inbox;
  buf[1] = len;
  memcpy(buf + 2, data, len);

  status = send(connection, NO_RESPONSE, MESSAGEWRITE, buf, len + 2);
  free(buf);

  return status;
}

/* resetmotorposition */

int lothar_resetmotorposition(lothar_connection_t *connection, enum lothar_output_port port, uint8_t relative)
{
  uint8_t buf[2] = {port, relative ? 1 : 0};
  return -send(connection, NO_RESPONSE, RESETMOTORPOSITION, buf, 2);
}

/* getbatterylevel */

int lothar_getbatterylevel(lothar_connection_t *connection, uint16_t *batterylevel)
{
  int status;
  uint8_t buf[5];

  if((status = send(connection, RESPONSE, GETBATTERYLEVEL, NULL, 0)) < 0)
    return status;

  if((status = recv(connection, GETBATTERYLEVEL, buf, 5)) < 0)
    return status;
  
  if(buf[2])
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_NXT_READ_ERROR);

  if(batterylevel)
    *batterylevel = lothar_nxttohs(buf + 3);
  
  return status;
}

/* stopsoundplayback */

int lothar_stopsoundplayback(lothar_connection_t *connection)
{
  return send(connection, NO_RESPONSE, STOPSOUNDPLAYBACK, NULL, 0);
}

/* keepalive */

int lothar_keepalive(lothar_connection_t *connection, uint32_t *sleeptime)
{
  int status;
  uint8_t buf[7];

  if((status = send(connection, RESPONSE, KEEPALIVE, NULL, 0)) < 0)
    return status;

  if((status = recv(connection, KEEPALIVE, buf, 7)) < 0)
    return status;

  if(buf[2])
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_NXT_READ_ERROR);

  if(sleeptime)
    *sleeptime = lothar_nxttohl(buf + 3);

  return 0;
}

/* lsgetstatus */

int lothar_lsgetstatus(lothar_connection_t *connection, enum lothar_input_port port, uint8_t *bytesready)
{
  int status;
  uint8_t buf[4];
  buf[0] = port;

  if((status = send(connection, RESPONSE, LSGETSTATUS, buf, 1)) < 0)
    return status;

  if((status = recv(connection, LSGETSTATUS, buf, 4)) < 0)
    return status;

  if(bytesready)
    *bytesready = buf[3];

  return 0;  
}

/* lswrite */

int lothar_lswrite(lothar_connection_t *connection, enum lothar_input_port port, uint8_t const *txdata, uint8_t txlen, uint8_t rxlen)
{
  int status;
  uint8_t *buf;

  if(txlen > 16 || rxlen > 16)
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_INVALID_ARGUMENT);

  buf = (uint8_t *)lothar_malloc(3 + txlen);
  buf[0] = port;
  buf[1] = txlen;
  buf[2] = rxlen;
  memcpy(buf + 3, txdata, txlen);

  status = send(connection, NO_RESPONSE, LSWRITE, buf, txlen + 3);

  free(buf);
  
  return status;  
}

/* lsread */

int lothar_lsread(lothar_connection_t *connection, enum lothar_input_port port, uint8_t *rxdata, size_t bufsize, uint8_t *rxlen)
{
  int status;
  uint8_t buf[20];

  buf[0] = port;

  if((status = send(connection, RESPONSE, LSREAD, buf, 1)) < 0)
    return status;

  if((status = recv(connection, LSREAD, buf, 20)) < 0)
    return status;
  
  if(rxlen)
    *rxlen = buf[3];

  if(rxdata)
  {
    if(bufsize < buf[3])
    {
      memcpy(rxdata, buf + 4, bufsize);
      LOTHAR_DEBUG("%d vs %d\n", (int)bufsize, (int)buf[3]);
      LOTHAR_RETURN_ERROR(LOTHAR_ERROR_BUFFER_TOO_SMALL);
    }
    else
    {
      memcpy(rxdata, buf + 4, buf[3]);
      status = 0;
    }
  }

  return status;
}

/* getcurrentprogramname */

int lothar_getcurrentprogramname(lothar_connection_t *connection, char filename[19])
{
  int status;
  uint8_t buf[22];

  if((status = send(connection, RESPONSE, GETCURRENTPROGRAMNAME, NULL, 0)) < 0)
    return status;

  if((status = recv(connection, GETCURRENTPROGRAMNAME, buf, 22)) < 0)
    return status;

  if(filename)
    memcpy(filename, buf + 3, 19);

  return 0;
}

/* messageread */

int lothar_messageread(lothar_connection_t *connection, uint8_t remote_inbox, uint8_t local_inbox, uint8_t remove, uint8_t data[59], uint8_t *len)
{
  int status;
  uint8_t buf[64];

  if(remote_inbox > 16 || local_inbox > 9)
    LOTHAR_RETURN_ERROR(LOTHAR_ERROR_INVALID_ARGUMENT);

  buf[0] = remote_inbox;
  buf[1] = local_inbox;
  buf[2] = remove;

  if((status = send(connection, RESPONSE, MESSAGEREAD, buf, 3)) < 0)
    return status;

  if((status = recv(connection, MESSAGEREAD, buf, 64)) < 0)
    return status;

  if(len)
    *len = buf[4];

  if(data)
  {
    if(buf[4] > 59) // should never happen
    {
      memcpy(data, buf + 5, 59);
      LOTHAR_RETURN_ERROR(LOTHAR_ERROR_BUFFER_TOO_SMALL);
    }
    else
    {
      memcpy(data, buf + 5, buf[4]);
      status = 0;
    }
  }

  return status;
}
