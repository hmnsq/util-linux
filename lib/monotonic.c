/*
 * Please, don't add this file to libcommon because clock_gettime() requires
 * -lrt on systems with old libc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#అన్తర్భావ <time.h>
#అన్తర్భావ <signal.h>
#నిర్వచితమైతే HAVE_SYSINFO
#అన్తర్భావ <sys/sysinfo.h>
#అయితే_ముగింపు
#అన్తర్భావ <sys/time.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "monotonic.h"

పూర్ణాంకం get_boot_సమయం(నిర్మాణం timeval *boot_సమయం __attribute__((__unused__)))
{
#నిర్వచితమైతే CLOCK_BOOTTIME
	నిర్మాణం timespec hires_uptime = { 0 };
	నిర్మాణం timeval lores_uptime = { 0 };
#అయితే_ముగింపు
	నిర్మాణం timeval now = { 0 };
#నిర్వచితమైతే HAVE_SYSINFO
	నిర్మాణం sysinfo info;
#అయితే_ముగింపు

	ఒకవేళ (gettimeofday(&now, ఖాళీ) != 0)
		తిరిగి -errno;
#నిర్వచితమైతే CLOCK_BOOTTIME
	ఒకవేళ (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_గమ్యం_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_సమయం);
		తిరిగి 0;
	}
#అయితే_ముగింపు
#నిర్వచితమైతే HAVE_SYSINFO
	/* fallback */
	ఒకవేళ (sysinfo(&info) != 0)
		తిరిగి -errno;

	boot_సమయం->tv_sec = now.tv_sec - info.uptime;
	boot_సమయం->tv_usec = 0;
	తిరిగి 0;
#లేకపోతే
	తిరిగి -ENOSYS;
#అయితే_ముగింపు
}

usec_t get_suspended_సమయం(శూన్య)
{
#ఒకవేళ defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	నిర్మాణం timespec boot, mono;

	ఒకవేళ (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		తిరిగి timespec_గమ్యం_usec(&boot) - timespec_గమ్యం_usec(&mono);
#అయితే_ముగింపు
	తిరిగి 0;
}

పూర్ణాంకం gettime_monotonic(నిర్మాణం timeval *tv)
{
#నిర్వచితమైతే CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	పూర్ణాంకం ret;
	నిర్మాణం timespec ts;

	/* Linux specific, can't slew */
	ఒకవేళ (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_చొప్పున_USEC;
	}
	తిరిగి ret;
#లేకపోతే
	తిరిగి gettimeofday(tv, ఖాళీ);
#అయితే_ముగింపు
}


