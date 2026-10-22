/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * General purpose random utilities. Based on libuuid code.
 *
 * This code is free software; you can redistribute it and/or modify it under
 * the terms of the Modified BSD License. The complete text of the license is
 * available in the Documentation/licenses/COPYING.BSD-3-Clause file.
 */
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/time.h>
#ifdef HAVE_SYS_SYSCALL_H
#અન્તર્ભાવ <sys/syscall.h>
#endif
#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "timeutils.h"
#અન્તર્ભાવ "randutils.h"
#અન્તર્ભાવ "nls.h"


#ifdef HAVE_GETRANDOM
# include <sys/random.h>
#અથવાજો defined (__linux__)
# જો !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#endif

#જો !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
#define GRND_NONBLOCK 0x01
#define GRND_RANDOM 0x02
સ્થિર પૂર્ણાંક getrandom(શૂન્ય *buf, size_t buflen, unsigned પૂર્ણાંક flags)
{
	પાછા (syscall(SYS_getrandom, buf, buflen, flags));
}
# define HAVE_GETRANDOM
#endif

#ifdef HAVE_SRANDOM
#define srand(x)	srandom(x)
#define rand()		random()
#endif

#જો defined(__linux__) && defined(__NR_gettid) && defined(HAVE_JRAND48)
#define DO_JRAND_MIX
THREAD_LOCAL unsigned short ul_jrand_seed[3];
#endif

સ્થિર શૂન્ય crank_random(શૂન્ય)
{
	પૂર્ણાંક i;
	સંરચના timeval tv;
	unsigned પૂર્ણાંક n_pid, n_uid;

	gettimeofday(&tv, ખાલી);
	n_pid = getpid();
	n_uid = getuid();
	srand((n_pid << 16) ^ n_uid ^ tv.tv_sec ^ tv.tv_usec);

#ifdef DO_JRAND_MIX
	ul_jrand_seed[0] = getpid() ^ (tv.tv_sec & 0xFFFF);
	ul_jrand_seed[1] = getppid() ^ (tv.tv_usec & 0xFFFF);
	ul_jrand_seed[2] = (tv.tv_sec ^ tv.tv_usec) >> 16;
#endif
	/* Crank the random number generator a few times */
	gettimeofday(&tv, ખાલી);
	માટે (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		rand();
}

સ્થિર પૂર્ણાંક random_get_fd(ul_random_src_t *src)
{
	પૂર્ણાંક fd;

	*src = UL_RAND_URANDOM;
	fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	જો (fd == -1) {
		*src = UL_RAND_RANDOM;
		fd = open("/dev/random", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	crank_random();
	પાછા fd;
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
ul_random_src_t ul_random_get_bytes(શૂન્ય *buf, size_t nbytes)
{
	unsigned અક્ષર *cp = (unsigned અક્ષર *)buf;
	size_t i, n = nbytes;
	ul_random_src_t src;
	પૂર્ણાંક lose_counter = 0;

#ifdef HAVE_GETRANDOM
	જ્યારે (n > 0) {
		પૂર્ણાંક x;

		errno = 0;
		x = getrandom(cp, n, GRND_NONBLOCK);
		જો (x > 0) {			/* success */
		       n -= x;
		       cp += x;
		       lose_counter = 0;
		       errno = 0;
		} નહીંતર જો (errno == ENOSYS) {	/* kernel without getrandom() */
			તોડો;

		} નહીંતર જો (errno == EAGAIN && lose_counter < UL_RAND_READ_ATTEMPTS) {
			xusleep(UL_RAND_READ_DELAY);	/* no entropy, wait and try again */
			lose_counter++;
		} નહીંતર
			તોડો;
	}
	જો (n == 0)
		પાછા UL_RAND_GETRANDOM;

	જો (errno == ENOSYS)
	/*
	 * We've been built against headers that support getrandom, but the
	 * running kernel does not.  Fallback to reading from /dev/{u,}random
	 * as before
	 */
#endif
	{
		પૂર્ણાંક fd = random_get_fd(&src);

		lose_counter = 0;
		જો (fd >= 0) {
			જ્યારે (n > 0) {
				ssize_t x = પઠ(fd, cp, n);
				જો (x <= 0) {
					જો (lose_counter++ > UL_RAND_READ_ATTEMPTS)
						તોડો;
					xusleep(UL_RAND_READ_DELAY);
					ચાલુ;
				}
				n -= x;
				cp += x;
				lose_counter = 0;
			}

			close(fd);
		}
		જો (n == 0)
			પાછા src;
	}
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	crank_random();
	માટે (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (rand() >> 7) & 0xFF;

#ifdef DO_JRAND_MIX
	{
		unsigned short tmp_seed[3];

		memcpy(tmp_seed, ul_jrand_seed, કદ(tmp_seed));
		ul_jrand_seed[2] = ul_jrand_seed[2] ^ syscall(__NR_gettid);
		માટે (cp = buf, i = 0; i < nbytes; i++)
			*cp++ ^= (jrand48(tmp_seed) >> 7) & 0xFF;
		memcpy(ul_jrand_seed, tmp_seed,
		       કદ(ul_jrand_seed)-કદ(unsigned short));
	}
#endif

	પાછા UL_RAND_WEAK;
}


/*
 * Tell source of randomness.
 */
અચલ અક્ષર *ul_random_tell_source(ul_random_src_t src)
{
	અચલ અક્ષર *s;

	પસંદગી (src) {
	સ્થિતિ UL_RAND_GETRANDOM:
		s = _("getrandom() function");
		તોડો;
	સ્થિતિ UL_RAND_RANDOM:
		s = "/dev/random";
		તોડો;
	સ્થિતિ UL_RAND_URANDOM:
		s = "/dev/urandom";
		તોડો;
	મૂળભૂત:
		s = _("libc pseudo-random functions");
		તોડો;
	}

	પાછા s;
}

#ifdef TEST_PROGRAM_RANDUTILS
#અન્તર્ભાવ <inttypes.h>

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[])
{
	size_t i, n;
	int64_t *vp, v;
	અક્ષર *buf;
	size_t bufsz;

	n = argc == 1 ? 16 : atoi(argv[1]);

	printf("Multiple random calls:\n");
	માટે (i = 0; i < n; i++) {
		ul_random_get_bytes(&v, કદ(v));
		printf("#%02zu: %25"PRId64"\n", i, v);
	}


	printf("One random call:\n");
	bufsz = n * કદ(*vp);
	buf = malloc(bufsz);
	જો (!buf)
		err(EXIT_FAILURE, "failed to allocate buffer");

	ul_random_get_bytes(buf, bufsz);
	માટે (i = 0; i < n; i++) {
		vp = (int64_t *) (buf + (i * કદ(*vp)));
		printf("#%02zu: %25"PRId64"\n", i, *vp);
	}

	પાછા EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_RANDUTILS */
