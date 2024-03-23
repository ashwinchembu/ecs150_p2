#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "sem.h"
#include "private.h"


/*======================================================================*/
/* Data Structures */
/*======================================================================*/

struct semaphore
{
	int sem_count;			// Resource count
	struct queue *sem_q;	// Queue for waiting threads
};

/*======================================================================*/
/* Implementation */
/*======================================================================*/

/* Create and initialize semaphore */
sem_t sem_create(size_t count)
{
	struct semaphore *semObj = malloc(sizeof(struct semaphore));

	if ((int)count < 0)				// Set semaphore struct resource count
	{
		semObj->sem_count = 0;
	}
	else
	{
		semObj->sem_count = count;
	}

	semObj->sem_q = queue_create();	// Create queue for semaphore

	return semObj;
}

/* Destroy semaphore */
int sem_destroy(sem_t sem)
{
	if (sem == NULL)
	{
		return -1;
	}

	queue_destroy(sem->sem_q);	// Destroy queue
	free(sem);				// Free memory

	return 0;
}

/* Take resource from semaphore */
int sem_down(sem_t sem)
{
	if (sem == NULL)
	{
		return -1;
	}

	preempt_disable();
	/* If no more resources left to take, block thread and add to waiting queue */
	while (sem->sem_count == 0)	
	{
		struct uthread_tcb *blockThread = uthread_current();
		queue_enqueue(sem->sem_q, blockThread);
		uthread_block();
	}

	sem->sem_count--;	// Decrease resource count

	preempt_enable();
	return 0;
}

/* Release resource to semaphore */
int sem_up(sem_t sem)
{
	if (sem == NULL)
	{
		return -1;
	}

	preempt_disable();
	sem->sem_count++;	// Increase resource count

	/* Give resource to next thread waiting in queue */
	if (sem->sem_count > 0)
	{
		struct uthread_tcb *unblockThread;							// Get thread
		int valid = queue_dequeue(sem->sem_q, (void **)&unblockThread);

		if (valid != -1)
		{
			uthread_unblock(unblockThread);							// Unblock thread
		}
	}
	preempt_enable();

	return 0;
}
