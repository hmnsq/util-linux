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

ကိန်းပြည့် get_boot_time(sa_nracanaa timeval *boot_time __attribute__((__unused__)))
{
#ifdef CLOCK_BOOTTIME
	sa_nracanaa timespec hires_uptime = { 0 };
	sa_nracanaa timeval lores_uptime = { 0 };
#endif
	sa_nracanaa timeval now = { 0 };
#ifdef HAVE_SYSINFO
	sa_nracanaa sysinfo info;
#endif

	အကယ်၍ (gettimeofday(&now, NULL) != 0)
		ပြန်ပေး -errno;
#ifdef CLOCK_BOOTTIME
	အကယ်၍ (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_TO_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_time);
		ပြန်ပေး 0;
	}
#endif
#ifdef HAVE_SYSINFO
	/* fallback */
	အကယ်၍ (sysinfo(&info) != 0)
		ပြန်ပေး -errno;

	boot_time->tv_sec = now.tv_sec - info.uptime;
	boot_time->tv_usec = 0;
	ပြန်ပေး 0;
#မဟုတ်ပါက
	ပြန်ပေး -ENOSYS;
#endif
}

usec_t get_suspended_time(void)
{
#အကယ်၍ defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	sa_nracanaa timespec boot, mono;

	အကယ်၍ (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		ပြန်ပေး timespec_to_usec(&boot) - timespec_to_usec(&mono);
#endif
	ပြန်ပေး 0;
}

ကိန်းပြည့် gettime_monotonic(sa_nracanaa timeval *tv)
{
#ifdef CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	ကိန်းပြည့် ret;
	sa_nracanaa timespec ts;

	/* Linux specific, can't slew */
	အကယ်၍ (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	}
	ပြန်ပေး ret;
#မဟုတ်ပါက
	ပြန်ပေး gettimeofday(tv, NULL);
#endif
}


