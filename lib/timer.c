/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Please, don't add this file to libcommon because timers requires
 * -lrt on systems with old libc (and probably also -lpthread for static
 *  build).
 */
#समावेश <time.h>
#समावेश <signal.h>
#समावेश <sys/time.h>

#समावेश "c.h"
#समावेश "timeutils.h"
#समावेश "timer.h"

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
#अगर_घोषित HAVE_TIMER_CREATE
पूर्णांक setup_timer(काष्ठा ul_timer *timer,
		काष्ठा itimerval *timeout,
		व्योम (*timeout_handler)(पूर्णांक, siginfo_t *, व्योम *))
{
	समय_प्रकार sec = timeout->it_value.tv_sec;
	दीर्घ usec = timeout->it_value.tv_usec;
	काष्ठा sigaction sig_a;
	अटल काष्ठा sigevent sig_e = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = SIGALRM
	};
	काष्ठा itimerspec val = {
		.it_value.tv_sec = sec,
		.it_value.tv_nsec = usec * NSEC_PER_USEC,
		.it_interval.tv_sec = sec / 100,
		.it_interval.tv_nsec = (sec ? sec % 100 : 1) * 10 * NSEC_PER_MSEC
	};

	अगर (sigemptyset(&sig_a.sa_mask))
		वापस 1;

	sig_a.sa_flags = SA_SIGINFO;
	sig_a.sa_sigaction = timeout_handler;

	अगर (sigaction(SIGALRM, &sig_a, शून्य))
		वापस 1;
	अगर (timer_create(CLOCK_MONOTONIC, &sig_e, &timer->t_id))
		वापस 1;
	अगर (timer_settime(timer->t_id, 0, &val, शून्य))
		वापस 1;
	वापस 0;
}
व्योम cancel_timer(काष्ठा ul_timer *timer)
{
	timer_delete(timer->t_id);
}

#अन्यथा /* !HAVE_TIMER_CREATE */

पूर्णांक setup_timer(काष्ठा ul_timer *timer,
		काष्ठा itimerval *timeout,
		व्योम (*timeout_handler)(पूर्णांक, siginfo_t *, व्योम *))
{
	काष्ठा sigaction sa;

	स_रखो(&sa, 0, माप sa);
	स_रखो(timer, 0, माप(*timer));

	sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
	sa.sa_sigaction = timeout_handler;

	अगर (sigaction(SIGALRM, &sa, &timer->old_sa))
		वापस 1;
	अगर (setitimer(ITIMER_REAL, timeout, &timer->old_timer) != 0)
		वापस 1;
	वापस 0;
}

व्योम cancel_timer(काष्ठा ul_timer *timer)
{
	setitimer(ITIMER_REAL, &timer->old_timer, शून्य);
	sigaction(SIGALRM, &timer->old_sa, शून्य);

}
#पूर्ण_अगर /* !HAVE_TIMER_CREATE */
