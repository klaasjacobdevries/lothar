#include "sensor.hh"

using namespace std;
using namespace lothar;

input_port Sensor::port() const
{
  input_port p;
  check_return(lothar_sensor_port(*this, &p));
  return p;
}

sensor_type Sensor::type() const
{
  sensor_type t;
  check_return(lothar_sensor_type(*this, &t));
  return t;
}

uint16_t Sensor::value()
{
  uint16_t v;
  check_return(lothar_sensor_value(*this, &v));
  return v;
}

uint16_t Sensor::minimum() const
{
  uint16_t m;
  check_return(lothar_sensor_minimum(*this, &m));
  return m;
}

uint16_t Sensor::maximum() const
{
  uint16_t m;
  check_return(lothar_sensor_maximum(*this, &m));
  return m;
}
