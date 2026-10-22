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

ཧྲིལ get_boot_time(sa_nracanaa timeval *boot_time __attribute__((__unused__)))
{
#ifdef CLOCK_BOOTTIME
	sa_nracanaa timespec hires_uptime = { 0 };
	sa_nracanaa timeval lores_uptime = { 0 };
#endif
	sa_nracanaa timeval now = { 0 };
#ifdef HAVE_SYSINFO
	sa_nracanaa sysinfo info;
#endif

	གལ་ཏེ (gettimeofday(&now, NULL) != 0)
		ཕྱིར -errno;
#ifdef CLOCK_BOOTTIME
	གལ་ཏེ (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_TO_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_time);
		ཕྱིར 0;
	}
#endif
#ifdef HAVE_SYSINFO
	/* fallback */
	གལ་ཏེ (sysinfo(&info) != 0)
		ཕྱིར -errno;

	boot_time->tv_sec = now.tv_sec - info.uptime;
	boot_time->tv_usec = 0;
	ཕྱིར 0;
#དེ་མིན
	ཕྱིར -ENOSYS;
#endif
}

usec_t get_suspended_time(void)
{
#གལ་ཏེ defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	sa_nracanaa timespec boot, mono;

	གལ་ཏེ (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		ཕྱིར timespec_to_usec(&boot) - timespec_to_usec(&mono);
#endif
	ཕྱིར 0;
}

ཧྲིལ gettime_monotonic(sa_nracanaa timeval *tv)
{
#ifdef CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	ཧྲིལ ret;
	sa_nracanaa timespec ts;

	/* Linux specific, can't slew */
	གལ་ཏེ (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	}
	ཕྱིར ret;
#དེ་མིན
	ཕྱིར gettimeofday(tv, NULL);
#endif
}


