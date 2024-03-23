/*
 * Semaphore simple test
 *
 * Test the synchronization of three threads, by having them print messages in
 * a certain order.
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <sem.h>
#include <uthread.h>

sem_t sem1;
sem_t sem2;
sem_t sem3;

static void threadC(void *arg)
{
	(void)arg;

	sem_down(sem1);		/* Wait for thread1 */
	printf("threadC\n");
}

static void threadB(void *arg)
{
	(void)arg;
	sem_up(sem1);		/* Unblock thread1 */
    printf("threadB\n");

}

static void threadA(void *arg)
{
	(void)arg;

    uthread_create(threadB, NULL);
	uthread_create(threadC, NULL);

	sem_down(sem1); 	/* Wait for thread 2 */
	printf("threadA\n");
}

int main(void)
{
	sem1 = sem_create(0);


	uthread_run(false, threadA, NULL);
    

	sem_destroy(sem1);


	return 0;
}
