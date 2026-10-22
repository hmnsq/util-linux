/*
 * Please, don't add this file to libcommon because clock_gettime() requires
 * -lrt on systems with old libc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#include <time.h>
#include <signal.h>
#ifdef HAVE_SYSINFO
#include <sys/sysinfo.h>
#endif
#include <sys/time.h>

#include "c.h"
#include "monotonic.h"

số_nguyên get_boot_time(struct timeval *boot_time __attribute__((__unused__)))
{
#ifdef CLOCK_BOOTTIME
	struct timespec hires_uptime = { 0 };
	struct timeval lores_uptime = { 0 };
#endif
	struct timeval now = { 0 };
#ifdef HAVE_SYSINFO
	struct sysinfo info;
#endif

	nếu (gettimeofday(&now, NULL) != 0)
		trả_về -errno;
#ifdef CLOCK_BOOTTIME
	nếu (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_TO_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_time);
		trả_về 0;
	}
#endif
#ifdef HAVE_SYSINFO
	/* fallback */
	nếu (sysinfo(&info) != 0)
		trả_về -errno;

	boot_time->tv_sec = now.tv_sec - info.uptime;
	boot_time->tv_usec = 0;
	trả_về 0;
#không_thì
	trả_về -ENOSYS;
#endif
}

usec_t get_suspended_time(void)
{
#nếu defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	struct timespec boot, mono;

	nếu (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		trả_về timespec_to_usec(&boot) - timespec_to_usec(&mono);
#endif
	trả_về 0;
}

số_nguyên gettime_monotonic(struct timeval *tv)
{
#ifdef CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	số_nguyên ret;
	struct timespec ts;

	/* Linux specific, can't slew */
	nếu (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	}
	trả_về ret;
#không_thì
	trả_về gettimeofday(tv, NULL);
#endif
}


