/*
 * Please, don't add this file to libcommon because clock_gettime() requires
 * -lrt on systems with old libc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <signal.h>
#ifdef HAVE_SYSINFO
#અન્તર્ભાવ <sys/sysinfo.h>
#endif
#અન્તર્ભાવ <sys/time.h>

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "monotonic.h"

પૂર્ણાંક get_boot_time(સંરચના timeval *boot_time __attribute__((__unused__)))
{
#ifdef CLOCK_BOOTTIME
	સંરચના timespec hires_uptime = { 0 };
	સંરચના timeval lores_uptime = { 0 };
#endif
	સંરચના timeval now = { 0 };
#ifdef HAVE_SYSINFO
	સંરચના sysinfo info;
#endif

	જો (gettimeofday(&now, ખાલી) != 0)
		પાછા -errno;
#ifdef CLOCK_BOOTTIME
	જો (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_TO_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_time);
		પાછા 0;
	}
#endif
#ifdef HAVE_SYSINFO
	/* fallback */
	જો (sysinfo(&info) != 0)
		પાછા -errno;

	boot_time->tv_sec = now.tv_sec - info.uptime;
	boot_time->tv_usec = 0;
	પાછા 0;
#નહીંતર
	પાછા -ENOSYS;
#endif
}

usec_t get_suspended_time(શૂન્ય)
{
#જો defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	સંરચના timespec boot, mono;

	જો (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		પાછા timespec_to_usec(&boot) - timespec_to_usec(&mono);
#endif
	પાછા 0;
}

પૂર્ણાંક gettime_monotonic(સંરચના timeval *tv)
{
#ifdef CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	પૂર્ણાંક ret;
	સંરચના timespec ts;

	/* Linux specific, can't slew */
	જો (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	}
	પાછા ret;
#નહીંતર
	પાછા gettimeofday(tv, ખાલી);
#endif
}


