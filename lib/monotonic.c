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

puur_naa_nka get_boot_time(sa_nracanaa timeval *boot_time __attribute__((__unused__)))
{
#ifdef CLOCK_BOOTTIME
	sa_nracanaa timespec hires_uptime = { 0 };
	sa_nracanaa timeval lores_uptime = { 0 };
#endif
	sa_nracanaa timeval now = { 0 };
#ifdef HAVE_SYSINFO
	sa_nracanaa sysinfo info;
#endif

	ᬬᬾᬦ᭄ (gettimeofday(&now, NULL) != 0)
		ᬯᬮᬶ -errno;
#ifdef CLOCK_BOOTTIME
	ᬬᬾᬦ᭄ (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_TO_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_time);
		ᬯᬮᬶ 0;
	}
#endif
#ifdef HAVE_SYSINFO
	/* fallback */
	ᬬᬾᬦ᭄ (sysinfo(&info) != 0)
		ᬯᬮᬶ -errno;

	boot_time->tv_sec = now.tv_sec - info.uptime;
	boot_time->tv_usec = 0;
	ᬯᬮᬶ 0;
#ᬮᬶᬬ
	ᬯᬮᬶ -ENOSYS;
#endif
}

usec_t get_suspended_time(void)
{
#ᬬᬾᬦ᭄ defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	sa_nracanaa timespec boot, mono;

	ᬬᬾᬦ᭄ (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		ᬯᬮᬶ timespec_to_usec(&boot) - timespec_to_usec(&mono);
#endif
	ᬯᬮᬶ 0;
}

puur_naa_nka gettime_monotonic(sa_nracanaa timeval *tv)
{
#ifdef CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	puur_naa_nka ret;
	sa_nracanaa timespec ts;

	/* Linux specific, can't slew */
	ᬬᬾᬦ᭄ (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	}
	ᬯᬮᬶ ret;
#ᬮᬶᬬ
	ᬯᬮᬶ gettimeofday(tv, NULL);
#endif
}


