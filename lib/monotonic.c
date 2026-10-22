/*
 * Please, don't add this file to libcommon because clock_gettime() requires
 * -lrt on systems with old libc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ضمن <time.h>
#ضمن <signal.h>
#اگر_تعریف_شده HAVE_SYSINFO
#ضمن <sys/sysinfo.h>
#پایان_اگر
#ضمن <sys/time.h>

#ضمن "c.h"
#ضمن "monotonic.h"

صحيح get_boot_زمان(بنية timeval *boot_زمان __attribute__((__unused__)))
{
#اگر_تعریف_شده CLOCK_BOOTTIME
	بنية timespec hires_uptime = { 0 };
	بنية timeval lores_uptime = { 0 };
#پایان_اگر
	بنية timeval now = { 0 };
#اگر_تعریف_شده HAVE_SYSINFO
	بنية sysinfo info;
#پایان_اگر

	إذا (gettimeofday(&now, عدم) != 0)
		أرجع -errno;
#اگر_تعریف_شده CLOCK_BOOTTIME
	إذا (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_مقصد_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_زمان);
		أرجع 0;
	}
#پایان_اگر
#اگر_تعریف_شده HAVE_SYSINFO
	/* fallback */
	إذا (sysinfo(&info) != 0)
		أرجع -errno;

	boot_زمان->tv_sec = now.tv_sec - info.uptime;
	boot_زمان->tv_usec = 0;
	أرجع 0;
#وإلا
	أرجع -ENOSYS;
#پایان_اگر
}

usec_t get_suspended_زمان(فراغ)
{
#إذا defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	بنية timespec boot, mono;

	إذا (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		أرجع timespec_مقصد_usec(&boot) - timespec_مقصد_usec(&mono);
#پایان_اگر
	أرجع 0;
}

صحيح gettime_monotonic(بنية timeval *tv)
{
#اگر_تعریف_شده CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	صحيح ret;
	بنية timespec ts;

	/* Linux specific, can't slew */
	إذا (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_در_هر_USEC;
	}
	أرجع ret;
#وإلا
	أرجع gettimeofday(tv, عدم);
#پایان_اگر
}


