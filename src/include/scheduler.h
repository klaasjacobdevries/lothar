#ifndef LOTHAR_SCHEDULER_H
#define LOTHAR_SCHEDULER_H

#include "utils.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** \file scheduler.h
 *
 * This is a general-purpose scheduler, often usefull in interactive applications. Note that this is somewhat the odd
 * one out, it is not really part of lothar but more an advanced utility.
 */

/** \brief Opaque scheduler object
 */
struct heap_t;
typedef struct heap_t lothar_scheduler_t;

/** \brief Create a scheduler instance
 */
lothar_scheduler_t *lothar_scheduler_create(void);

/** \brief Destroy the scheduler
 */
int lothar_scheduler_destroy(lothar_scheduler_t **scheduler);

/** \brief Add a job to be scheduler
 *
 * \param function_callback the function to be executed (may be NULL, but that would be pointless) 
 * \param cleanup_callback  the function to be executed when we're done (suggested ussage: use it to free() private_data) (may be NULL) 
 * \param private_data      pointer to be passed to function_callback and cleanup_callback 
 * \param arrival_time      when do you want the job to be executed, relative to now (so a value of 2 means two milliseconds in the future) 
 * \param estimate          estimate about how long this job is expected to take. Will be updated runtime.
 * \param nice              nice value. Jobs with a higher nice value give way to jobs with a lower nice value
 */
int lothar_scheduler_add(lothar_scheduler_t *scheduler, void (*function_callback)(void *), void (*cleanup_callback)(void *), void *private_data, lothar_time_t arrival_time, lothar_time_t estimate, unsigned nice);

/** \brief Add a reoccuring job, the job will run repeatedly
 *
 * parameters are  same as lothar_scheduler_add(), but with addition of:
 * \param interval interval between the jobs. Please don't put to 0, or your scheduler will clog up (or use a high nice value)
 * \param ntimes   how often should the function be called. 0 means keep running till doomsday comes or the scheduler is explicitely stopped.
 */
int lothar_scheduler_add_interval(lothar_scheduler_t *scheduler, void (*function_callback)(void *), void (*cleanup_callback)(void *), void *private_data, lothar_time_t arrival_time, lothar_time_t estimate, unsigned nice, lothar_time_t interval, unsigned ntimes);

/** \brief Returns true if there are no jobs left to run
 */
int lothar_scheduler_empty(lothar_scheduler_t const *scheduler, int *isempty);

/** \brief Returns the number of jobs left
 */
int lothar_scheduler_size(lothar_scheduler_t const *scheduler, size_t *size);

/** \brief Returns the time until the next job starts to run
 */
int lothar_scheduler_peek(lothar_scheduler_t const *scheduler, lothar_time_t *next);

/** \brief run the next job.
 *
 * If the next job is scheduled in the future, this function will block until that time.
 * returns LOTHAR_ERROR_ENTITY_CLOSED if there are no jobs in the scheduler.
 */
int lothar_scheduler_run_single(lothar_scheduler_t *scheduler);

/** \brief keep running all jobs in the scheduler, until there are none left 
 */
int lothar_scheduler_run(lothar_scheduler_t *scheduler);

/** \brief stop the scheduler, purging all remaining jobs 
 */
int lothar_scheduler_stop(lothar_scheduler_t *scheduler);

/** \brief Is the scheduler done?
 *
 *breach of consitency 1 on true, 0 on false 
 */
int lothar_scheduler_done(lothar_scheduler_t const *scheduler);

/** \brief The time until the next job is due
 */
lothar_time_t lothar_scheduler_next(lothar_scheduler_t const *scheduler);

#ifdef __cplusplus
}
#endif

#endif
