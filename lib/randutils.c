/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * General purpose random utilities. Based on libuuid code.
 *
 * This code is free software; you can redistribute it and/or modify it under
 * the terms of the Modified BSD License. The complete text of the license is
 * available in the Documentation/licenses/COPYING.BSD-3-Clause file.
 */
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/time.h>
#ifdef HAVE_SYS_SYSCALL_H
#অন্তর্ভাব <sys/syscall.h>
#endif
#অন্তর্ভাব "c.h"
#অন্তর্ভাব "timeutils.h"
#অন্তর্ভাব "randutils.h"
#অন্তর্ভাব "nls.h"


#ifdef HAVE_GETRANDOM
# include <sys/random.h>
#নতুবা_যদি defined (__linux__)
# যদি !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#endif

#যদি !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
#define GRND_NONBLOCK 0x01
#define GRND_RANDOM 0x02
স্থির পূর্ণসংখ্যা getrandom(শূন্য *buf, size_t buflen, unsigned পূর্ণসংখ্যা flags)
{
	ফেরত (syscall(SYS_getrandom, buf, buflen, flags));
}
# define HAVE_GETRANDOM
#endif

#ifdef HAVE_SRANDOM
#define srand(x)	srandom(x)
#define rand()		random()
#endif

#যদি defined(__linux__) && defined(__NR_gettid) && defined(HAVE_JRAND48)
#define DO_JRAND_MIX
THREAD_LOCAL unsigned short ul_jrand_seed[3];
#endif

স্থির শূন্য crank_random(শূন্য)
{
	পূর্ণসংখ্যা i;
	কাঠামো timeval tv;
	unsigned পূর্ণসংখ্যা n_pid, n_uid;

	gettimeofday(&tv, ফাঁকা);
	n_pid = getpid();
	n_uid = getuid();
	srand((n_pid << 16) ^ n_uid ^ tv.tv_sec ^ tv.tv_usec);

#ifdef DO_JRAND_MIX
	ul_jrand_seed[0] = getpid() ^ (tv.tv_sec & 0xFFFF);
	ul_jrand_seed[1] = getppid() ^ (tv.tv_usec & 0xFFFF);
	ul_jrand_seed[2] = (tv.tv_sec ^ tv.tv_usec) >> 16;
#endif
	/* Crank the random number generator a few times */
	gettimeofday(&tv, ফাঁকা);
	জন্য (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		rand();
}

স্থির পূর্ণসংখ্যা random_get_fd(ul_random_src_t *src)
{
	পূর্ণসংখ্যা fd;

	*src = UL_RAND_URANDOM;
	fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	যদি (fd == -1) {
		*src = UL_RAND_RANDOM;
		fd = open("/dev/random", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	crank_random();
	ফেরত fd;
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
ul_random_src_t ul_random_get_bytes(শূন্য *buf, size_t nbytes)
{
	unsigned অক্ষর *cp = (unsigned অক্ষর *)buf;
	size_t i, n = nbytes;
	ul_random_src_t src;
	পূর্ণসংখ্যা lose_গণক = 0;

#ifdef HAVE_GETRANDOM
	যখন (n > 0) {
		পূর্ণসংখ্যা x;

		errno = 0;
		x = getrandom(cp, n, GRND_NONBLOCK);
		যদি (x > 0) {			/* success */
		       n -= x;
		       cp += x;
		       lose_গণক = 0;
		       errno = 0;
		} অন্যথা যদি (errno == ENOSYS) {	/* kernel without getrandom() */
			বিরতি;

		} অন্যথা যদি (errno == EAGAIN && lose_গণক < UL_RAND_READ_ATTEMPTS) {
			xusleep(UL_RAND_READ_DELAY);	/* no entropy, wait and try again */
			lose_গণক++;
		} অন্যথা
			বিরতি;
	}
	যদি (n == 0)
		ফেরত UL_RAND_GETRANDOM;

	যদি (errno == ENOSYS)
	/*
	 * We've been built against headers that support getrandom, but the
	 * running kernel does not.  Fallback to reading from /dev/{u,}random
	 * as before
	 */
#endif
	{
		পূর্ণসংখ্যা fd = random_get_fd(&src);

		lose_গণক = 0;
		যদি (fd >= 0) {
			যখন (n > 0) {
				ssize_t x = পঠ(fd, cp, n);
				যদি (x <= 0) {
					যদি (lose_গণক++ > UL_RAND_READ_ATTEMPTS)
						বিরতি;
					xusleep(UL_RAND_READ_DELAY);
					চালু;
				}
				n -= x;
				cp += x;
				lose_গণক = 0;
			}

			close(fd);
		}
		যদি (n == 0)
			ফেরত src;
	}
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	crank_random();
	জন্য (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (rand() >> 7) & 0xFF;

#ifdef DO_JRAND_MIX
	{
		unsigned short tmp_seed[3];

		memcpy(tmp_seed, ul_jrand_seed, আকার(tmp_seed));
		ul_jrand_seed[2] = ul_jrand_seed[2] ^ syscall(__NR_gettid);
		জন্য (cp = buf, i = 0; i < nbytes; i++)
			*cp++ ^= (jrand48(tmp_seed) >> 7) & 0xFF;
		memcpy(ul_jrand_seed, tmp_seed,
		       আকার(ul_jrand_seed)-আকার(unsigned short));
	}
#endif

	ফেরত UL_RAND_WEAK;
}


/*
 * Tell source of randomness.
 */
ধ্রুবক অক্ষর *ul_random_tell_source(ul_random_src_t src)
{
	ধ্রুবক অক্ষর *s;

	নির্বাচন (src) {
	ক্ষেত্র UL_RAND_GETRANDOM:
		s = _("getrandom() function");
		বিরতি;
	ক্ষেত্র UL_RAND_RANDOM:
		s = "/dev/random";
		বিরতি;
	ক্ষেত্র UL_RAND_URANDOM:
		s = "/dev/urandom";
		বিরতি;
	পূর্বনির্ধারিত:
		s = _("libc pseudo-random functions");
		বিরতি;
	}

	ফেরত s;
}

#ifdef TEST_PROGRAM_RANDUTILS
#অন্তর্ভাব <inttypes.h>

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	size_t i, n;
	int64_t *vp, v;
	অক্ষর *buf;
	size_t bufsz;

	n = argc == 1 ? 16 : atoi(argv[1]);

	printf("Multiple random calls:\n");
	জন্য (i = 0; i < n; i++) {
		ul_random_get_bytes(&v, আকার(v));
		printf("#%02zu: %25"PRId64"\n", i, v);
	}


	printf("One random call:\n");
	bufsz = n * আকার(*vp);
	buf = malloc(bufsz);
	যদি (!buf)
		err(EXIT_FAILURE, "failed to allocate buffer");

	ul_random_get_bytes(buf, bufsz);
	জন্য (i = 0; i < n; i++) {
		vp = (int64_t *) (buf + (i * আকার(*vp)));
		printf("#%02zu: %25"PRId64"\n", i, *vp);
	}

	ফেরত EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_RANDUTILS */
