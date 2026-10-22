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

	إذا (gettimeofday(&now, عدم) != 0)
		إرجاع -errno;
#ifdef CLOCK_BOOTTIME
	إذا (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_TO_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_time);
		إرجاع 0;
	}
#endif
#ifdef HAVE_SYSINFO
	/* fallback */
	إذا (sysinfo(&info) != 0)
		إرجاع -errno;

	boot_time->tv_sec = now.tv_sec - info.uptime;
	boot_time->tv_usec = 0;
	إرجاع 0;
#وإلا
	إرجاع -ENOSYS;
#endif
}

usec_t get_suspended_time(فراغ)
{
#إذا defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	بنية timespec boot, mono;

	إذا (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		إرجاع timespec_to_usec(&boot) - timespec_to_usec(&mono);
#endif
	إرجاع 0;
}

صحيح gettime_monotonic(بنية timeval *tv)
{
#ifdef CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	صحيح ret;
	بنية timespec ts;

	/* Linux specific, can't slew */
	إذا (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	}
	إرجاع ret;
#وإلا
	إرجاع gettimeofday(tv, عدم);
#endif
}


