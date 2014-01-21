#ifndef LOTHAR_SCHEDULER_HH
#define LOTHAR_SCHEDULER_HH

#include "utils.hh"
#include "scheduler.h"

namespace lothar
{
  /** \brief Active object
   *
   * Derive from this class to have a callable object in the scheduler.
   */
  class ActiveObject
  {
    time_t   d_estimate;
    unsigned d_nice;   

  public:
    /** \brief Constructor
     *
     * \param estimate An estimate of the number of milliseconds a single run() will take.
     * \param nice     A nice value, to prioritize jobs.
     */
    ActiveObject(time_t estimate = 10, unsigned nice = 10) : d_estimate(estimate), d_nice(nice)
    {}

    virtual ~ActiveObject()
    {}

    /** \brief run the object
     *
     * Override this to make your object do it's job
     *
     * \note If this function throws an exception, the scheduler will catch it, print it, and continue.
     */
    virtual void run() = 0;

    /** \brief clean up the object
     *
     * This is done as a member function and not as a desctructor, to account for multiple parts of your code
     * having a reference to this object.
     */
    virtual void clean() = 0;

    time_t const estimate() const
    {
      return d_estimate;
    }

    unsigned nice() const
    {
      return d_nice;
    }
  };

  typedef lothar::shared_ptr<ActiveObject> ActiveObjectPtr;

  /** \brief Scheduler object
   */
  class Scheduler : public no_copy
  {
    lothar_scheduler_t *d_scheduler;

  public:
    Scheduler() : d_scheduler(lothar_scheduler_create())
    {
      if(!d_scheduler)
        throw Error();
    }

    ~Scheduler()
    {
      if(d_scheduler)
        lothar_scheduler_destroy(&d_scheduler);
    }

    /** \brief Access to the underlying lothar_scheduler_t
     */
    operator lothar_scheduler_t const *() const
    {
      return d_scheduler;
    }

    /** \brief Access to the underlying lothar_scheduler_t
     */
    operator lothar_scheduler_t *()
    {
      return d_scheduler;
    }

    /** \brief Add a job to be scheduler
     *
     * \param active_object Active object to add.
     * \param arrival_time  when do you want the job to be executed, relative to now (so a value of 2 means two milliseconds in the future) 
     */
    void add(ActiveObjectPtr &active_object, time_t arrival_time);

    /** \brief Add a reoccuring job, the job will run repeatedly
     *
     * parameters are  same as add(), but with addition of:
     * \param interval interval between the jobs. Please don't put to 0, or your scheduler will clog up (or use a high nice value)
     * \param ntimes   how often should the function be called. 0 means keep running till doomsday comes or the scheduler is explicitely stopped.
     */
    void add_interval(ActiveObjectPtr &active_object, time_t arrival_time, time_t interval, unsigned ntimes = 0);

    /** \brief Returns true if there are no jobs left to run
     */
    bool empty() const;
    
    /** \brief Returns the number of jobs left
     */
    size_t size() const;

    /** \brief Returns the time until the next job starts to run
     */
    time_t peek() const;

    /** \brief run the next job.
     *
     * If the next job is scheduled in the future, this function will block until that time.
     * \throws Error with LOTHAR_ERROR_ENTITY_CLOSED if there are no jobs left to run.
     */
    void run_single()
    {
      check_return(lothar_scheduler_run_single(d_scheduler));
    }

    /** \brief keep running all jobs in the scheduler, until there are none left 
     */
    void run()
    {
      check_return(lothar_scheduler_run(d_scheduler));
    }

    /** \brief stop the scheduler, purging all remaining jobs 
     */
    void stop()
    {
      check_return(lothar_scheduler_stop(d_scheduler));
    }

    /** \brief Is the scheduler done?
     *
     * Equivalent to empty()
     */
    bool done() const
    {
      return lothar_scheduler_done(d_scheduler);
    }

    /** \brief The time until the next job is due
     */
    time_t next() const
    {
      return lothar_scheduler_next(d_scheduler);
    }
  };
}

#endif // LOTHAR_SCHEDULER_HH
