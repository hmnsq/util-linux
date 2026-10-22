/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * General purpose random utilities. Based on libuuid code.
 *
 * This code is free software; you can redistribute it and/or modify it under
 * the terms of the Modified BSD License. The complete text of the license is
 * available in the Documentation/licenses/COPYING.BSD-3-Clause file.
 */
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/time.h>
#ifdef HAVE_SYS_SYSCALL_H
#ಅನ್ತರ್ಭಾವ <sys/syscall.h>
#endif
#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "timeutils.h"
#ಅನ್ತರ್ಭಾವ "randutils.h"
#ಅನ್ತರ್ಭಾವ "nls.h"


#ifdef HAVE_GETRANDOM
# include <sys/random.h>
#ನೋಚೇದ್ಯದಿ defined (__linux__)
# ಯದಿ !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#endif

#ಯದಿ !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
#define GRND_NONBLOCK 0x01
#define GRND_RANDOM 0x02
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getrandom(ಶೂನ್ಯ *buf, size_t buflen, unsigned ಪೂರ್ಣಾಙ್ಕ flags)
{
	ಪ್ರತ್ಯಾಗಮ (syscall(SYS_getrandom, buf, buflen, flags));
}
# define HAVE_GETRANDOM
#endif

#ifdef HAVE_SRANDOM
#define srand(x)	srandom(x)
#define rand()		random()
#endif

#ಯದಿ defined(__linux__) && defined(__NR_gettid) && defined(HAVE_JRAND48)
#define DO_JRAND_MIX
THREAD_LOCAL unsigned short ul_jrand_seed[3];
#endif

ಸ್ಥಿರ ಶೂನ್ಯ crank_random(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	ಸಂರಚನಾ timeval tv;
	unsigned ಪೂರ್ಣಾಙ್ಕ n_pid, n_uid;

	gettimeofday(&tv, ರಿಕ್ತ);
	n_pid = getpid();
	n_uid = getuid();
	srand((n_pid << 16) ^ n_uid ^ tv.tv_sec ^ tv.tv_usec);

#ifdef DO_JRAND_MIX
	ul_jrand_seed[0] = getpid() ^ (tv.tv_sec & 0xFFFF);
	ul_jrand_seed[1] = getppid() ^ (tv.tv_usec & 0xFFFF);
	ul_jrand_seed[2] = (tv.tv_sec ^ tv.tv_usec) >> 16;
#endif
	/* Crank the random number generator a few times */
	gettimeofday(&tv, ರಿಕ್ತ);
	ಪ್ರತಿ (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		rand();
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ random_get_fd(ul_random_src_t *src)
{
	ಪೂರ್ಣಾಙ್ಕ fd;

	*src = UL_RAND_URANDOM;
	fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	ಯದಿ (fd == -1) {
		*src = UL_RAND_RANDOM;
		fd = open("/dev/random", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	crank_random();
	ಪ್ರತ್ಯಾಗಮ fd;
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
ul_random_src_t ul_random_get_bytes(ಶೂನ್ಯ *buf, size_t nbytes)
{
	unsigned ವರ್ಣ *cp = (unsigned ವರ್ಣ *)buf;
	size_t i, n = nbytes;
	ul_random_src_t src;
	ಪೂರ್ಣಾಙ್ಕ lose_counter = 0;

#ifdef HAVE_GETRANDOM
	ಯಾವತ್ (n > 0) {
		ಪೂರ್ಣಾಙ್ಕ x;

		errno = 0;
		x = getrandom(cp, n, GRND_NONBLOCK);
		ಯದಿ (x > 0) {			/* success */
		       n -= x;
		       cp += x;
		       lose_counter = 0;
		       errno = 0;
		} ಅನ್ಯಥಾ ಯದಿ (errno == ENOSYS) {	/* kernel without getrandom() */
			ಭಙ್ಗ;

		} ಅನ್ಯಥಾ ಯದಿ (errno == EAGAIN && lose_counter < UL_RAND_READ_ATTEMPTS) {
			xusleep(UL_RAND_READ_DELAY);	/* no entropy, wait and try again */
			lose_counter++;
		} ಅನ್ಯಥಾ
			ಭಙ್ಗ;
	}
	ಯದಿ (n == 0)
		ಪ್ರತ್ಯಾಗಮ UL_RAND_GETRANDOM;

	ಯದಿ (errno == ENOSYS)
	/*
	 * We've been built against headers that support getrandom, but the
	 * running kernel does not.  Fallback to reading from /dev/{u,}random
	 * as before
	 */
#endif
	{
		ಪೂರ್ಣಾಙ್ಕ fd = random_get_fd(&src);

		lose_counter = 0;
		ಯದಿ (fd >= 0) {
			ಯಾವತ್ (n > 0) {
				ssize_t x = ಪಠ(fd, cp, n);
				ಯದಿ (x <= 0) {
					ಯದಿ (lose_counter++ > UL_RAND_READ_ATTEMPTS)
						ಭಙ್ಗ;
					xusleep(UL_RAND_READ_DELAY);
					ಅನುವರ್ತ;
				}
				n -= x;
				cp += x;
				lose_counter = 0;
			}

			close(fd);
		}
		ಯದಿ (n == 0)
			ಪ್ರತ್ಯಾಗಮ src;
	}
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	crank_random();
	ಪ್ರತಿ (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (rand() >> 7) & 0xFF;

#ifdef DO_JRAND_MIX
	{
		unsigned short tmp_seed[3];

		memcpy(tmp_seed, ul_jrand_seed, ಪರಿಮಾಣ(tmp_seed));
		ul_jrand_seed[2] = ul_jrand_seed[2] ^ syscall(__NR_gettid);
		ಪ್ರತಿ (cp = buf, i = 0; i < nbytes; i++)
			*cp++ ^= (jrand48(tmp_seed) >> 7) & 0xFF;
		memcpy(ul_jrand_seed, tmp_seed,
		       ಪರಿಮಾಣ(ul_jrand_seed)-ಪರಿಮಾಣ(unsigned short));
	}
#endif

	ಪ್ರತ್ಯಾಗಮ UL_RAND_WEAK;
}


/*
 * Tell source of randomness.
 */
ಧ್ರುವ ವರ್ಣ *ul_random_tell_source(ul_random_src_t src)
{
	ಧ್ರುವ ವರ್ಣ *s;

	ವಿಕಲ್ಪ (src) {
	ಪಕ್ಷ UL_RAND_GETRANDOM:
		s = _("getrandom() function");
		ಭಙ್ಗ;
	ಪಕ್ಷ UL_RAND_RANDOM:
		s = "/dev/random";
		ಭಙ್ಗ;
	ಪಕ್ಷ UL_RAND_URANDOM:
		s = "/dev/urandom";
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		s = _("libc pseudo-random functions");
		ಭಙ್ಗ;
	}

	ಪ್ರತ್ಯಾಗಮ s;
}

#ifdef TEST_PROGRAM_RANDUTILS
#ಅನ್ತರ್ಭಾವ <inttypes.h>

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	size_t i, n;
	int64_t *vp, v;
	ವರ್ಣ *buf;
	size_t bufsz;

	n = argc == 1 ? 16 : atoi(argv[1]);

	printf("Multiple random calls:\n");
	ಪ್ರತಿ (i = 0; i < n; i++) {
		ul_random_get_bytes(&v, ಪರಿಮಾಣ(v));
		printf("#%02zu: %25"PRId64"\n", i, v);
	}


	printf("One random call:\n");
	bufsz = n * ಪರಿಮಾಣ(*vp);
	buf = malloc(bufsz);
	ಯದಿ (!buf)
		err(EXIT_FAILURE, "failed to allocate buffer");

	ul_random_get_bytes(buf, bufsz);
	ಪ್ರತಿ (i = 0; i < n; i++) {
		vp = (int64_t *) (buf + (i * ಪರಿಮಾಣ(*vp)));
		printf("#%02zu: %25"PRId64"\n", i, *vp);
	}

	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_RANDUTILS */
