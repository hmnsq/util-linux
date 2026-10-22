/*
 * Please, don't add this file to libcommon because clock_gettime() requires
 * -lrt on systems with old libc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#अन्तर्भाव <time.h>
#अन्तर्भाव <signal.h>
#ifdef HAVE_SYSINFO
#अन्तर्भाव <sys/sysinfo.h>
#endif
#अन्तर्भाव <sys/time.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "monotonic.h"

पूर्णांक get_boot_time(संरचना timeval *boot_time __attribute__((__unused__)))
{
#ifdef CLOCK_BOOTTIME
	संरचना timespec hires_uptime = { 0 };
	संरचना timeval lores_uptime = { 0 };
#endif
	संरचना timeval now = { 0 };
#ifdef HAVE_SYSINFO
	संरचना sysinfo info;
#endif

	जर (gettimeofday(&now, रिक्त) != 0)
		परत -त्रुटि_सं;
#ifdef CLOCK_BOOTTIME
	जर (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_TO_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_time);
		परत 0;
	}
#endif
#ifdef HAVE_SYSINFO
	/* fallback */
	जर (sysinfo(&info) != 0)
		परत -त्रुटि_सं;

	boot_time->tv_sec = now.tv_sec - info.uptime;
	boot_time->tv_usec = 0;
	परत 0;
#अन्यथा
	परत -ENOSYS;
#endif
}

usec_t get_suspended_time(शून्य)
{
#जर defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	संरचना timespec boot, mono;

	जर (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		परत timespec_to_usec(&boot) - timespec_to_usec(&mono);
#endif
	परत 0;
}

पूर्णांक gettime_monotonic(संरचना timeval *tv)
{
#ifdef CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	पूर्णांक ret;
	संरचना timespec ts;

	/* Linux specific, can't slew */
	जर (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	}
	परत ret;
#अन्यथा
	परत gettimeofday(tv, रिक्त);
#endif
}


