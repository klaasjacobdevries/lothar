#include "motor.hh"

using namespace std;
using namespace lothar;

output_port Motor::port() const
{
  output_port p;
  check_return(lothar_motor_port(*this, &p));
  return p;
}

int32_t Motor::degrees(bool relative)
{
  int32_t d;
  check_return(lothar_motor_degrees(*this, &d, relative));
  return d;
}

int8_t Motor::power()
{
  int8_t p;
  check_return(lothar_motor_power(*this, &p));
  return p;
}

