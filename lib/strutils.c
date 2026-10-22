/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Karel Zak <kzak@redhat.com> [2010]
 *          Davidlohr Bueso <dave@gnu.org> [2010]
 */
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <inttypes.h>
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <strings.h>
#অন্তর্ভাব <assert.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "cctype.h"
#অন্তর্ভাব "nls.h"
#অন্তর্ভাব "strutils.h"
#অন্তর্ভাব "timeutils.h"
#অন্তর্ভাব "bitops.h"
#অন্তর্ভাব "pathnames.h"

স্থির পূর্ণসংখ্যা STRTOXX_EXIT_CODE = EXIT_FAILURE;

শূন্য strutils_set_exitcode(পূর্ণসংখ্যা ex)
{
	STRTOXX_EXIT_CODE = ex;
}

স্থির পূর্ণসংখ্যা do_scale_by_power (uintmax_t *x, পূর্ণসংখ্যা base, পূর্ণসংখ্যা power)
{
	যখন (power--) {
		যদি (UINTMAX_MAX / base < *x)
			ফেরত -ERANGE;
		*x *= base;
	}
	ফেরত 0;
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
পূর্ণসংখ্যা ul_parse_আকার(ধ্রুবক অক্ষর *str, uintmax_t *res, পূর্ণসংখ্যা *power)
{
	ধ্রুবক অক্ষর *p;
	অক্ষর *end;
	uintmax_t x, frac = 0;
	পূর্ণসংখ্যা base = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	স্থির ধ্রুবক অক্ষর *suf  = "KMGTPEZY";
	স্থির ধ্রুবক অক্ষর *suf2 = "kmgtpezy";
	ধ্রুবক অক্ষর *sp;

	*res = 0;

	যদি (!str || !*str) {
		rc = -EINVAL;
		যাও err;
	}

	/* Only positive numbers are acceptable
	 *
	 * Note that this check is not perfect, it would be better to
	 * use lconv->negative_sign. But coreutils use the same solution,
	 * so it's probably good enough...
	 */
	p = str;
	যখন (isspace((unsigned অক্ষর) *p))
		p++;
	যদি (*p == '-') {
		rc = -EINVAL;
		যাও err;
	}

	errno = 0, end = ফাঁকা;
	x = strtoumax(str, &end, 0);

	যদি (end == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		যাও err;
	}
	যদি (!end || !*end)
		যাও done;			/* without suffix */
	p = end;

	/*
	 * Check size suffixes
	 */
check_suffix:
	যদি (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		base = 1024;			/* XiB, 2^N */
	অন্যথা যদি ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		base = 1000;			/* XB, 10^N */
	অন্যথা যদি (*(p + 1)) {
		কাঠামো lconv ধ্রুবক *l = localeconv();
		ধ্রুবক অক্ষর *dp = l ? l->decimal_point : ফাঁকা;
		size_t dpsz = dp ? strlen(dp) : 0;

		যদি (frac == 0 && *p && dp && strncmp(dp, p, dpsz) == 0) {
			ধ্রুবক অক্ষর *fstr = p + dpsz;

			জন্য (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			যদি (isdigit(*fstr)) {
				errno = 0, end = ফাঁকা;
				frac = strtoumax(fstr, &end, 0);
				যদি (end == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					যাও err;
				}
			} অন্যথা
				end = (অক্ষর *) p;

			যদি (!end || !*end) {
				rc = -EINVAL;
				যাও err;	/* without suffix, but with fractions */
			}
			p = end;
			যাও check_suffix;
		}
		rc = -EINVAL;
		যাও err;			/* unexpected suffix */
	}

	sp = strchr(suf, *p);
	যদি (sp)
		pwr = (sp - suf) + 1;
	অন্যথা {
		sp = strchr(suf2, *p);
		যদি (sp)
			pwr = (sp - suf2) + 1;
		অন্যথা {
			rc = -EINVAL;
			যাও err;
		}
	}

	যদি (power)
		*power = pwr;

	rc = do_scale_by_power(&x, base, pwr);
	যদি (rc)
		যাও err;

	যদি (frac && pwr) {
		পূর্ণসংখ্যা i;
		uintmax_t frac_base = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_base, base, pwr);
		যদি (rc)
			যাও err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		করো {
			frac_val = ((frac % 10) * frac_base + frac_val) / 10;
			frac /= 10;
		} যখন (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		জন্য (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		যদি (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			যাও err;
		}
		x += frac_val;
	}
done:
	*res = x;
err:
	যদি (rc < 0)
		errno = -rc;
	ফেরত rc;
}

পূর্ণসংখ্যা strtosize(ধ্রুবক অক্ষর *str, uintmax_t *res)
{
	ফেরত ul_parse_আকার(str, res, ফাঁকা);
}

পূর্ণসংখ্যা isdigit_strend(ধ্রুবক অক্ষর *str, ধ্রুবক অক্ষর **end)
{
	ধ্রুবক অক্ষর *p;

	জন্য (p = str; p && *p && isdigit((unsigned অক্ষর) *p); p++);

	যদি (end)
		*end = p;
	ফেরত p && p > str && !*p;
}

পূর্ণসংখ্যা isxdigit_strend(ধ্রুবক অক্ষর *str, ধ্রুবক অক্ষর **end)
{
	ধ্রুবক অক্ষর *p;

	জন্য (p = str; p && *p && isxdigit((unsigned অক্ষর) *p); p++);

	যদি (end)
		*end = p;

	ফেরত p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
পূর্ণসংখ্যা ul_strtobool(ধ্রুবক অক্ষর *str, bool *ফলাফল)
{
	স্থির ধ্রুবক অক্ষর *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	স্থির ধ্রুবক অক্ষর *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	যদি (!str || !ফলাফল)
		ফেরত -(errno = EINVAL);

	errno = 0;

	জন্য (i = 0; i < ARRAY_আকার(bool_true); i++) {
		যদি (c_strcasecmp(str, bool_true[i]) == 0) {
			*ফলাফল = true;
			ফেরত 0;
		}
	}

	জন্য (i = 0; i < ARRAY_আকার(bool_false); i++) {
		যদি (c_strcasecmp(str, bool_false[i]) == 0) {
			*ফলাফল = false;
			ফেরত 0;
		}
	}

	ফেরত -(errno = EINVAL);
}

পূর্ণসংখ্যা ul_parse_switch(ধ্রুবক অক্ষর *arg, ...)
{
	ধ্রুবক অক্ষর *a, *b;
	va_list ap;

	va_start(ap, arg);
	করো {
		a = va_arg(ap, অক্ষর *);
		যদি (!a)
			বিরতি;
		b = va_arg(ap, অক্ষর *);
		যদি (!b)
			বিরতি;

		যদি (strcmp(arg, a) == 0) {
			va_end(ap);
			ফেরত 1;
		}

		যদি (strcmp(arg, b) == 0) {
			va_end(ap);
			ফেরত 0;
		}
	} যখন (1);
	va_end(ap);

	errx(STRTOXX_EXIT_CODE, _("unsupported argument: %s"), arg);
}

#ifndef HAVE_MEMPCPY
শূন্য *mempcpy(শূন্য *restrict dest, ধ্রুবক শূন্য *restrict src, size_t n)
{
	ফেরত ((অক্ষর *)memcpy(dest, src, n)) + n;
}
#endif

#ifndef HAVE_STRNLEN
size_t strnlen(ধ্রুবক অক্ষর *s, size_t maxlen)
{
	size_t i;

	জন্য (i = 0; i < maxlen; i++) {
		যদি (s[i] == '\0')
			ফেরত i;
	}
	ফেরত maxlen;
}
#endif

#ifndef HAVE_STRNCHR
অক্ষর *strnchr(ধ্রুবক অক্ষর *s, size_t maxlen, পূর্ণসংখ্যা c)
{
	জন্য (; maxlen-- && *s != '\0'; ++s)
		যদি (*s == (অক্ষর)c)
			ফেরত (অক্ষর *)s;
	ফেরত ফাঁকা;
}
#endif

#ifndef HAVE_STRNDUP
অক্ষর *strndup(ধ্রুবক অক্ষর *s, size_t n)
{
	size_t len = strnlen(s, n);
	অক্ষর *new = malloc((len + 1) * আকার(অক্ষর));
	যদি (!new)
		ফেরত ফাঁকা;
	new[len] = '\0';
	ফেরত (অক্ষর *) memcpy(new, s, len);
}
#endif

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
পূর্ণসংখ্যা ul_strtos64(ধ্রুবক অক্ষর *str, int64_t *অঙ্ক, পূর্ণসংখ্যা base)
{
	অক্ষর *end = ফাঁকা;

	যদি (str == ফাঁকা || *str == '\0')
		ফেরত -(errno = EINVAL);

	errno = 0;
	*অঙ্ক = (int64_t) strtoimax(str, &end, base);

	যদি (errno != 0)
		ফেরত -errno;
	যদি (str == end || (end && *end))
		ফেরত -(errno = EINVAL);
	ফেরত 0;
}

পূর্ণসংখ্যা ul_strtou64(ধ্রুবক অক্ষর *str, uint64_t *অঙ্ক, পূর্ণসংখ্যা base)
{
	অক্ষর *end = ফাঁকা;
	int64_t tmp;

	যদি (str == ফাঁকা || *str == '\0')
		ফেরত -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &end, base);
	যদি (tmp < 0)
		errno = ERANGE;
	অন্যথা {
		errno = 0;
		*অঙ্ক = strtoumax(str, &end, base);
	}

	যদি (errno != 0)
		ফেরত -errno;
	যদি (str == end || (end && *end))
		ফেরত -(errno = EINVAL);
	ফেরত 0;
}

পূর্ণসংখ্যা ul_strtos32(ধ্রুবক অক্ষর *str, int32_t *অঙ্ক, পূর্ণসংখ্যা base)
{
	int64_t tmp;
	পূর্ণসংখ্যা rc;

	rc = ul_strtos64(str, &tmp, base);
	যদি (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	যদি (rc == 0)
		*অঙ্ক = (int32_t) tmp;
	ফেরত rc;
}

পূর্ণসংখ্যা ul_strtou32(ধ্রুবক অক্ষর *str, uint32_t *অঙ্ক, পূর্ণসংখ্যা base)
{
	uint64_t tmp;
	পূর্ণসংখ্যা rc;

	rc = ul_strtou64(str, &tmp, base);
	যদি (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	যদি (rc == 0)
		*অঙ্ক = (uint32_t) tmp;
	ফেরত rc;
}

পূর্ণসংখ্যা ul_strtou16(ধ্রুবক অক্ষর *str, uint16_t *অঙ্ক, পূর্ণসংখ্যা base)
{
	uint64_t tmp;
	পূর্ণসংখ্যা rc;

	rc = ul_strtou64(str, &tmp, base);
	যদি (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	যদি (rc == 0)
		*অঙ্ক = (uint16_t) tmp;
	ফেরত rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(ধ্রুবক অক্ষর *str, পূর্ণসংখ্যা base, ধ্রুবক অক্ষর *errmesg,
			     int64_t নিচের, int64_t up)
{
	int64_t অঙ্ক = 0;
	পূর্ণসংখ্যা rc;

	rc = ul_strtos64(str, &অঙ্ক, base);
	যদি (rc == 0 && ((নিচের && অঙ্ক < নিচের) || (up && অঙ্ক > up)))
		rc = -(errno = ERANGE);

	যদি (rc) {
		যদি (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	ফেরত অঙ্ক;
}

uint64_t str2unum_or_err(ধ্রুবক অক্ষর *str, পূর্ণসংখ্যা base, ধ্রুবক অক্ষর *errmesg, uint64_t up)
{
	uint64_t অঙ্ক = 0;
	পূর্ণসংখ্যা rc;

	rc = ul_strtou64(str, &অঙ্ক, base);
	যদি (rc == 0 && (up && অঙ্ক > up))
		rc = -(errno = ERANGE);

	যদি (rc) {
		যদি (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	ফেরত অঙ্ক;
}

দ্বিগুণ strtod_or_err(ধ্রুবক অক্ষর *str, ধ্রুবক অক্ষর *errmesg)
{
	দ্বিগুণ অঙ্ক;
	অক্ষর *end = ফাঁকা;

	errno = 0;
	যদি (str == ফাঁকা || *str == '\0')
		যাও err;
	অঙ্ক = strtod(str, &end);

	যদি (errno || str == end || (end && *end))
		যাও err;

	ফেরত অঙ্ক;
err:
	যদি (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

পূর্ণসংখ্যা ul_strtold(ধ্রুবক অক্ষর *str, long দ্বিগুণ *অঙ্ক)
{
	অক্ষর *end = ফাঁকা;

	errno = 0;
	যদি (str == ফাঁকা || *str == '\0')
		ফেরত -(errno = EINVAL);
	*অঙ্ক = strtold(str, &end);

	যদি (errno != 0)
		ফেরত -errno;
	যদি (str == end || (end && *end))
		ফেরত -(errno = EINVAL);
	ফেরত 0;
}

long দ্বিগুণ strtold_or_err(ধ্রুবক অক্ষর *str, ধ্রুবক অক্ষর *errmesg)
{
	long দ্বিগুণ অঙ্ক = 0;

	যদি (ul_strtold(str, &অঙ্ক) == 0)
		ফেরত অঙ্ক;
	যদি (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(ধ্রুবক অক্ষর *str, ধ্রুবক অক্ষর *errmesg)
{
	uintmax_t অঙ্ক;

	যদি (strtosize(str, &অঙ্ক) == 0)
		ফেরত অঙ্ক;

	যদি (errno)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}


শূন্য strtotimeval_or_err(ধ্রুবক অক্ষর *str, কাঠামো timeval *tv, ধ্রুবক অক্ষর *errmesg)
{
	long দ্বিগুণ user_প্রবেশ;

	user_প্রবেশ = strtold_or_err(str, errmesg);
	tv->tv_sec = (time_t) user_প্রবেশ;
	tv->tv_usec = (suseconds_t)((user_প্রবেশ - tv->tv_sec) * USEC_PER_SEC);
}

শূন্য strtotimespec_or_err(ধ্রুবক অক্ষর *str, কাঠামো timespec *ts, ধ্রুবক অক্ষর *errmesg)
{
	long দ্বিগুণ user_প্রবেশ;

	user_প্রবেশ = strtold_or_err(str, errmesg);
	ts->tv_sec = (time_t) user_প্রবেশ;
	ts->tv_nsec = (long)((user_প্রবেশ - ts->tv_sec) * NSEC_PER_SEC);
}

time_t strtotime_or_err(ধ্রুবক অক্ষর *str, ধ্রুবক অক্ষর *errmesg)
{
	int64_t user_প্রবেশ;

	user_প্রবেশ = strtos64_or_err(str, errmesg);
	ফেরত (time_t) user_প্রবেশ;
}

bool hyperlinkwanted(ধ্রুবক অক্ষর *mode)
{
	যদি (mode && strcmp(mode, "never") == 0)
		ফেরত false;

	যদি (mode && strcmp(mode, "always") == 0)
		ফেরত true;

	যদি (!mode || strcmp(mode, "auto") == 0)
		ফেরত isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), mode);
}

bool annotationwanted(ধ্রুবক অক্ষর *mode)
{
	যদি (mode && strcmp(mode, "never") == 0)
		ফেরত false;

	যদি (mode && strcmp(mode, "always") == 0)
		ফেরত true;

	যদি (!mode || strcmp(mode, "auto") == 0)
		ফেরত isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), mode);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
অক্ষর *xstrmode(mode_t mode, অক্ষর *str)
{
	unsigned short i = 0;

	যদি (S_ISDIR(mode))
		str[i++] = 'd';
	অন্যথা যদি (S_ISLNK(mode))
		str[i++] = 'l';
	অন্যথা যদি (S_ISCHR(mode))
		str[i++] = 'c';
	অন্যথা যদি (S_ISBLK(mode))
		str[i++] = 'b';
	অন্যথা যদি (S_ISSOCK(mode))
		str[i++] = 's';
	অন্যথা যদি (S_ISFIFO(mode))
		str[i++] = 'p';
	অন্যথা যদি (S_ISREG(mode))
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

	ফেরত str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
স্থির পূর্ণসংখ্যা get_exp(uint64_t n)
{
	পূর্ণসংখ্যা shft;

	জন্য (shft = 10; shft <= 60; shft += 10) {
		যদি (n < (1ULL << shft))
			বিরতি;
	}
	ফেরত shft - 10;
}

অক্ষর *আকার_to_human_string(পূর্ণসংখ্যা options, uint64_t bytes)
{
	অক্ষর buf[32];
	পূর্ণসংখ্যা dec, exp;
	uint64_t frac;
	ধ্রুবক অক্ষর *letters = "BKMGTPE";
	অক্ষর suffix[আকার(" KiB")], *psuf = suffix;
	অক্ষর c;

	যদি (options & আকার_SUFFIX_SPACE)
		*psuf++ = ' ';


	exp  = get_exp(bytes);
	c    = *(letters + (exp ? exp / 10 : 0));
	dec  = exp ? bytes / (1ULL << exp) : bytes;
	frac = exp ? bytes % (1ULL << exp) : 0;

	*psuf++ = c;

	যদি ((options & আকার_SUFFIX_3_LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/* round */
	যদি (frac) {
		/* get 3 digits after decimal point */
		যদি (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (exp - 10)) ;
		অন্যথা
			frac = (frac * 1000) / (1ULL << (exp)) ;

		যদি (options & আকার_DECIMAL_2_DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} অন্যথা {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		যদি (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	যদি (frac) {
		কাঠামো lconv ধ্রুবক *l = localeconv();
		অক্ষর *dp = l ? l->decimal_point : ফাঁকা;
		পূর্ণসংখ্যা len;

		যদি (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, আকার(buf), "%d%s%02" PRIu64, dec, dp, frac);
		যদি (len > 0 && (size_t) len < আকার(buf)) {
			/* remove potential extraneous zero */
			যদি (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, আকার(buf) - len);
		} অন্যথা
			*buf = '\0';	/* snprintf error */
	} অন্যথা
		snprintf(buf, আকার(buf), "%d%s", dec, suffix);

	ফেরত strdup(buf);
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
পূর্ণসংখ্যা string_to_idarray(ধ্রুবক অক্ষর *list, পূর্ণসংখ্যা ary[], size_t arysz,
			পূর্ণসংখ্যা (name2id)(ধ্রুবক অক্ষর *, size_t))
{
	ধ্রুবক অক্ষর *begin = ফাঁকা, *p;
	size_t n = 0;

	যদি (!list || !*list || !ary || !arysz || !name2id)
		ফেরত -1;

	জন্য (p = list; p && *p; p++) {
		ধ্রুবক অক্ষর *end = ফাঁকা;
		পূর্ণসংখ্যা id;

		যদি (n >= arysz)
			ফেরত -2;
		যদি (!begin)
			begin = p;		/* begin of the column name */
		যদি (*p == ',')
			end = p;		/* terminate the name */
		যদি (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		যদি (!begin || !end)
			চালু;
		যদি (end <= begin)
			ফেরত -1;

		id = name2id(begin, end - begin);
		যদি (id == -1)
			ফেরত -1;
		ary[ n++ ] = id;
		begin = ফাঁকা;
		যদি (end && !*end)
			বিরতি;
	}
	ফেরত n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
পূর্ণসংখ্যা string_add_to_idarray(ধ্রুবক অক্ষর *list, পূর্ণসংখ্যা ary[], size_t arysz,
			size_t *ary_pos, পূর্ণসংখ্যা (name2id)(ধ্রুবক অক্ষর *, size_t))
{
	ধ্রুবক অক্ষর *list_add;
	পূর্ণসংখ্যা r;

	যদি (!list || !*list || !ary_pos || *ary_pos > arysz)
		ফেরত -1;

	যদি (list[0] == '+')
		list_add = &list[1];
	অন্যথা {
		list_add = list;
		*ary_pos = 0;
	}

	r = string_to_idarray(list_add, &ary[*ary_pos], arysz - *ary_pos, name2id);
	যদি (r > 0)
		*ary_pos += r;
	ফেরত r;
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
পূর্ণসংখ্যা string_to_bitarray(ধ্রুবক অক্ষর *list,
		     অক্ষর *ary,
		     পূর্ণসংখ্যা (*name2bit)(ধ্রুবক অক্ষর *, size_t),
		     size_t allow_range)
{
	ধ্রুবক অক্ষর *begin = ফাঁকা, *p;

	যদি (!list || !name2bit || !ary)
		ফেরত -EINVAL;

	জন্য (p = list; p && *p; p++) {
		ধ্রুবক অক্ষর *end = ফাঁকা;
		পূর্ণসংখ্যা bit, set_lower = 0, set_higher = 0;

		যদি (!begin)
			begin = p;		/* begin of the level name */
		যদি (*p == ',')
			end = p;		/* terminate the name */
		যদি (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		যদি (!begin || !end)
			চালু;
		যদি (end <= begin)
			ফেরত -1;
		যদি (allow_range) {
			যদি (*(end - 1) == '+') {
				end--;
				set_lower = 1;
			} অন্যথা যদি (*begin == '+') {
				begin++;
				set_higher = 1;
			}
		}

		bit = name2bit(begin, end - begin);
		যদি (bit < 0)
			ফেরত bit;
		setbit(ary, bit);
		যদি (set_lower)
			যখন (--bit >= 0)
				setbit(ary, bit);
		অন্যথা যদি (set_higher)
			যখন (++bit < (পূর্ণসংখ্যা) allow_range)
				setbit(ary, bit);
		begin = ফাঁকা;
		যদি (end && !*end)
			বিরতি;
	}
	ফেরত 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
পূর্ণসংখ্যা string_to_bitmask(ধ্রুবক অক্ষর *list,
		     unsigned long *mask,
		     long (*name2flag)(ধ্রুবক অক্ষর *, size_t))
{
	ধ্রুবক অক্ষর *begin = ফাঁকা, *p;

	যদি (!list || !name2flag || !mask)
		ফেরত -EINVAL;

	জন্য (p = list; p && *p; p++) {
		ধ্রুবক অক্ষর *end = ফাঁকা;
		long flag;

		যদি (!begin)
			begin = p;		/* begin of the level name */
		যদি (*p == ',')
			end = p;		/* terminate the name */
		যদি (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		যদি (!begin || !end)
			চালু;
		যদি (end <= begin)
			ফেরত -1;

		flag = name2flag(begin, end - begin);
		যদি (flag < 0)
			ফেরত flag;	/* error */
		*mask |= flag;
		begin = ফাঁকা;
		যদি (end && !*end)
			বিরতি;
	}
	ফেরত 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
পূর্ণসংখ্যা ul_parse_range(ধ্রুবক অক্ষর *str, পূর্ণসংখ্যা *lower, পূর্ণসংখ্যা *upper, পূর্ণসংখ্যা def)
{
	অক্ষর *end = ফাঁকা;

	যদি (!str)
		ফেরত 0;

	*upper = *lower = def;
	errno = 0;

	যদি (*str == ':') {				/* <:N> */
		str++;
		*upper = strtol(str, &end, 10);
		যদি (errno || !end || *end || end == str)
			ফেরত -1;
	} অন্যথা {
		*upper = *lower = strtol(str, &end, 10);
		যদি (errno || !end || end == str)
			ফেরত -1;

		যদি (*end == ':' && !*(end + 1))		/* <M:> */
			*upper = def;
		অন্যথা যদি (*end == '-' || *end == ':') {	/* <M:N> <M-N> */
			str = end + 1;
			end = ফাঁকা;
			errno = 0;
			*upper = strtol(str, &end, 10);

			যদি (errno || !end || *end || end == str)
				ফেরত -1;
		}
	}
	ফেরত 0;
}

ধ্রুবক অক্ষর *ul_path_পরবর্তী_segment(ধ্রুবক অক্ষর *str, size_t *sz)
{
	ধ্রুবক অক্ষর *start, *p;

	start = str;
	*sz = 0;
	যখন (start && *start == '/' && *(start + 1) == '/')
		start++;

	যদি (!start || !*start)
		ফেরত ফাঁকা;

	জন্য (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	ফেরত start;
}

পূর্ণসংখ্যা streq_paths(ধ্রুবক অক্ষর *a, ধ্রুবক অক্ষর *b)
{
	যখন (a && b) {
		size_t a_sz, b_sz;
		ধ্রুবক অক্ষর *a_seg = ul_path_পরবর্তী_segment(a, &a_sz);
		ধ্রুবক অক্ষর *b_seg = ul_path_পরবর্তী_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		যদি (a_sz + b_sz == 0)
			ফেরত 1;

		/* ignore trailing slash */
		যদি (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			ফেরত 1;

		যদি (!a_seg || !b_seg)
			বিরতি;
		যদি (a_sz != b_sz || strncmp(a_seg, b_seg, a_sz) != 0)
			বিরতি;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	ফেরত 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
পূর্ণসংখ্যা ul_path_is_within(ধ্রুবক অক্ষর *path, ধ্রুবক অক্ষর *dir)
{
	যদি (!path || !dir || !*path || !*dir)
		ফেরত 0;

	যখন (path && dir) {
		size_t p_sz, d_sz;
		ধ্রুবক অক্ষর *p_seg = ul_path_পরবর্তী_segment(path, &p_sz);
		ধ্রুবক অক্ষর *d_seg = ul_path_পরবর্তী_segment(dir, &d_sz);

		/* all @dir segments matched */
		যদি (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			ফেরত 1;

		যদি (!p_seg || !d_seg)
			বিরতি;
		যদি (p_sz != d_sz || strncmp(p_seg, d_seg, p_sz) != 0)
			বিরতি;

		path = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	ফেরত 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
অক্ষর *ul_strnconcat(ধ্রুবক অক্ষর *s, ধ্রুবক অক্ষর *suffix, size_t b)
{
	size_t a;
	অক্ষর *r;

	যদি (!s && !suffix)
		ফেরত strdup("");
	যদি (!s)
		ফেরত strndup(suffix, b);
	যদি (!suffix)
		ফেরত strdup(s);

	প্রতিজ্ঞা(s);
	প্রতিজ্ঞা(suffix);

	a = strlen(s);
	যদি (b > ((size_t) -1) - a)
		ফেরত ফাঁকা;

	r = malloc(a + b + 1);
	যদি (!r)
		ফেরত ফাঁকা;

	memcpy(r, s, a);
	memcpy(r + a, suffix, b);
	r[a+b] = 0;

	ফেরত r;
}

/* concatenate two strings to a new string */
অক্ষর *ul_strconcat(ধ্রুবক অক্ষর *s, ধ্রুবক অক্ষর *suffix)
{
	ফেরত ul_strnconcat(s, suffix, suffix ? strlen(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
অক্ষর *ul_strfconcat(ধ্রুবক অক্ষর *s, ধ্রুবক অক্ষর *format, ...)
{
	va_list ap;
	অক্ষর *val, *res;
	পূর্ণসংখ্যা sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	যদি (sz < 0)
		ফেরত ফাঁকা;

	res = ul_strnconcat(s, val, sz);
	মুক্ত(val);
	ফেরত res;
}

পূর্ণসংখ্যা ul_strappend(অক্ষর **a, ধ্রুবক অক্ষর *b)
{
	size_t al, bl;
	অক্ষর *tmp;

	যদি (!a)
		ফেরত -EINVAL;
	যদি (!b || !*b)
		ফেরত 0;
	যদি (!*a) {
		*a = strdup(b);
		ফেরত !*a ? -ENOMEM : 0;
	}

	al = strlen(*a);
	bl = strlen(b);

	tmp = realloc(*a, al + bl + 1);
	যদি (!tmp)
		ফেরত -ENOMEM;
	*a = tmp;
	memcpy((*a) + al, b, bl + 1);
	ফেরত 0;
}

/* the hybrid version of strfconcat and strappend. */
পূর্ণসংখ্যা strfappend(অক্ষর **a, ধ্রুবক অক্ষর *format, ...)
{
	va_list ap;
	পূর্ণসংখ্যা res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	ফেরত res;
}

বাহ্যিক পূর্ণসংখ্যা ul_strvfappend(অক্ষর **a, ধ্রুবক অক্ষর *format, va_list ap)
{
	অক্ষর *val;
	পূর্ণসংখ্যা sz;
	পূর্ণসংখ্যা res;

	sz = vasprintf(&val, format, ap);
	যদি (sz < 0)
		ফেরত -errno;

	res = ul_strappend(a, val);
	মুক্ত(val);
	ফেরত res;
}

স্থির size_t strcspn_escaped(ধ্রুবক অক্ষর *s, ধ্রুবক অক্ষর *reject)
{
	পূর্ণসংখ্যা escaped = 0;
	পূর্ণসংখ্যা n;

	জন্য (n=0; s[n]; n++) {
		যদি (escaped)
			escaped = 0;
		অন্যথা যদি (s[n] == '\\')
			escaped = 1;
		অন্যথা যদি (strchr(reject, s[n]))
			বিরতি;
	}

	/* if s ends in \, return index of previous char */
	ফেরত n - escaped;
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
অক্ষর *ul_strchr_escaped(ধ্রুবক অক্ষর *s, পূর্ণসংখ্যা c)
{
	অক্ষর *p;
	পূর্ণসংখ্যা esc = 0;

	জন্য (p = (অক্ষর *) s; p && *p; p++) {
		যদি (!esc && *p == '\\') {
			esc = 1;
			চালু;
		}
		যদি (*p == c && (!esc || c == '\\'))
			ফেরত p;
		esc = 0;
	}

	ফেরত ফাঁকা;
}

/* Split a string into words. */
ধ্রুবক অক্ষর *ul_split(ধ্রুবক অক্ষর **state, size_t *l, ধ্রুবক অক্ষর *separator, পূর্ণসংখ্যা quoted)
{
	ধ্রুবক অক্ষর *current;

	current = *state;

	যদি (!*current) {
		প্রতিজ্ঞা(**state == '\0');
		ফেরত ফাঁকা;
	}

	current += strspn(current, separator);
	যদি (!*current) {
		*state = current;
		ফেরত ফাঁকা;
	}

	যদি (quoted && strchr("\'\"", *current)) {
		অক্ষর quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		যদি (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			ফেরত ফাঁকা;
		}
		*state = current++ + *l + 2;
	} অন্যথা যদি (quoted) {
		*l = strcspn_escaped(current, separator);
		যদি (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			ফেরত ফাঁকা;
		}
		*state = current + *l;
	} অন্যথা {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	ফেরত current;
}

/* Rewind file pointer forward to new line.  */
পূর্ণসংখ্যা skip_fline(FILE *fp)
{
	পূর্ণসংখ্যা ch;

	করো {
		যদি ((ch = fgetc(fp)) == EOF)
			ফেরত 1;
		যদি (ch == '\n')
			ফেরত 0;
	} যখন (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
পূর্ণসংখ্যা ul_stralnumcmp(ধ্রুবক অক্ষর *p1, ধ্রুবক অক্ষর *p2)
{
	ধ্রুবক unsigned অক্ষর *s1 = (ধ্রুবক unsigned অক্ষর *) p1;
	ধ্রুবক unsigned অক্ষর *s2 = (ধ্রুবক unsigned অক্ষর *) p2;
	unsigned অক্ষর c1, c2;

	করো {
		করো {
			c1 = (unsigned অক্ষর) *s1++;
		} যখন (c1 != '\0' && !isalnum((unsigned পূর্ণসংখ্যা) c1));

		করো {
			c2 = (unsigned অক্ষর) *s2++;
		} যখন (c2 != '\0' && !isalnum((unsigned পূর্ণসংখ্যা) c2));

		যদি (c1 != '\0')
			c1 = tolower(c1);
		যদি (c2 != '\0')
			c2 = tolower(c2);
		যদি (c1 == '\0')
			ফেরত c1 - c2;
	} যখন (c1 == c2);

	ফেরত c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
পূর্ণসংখ্যা ul_optstr_পরবর্তী(অক্ষর **optstr, অক্ষর **name, size_t *namesz,
		   অক্ষর **মান, size_t *valsz)
{
	পূর্ণসংখ্যা open_quote = 0;
	অক্ষর *start = ফাঁকা, *stop = ফাঁকা, *p, *sep = ফাঁকা;
	অক্ষর *optstr0;

	প্রতিজ্ঞা(optstr);
	প্রতিজ্ঞা(*optstr);

	optstr0 = *optstr;

	যদি (name)
		*name = ফাঁকা;
	যদি (namesz)
		*namesz = 0;
	যদি (মান)
		*মান = ফাঁকা;
	যদি (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	যখন (optstr0 && *optstr0 == ',')
		optstr0++;

	জন্য (p = optstr0; p && *p; p++) {
		যদি (!start && *p == '=')
			ফেরত -EINVAL;
		যদি (!start)
			start = p;		/* beginning of the option item */
		যদি (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		যদি (open_quote)
			চালু;		/* still in quoted block */
		যদি (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		যদি (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		অন্যথা যদি (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		যদি (!start || !stop)
			চালু;
		যদি (stop <= start)
			ফেরত -EINVAL;

		যদি (name)
			*name = start;
		যদি (namesz)
			*namesz = sep ? sep - start : stop - start;
		*optstr = *stop ? stop + 1 : stop;

		যদি (sep) {
			যদি (মান)
				*মান = sep + 1;
			যদি (valsz)
				*valsz = stop - sep - 1;
		}
		ফেরত 0;
	}

	ফেরত 1;				/* end of optstr */
}

পূর্ণসংখ্যা ul_optstr_is_valid(ধ্রুবক অক্ষর *optstr)
{
	পূর্ণসংখ্যা rc;
	অক্ষর *p = (অক্ষর *) optstr;

	যখন ((rc = ul_optstr_পরবর্তী(&p, ফাঁকা, ফাঁকা, ফাঁকা, ফাঁকা)) == 0);
	ফেরত rc < 0 ? 0 : 1;
}

অক্ষর *ul_optstr_get_মান(ধ্রুবক অক্ষর *optstr, ধ্রুবক অক্ষর *চাবি)
{
	size_t sz, namesz = 0, valsz = 0;
	অক্ষর *name = ফাঁকা, *মান = ফাঁকা;
	অক্ষর *p = (অক্ষর *) optstr;

	যদি (!optstr || !চাবি || !*চাবি)
		ফেরত ফাঁকা;

	sz = strlen(চাবি);
	যখন (ul_optstr_পরবর্তী(&p, &name, &namesz, &মান, &valsz) == 0) {
		যদি (namesz != sz || !valsz)
			চালু;
		যদি (strncmp(name, চাবি, namesz) == 0)
			ফেরত strndup(মান, valsz);
	}
	ফেরত ফাঁকা;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(অক্ষর *str)
{
	size_t len;
	অক্ষর q;

	যদি (!str || !*str)
		ফেরত 0;

	len = strlen(str);
	q = *str;

	যদি ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		ফেরত len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	ফেরত len - 2;
}

#ifdef TEST_PROGRAM_STRUTILS

কাঠামো testS {
	অক্ষর *name;
	অক্ষর *মান;
};

স্থির পূর্ণসংখ্যা test_strdup_to_member(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	কাঠামো testS *xx;

	যদি (argc < 3)
		ফেরত EXIT_FAILURE;

	xx = calloc(1, আকার(*xx));
	যদি (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_to_struct_member(xx, name, argv[1]);
	strdup_to_struct_member(xx, মান, argv[2]);

	যদি (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->মান, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	printf("1: '%s', 2: '%s'\n", xx->name, xx->মান);

	মুক্ত(xx->name);
	মুক্ত(xx->মান);
	মুক্ত(xx);
	ফেরত EXIT_SUCCESS;
}

স্থির পূর্ণসংখ্যা test_strutils_sizes(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	uintmax_t আকার_2 = 0;
	অক্ষর *hum1, *hum2, *hum3;

	যদি (argc < 2)
		ফেরত EXIT_FAILURE;

	যদি (strtosize(argv[1], &আকার_2))
		errx(EXIT_FAILURE, "invalid size '%s' value", argv[1]);

	hum1 = আকার_to_human_string(আকার_SUFFIX_1_LETTER, আকার_2);
	hum2 = আকার_to_human_string(আকার_SUFFIX_3_LETTER |
				    আকার_SUFFIX_SPACE, আকার_2);
	hum3 = আকার_to_human_string(আকার_SUFFIX_3_LETTER |
				    আকার_SUFFIX_SPACE |
				    আকার_DECIMAL_2_DIGITS, আকার_2);

	printf("%25s : %20ju : %8s : %12s : %13s\n", argv[1], আকার_2, hum1, hum2, hum3);
	মুক্ত(hum1);
	মুক্ত(hum2);
	মুক্ত(hum3);

	ফেরত EXIT_SUCCESS;
}

স্থির পূর্ণসংখ্যা test_strutils_cmp_paths(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	পূর্ণসংখ্যা rc = streq_paths(argv[1], argv[2]);

	যদি (argc < 3)
		ফেরত EXIT_FAILURE;

	printf("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	ফেরত EXIT_SUCCESS;
}

স্থির পূর্ণসংখ্যা test_strutils_path_is_within(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	পূর্ণসংখ্যা rc;

	যদি (argc < 3)
		ফেরত EXIT_FAILURE;

	rc = ul_path_is_within(argv[1], argv[2]);

	printf("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	ফেরত EXIT_SUCCESS;
}

স্থির পূর্ণসংখ্যা test_strutils_normalize(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	unsigned অক্ষর *src, *dst, *org;
	size_t sz, len;

	যদি (argc < 2)
		ফেরত EXIT_FAILURE;

	org = (unsigned অক্ষর *) strdup(argv[1]);
	src = (unsigned অক্ষর *) strdup((অক্ষর *) org);
	len = strlen((অক্ষর *) src);
	dst = malloc(len + 1);

	যদি (!org || !src || !dst)
		যাও done;

	/* two buffers */
	sz = __normalize_whitespace(src, len, dst, len + 1);
	printf("1: '%s' --> '%s' [sz=%zu]\n", src, dst, sz);

	/* one buffer */
	sz = normalize_whitespace(src);
	printf("2: '%s' --> '%s' [sz=%zu]\n", org, src, sz);

done:
	মুক্ত(src);
	মুক্ত(dst);
	মুক্ত(org);

	ফেরত EXIT_SUCCESS;
}

স্থির পূর্ণসংখ্যা test_strutils_cstrcasecmp(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	অক্ষর *a, *b;

	যদি (argc < 3)
		ফেরত EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	যদি (!a || !b)
		ফেরত EXIT_FAILURE;

	printf("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	printf("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	printf("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, strlen(a)));

	ফেরত EXIT_SUCCESS;
}

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	যদি (argc == 3 && strcmp(argv[1], "--size") == 0) {
		ফেরত test_strutils_sizes(argc - 1, argv + 1);

	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		printf("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						ফাঁকা));
		ফেরত EXIT_SUCCESS;
	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		যদি (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		printf("'%s'-->%s\n", argv[2], val ? "true" : "false");
		ফেরত EXIT_SUCCESS;

	} অন্যথা যদি (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		ফেরত test_strutils_cmp_paths(argc - 1, argv + 1);

	} অন্যথা যদি (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		ফেরত test_strutils_path_is_within(argc - 1, argv + 1);

	} অন্যথা যদি (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		ফেরত test_strdup_to_member(argc - 1, argv + 1);

	} অন্যথা যদি  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		printf("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		ফেরত EXIT_SUCCESS;

	} অন্যথা যদি (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		ফেরত test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		ফেরত test_strutils_normalize(argc - 1, argv + 1);

	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		printf("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		ফেরত EXIT_SUCCESS;
	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		printf("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		ফেরত EXIT_SUCCESS;
	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		printf("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		ফেরত EXIT_SUCCESS;
	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		printf("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		ফেরত EXIT_SUCCESS;
	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		printf("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		ফেরত EXIT_SUCCESS;
	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		printf("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		ফেরত EXIT_SUCCESS;

	} অন্যথা যদি (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		printf("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		ফেরত EXIT_SUCCESS;

	} অন্যথা যদি (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		অক্ষর *buf = "abc\0Y\0\0xyz\0X";
		অক্ষর *end = buf + 12;
		অক্ষর *p = buf;

		করো {
			printf("str: '%s'\n", p);
		} যখন ((p = ul_পরবর্তী_string(p, end)));

		ফেরত EXIT_SUCCESS;

	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		অক্ষর *name = ফাঁকা, *val = ফাঁকা;
		অক্ষর *p = argv[2];
		পূর্ণসংখ্যা rc;

		যদি (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		যখন ((rc = ul_optstr_পরবর্তী(&p, &name, &namesz, &val, &valsz)) == 0) {
			printf("'%.*s' : '%.*s'\n", (পূর্ণসংখ্যা) namesz, name,
						   (পূর্ণসংখ্যা) valsz, val);
		}
		যদি (rc == 1)
			ফেরত EXIT_SUCCESS;
	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		অক্ষর *unquoted = strdup(argv[2]);

		যদি (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		printf("%s-->%s\n", argv[2], unquoted);
		মুক্ত(unquoted);
		ফেরত EXIT_SUCCESS;

	} অন্যথা {
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

	ফেরত EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_STRUTILS */
