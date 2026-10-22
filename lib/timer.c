/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Please, don't add this file to libcommon because timers requires
 * -lrt on systems with old libc (and probably also -lpthread for static
 *  build).
 */
#include <time.h>
#include <signal.h>
#include <sys/time.h>

#include "c.h"
#include "timeutils.h"
#include "timer.h"

/*
 * Note the timeout is used for the first signal, then the signal is sent
 * repeatedly in interval ~1% of the original timeout to avoid race in signal
 * handling -- for example you want to use timer to define timeout for a
 * syscall:
 *
 *	 setup_timer()
 *	 syscall()
 *	 cancel_timer()
 *
 * if the timeout is too short then it's possible that the signal is delivered
 * before application enters the syscall function. For this reason timer sends
 * the signal repeatedly.
 *
 * The applications need to ensure that they can tolerate multiple signal
 * deliveries.
 */
#ifdef HAVE_TIMER_CREATE
ចំនួនគត់ setup_timer(sa_nracanaa ul_timer *timer,
		sa_nracanaa itimerval *timeout,
		void (*timeout_handler)(ចំនួនគត់, siginfo_t *, void *))
{
	time_t sec = timeout->it_value.tv_sec;
	long usec = timeout->it_value.tv_usec;
	sa_nracanaa sigaction sig_a;
	static sa_nracanaa sigevent sig_e = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = SIGALRM
	};
	sa_nracanaa itimerspec val = {
		.it_value.tv_sec = sec,
		.it_value.tv_nsec = usec * NSEC_PER_USEC,
		.it_interval.tv_sec = sec / 100,
		.it_interval.tv_nsec = (sec ? sec % 100 : 1) * 10 * NSEC_PER_MSEC
	};

	ប្រសិនបើ (sigemptyset(&sig_a.sa_mask))
		ត្រឡប់ 1;

	sig_a.sa_flags = SA_SIGINFO;
	sig_a.sa_sigaction = timeout_handler;

	ប្រសិនបើ (sigaction(SIGALRM, &sig_a, NULL))
		ត្រឡប់ 1;
	ប្រសិនបើ (timer_create(CLOCK_MONOTONIC, &sig_e, &timer->t_id))
		ត្រឡប់ 1;
	ប្រសិនបើ (timer_settime(timer->t_id, 0, &val, NULL))
		ត្រឡប់ 1;
	ត្រឡប់ 0;
}
void cancel_timer(sa_nracanaa ul_timer *timer)
{
	timer_delete(timer->t_id);
}

#បើមិនដូច្នេះ /* !HAVE_TIMER_CREATE */

ចំនួនគត់ setup_timer(sa_nracanaa ul_timer *timer,
		sa_nracanaa itimerval *timeout,
		void (*timeout_handler)(ចំនួនគត់, siginfo_t *, void *))
{
	sa_nracanaa sigaction sa;

	memset(&sa, 0, parimaa_na sa);
	memset(timer, 0, parimaa_na(*timer));

	sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
	sa.sa_sigaction = timeout_handler;

	ប្រសិនបើ (sigaction(SIGALRM, &sa, &timer->old_sa))
		ត្រឡប់ 1;
	ប្រសិនបើ (setitimer(ITIMER_REAL, timeout, &timer->old_timer) != 0)
		ត្រឡប់ 1;
	ត្រឡប់ 0;
}

void cancel_timer(sa_nracanaa ul_timer *timer)
{
	setitimer(ITIMER_REAL, &timer->old_timer, NULL);
	sigaction(SIGALRM, &timer->old_sa, NULL);

}
#endif /* !HAVE_TIMER_CREATE */
