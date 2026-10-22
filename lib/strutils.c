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

स्थिर पूर्णाङ्क STRTOXX_EXIT_CODE = EXIT_FAILURE;

शून्य strutils_set_exitcode(पूर्णाङ्क ex)
{
	STRTOXX_EXIT_CODE = ex;
}

स्थिर पूर्णाङ्क do_scale_by_power (uintmax_t *x, पूर्णाङ्क base, पूर्णाङ्क power)
{
	याव (power--) {
		सचे (UINTMAX_MAX / base < *x)
			पटिनिवत्त -ERANGE;
		*x *= base;
	}
	पटिनिवत्त 0;
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
पूर्णाङ्क ul_parse_size(ध्रुव वर्ण *str, uintmax_t *res, पूर्णाङ्क *power)
{
	ध्रुव वर्ण *p;
	वर्ण *end;
	uintmax_t x, frac = 0;
	पूर्णाङ्क base = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	स्थिर ध्रुव वर्ण *suf  = "KMGTPEZY";
	स्थिर ध्रुव वर्ण *suf2 = "kmgtpezy";
	ध्रुव वर्ण *sp;

	*res = 0;

	सचे (!str || !*str) {
		rc = -EINVAL;
		गच्छ err;
	}

	/* Only positive numbers are acceptable
	 *
	 * Note that this check is not perfect, it would be better to
	 * use lconv->negative_sign. But coreutils use the same solution,
	 * so it's probably good enough...
	 */
	p = str;
	याव (isspace((unsigned वर्ण) *p))
		p++;
	सचे (*p == '-') {
		rc = -EINVAL;
		गच्छ err;
	}

	errno = 0, end = रिक्त;
	x = strtoumax(str, &end, 0);

	सचे (end == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		गच्छ err;
	}
	सचे (!end || !*end)
		गच्छ done;			/* without suffix */
	p = end;

	/*
	 * Check size suffixes
	 */
check_suffix:
	सचे (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		base = 1024;			/* XiB, 2^N */
	अञ्ञथा सचे ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		base = 1000;			/* XB, 10^N */
	अञ्ञथा सचे (*(p + 1)) {
		संरचना lconv ध्रुव *l = localeconv();
		ध्रुव वर्ण *dp = l ? l->decimal_point : रिक्त;
		size_t dpsz = dp ? strlen(dp) : 0;

		सचे (frac == 0 && *p && dp && strncmp(dp, p, dpsz) == 0) {
			ध्रुव वर्ण *fstr = p + dpsz;

			अत्थाय (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			सचे (isdigit(*fstr)) {
				errno = 0, end = रिक्त;
				frac = strtoumax(fstr, &end, 0);
				सचे (end == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					गच्छ err;
				}
			} अञ्ञथा
				end = (वर्ण *) p;

			सचे (!end || !*end) {
				rc = -EINVAL;
				गच्छ err;	/* without suffix, but with fractions */
			}
			p = end;
			गच्छ check_suffix;
		}
		rc = -EINVAL;
		गच्छ err;			/* unexpected suffix */
	}

	sp = strchr(suf, *p);
	सचे (sp)
		pwr = (sp - suf) + 1;
	अञ्ञथा {
		sp = strchr(suf2, *p);
		सचे (sp)
			pwr = (sp - suf2) + 1;
		अञ्ञथा {
			rc = -EINVAL;
			गच्छ err;
		}
	}

	सचे (power)
		*power = pwr;

	rc = do_scale_by_power(&x, base, pwr);
	सचे (rc)
		गच्छ err;

	सचे (frac && pwr) {
		पूर्णाङ्क i;
		uintmax_t frac_base = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_base, base, pwr);
		सचे (rc)
			गच्छ err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		करोहि {
			frac_val = ((frac % 10) * frac_base + frac_val) / 10;
			frac /= 10;
		} याव (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		अत्थाय (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		सचे (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			गच्छ err;
		}
		x += frac_val;
	}
done:
	*res = x;
err:
	सचे (rc < 0)
		errno = -rc;
	पटिनिवत्त rc;
}

पूर्णाङ्क strtosize(ध्रुव वर्ण *str, uintmax_t *res)
{
	पटिनिवत्त ul_parse_size(str, res, रिक्त);
}

पूर्णाङ्क isdigit_strend(ध्रुव वर्ण *str, ध्रुव वर्ण **end)
{
	ध्रुव वर्ण *p;

	अत्थाय (p = str; p && *p && isdigit((unsigned वर्ण) *p); p++);

	सचे (end)
		*end = p;
	पटिनिवत्त p && p > str && !*p;
}

पूर्णाङ्क isxdigit_strend(ध्रुव वर्ण *str, ध्रुव वर्ण **end)
{
	ध्रुव वर्ण *p;

	अत्थाय (p = str; p && *p && isxdigit((unsigned वर्ण) *p); p++);

	सचे (end)
		*end = p;

	पटिनिवत्त p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
पूर्णाङ्क ul_strtobool(ध्रुव वर्ण *str, bool *result)
{
	स्थिर ध्रुव वर्ण *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	स्थिर ध्रुव वर्ण *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	सचे (!str || !result)
		पटिनिवत्त -(errno = EINVAL);

	errno = 0;

	अत्थाय (i = 0; i < ARRAY_SIZE(bool_true); i++) {
		सचे (c_strcasecmp(str, bool_true[i]) == 0) {
			*result = true;
			पटिनिवत्त 0;
		}
	}

	अत्थाय (i = 0; i < ARRAY_SIZE(bool_false); i++) {
		सचे (c_strcasecmp(str, bool_false[i]) == 0) {
			*result = false;
			पटिनिवत्त 0;
		}
	}

	पटिनिवत्त -(errno = EINVAL);
}

पूर्णाङ्क ul_parse_switch(ध्रुव वर्ण *arg, ...)
{
	ध्रुव वर्ण *a, *b;
	va_list ap;

	va_start(ap, arg);
	करोहि {
		a = va_arg(ap, वर्ण *);
		सचे (!a)
			भङ्ग;
		b = va_arg(ap, वर्ण *);
		सचे (!b)
			भङ्ग;

		सचे (strcmp(arg, a) == 0) {
			va_end(ap);
			पटिनिवत्त 1;
		}

		सचे (strcmp(arg, b) == 0) {
			va_end(ap);
			पटिनिवत्त 0;
		}
	} याव (1);
	va_end(ap);

	errx(STRTOXX_EXIT_CODE, _("unsupported argument: %s"), arg);
}

#ifndef HAVE_MEMPCPY
शून्य *mempcpy(शून्य *restrict dest, ध्रुव शून्य *restrict src, size_t n)
{
	पटिनिवत्त ((वर्ण *)memcpy(dest, src, n)) + n;
}
#endif

#ifndef HAVE_STRNLEN
size_t strnlen(ध्रुव वर्ण *s, size_t maxlen)
{
	size_t i;

	अत्थाय (i = 0; i < maxlen; i++) {
		सचे (s[i] == '\0')
			पटिनिवत्त i;
	}
	पटिनिवत्त maxlen;
}
#endif

#ifndef HAVE_STRNCHR
वर्ण *strnchr(ध्रुव वर्ण *s, size_t maxlen, पूर्णाङ्क c)
{
	अत्थाय (; maxlen-- && *s != '\0'; ++s)
		सचे (*s == (वर्ण)c)
			पटिनिवत्त (वर्ण *)s;
	पटिनिवत्त रिक्त;
}
#endif

#ifndef HAVE_STRNDUP
वर्ण *strndup(ध्रुव वर्ण *s, size_t n)
{
	size_t len = strnlen(s, n);
	वर्ण *new = malloc((len + 1) * परिमाण(वर्ण));
	सचे (!new)
		पटिनिवत्त रिक्त;
	new[len] = '\0';
	पटिनिवत्त (वर्ण *) memcpy(new, s, len);
}
#endif

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
पूर्णाङ्क ul_strtos64(ध्रुव वर्ण *str, int64_t *अङ्कः, पूर्णाङ्क base)
{
	वर्ण *end = रिक्त;

	सचे (str == रिक्त || *str == '\0')
		पटिनिवत्त -(errno = EINVAL);

	errno = 0;
	*अङ्कः = (int64_t) strtoimax(str, &end, base);

	सचे (errno != 0)
		पटिनिवत्त -errno;
	सचे (str == end || (end && *end))
		पटिनिवत्त -(errno = EINVAL);
	पटिनिवत्त 0;
}

पूर्णाङ्क ul_strtou64(ध्रुव वर्ण *str, uint64_t *अङ्कः, पूर्णाङ्क base)
{
	वर्ण *end = रिक्त;
	int64_t tmp;

	सचे (str == रिक्त || *str == '\0')
		पटिनिवत्त -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &end, base);
	सचे (tmp < 0)
		errno = ERANGE;
	अञ्ञथा {
		errno = 0;
		*अङ्कः = strtoumax(str, &end, base);
	}

	सचे (errno != 0)
		पटिनिवत्त -errno;
	सचे (str == end || (end && *end))
		पटिनिवत्त -(errno = EINVAL);
	पटिनिवत्त 0;
}

पूर्णाङ्क ul_strtos32(ध्रुव वर्ण *str, int32_t *अङ्कः, पूर्णाङ्क base)
{
	int64_t tmp;
	पूर्णाङ्क rc;

	rc = ul_strtos64(str, &tmp, base);
	सचे (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	सचे (rc == 0)
		*अङ्कः = (int32_t) tmp;
	पटिनिवत्त rc;
}

पूर्णाङ्क ul_strtou32(ध्रुव वर्ण *str, uint32_t *अङ्कः, पूर्णाङ्क base)
{
	uint64_t tmp;
	पूर्णाङ्क rc;

	rc = ul_strtou64(str, &tmp, base);
	सचे (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	सचे (rc == 0)
		*अङ्कः = (uint32_t) tmp;
	पटिनिवत्त rc;
}

पूर्णाङ्क ul_strtou16(ध्रुव वर्ण *str, uint16_t *अङ्कः, पूर्णाङ्क base)
{
	uint64_t tmp;
	पूर्णाङ्क rc;

	rc = ul_strtou64(str, &tmp, base);
	सचे (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	सचे (rc == 0)
		*अङ्कः = (uint16_t) tmp;
	पटिनिवत्त rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(ध्रुव वर्ण *str, पूर्णाङ्क base, ध्रुव वर्ण *errmesg,
			     int64_t नीचः, int64_t up)
{
	int64_t अङ्कः = 0;
	पूर्णाङ्क rc;

	rc = ul_strtos64(str, &अङ्कः, base);
	सचे (rc == 0 && ((नीचः && अङ्कः < नीचः) || (up && अङ्कः > up)))
		rc = -(errno = ERANGE);

	सचे (rc) {
		सचे (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	पटिनिवत्त अङ्कः;
}

uint64_t str2unum_or_err(ध्रुव वर्ण *str, पूर्णाङ्क base, ध्रुव वर्ण *errmesg, uint64_t up)
{
	uint64_t अङ्कः = 0;
	पूर्णाङ्क rc;

	rc = ul_strtou64(str, &अङ्कः, base);
	सचे (rc == 0 && (up && अङ्कः > up))
		rc = -(errno = ERANGE);

	सचे (rc) {
		सचे (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	पटिनिवत्त अङ्कः;
}

द्विगुण strtod_or_err(ध्रुव वर्ण *str, ध्रुव वर्ण *errmesg)
{
	द्विगुण अङ्कः;
	वर्ण *end = रिक्त;

	errno = 0;
	सचे (str == रिक्त || *str == '\0')
		गच्छ err;
	अङ्कः = strtod(str, &end);

	सचे (errno || str == end || (end && *end))
		गच्छ err;

	पटिनिवत्त अङ्कः;
err:
	सचे (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

पूर्णाङ्क ul_strtold(ध्रुव वर्ण *str, long द्विगुण *अङ्कः)
{
	वर्ण *end = रिक्त;

	errno = 0;
	सचे (str == रिक्त || *str == '\0')
		पटिनिवत्त -(errno = EINVAL);
	*अङ्कः = strtold(str, &end);

	सचे (errno != 0)
		पटिनिवत्त -errno;
	सचे (str == end || (end && *end))
		पटिनिवत्त -(errno = EINVAL);
	पटिनिवत्त 0;
}

long द्विगुण strtold_or_err(ध्रुव वर्ण *str, ध्रुव वर्ण *errmesg)
{
	long द्विगुण अङ्कः = 0;

	सचे (ul_strtold(str, &अङ्कः) == 0)
		पटिनिवत्त अङ्कः;
	सचे (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(ध्रुव वर्ण *str, ध्रुव वर्ण *errmesg)
{
	uintmax_t अङ्कः;

	सचे (strtosize(str, &अङ्कः) == 0)
		पटिनिवत्त अङ्कः;

	सचे (errno)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}


शून्य strtotimeval_or_err(ध्रुव वर्ण *str, संरचना timeval *tv, ध्रुव वर्ण *errmesg)
{
	long द्विगुण user_निवेशः;

	user_निवेशः = strtold_or_err(str, errmesg);
	tv->tv_sec = (time_t) user_निवेशः;
	tv->tv_usec = (suseconds_t)((user_निवेशः - tv->tv_sec) * USEC_PER_SEC);
}

शून्य strtotimespec_or_err(ध्रुव वर्ण *str, संरचना timespec *ts, ध्रुव वर्ण *errmesg)
{
	long द्विगुण user_निवेशः;

	user_निवेशः = strtold_or_err(str, errmesg);
	ts->tv_sec = (time_t) user_निवेशः;
	ts->tv_nsec = (long)((user_निवेशः - ts->tv_sec) * NSEC_PER_SEC);
}

time_t strtotime_or_err(ध्रुव वर्ण *str, ध्रुव वर्ण *errmesg)
{
	int64_t user_निवेशः;

	user_निवेशः = strtos64_or_err(str, errmesg);
	पटिनिवत्त (time_t) user_निवेशः;
}

bool hyperlinkwanted(ध्रुव वर्ण *mode)
{
	सचे (mode && strcmp(mode, "never") == 0)
		पटिनिवत्त false;

	सचे (mode && strcmp(mode, "always") == 0)
		पटिनिवत्त true;

	सचे (!mode || strcmp(mode, "auto") == 0)
		पटिनिवत्त isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), mode);
}

bool annotationwanted(ध्रुव वर्ण *mode)
{
	सचे (mode && strcmp(mode, "never") == 0)
		पटिनिवत्त false;

	सचे (mode && strcmp(mode, "always") == 0)
		पटिनिवत्त true;

	सचे (!mode || strcmp(mode, "auto") == 0)
		पटिनिवत्त isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), mode);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
वर्ण *xstrmode(mode_t mode, वर्ण *str)
{
	unsigned short i = 0;

	सचे (S_ISDIR(mode))
		str[i++] = 'd';
	अञ्ञथा सचे (S_ISLNK(mode))
		str[i++] = 'l';
	अञ्ञथा सचे (S_ISCHR(mode))
		str[i++] = 'c';
	अञ्ञथा सचे (S_ISBLK(mode))
		str[i++] = 'b';
	अञ्ञथा सचे (S_ISSOCK(mode))
		str[i++] = 's';
	अञ्ञथा सचे (S_ISFIFO(mode))
		str[i++] = 'p';
	अञ्ञथा सचे (S_ISREG(mode))
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

	पटिनिवत्त str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
स्थिर पूर्णाङ्क get_exp(uint64_t n)
{
	पूर्णाङ्क shft;

	अत्थाय (shft = 10; shft <= 60; shft += 10) {
		सचे (n < (1ULL << shft))
			भङ्ग;
	}
	पटिनिवत्त shft - 10;
}

वर्ण *size_to_human_string(पूर्णाङ्क options, uint64_t bytes)
{
	वर्ण buf[32];
	पूर्णाङ्क dec, exp;
	uint64_t frac;
	ध्रुव वर्ण *letters = "BKMGTPE";
	वर्ण suffix[परिमाण(" KiB")], *psuf = suffix;
	वर्ण c;

	सचे (options & SIZE_SUFFIX_SPACE)
		*psuf++ = ' ';


	exp  = get_exp(bytes);
	c    = *(letters + (exp ? exp / 10 : 0));
	dec  = exp ? bytes / (1ULL << exp) : bytes;
	frac = exp ? bytes % (1ULL << exp) : 0;

	*psuf++ = c;

	सचे ((options & SIZE_SUFFIX_3LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/* round */
	सचे (frac) {
		/* get 3 digits after decimal point */
		सचे (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (exp - 10)) ;
		अञ्ञथा
			frac = (frac * 1000) / (1ULL << (exp)) ;

		सचे (options & SIZE_DECIMAL_2DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} अञ्ञथा {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		सचे (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	सचे (frac) {
		संरचना lconv ध्रुव *l = localeconv();
		वर्ण *dp = l ? l->decimal_point : रिक्त;
		पूर्णाङ्क len;

		सचे (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, परिमाण(buf), "%d%s%02" PRIu64, dec, dp, frac);
		सचे (len > 0 && (size_t) len < परिमाण(buf)) {
			/* remove potential extraneous zero */
			सचे (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, परिमाण(buf) - len);
		} अञ्ञथा
			*buf = '\0';	/* snprintf error */
	} अञ्ञथा
		snprintf(buf, परिमाण(buf), "%d%s", dec, suffix);

	पटिनिवत्त strdup(buf);
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
पूर्णाङ्क string_to_idarray(ध्रुव वर्ण *list, पूर्णाङ्क ary[], size_t arysz,
			पूर्णाङ्क (name2id)(ध्रुव वर्ण *, size_t))
{
	ध्रुव वर्ण *begin = रिक्त, *p;
	size_t n = 0;

	सचे (!list || !*list || !ary || !arysz || !name2id)
		पटिनिवत्त -1;

	अत्थाय (p = list; p && *p; p++) {
		ध्रुव वर्ण *end = रिक्त;
		पूर्णाङ्क id;

		सचे (n >= arysz)
			पटिनिवत्त -2;
		सचे (!begin)
			begin = p;		/* begin of the column name */
		सचे (*p == ',')
			end = p;		/* terminate the name */
		सचे (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		सचे (!begin || !end)
			अनुवर्त;
		सचे (end <= begin)
			पटिनिवत्त -1;

		id = name2id(begin, end - begin);
		सचे (id == -1)
			पटिनिवत्त -1;
		ary[ n++ ] = id;
		begin = रिक्त;
		सचे (end && !*end)
			भङ्ग;
	}
	पटिनिवत्त n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
पूर्णाङ्क string_add_to_idarray(ध्रुव वर्ण *list, पूर्णाङ्क ary[], size_t arysz,
			size_t *ary_pos, पूर्णाङ्क (name2id)(ध्रुव वर्ण *, size_t))
{
	ध्रुव वर्ण *list_add;
	पूर्णाङ्क r;

	सचे (!list || !*list || !ary_pos || *ary_pos > arysz)
		पटिनिवत्त -1;

	सचे (list[0] == '+')
		list_add = &list[1];
	अञ्ञथा {
		list_add = list;
		*ary_pos = 0;
	}

	r = string_to_idarray(list_add, &ary[*ary_pos], arysz - *ary_pos, name2id);
	सचे (r > 0)
		*ary_pos += r;
	पटिनिवत्त r;
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
पूर्णाङ्क string_to_bitarray(ध्रुव वर्ण *list,
		     वर्ण *ary,
		     पूर्णाङ्क (*name2bit)(ध्रुव वर्ण *, size_t),
		     size_t allow_range)
{
	ध्रुव वर्ण *begin = रिक्त, *p;

	सचे (!list || !name2bit || !ary)
		पटिनिवत्त -EINVAL;

	अत्थाय (p = list; p && *p; p++) {
		ध्रुव वर्ण *end = रिक्त;
		पूर्णाङ्क bit, set_lower = 0, set_higher = 0;

		सचे (!begin)
			begin = p;		/* begin of the level name */
		सचे (*p == ',')
			end = p;		/* terminate the name */
		सचे (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		सचे (!begin || !end)
			अनुवर्त;
		सचे (end <= begin)
			पटिनिवत्त -1;
		सचे (allow_range) {
			सचे (*(end - 1) == '+') {
				end--;
				set_lower = 1;
			} अञ्ञथा सचे (*begin == '+') {
				begin++;
				set_higher = 1;
			}
		}

		bit = name2bit(begin, end - begin);
		सचे (bit < 0)
			पटिनिवत्त bit;
		setbit(ary, bit);
		सचे (set_lower)
			याव (--bit >= 0)
				setbit(ary, bit);
		अञ्ञथा सचे (set_higher)
			याव (++bit < (पूर्णाङ्क) allow_range)
				setbit(ary, bit);
		begin = रिक्त;
		सचे (end && !*end)
			भङ्ग;
	}
	पटिनिवत्त 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
पूर्णाङ्क string_to_bitmask(ध्रुव वर्ण *list,
		     unsigned long *mask,
		     long (*name2flag)(ध्रुव वर्ण *, size_t))
{
	ध्रुव वर्ण *begin = रिक्त, *p;

	सचे (!list || !name2flag || !mask)
		पटिनिवत्त -EINVAL;

	अत्थाय (p = list; p && *p; p++) {
		ध्रुव वर्ण *end = रिक्त;
		long flag;

		सचे (!begin)
			begin = p;		/* begin of the level name */
		सचे (*p == ',')
			end = p;		/* terminate the name */
		सचे (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		सचे (!begin || !end)
			अनुवर्त;
		सचे (end <= begin)
			पटिनिवत्त -1;

		flag = name2flag(begin, end - begin);
		सचे (flag < 0)
			पटिनिवत्त flag;	/* error */
		*mask |= flag;
		begin = रिक्त;
		सचे (end && !*end)
			भङ्ग;
	}
	पटिनिवत्त 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
पूर्णाङ्क ul_parse_range(ध्रुव वर्ण *str, पूर्णाङ्क *lower, पूर्णाङ्क *upper, पूर्णाङ्क def)
{
	वर्ण *end = रिक्त;

	सचे (!str)
		पटिनिवत्त 0;

	*upper = *lower = def;
	errno = 0;

	सचे (*str == ':') {				/* <:N> */
		str++;
		*upper = strtol(str, &end, 10);
		सचे (errno || !end || *end || end == str)
			पटिनिवत्त -1;
	} अञ्ञथा {
		*upper = *lower = strtol(str, &end, 10);
		सचे (errno || !end || end == str)
			पटिनिवत्त -1;

		सचे (*end == ':' && !*(end + 1))		/* <M:> */
			*upper = def;
		अञ्ञथा सचे (*end == '-' || *end == ':') {	/* <M:N> <M-N> */
			str = end + 1;
			end = रिक्त;
			errno = 0;
			*upper = strtol(str, &end, 10);

			सचे (errno || !end || *end || end == str)
				पटिनिवत्त -1;
		}
	}
	पटिनिवत्त 0;
}

ध्रुव वर्ण *ul_path_next_segment(ध्रुव वर्ण *str, size_t *sz)
{
	ध्रुव वर्ण *start, *p;

	start = str;
	*sz = 0;
	याव (start && *start == '/' && *(start + 1) == '/')
		start++;

	सचे (!start || !*start)
		पटिनिवत्त रिक्त;

	अत्थाय (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	पटिनिवत्त start;
}

पूर्णाङ्क streq_paths(ध्रुव वर्ण *a, ध्रुव वर्ण *b)
{
	याव (a && b) {
		size_t a_sz, b_sz;
		ध्रुव वर्ण *a_seg = ul_path_next_segment(a, &a_sz);
		ध्रुव वर्ण *b_seg = ul_path_next_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		सचे (a_sz + b_sz == 0)
			पटिनिवत्त 1;

		/* ignore trailing slash */
		सचे (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			पटिनिवत्त 1;

		सचे (!a_seg || !b_seg)
			भङ्ग;
		सचे (a_sz != b_sz || strncmp(a_seg, b_seg, a_sz) != 0)
			भङ्ग;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	पटिनिवत्त 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
पूर्णाङ्क ul_path_is_within(ध्रुव वर्ण *path, ध्रुव वर्ण *dir)
{
	सचे (!path || !dir || !*path || !*dir)
		पटिनिवत्त 0;

	याव (path && dir) {
		size_t p_sz, d_sz;
		ध्रुव वर्ण *p_seg = ul_path_next_segment(path, &p_sz);
		ध्रुव वर्ण *d_seg = ul_path_next_segment(dir, &d_sz);

		/* all @dir segments matched */
		सचे (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			पटिनिवत्त 1;

		सचे (!p_seg || !d_seg)
			भङ्ग;
		सचे (p_sz != d_sz || strncmp(p_seg, d_seg, p_sz) != 0)
			भङ्ग;

		path = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	पटिनिवत्त 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
वर्ण *ul_strnconcat(ध्रुव वर्ण *s, ध्रुव वर्ण *suffix, size_t b)
{
	size_t a;
	वर्ण *r;

	सचे (!s && !suffix)
		पटिनिवत्त strdup("");
	सचे (!s)
		पटिनिवत्त strndup(suffix, b);
	सचे (!suffix)
		पटिनिवत्त strdup(s);

	प्रतिज्ञा(s);
	प्रतिज्ञा(suffix);

	a = strlen(s);
	सचे (b > ((size_t) -1) - a)
		पटिनिवत्त रिक्त;

	r = malloc(a + b + 1);
	सचे (!r)
		पटिनिवत्त रिक्त;

	memcpy(r, s, a);
	memcpy(r + a, suffix, b);
	r[a+b] = 0;

	पटिनिवत्त r;
}

/* concatenate two strings to a new string */
वर्ण *ul_strconcat(ध्रुव वर्ण *s, ध्रुव वर्ण *suffix)
{
	पटिनिवत्त ul_strnconcat(s, suffix, suffix ? strlen(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
वर्ण *ul_strfconcat(ध्रुव वर्ण *s, ध्रुव वर्ण *format, ...)
{
	va_list ap;
	वर्ण *val, *res;
	पूर्णाङ्क sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	सचे (sz < 0)
		पटिनिवत्त रिक्त;

	res = ul_strnconcat(s, val, sz);
	मुक्त(val);
	पटिनिवत्त res;
}

पूर्णाङ्क ul_strappend(वर्ण **a, ध्रुव वर्ण *b)
{
	size_t al, bl;
	वर्ण *tmp;

	सचे (!a)
		पटिनिवत्त -EINVAL;
	सचे (!b || !*b)
		पटिनिवत्त 0;
	सचे (!*a) {
		*a = strdup(b);
		पटिनिवत्त !*a ? -ENOMEM : 0;
	}

	al = strlen(*a);
	bl = strlen(b);

	tmp = realloc(*a, al + bl + 1);
	सचे (!tmp)
		पटिनिवत्त -ENOMEM;
	*a = tmp;
	memcpy((*a) + al, b, bl + 1);
	पटिनिवत्त 0;
}

/* the hybrid version of strfconcat and strappend. */
पूर्णाङ्क strfappend(वर्ण **a, ध्रुव वर्ण *format, ...)
{
	va_list ap;
	पूर्णाङ्क res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	पटिनिवत्त res;
}

बाह्य पूर्णाङ्क ul_strvfappend(वर्ण **a, ध्रुव वर्ण *format, va_list ap)
{
	वर्ण *val;
	पूर्णाङ्क sz;
	पूर्णाङ्क res;

	sz = vasprintf(&val, format, ap);
	सचे (sz < 0)
		पटिनिवत्त -errno;

	res = ul_strappend(a, val);
	मुक्त(val);
	पटिनिवत्त res;
}

स्थिर size_t strcspn_escaped(ध्रुव वर्ण *s, ध्रुव वर्ण *reject)
{
	पूर्णाङ्क escaped = 0;
	पूर्णाङ्क n;

	अत्थाय (n=0; s[n]; n++) {
		सचे (escaped)
			escaped = 0;
		अञ्ञथा सचे (s[n] == '\\')
			escaped = 1;
		अञ्ञथा सचे (strchr(reject, s[n]))
			भङ्ग;
	}

	/* if s ends in \, return index of previous char */
	पटिनिवत्त n - escaped;
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
वर्ण *ul_strchr_escaped(ध्रुव वर्ण *s, पूर्णाङ्क c)
{
	वर्ण *p;
	पूर्णाङ्क esc = 0;

	अत्थाय (p = (वर्ण *) s; p && *p; p++) {
		सचे (!esc && *p == '\\') {
			esc = 1;
			अनुवर्त;
		}
		सचे (*p == c && (!esc || c == '\\'))
			पटिनिवत्त p;
		esc = 0;
	}

	पटिनिवत्त रिक्त;
}

/* Split a string into words. */
ध्रुव वर्ण *ul_split(ध्रुव वर्ण **state, size_t *l, ध्रुव वर्ण *separator, पूर्णाङ्क quoted)
{
	ध्रुव वर्ण *current;

	current = *state;

	सचे (!*current) {
		प्रतिज्ञा(**state == '\0');
		पटिनिवत्त रिक्त;
	}

	current += strspn(current, separator);
	सचे (!*current) {
		*state = current;
		पटिनिवत्त रिक्त;
	}

	सचे (quoted && strchr("\'\"", *current)) {
		वर्ण quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		सचे (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			पटिनिवत्त रिक्त;
		}
		*state = current++ + *l + 2;
	} अञ्ञथा सचे (quoted) {
		*l = strcspn_escaped(current, separator);
		सचे (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			पटिनिवत्त रिक्त;
		}
		*state = current + *l;
	} अञ्ञथा {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	पटिनिवत्त current;
}

/* Rewind file pointer forward to new line.  */
पूर्णाङ्क skip_fline(FILE *fp)
{
	पूर्णाङ्क ch;

	करोहि {
		सचे ((ch = fgetc(fp)) == EOF)
			पटिनिवत्त 1;
		सचे (ch == '\n')
			पटिनिवत्त 0;
	} याव (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
पूर्णाङ्क ul_stralnumcmp(ध्रुव वर्ण *p1, ध्रुव वर्ण *p2)
{
	ध्रुव unsigned वर्ण *s1 = (ध्रुव unsigned वर्ण *) p1;
	ध्रुव unsigned वर्ण *s2 = (ध्रुव unsigned वर्ण *) p2;
	unsigned वर्ण c1, c2;

	करोहि {
		करोहि {
			c1 = (unsigned वर्ण) *s1++;
		} याव (c1 != '\0' && !isalnum((unsigned पूर्णाङ्क) c1));

		करोहि {
			c2 = (unsigned वर्ण) *s2++;
		} याव (c2 != '\0' && !isalnum((unsigned पूर्णाङ्क) c2));

		सचे (c1 != '\0')
			c1 = tolower(c1);
		सचे (c2 != '\0')
			c2 = tolower(c2);
		सचे (c1 == '\0')
			पटिनिवत्त c1 - c2;
	} याव (c1 == c2);

	पटिनिवत्त c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
पूर्णाङ्क ul_optstr_next(वर्ण **optstr, वर्ण **name, size_t *namesz,
		   वर्ण **मानम्, size_t *valsz)
{
	पूर्णाङ्क open_quote = 0;
	वर्ण *start = रिक्त, *stop = रिक्त, *p, *sep = रिक्त;
	वर्ण *optstr0;

	प्रतिज्ञा(optstr);
	प्रतिज्ञा(*optstr);

	optstr0 = *optstr;

	सचे (name)
		*name = रिक्त;
	सचे (namesz)
		*namesz = 0;
	सचे (मानम्)
		*मानम् = रिक्त;
	सचे (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	याव (optstr0 && *optstr0 == ',')
		optstr0++;

	अत्थाय (p = optstr0; p && *p; p++) {
		सचे (!start && *p == '=')
			पटिनिवत्त -EINVAL;
		सचे (!start)
			start = p;		/* beginning of the option item */
		सचे (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		सचे (open_quote)
			अनुवर्त;		/* still in quoted block */
		सचे (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		सचे (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		अञ्ञथा सचे (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		सचे (!start || !stop)
			अनुवर्त;
		सचे (stop <= start)
			पटिनिवत्त -EINVAL;

		सचे (name)
			*name = start;
		सचे (namesz)
			*namesz = sep ? sep - start : stop - start;
		*optstr = *stop ? stop + 1 : stop;

		सचे (sep) {
			सचे (मानम्)
				*मानम् = sep + 1;
			सचे (valsz)
				*valsz = stop - sep - 1;
		}
		पटिनिवत्त 0;
	}

	पटिनिवत्त 1;				/* end of optstr */
}

पूर्णाङ्क ul_optstr_is_valid(ध्रुव वर्ण *optstr)
{
	पूर्णाङ्क rc;
	वर्ण *p = (वर्ण *) optstr;

	याव ((rc = ul_optstr_next(&p, रिक्त, रिक्त, रिक्त, रिक्त)) == 0);
	पटिनिवत्त rc < 0 ? 0 : 1;
}

वर्ण *ul_optstr_get_मानम्(ध्रुव वर्ण *optstr, ध्रुव वर्ण *कुञ्चिका)
{
	size_t sz, namesz = 0, valsz = 0;
	वर्ण *name = रिक्त, *मानम् = रिक्त;
	वर्ण *p = (वर्ण *) optstr;

	सचे (!optstr || !कुञ्चिका || !*कुञ्चिका)
		पटिनिवत्त रिक्त;

	sz = strlen(कुञ्चिका);
	याव (ul_optstr_next(&p, &name, &namesz, &मानम्, &valsz) == 0) {
		सचे (namesz != sz || !valsz)
			अनुवर्त;
		सचे (strncmp(name, कुञ्चिका, namesz) == 0)
			पटिनिवत्त strndup(मानम्, valsz);
	}
	पटिनिवत्त रिक्त;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(वर्ण *str)
{
	size_t len;
	वर्ण q;

	सचे (!str || !*str)
		पटिनिवत्त 0;

	len = strlen(str);
	q = *str;

	सचे ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		पटिनिवत्त len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	पटिनिवत्त len - 2;
}

#ifdef TEST_PROGRAM_STRUTILS

संरचना testS {
	वर्ण *name;
	वर्ण *मानम्;
};

स्थिर पूर्णाङ्क test_strdup_to_member(पूर्णाङ्क argc, वर्ण *argv[])
{
	संरचना testS *xx;

	सचे (argc < 3)
		पटिनिवत्त EXIT_FAILURE;

	xx = calloc(1, परिमाण(*xx));
	सचे (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_to_struct_member(xx, name, argv[1]);
	strdup_to_struct_member(xx, मानम्, argv[2]);

	सचे (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->मानम्, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	printf("1: '%s', 2: '%s'\n", xx->name, xx->मानम्);

	मुक्त(xx->name);
	मुक्त(xx->मानम्);
	मुक्त(xx);
	पटिनिवत्त EXIT_SUCCESS;
}

स्थिर पूर्णाङ्क test_strutils_sizes(पूर्णाङ्क argc, वर्ण *argv[])
{
	uintmax_t size = 0;
	वर्ण *hum1, *hum2, *hum3;

	सचे (argc < 2)
		पटिनिवत्त EXIT_FAILURE;

	सचे (strtosize(argv[1], &size))
		errx(EXIT_FAILURE, "invalid size '%s' value", argv[1]);

	hum1 = size_to_human_string(SIZE_SUFFIX_1LETTER, size);
	hum2 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE, size);
	hum3 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE |
				    SIZE_DECIMAL_2DIGITS, size);

	printf("%25s : %20ju : %8s : %12s : %13s\n", argv[1], size, hum1, hum2, hum3);
	मुक्त(hum1);
	मुक्त(hum2);
	मुक्त(hum3);

	पटिनिवत्त EXIT_SUCCESS;
}

स्थिर पूर्णाङ्क test_strutils_cmp_paths(पूर्णाङ्क argc, वर्ण *argv[])
{
	पूर्णाङ्क rc = streq_paths(argv[1], argv[2]);

	सचे (argc < 3)
		पटिनिवत्त EXIT_FAILURE;

	printf("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	पटिनिवत्त EXIT_SUCCESS;
}

स्थिर पूर्णाङ्क test_strutils_path_is_within(पूर्णाङ्क argc, वर्ण *argv[])
{
	पूर्णाङ्क rc;

	सचे (argc < 3)
		पटिनिवत्त EXIT_FAILURE;

	rc = ul_path_is_within(argv[1], argv[2]);

	printf("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	पटिनिवत्त EXIT_SUCCESS;
}

स्थिर पूर्णाङ्क test_strutils_normalize(पूर्णाङ्क argc, वर्ण *argv[])
{
	unsigned वर्ण *src, *dst, *org;
	size_t sz, len;

	सचे (argc < 2)
		पटिनिवत्त EXIT_FAILURE;

	org = (unsigned वर्ण *) strdup(argv[1]);
	src = (unsigned वर्ण *) strdup((वर्ण *) org);
	len = strlen((वर्ण *) src);
	dst = malloc(len + 1);

	सचे (!org || !src || !dst)
		गच्छ done;

	/* two buffers */
	sz = __normalize_whitespace(src, len, dst, len + 1);
	printf("1: '%s' --> '%s' [sz=%zu]\n", src, dst, sz);

	/* one buffer */
	sz = normalize_whitespace(src);
	printf("2: '%s' --> '%s' [sz=%zu]\n", org, src, sz);

done:
	मुक्त(src);
	मुक्त(dst);
	मुक्त(org);

	पटिनिवत्त EXIT_SUCCESS;
}

स्थिर पूर्णाङ्क test_strutils_cstrcasecmp(पूर्णाङ्क argc, वर्ण *argv[])
{
	वर्ण *a, *b;

	सचे (argc < 3)
		पटिनिवत्त EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	सचे (!a || !b)
		पटिनिवत्त EXIT_FAILURE;

	printf("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	printf("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	printf("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, strlen(a)));

	पटिनिवत्त EXIT_SUCCESS;
}

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	सचे (argc == 3 && strcmp(argv[1], "--size") == 0) {
		पटिनिवत्त test_strutils_sizes(argc - 1, argv + 1);

	} अञ्ञथा सचे (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		printf("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						रिक्त));
		पटिनिवत्त EXIT_SUCCESS;
	} अञ्ञथा सचे (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		सचे (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		printf("'%s'-->%s\n", argv[2], val ? "true" : "false");
		पटिनिवत्त EXIT_SUCCESS;

	} अञ्ञथा सचे (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		पटिनिवत्त test_strutils_cmp_paths(argc - 1, argv + 1);

	} अञ्ञथा सचे (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		पटिनिवत्त test_strutils_path_is_within(argc - 1, argv + 1);

	} अञ्ञथा सचे (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		पटिनिवत्त test_strdup_to_member(argc - 1, argv + 1);

	} अञ्ञथा सचे  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		printf("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		पटिनिवत्त EXIT_SUCCESS;

	} अञ्ञथा सचे (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		पटिनिवत्त test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} अञ्ञथा सचे (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		पटिनिवत्त test_strutils_normalize(argc - 1, argv + 1);

	} अञ्ञथा सचे (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		printf("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		पटिनिवत्त EXIT_SUCCESS;
	} अञ्ञथा सचे (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		printf("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		पटिनिवत्त EXIT_SUCCESS;
	} अञ्ञथा सचे (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		printf("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		पटिनिवत्त EXIT_SUCCESS;
	} अञ्ञथा सचे (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		printf("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		पटिनिवत्त EXIT_SUCCESS;
	} अञ्ञथा सचे (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		printf("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		पटिनिवत्त EXIT_SUCCESS;
	} अञ्ञथा सचे (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		printf("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		पटिनिवत्त EXIT_SUCCESS;

	} अञ्ञथा सचे (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		printf("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		पटिनिवत्त EXIT_SUCCESS;

	} अञ्ञथा सचे (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		वर्ण *buf = "abc\0Y\0\0xyz\0X";
		वर्ण *end = buf + 12;
		वर्ण *p = buf;

		करोहि {
			printf("str: '%s'\n", p);
		} याव ((p = ul_next_string(p, end)));

		पटिनिवत्त EXIT_SUCCESS;

	} अञ्ञथा सचे (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		वर्ण *name = रिक्त, *val = रिक्त;
		वर्ण *p = argv[2];
		पूर्णाङ्क rc;

		सचे (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		याव ((rc = ul_optstr_next(&p, &name, &namesz, &val, &valsz)) == 0) {
			printf("'%.*s' : '%.*s'\n", (पूर्णाङ्क) namesz, name,
						   (पूर्णाङ्क) valsz, val);
		}
		सचे (rc == 1)
			पटिनिवत्त EXIT_SUCCESS;
	} अञ्ञथा सचे (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		वर्ण *unquoted = strdup(argv[2]);

		सचे (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		printf("%s-->%s\n", argv[2], unquoted);
		मुक्त(unquoted);
		पटिनिवत्त EXIT_SUCCESS;

	} अञ्ञथा {
		fprintf(stderr, "usage: %1$s --size <number>[suffix]\n"
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
		exit(EXIT_FAILURE);
	}

	पटिनिवत्त EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_STRUTILS */
