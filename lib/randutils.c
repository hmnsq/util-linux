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
#अथवायदि defined (__linux__)
# यदि !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#endif

#यदि !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
#define GRND_NONBLOCK 0x01
#define GRND_RANDOM 0x02
स्थिर पूर्णाङ्क getrandom(शून्य *buf, माप_प्रकार buflen, अचिन्हित पूर्णाङ्क flags)
{
	फर्काऊ (syscall(SYS_getrandom, buf, buflen, flags));
}
# define HAVE_GETRANDOM
#endif

#ifdef HAVE_SRANDOM
#define बेक्रम(x)	srandom(x)
#define अक्रम()		random()
#endif

#यदि defined(__linux__) && defined(__NR_gettid) && defined(HAVE_JRAND48)
#define DO_JRAND_MIX
THREAD_LOCAL अचिन्हित लघु ul_jrand_seed[3];
#endif

स्थिर शून्य crank_random(शून्य)
{
	पूर्णाङ्क i;
	संरचना timeval tv;
	अचिन्हित पूर्णाङ्क n_pid, n_uid;

	gettimeofday(&tv, रिक्त);
	n_pid = getpid();
	n_uid = getuid();
	बेक्रम((n_pid << 16) ^ n_uid ^ tv.tv_sec ^ tv.tv_usec);

#ifdef DO_JRAND_MIX
	ul_jrand_seed[0] = getpid() ^ (tv.tv_sec & 0xFFFF);
	ul_jrand_seed[1] = getppid() ^ (tv.tv_usec & 0xFFFF);
	ul_jrand_seed[2] = (tv.tv_sec ^ tv.tv_usec) >> 16;
#endif
	/* Crank the random number generator a few times */
	gettimeofday(&tv, रिक्त);
	लागि (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		अक्रम();
}

स्थिर पूर्णाङ्क random_get_fd(ul_random_src_t *src)
{
	पूर्णाङ्क fd;

	*src = UL_RAND_URANDOM;
	fd = खोलो("/dev/urandom", O_RDONLY | O_CLOEXEC);
	यदि (fd == -1) {
		*src = UL_RAND_RANDOM;
		fd = खोलो("/dev/random", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	crank_random();
	फर्काऊ fd;
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
ul_random_src_t ul_random_get_bytes(शून्य *buf, माप_प्रकार nbytes)
{
	अचिन्हित वर्ण *cp = (अचिन्हित वर्ण *)buf;
	माप_प्रकार i, n = nbytes;
	ul_random_src_t src;
	पूर्णाङ्क lose_counter = 0;

#ifdef HAVE_GETRANDOM
	जबसम्म (n > 0) {
		पूर्णाङ्क x;

		त्रुटि_सं = 0;
		x = getrandom(cp, n, GRND_NONBLOCK);
		यदि (x > 0) {			/* success */
		       n -= x;
		       cp += x;
		       lose_counter = 0;
		       त्रुटि_सं = 0;
		} अन्यथा यदि (त्रुटि_सं == ENOSYS) {	/* kernel without getrandom() */
			रोक;

		} अन्यथा यदि (त्रुटि_सं == EAGAIN && lose_counter < UL_RAND_READ_ATTEMPTS) {
			xusleep(UL_RAND_READ_DELAY);	/* no entropy, wait and try again */
			lose_counter++;
		} अन्यथा
			रोक;
	}
	यदि (n == 0)
		फर्काऊ UL_RAND_GETRANDOM;

	यदि (त्रुटि_सं == ENOSYS)
	/*
	 * We've been built against headers that support getrandom, but the
	 * running kernel does not.  Fallback to reading from /dev/{u,}random
	 * as before
	 */
#endif
	{
		पूर्णाङ्क fd = random_get_fd(&src);

		lose_counter = 0;
		यदि (fd >= 0) {
			जबसम्म (n > 0) {
				ssize_t x = पठ(fd, cp, n);
				यदि (x <= 0) {
					यदि (lose_counter++ > UL_RAND_READ_ATTEMPTS)
						रोक;
					xusleep(UL_RAND_READ_DELAY);
					जारी;
				}
				n -= x;
				cp += x;
				lose_counter = 0;
			}

			बंद(fd);
		}
		यदि (n == 0)
			फर्काऊ src;
	}
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	crank_random();
	लागि (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (अक्रम() >> 7) & 0xFF;

#ifdef DO_JRAND_MIX
	{
		अचिन्हित लघु tmp_seed[3];

		स_नकल(tmp_seed, ul_jrand_seed, परिमाण(tmp_seed));
		ul_jrand_seed[2] = ul_jrand_seed[2] ^ syscall(__NR_gettid);
		लागि (cp = buf, i = 0; i < nbytes; i++)
			*cp++ ^= (jrand48(tmp_seed) >> 7) & 0xFF;
		स_नकल(ul_jrand_seed, tmp_seed,
		       परिमाण(ul_jrand_seed)-परिमाण(अचिन्हित लघु));
	}
#endif

	फर्काऊ UL_RAND_WEAK;
}


/*
 * Tell source of randomness.
 */
अचर वर्ण *ul_random_tell_source(ul_random_src_t src)
{
	अचर वर्ण *s;

	विकल्प (src) {
	पक्ष UL_RAND_GETRANDOM:
		s = _("getrandom() function");
		रोक;
	पक्ष UL_RAND_RANDOM:
		s = "/dev/random";
		रोक;
	पक्ष UL_RAND_URANDOM:
		s = "/dev/urandom";
		रोक;
	स्वभाव:
		s = _("libc pseudo-random functions");
		रोक;
	}

	फर्काऊ s;
}

#ifdef TEST_PROGRAM_RANDUTILS
#अन्तर्भाव <inttypes.h>

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	माप_प्रकार i, n;
	int64_t *vp, v;
	वर्ण *buf;
	माप_प्रकार bufsz;

	n = argc == 1 ? 16 : म_से_प(argv[1]);

	म_लिखो("Multiple random calls:\n");
	लागि (i = 0; i < n; i++) {
		ul_random_get_bytes(&v, परिमाण(v));
		म_लिखो("#%02zu: %25"PRId64"\n", i, v);
	}


	म_लिखो("One random call:\n");
	bufsz = n * परिमाण(*vp);
	buf = दो_स्मृति(bufsz);
	यदि (!buf)
		err(निकास_त्रुटि, "failed to allocate buffer");

	ul_random_get_bytes(buf, bufsz);
	लागि (i = 0; i < n; i++) {
		vp = (int64_t *) (buf + (i * परिमाण(*vp)));
		म_लिखो("#%02zu: %25"PRId64"\n", i, *vp);
	}

	फर्काऊ निकास_सफल;
}
#endif /* TEST_PROGRAM_RANDUTILS */
