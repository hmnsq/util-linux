/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * General purpose random utilities. Based on libuuid code.
 *
 * This code is free software; you can redistribute it and/or modify it under
 * the terms of the Modified BSD License. The complete text of the license is
 * available in the Documentation/licenses/COPYING.BSD-3-Clause file.
 */
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/time.h>
#ifdef HAVE_SYS_SYSCALL_H
#ଅନ୍ତର୍ଭାଵ <sys/syscall.h>
#endif
#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "timeutils.h"
#ଅନ୍ତର୍ଭାଵ "randutils.h"
#ଅନ୍ତର୍ଭାଵ "nls.h"


#ifdef HAVE_GETRANDOM
# include <sys/random.h>
#ନଚେତଯଦି defined (__linux__)
# ଯଦି !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#endif

#ଯଦି !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
#define GRND_NONBLOCK 0x01
#define GRND_RANDOM 0x02
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getrandom(ଶୂନ୍ୟ *buf, size_t buflen, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	ଫେରନ୍ତୁ (syscall(SYS_getrandom, buf, buflen, flags));
}
# define HAVE_GETRANDOM
#endif

#ifdef HAVE_SRANDOM
#define srand(x)	srandom(x)
#define rand()		random()
#endif

#ଯଦି defined(__linux__) && defined(__NR_gettid) && defined(HAVE_JRAND48)
#define DO_JRAND_MIX
THREAD_LOCAL unsigned short ul_jrand_seed[3];
#endif

ସ୍ଥିର ଶୂନ୍ୟ crank_random(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା timeval tv;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_pid, n_uid;

	gettimeofday(&tv, ଖାଲି);
	n_pid = getpid();
	n_uid = getuid();
	srand((n_pid << 16) ^ n_uid ^ tv.tv_sec ^ tv.tv_usec);

#ifdef DO_JRAND_MIX
	ul_jrand_seed[0] = getpid() ^ (tv.tv_sec & 0xFFFF);
	ul_jrand_seed[1] = getppid() ^ (tv.tv_usec & 0xFFFF);
	ul_jrand_seed[2] = (tv.tv_sec ^ tv.tv_usec) >> 16;
#endif
	/* Crank the random number generator a few times */
	gettimeofday(&tv, ଖାଲି);
	ପାଇଁ (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		rand();
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା random_get_fd(ul_random_src_t *src)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;

	*src = UL_RAND_URANDOM;
	fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	ଯଦି (fd == -1) {
		*src = UL_RAND_RANDOM;
		fd = open("/dev/random", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	crank_random();
	ଫେରନ୍ତୁ fd;
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
ul_random_src_t ul_random_get_bytes(ଶୂନ୍ୟ *buf, size_t nbytes)
{
	unsigned ଅକ୍ଷର *cp = (unsigned ଅକ୍ଷର *)buf;
	size_t i, n = nbytes;
	ul_random_src_t src;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା lose_counter = 0;

#ifdef HAVE_GETRANDOM
	ଯେତେବେଳେ (n > 0) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା x;

		errno = 0;
		x = getrandom(cp, n, GRND_NONBLOCK);
		ଯଦି (x > 0) {			/* success */
		       n -= x;
		       cp += x;
		       lose_counter = 0;
		       errno = 0;
		} ଅନ୍ୟଥା ଯଦି (errno == ENOSYS) {	/* kernel without getrandom() */
			ଭାଙ୍ଗ;

		} ଅନ୍ୟଥା ଯଦି (errno == EAGAIN && lose_counter < UL_RAND_READ_ATTEMPTS) {
			xusleep(UL_RAND_READ_DELAY);	/* no entropy, wait and try again */
			lose_counter++;
		} ଅନ୍ୟଥା
			ଭାଙ୍ଗ;
	}
	ଯଦି (n == 0)
		ଫେରନ୍ତୁ UL_RAND_GETRANDOM;

	ଯଦି (errno == ENOSYS)
	/*
	 * We've been built against headers that support getrandom, but the
	 * running kernel does not.  Fallback to reading from /dev/{u,}random
	 * as before
	 */
#endif
	{
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = random_get_fd(&src);

		lose_counter = 0;
		ଯଦି (fd >= 0) {
			ଯେତେବେଳେ (n > 0) {
				ssize_t x = ପଠ(fd, cp, n);
				ଯଦି (x <= 0) {
					ଯଦି (lose_counter++ > UL_RAND_READ_ATTEMPTS)
						ଭାଙ୍ଗ;
					xusleep(UL_RAND_READ_DELAY);
					ଜାରି;
				}
				n -= x;
				cp += x;
				lose_counter = 0;
			}

			close(fd);
		}
		ଯଦି (n == 0)
			ଫେରନ୍ତୁ src;
	}
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	crank_random();
	ପାଇଁ (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (rand() >> 7) & 0xFF;

#ifdef DO_JRAND_MIX
	{
		unsigned short tmp_seed[3];

		memcpy(tmp_seed, ul_jrand_seed, ଆକାର(tmp_seed));
		ul_jrand_seed[2] = ul_jrand_seed[2] ^ syscall(__NR_gettid);
		ପାଇଁ (cp = buf, i = 0; i < nbytes; i++)
			*cp++ ^= (jrand48(tmp_seed) >> 7) & 0xFF;
		memcpy(ul_jrand_seed, tmp_seed,
		       ଆକାର(ul_jrand_seed)-ଆକାର(unsigned short));
	}
#endif

	ଫେରନ୍ତୁ UL_RAND_WEAK;
}


/*
 * Tell source of randomness.
 */
ଅଚଳ ଅକ୍ଷର *ul_random_tell_source(ul_random_src_t src)
{
	ଅଚଳ ଅକ୍ଷର *s;

	ବାଛ (src) {
	ସ୍ଥିତି UL_RAND_GETRANDOM:
		s = _("getrandom() function");
		ଭାଙ୍ଗ;
	ସ୍ଥିତି UL_RAND_RANDOM:
		s = "/dev/random";
		ଭାଙ୍ଗ;
	ସ୍ଥିତି UL_RAND_URANDOM:
		s = "/dev/urandom";
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		s = _("libc pseudo-random functions");
		ଭାଙ୍ଗ;
	}

	ଫେରନ୍ତୁ s;
}

#ifdef TEST_PROGRAM_RANDUTILS
#ଅନ୍ତର୍ଭାଵ <inttypes.h>

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	size_t i, n;
	int64_t *vp, v;
	ଅକ୍ଷର *buf;
	size_t bufsz;

	n = argc == 1 ? 16 : atoi(argv[1]);

	printf("Multiple random calls:\n");
	ପାଇଁ (i = 0; i < n; i++) {
		ul_random_get_bytes(&v, ଆକାର(v));
		printf("#%02zu: %25"PRId64"\n", i, v);
	}


	printf("One random call:\n");
	bufsz = n * ଆକାର(*vp);
	buf = malloc(bufsz);
	ଯଦି (!buf)
		err(EXIT_FAILURE, "failed to allocate buffer");

	ul_random_get_bytes(buf, bufsz);
	ପାଇଁ (i = 0; i < n; i++) {
		vp = (int64_t *) (buf + (i * ଆକାର(*vp)));
		printf("#%02zu: %25"PRId64"\n", i, *vp);
	}

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_RANDUTILS */
