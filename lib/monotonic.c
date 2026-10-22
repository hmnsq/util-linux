/*
 * Please, don't add this file to libcommon because clock_gettime() requires
 * -lrt on systems with old libc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <signal.h>
#ifdef HAVE_SYSINFO
#அந்தர்பாவ <sys/sysinfo.h>
#endif
#அந்தர்பாவ <sys/time.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "monotonic.h"

முழு get_boot_time(கட்டமைப்பு timeval *boot_time __attribute__((__unused__)))
{
#ifdef CLOCK_BOOTTIME
	கட்டமைப்பு timespec hires_uptime = { 0 };
	கட்டமைப்பு timeval lores_uptime = { 0 };
#endif
	கட்டமைப்பு timeval now = { 0 };
#ifdef HAVE_SYSINFO
	கட்டமைப்பு sysinfo info;
#endif

	எனில் (gettimeofday(&now, வெற்று) != 0)
		திரும்பு -errno;
#ifdef CLOCK_BOOTTIME
	எனில் (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_TO_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_time);
		திரும்பு 0;
	}
#endif
#ifdef HAVE_SYSINFO
	/* fallback */
	எனில் (sysinfo(&info) != 0)
		திரும்பு -errno;

	boot_time->tv_sec = now.tv_sec - info.uptime;
	boot_time->tv_usec = 0;
	திரும்பு 0;
#இல்லையெனில்
	திரும்பு -ENOSYS;
#endif
}

usec_t get_suspended_time(வெறுமை)
{
#எனில் defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	கட்டமைப்பு timespec boot, mono;

	எனில் (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		திரும்பு timespec_to_usec(&boot) - timespec_to_usec(&mono);
#endif
	திரும்பு 0;
}

முழு gettime_monotonic(கட்டமைப்பு timeval *tv)
{
#ifdef CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	முழு ret;
	கட்டமைப்பு timespec ts;

	/* Linux specific, can't slew */
	எனில் (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	}
	திரும்பு ret;
#இல்லையெனில்
	திரும்பு gettimeofday(tv, வெற்று);
#endif
}


