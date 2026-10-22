/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * General purpose random utilities. Based on libuuid code.
 *
 * This code is free software; you can redistribute it and/or modify it under
 * the terms of the Modified BSD License. The complete text of the license is
 * available in the Documentation/licenses/COPYING.BSD-3-Clause file.
 */
#شامل <stdio.h>
#شامل <unistd.h>
#شامل <fcntl.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <sys/time.h>
#ifdef HAVE_SYS_SYSCALL_H
#شامل <sys/syscall.h>
#endif
#شامل "c.h"
#شامل "timeutils.h"
#شامل "randutils.h"
#شامل "nls.h"


#ifdef HAVE_GETRANDOM
# include <sys/random.h>
#وگرنهاگر defined (__linux__)
# اگر !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#endif

#اگر !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
#define GRND_NONBLOCK 0x01
#define GRND_RANDOM 0x02
ایستا صحیح getrandom(تهی *buf, size_t buflen, unsigned صحیح flags)
{
	بازگشت (syscall(SYS_getrandom, buf, buflen, flags));
}
# define HAVE_GETRANDOM
#endif

#ifdef HAVE_SRANDOM
#define srand(x)	srandom(x)
#define rand()		random()
#endif

#اگر defined(__linux__) && defined(__NR_gettid) && defined(HAVE_JRAND48)
#define DO_JRAND_MIX
THREAD_LOCAL unsigned short ul_jrand_seed[3];
#endif

ایستا تهی crank_random(تهی)
{
	صحیح i;
	ساختار timeval tv;
	unsigned صحیح n_pid, n_uid;

	gettimeofday(&tv, پوچ);
	n_pid = getpid();
	n_uid = getuid();
	srand((n_pid << 16) ^ n_uid ^ tv.tv_sec ^ tv.tv_usec);

#ifdef DO_JRAND_MIX
	ul_jrand_seed[0] = getpid() ^ (tv.tv_sec & 0xFFFF);
	ul_jrand_seed[1] = getppid() ^ (tv.tv_usec & 0xFFFF);
	ul_jrand_seed[2] = (tv.tv_sec ^ tv.tv_usec) >> 16;
#endif
	/* Crank the random number generator a few times */
	gettimeofday(&tv, پوچ);
	برای (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		rand();
}

ایستا صحیح random_get_fd(ul_random_src_t *src)
{
	صحیح fd;

	*src = UL_RAND_URANDOM;
	fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	اگر (fd == -1) {
		*src = UL_RAND_RANDOM;
		fd = open("/dev/random", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	crank_random();
	بازگشت fd;
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
ul_random_src_t ul_random_get_bytes(تهی *buf, size_t nbytes)
{
	unsigned نویسه *cp = (unsigned نویسه *)buf;
	size_t i, n = nbytes;
	ul_random_src_t src;
	صحیح lose_counter = 0;

#ifdef HAVE_GETRANDOM
	وقتی (n > 0) {
		صحیح x;

		errno = 0;
		x = getrandom(cp, n, GRND_NONBLOCK);
		اگر (x > 0) {			/* success */
		       n -= x;
		       cp += x;
		       lose_counter = 0;
		       errno = 0;
		} وگرنه اگر (errno == ENOSYS) {	/* kernel without getrandom() */
			بشکن;

		} وگرنه اگر (errno == EAGAIN && lose_counter < UL_RAND_READ_ATTEMPTS) {
			xusleep(UL_RAND_READ_DELAY);	/* no entropy, wait and try again */
			lose_counter++;
		} وگرنه
			بشکن;
	}
	اگر (n == 0)
		بازگشت UL_RAND_GETRANDOM;

	اگر (errno == ENOSYS)
	/*
	 * We've been built against headers that support getrandom, but the
	 * running kernel does not.  Fallback to reading from /dev/{u,}random
	 * as before
	 */
#endif
	{
		صحیح fd = random_get_fd(&src);

		lose_counter = 0;
		اگر (fd >= 0) {
			وقتی (n > 0) {
				ssize_t x = اقرأ(fd, cp, n);
				اگر (x <= 0) {
					اگر (lose_counter++ > UL_RAND_READ_ATTEMPTS)
						بشکن;
					xusleep(UL_RAND_READ_DELAY);
					ادامه;
				}
				n -= x;
				cp += x;
				lose_counter = 0;
			}

			close(fd);
		}
		اگر (n == 0)
			بازگشت src;
	}
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	crank_random();
	برای (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (rand() >> 7) & 0xFF;

#ifdef DO_JRAND_MIX
	{
		unsigned short tmp_seed[3];

		memcpy(tmp_seed, ul_jrand_seed, اندازه(tmp_seed));
		ul_jrand_seed[2] = ul_jrand_seed[2] ^ syscall(__NR_gettid);
		برای (cp = buf, i = 0; i < nbytes; i++)
			*cp++ ^= (jrand48(tmp_seed) >> 7) & 0xFF;
		memcpy(ul_jrand_seed, tmp_seed,
		       اندازه(ul_jrand_seed)-اندازه(unsigned short));
	}
#endif

	بازگشت UL_RAND_WEAK;
}


/*
 * Tell source of randomness.
 */
ثابت نویسه *ul_random_tell_source(ul_random_src_t src)
{
	ثابت نویسه *s;

	انتخاب (src) {
	حالت UL_RAND_GETRANDOM:
		s = _("getrandom() function");
		بشکن;
	حالت UL_RAND_RANDOM:
		s = "/dev/random";
		بشکن;
	حالت UL_RAND_URANDOM:
		s = "/dev/urandom";
		بشکن;
	پیشفرض:
		s = _("libc pseudo-random functions");
		بشکن;
	}

	بازگشت s;
}

#ifdef TEST_PROGRAM_RANDUTILS
#شامل <inttypes.h>

صحیح اصلی(صحیح argc, نویسه *argv[])
{
	size_t i, n;
	int64_t *vp, v;
	نویسه *buf;
	size_t bufsz;

	n = argc == 1 ? 16 : atoi(argv[1]);

	printf("Multiple random calls:\n");
	برای (i = 0; i < n; i++) {
		ul_random_get_bytes(&v, اندازه(v));
		printf("#%02zu: %25"PRId64"\n", i, v);
	}


	printf("One random call:\n");
	bufsz = n * اندازه(*vp);
	buf = malloc(bufsz);
	اگر (!buf)
		err(EXIT_FAILURE, "failed to allocate buffer");

	ul_random_get_bytes(buf, bufsz);
	برای (i = 0; i < n; i++) {
		vp = (int64_t *) (buf + (i * اندازه(*vp)));
		printf("#%02zu: %25"PRId64"\n", i, *vp);
	}

	بازگشت EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_RANDUTILS */
