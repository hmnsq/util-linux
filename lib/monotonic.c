/*
 * Please, don't add this file to libcommon because clock_gettime() requires
 * -lrt on systems with old libc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ضمن <time.h>
#ضمن <signal.h>
#جيڪڏهن_وصف_ٿيل HAVE_SYSINFO
#ضمن <sys/sysinfo.h>
#جيڪڏهن_پورو
#ضمن <sys/time.h>

#ضمن "c.h"
#ضمن "monotonic.h"

صحيح get_boot_وقت(بنية timeval *boot_وقت __attribute__((__unused__)))
{
#جيڪڏهن_وصف_ٿيل CLOCK_BOOTTIME
	بنية timespec hires_uptime = { 0 };
	بنية timeval lores_uptime = { 0 };
#جيڪڏهن_پورو
	بنية timeval now = { 0 };
#جيڪڏهن_وصف_ٿيل HAVE_SYSINFO
	بنية sysinfo info;
#جيڪڏهن_پورو

	إذا (gettimeofday(&now, عدم) != 0)
		أرجع -errno;
#جيڪڏهن_وصف_ٿيل CLOCK_BOOTTIME
	إذا (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_منزل_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_وقت);
		أرجع 0;
	}
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل HAVE_SYSINFO
	/* fallback */
	إذا (sysinfo(&info) != 0)
		أرجع -errno;

	boot_وقت->tv_sec = now.tv_sec - info.uptime;
	boot_وقت->tv_usec = 0;
	أرجع 0;
#وإلا
	أرجع -ENOSYS;
#جيڪڏهن_پورو
}

usec_t get_suspended_وقت(فراغ)
{
#إذا defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	بنية timespec boot, mono;

	إذا (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		أرجع timespec_منزل_usec(&boot) - timespec_منزل_usec(&mono);
#جيڪڏهن_پورو
	أرجع 0;
}

صحيح gettime_monotonic(بنية timeval *tv)
{
#جيڪڏهن_وصف_ٿيل CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	صحيح ret;
	بنية timespec ts;

	/* Linux specific, can't slew */
	إذا (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_في_USEC;
	}
	أرجع ret;
#وإلا
	أرجع gettimeofday(tv, عدم);
#جيڪڏهن_پورو
}


