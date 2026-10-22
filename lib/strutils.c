/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Karel Zak <kzak@redhat.com> [2010]
 *          Davidlohr Bueso <dave@gnu.org> [2010]
 */
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <inttypes.h>
#অন্তর্ভাৱ <ctype.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <sys/stat.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <strings.h>
#অন্তর্ভাৱ <assert.h>

#অন্তর্ভাৱ "c.h"
#অন্তর্ভাৱ "cctype.h"
#অন্তর্ভাৱ "nls.h"
#অন্তর্ভাৱ "strutils.h"
#অন্তর্ভাৱ "timeutils.h"
#অন্তর্ভাৱ "bitops.h"
#অন্তর্ভাৱ "pathnames.h"

স্থির পূর্ণাঙ্ক STRTOXX_EXIT_CODE = EXIT_FAILURE;

শূন্য strutils_set_exitcode(পূর্ণাঙ্ক ex)
{
	STRTOXX_EXIT_CODE = ex;
}

স্থির পূর্ণাঙ্ক do_scale_by_power (uintmax_t *x, পূর্ণাঙ্ক base, পূর্ণাঙ্ক power)
{
	যেতিয়ালৈ (power--) {
		যদি (UINTMAX_MAX / base < *x)
			ঘূৰাই -ERANGE;
		*x *= base;
	}
	ঘূৰাই 0;
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
পূর্ণাঙ্ক ul_parse_size(ধ্রুৱ ৱর্ণ *str, uintmax_t *res, পূর্ণাঙ্ক *power)
{
	ধ্রুৱ ৱর্ণ *p;
	ৱর্ণ *end;
	uintmax_t x, frac = 0;
	পূর্ণাঙ্ক base = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	স্থির ধ্রুৱ ৱর্ণ *suf  = "KMGTPEZY";
	স্থির ধ্রুৱ ৱর্ণ *suf2 = "kmgtpezy";
	ধ্রুৱ ৱর্ণ *sp;

	*res = 0;

	যদি (!str || !*str) {
		rc = -EINVAL;
		গচ্ছ err;
	}

	/* Only positive numbers are acceptable
	 *
	 * Note that this check is not perfect, it would be better to
	 * use lconv->negative_sign. But coreutils use the same solution,
	 * so it's probably good enough...
	 */
	p = str;
	যেতিয়ালৈ (isspace((unsigned ৱর্ণ) *p))
		p++;
	যদি (*p == '-') {
		rc = -EINVAL;
		গচ্ছ err;
	}

	errno = 0, end = রিক্ত;
	x = strtoumax(str, &end, 0);

	যদি (end == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		গচ্ছ err;
	}
	যদি (!end || !*end)
		গচ্ছ done;			/* without suffix */
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
		সংরচনা lconv ধ্রুৱ *l = localeconv();
		ধ্রুৱ ৱর্ণ *dp = l ? l->decimal_point : রিক্ত;
		size_t dpsz = dp ? strlen(dp) : 0;

		যদি (frac == 0 && *p && dp && strncmp(dp, p, dpsz) == 0) {
			ধ্রুৱ ৱর্ণ *fstr = p + dpsz;

			বাবে (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			যদি (isdigit(*fstr)) {
				errno = 0, end = রিক্ত;
				frac = strtoumax(fstr, &end, 0);
				যদি (end == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					গচ্ছ err;
				}
			} অন্যথা
				end = (ৱর্ণ *) p;

			যদি (!end || !*end) {
				rc = -EINVAL;
				গচ্ছ err;	/* without suffix, but with fractions */
			}
			p = end;
			গচ্ছ check_suffix;
		}
		rc = -EINVAL;
		গচ্ছ err;			/* unexpected suffix */
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
			গচ্ছ err;
		}
	}

	যদি (power)
		*power = pwr;

	rc = do_scale_by_power(&x, base, pwr);
	যদি (rc)
		গচ্ছ err;

	যদি (frac && pwr) {
		পূর্ণাঙ্ক i;
		uintmax_t frac_base = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_base, base, pwr);
		যদি (rc)
			গচ্ছ err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		কৰা {
			frac_val = ((frac % 10) * frac_base + frac_val) / 10;
			frac /= 10;
		} যেতিয়ালৈ (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		বাবে (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		যদি (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			গচ্ছ err;
		}
		x += frac_val;
	}
done:
	*res = x;
err:
	যদি (rc < 0)
		errno = -rc;
	ঘূৰাই rc;
}

পূর্ণাঙ্ক strtosize(ধ্রুৱ ৱর্ণ *str, uintmax_t *res)
{
	ঘূৰাই ul_parse_size(str, res, রিক্ত);
}

পূর্ণাঙ্ক isdigit_strend(ধ্রুৱ ৱর্ণ *str, ধ্রুৱ ৱর্ণ **end)
{
	ধ্রুৱ ৱর্ণ *p;

	বাবে (p = str; p && *p && isdigit((unsigned ৱর্ণ) *p); p++);

	যদি (end)
		*end = p;
	ঘূৰাই p && p > str && !*p;
}

পূর্ণাঙ্ক isxdigit_strend(ধ্রুৱ ৱর্ণ *str, ধ্রুৱ ৱর্ণ **end)
{
	ধ্রুৱ ৱর্ণ *p;

	বাবে (p = str; p && *p && isxdigit((unsigned ৱর্ণ) *p); p++);

	যদি (end)
		*end = p;

	ঘূৰাই p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
পূর্ণাঙ্ক ul_strtobool(ধ্রুৱ ৱর্ণ *str, bool *result)
{
	স্থির ধ্রুৱ ৱর্ণ *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	স্থির ধ্রুৱ ৱর্ণ *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	যদি (!str || !result)
		ঘূৰাই -(errno = EINVAL);

	errno = 0;

	বাবে (i = 0; i < ARRAY_SIZE(bool_true); i++) {
		যদি (c_strcasecmp(str, bool_true[i]) == 0) {
			*result = true;
			ঘূৰাই 0;
		}
	}

	বাবে (i = 0; i < ARRAY_SIZE(bool_false); i++) {
		যদি (c_strcasecmp(str, bool_false[i]) == 0) {
			*result = false;
			ঘূৰাই 0;
		}
	}

	ঘূৰাই -(errno = EINVAL);
}

পূর্ণাঙ্ক ul_parse_switch(ধ্রুৱ ৱর্ণ *arg, ...)
{
	ধ্রুৱ ৱর্ণ *a, *b;
	va_list ap;

	va_start(ap, arg);
	কৰা {
		a = va_arg(ap, ৱর্ণ *);
		যদি (!a)
			ভঙ্গ;
		b = va_arg(ap, ৱর্ণ *);
		যদি (!b)
			ভঙ্গ;

		যদি (strcmp(arg, a) == 0) {
			va_end(ap);
			ঘূৰাই 1;
		}

		যদি (strcmp(arg, b) == 0) {
			va_end(ap);
			ঘূৰাই 0;
		}
	} যেতিয়ালৈ (1);
	va_end(ap);

	errx(STRTOXX_EXIT_CODE, _("unsupported argument: %s"), arg);
}

#ifndef HAVE_MEMPCPY
শূন্য *mempcpy(শূন্য *restrict dest, ধ্রুৱ শূন্য *restrict src, size_t n)
{
	ঘূৰাই ((ৱর্ণ *)memcpy(dest, src, n)) + n;
}
#endif

#ifndef HAVE_STRNLEN
size_t strnlen(ধ্রুৱ ৱর্ণ *s, size_t maxlen)
{
	size_t i;

	বাবে (i = 0; i < maxlen; i++) {
		যদি (s[i] == '\0')
			ঘূৰাই i;
	}
	ঘূৰাই maxlen;
}
#endif

#ifndef HAVE_STRNCHR
ৱর্ণ *strnchr(ধ্রুৱ ৱর্ণ *s, size_t maxlen, পূর্ণাঙ্ক c)
{
	বাবে (; maxlen-- && *s != '\0'; ++s)
		যদি (*s == (ৱর্ণ)c)
			ঘূৰাই (ৱর্ণ *)s;
	ঘূৰাই রিক্ত;
}
#endif

#ifndef HAVE_STRNDUP
ৱর্ণ *strndup(ধ্রুৱ ৱর্ণ *s, size_t n)
{
	size_t len = strnlen(s, n);
	ৱর্ণ *new = malloc((len + 1) * পরিমাণ(ৱর্ণ));
	যদি (!new)
		ঘূৰাই রিক্ত;
	new[len] = '\0';
	ঘূৰাই (ৱর্ণ *) memcpy(new, s, len);
}
#endif

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
পূর্ণাঙ্ক ul_strtos64(ধ্রুৱ ৱর্ণ *str, int64_t *অঙ্ক, পূর্ণাঙ্ক base)
{
	ৱর্ণ *end = রিক্ত;

	যদি (str == রিক্ত || *str == '\0')
		ঘূৰাই -(errno = EINVAL);

	errno = 0;
	*অঙ্ক = (int64_t) strtoimax(str, &end, base);

	যদি (errno != 0)
		ঘূৰাই -errno;
	যদি (str == end || (end && *end))
		ঘূৰাই -(errno = EINVAL);
	ঘূৰাই 0;
}

পূর্ণাঙ্ক ul_strtou64(ধ্রুৱ ৱর্ণ *str, uint64_t *অঙ্ক, পূর্ণাঙ্ক base)
{
	ৱর্ণ *end = রিক্ত;
	int64_t tmp;

	যদি (str == রিক্ত || *str == '\0')
		ঘূৰাই -(errno = EINVAL);

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
		ঘূৰাই -errno;
	যদি (str == end || (end && *end))
		ঘূৰাই -(errno = EINVAL);
	ঘূৰাই 0;
}

পূর্ণাঙ্ক ul_strtos32(ধ্রুৱ ৱর্ণ *str, int32_t *অঙ্ক, পূর্ণাঙ্ক base)
{
	int64_t tmp;
	পূর্ণাঙ্ক rc;

	rc = ul_strtos64(str, &tmp, base);
	যদি (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	যদি (rc == 0)
		*অঙ্ক = (int32_t) tmp;
	ঘূৰাই rc;
}

পূর্ণাঙ্ক ul_strtou32(ধ্রুৱ ৱর্ণ *str, uint32_t *অঙ্ক, পূর্ণাঙ্ক base)
{
	uint64_t tmp;
	পূর্ণাঙ্ক rc;

	rc = ul_strtou64(str, &tmp, base);
	যদি (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	যদি (rc == 0)
		*অঙ্ক = (uint32_t) tmp;
	ঘূৰাই rc;
}

পূর্ণাঙ্ক ul_strtou16(ধ্রুৱ ৱর্ণ *str, uint16_t *অঙ্ক, পূর্ণাঙ্ক base)
{
	uint64_t tmp;
	পূর্ণাঙ্ক rc;

	rc = ul_strtou64(str, &tmp, base);
	যদি (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	যদি (rc == 0)
		*অঙ্ক = (uint16_t) tmp;
	ঘূৰাই rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(ধ্রুৱ ৱর্ণ *str, পূর্ণাঙ্ক base, ধ্রুৱ ৱর্ণ *errmesg,
			     int64_t নিচেৰ, int64_t up)
{
	int64_t অঙ্ক = 0;
	পূর্ণাঙ্ক rc;

	rc = ul_strtos64(str, &অঙ্ক, base);
	যদি (rc == 0 && ((নিচেৰ && অঙ্ক < নিচেৰ) || (up && অঙ্ক > up)))
		rc = -(errno = ERANGE);

	যদি (rc) {
		যদি (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	ঘূৰাই অঙ্ক;
}

uint64_t str2unum_or_err(ধ্রুৱ ৱর্ণ *str, পূর্ণাঙ্ক base, ধ্রুৱ ৱর্ণ *errmesg, uint64_t up)
{
	uint64_t অঙ্ক = 0;
	পূর্ণাঙ্ক rc;

	rc = ul_strtou64(str, &অঙ্ক, base);
	যদি (rc == 0 && (up && অঙ্ক > up))
		rc = -(errno = ERANGE);

	যদি (rc) {
		যদি (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	ঘূৰাই অঙ্ক;
}

দ্ৱিগুণ strtod_or_err(ধ্রুৱ ৱর্ণ *str, ধ্রুৱ ৱর্ণ *errmesg)
{
	দ্ৱিগুণ অঙ্ক;
	ৱর্ণ *end = রিক্ত;

	errno = 0;
	যদি (str == রিক্ত || *str == '\0')
		গচ্ছ err;
	অঙ্ক = strtod(str, &end);

	যদি (errno || str == end || (end && *end))
		গচ্ছ err;

	ঘূৰাই অঙ্ক;
err:
	যদি (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

পূর্ণাঙ্ক ul_strtold(ধ্রুৱ ৱর্ণ *str, long দ্ৱিগুণ *অঙ্ক)
{
	ৱর্ণ *end = রিক্ত;

	errno = 0;
	যদি (str == রিক্ত || *str == '\0')
		ঘূৰাই -(errno = EINVAL);
	*অঙ্ক = strtold(str, &end);

	যদি (errno != 0)
		ঘূৰাই -errno;
	যদি (str == end || (end && *end))
		ঘূৰাই -(errno = EINVAL);
	ঘূৰাই 0;
}

long দ্ৱিগুণ strtold_or_err(ধ্রুৱ ৱর্ণ *str, ধ্রুৱ ৱর্ণ *errmesg)
{
	long দ্ৱিগুণ অঙ্ক = 0;

	যদি (ul_strtold(str, &অঙ্ক) == 0)
		ঘূৰাই অঙ্ক;
	যদি (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(ধ্রুৱ ৱর্ণ *str, ধ্রুৱ ৱর্ণ *errmesg)
{
	uintmax_t অঙ্ক;

	যদি (strtosize(str, &অঙ্ক) == 0)
		ঘূৰাই অঙ্ক;

	যদি (errno)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}


শূন্য strtotimeval_or_err(ধ্রুৱ ৱর্ণ *str, সংরচনা timeval *tv, ধ্রুৱ ৱর্ণ *errmesg)
{
	long দ্ৱিগুণ user_প্ৰবেশ;

	user_প্ৰবেশ = strtold_or_err(str, errmesg);
	tv->tv_sec = (time_t) user_প্ৰবেশ;
	tv->tv_usec = (suseconds_t)((user_প্ৰবেশ - tv->tv_sec) * USEC_PER_SEC);
}

শূন্য strtotimespec_or_err(ধ্রুৱ ৱর্ণ *str, সংরচনা timespec *ts, ধ্রুৱ ৱর্ণ *errmesg)
{
	long দ্ৱিগুণ user_প্ৰবেশ;

	user_প্ৰবেশ = strtold_or_err(str, errmesg);
	ts->tv_sec = (time_t) user_প্ৰবেশ;
	ts->tv_nsec = (long)((user_প্ৰবেশ - ts->tv_sec) * NSEC_PER_SEC);
}

time_t strtotime_or_err(ধ্রুৱ ৱর্ণ *str, ধ্রুৱ ৱর্ণ *errmesg)
{
	int64_t user_প্ৰবেশ;

	user_প্ৰবেশ = strtos64_or_err(str, errmesg);
	ঘূৰাই (time_t) user_প্ৰবেশ;
}

bool hyperlinkwanted(ধ্রুৱ ৱর্ণ *mode)
{
	যদি (mode && strcmp(mode, "never") == 0)
		ঘূৰাই false;

	যদি (mode && strcmp(mode, "always") == 0)
		ঘূৰাই true;

	যদি (!mode || strcmp(mode, "auto") == 0)
		ঘূৰাই isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), mode);
}

bool annotationwanted(ধ্রুৱ ৱর্ণ *mode)
{
	যদি (mode && strcmp(mode, "never") == 0)
		ঘূৰাই false;

	যদি (mode && strcmp(mode, "always") == 0)
		ঘূৰাই true;

	যদি (!mode || strcmp(mode, "auto") == 0)
		ঘূৰাই isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), mode);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
ৱর্ণ *xstrmode(mode_t mode, ৱর্ণ *str)
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

	ঘূৰাই str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
স্থির পূর্ণাঙ্ক get_exp(uint64_t n)
{
	পূর্ণাঙ্ক shft;

	বাবে (shft = 10; shft <= 60; shft += 10) {
		যদি (n < (1ULL << shft))
			ভঙ্গ;
	}
	ঘূৰাই shft - 10;
}

ৱর্ণ *size_to_human_string(পূর্ণাঙ্ক options, uint64_t bytes)
{
	ৱর্ণ buf[32];
	পূর্ণাঙ্ক dec, exp;
	uint64_t frac;
	ধ্রুৱ ৱর্ণ *letters = "BKMGTPE";
	ৱর্ণ suffix[পরিমাণ(" KiB")], *psuf = suffix;
	ৱর্ণ c;

	যদি (options & SIZE_SUFFIX_SPACE)
		*psuf++ = ' ';


	exp  = get_exp(bytes);
	c    = *(letters + (exp ? exp / 10 : 0));
	dec  = exp ? bytes / (1ULL << exp) : bytes;
	frac = exp ? bytes % (1ULL << exp) : 0;

	*psuf++ = c;

	যদি ((options & SIZE_SUFFIX_3LETTER) && (c != 'B')) {
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

		যদি (options & SIZE_DECIMAL_2DIGITS) {
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
		সংরচনা lconv ধ্রুৱ *l = localeconv();
		ৱর্ণ *dp = l ? l->decimal_point : রিক্ত;
		পূর্ণাঙ্ক len;

		যদি (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, পরিমাণ(buf), "%d%s%02" PRIu64, dec, dp, frac);
		যদি (len > 0 && (size_t) len < পরিমাণ(buf)) {
			/* remove potential extraneous zero */
			যদি (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, পরিমাণ(buf) - len);
		} অন্যথা
			*buf = '\0';	/* snprintf error */
	} অন্যথা
		snprintf(buf, পরিমাণ(buf), "%d%s", dec, suffix);

	ঘূৰাই strdup(buf);
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
পূর্ণাঙ্ক string_to_idarray(ধ্রুৱ ৱর্ণ *list, পূর্ণাঙ্ক ary[], size_t arysz,
			পূর্ণাঙ্ক (name2id)(ধ্রুৱ ৱর্ণ *, size_t))
{
	ধ্রুৱ ৱর্ণ *begin = রিক্ত, *p;
	size_t n = 0;

	যদি (!list || !*list || !ary || !arysz || !name2id)
		ঘূৰাই -1;

	বাবে (p = list; p && *p; p++) {
		ধ্রুৱ ৱর্ণ *end = রিক্ত;
		পূর্ণাঙ্ক id;

		যদি (n >= arysz)
			ঘূৰাই -2;
		যদি (!begin)
			begin = p;		/* begin of the column name */
		যদি (*p == ',')
			end = p;		/* terminate the name */
		যদি (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		যদি (!begin || !end)
			অনুৱর্ত;
		যদি (end <= begin)
			ঘূৰাই -1;

		id = name2id(begin, end - begin);
		যদি (id == -1)
			ঘূৰাই -1;
		ary[ n++ ] = id;
		begin = রিক্ত;
		যদি (end && !*end)
			ভঙ্গ;
	}
	ঘূৰাই n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
পূর্ণাঙ্ক string_add_to_idarray(ধ্রুৱ ৱর্ণ *list, পূর্ণাঙ্ক ary[], size_t arysz,
			size_t *ary_pos, পূর্ণাঙ্ক (name2id)(ধ্রুৱ ৱর্ণ *, size_t))
{
	ধ্রুৱ ৱর্ণ *list_add;
	পূর্ণাঙ্ক r;

	যদি (!list || !*list || !ary_pos || *ary_pos > arysz)
		ঘূৰাই -1;

	যদি (list[0] == '+')
		list_add = &list[1];
	অন্যথা {
		list_add = list;
		*ary_pos = 0;
	}

	r = string_to_idarray(list_add, &ary[*ary_pos], arysz - *ary_pos, name2id);
	যদি (r > 0)
		*ary_pos += r;
	ঘূৰাই r;
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
পূর্ণাঙ্ক string_to_bitarray(ধ্রুৱ ৱর্ণ *list,
		     ৱর্ণ *ary,
		     পূর্ণাঙ্ক (*name2bit)(ধ্রুৱ ৱর্ণ *, size_t),
		     size_t allow_range)
{
	ধ্রুৱ ৱর্ণ *begin = রিক্ত, *p;

	যদি (!list || !name2bit || !ary)
		ঘূৰাই -EINVAL;

	বাবে (p = list; p && *p; p++) {
		ধ্রুৱ ৱর্ণ *end = রিক্ত;
		পূর্ণাঙ্ক bit, set_lower = 0, set_higher = 0;

		যদি (!begin)
			begin = p;		/* begin of the level name */
		যদি (*p == ',')
			end = p;		/* terminate the name */
		যদি (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		যদি (!begin || !end)
			অনুৱর্ত;
		যদি (end <= begin)
			ঘূৰাই -1;
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
			ঘূৰাই bit;
		setbit(ary, bit);
		যদি (set_lower)
			যেতিয়ালৈ (--bit >= 0)
				setbit(ary, bit);
		অন্যথা যদি (set_higher)
			যেতিয়ালৈ (++bit < (পূর্ণাঙ্ক) allow_range)
				setbit(ary, bit);
		begin = রিক্ত;
		যদি (end && !*end)
			ভঙ্গ;
	}
	ঘূৰাই 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
পূর্ণাঙ্ক string_to_bitmask(ধ্রুৱ ৱর্ণ *list,
		     unsigned long *mask,
		     long (*name2flag)(ধ্রুৱ ৱর্ণ *, size_t))
{
	ধ্রুৱ ৱর্ণ *begin = রিক্ত, *p;

	যদি (!list || !name2flag || !mask)
		ঘূৰাই -EINVAL;

	বাবে (p = list; p && *p; p++) {
		ধ্রুৱ ৱর্ণ *end = রিক্ত;
		long flag;

		যদি (!begin)
			begin = p;		/* begin of the level name */
		যদি (*p == ',')
			end = p;		/* terminate the name */
		যদি (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		যদি (!begin || !end)
			অনুৱর্ত;
		যদি (end <= begin)
			ঘূৰাই -1;

		flag = name2flag(begin, end - begin);
		যদি (flag < 0)
			ঘূৰাই flag;	/* error */
		*mask |= flag;
		begin = রিক্ত;
		যদি (end && !*end)
			ভঙ্গ;
	}
	ঘূৰাই 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
পূর্ণাঙ্ক ul_parse_range(ধ্রুৱ ৱর্ণ *str, পূর্ণাঙ্ক *lower, পূর্ণাঙ্ক *upper, পূর্ণাঙ্ক def)
{
	ৱর্ণ *end = রিক্ত;

	যদি (!str)
		ঘূৰাই 0;

	*upper = *lower = def;
	errno = 0;

	যদি (*str == ':') {				/* <:N> */
		str++;
		*upper = strtol(str, &end, 10);
		যদি (errno || !end || *end || end == str)
			ঘূৰাই -1;
	} অন্যথা {
		*upper = *lower = strtol(str, &end, 10);
		যদি (errno || !end || end == str)
			ঘূৰাই -1;

		যদি (*end == ':' && !*(end + 1))		/* <M:> */
			*upper = def;
		অন্যথা যদি (*end == '-' || *end == ':') {	/* <M:N> <M-N> */
			str = end + 1;
			end = রিক্ত;
			errno = 0;
			*upper = strtol(str, &end, 10);

			যদি (errno || !end || *end || end == str)
				ঘূৰাই -1;
		}
	}
	ঘূৰাই 0;
}

ধ্রুৱ ৱর্ণ *ul_path_next_segment(ধ্রুৱ ৱর্ণ *str, size_t *sz)
{
	ধ্রুৱ ৱর্ণ *start, *p;

	start = str;
	*sz = 0;
	যেতিয়ালৈ (start && *start == '/' && *(start + 1) == '/')
		start++;

	যদি (!start || !*start)
		ঘূৰাই রিক্ত;

	বাবে (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	ঘূৰাই start;
}

পূর্ণাঙ্ক streq_paths(ধ্রুৱ ৱর্ণ *a, ধ্রুৱ ৱর্ণ *b)
{
	যেতিয়ালৈ (a && b) {
		size_t a_sz, b_sz;
		ধ্রুৱ ৱর্ণ *a_seg = ul_path_next_segment(a, &a_sz);
		ধ্রুৱ ৱর্ণ *b_seg = ul_path_next_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		যদি (a_sz + b_sz == 0)
			ঘূৰাই 1;

		/* ignore trailing slash */
		যদি (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			ঘূৰাই 1;

		যদি (!a_seg || !b_seg)
			ভঙ্গ;
		যদি (a_sz != b_sz || strncmp(a_seg, b_seg, a_sz) != 0)
			ভঙ্গ;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	ঘূৰাই 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
পূর্ণাঙ্ক ul_path_is_within(ধ্রুৱ ৱর্ণ *path, ধ্রুৱ ৱর্ণ *dir)
{
	যদি (!path || !dir || !*path || !*dir)
		ঘূৰাই 0;

	যেতিয়ালৈ (path && dir) {
		size_t p_sz, d_sz;
		ধ্রুৱ ৱর্ণ *p_seg = ul_path_next_segment(path, &p_sz);
		ধ্রুৱ ৱর্ণ *d_seg = ul_path_next_segment(dir, &d_sz);

		/* all @dir segments matched */
		যদি (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			ঘূৰাই 1;

		যদি (!p_seg || !d_seg)
			ভঙ্গ;
		যদি (p_sz != d_sz || strncmp(p_seg, d_seg, p_sz) != 0)
			ভঙ্গ;

		path = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	ঘূৰাই 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
ৱর্ণ *ul_strnconcat(ধ্রুৱ ৱর্ণ *s, ধ্রুৱ ৱর্ণ *suffix, size_t b)
{
	size_t a;
	ৱর্ণ *r;

	যদি (!s && !suffix)
		ঘূৰাই strdup("");
	যদি (!s)
		ঘূৰাই strndup(suffix, b);
	যদি (!suffix)
		ঘূৰাই strdup(s);

	প্রতিজ্ঞা(s);
	প্রতিজ্ঞা(suffix);

	a = strlen(s);
	যদি (b > ((size_t) -1) - a)
		ঘূৰাই রিক্ত;

	r = malloc(a + b + 1);
	যদি (!r)
		ঘূৰাই রিক্ত;

	memcpy(r, s, a);
	memcpy(r + a, suffix, b);
	r[a+b] = 0;

	ঘূৰাই r;
}

/* concatenate two strings to a new string */
ৱর্ণ *ul_strconcat(ধ্রুৱ ৱর্ণ *s, ধ্রুৱ ৱর্ণ *suffix)
{
	ঘূৰাই ul_strnconcat(s, suffix, suffix ? strlen(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
ৱর্ণ *ul_strfconcat(ধ্রুৱ ৱর্ণ *s, ধ্রুৱ ৱর্ণ *format, ...)
{
	va_list ap;
	ৱর্ণ *val, *res;
	পূর্ণাঙ্ক sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	যদি (sz < 0)
		ঘূৰাই রিক্ত;

	res = ul_strnconcat(s, val, sz);
	মুক্ত(val);
	ঘূৰাই res;
}

পূর্ণাঙ্ক ul_strappend(ৱর্ণ **a, ধ্রুৱ ৱর্ণ *b)
{
	size_t al, bl;
	ৱর্ণ *tmp;

	যদি (!a)
		ঘূৰাই -EINVAL;
	যদি (!b || !*b)
		ঘূৰাই 0;
	যদি (!*a) {
		*a = strdup(b);
		ঘূৰাই !*a ? -ENOMEM : 0;
	}

	al = strlen(*a);
	bl = strlen(b);

	tmp = realloc(*a, al + bl + 1);
	যদি (!tmp)
		ঘূৰাই -ENOMEM;
	*a = tmp;
	memcpy((*a) + al, b, bl + 1);
	ঘূৰাই 0;
}

/* the hybrid version of strfconcat and strappend. */
পূর্ণাঙ্ক strfappend(ৱর্ণ **a, ধ্রুৱ ৱর্ণ *format, ...)
{
	va_list ap;
	পূর্ণাঙ্ক res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	ঘূৰাই res;
}

বাহ্য পূর্ণাঙ্ক ul_strvfappend(ৱর্ণ **a, ধ্রুৱ ৱর্ণ *format, va_list ap)
{
	ৱর্ণ *val;
	পূর্ণাঙ্ক sz;
	পূর্ণাঙ্ক res;

	sz = vasprintf(&val, format, ap);
	যদি (sz < 0)
		ঘূৰাই -errno;

	res = ul_strappend(a, val);
	মুক্ত(val);
	ঘূৰাই res;
}

স্থির size_t strcspn_escaped(ধ্রুৱ ৱর্ণ *s, ধ্রুৱ ৱর্ণ *reject)
{
	পূর্ণাঙ্ক escaped = 0;
	পূর্ণাঙ্ক n;

	বাবে (n=0; s[n]; n++) {
		যদি (escaped)
			escaped = 0;
		অন্যথা যদি (s[n] == '\\')
			escaped = 1;
		অন্যথা যদি (strchr(reject, s[n]))
			ভঙ্গ;
	}

	/* if s ends in \, return index of previous char */
	ঘূৰাই n - escaped;
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
ৱর্ণ *ul_strchr_escaped(ধ্রুৱ ৱর্ণ *s, পূর্ণাঙ্ক c)
{
	ৱর্ণ *p;
	পূর্ণাঙ্ক esc = 0;

	বাবে (p = (ৱর্ণ *) s; p && *p; p++) {
		যদি (!esc && *p == '\\') {
			esc = 1;
			অনুৱর্ত;
		}
		যদি (*p == c && (!esc || c == '\\'))
			ঘূৰাই p;
		esc = 0;
	}

	ঘূৰাই রিক্ত;
}

/* Split a string into words. */
ধ্রুৱ ৱর্ণ *ul_split(ধ্রুৱ ৱর্ণ **state, size_t *l, ধ্রুৱ ৱর্ণ *separator, পূর্ণাঙ্ক quoted)
{
	ধ্রুৱ ৱর্ণ *current;

	current = *state;

	যদি (!*current) {
		প্রতিজ্ঞা(**state == '\0');
		ঘূৰাই রিক্ত;
	}

	current += strspn(current, separator);
	যদি (!*current) {
		*state = current;
		ঘূৰাই রিক্ত;
	}

	যদি (quoted && strchr("\'\"", *current)) {
		ৱর্ণ quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		যদি (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			ঘূৰাই রিক্ত;
		}
		*state = current++ + *l + 2;
	} অন্যথা যদি (quoted) {
		*l = strcspn_escaped(current, separator);
		যদি (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			ঘূৰাই রিক্ত;
		}
		*state = current + *l;
	} অন্যথা {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	ঘূৰাই current;
}

/* Rewind file pointer forward to new line.  */
পূর্ণাঙ্ক skip_fline(FILE *fp)
{
	পূর্ণাঙ্ক ch;

	কৰা {
		যদি ((ch = fgetc(fp)) == EOF)
			ঘূৰাই 1;
		যদি (ch == '\n')
			ঘূৰাই 0;
	} যেতিয়ালৈ (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
পূর্ণাঙ্ক ul_stralnumcmp(ধ্রুৱ ৱর্ণ *p1, ধ্রুৱ ৱর্ণ *p2)
{
	ধ্রুৱ unsigned ৱর্ণ *s1 = (ধ্রুৱ unsigned ৱর্ণ *) p1;
	ধ্রুৱ unsigned ৱর্ণ *s2 = (ধ্রুৱ unsigned ৱর্ণ *) p2;
	unsigned ৱর্ণ c1, c2;

	কৰা {
		কৰা {
			c1 = (unsigned ৱর্ণ) *s1++;
		} যেতিয়ালৈ (c1 != '\0' && !isalnum((unsigned পূর্ণাঙ্ক) c1));

		কৰা {
			c2 = (unsigned ৱর্ণ) *s2++;
		} যেতিয়ালৈ (c2 != '\0' && !isalnum((unsigned পূর্ণাঙ্ক) c2));

		যদি (c1 != '\0')
			c1 = tolower(c1);
		যদি (c2 != '\0')
			c2 = tolower(c2);
		যদি (c1 == '\0')
			ঘূৰাই c1 - c2;
	} যেতিয়ালৈ (c1 == c2);

	ঘূৰাই c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
পূর্ণাঙ্ক ul_optstr_next(ৱর্ণ **optstr, ৱর্ণ **name, size_t *namesz,
		   ৱর্ণ **মান, size_t *valsz)
{
	পূর্ণাঙ্ক open_quote = 0;
	ৱর্ণ *start = রিক্ত, *stop = রিক্ত, *p, *sep = রিক্ত;
	ৱর্ণ *optstr0;

	প্রতিজ্ঞা(optstr);
	প্রতিজ্ঞা(*optstr);

	optstr0 = *optstr;

	যদি (name)
		*name = রিক্ত;
	যদি (namesz)
		*namesz = 0;
	যদি (মান)
		*মান = রিক্ত;
	যদি (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	যেতিয়ালৈ (optstr0 && *optstr0 == ',')
		optstr0++;

	বাবে (p = optstr0; p && *p; p++) {
		যদি (!start && *p == '=')
			ঘূৰাই -EINVAL;
		যদি (!start)
			start = p;		/* beginning of the option item */
		যদি (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		যদি (open_quote)
			অনুৱর্ত;		/* still in quoted block */
		যদি (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		যদি (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		অন্যথা যদি (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		যদি (!start || !stop)
			অনুৱর্ত;
		যদি (stop <= start)
			ঘূৰাই -EINVAL;

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
		ঘূৰাই 0;
	}

	ঘূৰাই 1;				/* end of optstr */
}

পূর্ণাঙ্ক ul_optstr_is_valid(ধ্রুৱ ৱর্ণ *optstr)
{
	পূর্ণাঙ্ক rc;
	ৱর্ণ *p = (ৱর্ণ *) optstr;

	যেতিয়ালৈ ((rc = ul_optstr_next(&p, রিক্ত, রিক্ত, রিক্ত, রিক্ত)) == 0);
	ঘূৰাই rc < 0 ? 0 : 1;
}

ৱর্ণ *ul_optstr_get_মান(ধ্রুৱ ৱর্ণ *optstr, ধ্রুৱ ৱর্ণ *চাবি)
{
	size_t sz, namesz = 0, valsz = 0;
	ৱর্ণ *name = রিক্ত, *মান = রিক্ত;
	ৱর্ণ *p = (ৱর্ণ *) optstr;

	যদি (!optstr || !চাবি || !*চাবি)
		ঘূৰাই রিক্ত;

	sz = strlen(চাবি);
	যেতিয়ালৈ (ul_optstr_next(&p, &name, &namesz, &মান, &valsz) == 0) {
		যদি (namesz != sz || !valsz)
			অনুৱর্ত;
		যদি (strncmp(name, চাবি, namesz) == 0)
			ঘূৰাই strndup(মান, valsz);
	}
	ঘূৰাই রিক্ত;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(ৱর্ণ *str)
{
	size_t len;
	ৱর্ণ q;

	যদি (!str || !*str)
		ঘূৰাই 0;

	len = strlen(str);
	q = *str;

	যদি ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		ঘূৰাই len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	ঘূৰাই len - 2;
}

#ifdef TEST_PROGRAM_STRUTILS

সংরচনা testS {
	ৱর্ণ *name;
	ৱর্ণ *মান;
};

স্থির পূর্ণাঙ্ক test_strdup_to_member(পূর্ণাঙ্ক argc, ৱর্ণ *argv[])
{
	সংরচনা testS *xx;

	যদি (argc < 3)
		ঘূৰাই EXIT_FAILURE;

	xx = calloc(1, পরিমাণ(*xx));
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
	ঘূৰাই EXIT_SUCCESS;
}

স্থির পূর্ণাঙ্ক test_strutils_sizes(পূর্ণাঙ্ক argc, ৱর্ণ *argv[])
{
	uintmax_t size = 0;
	ৱর্ণ *hum1, *hum2, *hum3;

	যদি (argc < 2)
		ঘূৰাই EXIT_FAILURE;

	যদি (strtosize(argv[1], &size))
		errx(EXIT_FAILURE, "invalid size '%s' value", argv[1]);

	hum1 = size_to_human_string(SIZE_SUFFIX_1LETTER, size);
	hum2 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE, size);
	hum3 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE |
				    SIZE_DECIMAL_2DIGITS, size);

	printf("%25s : %20ju : %8s : %12s : %13s\n", argv[1], size, hum1, hum2, hum3);
	মুক্ত(hum1);
	মুক্ত(hum2);
	মুক্ত(hum3);

	ঘূৰাই EXIT_SUCCESS;
}

স্থির পূর্ণাঙ্ক test_strutils_cmp_paths(পূর্ণাঙ্ক argc, ৱর্ণ *argv[])
{
	পূর্ণাঙ্ক rc = streq_paths(argv[1], argv[2]);

	যদি (argc < 3)
		ঘূৰাই EXIT_FAILURE;

	printf("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	ঘূৰাই EXIT_SUCCESS;
}

স্থির পূর্ণাঙ্ক test_strutils_path_is_within(পূর্ণাঙ্ক argc, ৱর্ণ *argv[])
{
	পূর্ণাঙ্ক rc;

	যদি (argc < 3)
		ঘূৰাই EXIT_FAILURE;

	rc = ul_path_is_within(argv[1], argv[2]);

	printf("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	ঘূৰাই EXIT_SUCCESS;
}

স্থির পূর্ণাঙ্ক test_strutils_normalize(পূর্ণাঙ্ক argc, ৱর্ণ *argv[])
{
	unsigned ৱর্ণ *src, *dst, *org;
	size_t sz, len;

	যদি (argc < 2)
		ঘূৰাই EXIT_FAILURE;

	org = (unsigned ৱর্ণ *) strdup(argv[1]);
	src = (unsigned ৱর্ণ *) strdup((ৱর্ণ *) org);
	len = strlen((ৱর্ণ *) src);
	dst = malloc(len + 1);

	যদি (!org || !src || !dst)
		গচ্ছ done;

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

	ঘূৰাই EXIT_SUCCESS;
}

স্থির পূর্ণাঙ্ক test_strutils_cstrcasecmp(পূর্ণাঙ্ক argc, ৱর্ণ *argv[])
{
	ৱর্ণ *a, *b;

	যদি (argc < 3)
		ঘূৰাই EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	যদি (!a || !b)
		ঘূৰাই EXIT_FAILURE;

	printf("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	printf("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	printf("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, strlen(a)));

	ঘূৰাই EXIT_SUCCESS;
}

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ *argv[])
{
	যদি (argc == 3 && strcmp(argv[1], "--size") == 0) {
		ঘূৰাই test_strutils_sizes(argc - 1, argv + 1);

	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		printf("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						রিক্ত));
		ঘূৰাই EXIT_SUCCESS;
	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		যদি (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		printf("'%s'-->%s\n", argv[2], val ? "true" : "false");
		ঘূৰাই EXIT_SUCCESS;

	} অন্যথা যদি (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		ঘূৰাই test_strutils_cmp_paths(argc - 1, argv + 1);

	} অন্যথা যদি (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		ঘূৰাই test_strutils_path_is_within(argc - 1, argv + 1);

	} অন্যথা যদি (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		ঘূৰাই test_strdup_to_member(argc - 1, argv + 1);

	} অন্যথা যদি  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		printf("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		ঘূৰাই EXIT_SUCCESS;

	} অন্যথা যদি (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		ঘূৰাই test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		ঘূৰাই test_strutils_normalize(argc - 1, argv + 1);

	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		printf("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		ঘূৰাই EXIT_SUCCESS;
	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		printf("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		ঘূৰাই EXIT_SUCCESS;
	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		printf("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		ঘূৰাই EXIT_SUCCESS;
	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		printf("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		ঘূৰাই EXIT_SUCCESS;
	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		printf("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		ঘূৰাই EXIT_SUCCESS;
	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		printf("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		ঘূৰাই EXIT_SUCCESS;

	} অন্যথা যদি (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		printf("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		ঘূৰাই EXIT_SUCCESS;

	} অন্যথা যদি (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		ৱর্ণ *buf = "abc\0Y\0\0xyz\0X";
		ৱর্ণ *end = buf + 12;
		ৱর্ণ *p = buf;

		কৰা {
			printf("str: '%s'\n", p);
		} যেতিয়ালৈ ((p = ul_next_string(p, end)));

		ঘূৰাই EXIT_SUCCESS;

	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		ৱর্ণ *name = রিক্ত, *val = রিক্ত;
		ৱর্ণ *p = argv[2];
		পূর্ণাঙ্ক rc;

		যদি (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		যেতিয়ালৈ ((rc = ul_optstr_next(&p, &name, &namesz, &val, &valsz)) == 0) {
			printf("'%.*s' : '%.*s'\n", (পূর্ণাঙ্ক) namesz, name,
						   (পূর্ণাঙ্ক) valsz, val);
		}
		যদি (rc == 1)
			ঘূৰাই EXIT_SUCCESS;
	} অন্যথা যদি (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		ৱর্ণ *unquoted = strdup(argv[2]);

		যদি (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		printf("%s-->%s\n", argv[2], unquoted);
		মুক্ত(unquoted);
		ঘূৰাই EXIT_SUCCESS;

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

	ঘূৰাই EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_STRUTILS */
