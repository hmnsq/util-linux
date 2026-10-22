/*
 * Please, don't add this file to libcommon because clock_gettime() requires
 * -lrt on systems with old libc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ضمن <time.h>
#ضمن <signal.h>
#که_تعریف_شوی HAVE_SYSINFO
#ضمن <sys/sysinfo.h>
#د_که_پای
#ضمن <sys/time.h>

#ضمن "c.h"
#ضمن "monotonic.h"

صحيح get_boot_وخت(بنية timeval *boot_وخت __attribute__((__unused__)))
{
#که_تعریف_شوی CLOCK_BOOTTIME
	بنية timespec hires_uptime = { 0 };
	بنية timeval lores_uptime = { 0 };
#د_که_پای
	بنية timeval now = { 0 };
#که_تعریف_شوی HAVE_SYSINFO
	بنية sysinfo info;
#د_که_پای

	إذا (gettimeofday(&now, عدم) != 0)
		أرجع -errno;
#که_تعریف_شوی CLOCK_BOOTTIME
	إذا (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_منزل_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_وخت);
		أرجع 0;
	}
#د_که_پای
#که_تعریف_شوی HAVE_SYSINFO
	/* fallback */
	إذا (sysinfo(&info) != 0)
		أرجع -errno;

	boot_وخت->tv_sec = now.tv_sec - info.uptime;
	boot_وخت->tv_usec = 0;
	أرجع 0;
#وإلا
	أرجع -ENOSYS;
#د_که_پای
}

usec_t get_suspended_وخت(فراغ)
{
#إذا defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	بنية timespec boot, mono;

	إذا (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		أرجع timespec_منزل_usec(&boot) - timespec_منزل_usec(&mono);
#د_که_پای
	أرجع 0;
}

صحيح gettime_monotonic(بنية timeval *tv)
{
#که_تعریف_شوی CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	صحيح ret;
	بنية timespec ts;

	/* Linux specific, can't slew */
	إذا (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_فی_USEC;
	}
	أرجع ret;
#وإلا
	أرجع gettimeofday(tv, عدم);
#د_که_پای
}


