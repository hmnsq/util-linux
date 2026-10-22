/*
 * Please, don't add this file to libcommon because clock_gettime() requires
 * -lrt on systems with old libc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#समावेश <time.h>
#समावेश <signal.h>
#अगर_घोषित HAVE_SYSINFO
#समावेश <sys/sysinfo.h>
#पूर्ण_अगर
#समावेश <sys/time.h>

#समावेश "c.h"
#समावेश "monotonic.h"

पूर्णांक get_boot_time(काष्ठा timeval *boot_time __attribute__((__unused__)))
{
#अगर_घोषित CLOCK_BOOTTIME
	काष्ठा timespec hires_uptime = { 0 };
	काष्ठा timeval lores_uptime = { 0 };
#पूर्ण_अगर
	काष्ठा timeval now = { 0 };
#अगर_घोषित HAVE_SYSINFO
	काष्ठा sysinfo info;
#पूर्ण_अगर

	अगर (gettimeofday(&now, शून्य) != 0)
		वापस -त्रुटि_सं;
#अगर_घोषित CLOCK_BOOTTIME
	अगर (clock_gettime(CLOCK_BOOTTIME, &hires_uptime) == 0) {
		TIMESPEC_TO_TIMEVAL(&lores_uptime, &hires_uptime);
		timersub(&now, &lores_uptime, boot_time);
		वापस 0;
	}
#पूर्ण_अगर
#अगर_घोषित HAVE_SYSINFO
	/* fallback */
	अगर (sysinfo(&info) != 0)
		वापस -त्रुटि_सं;

	boot_time->tv_sec = now.tv_sec - info.uptime;
	boot_time->tv_usec = 0;
	वापस 0;
#अन्यथा
	वापस -ENOSYS;
#पूर्ण_अगर
}

usec_t get_suspended_time(व्योम)
{
#अगर defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
	काष्ठा timespec boot, mono;

	अगर (clock_gettime(CLOCK_BOOTTIME, &boot) == 0 &&
	    clock_gettime(CLOCK_MONOTONIC, &mono) == 0)
		वापस timespec_to_usec(&boot) - timespec_to_usec(&mono);
#पूर्ण_अगर
	वापस 0;
}

पूर्णांक gettime_monotonic(काष्ठा timeval *tv)
{
#अगर_घोषित CLOCK_MONOTONIC
	/* Can slew only by ntp and adjtime */
	पूर्णांक ret;
	काष्ठा timespec ts;

	/* Linux specific, can't slew */
	अगर (!(ret = clock_gettime(UL_CLOCK_MONOTONIC, &ts))) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	}
	वापस ret;
#अन्यथा
	वापस gettimeofday(tv, शून्य);
#पूर्ण_अगर
}


