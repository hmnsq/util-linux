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
#ifdef HAVE_SYS_SYSCALL_H
#ضمن <sys/syscall.h>
#endif
#ضمن "c.h"
#ضمن "timeutils.h"
#ضمن "randutils.h"
#ضمن "nls.h"


#ifdef HAVE_GETRANDOM
# include <sys/random.h>
#وإلاإذا defined (__linux__)
# אם !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#endif

#אם !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
#define GRND_NONBLOCK 0x01
#define GRND_RANDOM 0x02
ساكن صحيح getrandom(فراغ *buf, size_t buflen, unsigned صحيح flags)
{
	החזר (syscall(SYS_getrandom, buf, buflen, flags));
}
# define HAVE_GETRANDOM
#endif

#ifdef HAVE_SRANDOM
#define srand(x)	srandom(x)
#define rand()		random()
#endif

#אם defined(__linux__) && defined(__NR_gettid) && defined(HAVE_JRAND48)
#define DO_JRAND_MIX
THREAD_LOCAL unsigned short ul_jrand_seed[3];
#endif

ساكن فراغ crank_random(فراغ)
{
	صحيح i;
	بنية timeval tv;
	unsigned صحيح n_pid, n_uid;

	gettimeofday(&tv, عدم);
	n_pid = getpid();
	n_uid = getuid();
	srand((n_pid << 16) ^ n_uid ^ tv.tv_sec ^ tv.tv_usec);

#ifdef DO_JRAND_MIX
	ul_jrand_seed[0] = getpid() ^ (tv.tv_sec & 0xFFFF);
	ul_jrand_seed[1] = getppid() ^ (tv.tv_usec & 0xFFFF);
	ul_jrand_seed[2] = (tv.tv_sec ^ tv.tv_usec) >> 16;
#endif
	/* Crank the random number generator a few times */
	gettimeofday(&tv, عدم);
	עבור (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		rand();
}

ساكن صحيح random_get_fd(ul_random_src_t *src)
{
	صحيح fd;

	*src = UL_RAND_URANDOM;
	fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	אם (fd == -1) {
		*src = UL_RAND_RANDOM;
		fd = open("/dev/random", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	crank_random();
	החזר fd;
}

/*
 * Generate a stream of random nbytes into buf.
 * Use /dev/urandom if possible, and if not,
 * use glibc pseudo-random functions.
 */
#define UL_RAND_READ_ATTEMPTS	8
#define UL_RAND_READ_DELAY	125000	/* microseconds */

/*
 * Write @nbytes random bytes into @buf.
 *
 * Returns source of random bytes (0 for weak quality).
 */
ul_random_src_t ul_random_get_bytes(فراغ *buf, size_t nbytes)
{
	unsigned حرف *cp = (unsigned حرف *)buf;
	size_t i, n = nbytes;
	ul_random_src_t src;
	صحيح lose_counter = 0;

#ifdef HAVE_GETRANDOM
	כלעוד (n > 0) {
		صحيح x;

		errno = 0;
		x = getrandom(cp, n, GRND_NONBLOCK);
		אם (x > 0) {			/* success */
		       n -= x;
		       cp += x;
		       lose_counter = 0;
		       errno = 0;
		} אחרת אם (errno == ENOSYS) {	/* kernel without getrandom() */
			اكسر;

		} אחרת אם (errno == EAGAIN && lose_counter < UL_RAND_READ_ATTEMPTS) {
			xusleep(UL_RAND_READ_DELAY);	/* no entropy, wait and try again */
			lose_counter++;
		} אחרת
			اكسر;
	}
	אם (n == 0)
		החזר UL_RAND_GETRANDOM;

	אם (errno == ENOSYS)
	/*
	 * We've been built against headers that support getrandom, but the
	 * running kernel does not.  Fallback to reading from /dev/{u,}random
	 * as before
	 */
#endif
	{
		صحيح fd = random_get_fd(&src);

		lose_counter = 0;
		אם (fd >= 0) {
			כלעוד (n > 0) {
				ssize_t x = اقرأ(fd, cp, n);
				אם (x <= 0) {
					אם (lose_counter++ > UL_RAND_READ_ATTEMPTS)
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
		אם (n == 0)
			החזר src;
	}
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	crank_random();
	עבור (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (rand() >> 7) & 0xFF;

#ifdef DO_JRAND_MIX
	{
		unsigned short tmp_seed[3];

		memcpy(tmp_seed, ul_jrand_seed, حجم(tmp_seed));
		ul_jrand_seed[2] = ul_jrand_seed[2] ^ syscall(__NR_gettid);
		עבור (cp = buf, i = 0; i < nbytes; i++)
			*cp++ ^= (jrand48(tmp_seed) >> 7) & 0xFF;
		memcpy(ul_jrand_seed, tmp_seed,
		       حجم(ul_jrand_seed)-حجم(unsigned short));
	}
#endif

	החזר UL_RAND_WEAK;
}


/*
 * Tell source of randomness.
 */
ثابت حرف *ul_random_tell_source(ul_random_src_t src)
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

	החזר s;
}

#ifdef TEST_PROGRAM_RANDUTILS
#ضمن <inttypes.h>

صحيح main(صحيح argc, حرف *argv[])
{
	size_t i, n;
	int64_t *vp, v;
	حرف *buf;
	size_t bufsz;

	n = argc == 1 ? 16 : atoi(argv[1]);

	printf("Multiple random calls:\n");
	עבור (i = 0; i < n; i++) {
		ul_random_get_bytes(&v, حجم(v));
		printf("#%02zu: %25"PRId64"\n", i, v);
	}


	printf("One random call:\n");
	bufsz = n * حجم(*vp);
	buf = malloc(bufsz);
	אם (!buf)
		err(EXIT_FAILURE, "failed to allocate buffer");

	ul_random_get_bytes(buf, bufsz);
	עבור (i = 0; i < n; i++) {
		vp = (int64_t *) (buf + (i * حجم(*vp)));
		printf("#%02zu: %25"PRId64"\n", i, *vp);
	}

	החזר EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_RANDUTILS */
