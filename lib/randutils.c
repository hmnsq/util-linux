/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * General purpose random utilities. Based on libuuid code.
 *
 * This code is free software; you can redistribute it and/or modify it under
 * the terms of the Modified BSD License. The complete text of the license is
 * available in the Documentation/licenses/COPYING.BSD-3-Clause file.
 */
#شامل <stdio.h>
#شامل <unistd.h>
#شامل <fcntl.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <sys/time.h>
#اگر_تعریف_شدہ HAVE_SYS_SYSCALL_H
#شامل <sys/syscall.h>
#اگر_ختم
#شامل "c.h"
#شامل "timeutils.h"
#شامل "randutils.h"
#شامل "nls.h"


#اگر_تعریف_شدہ HAVE_GETRANDOM
# include <sys/random.h>
#وگرنہ_اگر defined (__linux__)
# اگر !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#اگر_ختم

#اگر !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
#تعریف_کرو GRND_NONBLOCK 0x01
#تعریف_کرو GRND_RANDOM 0x02
ساکن صحیح getrandom(خالی *buf, size_t buflen, غیرعلامتی صحیح flags)
{
	واپس (syscall(SYS_getrandom, buf, buflen, flags));
}
# define HAVE_GETRANDOM
#اگر_ختم

#اگر_تعریف_شدہ HAVE_SRANDOM
#تعریف_کرو srand(x)	srandom(x)
#تعریف_کرو rand()		random()
#اگر_ختم

#اگر defined(__linux__) && defined(__NR_gettid) && defined(HAVE_JRAND48)
#تعریف_کرو DO_JRAND_MIX
THREAD_LOCAL غیرعلامتی مختصر ul_jrand_بیج[3];
#اگر_ختم

ساکن خالی crank_random(خالی)
{
	صحیح i;
	ساخت timeval tv;
	غیرعلامتی صحیح n_pid, n_uid;

	gettimeofday(&tv, تہی);
	n_pid = getpid();
	n_uid = getuid();
	srand((n_pid << 16) ^ n_uid ^ tv.tv_sec ^ tv.tv_usec);

#اگر_تعریف_شدہ DO_JRAND_MIX
	ul_jrand_بیج[0] = getpid() ^ (tv.tv_sec & 0xFFFF);
	ul_jrand_بیج[1] = getppid() ^ (tv.tv_usec & 0xFFFF);
	ul_jrand_بیج[2] = (tv.tv_sec ^ tv.tv_usec) >> 16;
#اگر_ختم
	/* Crank the random number generator a few times */
	gettimeofday(&tv, تہی);
	برائے (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		rand();
}

ساکن صحیح random_get_fd(ul_random_src_t *src)
{
	صحیح fd;

	*src = UL_RAND_URANDOM;
	fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	اگر (fd == -1) {
		*src = UL_RAND_RANDOM;
		fd = open("/dev/random", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	crank_random();
	واپس fd;
}

/*
 * Generate a stream of random nbytes into buf.
 * Use /dev/urandom if possible, and if not,
 * use glibc pseudo-random functions.
 */
#تعریف_کرو UL_RAND_READ_ATTEMPTS	8
#تعریف_کرو UL_RAND_READ_DELAY	125000	/* microseconds */

/*
 * Write @nbytes random bytes into @buf.
 *
 * Returns source of random bytes (0 for weak quality).
 */
ul_random_src_t ul_random_get_بائٹیں(خالی *buf, size_t nbytes)
{
	غیرعلامتی حرف *cp = (غیرعلامتی حرف *)buf;
	size_t i, n = nbytes;
	ul_random_src_t src;
	صحیح lose_شمار = 0;

#اگر_تعریف_شدہ HAVE_GETRANDOM
	جب (n > 0) {
		صحیح x;

		errno = 0;
		x = getrandom(cp, n, GRND_NONBLOCK);
		اگر (x > 0) {			/* success */
		       n -= x;
		       cp += x;
		       lose_شمار = 0;
		       errno = 0;
		} ورنہ اگر (errno == ENOSYS) {	/* kernel without getrandom() */
			توڑو;

		} ورنہ اگر (errno == EAGAIN && lose_شمار < UL_RAND_READ_ATTEMPTS) {
			xusleep(UL_RAND_READ_DELAY);	/* no entropy, wait and try again */
			lose_شمار++;
		} ورنہ
			توڑو;
	}
	اگر (n == 0)
		واپس UL_RAND_GETRANDOM;

	اگر (errno == ENOSYS)
	/*
	 * We've been built against headers that support getrandom, but the
	 * running kernel does not.  Fallback to reading from /dev/{u,}random
	 * as before
	 */
#اگر_ختم
	{
		صحیح fd = random_get_fd(&src);

		lose_شمار = 0;
		اگر (fd >= 0) {
			جب (n > 0) {
				ssize_t x = اقرأ(fd, cp, n);
				اگر (x <= 0) {
					اگر (lose_شمار++ > UL_RAND_READ_ATTEMPTS)
						توڑو;
					xusleep(UL_RAND_READ_DELAY);
					جاری;
				}
				n -= x;
				cp += x;
				lose_شمار = 0;
			}

			close(fd);
		}
		اگر (n == 0)
			واپس src;
	}
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	crank_random();
	برائے (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (rand() >> 7) & 0xFF;

#اگر_تعریف_شدہ DO_JRAND_MIX
	{
		غیرعلامتی مختصر tmp_بیج[3];

		حافظہ_نقل(tmp_بیج, ul_jrand_بیج, حجم(tmp_بیج));
		ul_jrand_بیج[2] = ul_jrand_بیج[2] ^ syscall(__NR_gettid);
		برائے (cp = buf, i = 0; i < nbytes; i++)
			*cp++ ^= (jrand48(tmp_بیج) >> 7) & 0xFF;
		حافظہ_نقل(ul_jrand_بیج, tmp_بیج,
		       حجم(ul_jrand_بیج)-حجم(غیرعلامتی مختصر));
	}
#اگر_ختم

	واپس UL_RAND_WEAK;
}


/*
 * Tell source of randomness.
 */
ثابت حرف *ul_random_tell_ماخذ(ul_random_src_t src)
{
	ثابت حرف *s;

	انتخاب (src) {
	حالت UL_RAND_GETRANDOM:
		s = _("getrandom() function");
		توڑو;
	حالت UL_RAND_RANDOM:
		s = "/dev/random";
		توڑو;
	حالت UL_RAND_URANDOM:
		s = "/dev/urandom";
		توڑو;
	طےشدہ:
		s = _("libc pseudo-random functions");
		توڑو;
	}

	واپس s;
}

#اگر_تعریف_شدہ TEST_پروگرام_RANDUTILS
#شامل <inttypes.h>

صحیح مین(صحیح argc, حرف *argv[])
{
	size_t i, n;
	int64_t *vp, v;
	حرف *buf;
	size_t bufsz;

	n = argc == 1 ? 16 : atoi(argv[1]);

	لکھو("Multiple random calls:\n");
	برائے (i = 0; i < n; i++) {
		ul_random_get_بائٹیں(&v, حجم(v));
		لکھو("#%02zu: %25"PRId64"\n", i, v);
	}


	لکھو("One random call:\n");
	bufsz = n * حجم(*vp);
	buf = حافظہ_دو(bufsz);
	اگر (!buf)
		err(EXIT_FAILURE, "failed to allocate buffer");

	ul_random_get_بائٹیں(buf, bufsz);
	برائے (i = 0; i < n; i++) {
		vp = (int64_t *) (buf + (i * حجم(*vp)));
		لکھو("#%02zu: %25"PRId64"\n", i, *vp);
	}

	واپس EXIT_SUCCESS;
}
#اگر_ختم /* TEST_PROGRAM_RANDUTILS */
