# uthread : User-level thread library

## Summary:

This project involves implementing a basic user-level thread library in C, 
providing an interface for applications to create and manage independent threads 
concurrently. The library supports thread creation, scheduling, preemption, and 
synchronization using semaphores.

## Implementation:

The implementation can be broken down into 4 main steps:
1. Queue API
2. uthread API
3. Semaphore API
4. Preemption


### Queue API:
The Queue API consists of a few key operations which are used to hold the threads
created by the multithreading API.

Create: Initializes a new empty queue. This is typically done at the start of 
the program or when a new set of threads needs to be managed.

Enqueue: Adds a new item (in this context, a thread control block or TCB) to the
end of the queue. This operation is used to add a new thread to the ready queue 
when it is created or when it yields control and needs to be rescheduled.

Dequeue: Removes and returns the item at the front of the queue. This is used by 
the scheduler to select the next thread to run. The memory allocated for the node
is freed here.

Destroy: Frees any resources associated with the queue when it is no longer needed.

### uThread API

The primary data structure is the struct uthread_tcb, which represents a thread 
control block (TCB). Each TCB contains a thread's context, a pointer to the top 
of its stack, and its status (RUNNING, READY, or BLOCK). These are used to save
the context of the currently running thread so that it can be returned to later.

The library uses several global variables, including uthread_q, a queue that holds
threads in the READY state; idle_ctx, the context of the idle thread; and
current_thread, a pointer to the TCB of the currently running thread.

`uthread_current()` returns the TCB of the currently running thread, which is 
continually updated through the current thread global variable. `uthread_yield()`
is used to grab the next READY thread in the queue to context switch to. It 
enqueues the current thread back into the ready queue and context switches to 
the next READY thread. Preemption is disabled during this process as the queue
is a shared resource so an interrupt could cause unexpected behavior.
`uthread_exit()` terminates the current thread, frees its resources, and switches
to the next READY thread. It functions similarly to yield in that it searches
for the next READY thread, however it must also deallocate the stack of the
thread. `uthread_create()` creates a new thread with a given function and
argument, initializes its context and stack, and enqueues it in the ready queue.
`uthread_run()` initializes the thread library, creates the idle thread, and 
starts the thread scheduler. It continually yields to ready threads until the 
queue is empty, by sitting in an infinite loop. Once the queue is empty, meaning
all threads have completed, `uthread_run()` breaks out and destroys the rest of
the allocated memory(queue, idle_thread & idle_thread stack). `uthread_block` and `uthread_unblock` are both called from 
the Semaphore API, to set the status of a thread and prevent blocked threads
from running. `uthread_block()` blocks the current thread by setting its status 
to BLOCK and yielding to the next ready thread. `uthread_unblock()` unblocks a 
specified thread by setting its status to READY.

The code also includes functionality for preemption, with calls to `preempt_disable()` 
and `preempt_enable()` to manage preemption around critical sections. The 
`uthread_run()` function also handles starting and stopping preemption based on 
its input parameter.

### Semaphore API

The data structure of the semaphore API is the struct semaphore, which represents 
a semaphore with a resource count (sem_count) and a queue (sem_q) for threads 
waiting to acquire the semaphore.

`sem_create(size_t count)`: Creates and initializes a semaphore with a specified
resource count. It allocates memory for the semaphore structure, initializes the 
resource count, and creates a queue for threads that will wait for the semaphore.
If the resource count given is a negative number, it is initialized to 0 since
having negative resources is not possible. 

`sem_destroy(sem_t sem)`: Simply destroys the semaphore, freeing the memory 
allocated for the semaphore structure and the associated queue.

`sem_down()` and `sem_up()` handle the bulk of resource allocation, with down
taking resources and up freeing them. 

`sem_down(sem_t sem)`: Decrements the semaphore's resource count (if available) 
and blocks the calling thread if no resources are available. The blocking is
handled by a call to `uthread_block()` which sets the thread's status. The thread 
is then enqueued in the semaphore's waiting queue, to be popped when the resource
becomes available. Preemption is disabled during this process as the semaphore's
queue is a shared resource, so an interrupt could cause unexpected behavior.

`sem_up(sem_t sem)`: Increments the semaphore's resource count and unblocks a 
thread waiting in the semaphore's queue, if any. The status of the thread is set
so that it is available to run the next time it is popped from the queue.

The code also includes calls to preempt_disable() and preempt_enable() to manage 
preemption around critical sections. This ensures that semaphore operations are 
performed atomically, preventing inconsistencies in the semaphore state.


### Preemption:

The code implements preemption for a user-level thread library using POSIX 
signals and timers. The preemption frequency is set to 100Hz, meaning that the 
system will attempt to switch threads 100 times per second. To achieve this, the
code uses a signal handler (signal_handler) that is triggered by the SIGALRM 
signal. When this signal is received, the handler calls uthread_yield() to yield 
execution to the next thread in the ready queue.

Preemption is controlled using two main functions: preempt_disable() and 
preempt_enable(). These functions manage a signal set (ss) that determines 
whether the SIGALRM signal should be blocked or allowed. When preemption is 
disabled, the signal is removed from the set, preventing the signal handler from 
being invoked. When preemption is enabled, the signal is added back to the set, 
allowing the handler to be triggered by timer interrupts.

The preempt_start() function is responsible for initializing and starting the 
preemption timer. It saves the current signal handler and timer settings, sets up 
the new signal handler for SIGALRM, and configures the timer to generate 
interrupts at the specified frequency. If preemption is not enabled, the 
function simply returns without making any changes.

Finally, the preempt_stop() function stops preemption by restoring the previous 
signal handler and timer settings. This effectively disables the preemption 
timer and returns the system to its original state.

## Testing

uthread_yield.c and uthread_hello.c test basic thread operations.

test_preempt.c verifies the preemption functionality, ensuring that threads are 
preempted and switched at regular intervals to ensure fair scheduling.

Files like sem_simple.c, sem_prime.c, sem_count.c, and sem_buffer.c test various 
aspects of semaphore functionality. These tests cover basic semaphore operations, 
counting semaphores, and more complex scenarios like a prime number sieve and a 
producer-consumer problem using semaphores.

queue_tester_example.c tests the queue functionality used to manage ready 
threads, and corner_case.c contains a test for specific edge case that the 
thread library needs to handle correctly.

## Work of:

This program is the work of Ashwin Chembu and Saunak Singh