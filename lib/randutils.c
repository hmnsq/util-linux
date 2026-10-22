/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * General purpose random utilities. Based on libuuid code.
 *
 * This code is free software; you can redistribute it and/or modify it under
 * the terms of the Modified BSD License. The complete text of the license is
 * available in the Documentation/licenses/COPYING.BSD-3-Clause file.
 */
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/time.h>
#నిర్వచితమైతే HAVE_SYS_SYSCALL_H
#అన్తర్భావ <sys/syscall.h>
#అయితే_ముగింపు
#అన్తర్భావ "c.h"
#అన్తర్భావ "timeutils.h"
#అన్తర్భావ "randutils.h"
#అన్తర్భావ "nls.h"


#నిర్వచితమైతే HAVE_GETRANDOM
# include <sys/random.h>
#లేదావేళ defined (__linux__)
# ఒకవేళ !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#అయితే_ముగింపు

#ఒకవేళ !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
#నిర్వచించు GRND_NONBLOCK 0x01
#నిర్వచించు GRND_RANDOM 0x02
స్థిర పూర్ణాంకం getrandom(శూన్య *buf, size_t buflen, గుర్తులేని పూర్ణాంకం flags)
{
	తిరిగి (syscall(SYS_getrandom, buf, buflen, flags));
}
# define HAVE_GETRANDOM
#అయితే_ముగింపు

#నిర్వచితమైతే HAVE_SRANDOM
#నిర్వచించు srand(x)	srandom(x)
#నిర్వచించు rand()		random()
#అయితే_ముగింపు

#ఒకవేళ defined(__linux__) && defined(__NR_gettid) && defined(HAVE_JRAND48)
#నిర్వచించు DO_JRAND_MIX
THREAD_LOCAL గుర్తులేని పొట్టి ul_jrand_విత్తనం[3];
#అయితే_ముగింపు

స్థిర శూన్య crank_random(శూన్య)
{
	పూర్ణాంకం i;
	నిర్మాణం timeval tv;
	గుర్తులేని పూర్ణాంకం n_pid, n_uid;

	gettimeofday(&tv, ఖాళీ);
	n_pid = getpid();
	n_uid = getuid();
	srand((n_pid << 16) ^ n_uid ^ tv.tv_sec ^ tv.tv_usec);

#నిర్వచితమైతే DO_JRAND_MIX
	ul_jrand_విత్తనం[0] = getpid() ^ (tv.tv_sec & 0xFFFF);
	ul_jrand_విత్తనం[1] = getppid() ^ (tv.tv_usec & 0xFFFF);
	ul_jrand_విత్తనం[2] = (tv.tv_sec ^ tv.tv_usec) >> 16;
#అయితే_ముగింపు
	/* Crank the random number generator a few times */
	gettimeofday(&tv, ఖాళీ);
	కొరకు (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		rand();
}

స్థిర పూర్ణాంకం random_get_fd(ul_random_src_t *src)
{
	పూర్ణాంకం fd;

	*src = UL_RAND_URANDOM;
	fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	ఒకవేళ (fd == -1) {
		*src = UL_RAND_RANDOM;
		fd = open("/dev/random", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	crank_random();
	తిరిగి fd;
}

/*
 * Generate a stream of random nbytes into buf.
 * Use /dev/urandom if possible, and if not,
 * use glibc pseudo-random functions.
 */
#నిర్వచించు UL_RAND_READ_ATTEMPTS	8
#నిర్వచించు UL_RAND_READ_DELAY	125000	/* microseconds */

/*
 * Write @nbytes random bytes into @buf.
 *
 * Returns source of random bytes (0 for weak quality).
 */
ul_random_src_t ul_random_get_బైట్లు(శూన్య *buf, size_t nbytes)
{
	గుర్తులేని అక్షరం *cp = (గుర్తులేని అక్షరం *)buf;
	size_t i, n = nbytes;
	ul_random_src_t src;
	పూర్ణాంకం lose_counter = 0;

#నిర్వచితమైతే HAVE_GETRANDOM
	అయితే (n > 0) {
		పూర్ణాంకం x;

		errno = 0;
		x = getrandom(cp, n, GRND_NONBLOCK);
		ఒకవేళ (x > 0) {			/* success */
		       n -= x;
		       cp += x;
		       lose_counter = 0;
		       errno = 0;
		} లేకపోతే ఒకవేళ (errno == ENOSYS) {	/* kernel without getrandom() */
			ఆపు;

		} లేకపోతే ఒకవేళ (errno == EAGAIN && lose_counter < UL_RAND_READ_ATTEMPTS) {
			xusleep(UL_RAND_READ_DELAY);	/* no entropy, wait and try again */
			lose_counter++;
		} లేకపోతే
			ఆపు;
	}
	ఒకవేళ (n == 0)
		తిరిగి UL_RAND_GETRANDOM;

	ఒకవేళ (errno == ENOSYS)
	/*
	 * We've been built against headers that support getrandom, but the
	 * running kernel does not.  Fallback to reading from /dev/{u,}random
	 * as before
	 */
#అయితే_ముగింపు
	{
		పూర్ణాంకం fd = random_get_fd(&src);

		lose_counter = 0;
		ఒకవేళ (fd >= 0) {
			అయితే (n > 0) {
				ssize_t x = పఠ(fd, cp, n);
				ఒకవేళ (x <= 0) {
					ఒకవేళ (lose_counter++ > UL_RAND_READ_ATTEMPTS)
						ఆపు;
					xusleep(UL_RAND_READ_DELAY);
					కొనసాగు;
				}
				n -= x;
				cp += x;
				lose_counter = 0;
			}

			close(fd);
		}
		ఒకవేళ (n == 0)
			తిరిగి src;
	}
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	crank_random();
	కొరకు (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (rand() >> 7) & 0xFF;

#నిర్వచితమైతే DO_JRAND_MIX
	{
		గుర్తులేని పొట్టి tmp_విత్తనం[3];

		జ్ఞాపకం_నకలు(tmp_విత్తనం, ul_jrand_విత్తనం, పరిమాణం(tmp_విత్తనం));
		ul_jrand_విత్తనం[2] = ul_jrand_విత్తనం[2] ^ syscall(__NR_gettid);
		కొరకు (cp = buf, i = 0; i < nbytes; i++)
			*cp++ ^= (jrand48(tmp_విత్తనం) >> 7) & 0xFF;
		జ్ఞాపకం_నకలు(ul_jrand_విత్తనం, tmp_విత్తనం,
		       పరిమాణం(ul_jrand_విత్తనం)-పరిమాణం(గుర్తులేని పొట్టి));
	}
#అయితే_ముగింపు

	తిరిగి UL_RAND_WEAK;
}


/*
 * Tell source of randomness.
 */
స్థిరరాశి అక్షరం *ul_random_tell_ఆధారం(ul_random_src_t src)
{
	స్థిరరాశి అక్షరం *s;

	ఎంపిక (src) {
	స్థితి UL_RAND_GETRANDOM:
		s = _("getrandom() function");
		ఆపు;
	స్థితి UL_RAND_RANDOM:
		s = "/dev/random";
		ఆపు;
	స్థితి UL_RAND_URANDOM:
		s = "/dev/urandom";
		ఆపు;
	అప్రమేయ:
		s = _("libc pseudo-random functions");
		ఆపు;
	}

	తిరిగి s;
}

#నిర్వచితమైతే TEST_కార్యక్రమం_RANDUTILS
#అన్తర్భావ <inttypes.h>

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[])
{
	size_t i, n;
	int64_t *vp, v;
	అక్షరం *buf;
	size_t bufsz;

	n = argc == 1 ? 16 : atoi(argv[1]);

	రాయి("Multiple random calls:\n");
	కొరకు (i = 0; i < n; i++) {
		ul_random_get_బైట్లు(&v, పరిమాణం(v));
		రాయి("#%02zu: %25"PRId64"\n", i, v);
	}


	రాయి("One random call:\n");
	bufsz = n * పరిమాణం(*vp);
	buf = జ్ఞాపకం_ఇవ్వు(bufsz);
	ఒకవేళ (!buf)
		err(EXIT_FAILURE, "failed to allocate buffer");

	ul_random_get_బైట్లు(buf, bufsz);
	కొరకు (i = 0; i < n; i++) {
		vp = (int64_t *) (buf + (i * పరిమాణం(*vp)));
		రాయి("#%02zu: %25"PRId64"\n", i, *vp);
	}

	తిరిగి EXIT_SUCCESS;
}
#అయితే_ముగింపు /* TEST_PROGRAM_RANDUTILS */
