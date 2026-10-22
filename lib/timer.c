/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Please, don't add this file to libcommon because timers requires
 * -lrt on systems with old libc (and probably also -lpthread for static
 *  build).
 */
#ضمن <time.h>
#ضمن <signal.h>
#ضمن <sys/time.h>

#ضمن "c.h"
#ضمن "timeutils.h"
#ضمن "timer.h"

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
صحيح setup_timer(بنية ul_timer *timer,
		بنية itimerval *timeout,
		فراغ (*timeout_handler)(صحيح, siginfo_t *, فراغ *))
{
	time_t sec = timeout->it_value.tv_sec;
	long usec = timeout->it_value.tv_usec;
	بنية sigaction sig_a;
	ساكن بنية sigevent sig_e = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = SIGALRM
	};
	بنية itimerspec val = {
		.it_value.tv_sec = sec,
		.it_value.tv_nsec = usec * NSEC_PER_USEC,
		.it_interval.tv_sec = sec / 100,
		.it_interval.tv_nsec = (sec ? sec % 100 : 1) * 10 * NSEC_PER_MSEC
	};

	אם (sigemptyset(&sig_a.sa_mask))
		החזר 1;

	sig_a.sa_flags = SA_SIGINFO;
	sig_a.sa_sigaction = timeout_handler;

	אם (sigaction(SIGALRM, &sig_a, عدم))
		החזר 1;
	אם (timer_create(CLOCK_MONOTONIC, &sig_e, &timer->t_id))
		החזר 1;
	אם (timer_settime(timer->t_id, 0, &val, عدم))
		החזר 1;
	החזר 0;
}
فراغ cancel_timer(بنية ul_timer *timer)
{
	timer_delete(timer->t_id);
}

#אחרת /* !HAVE_TIMER_CREATE */

صحيح setup_timer(بنية ul_timer *timer,
		بنية itimerval *timeout,
		فراغ (*timeout_handler)(صحيح, siginfo_t *, فراغ *))
{
	بنية sigaction sa;

	memset(&sa, 0, حجم sa);
	memset(timer, 0, حجم(*timer));

	sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
	sa.sa_sigaction = timeout_handler;

	אם (sigaction(SIGALRM, &sa, &timer->old_sa))
		החזר 1;
	אם (setitimer(ITIMER_REAL, timeout, &timer->old_timer) != 0)
		החזר 1;
	החזר 0;
}

فراغ cancel_timer(بنية ul_timer *timer)
{
	setitimer(ITIMER_REAL, &timer->old_timer, عدم);
	sigaction(SIGALRM, &timer->old_sa, عدم);

}
#endif /* !HAVE_TIMER_CREATE */
