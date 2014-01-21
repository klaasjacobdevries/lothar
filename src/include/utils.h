#ifndef LOTHAR_UTILS_H
#define LOTHAR_UTILS_H

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "error_handling.h"
#include "config.h"

#ifndef M_PI
#define M_PI 3.141592653589793
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/** \brief Clamp a value between to bounds
 *
 * This ensures that val is no smaller than min and no larger than max
 */
#define CLAMP(val, min, max) ( (val < min ? min : (val > max ? max : val)) )

/** \brief The minimum value of a and b
 */
#define MIN(a, b) (a > b ? b : a)

/** \brief The maximum value of a and b
 */
#define MAX(a, b) (a > b ? a : b)

/** \brief Convert degrees to radians
 */
#define DEGTORAD(deg) (deg * M_PI / 180.0)

/** \brief Convert radians to degrees
 */
#define RADTODEG(rad) (rad * 180.0 / M_PI)

/** \brief Convert a short from host endianness to NXT (small) endianness
 *
 * \param val a 32 bit integer
 * \param buf a 2 byte buffer to hold the result
 */
static inline void lothar_htonxts(uint16_t val, uint8_t buf[2])
{
  buf[0] = val & 0xFF; buf[1] = val >> 8;
}

/** \brief Convert a short from NXT (small) endianness to host endianness
 *
 * \param buf a 2 byte buffer received from the NXT
 * \returns the value in host endianness
 */
static inline uint16_t lothar_nxttohs(uint8_t const buf[2])
{
  return buf[0] | (buf[1] << 8);
}

/** \brief Convert a long from host endianness to NXT (small) endianness
 *
 * \param val a 32 bit integer
 * \param buf a 4 byte buffer to hold the result
 */
static inline void lothar_htonxtl(uint32_t val, uint8_t buf[4])
{
  buf[0] = val; buf[1] = val >> 8; buf[2] = val >> 16; buf[3] = val >> 24;
}

/** \brief Convert a long from NXT (small) endianness to host endianness
 *
 * \param buf a 4 byte buffer received from the NXT
 * \returns the value in host endianness
 */
static inline uint32_t lothar_nxttohl(uint8_t const buf[4])
{
  return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

/** \brief The output ports (A-C, ALL)
 */
enum lothar_output_port
{
  OUTPUT_A   = 0,   
  OUTPUT_B   = 1,
  OUTPUT_C   = 2,
  /** Special value to describe all ports */
  OUTPUT_ALL = 0xFF 
};

/** \brief The input ports (1-4)
 */
enum lothar_input_port
{
  INPUT_1 = 0,
  INPUT_2 = 1,
  INPUT_3 = 2,
  INPUT_4 = 3 
};

/** \brief The motor modes
 */
enum lothar_output_motor_mode
{
  MOTOR_MODE_MOTORON   = 0x01,
  MOTOR_MODE_BRAKE     = 0x02,
  MOTOR_MODE_REGULATED = 0x04
};

/** \brief The motor regulation modes
 */
enum lothar_output_regulation_mode
{
  REGULATION_MODE_IDLE       = 0x00,
  REGULATION_MODE_SPEED      = 0x01,
  REGULATION_MODE_MOTOR_SYNC = 0x02
};

/** \brief The motor runstates
 */
enum lothar_output_runstate
{
  RUNSTATE_IDLE     = 0x00,
  RUNSTATE_RAMPUP   = 0x10,
  RUNSTATE_RUNNING  = 0x20,
  RUNSTATE_RAMPDOWN = 0x40
};

/** \brief Varius types of sensors
 */
enum lothar_sensor_type
{
  /** dummy */
  SENSOR_NO_SENSOR          = 0x00, 
  /** touch sensor */
  SENSOR_SWITCH             = 0x01,
  /** (legacy) temperature */
  SENSOR_TEMPERATURE        = 0x02,
  /** (legacy) light sensor */
  SENSOR_REFLECTION         = 0x03,
  /** (legacy) rotation sensor */
  SENSOR_ANGLE              = 0x04, 
  /** light sensor */
  SENSOR_LIGHT_ACTIVE       = 0x05, 
  /** light sensor (inactive) */
  SENSOR_LIGHT_INACTIVE     = 0x06,
  /** sound sensor */
  SENSOR_SOUND_DB           = 0x07, 
  /** idem */
  SENSOR_SOUND_DBA          = 0x08, 
  /** not used */
  SENSOR_CUSTOM             = 0x09,
  /** ? */
  SENSOR_LOWSPEED           = 0x0A,
  /** for ultrasound */
  SENSOR_LOWSPEED_9V        = 0x0B,
  
  /* these are documented in the NXT executable file specification, but curiously not in the direct commands documentation */
  /** not used */
  SENSOR_HIGHSPEED          = 0x0C, 
  /** color detector full */
  SENSOR_COLORFULL          = 0x0D, 
  /** red light */
  SENSOR_COLORRED           = 0x0E,
  /** green light */
  SENSOR_COLORGREEN         = 0x0F,
  /** blue light */
  SENSOR_COLORBLUE          = 0x10,
  /** no light */
  SENSOR_COLORNONE          = 0x11,
  /** end marker */
  SENSOR_NO_OF_SENSOR_TYPES = 0x12 
};

/** \brief varius input modes */
enum lothar_sensor_mode
{
  SENSOR_MODE_RAWMODE          = 0x00,
  SENSOR_MODE_BOOLEANMODE      = 0x20,
  SENSOR_MODE_TRANSITIONMODE   = 0x40,
  SENSOR_MODE_PERIODCOUNTMODE  = 0x60,
  SENSOR_MODE_PCTFULLSCALEMODE = 0x80,
  SENSOR_MODE_CELSIUSMODE      = 0xA0,
  SENSOR_MODE_FAHRENHEITMODE   = 0xC0,
  SENSOR_MODE_ANGLESTEPMODE    = 0xE0,
  SENSOR_MODE_SLOPEMASK        = 0x1F,
  SENSOR_MODE_MODEMASK         = 0xE0
};

/** \brief enum for the SENSOR_COLORFULL detection mode */ 
enum lothar_color
{
  COLOR_BLACK  = 1,
  COLOR_BLUE   = 2,
  COLOR_GREEN  = 3,
  COLOR_YELLOW = 4,
  COLOR_RED    = 5,
  COLOR_WHITE  = 6
};

#define SENSOR_MAX_RETRIES 10 // try this many times at max to get valid data from a sensor

/* some time related utilities */

/* \brief Describes a time interval
 *
 * times are expressed in unsigned 64 bit values, representing milliseconds 
 * here, the epoch 0 is defined as the first call to lothar_time()
 */
typedef uint64_t lothar_time_t; 

/** \brief sleep for the specified number of milliseconds 
 */
int lothar_msleep(lothar_time_t ms);

/** \brief simple timer, returns the number of milliseconds since the last time this function was called (0 if this is
 * the first time this function is called)
 */
lothar_time_t lothar_time(void);

/** \brief slightly more advanced timer.
 *
 * use lothar_timer(NULL) to create a new timer, and pass the return value to the next calls to get the number of
 * milliseconds since the creation time
 */
static inline lothar_time_t lothar_timer(lothar_time_t *timer)
{
  return timer ? lothar_time() - *timer : lothar_time();
}

/** \brief I don't want to check the return value of malloc every time, just fail miserably when out of memory (as we
 * probably should)
 */
void *lothar_malloc(size_t size);

/** \brief I don't want to check the return value of malloc every time, just fail miserably when out of memory (as we
 * probably should)
 */
void *lothar_realloc(void *p, size_t size);

#ifdef __cplusplus
}
#endif

#endif
