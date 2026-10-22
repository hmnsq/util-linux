/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * General purpose random utilities. Based on libuuid code.
 *
 * This code is free software; you can redistribute it and/or modify it under
 * the terms of the Modified BSD License. The complete text of the license is
 * available in the Documentation/licenses/COPYING.BSD-3-Clause file.
 */
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <unistd.h>
#অন্তর্ভাৱ <fcntl.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <sys/time.h>
#ifdef HAVE_SYS_SYSCALL_H
#অন্তর্ভাৱ <sys/syscall.h>
#endif
#অন্তর্ভাৱ "c.h"
#অন্তর্ভাৱ "timeutils.h"
#অন্তর্ভাৱ "randutils.h"
#অন্তর্ভাৱ "nls.h"


#ifdef HAVE_GETRANDOM
# include <sys/random.h>
#নোচেদ্যদি defined (__linux__)
# যদি !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#endif

#যদি !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
#define GRND_NONBLOCK 0x01
#define GRND_RANDOM 0x02
স্থির পূর্ণাঙ্ক getrandom(শূন্য *buf, size_t buflen, unsigned পূর্ণাঙ্ক flags)
{
	ঘূৰাই (syscall(SYS_getrandom, buf, buflen, flags));
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
	পূর্ণাঙ্ক i;
	সংরচনা timeval tv;
	unsigned পূর্ণাঙ্ক n_pid, n_uid;

	gettimeofday(&tv, রিক্ত);
	n_pid = getpid();
	n_uid = getuid();
	srand((n_pid << 16) ^ n_uid ^ tv.tv_sec ^ tv.tv_usec);

#ifdef DO_JRAND_MIX
	ul_jrand_seed[0] = getpid() ^ (tv.tv_sec & 0xFFFF);
	ul_jrand_seed[1] = getppid() ^ (tv.tv_usec & 0xFFFF);
	ul_jrand_seed[2] = (tv.tv_sec ^ tv.tv_usec) >> 16;
#endif
	/* Crank the random number generator a few times */
	gettimeofday(&tv, রিক্ত);
	বাবে (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		rand();
}

স্থির পূর্ণাঙ্ক random_get_fd(ul_random_src_t *src)
{
	পূর্ণাঙ্ক fd;

	*src = UL_RAND_URANDOM;
	fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	যদি (fd == -1) {
		*src = UL_RAND_RANDOM;
		fd = open("/dev/random", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	crank_random();
	ঘূৰাই fd;
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
	unsigned ৱর্ণ *cp = (unsigned ৱর্ণ *)buf;
	size_t i, n = nbytes;
	ul_random_src_t src;
	পূর্ণাঙ্ক lose_counter = 0;

#ifdef HAVE_GETRANDOM
	যেতিয়ালৈ (n > 0) {
		পূর্ণাঙ্ক x;

		errno = 0;
		x = getrandom(cp, n, GRND_NONBLOCK);
		যদি (x > 0) {			/* success */
		       n -= x;
		       cp += x;
		       lose_counter = 0;
		       errno = 0;
		} অন্যথা যদি (errno == ENOSYS) {	/* kernel without getrandom() */
			ভঙ্গ;

		} অন্যথা যদি (errno == EAGAIN && lose_counter < UL_RAND_READ_ATTEMPTS) {
			xusleep(UL_RAND_READ_DELAY);	/* no entropy, wait and try again */
			lose_counter++;
		} অন্যথা
			ভঙ্গ;
	}
	যদি (n == 0)
		ঘূৰাই UL_RAND_GETRANDOM;

	যদি (errno == ENOSYS)
	/*
	 * We've been built against headers that support getrandom, but the
	 * running kernel does not.  Fallback to reading from /dev/{u,}random
	 * as before
	 */
#endif
	{
		পূর্ণাঙ্ক fd = random_get_fd(&src);

		lose_counter = 0;
		যদি (fd >= 0) {
			যেতিয়ালৈ (n > 0) {
				ssize_t x = পঠ(fd, cp, n);
				যদি (x <= 0) {
					যদি (lose_counter++ > UL_RAND_READ_ATTEMPTS)
						ভঙ্গ;
					xusleep(UL_RAND_READ_DELAY);
					অনুৱর্ত;
				}
				n -= x;
				cp += x;
				lose_counter = 0;
			}

			close(fd);
		}
		যদি (n == 0)
			ঘূৰাই src;
	}
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	crank_random();
	বাবে (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (rand() >> 7) & 0xFF;

#ifdef DO_JRAND_MIX
	{
		unsigned short tmp_seed[3];

		memcpy(tmp_seed, ul_jrand_seed, পরিমাণ(tmp_seed));
		ul_jrand_seed[2] = ul_jrand_seed[2] ^ syscall(__NR_gettid);
		বাবে (cp = buf, i = 0; i < nbytes; i++)
			*cp++ ^= (jrand48(tmp_seed) >> 7) & 0xFF;
		memcpy(ul_jrand_seed, tmp_seed,
		       পরিমাণ(ul_jrand_seed)-পরিমাণ(unsigned short));
	}
#endif

	ঘূৰাই UL_RAND_WEAK;
}


/*
 * Tell source of randomness.
 */
ধ্রুৱ ৱর্ণ *ul_random_tell_source(ul_random_src_t src)
{
	ধ্রুৱ ৱর্ণ *s;

	ৱিকল্প (src) {
	পক্ষ UL_RAND_GETRANDOM:
		s = _("getrandom() function");
		ভঙ্গ;
	পক্ষ UL_RAND_RANDOM:
		s = "/dev/random";
		ভঙ্গ;
	পক্ষ UL_RAND_URANDOM:
		s = "/dev/urandom";
		ভঙ্গ;
	স্ৱভাৱ:
		s = _("libc pseudo-random functions");
		ভঙ্গ;
	}

	ঘূৰাই s;
}

#ifdef TEST_PROGRAM_RANDUTILS
#অন্তর্ভাৱ <inttypes.h>

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ *argv[])
{
	size_t i, n;
	int64_t *vp, v;
	ৱর্ণ *buf;
	size_t bufsz;

	n = argc == 1 ? 16 : atoi(argv[1]);

	printf("Multiple random calls:\n");
	বাবে (i = 0; i < n; i++) {
		ul_random_get_bytes(&v, পরিমাণ(v));
		printf("#%02zu: %25"PRId64"\n", i, v);
	}


	printf("One random call:\n");
	bufsz = n * পরিমাণ(*vp);
	buf = malloc(bufsz);
	যদি (!buf)
		err(EXIT_FAILURE, "failed to allocate buffer");

	ul_random_get_bytes(buf, bufsz);
	বাবে (i = 0; i < n; i++) {
		vp = (int64_t *) (buf + (i * পরিমাণ(*vp)));
		printf("#%02zu: %25"PRId64"\n", i, *vp);
	}

	ঘূৰাই EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_RANDUTILS */
