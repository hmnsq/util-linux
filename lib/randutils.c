/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * General purpose random utilities. Based on libuuid code.
 *
 * This code is free software; you can redistribute it and/or modify it under
 * the terms of the Modified BSD License. The complete text of the license is
 * available in the Documentation/licenses/COPYING.BSD-3-Clause file.
 */
#ضمن <stdio.h>
#ضمن <unistd.h>
#ضمن <fcntl.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <sys/time.h>
#جے_تعریف_شدہ HAVE_SYS_SYSCALL_H
#ضمن <sys/syscall.h>
#جے_انت
#ضمن "c.h"
#ضمن "timeutils.h"
#ضمن "randutils.h"
#ضمن "nls.h"


#جے_تعریف_شدہ HAVE_GETRANDOM
# include <sys/random.h>
#وإلاإذا defined (__linux__)
# إذا !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#جے_انت

#إذا !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
#تعریف_کرو GRND_NONBLOCK 0x01
#تعریف_کرو GRND_RANDOM 0x02
ساكن صحيح getrandom(فراغ *buf, size_t buflen, بنانشانی صحيح flags)
{
	أرجع (syscall(SYS_getrandom, buf, buflen, flags));
}
# define HAVE_GETRANDOM
#جے_انت

#جے_تعریف_شدہ HAVE_SRANDOM
#تعریف_کرو srand(x)	srandom(x)
#تعریف_کرو rand()		random()
#جے_انت

#إذا defined(__linux__) && defined(__NR_gettid) && defined(HAVE_JRAND48)
#تعریف_کرو DO_JRAND_MIX
THREAD_LOCAL بنانشانی چھوٹا ul_jrand_بیج[3];
#جے_انت

ساكن فراغ crank_random(فراغ)
{
	صحيح i;
	بنية timeval tv;
	بنانشانی صحيح n_pid, n_uid;

	gettimeofday(&tv, عدم);
	n_pid = getpid();
	n_uid = getuid();
	srand((n_pid << 16) ^ n_uid ^ tv.tv_sec ^ tv.tv_usec);

#جے_تعریف_شدہ DO_JRAND_MIX
	ul_jrand_بیج[0] = getpid() ^ (tv.tv_sec & 0xFFFF);
	ul_jrand_بیج[1] = getppid() ^ (tv.tv_usec & 0xFFFF);
	ul_jrand_بیج[2] = (tv.tv_sec ^ tv.tv_usec) >> 16;
#جے_انت
	/* Crank the random number generator a few times */
	gettimeofday(&tv, عدم);
	لكل (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		rand();
}

ساكن صحيح random_get_fd(ul_random_src_t *src)
{
	صحيح fd;

	*src = UL_RAND_URANDOM;
	fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	إذا (fd == -1) {
		*src = UL_RAND_RANDOM;
		fd = open("/dev/random", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	crank_random();
	أرجع fd;
}

/*
 * Generate a stream of random nbytes into buf.
 * Use /dev/urandom if possible, and if not,
 * use glibc pseudo-random functions.
 */
#تعریف_کرو UL_RAND_READ_ATTEMPTS	8
#تعریف_کرو UL_RAND_READ_DELAY	125000	/* microseconds */

/*
 * Write @nbytes random bytes into @buf.
 *
 * Returns source of random bytes (0 for weak quality).
 */
ul_random_src_t ul_random_get_بائٹاں(فراغ *buf, size_t nbytes)
{
	بنانشانی حرف *cp = (بنانشانی حرف *)buf;
	size_t i, n = nbytes;
	ul_random_src_t src;
	صحيح lose_counter = 0;

#جے_تعریف_شدہ HAVE_GETRANDOM
	طالما (n > 0) {
		صحيح x;

		errno = 0;
		x = getrandom(cp, n, GRND_NONBLOCK);
		إذا (x > 0) {			/* success */
		       n -= x;
		       cp += x;
		       lose_counter = 0;
		       errno = 0;
		} وإلا إذا (errno == ENOSYS) {	/* kernel without getrandom() */
			اكسر;

		} وإلا إذا (errno == EAGAIN && lose_counter < UL_RAND_READ_ATTEMPTS) {
			xusleep(UL_RAND_READ_DELAY);	/* no entropy, wait and try again */
			lose_counter++;
		} وإلا
			اكسر;
	}
	إذا (n == 0)
		أرجع UL_RAND_GETRANDOM;

	إذا (errno == ENOSYS)
	/*
	 * We've been built against headers that support getrandom, but the
	 * running kernel does not.  Fallback to reading from /dev/{u,}random
	 * as before
	 */
#جے_انت
	{
		صحيح fd = random_get_fd(&src);

		lose_counter = 0;
		إذا (fd >= 0) {
			طالما (n > 0) {
				ssize_t x = اقرأ(fd, cp, n);
				إذا (x <= 0) {
					إذا (lose_counter++ > UL_RAND_READ_ATTEMPTS)
						اكسر;
					xusleep(UL_RAND_READ_DELAY);
					استمر;
				}
				n -= x;
				cp += x;
				lose_counter = 0;
			}

			close(fd);
		}
		إذا (n == 0)
			أرجع src;
	}
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	crank_random();
	لكل (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (rand() >> 7) & 0xFF;

#جے_تعریف_شدہ DO_JRAND_MIX
	{
		بنانشانی چھوٹا tmp_بیج[3];

		حافظہ_نقل(tmp_بیج, ul_jrand_بیج, حجم(tmp_بیج));
		ul_jrand_بیج[2] = ul_jrand_بیج[2] ^ syscall(__NR_gettid);
		لكل (cp = buf, i = 0; i < nbytes; i++)
			*cp++ ^= (jrand48(tmp_بیج) >> 7) & 0xFF;
		حافظہ_نقل(ul_jrand_بیج, tmp_بیج,
		       حجم(ul_jrand_بیج)-حجم(بنانشانی چھوٹا));
	}
#جے_انت

	أرجع UL_RAND_WEAK;
}


/*
 * Tell source of randomness.
 */
ثابت حرف *ul_random_tell_سروت(ul_random_src_t src)
{
	ثابت حرف *s;

	بدل (src) {
	حالة UL_RAND_GETRANDOM:
		s = _("getrandom() function");
		اكسر;
	حالة UL_RAND_RANDOM:
		s = "/dev/random";
		اكسر;
	حالة UL_RAND_URANDOM:
		s = "/dev/urandom";
		اكسر;
	افتراضي:
		s = _("libc pseudo-random functions");
		اكسر;
	}

	أرجع s;
}

#جے_تعریف_شدہ TEST_پروگرام_RANDUTILS
#ضمن <inttypes.h>

صحيح مکھ(صحيح argc, حرف *argv[])
{
	size_t i, n;
	int64_t *vp, v;
	حرف *buf;
	size_t bufsz;

	n = argc == 1 ? 16 : atoi(argv[1]);

	لکھو("Multiple random calls:\n");
	لكل (i = 0; i < n; i++) {
		ul_random_get_بائٹاں(&v, حجم(v));
		لکھو("#%02zu: %25"PRId64"\n", i, v);
	}


	لکھو("One random call:\n");
	bufsz = n * حجم(*vp);
	buf = حافظہ_دیو(bufsz);
	إذا (!buf)
		err(EXIT_FAILURE, "failed to allocate buffer");

	ul_random_get_بائٹاں(buf, bufsz);
	لكل (i = 0; i < n; i++) {
		vp = (int64_t *) (buf + (i * حجم(*vp)));
		لکھو("#%02zu: %25"PRId64"\n", i, *vp);
	}

	أرجع EXIT_SUCCESS;
}
#جے_انت /* TEST_PROGRAM_RANDUTILS */
