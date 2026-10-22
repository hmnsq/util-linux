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
# 만약 !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#endif

#만약 !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
#define GRND_NONBLOCK 0x01
#define GRND_RANDOM 0x02
static 정수 getrandom(void *buf, size_t buflen, unsigned 정수 flags)
{
	반환 (syscall(SYS_getrandom, buf, buflen, flags));
}
# define HAVE_GETRANDOM
#endif

#ifdef HAVE_SRANDOM
#define srand(x)	srandom(x)
#define rand()		random()
#endif

#만약 defined(__linux__) && defined(__NR_gettid) && defined(HAVE_JRAND48)
#define DO_JRAND_MIX
THREAD_LOCAL unsigned short ul_jrand_seed[3];
#endif

static void crank_random(void)
{
	정수 i;
	struct timeval tv;
	unsigned 정수 n_pid, n_uid;

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
	위해 (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		rand();
}

static 정수 random_get_fd(ul_random_src_t *src)
{
	정수 fd;

	*src = UL_RAND_URANDOM;
	fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	만약 (fd == -1) {
		*src = UL_RAND_RANDOM;
		fd = open("/dev/random", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	crank_random();
	반환 fd;
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
	unsigned char *cp = (unsigned char *)buf;
	size_t i, n = nbytes;
	ul_random_src_t src;
	정수 lose_counter = 0;

#ifdef HAVE_GETRANDOM
	동안 (n > 0) {
		정수 x;

		errno = 0;
		x = getrandom(cp, n, GRND_NONBLOCK);
		만약 (x > 0) {			/* success */
		       n -= x;
		       cp += x;
		       lose_counter = 0;
		       errno = 0;
		} 아니면 만약 (errno == ENOSYS) {	/* kernel without getrandom() */
			break;

		} 아니면 만약 (errno == EAGAIN && lose_counter < UL_RAND_READ_ATTEMPTS) {
			xusleep(UL_RAND_READ_DELAY);	/* no entropy, wait and try again */
			lose_counter++;
		} 아니면
			break;
	}
	만약 (n == 0)
		반환 UL_RAND_GETRANDOM;

	만약 (errno == ENOSYS)
	/*
	 * We've been built against headers that support getrandom, but the
	 * running kernel does not.  Fallback to reading from /dev/{u,}random
	 * as before
	 */
#endif
	{
		정수 fd = random_get_fd(&src);

		lose_counter = 0;
		만약 (fd >= 0) {
			동안 (n > 0) {
				ssize_t x = read(fd, cp, n);
				만약 (x <= 0) {
					만약 (lose_counter++ > UL_RAND_READ_ATTEMPTS)
						break;
					xusleep(UL_RAND_READ_DELAY);
					continue;
				}
				n -= x;
				cp += x;
				lose_counter = 0;
			}

			close(fd);
		}
		만약 (n == 0)
			반환 src;
	}
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	crank_random();
	위해 (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (rand() >> 7) & 0xFF;

#ifdef DO_JRAND_MIX
	{
		unsigned short tmp_seed[3];

		memcpy(tmp_seed, ul_jrand_seed, sizeof(tmp_seed));
		ul_jrand_seed[2] = ul_jrand_seed[2] ^ syscall(__NR_gettid);
		위해 (cp = buf, i = 0; i < nbytes; i++)
			*cp++ ^= (jrand48(tmp_seed) >> 7) & 0xFF;
		memcpy(ul_jrand_seed, tmp_seed,
		       sizeof(ul_jrand_seed)-sizeof(unsigned short));
	}
#endif

	반환 UL_RAND_WEAK;
}


/*
 * Tell source of randomness.
 */
const char *ul_random_tell_source(ul_random_src_t src)
{
	const char *s;

	switch (src) {
	case UL_RAND_GETRANDOM:
		s = _("getrandom() function");
		break;
	case UL_RAND_RANDOM:
		s = "/dev/random";
		break;
	case UL_RAND_URANDOM:
		s = "/dev/urandom";
		break;
	default:
		s = _("libc pseudo-random functions");
		break;
	}

	반환 s;
}

#ifdef TEST_PROGRAM_RANDUTILS
#include <inttypes.h>

정수 main(정수 argc, char *argv[])
{
	size_t i, n;
	int64_t *vp, v;
	char *buf;
	size_t bufsz;

	n = argc == 1 ? 16 : atoi(argv[1]);

	printf("Multiple random calls:\n");
	위해 (i = 0; i < n; i++) {
		ul_random_get_bytes(&v, sizeof(v));
		printf("#%02zu: %25"PRId64"\n", i, v);
	}


	printf("One random call:\n");
	bufsz = n * sizeof(*vp);
	buf = malloc(bufsz);
	만약 (!buf)
		err(EXIT_FAILURE, "failed to allocate buffer");

	ul_random_get_bytes(buf, bufsz);
	위해 (i = 0; i < n; i++) {
		vp = (int64_t *) (buf + (i * sizeof(*vp)));
		printf("#%02zu: %25"PRId64"\n", i, *vp);
	}

	반환 EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_RANDUTILS */
