/*
 * Please, don't add this file to libcommon because clock_gettime() requires
 * -lrt on systems with old libc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <signal.h>
#ifdef HAVE_SYSINFO
#অন্তর্ভাব <sys/sysinfo.h>
#endif
#অন্তর্ভাব <sys/time.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "monotonic.h"

পূর্ণাঙ্ক get_boot_time(সংরচনা timeval *boot_time __attribute__((__unused__)))
{
#ifdef CLOCK_BOOTTIME
	সংরচনা timespec hires_uptime = { 0 };
	সংরচনা timeval lores_uptime = { 0 };
#endif
	সংরচনা timeval now = { 0 };
#ifdef HAVE_SYSINFO
	সংরচনা sysinfo info;
#endif

	যদি (gettimeofday(&now, রিক্ত) != 0)
		প্রত্যাগম -errno;
#ifdef CLOCK_BOOTTIME
	যদি (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_TO_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_time);
		প্রত্যাগম 0;
	}
#endif
#ifdef HAVE_SYSINFO
	/* fallback */
	যদি (sysinfo(&info) != 0)
		প্রত্যাগম -errno;

	boot_time->tv_sec = now.tv_sec - info.uptime;
	boot_time->tv_usec = 0;
	প্রত্যাগম 0;
#অন্যথা
	প্রত্যাগম -ENOSYS;
#endif
}

usec_t get_suspended_time(শূন্য)
{
#যদি defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	সংরচনা timespec boot, mono;

	যদি (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		প্রত্যাগম timespec_to_usec(&boot) - timespec_to_usec(&mono);
#endif
	প্রত্যাগম 0;
}

পূর্ণাঙ্ক gettime_monotonic(সংরচনা timeval *tv)
{
#ifdef CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	পূর্ণাঙ্ক ret;
	সংরচনা timespec ts;

	/* Linux specific, can't slew */
	যদি (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	}
	প্রত্যাগম ret;
#অন্যথা
	প্রত্যাগম gettimeofday(tv, রিক্ত);
#endif
}


