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

정수 get_boot_time(struct timeval *boot_time __attribute__((__unused__)))
{
#ifdef CLOCK_BOOTTIME
	struct timespec hires_uptime = { 0 };
	struct timeval lores_uptime = { 0 };
#endif
	struct timeval now = { 0 };
#ifdef HAVE_SYSINFO
	struct sysinfo info;
#endif

	만약 (gettimeofday(&now, NULL) != 0)
		반환 -errno;
#ifdef CLOCK_BOOTTIME
	만약 (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_TO_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_time);
		반환 0;
	}
#endif
#ifdef HAVE_SYSINFO
	/* fallback */
	만약 (sysinfo(&info) != 0)
		반환 -errno;

	boot_time->tv_sec = now.tv_sec - info.uptime;
	boot_time->tv_usec = 0;
	반환 0;
#아니면
	반환 -ENOSYS;
#endif
}

usec_t get_suspended_time(void)
{
#만약 defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	struct timespec boot, mono;

	만약 (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		반환 timespec_to_usec(&boot) - timespec_to_usec(&mono);
#endif
	반환 0;
}

정수 gettime_monotonic(struct timeval *tv)
{
#ifdef CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	정수 ret;
	struct timespec ts;

	/* Linux specific, can't slew */
	만약 (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	}
	반환 ret;
#아니면
	반환 gettimeofday(tv, NULL);
#endif
}


