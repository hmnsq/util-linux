/*
 * Please, don't add this file to libcommon because clock_gettime() requires
 * -lrt on systems with old libc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#شامل <time.h>
#شامل <signal.h>
#ifdef HAVE_SYSINFO
#شامل <sys/sysinfo.h>
#endif
#شامل <sys/time.h>

#شامل "c.h"
#شامل "monotonic.h"

صحیح get_boot_time(ساختار timeval *boot_time __attribute__((__unused__)))
{
#ifdef CLOCK_BOOTTIME
	ساختار timespec hires_uptime = { 0 };
	ساختار timeval lores_uptime = { 0 };
#endif
	ساختار timeval now = { 0 };
#ifdef HAVE_SYSINFO
	ساختار sysinfo info;
#endif

	اگر (gettimeofday(&now, پوچ) != 0)
		بازگشت -errno;
#ifdef CLOCK_BOOTTIME
	اگر (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_TO_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_time);
		بازگشت 0;
	}
#endif
#ifdef HAVE_SYSINFO
	/* fallback */
	اگر (sysinfo(&info) != 0)
		بازگشت -errno;

	boot_time->tv_sec = now.tv_sec - info.uptime;
	boot_time->tv_usec = 0;
	بازگشت 0;
#وگرنه
	بازگشت -ENOSYS;
#endif
}

usec_t get_suspended_time(تهی)
{
#اگر defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	ساختار timespec boot, mono;

	اگر (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		بازگشت timespec_to_usec(&boot) - timespec_to_usec(&mono);
#endif
	بازگشت 0;
}

صحیح gettime_monotonic(ساختار timeval *tv)
{
#ifdef CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	صحیح ret;
	ساختار timespec ts;

	/* Linux specific, can't slew */
	اگر (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	}
	بازگشت ret;
#وگرنه
	بازگشت gettimeofday(tv, پوچ);
#endif
}


