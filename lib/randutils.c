/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * General purpose random utilities. Based on libuuid code.
 *
 * This code is free software; you can redistribute it and/or modify it under
 * the terms of the Modified BSD License. The complete text of the license is
 * available in the Documentation/licenses/COPYING.BSD-3-Clause file.
 */
#समावेश <stdio.h>
#समावेश <unistd.h>
#समावेश <fcntl.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <sys/time.h>
#अगर_घोषित HAVE_SYS_SYSCALL_H
#समावेश <sys/syscall.h>
#पूर्ण_अगर
#समावेश "c.h"
#समावेश "timeutils.h"
#समावेश "randutils.h"
#समावेश "nls.h"


#अगर_घोषित HAVE_GETRANDOM
# include <sys/random.h>
#या_अगर defined (__linux__)
# अगर !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#पूर्ण_अगर

#अगर !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
#घोषणा GRND_NONBLOCK 0x01
#घोषणा GRND_RANDOM 0x02
अटल पूर्णांक getrandom(व्योम *buf, माप_प्रकार buflen, अचिन्हित पूर्णांक flags)
{
	वापस (syscall(SYS_getrandom, buf, buflen, flags));
}
# define HAVE_GETRANDOM
#पूर्ण_अगर

#अगर_घोषित HAVE_SRANDOM
#घोषणा बेक्रम(x)	srandom(x)
#घोषणा अक्रम()		random()
#पूर्ण_अगर

#अगर defined(__linux__) && defined(__NR_gettid) && defined(HAVE_JRAND48)
#घोषणा DO_JRAND_MIX
THREAD_LOCAL अचिन्हित लघु ul_jrand_seed[3];
#पूर्ण_अगर

अटल व्योम crank_random(व्योम)
{
	पूर्णांक i;
	काष्ठा timeval tv;
	अचिन्हित पूर्णांक n_pid, n_uid;

	gettimeofday(&tv, शून्य);
	n_pid = getpid();
	n_uid = getuid();
	बेक्रम((n_pid << 16) ^ n_uid ^ tv.tv_sec ^ tv.tv_usec);

#अगर_घोषित DO_JRAND_MIX
	ul_jrand_seed[0] = getpid() ^ (tv.tv_sec & 0xFFFF);
	ul_jrand_seed[1] = getppid() ^ (tv.tv_usec & 0xFFFF);
	ul_jrand_seed[2] = (tv.tv_sec ^ tv.tv_usec) >> 16;
#पूर्ण_अगर
	/* Crank the random number generator a few times */
	gettimeofday(&tv, शून्य);
	क्रम (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
		अक्रम();
}

अटल पूर्णांक random_get_fd(ul_random_src_t *src)
{
	पूर्णांक fd;

	*src = UL_RAND_URANDOM;
	fd = खोलो("/dev/urandom", O_RDONLY | O_CLOEXEC);
	अगर (fd == -1) {
		*src = UL_RAND_RANDOM;
		fd = खोलो("/dev/random", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	}
	crank_random();
	वापस fd;
}

/*
 * Generate a stream of random nbytes into buf.
 * Use /dev/urandom if possible, and if not,
 * use glibc pseudo-random functions.
 */
#घोषणा UL_RAND_READ_ATTEMPTS	8
#घोषणा UL_RAND_READ_DELAY	125000	/* microseconds */

/*
 * Write @nbytes random bytes into @buf.
 *
 * Returns source of random bytes (0 for weak quality).
 */
ul_random_src_t ul_random_get_bytes(व्योम *buf, माप_प्रकार nbytes)
{
	अचिन्हित अक्षर *cp = (अचिन्हित अक्षर *)buf;
	माप_प्रकार i, n = nbytes;
	ul_random_src_t src;
	पूर्णांक lose_गणक = 0;

#अगर_घोषित HAVE_GETRANDOM
	जबतक (n > 0) {
		पूर्णांक x;

		त्रुटि_सं = 0;
		x = getrandom(cp, n, GRND_NONBLOCK);
		अगर (x > 0) {			/* success */
		       n -= x;
		       cp += x;
		       lose_गणक = 0;
		       त्रुटि_सं = 0;
		} अन्यथा अगर (त्रुटि_सं == ENOSYS) {	/* kernel without getrandom() */
			अवरोध;

		} अन्यथा अगर (त्रुटि_सं == EAGAIN && lose_गणक < UL_RAND_READ_ATTEMPTS) {
			xusleep(UL_RAND_READ_DELAY);	/* no entropy, wait and try again */
			lose_गणक++;
		} अन्यथा
			अवरोध;
	}
	अगर (n == 0)
		वापस UL_RAND_GETRANDOM;

	अगर (त्रुटि_सं == ENOSYS)
	/*
	 * We've been built against headers that support getrandom, but the
	 * running kernel does not.  Fallback to reading from /dev/{u,}random
	 * as before
	 */
#पूर्ण_अगर
	{
		पूर्णांक fd = random_get_fd(&src);

		lose_गणक = 0;
		अगर (fd >= 0) {
			जबतक (n > 0) {
				ssize_t x = पढ़ो(fd, cp, n);
				अगर (x <= 0) {
					अगर (lose_गणक++ > UL_RAND_READ_ATTEMPTS)
						अवरोध;
					xusleep(UL_RAND_READ_DELAY);
					जारी;
				}
				n -= x;
				cp += x;
				lose_गणक = 0;
			}

			बंद(fd);
		}
		अगर (n == 0)
			वापस src;
	}
	/*
	 * We do this all the time, but this is the only source of
	 * randomness if /dev/random/urandom is out to lunch.
	 */
	crank_random();
	क्रम (cp = buf, i = 0; i < nbytes; i++)
		*cp++ ^= (अक्रम() >> 7) & 0xFF;

#अगर_घोषित DO_JRAND_MIX
	{
		अचिन्हित लघु tmp_seed[3];

		स_नकल(tmp_seed, ul_jrand_seed, माप(tmp_seed));
		ul_jrand_seed[2] = ul_jrand_seed[2] ^ syscall(__NR_gettid);
		क्रम (cp = buf, i = 0; i < nbytes; i++)
			*cp++ ^= (jrand48(tmp_seed) >> 7) & 0xFF;
		स_नकल(ul_jrand_seed, tmp_seed,
		       माप(ul_jrand_seed)-माप(अचिन्हित लघु));
	}
#पूर्ण_अगर

	वापस UL_RAND_WEAK;
}


/*
 * Tell source of randomness.
 */
स्थिर अक्षर *ul_random_tell_source(ul_random_src_t src)
{
	स्थिर अक्षर *s;

	चयन (src) {
	हाल UL_RAND_GETRANDOM:
		s = _("getrandom() function");
		अवरोध;
	हाल UL_RAND_RANDOM:
		s = "/dev/random";
		अवरोध;
	हाल UL_RAND_URANDOM:
		s = "/dev/urandom";
		अवरोध;
	शेष:
		s = _("libc pseudo-random functions");
		अवरोध;
	}

	वापस s;
}

#अगर_घोषित TEST_PROGRAM_RANDUTILS
#समावेश <inttypes.h>

पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[])
{
	माप_प्रकार i, n;
	int64_t *vp, v;
	अक्षर *buf;
	माप_प्रकार bufsz;

	n = argc == 1 ? 16 : म_से_प(argv[1]);

	म_लिखो("Multiple random calls:\n");
	क्रम (i = 0; i < n; i++) {
		ul_random_get_bytes(&v, माप(v));
		म_लिखो("#%02zu: %25"PRId64"\n", i, v);
	}


	म_लिखो("One random call:\n");
	bufsz = n * माप(*vp);
	buf = दो_स्मृति(bufsz);
	अगर (!buf)
		err(निकास_त्रुटि, "failed to allocate buffer");

	ul_random_get_bytes(buf, bufsz);
	क्रम (i = 0; i < n; i++) {
		vp = (int64_t *) (buf + (i * माप(*vp)));
		म_लिखो("#%02zu: %25"PRId64"\n", i, *vp);
	}

	वापस निकास_सफल;
}
#पूर्ण_अगर /* TEST_PROGRAM_RANDUTILS */
