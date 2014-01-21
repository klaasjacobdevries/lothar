#include "steering.h"
#include "config.h"
#include <math.h>

#define DEFAULT_CONVERSION 8

static inline double sanitize_rad(double r)
{
  if(r >= 2 * M_PI)
    return r - ((int)(r / (2 * M_PI)) * 2 * M_PI);
  else if(r < 0)
    return r + ((int)(1 + (r / (2 * M_PI))) * 2 * M_PI);
  return r;
}

static inline double deg_to_rad(int d)
{
  return (double)d * M_PI / 180.0;
}

#define IS_VALID(s) { if(!s) { LOTHAR_FAIL("invalid steering\n"); LOTHAR_RETURN_ERROR(LOTHAR_ERROR_ENTITY_CLOSED);} }

struct lothar_steering_t
{
  lothar_motor_t *left;
  lothar_motor_t *right;
  double radius;
  double distance;
  lothar_time_t t;
  double x;
  double y;
  double o;

  double vl;
  double vr;

  double left_conversion;
  double right_conversion;
};

lothar_steering_t *lothar_steering_open(lothar_connection_t *connection, enum lothar_output_port left, enum lothar_output_port right, double radius, double distance)
{
  lothar_steering_t *result = (lothar_steering_t *)lothar_malloc(sizeof(lothar_steering_t));

  result->left = lothar_motor_open(connection, left);
  result->right = lothar_motor_open(connection, right);
  result->radius = radius;
  result->distance = distance;

  result->t = lothar_timer(NULL);
  result->x = 0;
  result->y = 0;
  result->o = 0;

  result->vl = 0;
  result->vr = 0;

  result->left_conversion = deg_to_rad(DEFAULT_CONVERSION);
  result->right_conversion = deg_to_rad(DEFAULT_CONVERSION);

  lothar_motor_reset(result->left, 0);
  lothar_motor_reset(result->right, 0);

  return result;  
}

int lothar_steering_close(lothar_steering_t **steering)
{
  IS_VALID(steering);

  lothar_motor_close(&((*steering)->left));
  lothar_motor_close(&((*steering)->right));

  free(*steering);
  steering = NULL;

  return 0;
}

int lothar_steering_connection(lothar_steering_t const *steering, lothar_connection_t **connection)
{
  IS_VALID(steering);

  return lothar_motor_connection(steering->left, connection);
}

int lothar_steering_forward(lothar_steering_t *steering, double speed)
{
  int status;
  double v;
  int8_t pl;
  int8_t pr;

  IS_VALID(steering);
  
  if((status = lothar_steering_update(steering)) < 0)
    return status;

  v = speed / steering->radius; // rad/s

  pl = (int8_t)CLAMP(v / steering->left_conversion, -100, 100);
  pr = (int8_t)CLAMP(v / steering->right_conversion, -100, 100);

  LOTHAR_DEBUG("v = %f, p = (%d, %d) %f %f\n", v, pl, pr, speed, steering->radius);

  status = MIN(lothar_motor_run(steering->left, pl), lothar_motor_run(steering->right, pr));
  return status;
}

int lothar_steering_turn(lothar_steering_t *steering, double speed, double turnspeed)
{
  int status;
  double vl;
  double vr;
  double dv;
  int8_t pl;
  int8_t pr;

  IS_VALID(steering);

  if((status = lothar_steering_update(steering)) < 0)
    return status;

  dv = turnspeed * steering->distance;
  vl = speed - dv / 2;
  vr = speed + dv / 2;

  vl /= steering->radius; // rad/s
  vr /= steering->radius;

  pl = (int8_t)CLAMP(vl / steering->left_conversion, -100, 100);
  pr = (int8_t)CLAMP(vr / steering->right_conversion, -100, 100);

  LOTHAR_DEBUG("v = (%f, %f), p = (%d, %d) %f %f %f\n", vl, vr, pl, pr, speed, dv, turnspeed);

  status = MIN(lothar_motor_run(steering->left, pl), lothar_motor_run(steering->right, pr));
  return status;
}

int lothar_steering_stop(lothar_steering_t *steering)
{
  int status;
  IS_VALID(steering);

  if((status = lothar_steering_update(steering)))
    return status;

  status = MIN(lothar_motor_stop(steering->left), lothar_motor_stop(steering->right));
  return status;
}

int lothar_steering_brake(lothar_steering_t *steering)
{
  int status;
  IS_VALID(steering);

  if((status = lothar_steering_update(steering)))
    return status;

  status = MIN(lothar_motor_brake(steering->left), lothar_motor_brake(steering->right));
  return status;
}

int lothar_steering_get_odometry(lothar_steering_t const *steering, double *x, double *y, double *o)
{
  IS_VALID(steering);
 
  if(x)
    *x = steering->x;
  if(y)
    *y = steering->y;
  if(o)
    *o = steering->o;

  return 0;
}

int lothar_steering_set_odometry(lothar_steering_t *steering, double x, double y, double o)
{
  IS_VALID(steering);

  steering->x = x;
  steering->y = y;
  steering->o = o;

  return 0;
}

// reference: http://rossum.sourceforge.net/papers/DiffSteer/
int lothar_steering_update(lothar_steering_t *steering)
{
  int status = 0;
  double vl;
  double vr;
  lothar_time_t _t;
  double t;

  int32_t dl;
  int32_t dr;
  int8_t pl;
  int8_t pr;

  IS_VALID(steering);

  _t = lothar_timer(&steering->t);
    
  if(_t)
  {   
    if((status = lothar_motor_degrees(steering->left, &dl, 0)) < 0)
      return status;
    if((status = lothar_motor_reset(steering->left, 0)) < 0)
      return status;
    if((status = lothar_motor_degrees(steering->right, &dr, 0)) < 0)
      return status;
    if((status = lothar_motor_reset(steering->right, 0)) < 0)
      return status;
   
    steering->t = lothar_timer(NULL);

    if((status = lothar_motor_power(steering->left, &pl)) < 0)
      return status;
    if((status = lothar_motor_power(steering->right, &pr)) < 0)
      return status;

    // optimizing can be done
    t = _t / 1000.0;

    if(dl != dr)
    {
      vl = deg_to_rad(dl) * steering->radius / t;
      vr = deg_to_rad(dr) * steering->radius / t;

      LOTHAR_DEBUG("t = %f, v = (%f, %f), d = (%d, %d)\n", t, vl, vr, dl, dr);

      steering->x += ((steering->distance * (vr + vl)) / (2 * (vr - vl))) * (sin(((vr - vl) * t) / steering->distance + steering->o) - sin(steering->o));
      
      steering->y -= ((steering->distance * (vr + vl)) / (2 * (vr - vl))) * (cos(((vr - vl) * t) / steering->distance + steering->o) - cos(steering->o));
      
      steering->o += (vr - vl) * t / steering->distance;
      
      LOTHAR_DEBUG("x = %f, y = %f, o = %f\n", steering->x, steering->y, steering->o);

      steering->o = sanitize_rad(steering->o);
    }
    else if(dl)
    {
      vl = deg_to_rad(dl) * steering->radius;
      steering->x += vl * cos(steering->o);
      steering->y += vl * sin(steering->o);

      LOTHAR_DEBUG("x = %f, y = %f, o = %f\n", steering->x, steering->y, steering->o);
    }
    
    // this is a precicion tool: don't grow or shrink by more then 10 %
    if(pl && dl)
      steering->left_conversion = CLAMP(deg_to_rad(dl) / (t * pl), steering->left_conversion * 0.9, steering->left_conversion * (1.0 / 0.9));
    if(pr && dr)
      steering->right_conversion = CLAMP(deg_to_rad(dr) / (t * pr), steering->right_conversion * 0.9, steering->right_conversion * (1.0 / 0.9));
      
    LOTHAR_DEBUG("conv = (%f, %f) (%d, %d) (%f) %d,%d\n", steering->left_conversion, steering->right_conversion, pl, pr, t, dl, dr);

  }

  return status;
}

double lothar_steering_x(lothar_steering_t const *steering)
{
  return steering->x;
}

double lothar_steering_y(lothar_steering_t const *steering)
{
  return steering->y;
}

double lothar_steering_o(lothar_steering_t const *steering)
{
  return steering->o;
}
