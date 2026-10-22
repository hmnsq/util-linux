/*
 * Please, don't add this file to libcommon because clock_gettime() requires
 * -lrt on systems with old libc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ضمن <time.h>
#ضمن <signal.h>
#ifdef HAVE_SYSINFO
#ضمن <sys/sysinfo.h>
#endif
#ضمن <sys/time.h>

#ضمن "c.h"
#ضمن "monotonic.h"

صحيح get_boot_time(بنية timeval *boot_time __attribute__((__unused__)))
{
#ifdef CLOCK_BOOTTIME
	بنية timespec hires_uptime = { 0 };
	بنية timeval lores_uptime = { 0 };
#endif
	بنية timeval now = { 0 };
#ifdef HAVE_SYSINFO
	بنية sysinfo info;
#endif

	ܐܢ (gettimeofday(&now, عدم) != 0)
		ܦܢܐ -errno;
#ifdef CLOCK_BOOTTIME
	ܐܢ (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_TO_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_time);
		ܦܢܐ 0;
	}
#endif
#ifdef HAVE_SYSINFO
	/* fallback */
	ܐܢ (sysinfo(&info) != 0)
		ܦܢܐ -errno;

	boot_time->tv_sec = now.tv_sec - info.uptime;
	boot_time->tv_usec = 0;
	ܦܢܐ 0;
#ܐܚܪܢܐ
	ܦܢܐ -ENOSYS;
#endif
}

usec_t get_suspended_time(فراغ)
{
#ܐܢ defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	بنية timespec boot, mono;

	ܐܢ (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		ܦܢܐ timespec_to_usec(&boot) - timespec_to_usec(&mono);
#endif
	ܦܢܐ 0;
}

صحيح gettime_monotonic(بنية timeval *tv)
{
#ifdef CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	صحيح ret;
	بنية timespec ts;

	/* Linux specific, can't slew */
	ܐܢ (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	}
	ܦܢܐ ret;
#ܐܚܪܢܐ
	ܦܢܐ gettimeofday(tv, عدم);
#endif
}


