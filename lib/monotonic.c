/*
 * Please, don't add this file to libcommon because clock_gettime() requires
 * -lrt on systems with old libc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <signal.h>
#ifdef HAVE_SYSINFO
#അന്തര്ഭാവ <sys/sysinfo.h>
#endif
#അന്തര്ഭാവ <sys/time.h>

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "monotonic.h"

പൂർണ്ണസംഖ്യ get_boot_time(ഘടന timeval *boot_time __attribute__((__unused__)))
{
#ifdef CLOCK_BOOTTIME
	ഘടന timespec hires_uptime = { 0 };
	ഘടന timeval lores_uptime = { 0 };
#endif
	ഘടന timeval now = { 0 };
#ifdef HAVE_SYSINFO
	ഘടന sysinfo info;
#endif

	എങ്കിൽ (gettimeofday(&now, ശൂന്യ) != 0)
		തിരികെ -errno;
#ifdef CLOCK_BOOTTIME
	എങ്കിൽ (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_TO_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_time);
		തിരികെ 0;
	}
#endif
#ifdef HAVE_SYSINFO
	/* fallback */
	എങ്കിൽ (sysinfo(&info) != 0)
		തിരികെ -errno;

	boot_time->tv_sec = now.tv_sec - info.uptime;
	boot_time->tv_usec = 0;
	തിരികെ 0;
#അല്ലെങ്കിൽ
	തിരികെ -ENOSYS;
#endif
}

usec_t get_suspended_time(ശൂന്യം)
{
#എങ്കിൽ defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	ഘടന timespec boot, mono;

	എങ്കിൽ (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		തിരികെ timespec_to_usec(&boot) - timespec_to_usec(&mono);
#endif
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ gettime_monotonic(ഘടന timeval *tv)
{
#ifdef CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	പൂർണ്ണസംഖ്യ ret;
	ഘടന timespec ts;

	/* Linux specific, can't slew */
	എങ്കിൽ (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	}
	തിരികെ ret;
#അല്ലെങ്കിൽ
	തിരികെ gettimeofday(tv, ശൂന്യ);
#endif
}


