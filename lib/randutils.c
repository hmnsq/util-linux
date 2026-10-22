/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * General purpose random utilities. Based on libuuid code.
 *
 * This code is free software; you can redistribute it and/or modify it under
 * the terms of the Modified BSD License. The complete text of the license is
 * available in the Documentation/licenses/COPYING.BSD-3-Clause file.
 */
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/time.h>
#ifdef HAVE_SYS_SYSCALL_H
#അന്തര്ഭാവ <sys/syscall.h>
#endif
#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "timeutils.h"
#അന്തര്ഭാവ "randutils.h"
#അന്തര്ഭാവ "nls.h"


#ifdef HAVE_GETRANDOM
# include <sys/random.h>
#അല്ലെങ്കിലെങ്കിൽ defined (__linux__)
# എങ്കിൽ !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#endif

#എങ്കിൽ !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
#define GRND_NONBLOCK 0x01
#define GRND_RANDOM 0x02
സ്ഥിതം പൂർണ്ണസംഖ്യ getrandom(ശൂന്യം *buf, size_t buflen, unsigned പൂർണ്ണസംഖ്യ flags)
{
	തിരികെ (syscall(SYS_getrandom, buf, buflen, flags));
}
# define HAVE_GETRANDOM
#endif

#ifdef HAVE_SRANDOM
#define srand(x)	srandom(x)
#define rand()		random()
#endif

#എങ്കിൽ defined(__linux__) && defined(__NR_gettid) && defined(HAVE_JRAND48)
#define DO_JRAND_MIX
THREAD_LOCAL unsigned short ul_jrand_seed[3];
#endif

സ്ഥിതം ശൂന്യം crank_random(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ i;
	ഘടന timeval tv;
	unsigned പൂർണ്ണസംഖ്യ n_pid, n_uid;

	gettimeofday(&tv, ശൂന്യ);
	n_pid = getpid();
	n_uid = getuid();
	srand((n_pid << 16) ^ n_uid ^ tv.tv_sec ^ tv.tv_usec);

#ifdef DO_JRAND_MIX
	ul_jrand_seed[0] = getpid() ^ (tv.tv_sec & 0xFFFF);
	ul_jrand_seed[1] = getppid() ^ (tv.tv_usec & 0xFFFF);
	ul_jrand_seed[2] = (tv.tv_sec ^ tv.tv_usec) >> 16;
#endif
	/* Crank the random number generator a few times */
	gettimeofday(&tv, ശൂന്യ);
	വേണ്ടി (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		rand();
}

സ്ഥിതം പൂർണ്ണസംഖ്യ random_get_fd(ul_random_src_t *src)
{
	പൂർണ്ണസംഖ്യ fd;

	*src = UL_RAND_URANDOM;
	fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	എങ്കിൽ (fd == -1) {
		*src = UL_RAND_RANDOM;
		fd = open("/dev/random", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	crank_random();
	തിരികെ fd;
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
ul_random_src_t ul_random_get_bytes(ശൂന്യം *buf, size_t nbytes)
{
	unsigned അക്ഷരം *cp = (unsigned അക്ഷരം *)buf;
	size_t i, n = nbytes;
	ul_random_src_t src;
	പൂർണ്ണസംഖ്യ lose_counter = 0;

#ifdef HAVE_GETRANDOM
	ആകുമ്പോൾ (n > 0) {
		പൂർണ്ണസംഖ്യ x;

		errno = 0;
		x = getrandom(cp, n, GRND_NONBLOCK);
		എങ്കിൽ (x > 0) {			/* success */
		       n -= x;
		       cp += x;
		       lose_counter = 0;
		       errno = 0;
		} അല്ലെങ്കിൽ എങ്കിൽ (errno == ENOSYS) {	/* kernel without getrandom() */
			നിർത്തുക;

		} അല്ലെങ്കിൽ എങ്കിൽ (errno == EAGAIN && lose_counter < UL_RAND_READ_ATTEMPTS) {
			xusleep(UL_RAND_READ_DELAY);	/* no entropy, wait and try again */
			lose_counter++;
		} അല്ലെങ്കിൽ
			നിർത്തുക;
	}
	എങ്കിൽ (n == 0)
		തിരികെ UL_RAND_GETRANDOM;

	എങ്കിൽ (errno == ENOSYS)
	/*
	 * We've been built against headers that support getrandom, but the
	 * running kernel does not.  Fallback to reading from /dev/{u,}random
	 * as before
	 */
#endif
	{
		പൂർണ്ണസംഖ്യ fd = random_get_fd(&src);

		lose_counter = 0;
		എങ്കിൽ (fd >= 0) {
			ആകുമ്പോൾ (n > 0) {
				ssize_t x = പഠ(fd, cp, n);
				എങ്കിൽ (x <= 0) {
					എങ്കിൽ (lose_counter++ > UL_RAND_READ_ATTEMPTS)
						നിർത്തുക;
					xusleep(UL_RAND_READ_DELAY);
					തുടരുക;
				}
				n -= x;
				cp += x;
				lose_counter = 0;
			}

			close(fd);
		}
		എങ്കിൽ (n == 0)
			തിരികെ src;
	}
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	crank_random();
	വേണ്ടി (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (rand() >> 7) & 0xFF;

#ifdef DO_JRAND_MIX
	{
		unsigned short tmp_seed[3];

		memcpy(tmp_seed, ul_jrand_seed, വലുപ്പം(tmp_seed));
		ul_jrand_seed[2] = ul_jrand_seed[2] ^ syscall(__NR_gettid);
		വേണ്ടി (cp = buf, i = 0; i < nbytes; i++)
			*cp++ ^= (jrand48(tmp_seed) >> 7) & 0xFF;
		memcpy(ul_jrand_seed, tmp_seed,
		       വലുപ്പം(ul_jrand_seed)-വലുപ്പം(unsigned short));
	}
#endif

	തിരികെ UL_RAND_WEAK;
}


/*
 * Tell source of randomness.
 */
സ്ഥിരം അക്ഷരം *ul_random_tell_source(ul_random_src_t src)
{
	സ്ഥിരം അക്ഷരം *s;

	തിരഞ്ഞെടുപ്പ് (src) {
	അവസ്ഥ UL_RAND_GETRANDOM:
		s = _("getrandom() function");
		നിർത്തുക;
	അവസ്ഥ UL_RAND_RANDOM:
		s = "/dev/random";
		നിർത്തുക;
	അവസ്ഥ UL_RAND_URANDOM:
		s = "/dev/urandom";
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		s = _("libc pseudo-random functions");
		നിർത്തുക;
	}

	തിരികെ s;
}

#ifdef TEST_PROGRAM_RANDUTILS
#അന്തര്ഭാവ <inttypes.h>

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	size_t i, n;
	int64_t *vp, v;
	അക്ഷരം *buf;
	size_t bufsz;

	n = argc == 1 ? 16 : atoi(argv[1]);

	printf("Multiple random calls:\n");
	വേണ്ടി (i = 0; i < n; i++) {
		ul_random_get_bytes(&v, വലുപ്പം(v));
		printf("#%02zu: %25"PRId64"\n", i, v);
	}


	printf("One random call:\n");
	bufsz = n * വലുപ്പം(*vp);
	buf = malloc(bufsz);
	എങ്കിൽ (!buf)
		err(EXIT_FAILURE, "failed to allocate buffer");

	ul_random_get_bytes(buf, bufsz);
	വേണ്ടി (i = 0; i < n; i++) {
		vp = (int64_t *) (buf + (i * വലുപ്പം(*vp)));
		printf("#%02zu: %25"PRId64"\n", i, *vp);
	}

	തിരികെ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_RANDUTILS */
