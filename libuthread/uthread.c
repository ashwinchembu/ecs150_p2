#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

/*======================================================================*/
/* Global variables and macros */
/*======================================================================*/

#define RUNNING 0
#define READY 1
#define BLOCK 2

struct queue *uthread_q;
static uthread_ctx_t *idle_ctx;
static struct uthread_tcb *current_thread = NULL;

/*======================================================================*/
/* Data Structures */
/*======================================================================*/

struct uthread_tcb
{
	uthread_ctx_t *context;
	void *topOfStack;
	int status;
};

/*======================================================================*/
/* Implementation */
/*======================================================================*/

/* Return TCB of currently running thread */
struct uthread_tcb *uthread_current(void)
{
	return current_thread;
}

/* Yield to the next thread in the queue */
void uthread_yield(void)
{
	struct uthread_tcb *nextThread;

	preempt_disable();
	queue_enqueue(uthread_q, uthread_current()); 	// Enqueue current thread
	int valid = queue_dequeue(uthread_q, (void **)&nextThread); // Dequeue next thread to run

	while (valid && nextThread->status != READY)		// Loop until a ready thread is found
	{
		queue_enqueue(uthread_q, nextThread);
		valid = queue_dequeue(uthread_q, (void **)&nextThread);
	}
	preempt_enable();

	struct uthread_tcb *oldThread = uthread_current(); 	// Save old thread for context switch
	current_thread = nextThread;						// Update current thread global var
	uthread_ctx_switch(oldThread->context, current_thread->context);	// Switch to new thread
}

/* Exit currently running thread */
void uthread_exit(void)
{
	struct uthread_tcb *nextThread;
	
	preempt_disable();
	int valid = queue_dequeue(uthread_q, (void **)&nextThread);	// Dequeue next thread to run

	while (valid && nextThread->status != READY)		// Loop until a ready thread is found
	{
		queue_enqueue(uthread_q, nextThread);
		valid = queue_dequeue(uthread_q, (void **)&nextThread);
	}

	preempt_enable();

	struct uthread_tcb *exitThread = uthread_current();	// Get old thread and update current
	current_thread = nextThread;

	uthread_ctx_t *context = exitThread->context;			// Save context of old
	uthread_ctx_destroy_stack(exitThread);					// Deallocate stack of old thread
	uthread_ctx_switch(context, current_thread->context);	// Context switch to new thread
}

/* Create new thread */
int uthread_create(uthread_func_t func, void *arg)
{
	struct uthread_tcb *newThread = malloc(sizeof(struct uthread_tcb));	// Allocate memory for thread

	if (newThread == NULL)
	{
		return -1;
	}

	newThread->topOfStack = uthread_ctx_alloc_stack();		// Allocate stack for thread
	newThread->status = READY;
	newThread->context = malloc(sizeof(uthread_ctx_t));		// Allocate memory for context

	int ret = uthread_ctx_init(newThread->context, newThread->topOfStack, func, arg);	// Initialize thread

	/* If thread doesn't fail to initialize, enqueue it */
	if (ret != -1)
	{
		queue_enqueue(uthread_q, newThread);
	}

	return ret;	// Return 0
}

/* Start multithreading library and become idle thread */
int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
	/* Set preempt if indicated */
	if (preempt)
	{
		preempt_start(preempt);
	}

	/* Create idle thread */
	struct uthread_tcb *idleThread = malloc(sizeof(struct uthread_tcb));	// Allocate TCB
	idleThread->status = READY;												// Set status to ready
	idleThread->context = malloc(sizeof(uthread_ctx_t));					// Allocate context
	getcontext(idleThread->context);										// Fill context

	current_thread = idleThread;	// Set current thread and idle context global variables
	idle_ctx = idleThread->context;

	uthread_q = queue_create();				// Create queue and enqueue idle
	queue_enqueue(uthread_q, idleThread);

	uthread_create(func, arg);		// Create new thread from parameters

	struct uthread_tcb *curThread;			
	queue_dequeue(uthread_q, (void **)&curThread);

	uthread_yield();				// Yield to next thread

	while (1)						// Loop forever and yield to threads
	{					
		if (queue_length(uthread_q) > 0)
		{
			uthread_yield();
		}
		else
		{
			break;
		}
	}

	if (preempt)		// Stop preemption if enabled
	{
		preempt_stop();
	}

	uthread_ctx_destroy_stack(idleThread->context);		// Deallocate stack of idle thread
	free(idleThread);									// Free idle thread memory
	queue_destroy(uthread_q);							// Deallocate queue

	return 0;
}

/* Block current thread from running*/
void uthread_block(void)
{
	struct uthread_tcb *curThread = uthread_current();	// Get current thread
	curThread->status = BLOCK;							// Set status to blocked
	current_thread = curThread;

	uthread_yield();	// Yield to next thread that is ready
}

/* Unblock specified thread */
void uthread_unblock(struct uthread_tcb *uthread)
{
	uthread->status = READY;	// Set status to ready
}
