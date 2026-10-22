/*
 * Please, don't add this file to libcommon because clock_gettime() requires
 * -lrt on systems with old libc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#شامل <time.h>
#شامل <signal.h>
#اگر_تعریف_شدہ HAVE_SYSINFO
#شامل <sys/sysinfo.h>
#اگر_ختم
#شامل <sys/time.h>

#شامل "c.h"
#شامل "monotonic.h"

صحیح get_boot_وقت(ساخت timeval *boot_وقت __attribute__((__unused__)))
{
#اگر_تعریف_شدہ CLOCK_BOOTTIME
	ساخت timespec hires_uptime = { 0 };
	ساخت timeval lores_uptime = { 0 };
#اگر_ختم
	ساخت timeval now = { 0 };
#اگر_تعریف_شدہ HAVE_SYSINFO
	ساخت sysinfo info;
#اگر_ختم

	اگر (gettimeofday(&now, تہی) != 0)
		واپس -errno;
#اگر_تعریف_شدہ CLOCK_BOOTTIME
	اگر (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_مقصد_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_وقت);
		واپس 0;
	}
#اگر_ختم
#اگر_تعریف_شدہ HAVE_SYSINFO
	/* fallback */
	اگر (sysinfo(&info) != 0)
		واپس -errno;

	boot_وقت->tv_sec = now.tv_sec - info.uptime;
	boot_وقت->tv_usec = 0;
	واپس 0;
#ورنہ
	واپس -ENOSYS;
#اگر_ختم
}

usec_t get_suspended_وقت(خالی)
{
#اگر defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	ساخت timespec boot, mono;

	اگر (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		واپس timespec_مقصد_usec(&boot) - timespec_مقصد_usec(&mono);
#اگر_ختم
	واپس 0;
}

صحیح gettime_monotonic(ساخت timeval *tv)
{
#اگر_تعریف_شدہ CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	صحیح ret;
	ساخت timespec ts;

	/* Linux specific, can't slew */
	اگر (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_فی_USEC;
	}
	واپس ret;
#ورنہ
	واپس gettimeofday(tv, تہی);
#اگر_ختم
}


