/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * General purpose random utilities. Based on libuuid code.
 *
 * This code is free software; you can redistribute it and/or modify it under
 * the terms of the Modified BSD License. The complete text of the license is
 * available in the Documentation/licenses/COPYING.BSD-3-Clause file.
 */
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/time.h>
#ifdef HAVE_SYS_SYSCALL_H
#अन्तर्भाव <sys/syscall.h>
#endif
#अन्तर्भाव "c.h"
#अन्तर्भाव "timeutils.h"
#अन्तर्भाव "randutils.h"
#अन्तर्भाव "nls.h"


#ifdef HAVE_GETRANDOM
# include <sys/random.h>
#नोचेद्यदि defined (__linux__)
# सचे !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#endif

#सचे !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
#define GRND_NONBLOCK 0x01
#define GRND_RANDOM 0x02
स्थिर पूर्णाङ्क getrandom(शून्य *buf, size_t buflen, unsigned पूर्णाङ्क flags)
{
	पटिनिवत्त (syscall(SYS_getrandom, buf, buflen, flags));
}
# define HAVE_GETRANDOM
#endif

#ifdef HAVE_SRANDOM
#define srand(x)	srandom(x)
#define rand()		random()
#endif

#सचे defined(__linux__) && defined(__NR_gettid) && defined(HAVE_JRAND48)
#define DO_JRAND_MIX
THREAD_LOCAL unsigned short ul_jrand_seed[3];
#endif

स्थिर शून्य crank_random(शून्य)
{
	पूर्णाङ्क i;
	संरचना timeval tv;
	unsigned पूर्णाङ्क n_pid, n_uid;

	gettimeofday(&tv, रिक्त);
	n_pid = getpid();
	n_uid = getuid();
	srand((n_pid << 16) ^ n_uid ^ tv.tv_sec ^ tv.tv_usec);

#ifdef DO_JRAND_MIX
	ul_jrand_seed[0] = getpid() ^ (tv.tv_sec & 0xFFFF);
	ul_jrand_seed[1] = getppid() ^ (tv.tv_usec & 0xFFFF);
	ul_jrand_seed[2] = (tv.tv_sec ^ tv.tv_usec) >> 16;
#endif
	/* Crank the random number generator a few times */
	gettimeofday(&tv, रिक्त);
	अत्थाय (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		rand();
}

स्थिर पूर्णाङ्क random_get_fd(ul_random_src_t *src)
{
	पूर्णाङ्क fd;

	*src = UL_RAND_URANDOM;
	fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	सचे (fd == -1) {
		*src = UL_RAND_RANDOM;
		fd = open("/dev/random", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	crank_random();
	पटिनिवत्त fd;
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
ul_random_src_t ul_random_get_bytes(शून्य *buf, size_t nbytes)
{
	unsigned वर्ण *cp = (unsigned वर्ण *)buf;
	size_t i, n = nbytes;
	ul_random_src_t src;
	पूर्णाङ्क lose_counter = 0;

#ifdef HAVE_GETRANDOM
	याव (n > 0) {
		पूर्णाङ्क x;

		errno = 0;
		x = getrandom(cp, n, GRND_NONBLOCK);
		सचे (x > 0) {			/* success */
		       n -= x;
		       cp += x;
		       lose_counter = 0;
		       errno = 0;
		} अञ्ञथा सचे (errno == ENOSYS) {	/* kernel without getrandom() */
			भङ्ग;

		} अञ्ञथा सचे (errno == EAGAIN && lose_counter < UL_RAND_READ_ATTEMPTS) {
			xusleep(UL_RAND_READ_DELAY);	/* no entropy, wait and try again */
			lose_counter++;
		} अञ्ञथा
			भङ्ग;
	}
	सचे (n == 0)
		पटिनिवत्त UL_RAND_GETRANDOM;

	सचे (errno == ENOSYS)
	/*
	 * We've been built against headers that support getrandom, but the
	 * running kernel does not.  Fallback to reading from /dev/{u,}random
	 * as before
	 */
#endif
	{
		पूर्णाङ्क fd = random_get_fd(&src);

		lose_counter = 0;
		सचे (fd >= 0) {
			याव (n > 0) {
				ssize_t x = पठ(fd, cp, n);
				सचे (x <= 0) {
					सचे (lose_counter++ > UL_RAND_READ_ATTEMPTS)
						भङ्ग;
					xusleep(UL_RAND_READ_DELAY);
					अनुवर्त;
				}
				n -= x;
				cp += x;
				lose_counter = 0;
			}

			close(fd);
		}
		सचे (n == 0)
			पटिनिवत्त src;
	}
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	crank_random();
	अत्थाय (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (rand() >> 7) & 0xFF;

#ifdef DO_JRAND_MIX
	{
		unsigned short tmp_seed[3];

		memcpy(tmp_seed, ul_jrand_seed, परिमाण(tmp_seed));
		ul_jrand_seed[2] = ul_jrand_seed[2] ^ syscall(__NR_gettid);
		अत्थाय (cp = buf, i = 0; i < nbytes; i++)
			*cp++ ^= (jrand48(tmp_seed) >> 7) & 0xFF;
		memcpy(ul_jrand_seed, tmp_seed,
		       परिमाण(ul_jrand_seed)-परिमाण(unsigned short));
	}
#endif

	पटिनिवत्त UL_RAND_WEAK;
}


/*
 * Tell source of randomness.
 */
ध्रुव वर्ण *ul_random_tell_source(ul_random_src_t src)
{
	ध्रुव वर्ण *s;

	विकल्प (src) {
	पक्ष UL_RAND_GETRANDOM:
		s = _("getrandom() function");
		भङ्ग;
	पक्ष UL_RAND_RANDOM:
		s = "/dev/random";
		भङ्ग;
	पक्ष UL_RAND_URANDOM:
		s = "/dev/urandom";
		भङ्ग;
	स्वभाव:
		s = _("libc pseudo-random functions");
		भङ्ग;
	}

	पटिनिवत्त s;
}

#ifdef TEST_PROGRAM_RANDUTILS
#अन्तर्भाव <inttypes.h>

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	size_t i, n;
	int64_t *vp, v;
	वर्ण *buf;
	size_t bufsz;

	n = argc == 1 ? 16 : atoi(argv[1]);

	printf("Multiple random calls:\n");
	अत्थाय (i = 0; i < n; i++) {
		ul_random_get_bytes(&v, परिमाण(v));
		printf("#%02zu: %25"PRId64"\n", i, v);
	}


	printf("One random call:\n");
	bufsz = n * परिमाण(*vp);
	buf = malloc(bufsz);
	सचे (!buf)
		err(EXIT_FAILURE, "failed to allocate buffer");

	ul_random_get_bytes(buf, bufsz);
	अत्थाय (i = 0; i < n; i++) {
		vp = (int64_t *) (buf + (i * परिमाण(*vp)));
		printf("#%02zu: %25"PRId64"\n", i, *vp);
	}

	पटिनिवत्त EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_RANDUTILS */
