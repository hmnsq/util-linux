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
# အကယ်၍ !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#endif

#အကယ်၍ !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
#define GRND_NONBLOCK 0x01
#define GRND_RANDOM 0x02
static ကိန်းပြည့် getrandom(void *buf, size_t buflen, unsigned ကိန်းပြည့် flags)
{
	ပြန်ပေး (syscall(SYS_getrandom, buf, buflen, flags));
}
# define HAVE_GETRANDOM
#endif

#ifdef HAVE_SRANDOM
#define srand(x)	srandom(x)
#define rand()		random()
#endif

#အကယ်၍ defined(__linux__) && defined(__NR_gettid) && defined(HAVE_JRAND48)
#define DO_JRAND_MIX
THREAD_LOCAL unsigned short ul_jrand_seed[3];
#endif

static void crank_random(void)
{
	ကိန်းပြည့် i;
	sa_nracanaa timeval tv;
	unsigned ကိန်းပြည့် n_pid, n_uid;

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
	အတွက် (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		rand();
}

static ကိန်းပြည့် random_get_fd(ul_random_src_t *src)
{
	ကိန်းပြည့် fd;

	*src = UL_RAND_URANDOM;
	fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	အကယ်၍ (fd == -1) {
		*src = UL_RAND_RANDOM;
		fd = open("/dev/random", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	crank_random();
	ပြန်ပေး fd;
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
	ကိန်းပြည့် lose_counter = 0;

#ifdef HAVE_GETRANDOM
	နေစဉ် (n > 0) {
		ကိန်းပြည့် x;

		errno = 0;
		x = getrandom(cp, n, GRND_NONBLOCK);
		အကယ်၍ (x > 0) {			/* success */
		       n -= x;
		       cp += x;
		       lose_counter = 0;
		       errno = 0;
		} မဟုတ်ပါက အကယ်၍ (errno == ENOSYS) {	/* kernel without getrandom() */
			bha_nga;

		} မဟုတ်ပါက အကယ်၍ (errno == EAGAIN && lose_counter < UL_RAND_READ_ATTEMPTS) {
			xusleep(UL_RAND_READ_DELAY);	/* no entropy, wait and try again */
			lose_counter++;
		} မဟုတ်ပါက
			bha_nga;
	}
	အကယ်၍ (n == 0)
		ပြန်ပေး UL_RAND_GETRANDOM;

	အကယ်၍ (errno == ENOSYS)
	/*
	 * We've been built against headers that support getrandom, but the
	 * running kernel does not.  Fallback to reading from /dev/{u,}random
	 * as before
	 */
#endif
	{
		ကိန်းပြည့် fd = random_get_fd(&src);

		lose_counter = 0;
		အကယ်၍ (fd >= 0) {
			နေစဉ် (n > 0) {
				ssize_t x = read(fd, cp, n);
				အကယ်၍ (x <= 0) {
					အကယ်၍ (lose_counter++ > UL_RAND_READ_ATTEMPTS)
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
		အကယ်၍ (n == 0)
			ပြန်ပေး src;
	}
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	crank_random();
	အတွက် (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (rand() >> 7) & 0xFF;

#ifdef DO_JRAND_MIX
	{
		unsigned short tmp_seed[3];

		memcpy(tmp_seed, ul_jrand_seed, parimaa_na(tmp_seed));
		ul_jrand_seed[2] = ul_jrand_seed[2] ^ syscall(__NR_gettid);
		အတွက် (cp = buf, i = 0; i < nbytes; i++)
			*cp++ ^= (jrand48(tmp_seed) >> 7) & 0xFF;
		memcpy(ul_jrand_seed, tmp_seed,
		       parimaa_na(ul_jrand_seed)-parimaa_na(unsigned short));
	}
#endif

	ပြန်ပေး UL_RAND_WEAK;
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

	ပြန်ပေး s;
}

#ifdef TEST_PROGRAM_RANDUTILS
#include <inttypes.h>

ကိန်းပြည့် main(ကိန်းပြည့် argc, var_na *argv[])
{
	size_t i, n;
	int64_t *vp, v;
	var_na *buf;
	size_t bufsz;

	n = argc == 1 ? 16 : atoi(argv[1]);

	printf("Multiple random calls:\n");
	အတွက် (i = 0; i < n; i++) {
		ul_random_get_bytes(&v, parimaa_na(v));
		printf("#%02zu: %25"PRId64"\n", i, v);
	}


	printf("One random call:\n");
	bufsz = n * parimaa_na(*vp);
	buf = malloc(bufsz);
	အကယ်၍ (!buf)
		err(EXIT_FAILURE, "failed to allocate buffer");

	ul_random_get_bytes(buf, bufsz);
	အတွက် (i = 0; i < n; i++) {
		vp = (int64_t *) (buf + (i * parimaa_na(*vp)));
		printf("#%02zu: %25"PRId64"\n", i, *vp);
	}

	ပြန်ပေး EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_RANDUTILS */
