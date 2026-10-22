/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Karel Zak <kzak@redhat.com> [2010]
 *          Davidlohr Bueso <dave@gnu.org> [2010]
 */
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <inttypes.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>
#अन्तर्भाव <assert.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "cctype.h"
#अन्तर्भाव "nls.h"
#अन्तर्भाव "strutils.h"
#अन्तर्भाव "timeutils.h"
#अन्तर्भाव "bitops.h"
#अन्तर्भाव "pathnames.h"

स्थिर पूर्णांक STRTOXX_EXIT_CODE = निकास_त्रुटि;

शून्य strutils_set_exitcode(पूर्णांक ex)
{
	STRTOXX_EXIT_CODE = ex;
}

स्थिर पूर्णांक do_scale_by_power (uintmax_t *x, पूर्णांक base, पूर्णांक power)
{
	पर्यंत (power--) {
		जर (UINTMAX_MAX / base < *x)
			परत -दुस्फल;
		*x *= base;
	}
	परत 0;
}

/*
 * strtosize() - convert string to size (uintmax_t).
 *
 * Supported suffixes:
 *
 * XiB or X for 2^N
 *     where X = {K,M,G,T,P,E,Z,Y}
 *        or X = {k,m,g,t,p,e}  (undocumented for backward compatibility only)
 * for example:
 *		10KiB	= 10240
 *		10K	= 10240
 *
 * XB for 10^N
 *     where X = {K,M,G,T,P,E,Z,Y}
 * for example:
 *		10KB	= 10000
 *
 * The optional 'power' variable returns number associated with used suffix
 * {K,M,G,T,P,E,Z,Y}  = {1,2,3,4,5,6,7,8}.
 *
 * The function also supports decimal point, for example:
 *              0.5MB   = 500000
 *              0.5MiB  = 524288
 *
 * Note that the function does not accept numbers with '-' (negative sign)
 * prefix.
 */
पूर्णांक ul_parse_size(स्थिरांक अक्षर *str, uintmax_t *res, पूर्णांक *power)
{
	स्थिरांक अक्षर *p;
	अक्षर *end;
	uintmax_t x, frac = 0;
	पूर्णांक base = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	स्थिर स्थिरांक अक्षर *suf  = "KMGTPEZY";
	स्थिर स्थिरांक अक्षर *suf2 = "kmgtpezy";
	स्थिरांक अक्षर *sp;

	*res = 0;

	जर (!str || !*str) {
		rc = -EINVAL;
		जा err;
	}

	/* Only positive numbers are acceptable
	 *
	 * Note that this check is not perfect, it would be better to
	 * use lconv->negative_sign. But coreutils use the same solution,
	 * so it's probably good enough...
	 */
	p = str;
	पर्यंत (है_खाली((अचिन्हित अक्षर) *p))
		p++;
	जर (*p == '-') {
		rc = -EINVAL;
		जा err;
	}

	त्रुटि_सं = 0, end = रिक्त;
	x = strtoumax(str, &end, 0);

	जर (end == str ||
	    (त्रुटि_सं != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = त्रुटि_सं ? -त्रुटि_सं : -EINVAL;
		जा err;
	}
	जर (!end || !*end)
		जा done;			/* without suffix */
	p = end;

	/*
	 * Check size suffixes
	 */
check_suffix:
	जर (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		base = 1024;			/* XiB, 2^N */
	अन्यथा जर ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		base = 1000;			/* XB, 10^N */
	अन्यथा जर (*(p + 1)) {
		संरचना स्था_बदल स्थिरांक *l = क्षेत्र_बदलो();
		स्थिरांक अक्षर *dp = l ? l->decimal_point : रिक्त;
		माप_प्रकार dpsz = dp ? म_माप(dp) : 0;

		जर (frac == 0 && *p && dp && म_भेदन(dp, p, dpsz) == 0) {
			स्थिरांक अक्षर *fstr = p + dpsz;

			साठी (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			जर (है_अंक(*fstr)) {
				त्रुटि_सं = 0, end = रिक्त;
				frac = strtoumax(fstr, &end, 0);
				जर (end == fstr ||
				    (त्रुटि_सं != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = त्रुटि_सं ? -त्रुटि_सं : -EINVAL;
					जा err;
				}
			} अन्यथा
				end = (अक्षर *) p;

			जर (!end || !*end) {
				rc = -EINVAL;
				जा err;	/* without suffix, but with fractions */
			}
			p = end;
			जा check_suffix;
		}
		rc = -EINVAL;
		जा err;			/* unexpected suffix */
	}

	sp = म_अक्षर(suf, *p);
	जर (sp)
		pwr = (sp - suf) + 1;
	अन्यथा {
		sp = म_अक्षर(suf2, *p);
		जर (sp)
			pwr = (sp - suf2) + 1;
		अन्यथा {
			rc = -EINVAL;
			जा err;
		}
	}

	जर (power)
		*power = pwr;

	rc = do_scale_by_power(&x, base, pwr);
	जर (rc)
		जा err;

	जर (frac && pwr) {
		पूर्णांक i;
		uintmax_t frac_base = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_base, base, pwr);
		जर (rc)
			जा err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		करा {
			frac_val = ((frac % 10) * frac_base + frac_val) / 10;
			frac /= 10;
		} पर्यंत (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		साठी (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		जर (UINTMAX_MAX - x < frac_val) {
			rc = -दुस्फल;
			जा err;
		}
		x += frac_val;
	}
done:
	*res = x;
err:
	जर (rc < 0)
		त्रुटि_सं = -rc;
	परत rc;
}

पूर्णांक strtosize(स्थिरांक अक्षर *str, uintmax_t *res)
{
	परत ul_parse_size(str, res, रिक्त);
}

पूर्णांक isdigit_strend(स्थिरांक अक्षर *str, स्थिरांक अक्षर **end)
{
	स्थिरांक अक्षर *p;

	साठी (p = str; p && *p && है_अंक((अचिन्हित अक्षर) *p); p++);

	जर (end)
		*end = p;
	परत p && p > str && !*p;
}

पूर्णांक isxdigit_strend(स्थिरांक अक्षर *str, स्थिरांक अक्षर **end)
{
	स्थिरांक अक्षर *p;

	साठी (p = str; p && *p && है_षष्ठादशक((अचिन्हित अक्षर) *p); p++);

	जर (end)
		*end = p;

	परत p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
पूर्णांक ul_strtobool(स्थिरांक अक्षर *str, bool *result)
{
	स्थिर स्थिरांक अक्षर *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	स्थिर स्थिरांक अक्षर *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	माप_प्रकार i;

	जर (!str || !result)
		परत -(त्रुटि_सं = EINVAL);

	त्रुटि_सं = 0;

	साठी (i = 0; i < ARRAY_SIZE(bool_true); i++) {
		जर (c_strcasecmp(str, bool_true[i]) == 0) {
			*result = true;
			परत 0;
		}
	}

	साठी (i = 0; i < ARRAY_SIZE(bool_false); i++) {
		जर (c_strcasecmp(str, bool_false[i]) == 0) {
			*result = false;
			परत 0;
		}
	}

	परत -(त्रुटि_सं = EINVAL);
}

पूर्णांक ul_parse_switch(स्थिरांक अक्षर *arg, ...)
{
	स्थिरांक अक्षर *a, *b;
	बहु_सूची ap;

	बहु_शुरू(ap, arg);
	करा {
		a = बहु_तर्क(ap, अक्षर *);
		जर (!a)
			थांबा;
		b = बहु_तर्क(ap, अक्षर *);
		जर (!b)
			थांबा;

		जर (म_भेद(arg, a) == 0) {
			बहु_पूर्ण(ap);
			परत 1;
		}

		जर (म_भेद(arg, b) == 0) {
			बहु_पूर्ण(ap);
			परत 0;
		}
	} पर्यंत (1);
	बहु_पूर्ण(ap);

	errx(STRTOXX_EXIT_CODE, _("unsupported argument: %s"), arg);
}

#ifndef HAVE_MEMPCPY
शून्य *mempcpy(शून्य *restrict dest, स्थिरांक शून्य *restrict src, माप_प्रकार n)
{
	परत ((अक्षर *)स_नकल(dest, src, n)) + n;
}
#endif

#ifndef HAVE_STRNLEN
माप_प्रकार strnlen(स्थिरांक अक्षर *s, माप_प्रकार maxlen)
{
	माप_प्रकार i;

	साठी (i = 0; i < maxlen; i++) {
		जर (s[i] == '\0')
			परत i;
	}
	परत maxlen;
}
#endif

#ifndef HAVE_STRNCHR
अक्षर *strnchr(स्थिरांक अक्षर *s, माप_प्रकार maxlen, पूर्णांक c)
{
	साठी (; maxlen-- && *s != '\0'; ++s)
		जर (*s == (अक्षर)c)
			परत (अक्षर *)s;
	परत रिक्त;
}
#endif

#ifndef HAVE_STRNDUP
अक्षर *strndup(स्थिरांक अक्षर *s, माप_प्रकार n)
{
	माप_प्रकार len = strnlen(s, n);
	अक्षर *new = दो_स्मृति((len + 1) * आकार(अक्षर));
	जर (!new)
		परत रिक्त;
	new[len] = '\0';
	परत (अक्षर *) स_नकल(new, s, len);
}
#endif

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
पूर्णांक ul_strtos64(स्थिरांक अक्षर *str, int64_t *अंक, पूर्णांक base)
{
	अक्षर *end = रिक्त;

	जर (str == रिक्त || *str == '\0')
		परत -(त्रुटि_सं = EINVAL);

	त्रुटि_सं = 0;
	*अंक = (int64_t) strtoimax(str, &end, base);

	जर (त्रुटि_सं != 0)
		परत -त्रुटि_सं;
	जर (str == end || (end && *end))
		परत -(त्रुटि_सं = EINVAL);
	परत 0;
}

पूर्णांक ul_strtou64(स्थिरांक अक्षर *str, uint64_t *अंक, पूर्णांक base)
{
	अक्षर *end = रिक्त;
	int64_t tmp;

	जर (str == रिक्त || *str == '\0')
		परत -(त्रुटि_सं = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	त्रुटि_सं = 0;
	tmp = (int64_t) strtoimax(str, &end, base);
	जर (tmp < 0)
		त्रुटि_सं = दुस्फल;
	अन्यथा {
		त्रुटि_सं = 0;
		*अंक = strtoumax(str, &end, base);
	}

	जर (त्रुटि_सं != 0)
		परत -त्रुटि_सं;
	जर (str == end || (end && *end))
		परत -(त्रुटि_सं = EINVAL);
	परत 0;
}

पूर्णांक ul_strtos32(स्थिरांक अक्षर *str, int32_t *अंक, पूर्णांक base)
{
	int64_t tmp;
	पूर्णांक rc;

	rc = ul_strtos64(str, &tmp, base);
	जर (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(त्रुटि_सं = दुस्फल);
	जर (rc == 0)
		*अंक = (int32_t) tmp;
	परत rc;
}

पूर्णांक ul_strtou32(स्थिरांक अक्षर *str, uint32_t *अंक, पूर्णांक base)
{
	uint64_t tmp;
	पूर्णांक rc;

	rc = ul_strtou64(str, &tmp, base);
	जर (rc == 0 && tmp > UINT32_MAX)
		rc = -(त्रुटि_सं = दुस्फल);
	जर (rc == 0)
		*अंक = (uint32_t) tmp;
	परत rc;
}

पूर्णांक ul_strtou16(स्थिरांक अक्षर *str, uint16_t *अंक, पूर्णांक base)
{
	uint64_t tmp;
	पूर्णांक rc;

	rc = ul_strtou64(str, &tmp, base);
	जर (rc == 0 && tmp > UINT16_MAX)
		rc = -(त्रुटि_सं = दुस्फल);
	जर (rc == 0)
		*अंक = (uint16_t) tmp;
	परत rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(स्थिरांक अक्षर *str, पूर्णांक base, स्थिरांक अक्षर *errmesg,
			     int64_t खालचा, int64_t up)
{
	int64_t अंक = 0;
	पूर्णांक rc;

	rc = ul_strtos64(str, &अंक, base);
	जर (rc == 0 && ((खालचा && अंक < खालचा) || (up && अंक > up)))
		rc = -(त्रुटि_सं = दुस्फल);

	जर (rc) {
		जर (त्रुटि_सं == दुस्फल)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	परत अंक;
}

uint64_t str2unum_or_err(स्थिरांक अक्षर *str, पूर्णांक base, स्थिरांक अक्षर *errmesg, uint64_t up)
{
	uint64_t अंक = 0;
	पूर्णांक rc;

	rc = ul_strtou64(str, &अंक, base);
	जर (rc == 0 && (up && अंक > up))
		rc = -(त्रुटि_सं = दुस्फल);

	जर (rc) {
		जर (त्रुटि_सं == दुस्फल)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	परत अंक;
}

द्विगुण strtod_or_err(स्थिरांक अक्षर *str, स्थिरांक अक्षर *errmesg)
{
	द्विगुण अंक;
	अक्षर *end = रिक्त;

	त्रुटि_सं = 0;
	जर (str == रिक्त || *str == '\0')
		जा err;
	अंक = म_से_भग्न(str, &end);

	जर (त्रुटि_सं || str == end || (end && *end))
		जा err;

	परत अंक;
err:
	जर (त्रुटि_सं == दुस्फल)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

पूर्णांक ul_strtold(स्थिरांक अक्षर *str, दीर्घ द्विगुण *अंक)
{
	अक्षर *end = रिक्त;

	त्रुटि_सं = 0;
	जर (str == रिक्त || *str == '\0')
		परत -(त्रुटि_सं = EINVAL);
	*अंक = strtold(str, &end);

	जर (त्रुटि_सं != 0)
		परत -त्रुटि_सं;
	जर (str == end || (end && *end))
		परत -(त्रुटि_सं = EINVAL);
	परत 0;
}

दीर्घ द्विगुण strtold_or_err(स्थिरांक अक्षर *str, स्थिरांक अक्षर *errmesg)
{
	दीर्घ द्विगुण अंक = 0;

	जर (ul_strtold(str, &अंक) == 0)
		परत अंक;
	जर (त्रुटि_सं == दुस्फल)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(स्थिरांक अक्षर *str, स्थिरांक अक्षर *errmesg)
{
	uintmax_t अंक;

	जर (strtosize(str, &अंक) == 0)
		परत अंक;

	जर (त्रुटि_सं)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}


शून्य strtotimeval_or_err(स्थिरांक अक्षर *str, संरचना timeval *tv, स्थिरांक अक्षर *errmesg)
{
	दीर्घ द्विगुण user_आदान;

	user_आदान = strtold_or_err(str, errmesg);
	tv->tv_sec = (समय_प्रकार) user_आदान;
	tv->tv_usec = (suseconds_t)((user_आदान - tv->tv_sec) * USEC_PER_SEC);
}

शून्य strtotimespec_or_err(स्थिरांक अक्षर *str, संरचना timespec *ts, स्थिरांक अक्षर *errmesg)
{
	दीर्घ द्विगुण user_आदान;

	user_आदान = strtold_or_err(str, errmesg);
	ts->tv_sec = (समय_प्रकार) user_आदान;
	ts->tv_nsec = (दीर्घ)((user_आदान - ts->tv_sec) * NSEC_PER_SEC);
}

समय_प्रकार strtotime_or_err(स्थिरांक अक्षर *str, स्थिरांक अक्षर *errmesg)
{
	int64_t user_आदान;

	user_आदान = strtos64_or_err(str, errmesg);
	परत (समय_प्रकार) user_आदान;
}

bool hyperlinkwanted(स्थिरांक अक्षर *mode)
{
	जर (mode && म_भेद(mode, "never") == 0)
		परत false;

	जर (mode && म_भेद(mode, "always") == 0)
		परत true;

	जर (!mode || म_भेद(mode, "auto") == 0)
		परत isatty(STDOUT_FILENO) ? true : false;

	errx(निकास_त्रुटि, _("invalid argument of --hyperlink: %s"), mode);
}

bool annotationwanted(स्थिरांक अक्षर *mode)
{
	जर (mode && म_भेद(mode, "never") == 0)
		परत false;

	जर (mode && म_भेद(mode, "always") == 0)
		परत true;

	जर (!mode || म_भेद(mode, "auto") == 0)
		परत isatty(STDOUT_FILENO) ? true : false;

	errx(निकास_त्रुटि, _("invalid argument of --annotate: %s"), mode);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
अक्षर *xstrmode(mode_t mode, अक्षर *str)
{
	अचिन्हित लघु i = 0;

	जर (S_ISDIR(mode))
		str[i++] = 'd';
	अन्यथा जर (S_ISLNK(mode))
		str[i++] = 'l';
	अन्यथा जर (S_ISCHR(mode))
		str[i++] = 'c';
	अन्यथा जर (S_ISBLK(mode))
		str[i++] = 'b';
	अन्यथा जर (S_ISSOCK(mode))
		str[i++] = 's';
	अन्यथा जर (S_ISFIFO(mode))
		str[i++] = 'p';
	अन्यथा जर (S_ISREG(mode))
		str[i++] = '-';

	str[i++] = mode & S_IRUSR ? 'r' : '-';
	str[i++] = mode & S_IWUSR ? 'w' : '-';
	str[i++] = (mode & S_ISUID
		? (mode & S_IXUSR ? 's' : 'S')
		: (mode & S_IXUSR ? 'x' : '-'));
	str[i++] = mode & S_IRGRP ? 'r' : '-';
	str[i++] = mode & S_IWGRP ? 'w' : '-';
	str[i++] = (mode & S_ISGID
		? (mode & S_IXGRP ? 's' : 'S')
		: (mode & S_IXGRP ? 'x' : '-'));
	str[i++] = mode & S_IROTH ? 'r' : '-';
	str[i++] = mode & S_IWOTH ? 'w' : '-';
	str[i++] = (mode & S_ISVTX
		? (mode & S_IXOTH ? 't' : 'T')
		: (mode & S_IXOTH ? 'x' : '-'));
	str[i] = '\0';

	परत str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
स्थिर पूर्णांक get_exp(uint64_t n)
{
	पूर्णांक shft;

	साठी (shft = 10; shft <= 60; shft += 10) {
		जर (n < (1ULL << shft))
			थांबा;
	}
	परत shft - 10;
}

अक्षर *size_to_human_string(पूर्णांक options, uint64_t bytes)
{
	अक्षर buf[32];
	पूर्णांक dec, exp;
	uint64_t frac;
	स्थिरांक अक्षर *letters = "BKMGTPE";
	अक्षर suffix[आकार(" KiB")], *psuf = suffix;
	अक्षर c;

	जर (options & SIZE_SUFFIX_SPACE)
		*psuf++ = ' ';


	exp  = get_exp(bytes);
	c    = *(letters + (exp ? exp / 10 : 0));
	dec  = exp ? bytes / (1ULL << exp) : bytes;
	frac = exp ? bytes % (1ULL << exp) : 0;

	*psuf++ = c;

	जर ((options & SIZE_SUFFIX_3LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/* round */
	जर (frac) {
		/* get 3 digits after decimal point */
		जर (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (exp - 10)) ;
		अन्यथा
			frac = (frac * 1000) / (1ULL << (exp)) ;

		जर (options & SIZE_DECIMAL_2DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} अन्यथा {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		जर (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	जर (frac) {
		संरचना स्था_बदल स्थिरांक *l = क्षेत्र_बदलो();
		अक्षर *dp = l ? l->decimal_point : रिक्त;
		पूर्णांक len;

		जर (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, आकार(buf), "%d%s%02" PRIu64, dec, dp, frac);
		जर (len > 0 && (माप_प्रकार) len < आकार(buf)) {
			/* remove potential extraneous zero */
			जर (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, आकार(buf) - len);
		} अन्यथा
			*buf = '\0';	/* snprintf error */
	} अन्यथा
		snprintf(buf, आकार(buf), "%d%s", dec, suffix);

	परत strdup(buf);
}

/*
 * Parses comma delimited list to array with IDs, for example:
 *
 * "aaa,bbb,ccc" --> ary[0] = FOO_AAA;
 *                   ary[1] = FOO_BBB;
 *                   ary[3] = FOO_CCC;
 *
 * The function name2id() provides conversion from string to ID.
 *
 * Returns: >= 0  : number of items added to ary[]
 *            -1  : parse error or unknown item
 *            -2  : arysz reached
 */
पूर्णांक string_to_idarray(स्थिरांक अक्षर *list, पूर्णांक ary[], माप_प्रकार arysz,
			पूर्णांक (name2id)(स्थिरांक अक्षर *, माप_प्रकार))
{
	स्थिरांक अक्षर *begin = रिक्त, *p;
	माप_प्रकार n = 0;

	जर (!list || !*list || !ary || !arysz || !name2id)
		परत -1;

	साठी (p = list; p && *p; p++) {
		स्थिरांक अक्षर *end = रिक्त;
		पूर्णांक id;

		जर (n >= arysz)
			परत -2;
		जर (!begin)
			begin = p;		/* begin of the column name */
		जर (*p == ',')
			end = p;		/* terminate the name */
		जर (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		जर (!begin || !end)
			सुरूठेवा;
		जर (end <= begin)
			परत -1;

		id = name2id(begin, end - begin);
		जर (id == -1)
			परत -1;
		ary[ n++ ] = id;
		begin = रिक्त;
		जर (end && !*end)
			थांबा;
	}
	परत n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
पूर्णांक string_add_to_idarray(स्थिरांक अक्षर *list, पूर्णांक ary[], माप_प्रकार arysz,
			माप_प्रकार *ary_pos, पूर्णांक (name2id)(स्थिरांक अक्षर *, माप_प्रकार))
{
	स्थिरांक अक्षर *list_add;
	पूर्णांक r;

	जर (!list || !*list || !ary_pos || *ary_pos > arysz)
		परत -1;

	जर (list[0] == '+')
		list_add = &list[1];
	अन्यथा {
		list_add = list;
		*ary_pos = 0;
	}

	r = string_to_idarray(list_add, &ary[*ary_pos], arysz - *ary_pos, name2id);
	जर (r > 0)
		*ary_pos += r;
	परत r;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2id() function and the 'id' is used
 * as a position in the 'ary' bit array. It means that the 'id' has to be in
 * range <0..N> where N < sizeof(ary) * NBBY.
 *
 * If allow_range is enabled:
 * An item ending in '+' also sets all bits in <0..N>.
 * An item beginning with '+' also sets all bits in <N..allow_minus>.
 *
 * Returns: 0 on success, <0 on error.
 */
पूर्णांक string_to_bitarray(स्थिरांक अक्षर *list,
		     अक्षर *ary,
		     पूर्णांक (*name2bit)(स्थिरांक अक्षर *, माप_प्रकार),
		     माप_प्रकार allow_range)
{
	स्थिरांक अक्षर *begin = रिक्त, *p;

	जर (!list || !name2bit || !ary)
		परत -EINVAL;

	साठी (p = list; p && *p; p++) {
		स्थिरांक अक्षर *end = रिक्त;
		पूर्णांक bit, set_lower = 0, set_higher = 0;

		जर (!begin)
			begin = p;		/* begin of the level name */
		जर (*p == ',')
			end = p;		/* terminate the name */
		जर (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		जर (!begin || !end)
			सुरूठेवा;
		जर (end <= begin)
			परत -1;
		जर (allow_range) {
			जर (*(end - 1) == '+') {
				end--;
				set_lower = 1;
			} अन्यथा जर (*begin == '+') {
				begin++;
				set_higher = 1;
			}
		}

		bit = name2bit(begin, end - begin);
		जर (bit < 0)
			परत bit;
		setbit(ary, bit);
		जर (set_lower)
			पर्यंत (--bit >= 0)
				setbit(ary, bit);
		अन्यथा जर (set_higher)
			पर्यंत (++bit < (पूर्णांक) allow_range)
				setbit(ary, bit);
		begin = रिक्त;
		जर (end && !*end)
			थांबा;
	}
	परत 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
पूर्णांक string_to_bitmask(स्थिरांक अक्षर *list,
		     अचिन्हित दीर्घ *mask,
		     दीर्घ (*name2flag)(स्थिरांक अक्षर *, माप_प्रकार))
{
	स्थिरांक अक्षर *begin = रिक्त, *p;

	जर (!list || !name2flag || !mask)
		परत -EINVAL;

	साठी (p = list; p && *p; p++) {
		स्थिरांक अक्षर *end = रिक्त;
		दीर्घ flag;

		जर (!begin)
			begin = p;		/* begin of the level name */
		जर (*p == ',')
			end = p;		/* terminate the name */
		जर (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		जर (!begin || !end)
			सुरूठेवा;
		जर (end <= begin)
			परत -1;

		flag = name2flag(begin, end - begin);
		जर (flag < 0)
			परत flag;	/* error */
		*mask |= flag;
		begin = रिक्त;
		जर (end && !*end)
			थांबा;
	}
	परत 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
पूर्णांक ul_parse_range(स्थिरांक अक्षर *str, पूर्णांक *lower, पूर्णांक *upper, पूर्णांक def)
{
	अक्षर *end = रिक्त;

	जर (!str)
		परत 0;

	*upper = *lower = def;
	त्रुटि_सं = 0;

	जर (*str == ':') {				/* <:N> */
		str++;
		*upper = म_से_दीर्घ(str, &end, 10);
		जर (त्रुटि_सं || !end || *end || end == str)
			परत -1;
	} अन्यथा {
		*upper = *lower = म_से_दीर्घ(str, &end, 10);
		जर (त्रुटि_सं || !end || end == str)
			परत -1;

		जर (*end == ':' && !*(end + 1))		/* <M:> */
			*upper = def;
		अन्यथा जर (*end == '-' || *end == ':') {	/* <M:N> <M-N> */
			str = end + 1;
			end = रिक्त;
			त्रुटि_सं = 0;
			*upper = म_से_दीर्घ(str, &end, 10);

			जर (त्रुटि_सं || !end || *end || end == str)
				परत -1;
		}
	}
	परत 0;
}

स्थिरांक अक्षर *ul_path_next_segment(स्थिरांक अक्षर *str, माप_प्रकार *sz)
{
	स्थिरांक अक्षर *start, *p;

	start = str;
	*sz = 0;
	पर्यंत (start && *start == '/' && *(start + 1) == '/')
		start++;

	जर (!start || !*start)
		परत रिक्त;

	साठी (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	परत start;
}

पूर्णांक streq_paths(स्थिरांक अक्षर *a, स्थिरांक अक्षर *b)
{
	पर्यंत (a && b) {
		माप_प्रकार a_sz, b_sz;
		स्थिरांक अक्षर *a_seg = ul_path_next_segment(a, &a_sz);
		स्थिरांक अक्षर *b_seg = ul_path_next_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		जर (a_sz + b_sz == 0)
			परत 1;

		/* ignore trailing slash */
		जर (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			परत 1;

		जर (!a_seg || !b_seg)
			थांबा;
		जर (a_sz != b_sz || म_भेदन(a_seg, b_seg, a_sz) != 0)
			थांबा;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	परत 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
पूर्णांक ul_path_is_within(स्थिरांक अक्षर *path, स्थिरांक अक्षर *dir)
{
	जर (!path || !dir || !*path || !*dir)
		परत 0;

	पर्यंत (path && dir) {
		माप_प्रकार p_sz, d_sz;
		स्थिरांक अक्षर *p_seg = ul_path_next_segment(path, &p_sz);
		स्थिरांक अक्षर *d_seg = ul_path_next_segment(dir, &d_sz);

		/* all @dir segments matched */
		जर (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			परत 1;

		जर (!p_seg || !d_seg)
			थांबा;
		जर (p_sz != d_sz || म_भेदन(p_seg, d_seg, p_sz) != 0)
			थांबा;

		path = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	परत 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
अक्षर *ul_strnconcat(स्थिरांक अक्षर *s, स्थिरांक अक्षर *suffix, माप_प्रकार b)
{
	माप_प्रकार a;
	अक्षर *r;

	जर (!s && !suffix)
		परत strdup("");
	जर (!s)
		परत strndup(suffix, b);
	जर (!suffix)
		परत strdup(s);

	प्रतिज्ञा(s);
	प्रतिज्ञा(suffix);

	a = म_माप(s);
	जर (b > ((माप_प्रकार) -1) - a)
		परत रिक्त;

	r = दो_स्मृति(a + b + 1);
	जर (!r)
		परत रिक्त;

	स_नकल(r, s, a);
	स_नकल(r + a, suffix, b);
	r[a+b] = 0;

	परत r;
}

/* concatenate two strings to a new string */
अक्षर *ul_strconcat(स्थिरांक अक्षर *s, स्थिरांक अक्षर *suffix)
{
	परत ul_strnconcat(s, suffix, suffix ? म_माप(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
अक्षर *ul_strfconcat(स्थिरांक अक्षर *s, स्थिरांक अक्षर *format, ...)
{
	बहु_सूची ap;
	अक्षर *val, *res;
	पूर्णांक sz;

	बहु_शुरू(ap, format);
	sz = vasprintf(&val, format, ap);
	बहु_पूर्ण(ap);

	जर (sz < 0)
		परत रिक्त;

	res = ul_strnconcat(s, val, sz);
	मुक्त(val);
	परत res;
}

पूर्णांक ul_strappend(अक्षर **a, स्थिरांक अक्षर *b)
{
	माप_प्रकार al, bl;
	अक्षर *tmp;

	जर (!a)
		परत -EINVAL;
	जर (!b || !*b)
		परत 0;
	जर (!*a) {
		*a = strdup(b);
		परत !*a ? -ENOMEM : 0;
	}

	al = म_माप(*a);
	bl = म_माप(b);

	tmp = पुनः_स्मृति(*a, al + bl + 1);
	जर (!tmp)
		परत -ENOMEM;
	*a = tmp;
	स_नकल((*a) + al, b, bl + 1);
	परत 0;
}

/* the hybrid version of strfconcat and strappend. */
पूर्णांक strfappend(अक्षर **a, स्थिरांक अक्षर *format, ...)
{
	बहु_सूची ap;
	पूर्णांक res;

	बहु_शुरू(ap, format);
	res = ul_strvfappend(a, format, ap);
	बहु_पूर्ण(ap);

	परत res;
}

बाह्य पूर्णांक ul_strvfappend(अक्षर **a, स्थिरांक अक्षर *format, बहु_सूची ap)
{
	अक्षर *val;
	पूर्णांक sz;
	पूर्णांक res;

	sz = vasprintf(&val, format, ap);
	जर (sz < 0)
		परत -त्रुटि_सं;

	res = ul_strappend(a, val);
	मुक्त(val);
	परत res;
}

स्थिर माप_प्रकार strcspn_escaped(स्थिरांक अक्षर *s, स्थिरांक अक्षर *reject)
{
	पूर्णांक escaped = 0;
	पूर्णांक n;

	साठी (n=0; s[n]; n++) {
		जर (escaped)
			escaped = 0;
		अन्यथा जर (s[n] == '\\')
			escaped = 1;
		अन्यथा जर (म_अक्षर(reject, s[n]))
			थांबा;
	}

	/* if s ends in \, return index of previous char */
	परत n - escaped;
}

/*
 * Like strchr() but ignores @c if escaped by '\', '\\' is interpreted like '\'.
 *
 * For example for @c='X':
 *
 *      "abcdXefgXh"    --> "XefgXh"
 *	"abcd\XefgXh"   --> "Xh"
 *	"abcd\\XefgXh"  --> "XefgXh"
 *	"abcd\\\XefgXh" --> "Xh"
 *	"abcd\Xefg\Xh"  --> (null)
 *
 *	"abcd\\XefgXh"  --> "\XefgXh"   for @c='\\'
 */
अक्षर *ul_strchr_escaped(स्थिरांक अक्षर *s, पूर्णांक c)
{
	अक्षर *p;
	पूर्णांक esc = 0;

	साठी (p = (अक्षर *) s; p && *p; p++) {
		जर (!esc && *p == '\\') {
			esc = 1;
			सुरूठेवा;
		}
		जर (*p == c && (!esc || c == '\\'))
			परत p;
		esc = 0;
	}

	परत रिक्त;
}

/* Split a string into words. */
स्थिरांक अक्षर *ul_split(स्थिरांक अक्षर **state, माप_प्रकार *l, स्थिरांक अक्षर *separator, पूर्णांक quoted)
{
	स्थिरांक अक्षर *current;

	current = *state;

	जर (!*current) {
		प्रतिज्ञा(**state == '\0');
		परत रिक्त;
	}

	current += म_अखोज(current, separator);
	जर (!*current) {
		*state = current;
		परत रिक्त;
	}

	जर (quoted && म_अक्षर("\'\"", *current)) {
		अक्षर quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		जर (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !म_अक्षर(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			परत रिक्त;
		}
		*state = current++ + *l + 2;
	} अन्यथा जर (quoted) {
		*l = strcspn_escaped(current, separator);
		जर (current[*l] && !म_अक्षर(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			परत रिक्त;
		}
		*state = current + *l;
	} अन्यथा {
		*l = म_खोज(current, separator);
		*state = current + *l;
	}

	परत current;
}

/* Rewind file pointer forward to new line.  */
पूर्णांक skip_fline(खाता *fp)
{
	पूर्णांक ch;

	करा {
		जर ((ch = ख_अक्षर_लो(fp)) == खातापूर्ण)
			परत 1;
		जर (ch == '\n')
			परत 0;
	} पर्यंत (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
पूर्णांक ul_stralnumcmp(स्थिरांक अक्षर *p1, स्थिरांक अक्षर *p2)
{
	स्थिरांक अचिन्हित अक्षर *s1 = (स्थिरांक अचिन्हित अक्षर *) p1;
	स्थिरांक अचिन्हित अक्षर *s2 = (स्थिरांक अचिन्हित अक्षर *) p2;
	अचिन्हित अक्षर c1, c2;

	करा {
		करा {
			c1 = (अचिन्हित अक्षर) *s1++;
		} पर्यंत (c1 != '\0' && !है_अक्षर_अंक((अचिन्हित पूर्णांक) c1));

		करा {
			c2 = (अचिन्हित अक्षर) *s2++;
		} पर्यंत (c2 != '\0' && !है_अक्षर_अंक((अचिन्हित पूर्णांक) c2));

		जर (c1 != '\0')
			c1 = छोटे(c1);
		जर (c2 != '\0')
			c2 = छोटे(c2);
		जर (c1 == '\0')
			परत c1 - c2;
	} पर्यंत (c1 == c2);

	परत c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
पूर्णांक ul_optstr_next(अक्षर **optstr, अक्षर **name, माप_प्रकार *namesz,
		   अक्षर **मूल्य, माप_प्रकार *valsz)
{
	पूर्णांक open_quote = 0;
	अक्षर *start = रिक्त, *stop = रिक्त, *p, *sep = रिक्त;
	अक्षर *optstr0;

	प्रतिज्ञा(optstr);
	प्रतिज्ञा(*optstr);

	optstr0 = *optstr;

	जर (name)
		*name = रिक्त;
	जर (namesz)
		*namesz = 0;
	जर (मूल्य)
		*मूल्य = रिक्त;
	जर (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	पर्यंत (optstr0 && *optstr0 == ',')
		optstr0++;

	साठी (p = optstr0; p && *p; p++) {
		जर (!start && *p == '=')
			परत -EINVAL;
		जर (!start)
			start = p;		/* beginning of the option item */
		जर (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		जर (open_quote)
			सुरूठेवा;		/* still in quoted block */
		जर (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		जर (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		अन्यथा जर (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		जर (!start || !stop)
			सुरूठेवा;
		जर (stop <= start)
			परत -EINVAL;

		जर (name)
			*name = start;
		जर (namesz)
			*namesz = sep ? sep - start : stop - start;
		*optstr = *stop ? stop + 1 : stop;

		जर (sep) {
			जर (मूल्य)
				*मूल्य = sep + 1;
			जर (valsz)
				*valsz = stop - sep - 1;
		}
		परत 0;
	}

	परत 1;				/* end of optstr */
}

पूर्णांक ul_optstr_is_valid(स्थिरांक अक्षर *optstr)
{
	पूर्णांक rc;
	अक्षर *p = (अक्षर *) optstr;

	पर्यंत ((rc = ul_optstr_next(&p, रिक्त, रिक्त, रिक्त, रिक्त)) == 0);
	परत rc < 0 ? 0 : 1;
}

अक्षर *ul_optstr_get_मूल्य(स्थिरांक अक्षर *optstr, स्थिरांक अक्षर *किल्ली)
{
	माप_प्रकार sz, namesz = 0, valsz = 0;
	अक्षर *name = रिक्त, *मूल्य = रिक्त;
	अक्षर *p = (अक्षर *) optstr;

	जर (!optstr || !किल्ली || !*किल्ली)
		परत रिक्त;

	sz = म_माप(किल्ली);
	पर्यंत (ul_optstr_next(&p, &name, &namesz, &मूल्य, &valsz) == 0) {
		जर (namesz != sz || !valsz)
			सुरूठेवा;
		जर (म_भेदन(name, किल्ली, namesz) == 0)
			परत strndup(मूल्य, valsz);
	}
	परत रिक्त;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
माप_प्रकार ul_unquote(अक्षर *str)
{
	माप_प्रकार len;
	अक्षर q;

	जर (!str || !*str)
		परत 0;

	len = म_माप(str);
	q = *str;

	जर ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		परत len;

	स_हटाओ(str, str + 1, len - 2);
	str[len - 2] = '\0';
	परत len - 2;
}

#ifdef TEST_PROGRAM_STRUTILS

संरचना testS {
	अक्षर *name;
	अक्षर *मूल्य;
};

स्थिर पूर्णांक test_strdup_to_member(पूर्णांक argc, अक्षर *argv[])
{
	संरचना testS *xx;

	जर (argc < 3)
		परत निकास_त्रुटि;

	xx = सुस्मृति(1, आकार(*xx));
	जर (!xx)
		err(निकास_त्रुटि, "calloc() failed");

	strdup_to_struct_member(xx, name, argv[1]);
	strdup_to_struct_member(xx, मूल्य, argv[2]);

	जर (म_भेद(xx->name, argv[1]) != 0 &&
	    म_भेद(xx->मूल्य, argv[2]) != 0)
		errx(निकास_त्रुटि, "strdup_to_struct_member() failed");

	म_लिखो("1: '%s', 2: '%s'\n", xx->name, xx->मूल्य);

	मुक्त(xx->name);
	मुक्त(xx->मूल्य);
	मुक्त(xx);
	परत निकास_सफल;
}

स्थिर पूर्णांक test_strutils_sizes(पूर्णांक argc, अक्षर *argv[])
{
	uintmax_t size = 0;
	अक्षर *hum1, *hum2, *hum3;

	जर (argc < 2)
		परत निकास_त्रुटि;

	जर (strtosize(argv[1], &size))
		errx(निकास_त्रुटि, "invalid size '%s' value", argv[1]);

	hum1 = size_to_human_string(SIZE_SUFFIX_1LETTER, size);
	hum2 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE, size);
	hum3 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE |
				    SIZE_DECIMAL_2DIGITS, size);

	म_लिखो("%25s : %20ju : %8s : %12s : %13s\n", argv[1], size, hum1, hum2, hum3);
	मुक्त(hum1);
	मुक्त(hum2);
	मुक्त(hum3);

	परत निकास_सफल;
}

स्थिर पूर्णांक test_strutils_cmp_paths(पूर्णांक argc, अक्षर *argv[])
{
	पूर्णांक rc = streq_paths(argv[1], argv[2]);

	जर (argc < 3)
		परत निकास_त्रुटि;

	म_लिखो("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	परत निकास_सफल;
}

स्थिर पूर्णांक test_strutils_path_is_within(पूर्णांक argc, अक्षर *argv[])
{
	पूर्णांक rc;

	जर (argc < 3)
		परत निकास_त्रुटि;

	rc = ul_path_is_within(argv[1], argv[2]);

	म_लिखो("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	परत निकास_सफल;
}

स्थिर पूर्णांक test_strutils_normalize(पूर्णांक argc, अक्षर *argv[])
{
	अचिन्हित अक्षर *src, *dst, *org;
	माप_प्रकार sz, len;

	जर (argc < 2)
		परत निकास_त्रुटि;

	org = (अचिन्हित अक्षर *) strdup(argv[1]);
	src = (अचिन्हित अक्षर *) strdup((अक्षर *) org);
	len = म_माप((अक्षर *) src);
	dst = दो_स्मृति(len + 1);

	जर (!org || !src || !dst)
		जा done;

	/* two buffers */
	sz = __normalize_whitespace(src, len, dst, len + 1);
	म_लिखो("1: '%s' --> '%s' [sz=%zu]\n", src, dst, sz);

	/* one buffer */
	sz = normalize_whitespace(src);
	म_लिखो("2: '%s' --> '%s' [sz=%zu]\n", org, src, sz);

done:
	मुक्त(src);
	मुक्त(dst);
	मुक्त(org);

	परत निकास_सफल;
}

स्थिर पूर्णांक test_strutils_cstrcasecmp(पूर्णांक argc, अक्षर *argv[])
{
	अक्षर *a, *b;

	जर (argc < 3)
		परत निकास_त्रुटि;

	a = argv[1];
	b = argv[2];

	जर (!a || !b)
		परत निकास_त्रुटि;

	म_लिखो("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	म_लिखो("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	म_लिखो("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, म_माप(a)));

	परत निकास_सफल;
}

पूर्णांक main(पूर्णांक argc, अक्षर *argv[])
{
	जर (argc == 3 && म_भेद(argv[1], "--size") == 0) {
		परत test_strutils_sizes(argc - 1, argv + 1);

	} अन्यथा जर (argc == 3 && म_भेद(argv[1], "--parse-switch") == 0) {
		म_लिखो("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						रिक्त));
		परत निकास_सफल;
	} अन्यथा जर (argc == 3 && म_भेद(argv[1], "--strtobool") == 0) {
		bool val;
		जर (ul_strtobool(argv[2], &val))
			err(निकास_त्रुटि, "could not parse '%s'", argv[2]);
		म_लिखो("'%s'-->%s\n", argv[2], val ? "true" : "false");
		परत निकास_सफल;

	} अन्यथा जर (argc == 4 && म_भेद(argv[1], "--cmp-paths") == 0) {
		परत test_strutils_cmp_paths(argc - 1, argv + 1);

	} अन्यथा जर (argc == 4 && म_भेद(argv[1], "--path-is-within") == 0) {
		परत test_strutils_path_is_within(argc - 1, argv + 1);

	} अन्यथा जर (argc == 4 && म_भेद(argv[1], "--strdup-member") == 0) {
		परत test_strdup_to_member(argc - 1, argv + 1);

	} अन्यथा जर  (argc == 4 && म_भेद(argv[1], "--stralnumcmp") == 0) {
		म_लिखो("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		परत निकास_सफल;

	} अन्यथा जर (argc == 4 && म_भेद(argv[1], "--cstrcasecmp") == 0) {
		परत test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} अन्यथा जर (argc == 3 && म_भेद(argv[1], "--normalize") == 0) {
		परत test_strutils_normalize(argc - 1, argv + 1);

	} अन्यथा जर (argc == 3 && म_भेद(argv[1], "--strtos64") == 0) {
		म_लिखो("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		परत निकास_सफल;
	} अन्यथा जर (argc == 3 && म_भेद(argv[1], "--strtou64") == 0) {
		म_लिखो("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		परत निकास_सफल;
	} अन्यथा जर (argc == 3 && म_भेद(argv[1], "--strtos32") == 0) {
		म_लिखो("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		परत निकास_सफल;
	} अन्यथा जर (argc == 3 && म_भेद(argv[1], "--strtou32") == 0) {
		म_लिखो("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		परत निकास_सफल;
	} अन्यथा जर (argc == 3 && म_भेद(argv[1], "--strtos16") == 0) {
		म_लिखो("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		परत निकास_सफल;
	} अन्यथा जर (argc == 3 && म_भेद(argv[1], "--strtou16") == 0) {
		म_लिखो("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		परत निकास_सफल;

	} अन्यथा जर (argc == 4 && म_भेद(argv[1], "--strchr-escaped") == 0) {
		म_लिखो("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		परत निकास_सफल;

	} अन्यथा जर (argc == 2 && म_भेद(argv[1], "--next-string") == 0) {
		अक्षर *buf = "abc\0Y\0\0xyz\0X";
		अक्षर *end = buf + 12;
		अक्षर *p = buf;

		करा {
			म_लिखो("str: '%s'\n", p);
		} पर्यंत ((p = ul_next_string(p, end)));

		परत निकास_सफल;

	} अन्यथा जर (argc == 3 && म_भेद(argv[1], "--optstr") == 0) {

		माप_प्रकार namesz, valsz;
		अक्षर *name = रिक्त, *val = रिक्त;
		अक्षर *p = argv[2];
		पूर्णांक rc;

		जर (!ul_optstr_is_valid(p))
			errx(निकास_त्रुटि, _("unsupported option format: %s"), p);

		पर्यंत ((rc = ul_optstr_next(&p, &name, &namesz, &val, &valsz)) == 0) {
			म_लिखो("'%.*s' : '%.*s'\n", (पूर्णांक) namesz, name,
						   (पूर्णांक) valsz, val);
		}
		जर (rc == 1)
			परत निकास_सफल;
	} अन्यथा जर (argc == 3 && म_भेद(argv[1], "--unquote") == 0) {

		अक्षर *unquoted = strdup(argv[2]);

		जर (!unquoted)
			err(निकास_त्रुटि, "strdup() failed");

		ul_unquote(unquoted);
		म_लिखो("%s-->%s\n", argv[2], unquoted);
		मुक्त(unquoted);
		परत निकास_सफल;

	} अन्यथा {
		ख_लिखो(मानक_त्रुटि, "usage: %1$s --size <number>[suffix]\n"
				"       %1$s --strtobool <str>\n"
				"       %1$s --parse-switch <str>\n"
				"       %1$s --cmp-paths <path> <path>\n"
				"       %1$s --path-is-within <path> <dir>\n"
				"       %1$s --strdup-member <str> <str>\n"
				"       %1$s --stralnumcmp <str> <str>\n"
				"       %1$s --cstrcasecmp <str> <str>\n"
				"       %1$s --normalize <str>\n"
				"       %1$s --strto{s,u}{16,32,64} <str>\n"
				"       %1$s --optstr <str>\n"
				"       %1$s --unquote <str>\n",
				argv[0]);
		निकास(निकास_त्रुटि);
	}

	परत निकास_त्रुटि;
}
#endif /* TEST_PROGRAM_STRUTILS */
