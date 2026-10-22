/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Please, don't add this file to libcommon because timers requires
 * -lrt on systems with old libc (and probably also -lpthread for static
 *  build).
 */
#అన్తర్భావ <time.h>
#అన్తర్భావ <signal.h>
#అన్తర్భావ <sys/time.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "timeutils.h"
#అన్తర్భావ "timer.h"

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
#నిర్వచితమైతే HAVE_TIMER_CREATE
పూర్ణాంకం అమరిక_timer(నిర్మాణం ul_timer *timer,
		నిర్మాణం itimerval *timeout,
		శూన్య (*timeout_handler)(పూర్ణాంకం, siginfo_t *, శూన్య *))
{
	time_t sec = timeout->it_value.tv_sec;
	దీర్ఘ usec = timeout->it_value.tv_usec;
	నిర్మాణం sigaction sig_a;
	స్థిర నిర్మాణం sigevent sig_e = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = SIGALRM
	};
	నిర్మాణం itimerspec val = {
		.it_value.tv_sec = sec,
		.it_value.tv_nsec = usec * NSEC_చొప్పున_USEC,
		.it_interval.tv_sec = sec / 100,
		.it_interval.tv_nsec = (sec ? sec % 100 : 1) * 10 * NSEC_చొప్పున_MSEC
	};

	ఒకవేళ (sigemptyset(&sig_a.sa_mask))
		తిరిగి 1;

	sig_a.sa_flags = SA_SIGINFO;
	sig_a.sa_sigaction = timeout_handler;

	ఒకవేళ (sigaction(SIGALRM, &sig_a, ఖాళీ))
		తిరిగి 1;
	ఒకవేళ (timer_create(CLOCK_MONOTONIC, &sig_e, &timer->t_id))
		తిరిగి 1;
	ఒకవేళ (timer_settime(timer->t_id, 0, &val, ఖాళీ))
		తిరిగి 1;
	తిరిగి 0;
}
శూన్య cancel_timer(నిర్మాణం ul_timer *timer)
{
	timer_delete(timer->t_id);
}

#లేకపోతే /* !HAVE_TIMER_CREATE */

పూర్ణాంకం అమరిక_timer(నిర్మాణం ul_timer *timer,
		నిర్మాణం itimerval *timeout,
		శూన్య (*timeout_handler)(పూర్ణాంకం, siginfo_t *, శూన్య *))
{
	నిర్మాణం sigaction sa;

	memset(&sa, 0, పరిమాణం sa);
	memset(timer, 0, పరిమాణం(*timer));

	sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
	sa.sa_sigaction = timeout_handler;

	ఒకవేళ (sigaction(SIGALRM, &sa, &timer->old_sa))
		తిరిగి 1;
	ఒకవేళ (setitimer(ITIMER_REAL, timeout, &timer->old_timer) != 0)
		తిరిగి 1;
	తిరిగి 0;
}

శూన్య cancel_timer(నిర్మాణం ul_timer *timer)
{
	setitimer(ITIMER_REAL, &timer->old_timer, ఖాళీ);
	sigaction(SIGALRM, &timer->old_sa, ఖాళీ);

}
#అయితే_ముగింపు /* !HAVE_TIMER_CREATE */
