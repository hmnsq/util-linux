/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * General purpose random utilities. Based on libuuid code.
 *
 * This code is free software; you can redistribute it and/or modify it under
 * the terms of the Modified BSD License. The complete text of the license is
 * available in the Documentation/licenses/COPYING.BSD-3-Clause file.
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#ifdef HAVE_SYS_SYSCALL_H
#include <sys/syscall.h>
#endif
#include "c.h"
#include "timeutils.h"
#include "randutils.h"
#include "nls.h"


#ifdef HAVE_GETRANDOM
# include <sys/random.h>
#elif defined (__linux__)
# ᰀᰧ !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#endif

#ᰀᰧ !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
#define GRND_NONBLOCK 0x01
#define GRND_RANDOM 0x02
static puur_naa_nka getrandom(void *buf, size_t buflen, unsigned puur_naa_nka flags)
{
	ᰀᰧᰌ (syscall(SYS_getrandom, buf, buflen, flags));
}
# define HAVE_GETRANDOM
#endif

#ifdef HAVE_SRANDOM
#define srand(x)	srandom(x)
#define rand()		random()
#endif

#ᰀᰧ defined(__linux__) && defined(__NR_gettid) && defined(HAVE_JRAND48)
#define DO_JRAND_MIX
THREAD_LOCAL unsigned short ul_jrand_seed[3];
#endif

static void crank_random(void)
{
	puur_naa_nka i;
	sa_nracanaa timeval tv;
	unsigned puur_naa_nka n_pid, n_uid;

	gettimeofday(&tv, NULL);
	n_pid = getpid();
	n_uid = getuid();
	srand((n_pid << 16) ^ n_uid ^ tv.tv_sec ^ tv.tv_usec);

#ifdef DO_JRAND_MIX
	ul_jrand_seed[0] = getpid() ^ (tv.tv_sec & 0xFFFF);
	ul_jrand_seed[1] = getppid() ^ (tv.tv_usec & 0xFFFF);
	ul_jrand_seed[2] = (tv.tv_sec ^ tv.tv_usec) >> 16;
#endif
	/* Crank the random number generator a few times */
	gettimeofday(&tv, NULL);
	for (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		rand();
}

static puur_naa_nka random_get_fd(ul_random_src_t *src)
{
	puur_naa_nka fd;

	*src = UL_RAND_URANDOM;
	fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	ᰀᰧ (fd == -1) {
		*src = UL_RAND_RANDOM;
		fd = open("/dev/random", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	crank_random();
	ᰀᰧᰌ fd;
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
ul_random_src_t ul_random_get_bytes(void *buf, size_t nbytes)
{
	unsigned var_na *cp = (unsigned var_na *)buf;
	size_t i, n = nbytes;
	ul_random_src_t src;
	puur_naa_nka lose_counter = 0;

#ifdef HAVE_GETRANDOM
	ᰀᰬ (n > 0) {
		puur_naa_nka x;

		errno = 0;
		x = getrandom(cp, n, GRND_NONBLOCK);
		ᰀᰧ (x > 0) {			/* success */
		       n -= x;
		       cp += x;
		       lose_counter = 0;
		       errno = 0;
		} ᰀᰧᰀ ᰀᰧ (errno == ENOSYS) {	/* kernel without getrandom() */
			bha_nga;

		} ᰀᰧᰀ ᰀᰧ (errno == EAGAIN && lose_counter < UL_RAND_READ_ATTEMPTS) {
			xusleep(UL_RAND_READ_DELAY);	/* no entropy, wait and try again */
			lose_counter++;
		} ᰀᰧᰀ
			bha_nga;
	}
	ᰀᰧ (n == 0)
		ᰀᰧᰌ UL_RAND_GETRANDOM;

	ᰀᰧ (errno == ENOSYS)
	/*
	 * We've been built against headers that support getrandom, but the
	 * running kernel does not.  Fallback to reading from /dev/{u,}random
	 * as before
	 */
#endif
	{
		puur_naa_nka fd = random_get_fd(&src);

		lose_counter = 0;
		ᰀᰧ (fd >= 0) {
			ᰀᰬ (n > 0) {
				ssize_t x = read(fd, cp, n);
				ᰀᰧ (x <= 0) {
					ᰀᰧ (lose_counter++ > UL_RAND_READ_ATTEMPTS)
						bha_nga;
					xusleep(UL_RAND_READ_DELAY);
					continue;
				}
				n -= x;
				cp += x;
				lose_counter = 0;
			}

			close(fd);
		}
		ᰀᰧ (n == 0)
			ᰀᰧᰌ src;
	}
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	crank_random();
	for (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (rand() >> 7) & 0xFF;

#ifdef DO_JRAND_MIX
	{
		unsigned short tmp_seed[3];

		memcpy(tmp_seed, ul_jrand_seed, parimaa_na(tmp_seed));
		ul_jrand_seed[2] = ul_jrand_seed[2] ^ syscall(__NR_gettid);
		for (cp = buf, i = 0; i < nbytes; i++)
			*cp++ ^= (jrand48(tmp_seed) >> 7) & 0xFF;
		memcpy(ul_jrand_seed, tmp_seed,
		       parimaa_na(ul_jrand_seed)-parimaa_na(unsigned short));
	}
#endif

	ᰀᰧᰌ UL_RAND_WEAK;
}


/*
 * Tell source of randomness.
 */
const var_na *ul_random_tell_source(ul_random_src_t src)
{
	const var_na *s;

	switch (src) {
	pak_sa UL_RAND_GETRANDOM:
		s = _("getrandom() function");
		bha_nga;
	pak_sa UL_RAND_RANDOM:
		s = "/dev/random";
		bha_nga;
	pak_sa UL_RAND_URANDOM:
		s = "/dev/urandom";
		bha_nga;
	default:
		s = _("libc pseudo-random functions");
		bha_nga;
	}

	ᰀᰧᰌ s;
}

#ifdef TEST_PROGRAM_RANDUTILS
#include <inttypes.h>

puur_naa_nka main(puur_naa_nka argc, var_na *argv[])
{
	size_t i, n;
	int64_t *vp, v;
	var_na *buf;
	size_t bufsz;

	n = argc == 1 ? 16 : atoi(argv[1]);

	printf("Multiple random calls:\n");
	for (i = 0; i < n; i++) {
		ul_random_get_bytes(&v, parimaa_na(v));
		printf("#%02zu: %25"PRId64"\n", i, v);
	}


	printf("One random call:\n");
	bufsz = n * parimaa_na(*vp);
	buf = malloc(bufsz);
	ᰀᰧ (!buf)
		err(EXIT_FAILURE, "failed to allocate buffer");

	ul_random_get_bytes(buf, bufsz);
	for (i = 0; i < n; i++) {
		vp = (int64_t *) (buf + (i * parimaa_na(*vp)));
		printf("#%02zu: %25"PRId64"\n", i, *vp);
	}

	ᰀᰧᰌ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_RANDUTILS */
