/*
 * Please, don't add this file to libcommon because clock_gettime() requires
 * -lrt on systems with old libc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_SYSINFO
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/sysinfo.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/time.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "monotonic.h"

ਪੂਰਨ get_boot_ਸਮਾਂ(ਬਣਤਰ timeval *boot_ਸਮਾਂ __attribute__((__unused__)))
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CLOCK_BOOTTIME
	ਬਣਤਰ timespec hires_uptime = { 0 };
	ਬਣਤਰ timeval lores_uptime = { 0 };
#ਜੇ_ਅੰਤ
	ਬਣਤਰ timeval now = { 0 };
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_SYSINFO
	ਬਣਤਰ sysinfo info;
#ਜੇ_ਅੰਤ

	ਜੇ (gettimeofday(&now, ਨਿਰਾ) != 0)
		ਵਾਪਸ -errno;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CLOCK_BOOTTIME
	ਜੇ (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_ਮੰਜ਼ਿਲ_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_ਸਮਾਂ);
		ਵਾਪਸ 0;
	}
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_SYSINFO
	/* fallback */
	ਜੇ (sysinfo(&info) != 0)
		ਵਾਪਸ -errno;

	boot_ਸਮਾਂ->tv_sec = now.tv_sec - info.uptime;
	boot_ਸਮਾਂ->tv_usec = 0;
	ਵਾਪਸ 0;
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ -ENOSYS;
#ਜੇ_ਅੰਤ
}

usec_t get_suspended_ਸਮਾਂ(ਖਾਲੀ)
{
#ਜੇ defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	ਬਣਤਰ timespec boot, mono;

	ਜੇ (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		ਵਾਪਸ timespec_ਮੰਜ਼ਿਲ_usec(&boot) - timespec_ਮੰਜ਼ਿਲ_usec(&mono);
#ਜੇ_ਅੰਤ
	ਵਾਪਸ 0;
}

ਪੂਰਨ gettime_monotonic(ਬਣਤਰ timeval *tv)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	ਪੂਰਨ ret;
	ਬਣਤਰ timespec ts;

	/* Linux specific, can't slew */
	ਜੇ (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_ਪ੍ਰਤੀ_USEC;
	}
	ਵਾਪਸ ret;
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ gettimeofday(tv, ਨਿਰਾ);
#ਜੇ_ਅੰਤ
}


