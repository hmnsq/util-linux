/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Please, don't add this file to libcommon because timers requires
 * -lrt on systems with old libc (and probably also -lpthread for static
 *  build).
 */
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <sys/time.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "timeutils.h"
#அந்தர்பாவ "timer.h"

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
முழு setup_timer(கட்டமைப்பு ul_timer *timer,
		கட்டமைப்பு itimerval *timeout,
		வெறுமை (*timeout_handler)(முழு, siginfo_t *, வெறுமை *))
{
	time_t sec = timeout->it_value.tv_sec;
	long usec = timeout->it_value.tv_usec;
	கட்டமைப்பு sigaction sig_a;
	நிலையான கட்டமைப்பு sigevent sig_e = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = SIGALRM
	};
	கட்டமைப்பு itimerspec val = {
		.it_value.tv_sec = sec,
		.it_value.tv_nsec = usec * NSEC_PER_USEC,
		.it_interval.tv_sec = sec / 100,
		.it_interval.tv_nsec = (sec ? sec % 100 : 1) * 10 * NSEC_PER_MSEC
	};

	எனில் (sigemptyset(&sig_a.sa_mask))
		திரும்பு 1;

	sig_a.sa_flags = SA_SIGINFO;
	sig_a.sa_sigaction = timeout_handler;

	எனில் (sigaction(SIGALRM, &sig_a, வெற்று))
		திரும்பு 1;
	எனில் (timer_create(CLOCK_MONOTONIC, &sig_e, &timer->t_id))
		திரும்பு 1;
	எனில் (timer_settime(timer->t_id, 0, &val, வெற்று))
		திரும்பு 1;
	திரும்பு 0;
}
வெறுமை cancel_timer(கட்டமைப்பு ul_timer *timer)
{
	timer_delete(timer->t_id);
}

#இல்லையெனில் /* !HAVE_TIMER_CREATE */

முழு setup_timer(கட்டமைப்பு ul_timer *timer,
		கட்டமைப்பு itimerval *timeout,
		வெறுமை (*timeout_handler)(முழு, siginfo_t *, வெறுமை *))
{
	கட்டமைப்பு sigaction sa;

	memset(&sa, 0, அளவு sa);
	memset(timer, 0, அளவு(*timer));

	sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
	sa.sa_sigaction = timeout_handler;

	எனில் (sigaction(SIGALRM, &sa, &timer->old_sa))
		திரும்பு 1;
	எனில் (setitimer(ITIMER_REAL, timeout, &timer->old_timer) != 0)
		திரும்பு 1;
	திரும்பு 0;
}

வெறுமை cancel_timer(கட்டமைப்பு ul_timer *timer)
{
	setitimer(ITIMER_REAL, &timer->old_timer, வெற்று);
	sigaction(SIGALRM, &timer->old_sa, வெற்று);

}
#endif /* !HAVE_TIMER_CREATE */
