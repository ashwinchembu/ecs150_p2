#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*======================================================================*/
/* Global variables and macros */
/*======================================================================*/

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

/* Save previous settings to revert to */
struct sigaction prev_sa;
struct itimerval prev_timer;
sigset_t ss;


/*======================================================================*/
/* Implementation */
/*======================================================================*/

/* Signal handler to yield to next thread at alarm interrupt */
void signal_handler()
{
	uthread_yield();
}

/* Disable preemption */
void preempt_disable(void)
{
	sigdelset(&ss, SIGVTALRM);	// Remove alarm signal from set
}

/* Enable preemption */
void preempt_enable(void)
{
	sigemptyset(&ss);					// Initialize set
	sigaddset(&ss, SIGVTALRM);			// Add alarm signal to set
	sigprocmask(SIG_BLOCK, &ss, NULL);	// Change signal mask
}

/* Start preemption */
void preempt_start(bool preempt)
{
	if (!preempt) {
		return;
	}

	sigaction(SIGALRM, NULL, &prev_sa);		// Store current signal handler

	struct sigaction sa;					// Create sig action and fill parameters

	sa.sa_handler = signal_handler;			// Set signal handler as yield
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGALRM, &sa, NULL);			// Apply signal handler

	getitimer(ITIMER_REAL, &prev_timer);	// Store current timer

	struct itimerval timer;					// Create timer and apply interval (100 times per second)

	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 1000000 / HZ;

	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 1000000 / HZ;

	if (setitimer(ITIMER_REAL, &timer, NULL) == -1)	// Set new timer
	{
		perror("setitimer failed");
		exit(EXIT_FAILURE);
	}
}

/* Stop preemption */
void preempt_stop(void)
{
	sigaction(SIGALRM, &prev_sa, NULL);			// Restore previous settings
	setitimer(ITIMER_REAL, &prev_timer, NULL);
}
