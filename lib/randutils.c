/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * General purpose random utilities. Based on libuuid code.
 *
 * This code is free software; you can redistribute it and/or modify it under
 * the terms of the Modified BSD License. The complete text of the license is
 * available in the Documentation/licenses/COPYING.BSD-3-Clause file.
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/time.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_SYS_SYSCALL_H
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/syscall.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "timeutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "randutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "nls.h"


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_GETRANDOM
# include <sys/random.h>
#ਜਾਂਜੇ defined (__linux__)
# ਜੇ !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#ਜੇ_ਅੰਤ

#ਜੇ !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GRND_NONBLOCK 0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GRND_RANDOM 0x02
ਸਥਿਰ ਪੂਰਨ getrandom(ਖਾਲੀ *buf, size_t buflen, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flags)
{
	ਵਾਪਸ (syscall(SYS_getrandom, buf, buflen, flags));
}
# define HAVE_GETRANDOM
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_SRANDOM
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ srand(x)	srandom(x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ rand()		random()
#ਜੇ_ਅੰਤ

#ਜੇ defined(__linux__) && defined(__NR_gettid) && defined(HAVE_JRAND48)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DO_JRAND_MIX
THREAD_LOCAL ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ul_jrand_ਬੀਜ[3];
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਖਾਲੀ crank_random(ਖਾਲੀ)
{
	ਪੂਰਨ i;
	ਬਣਤਰ timeval tv;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ n_pid, n_uid;

	gettimeofday(&tv, ਨਿਰਾ);
	n_pid = getpid();
	n_uid = getuid();
	srand((n_pid << 16) ^ n_uid ^ tv.tv_sec ^ tv.tv_usec);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DO_JRAND_MIX
	ul_jrand_ਬੀਜ[0] = getpid() ^ (tv.tv_sec & 0xFFFF);
	ul_jrand_ਬੀਜ[1] = getppid() ^ (tv.tv_usec & 0xFFFF);
	ul_jrand_ਬੀਜ[2] = (tv.tv_sec ^ tv.tv_usec) >> 16;
#ਜੇ_ਅੰਤ
	/* Crank the random number generator a few times */
	gettimeofday(&tv, ਨਿਰਾ);
	ਲਈ (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		rand();
}

ਸਥਿਰ ਪੂਰਨ random_get_fd(ul_random_src_t *src)
{
	ਪੂਰਨ fd;

	*src = UL_RAND_URANDOM;
	fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	ਜੇ (fd == -1) {
		*src = UL_RAND_RANDOM;
		fd = open("/dev/random", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	crank_random();
	ਵਾਪਸ fd;
}

/*
 * Generate a stream of random nbytes into buf.
 * Use /dev/urandom if possible, and if not,
 * use glibc pseudo-random functions.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UL_RAND_READ_ATTEMPTS	8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UL_RAND_READ_DELAY	125000	/* microseconds */

/*
 * Write @nbytes random bytes into @buf.
 *
 * Returns source of random bytes (0 for weak quality).
 */
ul_random_src_t ul_random_get_ਬਾਈਟਾਂ(ਖਾਲੀ *buf, size_t nbytes)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *cp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)buf;
	size_t i, n = nbytes;
	ul_random_src_t src;
	ਪੂਰਨ lose_counter = 0;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_GETRANDOM
	ਜਦੋਂ (n > 0) {
		ਪੂਰਨ x;

		errno = 0;
		x = getrandom(cp, n, GRND_NONBLOCK);
		ਜੇ (x > 0) {			/* success */
		       n -= x;
		       cp += x;
		       lose_counter = 0;
		       errno = 0;
		} ਨਹੀਂਤਾਂ ਜੇ (errno == ENOSYS) {	/* kernel without getrandom() */
			ਰੋਕੋ;

		} ਨਹੀਂਤਾਂ ਜੇ (errno == EAGAIN && lose_counter < UL_RAND_READ_ATTEMPTS) {
			xusleep(UL_RAND_READ_DELAY);	/* no entropy, wait and try again */
			lose_counter++;
		} ਨਹੀਂਤਾਂ
			ਰੋਕੋ;
	}
	ਜੇ (n == 0)
		ਵਾਪਸ UL_RAND_GETRANDOM;

	ਜੇ (errno == ENOSYS)
	/*
	 * We've been built against headers that support getrandom, but the
	 * running kernel does not.  Fallback to reading from /dev/{u,}random
	 * as before
	 */
#ਜੇ_ਅੰਤ
	{
		ਪੂਰਨ fd = random_get_fd(&src);

		lose_counter = 0;
		ਜੇ (fd >= 0) {
			ਜਦੋਂ (n > 0) {
				ssize_t x = ਪਠ(fd, cp, n);
				ਜੇ (x <= 0) {
					ਜੇ (lose_counter++ > UL_RAND_READ_ATTEMPTS)
						ਰੋਕੋ;
					xusleep(UL_RAND_READ_DELAY);
					ਜਾਰੀ;
				}
				n -= x;
				cp += x;
				lose_counter = 0;
			}

			close(fd);
		}
		ਜੇ (n == 0)
			ਵਾਪਸ src;
	}
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	crank_random();
	ਲਈ (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (rand() >> 7) & 0xFF;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DO_JRAND_MIX
	{
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ tmp_ਬੀਜ[3];

		ਸਮ੍ਰਿਤੀ_ਨਕਲ(tmp_ਬੀਜ, ul_jrand_ਬੀਜ, ਆਕਾਰ(tmp_ਬੀਜ));
		ul_jrand_ਬੀਜ[2] = ul_jrand_ਬੀਜ[2] ^ syscall(__NR_gettid);
		ਲਈ (cp = buf, i = 0; i < nbytes; i++)
			*cp++ ^= (jrand48(tmp_ਬੀਜ) >> 7) & 0xFF;
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(ul_jrand_ਬੀਜ, tmp_ਬੀਜ,
		       ਆਕਾਰ(ul_jrand_ਬੀਜ)-ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ));
	}
#ਜੇ_ਅੰਤ

	ਵਾਪਸ UL_RAND_WEAK;
}


/*
 * Tell source of randomness.
 */
ਅਚਲ ਅੱਖਰ *ul_random_tell_ਸਰੋਤ(ul_random_src_t src)
{
	ਅਚਲ ਅੱਖਰ *s;

	ਚੋਣ (src) {
	ਹਾਲਤ UL_RAND_GETRANDOM:
		s = _("getrandom() function");
		ਰੋਕੋ;
	ਹਾਲਤ UL_RAND_RANDOM:
		s = "/dev/random";
		ਰੋਕੋ;
	ਹਾਲਤ UL_RAND_URANDOM:
		s = "/dev/urandom";
		ਰੋਕੋ;
	ਮੂਲ:
		s = _("libc pseudo-random functions");
		ਰੋਕੋ;
	}

	ਵਾਪਸ s;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_RANDUTILS
#ਅਨ੍ਤਰ੍ਭਾਵ <inttypes.h>

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	size_t i, n;
	int64_t *vp, v;
	ਅੱਖਰ *buf;
	size_t bufsz;

	n = argc == 1 ? 16 : atoi(argv[1]);

	ਲਿਖੋ("Multiple random calls:\n");
	ਲਈ (i = 0; i < n; i++) {
		ul_random_get_ਬਾਈਟਾਂ(&v, ਆਕਾਰ(v));
		ਲਿਖੋ("#%02zu: %25"PRId64"\n", i, v);
	}


	ਲਿਖੋ("One random call:\n");
	bufsz = n * ਆਕਾਰ(*vp);
	buf = ਸਮ੍ਰਿਤੀ_ਦਿਓ(bufsz);
	ਜੇ (!buf)
		err(EXIT_FAILURE, "failed to allocate buffer");

	ul_random_get_ਬਾਈਟਾਂ(buf, bufsz);
	ਲਈ (i = 0; i < n; i++) {
		vp = (int64_t *) (buf + (i * ਆਕਾਰ(*vp)));
		ਲਿਖੋ("#%02zu: %25"PRId64"\n", i, *vp);
	}

	ਵਾਪਸ EXIT_SUCCESS;
}
#ਜੇ_ਅੰਤ /* TEST_PROGRAM_RANDUTILS */
