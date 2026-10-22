/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * General purpose random utilities. Based on libuuid code.
 *
 * This code is free software; you can redistribute it and/or modify it under
 * the terms of the Modified BSD License. The complete text of the license is
 * available in the Documentation/licenses/COPYING.BSD-3-Clause file.
 */
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/time.h>
#ifdef HAVE_SYS_SYSCALL_H
#அந்தர்பாவ <sys/syscall.h>
#endif
#அந்தர்பாவ "c.h"
#அந்தர்பாவ "timeutils.h"
#அந்தர்பாவ "randutils.h"
#அந்தர்பாவ "nls.h"


#ifdef HAVE_GETRANDOM
# include <sys/random.h>
#அல்லதெனில் defined (__linux__)
# எனில் !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#endif

#எனில் !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
#define GRND_NONBLOCK 0x01
#define GRND_RANDOM 0x02
நிலையான முழு getrandom(வெறுமை *buf, size_t buflen, unsigned முழு flags)
{
	திரும்பு (syscall(SYS_getrandom, buf, buflen, flags));
}
# define HAVE_GETRANDOM
#endif

#ifdef HAVE_SRANDOM
#define srand(x)	srandom(x)
#define rand()		random()
#endif

#எனில் defined(__linux__) && defined(__NR_gettid) && defined(HAVE_JRAND48)
#define DO_JRAND_MIX
THREAD_LOCAL unsigned short ul_jrand_seed[3];
#endif

நிலையான வெறுமை crank_random(வெறுமை)
{
	முழு i;
	கட்டமைப்பு timeval tv;
	unsigned முழு n_pid, n_uid;

	gettimeofday(&tv, வெற்று);
	n_pid = getpid();
	n_uid = getuid();
	srand((n_pid << 16) ^ n_uid ^ tv.tv_sec ^ tv.tv_usec);

#ifdef DO_JRAND_MIX
	ul_jrand_seed[0] = getpid() ^ (tv.tv_sec & 0xFFFF);
	ul_jrand_seed[1] = getppid() ^ (tv.tv_usec & 0xFFFF);
	ul_jrand_seed[2] = (tv.tv_sec ^ tv.tv_usec) >> 16;
#endif
	/* Crank the random number generator a few times */
	gettimeofday(&tv, வெற்று);
	க்கு (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		rand();
}

நிலையான முழு random_get_fd(ul_random_src_t *src)
{
	முழு fd;

	*src = UL_RAND_URANDOM;
	fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	எனில் (fd == -1) {
		*src = UL_RAND_RANDOM;
		fd = open("/dev/random", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	crank_random();
	திரும்பு fd;
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
ul_random_src_t ul_random_get_bytes(வெறுமை *buf, size_t nbytes)
{
	unsigned எழுத்து *cp = (unsigned எழுத்து *)buf;
	size_t i, n = nbytes;
	ul_random_src_t src;
	முழு lose_counter = 0;

#ifdef HAVE_GETRANDOM
	போது (n > 0) {
		முழு x;

		errno = 0;
		x = getrandom(cp, n, GRND_NONBLOCK);
		எனில் (x > 0) {			/* success */
		       n -= x;
		       cp += x;
		       lose_counter = 0;
		       errno = 0;
		} இல்லையெனில் எனில் (errno == ENOSYS) {	/* kernel without getrandom() */
			நிறுத்து;

		} இல்லையெனில் எனில் (errno == EAGAIN && lose_counter < UL_RAND_READ_ATTEMPTS) {
			xusleep(UL_RAND_READ_DELAY);	/* no entropy, wait and try again */
			lose_counter++;
		} இல்லையெனில்
			நிறுத்து;
	}
	எனில் (n == 0)
		திரும்பு UL_RAND_GETRANDOM;

	எனில் (errno == ENOSYS)
	/*
	 * We've been built against headers that support getrandom, but the
	 * running kernel does not.  Fallback to reading from /dev/{u,}random
	 * as before
	 */
#endif
	{
		முழு fd = random_get_fd(&src);

		lose_counter = 0;
		எனில் (fd >= 0) {
			போது (n > 0) {
				ssize_t x = பட(fd, cp, n);
				எனில் (x <= 0) {
					எனில் (lose_counter++ > UL_RAND_READ_ATTEMPTS)
						நிறுத்து;
					xusleep(UL_RAND_READ_DELAY);
					தொடர்;
				}
				n -= x;
				cp += x;
				lose_counter = 0;
			}

			close(fd);
		}
		எனில் (n == 0)
			திரும்பு src;
	}
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	crank_random();
	க்கு (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (rand() >> 7) & 0xFF;

#ifdef DO_JRAND_MIX
	{
		unsigned short tmp_seed[3];

		memcpy(tmp_seed, ul_jrand_seed, அளவு(tmp_seed));
		ul_jrand_seed[2] = ul_jrand_seed[2] ^ syscall(__NR_gettid);
		க்கு (cp = buf, i = 0; i < nbytes; i++)
			*cp++ ^= (jrand48(tmp_seed) >> 7) & 0xFF;
		memcpy(ul_jrand_seed, tmp_seed,
		       அளவு(ul_jrand_seed)-அளவு(unsigned short));
	}
#endif

	திரும்பு UL_RAND_WEAK;
}


/*
 * Tell source of randomness.
 */
மாறிலி எழுத்து *ul_random_tell_source(ul_random_src_t src)
{
	மாறிலி எழுத்து *s;

	தேர்வு (src) {
	நிலை UL_RAND_GETRANDOM:
		s = _("getrandom() function");
		நிறுத்து;
	நிலை UL_RAND_RANDOM:
		s = "/dev/random";
		நிறுத்து;
	நிலை UL_RAND_URANDOM:
		s = "/dev/urandom";
		நிறுத்து;
	இயல்பு:
		s = _("libc pseudo-random functions");
		நிறுத்து;
	}

	திரும்பு s;
}

#ifdef TEST_PROGRAM_RANDUTILS
#அந்தர்பாவ <inttypes.h>

முழு முதன்மை(முழு argc, எழுத்து *argv[])
{
	size_t i, n;
	int64_t *vp, v;
	எழுத்து *buf;
	size_t bufsz;

	n = argc == 1 ? 16 : atoi(argv[1]);

	printf("Multiple random calls:\n");
	க்கு (i = 0; i < n; i++) {
		ul_random_get_bytes(&v, அளவு(v));
		printf("#%02zu: %25"PRId64"\n", i, v);
	}


	printf("One random call:\n");
	bufsz = n * அளவு(*vp);
	buf = malloc(bufsz);
	எனில் (!buf)
		err(EXIT_FAILURE, "failed to allocate buffer");

	ul_random_get_bytes(buf, bufsz);
	க்கு (i = 0; i < n; i++) {
		vp = (int64_t *) (buf + (i * அளவு(*vp)));
		printf("#%02zu: %25"PRId64"\n", i, *vp);
	}

	திரும்பு EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_RANDUTILS */
