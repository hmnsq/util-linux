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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_SYS_SYSCALL_H
#ಅನ್ತರ್ಭಾವ <sys/syscall.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "timeutils.h"
#ಅನ್ತರ್ಭಾವ "randutils.h"
#ಅನ್ತರ್ಭಾವ "nls.h"


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_GETRANDOM
# include <sys/random.h>
#ಅಥವಾಆದರೆ defined (__linux__)
# ಆದರೆ !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
#ವ್ಯಾಖ್ಯಾನಿಸು GRND_NONBLOCK 0x01
#ವ್ಯಾಖ್ಯಾನಿಸು GRND_RANDOM 0x02
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getrandom(ಶೂನ್ಯ *buf, size_t buflen, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ flags)
{
	ಹಿಂತಿರುಗಿ (syscall(SYS_getrandom, buf, buflen, flags));
}
# define HAVE_GETRANDOM
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_SRANDOM
#ವ್ಯಾಖ್ಯಾನಿಸು srand(x)	srandom(x)
#ವ್ಯಾಖ್ಯಾನಿಸು rand()		random()
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(__linux__) && defined(__NR_gettid) && defined(HAVE_JRAND48)
#ವ್ಯಾಖ್ಯಾನಿಸು DO_JRAND_MIX
THREAD_LOCAL ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ul_jrand_ಬೀಜ[3];
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಶೂನ್ಯ crank_random(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ i;
	ರಚನೆ timeval tv;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ n_pid, n_uid;

	gettimeofday(&tv, ಖಾಲಿ);
	n_pid = getpid();
	n_uid = getuid();
	srand((n_pid << 16) ^ n_uid ^ tv.tv_sec ^ tv.tv_usec);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DO_JRAND_MIX
	ul_jrand_ಬೀಜ[0] = getpid() ^ (tv.tv_sec & 0xFFFF);
	ul_jrand_ಬೀಜ[1] = getppid() ^ (tv.tv_usec & 0xFFFF);
	ul_jrand_ಬೀಜ[2] = (tv.tv_sec ^ tv.tv_usec) >> 16;
#ಆದರೆ_ಅಂತ್ಯ
	/* Crank the random number generator a few times */
	gettimeofday(&tv, ಖಾಲಿ);
	ಗಾಗಿ (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		rand();
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ random_get_fd(ul_random_src_t *src)
{
	ಪೂರ್ಣಾಂಕ fd;

	*src = UL_RAND_URANDOM;
	fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	ಆದರೆ (fd == -1) {
		*src = UL_RAND_RANDOM;
		fd = open("/dev/random", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	crank_random();
	ಹಿಂತಿರುಗಿ fd;
}

/*
 * Generate a stream of random nbytes into buf.
 * Use /dev/urandom if possible, and if not,
 * use glibc pseudo-random functions.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು UL_RAND_READ_ATTEMPTS	8
#ವ್ಯಾಖ್ಯಾನಿಸು UL_RAND_READ_DELAY	125000	/* microseconds */

/*
 * Write @nbytes random bytes into @buf.
 *
 * Returns source of random bytes (0 for weak quality).
 */
ul_random_src_t ul_random_get_ಬೈಟುಗಳು(ಶೂನ್ಯ *buf, size_t nbytes)
{
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *cp = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)buf;
	size_t i, n = nbytes;
	ul_random_src_t src;
	ಪೂರ್ಣಾಂಕ lose_counter = 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_GETRANDOM
	ಆಗ (n > 0) {
		ಪೂರ್ಣಾಂಕ x;

		errno = 0;
		x = getrandom(cp, n, GRND_NONBLOCK);
		ಆದರೆ (x > 0) {			/* success */
		       n -= x;
		       cp += x;
		       lose_counter = 0;
		       errno = 0;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (errno == ENOSYS) {	/* kernel without getrandom() */
			ನಿಲ್ಲಿಸು;

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (errno == EAGAIN && lose_counter < UL_RAND_READ_ATTEMPTS) {
			xusleep(UL_RAND_READ_DELAY);	/* no entropy, wait and try again */
			lose_counter++;
		} ಇಲ್ಲದಿದ್ದರೆ
			ನಿಲ್ಲಿಸು;
	}
	ಆದರೆ (n == 0)
		ಹಿಂತಿರುಗಿ UL_RAND_GETRANDOM;

	ಆದರೆ (errno == ENOSYS)
	/*
	 * We've been built against headers that support getrandom, but the
	 * running kernel does not.  Fallback to reading from /dev/{u,}random
	 * as before
	 */
#ಆದರೆ_ಅಂತ್ಯ
	{
		ಪೂರ್ಣಾಂಕ fd = random_get_fd(&src);

		lose_counter = 0;
		ಆದರೆ (fd >= 0) {
			ಆಗ (n > 0) {
				ssize_t x = ಪಠ(fd, cp, n);
				ಆದರೆ (x <= 0) {
					ಆದರೆ (lose_counter++ > UL_RAND_READ_ATTEMPTS)
						ನಿಲ್ಲಿಸು;
					xusleep(UL_RAND_READ_DELAY);
					ಮುಂದುವರಿ;
				}
				n -= x;
				cp += x;
				lose_counter = 0;
			}

			close(fd);
		}
		ಆದರೆ (n == 0)
			ಹಿಂತಿರುಗಿ src;
	}
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	crank_random();
	ಗಾಗಿ (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (rand() >> 7) & 0xFF;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DO_JRAND_MIX
	{
		ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ tmp_ಬೀಜ[3];

		ಸ್ಮರಣೆ_ನಕಲು(tmp_ಬೀಜ, ul_jrand_ಬೀಜ, ಗಾತ್ರ(tmp_ಬೀಜ));
		ul_jrand_ಬೀಜ[2] = ul_jrand_ಬೀಜ[2] ^ syscall(__NR_gettid);
		ಗಾಗಿ (cp = buf, i = 0; i < nbytes; i++)
			*cp++ ^= (jrand48(tmp_ಬೀಜ) >> 7) & 0xFF;
		ಸ್ಮರಣೆ_ನಕಲು(ul_jrand_ಬೀಜ, tmp_ಬೀಜ,
		       ಗಾತ್ರ(ul_jrand_ಬೀಜ)-ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ));
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ UL_RAND_WEAK;
}


/*
 * Tell source of randomness.
 */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ul_random_tell_ಮೂಲ(ul_random_src_t src)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s;

	ಆಯ್ಕೆ (src) {
	ಸ್ಥಿತಿ UL_RAND_GETRANDOM:
		s = _("getrandom() function");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ UL_RAND_RANDOM:
		s = "/dev/random";
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ UL_RAND_URANDOM:
		s = "/dev/urandom";
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		s = _("libc pseudo-random functions");
		ನಿಲ್ಲಿಸು;
	}

	ಹಿಂತಿರುಗಿ s;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_RANDUTILS
#ಅನ್ತರ್ಭಾವ <inttypes.h>

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	size_t i, n;
	int64_t *vp, v;
	ಅಕ್ಷರ *buf;
	size_t bufsz;

	n = argc == 1 ? 16 : atoi(argv[1]);

	ಬರೆ("Multiple random calls:\n");
	ಗಾಗಿ (i = 0; i < n; i++) {
		ul_random_get_ಬೈಟುಗಳು(&v, ಗಾತ್ರ(v));
		ಬರೆ("#%02zu: %25"PRId64"\n", i, v);
	}


	ಬರೆ("One random call:\n");
	bufsz = n * ಗಾತ್ರ(*vp);
	buf = ಸ್ಮರಣೆ_ಕೊಡು(bufsz);
	ಆದರೆ (!buf)
		err(EXIT_FAILURE, "failed to allocate buffer");

	ul_random_get_ಬೈಟುಗಳು(buf, bufsz);
	ಗಾಗಿ (i = 0; i < n; i++) {
		vp = (int64_t *) (buf + (i * ಗಾತ್ರ(*vp)));
		ಬರೆ("#%02zu: %25"PRId64"\n", i, *vp);
	}

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
#ಆದರೆ_ಅಂತ್ಯ /* TEST_PROGRAM_RANDUTILS */
