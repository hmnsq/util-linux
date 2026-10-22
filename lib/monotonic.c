/*
 * Please, don't add this file to libcommon because clock_gettime() requires
 * -lrt on systems with old libc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ضمن <time.h>
#ضمن <signal.h>
#جے_تعریف_شدہ HAVE_SYSINFO
#ضمن <sys/sysinfo.h>
#جے_انت
#ضمن <sys/time.h>

#ضمن "c.h"
#ضمن "monotonic.h"

صحيح get_boot_ویلا(بنية timeval *boot_ویلا __attribute__((__unused__)))
{
#جے_تعریف_شدہ CLOCK_BOOTTIME
	بنية timespec hires_uptime = { 0 };
	بنية timeval lores_uptime = { 0 };
#جے_انت
	بنية timeval now = { 0 };
#جے_تعریف_شدہ HAVE_SYSINFO
	بنية sysinfo info;
#جے_انت

	إذا (gettimeofday(&now, عدم) != 0)
		أرجع -errno;
#جے_تعریف_شدہ CLOCK_BOOTTIME
	إذا (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_منزل_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_ویلا);
		أرجع 0;
	}
#جے_انت
#جے_تعریف_شدہ HAVE_SYSINFO
	/* fallback */
	إذا (sysinfo(&info) != 0)
		أرجع -errno;

	boot_ویلا->tv_sec = now.tv_sec - info.uptime;
	boot_ویلا->tv_usec = 0;
	أرجع 0;
#وإلا
	أرجع -ENOSYS;
#جے_انت
}

usec_t get_suspended_ویلا(فراغ)
{
#إذا defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	بنية timespec boot, mono;

	إذا (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		أرجع timespec_منزل_usec(&boot) - timespec_منزل_usec(&mono);
#جے_انت
	أرجع 0;
}

صحيح gettime_monotonic(بنية timeval *tv)
{
#جے_تعریف_شدہ CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	صحيح ret;
	بنية timespec ts;

	/* Linux specific, can't slew */
	إذا (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_فی_USEC;
	}
	أرجع ret;
#وإلا
	أرجع gettimeofday(tv, عدم);
#جے_انت
}


