/*
 * Please, don't add this file to libcommon because clock_gettime() requires
 * -lrt on systems with old libc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <signal.h>
#ifdef HAVE_SYSINFO
#ଅନ୍ତର୍ଭାଵ <sys/sysinfo.h>
#endif
#ଅନ୍ତର୍ଭାଵ <sys/time.h>

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "monotonic.h"

ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_boot_time(ସଂରଚନା timeval *boot_time __attribute__((__unused__)))
{
#ifdef CLOCK_BOOTTIME
	ସଂରଚନା timespec hires_uptime = { 0 };
	ସଂରଚନା timeval lores_uptime = { 0 };
#endif
	ସଂରଚନା timeval now = { 0 };
#ifdef HAVE_SYSINFO
	ସଂରଚନା sysinfo info;
#endif

	ଯଦି (gettimeofday(&now, ଖାଲି) != 0)
		ଫେରନ୍ତୁ -errno;
#ifdef CLOCK_BOOTTIME
	ଯଦି (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_TO_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_time);
		ଫେରନ୍ତୁ 0;
	}
#endif
#ifdef HAVE_SYSINFO
	/* fallback */
	ଯଦି (sysinfo(&info) != 0)
		ଫେରନ୍ତୁ -errno;

	boot_time->tv_sec = now.tv_sec - info.uptime;
	boot_time->tv_usec = 0;
	ଫେରନ୍ତୁ 0;
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ -ENOSYS;
#endif
}

usec_t get_suspended_time(ଶୂନ୍ୟ)
{
#ଯଦି defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	ସଂରଚନା timespec boot, mono;

	ଯଦି (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		ଫେରନ୍ତୁ timespec_to_usec(&boot) - timespec_to_usec(&mono);
#endif
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା gettime_monotonic(ସଂରଚନା timeval *tv)
{
#ifdef CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	ସଂରଚନା timespec ts;

	/* Linux specific, can't slew */
	ଯଦି (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	}
	ଫେରନ୍ତୁ ret;
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ gettimeofday(tv, ଖାଲି);
#endif
}


