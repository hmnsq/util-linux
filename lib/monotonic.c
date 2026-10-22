/*
 * Please, don't add this file to libcommon because clock_gettime() requires
 * -lrt on systems with old libc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#अन्तर्भाव <time.h>
#अन्तर्भाव <signal.h>
#यदि_परिभाषितम् HAVE_SYSINFO
#अन्तर्भाव <sys/sysinfo.h>
#यदि_अन्तः
#अन्तर्भाव <sys/time.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "monotonic.h"

पूर्णाङ्क get_boot_time(संरचना timeval *boot_time __attribute__((__unused__)))
{
#यदि_परिभाषितम् CLOCK_BOOTTIME
	संरचना timespec hires_uptime = { 0 };
	संरचना timeval lores_uptime = { 0 };
#यदि_अन्तः
	संरचना timeval now = { 0 };
#यदि_परिभाषितम् HAVE_SYSINFO
	संरचना sysinfo info;
#यदि_अन्तः

	यदि (gettimeofday(&now, रिक्त) != 0)
		प्रत्यागम -errno;
#यदि_परिभाषितम् CLOCK_BOOTTIME
	यदि (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_गन्तव्यम्_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_time);
		प्रत्यागम 0;
	}
#यदि_अन्तः
#यदि_परिभाषितम् HAVE_SYSINFO
	/* fallback */
	यदि (sysinfo(&info) != 0)
		प्रत्यागम -errno;

	boot_time->tv_sec = now.tv_sec - info.uptime;
	boot_time->tv_usec = 0;
	प्रत्यागम 0;
#अन्यथा
	प्रत्यागम -ENOSYS;
#यदि_अन्तः
}

usec_t get_suspended_time(शून्य)
{
#यदि defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	संरचना timespec boot, mono;

	यदि (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		प्रत्यागम timespec_गन्तव्यम्_usec(&boot) - timespec_गन्तव्यम्_usec(&mono);
#यदि_अन्तः
	प्रत्यागम 0;
}

पूर्णाङ्क gettime_monotonic(संरचना timeval *tv)
{
#यदि_परिभाषितम् CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	पूर्णाङ्क ret;
	संरचना timespec ts;

	/* Linux specific, can't slew */
	यदि (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_प्रत्येकम्_USEC;
	}
	प्रत्यागम ret;
#अन्यथा
	प्रत्यागम gettimeofday(tv, रिक्त);
#यदि_अन्तः
}


