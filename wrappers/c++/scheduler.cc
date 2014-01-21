#include "scheduler.hh"

using namespace std;
using namespace lothar;

class _ActiveObject
{
public:
  _ActiveObject(ActiveObjectPtr &p) : object(p)
  {}
  ActiveObjectPtr object;
};

namespace 
{
  void function_callback(void *_object)
  {
    ActiveObjectPtr object = reinterpret_cast<_ActiveObject *>(_object)->object;
    // C can't handle exceptions, so we have to do it here
    try
    {
      object->run();
    }
    catch(std::exception const &e)
    {
      LOTHAR_WARN("%s\n", e.what());
    }
    catch(...)
    {
      LOTHAR_WARN("Unkown exception\n");
    }
  }

  void cleanup_callback(void *_object)
  {
    ActiveObjectPtr object = reinterpret_cast<_ActiveObject *>(_object)->object;
    // C can't handle exceptions, so we have to do it here
    try
    {
      object->clean();
    }
    catch(std::exception const &e)
    {
      LOTHAR_WARN("%s\n", e.what());
    }
    catch(...)
    {
      LOTHAR_WARN("Unkown exception\n");
    }
  }
}

void Scheduler::add(ActiveObjectPtr &active_object, lothar::time_t arrival_time)
{
  _ActiveObject *data = new _ActiveObject(active_object);
  check_return(lothar_scheduler_add(d_scheduler, function_callback, cleanup_callback, data, arrival_time, active_object->estimate(), active_object->nice()));
}

void Scheduler::add_interval(ActiveObjectPtr &active_object, lothar::time_t arrival_time, lothar::time_t interval, unsigned ntimes)
{
  _ActiveObject *data = new _ActiveObject(active_object);
  check_return(lothar_scheduler_add_interval(d_scheduler, function_callback, cleanup_callback, data, arrival_time, active_object->estimate(), active_object->nice(), interval, ntimes));
}

bool Scheduler::empty() const
{
  int e;
  check_return(lothar_scheduler_empty(d_scheduler, &e));
  return e;
}

size_t Scheduler::size() const
{
  size_t s;
  check_return(lothar_scheduler_size(d_scheduler, &s));
  return s;
}

lothar::time_t Scheduler::peek() const
{
  lothar::time_t t;
  check_return(lothar_scheduler_peek(d_scheduler, &t));
  return t;
}


