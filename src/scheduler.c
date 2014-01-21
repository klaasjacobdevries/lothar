#include "scheduler.h"
#include <limits.h>

#include "heap.h"

#define IS_VALID(pq) { if(!pq) { LOTHAR_FAIL("invalid scheduler priority queue\n"); LOTHAR_RETURN_ERROR(LOTHAR_ERROR_ENTITY_CLOSED); } }

enum whentofree {NEVER, FIRSTTIME, LASTTIME};

typedef struct
{
  void (*function)(void *); // the function callback
  void (*cleanup)(void *);     // cleanup function
  void *private_data;       // argument to the function
  
  lothar_time_t arrival_time; // when do we want this job to be executed
  lothar_time_t estimate;     // estimate about how long it will take
  unsigned nice;              // nice value

  enum whentofree wtf; // when to free (used to keep distinction between one-time and reoccuring jobs)
} job_t;

/* comparitor function, used to detirmine the priority of a over b */
static int job_cmp(void *_a, void *_b)
{
  job_t *a = (job_t *)_a;
  job_t *b = (job_t *)_b;

  unsigned nice_a = a->nice;
  unsigned nice_b = b->nice;

  lothar_time_t start_a = a->arrival_time;
  lothar_time_t start_b = b->arrival_time;

  lothar_time_t end_a = start_a + a->estimate;
  lothar_time_t end_b = start_b + b->estimate;

  if(end_a < start_b) // a will be done before b starts
    return 1;
  else if(end_b < start_a) // b will be done before a starts
    return -1;
  else // the jobs overlap, chooche priority
  {
    if(nice_a < nice_b)
      return 1;
    else if(nice_b < nice_a)
      return -1;
    else // they are equally nice, just choose starting time
    {
      if(start_a < start_b)
	return 1;
      else if(start_b < start_a)
	return -1;
      else // same arrival time, choose the shortest one
      {
	if(end_a < end_b)
	  return 1;
	else if(end_b < end_a)
	  return -1;
      }
    }
  }

  // we really don't care here

  return 0;
}

lothar_scheduler_t *lothar_scheduler_create(void)
{
  return (lothar_scheduler_t *)heap_new(job_cmp);
}

int lothar_scheduler_destroy(lothar_scheduler_t **scheduler)
{
  if (!scheduler || !(*scheduler))
    return 0;

  lothar_scheduler_stop(*scheduler); // purge jobjs
  heap_free((heap_t **)scheduler, 0);
  scheduler = NULL;
  
  return 0;
}


// shared code between add and interval_add
static int scheduler_add(lothar_scheduler_t *scheduler, void (*function)(void *), void (*cleanup)(void *), void *private_data, lothar_time_t arrival_time_fromnow, lothar_time_t estimate, unsigned nice, enum whentofree wtf)
{
  job_t *job;

  IS_VALID(scheduler);

  job = lothar_malloc(sizeof(job_t));

  job->function     = function;
  job->cleanup      = cleanup;
  job->private_data = private_data;
  job->arrival_time = lothar_time() + arrival_time_fromnow;
  job->estimate     = estimate;
  job->nice         = nice;
  job->wtf          = wtf;

  heap_push((heap_t *)scheduler, job);

  return 0;
}

int lothar_scheduler_add(lothar_scheduler_t *scheduler, void (*function)(void *), void (*cleanup)(void *), void *private_data, lothar_time_t arrival_time_fromnow, lothar_time_t estimate, unsigned nice)
{
  return scheduler_add(scheduler, function, cleanup, private_data, arrival_time_fromnow, estimate, nice, FIRSTTIME);
}

int lothar_scheduler_empty(lothar_scheduler_t const *scheduler, int *isempty)
{
  IS_VALID(scheduler);

  if(isempty)
    *isempty = heap_empty((heap_t const *)scheduler);

  return 0;
}

int lothar_scheduler_size(lothar_scheduler_t const *scheduler, size_t *size)
{
  IS_VALID(scheduler);
  
  if(size)
    *size = heap_size((heap_t const *)scheduler);
  
  return 0;
}

int lothar_scheduler_peek(lothar_scheduler_t const *scheduler, lothar_time_t *next)
{
  IS_VALID(scheduler);

  if(next)
    *next = heap_empty((heap_t const *)scheduler) ? 0 : ((job_t const *)heap_peek((heap_t const *)scheduler))->arrival_time;
  
  return 0;
}

int lothar_scheduler_run_single(lothar_scheduler_t *scheduler)
{
  IS_VALID(scheduler);

  if(!heap_empty((heap_t *)scheduler))
  {
    lothar_time_t next = lothar_scheduler_next(scheduler);
    lothar_time_t now  = lothar_time();
    job_t *job;
    int status;
    
    if(next > now)
    {
      if((status = lothar_msleep(next - now)) < 0)
	return status;
    }

    job = (job_t *)heap_pop((heap_t *)scheduler);

    if(job->function)
      job->function(job->private_data);

    if(job->wtf == FIRSTTIME && job->cleanup)
      job->cleanup(job->private_data);

    free(job);
  }
  
  return 0;
}

int lothar_scheduler_run(lothar_scheduler_t *scheduler)
{
  int status;

  IS_VALID(scheduler);

  // we test for scheduler as it is possible that a job called lothar_scheduler_stop()
  while(!heap_empty((heap_t *)scheduler))
  {
    if((status = lothar_scheduler_run_single(scheduler)) < 0)
      return status;
  }

  return 0;
}

int lothar_scheduler_stop(lothar_scheduler_t *scheduler)
{
  IS_VALID(scheduler);

  while(!lothar_scheduler_done(scheduler))
  {
    job_t *job = (job_t *)heap_pop((heap_t *)scheduler);

    if(job->wtf != NEVER && job->cleanup)
      job->cleanup(job->private_data);

    free(job);
  }
  
  return 0;
}

int lothar_scheduler_done(lothar_scheduler_t const *scheduler)
{
  int status, isempty;

  if((status = lothar_scheduler_empty(scheduler, &isempty)) < 0)
  {
    LOTHAR_WARN("surpressed error: (%d) %s\n", -status, lothar_strerror(-status));
    return 1; // return true in this case, as most likely this will indicate that the scheduler can't be run again
  }

  return isempty;
}

lothar_time_t lothar_scheduler_next(lothar_scheduler_t const *scheduler)
{
  lothar_time_t next;
  int status;

  IS_VALID(scheduler);

  if((status = lothar_scheduler_peek(scheduler, &next)) < 0)
  {
    LOTHAR_WARN("surpressed error: (%d) %s\n", -status, lothar_strerror(-status));
    return 0;
  }

  return next;
}

typedef struct
{
  lothar_scheduler_t *scheduler; // connected scheduler

  void (*function)(void *); // callback function
  void (*cleanup)(void *);  // cleanup function
  void *private_data;       // argument to callback function

  lothar_time_t interval;   // interval
  unsigned ntimes;          // how often should we be called? if this drops to zero, stop running. If initialized at zero, run forever
  unsigned nice;            // are we nice

  lothar_time_t average; // what is the average time this job needs?
  unsigned called;       // used to compute average
} interval_job_t;

static void interval_cleanup(interval_job_t *job)
{
  if(job->cleanup)
    job->cleanup(job->private_data);

  free(job);
}

static void interval_function(interval_job_t *job)
{
  lothar_time_t timer = lothar_timer(NULL);
  lothar_scheduler_t *scheduler = job->scheduler;

  if(job->function)
    job->function(job->private_data);

  if(job->ntimes == 1) // this was the last call
    return;

  if(job->ntimes)
    --job->ntimes;

  // reschedule
  if(!scheduler || scheduler_add(scheduler, (void (*)(void *))interval_function, (void (*)(void *))interval_cleanup, job, job->interval, job->average, job->nice, job->ntimes == 1 ? FIRSTTIME : LASTTIME))
  {
    interval_cleanup(job);
    return;
  }

  // hmm, after the scheduler_add, to include the time it takes to add the job to the scheduler
  if(job->called < UINT_MAX)
  {
    job->average = (job->average * job->called + lothar_timer(&timer)) / (job->called + 1);
    ++job->called;
  }
}

int lothar_scheduler_add_interval(lothar_scheduler_t *scheduler, void (*function)(void *), void (*cleanup)(void *), void *private_data, lothar_time_t first_arrival_time_fromnow, lothar_time_t estimate, unsigned nice, lothar_time_t interval, unsigned ntimes)
{
  interval_job_t *job;
  IS_VALID(scheduler);

  job = (interval_job_t *)lothar_malloc(sizeof(interval_job_t));

  job->scheduler    = scheduler;
  job->function     = function;
  job->cleanup      = cleanup;
  job->private_data = private_data;
  job->interval     = interval;
  job->ntimes       = ntimes;
  job->nice         = nice;

  // we weigh the estimate in the calculation
  job->average = estimate;
  job->called  = 1;

  return scheduler_add(scheduler, (void (*)(void *))interval_function, (void (*)(void *))interval_cleanup, job, first_arrival_time_fromnow, estimate, nice, ntimes == 1 ? FIRSTTIME : LASTTIME);
}
