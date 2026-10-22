/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Karel Zak <kzak@redhat.com> [2010]
 *          Davidlohr Bueso <dave@gnu.org> [2010]
 */
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <inttypes.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <strings.h>
#ಅನ್ತರ್ಭಾವ <assert.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "cctype.h"
#ಅನ್ತರ್ಭಾವ "nls.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"
#ಅನ್ತರ್ಭಾವ "timeutils.h"
#ಅನ್ತರ್ಭಾವ "bitops.h"
#ಅನ್ತರ್ಭಾವ "pathnames.h"

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ STRTOXX_EXIT_CODE = EXIT_FAILURE;

ಶೂನ್ಯ strutils_set_exitcode(ಪೂರ್ಣಾಙ್ಕ ex)
{
	STRTOXX_EXIT_CODE = ex;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ do_scale_by_power (uintmax_t *x, ಪೂರ್ಣಾಙ್ಕ base, ಪೂರ್ಣಾಙ್ಕ power)
{
	ಯಾವತ್ (power--) {
		ಯದಿ (UINTMAX_MAX / base < *x)
			ಪ್ರತ್ಯಾಗಮ -ERANGE;
		*x *= base;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
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
ಪೂರ್ಣಾಙ್ಕ ul_parse_size(ಧ್ರುವ ವರ್ಣ *str, uintmax_t *res, ಪೂರ್ಣಾಙ್ಕ *power)
{
	ಧ್ರುವ ವರ್ಣ *p;
	ವರ್ಣ *end;
	uintmax_t x, frac = 0;
	ಪೂರ್ಣಾಙ್ಕ base = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *suf  = "KMGTPEZY";
	ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *suf2 = "kmgtpezy";
	ಧ್ರುವ ವರ್ಣ *sp;

	*res = 0;

	ಯದಿ (!str || !*str) {
		rc = -EINVAL;
		ಗಚ್ಛ err;
	}

	/* Only positive numbers are acceptable
	 *
	 * Note that this check is not perfect, it would be better to
	 * use lconv->negative_sign. But coreutils use the same solution,
	 * so it's probably good enough...
	 */
	p = str;
	ಯಾವತ್ (isspace((unsigned ವರ್ಣ) *p))
		p++;
	ಯದಿ (*p == '-') {
		rc = -EINVAL;
		ಗಚ್ಛ err;
	}

	errno = 0, end = ರಿಕ್ತ;
	x = strtoumax(str, &end, 0);

	ಯದಿ (end == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		ಗಚ್ಛ err;
	}
	ಯದಿ (!end || !*end)
		ಗಚ್ಛ done;			/* without suffix */
	p = end;

	/*
	 * Check size suffixes
	 */
check_suffix:
	ಯದಿ (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		base = 1024;			/* XiB, 2^N */
	ಅನ್ಯಥಾ ಯದಿ ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		base = 1000;			/* XB, 10^N */
	ಅನ್ಯಥಾ ಯದಿ (*(p + 1)) {
		ಸಂರಚನಾ lconv ಧ್ರುವ *l = localeconv();
		ಧ್ರುವ ವರ್ಣ *dp = l ? l->decimal_point : ರಿಕ್ತ;
		size_t dpsz = dp ? strlen(dp) : 0;

		ಯದಿ (frac == 0 && *p && dp && strncmp(dp, p, dpsz) == 0) {
			ಧ್ರುವ ವರ್ಣ *fstr = p + dpsz;

			ಪ್ರತಿ (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			ಯದಿ (isdigit(*fstr)) {
				errno = 0, end = ರಿಕ್ತ;
				frac = strtoumax(fstr, &end, 0);
				ಯದಿ (end == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					ಗಚ್ಛ err;
				}
			} ಅನ್ಯಥಾ
				end = (ವರ್ಣ *) p;

			ಯದಿ (!end || !*end) {
				rc = -EINVAL;
				ಗಚ್ಛ err;	/* without suffix, but with fractions */
			}
			p = end;
			ಗಚ್ಛ check_suffix;
		}
		rc = -EINVAL;
		ಗಚ್ಛ err;			/* unexpected suffix */
	}

	sp = strchr(suf, *p);
	ಯದಿ (sp)
		pwr = (sp - suf) + 1;
	ಅನ್ಯಥಾ {
		sp = strchr(suf2, *p);
		ಯದಿ (sp)
			pwr = (sp - suf2) + 1;
		ಅನ್ಯಥಾ {
			rc = -EINVAL;
			ಗಚ್ಛ err;
		}
	}

	ಯದಿ (power)
		*power = pwr;

	rc = do_scale_by_power(&x, base, pwr);
	ಯದಿ (rc)
		ಗಚ್ಛ err;

	ಯದಿ (frac && pwr) {
		ಪೂರ್ಣಾಙ್ಕ i;
		uintmax_t frac_base = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_base, base, pwr);
		ಯದಿ (rc)
			ಗಚ್ಛ err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		ಕುರು {
			frac_val = ((frac % 10) * frac_base + frac_val) / 10;
			frac /= 10;
		} ಯಾವತ್ (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		ಪ್ರತಿ (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		ಯದಿ (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			ಗಚ್ಛ err;
		}
		x += frac_val;
	}
done:
	*res = x;
err:
	ಯದಿ (rc < 0)
		errno = -rc;
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ strtosize(ಧ್ರುವ ವರ್ಣ *str, uintmax_t *res)
{
	ಪ್ರತ್ಯಾಗಮ ul_parse_size(str, res, ರಿಕ್ತ);
}

ಪೂರ್ಣಾಙ್ಕ isdigit_strend(ಧ್ರುವ ವರ್ಣ *str, ಧ್ರುವ ವರ್ಣ **end)
{
	ಧ್ರುವ ವರ್ಣ *p;

	ಪ್ರತಿ (p = str; p && *p && isdigit((unsigned ವರ್ಣ) *p); p++);

	ಯದಿ (end)
		*end = p;
	ಪ್ರತ್ಯಾಗಮ p && p > str && !*p;
}

ಪೂರ್ಣಾಙ್ಕ isxdigit_strend(ಧ್ರುವ ವರ್ಣ *str, ಧ್ರುವ ವರ್ಣ **end)
{
	ಧ್ರುವ ವರ್ಣ *p;

	ಪ್ರತಿ (p = str; p && *p && isxdigit((unsigned ವರ್ಣ) *p); p++);

	ಯದಿ (end)
		*end = p;

	ಪ್ರತ್ಯಾಗಮ p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
ಪೂರ್ಣಾಙ್ಕ ul_strtobool(ಧ್ರುವ ವರ್ಣ *str, bool *result)
{
	ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	ಯದಿ (!str || !result)
		ಪ್ರತ್ಯಾಗಮ -(errno = EINVAL);

	errno = 0;

	ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(bool_true); i++) {
		ಯದಿ (c_strcasecmp(str, bool_true[i]) == 0) {
			*result = true;
			ಪ್ರತ್ಯಾಗಮ 0;
		}
	}

	ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(bool_false); i++) {
		ಯದಿ (c_strcasecmp(str, bool_false[i]) == 0) {
			*result = false;
			ಪ್ರತ್ಯಾಗಮ 0;
		}
	}

	ಪ್ರತ್ಯಾಗಮ -(errno = EINVAL);
}

ಪೂರ್ಣಾಙ್ಕ ul_parse_switch(ಧ್ರುವ ವರ್ಣ *arg, ...)
{
	ಧ್ರುವ ವರ್ಣ *a, *b;
	va_list ap;

	va_start(ap, arg);
	ಕುರು {
		a = va_arg(ap, ವರ್ಣ *);
		ಯದಿ (!a)
			ಭಙ್ಗ;
		b = va_arg(ap, ವರ್ಣ *);
		ಯದಿ (!b)
			ಭಙ್ಗ;

		ಯದಿ (strcmp(arg, a) == 0) {
			va_end(ap);
			ಪ್ರತ್ಯಾಗಮ 1;
		}

		ಯದಿ (strcmp(arg, b) == 0) {
			va_end(ap);
			ಪ್ರತ್ಯಾಗಮ 0;
		}
	} ಯಾವತ್ (1);
	va_end(ap);

	errx(STRTOXX_EXIT_CODE, _("unsupported argument: %s"), arg);
}

#ifndef HAVE_MEMPCPY
ಶೂನ್ಯ *mempcpy(ಶೂನ್ಯ *restrict dest, ಧ್ರುವ ಶೂನ್ಯ *restrict src, size_t n)
{
	ಪ್ರತ್ಯಾಗಮ ((ವರ್ಣ *)memcpy(dest, src, n)) + n;
}
#endif

#ifndef HAVE_STRNLEN
size_t strnlen(ಧ್ರುವ ವರ್ಣ *s, size_t maxlen)
{
	size_t i;

	ಪ್ರತಿ (i = 0; i < maxlen; i++) {
		ಯದಿ (s[i] == '\0')
			ಪ್ರತ್ಯಾಗಮ i;
	}
	ಪ್ರತ್ಯಾಗಮ maxlen;
}
#endif

#ifndef HAVE_STRNCHR
ವರ್ಣ *strnchr(ಧ್ರುವ ವರ್ಣ *s, size_t maxlen, ಪೂರ್ಣಾಙ್ಕ c)
{
	ಪ್ರತಿ (; maxlen-- && *s != '\0'; ++s)
		ಯದಿ (*s == (ವರ್ಣ)c)
			ಪ್ರತ್ಯಾಗಮ (ವರ್ಣ *)s;
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}
#endif

#ifndef HAVE_STRNDUP
ವರ್ಣ *strndup(ಧ್ರುವ ವರ್ಣ *s, size_t n)
{
	size_t len = strnlen(s, n);
	ವರ್ಣ *new = malloc((len + 1) * ಪರಿಮಾಣ(ವರ್ಣ));
	ಯದಿ (!new)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	new[len] = '\0';
	ಪ್ರತ್ಯಾಗಮ (ವರ್ಣ *) memcpy(new, s, len);
}
#endif

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
ಪೂರ್ಣಾಙ್ಕ ul_strtos64(ಧ್ರುವ ವರ್ಣ *str, int64_t *ಅಂಕೆ, ಪೂರ್ಣಾಙ್ಕ base)
{
	ವರ್ಣ *end = ರಿಕ್ತ;

	ಯದಿ (str == ರಿಕ್ತ || *str == '\0')
		ಪ್ರತ್ಯಾಗಮ -(errno = EINVAL);

	errno = 0;
	*ಅಂಕೆ = (int64_t) strtoimax(str, &end, base);

	ಯದಿ (errno != 0)
		ಪ್ರತ್ಯಾಗಮ -errno;
	ಯದಿ (str == end || (end && *end))
		ಪ್ರತ್ಯಾಗಮ -(errno = EINVAL);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ ul_strtou64(ಧ್ರುವ ವರ್ಣ *str, uint64_t *ಅಂಕೆ, ಪೂರ್ಣಾಙ್ಕ base)
{
	ವರ್ಣ *end = ರಿಕ್ತ;
	int64_t tmp;

	ಯದಿ (str == ರಿಕ್ತ || *str == '\0')
		ಪ್ರತ್ಯಾಗಮ -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &end, base);
	ಯದಿ (tmp < 0)
		errno = ERANGE;
	ಅನ್ಯಥಾ {
		errno = 0;
		*ಅಂಕೆ = strtoumax(str, &end, base);
	}

	ಯದಿ (errno != 0)
		ಪ್ರತ್ಯಾಗಮ -errno;
	ಯದಿ (str == end || (end && *end))
		ಪ್ರತ್ಯಾಗಮ -(errno = EINVAL);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ ul_strtos32(ಧ್ರುವ ವರ್ಣ *str, int32_t *ಅಂಕೆ, ಪೂರ್ಣಾಙ್ಕ base)
{
	int64_t tmp;
	ಪೂರ್ಣಾಙ್ಕ rc;

	rc = ul_strtos64(str, &tmp, base);
	ಯದಿ (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	ಯದಿ (rc == 0)
		*ಅಂಕೆ = (int32_t) tmp;
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ ul_strtou32(ಧ್ರುವ ವರ್ಣ *str, uint32_t *ಅಂಕೆ, ಪೂರ್ಣಾಙ್ಕ base)
{
	uint64_t tmp;
	ಪೂರ್ಣಾಙ್ಕ rc;

	rc = ul_strtou64(str, &tmp, base);
	ಯದಿ (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	ಯದಿ (rc == 0)
		*ಅಂಕೆ = (uint32_t) tmp;
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ ul_strtou16(ಧ್ರುವ ವರ್ಣ *str, uint16_t *ಅಂಕೆ, ಪೂರ್ಣಾಙ್ಕ base)
{
	uint64_t tmp;
	ಪೂರ್ಣಾಙ್ಕ rc;

	rc = ul_strtou64(str, &tmp, base);
	ಯದಿ (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	ಯದಿ (rc == 0)
		*ಅಂಕೆ = (uint16_t) tmp;
	ಪ್ರತ್ಯಾಗಮ rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(ಧ್ರುವ ವರ್ಣ *str, ಪೂರ್ಣಾಙ್ಕ base, ಧ್ರುವ ವರ್ಣ *errmesg,
			     int64_t ಕೆಳಗಿನ, int64_t up)
{
	int64_t ಅಂಕೆ = 0;
	ಪೂರ್ಣಾಙ್ಕ rc;

	rc = ul_strtos64(str, &ಅಂಕೆ, base);
	ಯದಿ (rc == 0 && ((ಕೆಳಗಿನ && ಅಂಕೆ < ಕೆಳಗಿನ) || (up && ಅಂಕೆ > up)))
		rc = -(errno = ERANGE);

	ಯದಿ (rc) {
		ಯದಿ (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	ಪ್ರತ್ಯಾಗಮ ಅಂಕೆ;
}

uint64_t str2unum_or_err(ಧ್ರುವ ವರ್ಣ *str, ಪೂರ್ಣಾಙ್ಕ base, ಧ್ರುವ ವರ್ಣ *errmesg, uint64_t up)
{
	uint64_t ಅಂಕೆ = 0;
	ಪೂರ್ಣಾಙ್ಕ rc;

	rc = ul_strtou64(str, &ಅಂಕೆ, base);
	ಯದಿ (rc == 0 && (up && ಅಂಕೆ > up))
		rc = -(errno = ERANGE);

	ಯದಿ (rc) {
		ಯದಿ (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	ಪ್ರತ್ಯಾಗಮ ಅಂಕೆ;
}

ದ್ವಿಗುಣ strtod_or_err(ಧ್ರುವ ವರ್ಣ *str, ಧ್ರುವ ವರ್ಣ *errmesg)
{
	ದ್ವಿಗುಣ ಅಂಕೆ;
	ವರ್ಣ *end = ರಿಕ್ತ;

	errno = 0;
	ಯದಿ (str == ರಿಕ್ತ || *str == '\0')
		ಗಚ್ಛ err;
	ಅಂಕೆ = strtod(str, &end);

	ಯದಿ (errno || str == end || (end && *end))
		ಗಚ್ಛ err;

	ಪ್ರತ್ಯಾಗಮ ಅಂಕೆ;
err:
	ಯದಿ (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

ಪೂರ್ಣಾಙ್ಕ ul_strtold(ಧ್ರುವ ವರ್ಣ *str, long ದ್ವಿಗುಣ *ಅಂಕೆ)
{
	ವರ್ಣ *end = ರಿಕ್ತ;

	errno = 0;
	ಯದಿ (str == ರಿಕ್ತ || *str == '\0')
		ಪ್ರತ್ಯಾಗಮ -(errno = EINVAL);
	*ಅಂಕೆ = strtold(str, &end);

	ಯದಿ (errno != 0)
		ಪ್ರತ್ಯಾಗಮ -errno;
	ಯದಿ (str == end || (end && *end))
		ಪ್ರತ್ಯಾಗಮ -(errno = EINVAL);
	ಪ್ರತ್ಯಾಗಮ 0;
}

long ದ್ವಿಗುಣ strtold_or_err(ಧ್ರುವ ವರ್ಣ *str, ಧ್ರುವ ವರ್ಣ *errmesg)
{
	long ದ್ವಿಗುಣ ಅಂಕೆ = 0;

	ಯದಿ (ul_strtold(str, &ಅಂಕೆ) == 0)
		ಪ್ರತ್ಯಾಗಮ ಅಂಕೆ;
	ಯದಿ (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(ಧ್ರುವ ವರ್ಣ *str, ಧ್ರುವ ವರ್ಣ *errmesg)
{
	uintmax_t ಅಂಕೆ;

	ಯದಿ (strtosize(str, &ಅಂಕೆ) == 0)
		ಪ್ರತ್ಯಾಗಮ ಅಂಕೆ;

	ಯದಿ (errno)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}


ಶೂನ್ಯ strtotimeval_or_err(ಧ್ರುವ ವರ್ಣ *str, ಸಂರಚನಾ timeval *tv, ಧ್ರುವ ವರ್ಣ *errmesg)
{
	long ದ್ವಿಗುಣ user_ಒಳಹರಿವು;

	user_ಒಳಹರಿವು = strtold_or_err(str, errmesg);
	tv->tv_sec = (time_t) user_ಒಳಹರಿವು;
	tv->tv_usec = (suseconds_t)((user_ಒಳಹರಿವು - tv->tv_sec) * USEC_PER_SEC);
}

ಶೂನ್ಯ strtotimespec_or_err(ಧ್ರುವ ವರ್ಣ *str, ಸಂರಚನಾ timespec *ts, ಧ್ರುವ ವರ್ಣ *errmesg)
{
	long ದ್ವಿಗುಣ user_ಒಳಹರಿವು;

	user_ಒಳಹರಿವು = strtold_or_err(str, errmesg);
	ts->tv_sec = (time_t) user_ಒಳಹರಿವು;
	ts->tv_nsec = (long)((user_ಒಳಹರಿವು - ts->tv_sec) * NSEC_PER_SEC);
}

time_t strtotime_or_err(ಧ್ರುವ ವರ್ಣ *str, ಧ್ರುವ ವರ್ಣ *errmesg)
{
	int64_t user_ಒಳಹರಿವು;

	user_ಒಳಹರಿವು = strtos64_or_err(str, errmesg);
	ಪ್ರತ್ಯಾಗಮ (time_t) user_ಒಳಹರಿವು;
}

bool hyperlinkwanted(ಧ್ರುವ ವರ್ಣ *mode)
{
	ಯದಿ (mode && strcmp(mode, "never") == 0)
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ (mode && strcmp(mode, "always") == 0)
		ಪ್ರತ್ಯಾಗಮ true;

	ಯದಿ (!mode || strcmp(mode, "auto") == 0)
		ಪ್ರತ್ಯಾಗಮ isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), mode);
}

bool annotationwanted(ಧ್ರುವ ವರ್ಣ *mode)
{
	ಯದಿ (mode && strcmp(mode, "never") == 0)
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ (mode && strcmp(mode, "always") == 0)
		ಪ್ರತ್ಯಾಗಮ true;

	ಯದಿ (!mode || strcmp(mode, "auto") == 0)
		ಪ್ರತ್ಯಾಗಮ isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), mode);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
ವರ್ಣ *xstrmode(mode_t mode, ವರ್ಣ *str)
{
	unsigned short i = 0;

	ಯದಿ (S_ISDIR(mode))
		str[i++] = 'd';
	ಅನ್ಯಥಾ ಯದಿ (S_ISLNK(mode))
		str[i++] = 'l';
	ಅನ್ಯಥಾ ಯದಿ (S_ISCHR(mode))
		str[i++] = 'c';
	ಅನ್ಯಥಾ ಯದಿ (S_ISBLK(mode))
		str[i++] = 'b';
	ಅನ್ಯಥಾ ಯದಿ (S_ISSOCK(mode))
		str[i++] = 's';
	ಅನ್ಯಥಾ ಯದಿ (S_ISFIFO(mode))
		str[i++] = 'p';
	ಅನ್ಯಥಾ ಯದಿ (S_ISREG(mode))
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

	ಪ್ರತ್ಯಾಗಮ str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ get_exp(uint64_t n)
{
	ಪೂರ್ಣಾಙ್ಕ shft;

	ಪ್ರತಿ (shft = 10; shft <= 60; shft += 10) {
		ಯದಿ (n < (1ULL << shft))
			ಭಙ್ಗ;
	}
	ಪ್ರತ್ಯಾಗಮ shft - 10;
}

ವರ್ಣ *size_to_human_string(ಪೂರ್ಣಾಙ್ಕ options, uint64_t bytes)
{
	ವರ್ಣ buf[32];
	ಪೂರ್ಣಾಙ್ಕ dec, exp;
	uint64_t frac;
	ಧ್ರುವ ವರ್ಣ *letters = "BKMGTPE";
	ವರ್ಣ suffix[ಪರಿಮಾಣ(" KiB")], *psuf = suffix;
	ವರ್ಣ c;

	ಯದಿ (options & SIZE_SUFFIX_SPACE)
		*psuf++ = ' ';


	exp  = get_exp(bytes);
	c    = *(letters + (exp ? exp / 10 : 0));
	dec  = exp ? bytes / (1ULL << exp) : bytes;
	frac = exp ? bytes % (1ULL << exp) : 0;

	*psuf++ = c;

	ಯದಿ ((options & SIZE_SUFFIX_3LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/* round */
	ಯದಿ (frac) {
		/* get 3 digits after decimal point */
		ಯದಿ (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (exp - 10)) ;
		ಅನ್ಯಥಾ
			frac = (frac * 1000) / (1ULL << (exp)) ;

		ಯದಿ (options & SIZE_DECIMAL_2DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} ಅನ್ಯಥಾ {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		ಯದಿ (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	ಯದಿ (frac) {
		ಸಂರಚನಾ lconv ಧ್ರುವ *l = localeconv();
		ವರ್ಣ *dp = l ? l->decimal_point : ರಿಕ್ತ;
		ಪೂರ್ಣಾಙ್ಕ len;

		ಯದಿ (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, ಪರಿಮಾಣ(buf), "%d%s%02" PRIu64, dec, dp, frac);
		ಯದಿ (len > 0 && (size_t) len < ಪರಿಮಾಣ(buf)) {
			/* remove potential extraneous zero */
			ಯದಿ (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, ಪರಿಮಾಣ(buf) - len);
		} ಅನ್ಯಥಾ
			*buf = '\0';	/* snprintf error */
	} ಅನ್ಯಥಾ
		snprintf(buf, ಪರಿಮಾಣ(buf), "%d%s", dec, suffix);

	ಪ್ರತ್ಯಾಗಮ strdup(buf);
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
ಪೂರ್ಣಾಙ್ಕ string_to_idarray(ಧ್ರುವ ವರ್ಣ *list, ಪೂರ್ಣಾಙ್ಕ ary[], size_t arysz,
			ಪೂರ್ಣಾಙ್ಕ (name2id)(ಧ್ರುವ ವರ್ಣ *, size_t))
{
	ಧ್ರುವ ವರ್ಣ *begin = ರಿಕ್ತ, *p;
	size_t n = 0;

	ಯದಿ (!list || !*list || !ary || !arysz || !name2id)
		ಪ್ರತ್ಯಾಗಮ -1;

	ಪ್ರತಿ (p = list; p && *p; p++) {
		ಧ್ರುವ ವರ್ಣ *end = ರಿಕ್ತ;
		ಪೂರ್ಣಾಙ್ಕ id;

		ಯದಿ (n >= arysz)
			ಪ್ರತ್ಯಾಗಮ -2;
		ಯದಿ (!begin)
			begin = p;		/* begin of the column name */
		ಯದಿ (*p == ',')
			end = p;		/* terminate the name */
		ಯದಿ (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		ಯದಿ (!begin || !end)
			ಅನುವರ್ತ;
		ಯದಿ (end <= begin)
			ಪ್ರತ್ಯಾಗಮ -1;

		id = name2id(begin, end - begin);
		ಯದಿ (id == -1)
			ಪ್ರತ್ಯಾಗಮ -1;
		ary[ n++ ] = id;
		begin = ರಿಕ್ತ;
		ಯದಿ (end && !*end)
			ಭಙ್ಗ;
	}
	ಪ್ರತ್ಯಾಗಮ n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
ಪೂರ್ಣಾಙ್ಕ string_add_to_idarray(ಧ್ರುವ ವರ್ಣ *list, ಪೂರ್ಣಾಙ್ಕ ary[], size_t arysz,
			size_t *ary_pos, ಪೂರ್ಣಾಙ್ಕ (name2id)(ಧ್ರುವ ವರ್ಣ *, size_t))
{
	ಧ್ರುವ ವರ್ಣ *list_add;
	ಪೂರ್ಣಾಙ್ಕ r;

	ಯದಿ (!list || !*list || !ary_pos || *ary_pos > arysz)
		ಪ್ರತ್ಯಾಗಮ -1;

	ಯದಿ (list[0] == '+')
		list_add = &list[1];
	ಅನ್ಯಥಾ {
		list_add = list;
		*ary_pos = 0;
	}

	r = string_to_idarray(list_add, &ary[*ary_pos], arysz - *ary_pos, name2id);
	ಯದಿ (r > 0)
		*ary_pos += r;
	ಪ್ರತ್ಯಾಗಮ r;
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
ಪೂರ್ಣಾಙ್ಕ string_to_bitarray(ಧ್ರುವ ವರ್ಣ *list,
		     ವರ್ಣ *ary,
		     ಪೂರ್ಣಾಙ್ಕ (*name2bit)(ಧ್ರುವ ವರ್ಣ *, size_t),
		     size_t allow_range)
{
	ಧ್ರುವ ವರ್ಣ *begin = ರಿಕ್ತ, *p;

	ಯದಿ (!list || !name2bit || !ary)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	ಪ್ರತಿ (p = list; p && *p; p++) {
		ಧ್ರುವ ವರ್ಣ *end = ರಿಕ್ತ;
		ಪೂರ್ಣಾಙ್ಕ bit, set_lower = 0, set_higher = 0;

		ಯದಿ (!begin)
			begin = p;		/* begin of the level name */
		ಯದಿ (*p == ',')
			end = p;		/* terminate the name */
		ಯದಿ (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		ಯದಿ (!begin || !end)
			ಅನುವರ್ತ;
		ಯದಿ (end <= begin)
			ಪ್ರತ್ಯಾಗಮ -1;
		ಯದಿ (allow_range) {
			ಯದಿ (*(end - 1) == '+') {
				end--;
				set_lower = 1;
			} ಅನ್ಯಥಾ ಯದಿ (*begin == '+') {
				begin++;
				set_higher = 1;
			}
		}

		bit = name2bit(begin, end - begin);
		ಯದಿ (bit < 0)
			ಪ್ರತ್ಯಾಗಮ bit;
		setbit(ary, bit);
		ಯದಿ (set_lower)
			ಯಾವತ್ (--bit >= 0)
				setbit(ary, bit);
		ಅನ್ಯಥಾ ಯದಿ (set_higher)
			ಯಾವತ್ (++bit < (ಪೂರ್ಣಾಙ್ಕ) allow_range)
				setbit(ary, bit);
		begin = ರಿಕ್ತ;
		ಯದಿ (end && !*end)
			ಭಙ್ಗ;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
ಪೂರ್ಣಾಙ್ಕ string_to_bitmask(ಧ್ರುವ ವರ್ಣ *list,
		     unsigned long *mask,
		     long (*name2flag)(ಧ್ರುವ ವರ್ಣ *, size_t))
{
	ಧ್ರುವ ವರ್ಣ *begin = ರಿಕ್ತ, *p;

	ಯದಿ (!list || !name2flag || !mask)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	ಪ್ರತಿ (p = list; p && *p; p++) {
		ಧ್ರುವ ವರ್ಣ *end = ರಿಕ್ತ;
		long flag;

		ಯದಿ (!begin)
			begin = p;		/* begin of the level name */
		ಯದಿ (*p == ',')
			end = p;		/* terminate the name */
		ಯದಿ (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		ಯದಿ (!begin || !end)
			ಅನುವರ್ತ;
		ಯದಿ (end <= begin)
			ಪ್ರತ್ಯಾಗಮ -1;

		flag = name2flag(begin, end - begin);
		ಯದಿ (flag < 0)
			ಪ್ರತ್ಯಾಗಮ flag;	/* error */
		*mask |= flag;
		begin = ರಿಕ್ತ;
		ಯದಿ (end && !*end)
			ಭಙ್ಗ;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
ಪೂರ್ಣಾಙ್ಕ ul_parse_range(ಧ್ರುವ ವರ್ಣ *str, ಪೂರ್ಣಾಙ್ಕ *lower, ಪೂರ್ಣಾಙ್ಕ *upper, ಪೂರ್ಣಾಙ್ಕ def)
{
	ವರ್ಣ *end = ರಿಕ್ತ;

	ಯದಿ (!str)
		ಪ್ರತ್ಯಾಗಮ 0;

	*upper = *lower = def;
	errno = 0;

	ಯದಿ (*str == ':') {				/* <:N> */
		str++;
		*upper = strtol(str, &end, 10);
		ಯದಿ (errno || !end || *end || end == str)
			ಪ್ರತ್ಯಾಗಮ -1;
	} ಅನ್ಯಥಾ {
		*upper = *lower = strtol(str, &end, 10);
		ಯದಿ (errno || !end || end == str)
			ಪ್ರತ್ಯಾಗಮ -1;

		ಯದಿ (*end == ':' && !*(end + 1))		/* <M:> */
			*upper = def;
		ಅನ್ಯಥಾ ಯದಿ (*end == '-' || *end == ':') {	/* <M:N> <M-N> */
			str = end + 1;
			end = ರಿಕ್ತ;
			errno = 0;
			*upper = strtol(str, &end, 10);

			ಯದಿ (errno || !end || *end || end == str)
				ಪ್ರತ್ಯಾಗಮ -1;
		}
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಧ್ರುವ ವರ್ಣ *ul_path_next_segment(ಧ್ರುವ ವರ್ಣ *str, size_t *sz)
{
	ಧ್ರುವ ವರ್ಣ *start, *p;

	start = str;
	*sz = 0;
	ಯಾವತ್ (start && *start == '/' && *(start + 1) == '/')
		start++;

	ಯದಿ (!start || !*start)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಪ್ರತಿ (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	ಪ್ರತ್ಯಾಗಮ start;
}

ಪೂರ್ಣಾಙ್ಕ streq_paths(ಧ್ರುವ ವರ್ಣ *a, ಧ್ರುವ ವರ್ಣ *b)
{
	ಯಾವತ್ (a && b) {
		size_t a_sz, b_sz;
		ಧ್ರುವ ವರ್ಣ *a_seg = ul_path_next_segment(a, &a_sz);
		ಧ್ರುವ ವರ್ಣ *b_seg = ul_path_next_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		ಯದಿ (a_sz + b_sz == 0)
			ಪ್ರತ್ಯಾಗಮ 1;

		/* ignore trailing slash */
		ಯದಿ (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			ಪ್ರತ್ಯಾಗಮ 1;

		ಯದಿ (!a_seg || !b_seg)
			ಭಙ್ಗ;
		ಯದಿ (a_sz != b_sz || strncmp(a_seg, b_seg, a_sz) != 0)
			ಭಙ್ಗ;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	ಪ್ರತ್ಯಾಗಮ 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
ಪೂರ್ಣಾಙ್ಕ ul_path_is_within(ಧ್ರುವ ವರ್ಣ *path, ಧ್ರುವ ವರ್ಣ *dir)
{
	ಯದಿ (!path || !dir || !*path || !*dir)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯಾವತ್ (path && dir) {
		size_t p_sz, d_sz;
		ಧ್ರುವ ವರ್ಣ *p_seg = ul_path_next_segment(path, &p_sz);
		ಧ್ರುವ ವರ್ಣ *d_seg = ul_path_next_segment(dir, &d_sz);

		/* all @dir segments matched */
		ಯದಿ (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			ಪ್ರತ್ಯಾಗಮ 1;

		ಯದಿ (!p_seg || !d_seg)
			ಭಙ್ಗ;
		ಯದಿ (p_sz != d_sz || strncmp(p_seg, d_seg, p_sz) != 0)
			ಭಙ್ಗ;

		path = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
ವರ್ಣ *ul_strnconcat(ಧ್ರುವ ವರ್ಣ *s, ಧ್ರುವ ವರ್ಣ *suffix, size_t b)
{
	size_t a;
	ವರ್ಣ *r;

	ಯದಿ (!s && !suffix)
		ಪ್ರತ್ಯಾಗಮ strdup("");
	ಯದಿ (!s)
		ಪ್ರತ್ಯಾಗಮ strndup(suffix, b);
	ಯದಿ (!suffix)
		ಪ್ರತ್ಯಾಗಮ strdup(s);

	ಪ್ರತಿಜ್ಞಾ(s);
	ಪ್ರತಿಜ್ಞಾ(suffix);

	a = strlen(s);
	ಯದಿ (b > ((size_t) -1) - a)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	r = malloc(a + b + 1);
	ಯದಿ (!r)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	memcpy(r, s, a);
	memcpy(r + a, suffix, b);
	r[a+b] = 0;

	ಪ್ರತ್ಯಾಗಮ r;
}

/* concatenate two strings to a new string */
ವರ್ಣ *ul_strconcat(ಧ್ರುವ ವರ್ಣ *s, ಧ್ರುವ ವರ್ಣ *suffix)
{
	ಪ್ರತ್ಯಾಗಮ ul_strnconcat(s, suffix, suffix ? strlen(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
ವರ್ಣ *ul_strfconcat(ಧ್ರುವ ವರ್ಣ *s, ಧ್ರುವ ವರ್ಣ *format, ...)
{
	va_list ap;
	ವರ್ಣ *val, *res;
	ಪೂರ್ಣಾಙ್ಕ sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	ಯದಿ (sz < 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	res = ul_strnconcat(s, val, sz);
	ಮುಕ್ತ(val);
	ಪ್ರತ್ಯಾಗಮ res;
}

ಪೂರ್ಣಾಙ್ಕ ul_strappend(ವರ್ಣ **a, ಧ್ರುವ ವರ್ಣ *b)
{
	size_t al, bl;
	ವರ್ಣ *tmp;

	ಯದಿ (!a)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	ಯದಿ (!b || !*b)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಯದಿ (!*a) {
		*a = strdup(b);
		ಪ್ರತ್ಯಾಗಮ !*a ? -ENOMEM : 0;
	}

	al = strlen(*a);
	bl = strlen(b);

	tmp = realloc(*a, al + bl + 1);
	ಯದಿ (!tmp)
		ಪ್ರತ್ಯಾಗಮ -ENOMEM;
	*a = tmp;
	memcpy((*a) + al, b, bl + 1);
	ಪ್ರತ್ಯಾಗಮ 0;
}

/* the hybrid version of strfconcat and strappend. */
ಪೂರ್ಣಾಙ್ಕ strfappend(ವರ್ಣ **a, ಧ್ರುವ ವರ್ಣ *format, ...)
{
	va_list ap;
	ಪೂರ್ಣಾಙ್ಕ res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ res;
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ ul_strvfappend(ವರ್ಣ **a, ಧ್ರುವ ವರ್ಣ *format, va_list ap)
{
	ವರ್ಣ *val;
	ಪೂರ್ಣಾಙ್ಕ sz;
	ಪೂರ್ಣಾಙ್ಕ res;

	sz = vasprintf(&val, format, ap);
	ಯದಿ (sz < 0)
		ಪ್ರತ್ಯಾಗಮ -errno;

	res = ul_strappend(a, val);
	ಮುಕ್ತ(val);
	ಪ್ರತ್ಯಾಗಮ res;
}

ಸ್ಥಿರ size_t strcspn_escaped(ಧ್ರುವ ವರ್ಣ *s, ಧ್ರುವ ವರ್ಣ *reject)
{
	ಪೂರ್ಣಾಙ್ಕ escaped = 0;
	ಪೂರ್ಣಾಙ್ಕ n;

	ಪ್ರತಿ (n=0; s[n]; n++) {
		ಯದಿ (escaped)
			escaped = 0;
		ಅನ್ಯಥಾ ಯದಿ (s[n] == '\\')
			escaped = 1;
		ಅನ್ಯಥಾ ಯದಿ (strchr(reject, s[n]))
			ಭಙ್ಗ;
	}

	/* if s ends in \, return index of previous char */
	ಪ್ರತ್ಯಾಗಮ n - escaped;
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
ವರ್ಣ *ul_strchr_escaped(ಧ್ರುವ ವರ್ಣ *s, ಪೂರ್ಣಾಙ್ಕ c)
{
	ವರ್ಣ *p;
	ಪೂರ್ಣಾಙ್ಕ esc = 0;

	ಪ್ರತಿ (p = (ವರ್ಣ *) s; p && *p; p++) {
		ಯದಿ (!esc && *p == '\\') {
			esc = 1;
			ಅನುವರ್ತ;
		}
		ಯದಿ (*p == c && (!esc || c == '\\'))
			ಪ್ರತ್ಯಾಗಮ p;
		esc = 0;
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/* Split a string into words. */
ಧ್ರುವ ವರ್ಣ *ul_split(ಧ್ರುವ ವರ್ಣ **state, size_t *l, ಧ್ರುವ ವರ್ಣ *separator, ಪೂರ್ಣಾಙ್ಕ quoted)
{
	ಧ್ರುವ ವರ್ಣ *current;

	current = *state;

	ಯದಿ (!*current) {
		ಪ್ರತಿಜ್ಞಾ(**state == '\0');
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	current += strspn(current, separator);
	ಯದಿ (!*current) {
		*state = current;
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಯದಿ (quoted && strchr("\'\"", *current)) {
		ವರ್ಣ quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		ಯದಿ (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
		*state = current++ + *l + 2;
	} ಅನ್ಯಥಾ ಯದಿ (quoted) {
		*l = strcspn_escaped(current, separator);
		ಯದಿ (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
		*state = current + *l;
	} ಅನ್ಯಥಾ {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	ಪ್ರತ್ಯಾಗಮ current;
}

/* Rewind file pointer forward to new line.  */
ಪೂರ್ಣಾಙ್ಕ skip_fline(FILE *fp)
{
	ಪೂರ್ಣಾಙ್ಕ ch;

	ಕುರು {
		ಯದಿ ((ch = fgetc(fp)) == EOF)
			ಪ್ರತ್ಯಾಗಮ 1;
		ಯದಿ (ch == '\n')
			ಪ್ರತ್ಯಾಗಮ 0;
	} ಯಾವತ್ (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
ಪೂರ್ಣಾಙ್ಕ ul_stralnumcmp(ಧ್ರುವ ವರ್ಣ *p1, ಧ್ರುವ ವರ್ಣ *p2)
{
	ಧ್ರುವ unsigned ವರ್ಣ *s1 = (ಧ್ರುವ unsigned ವರ್ಣ *) p1;
	ಧ್ರುವ unsigned ವರ್ಣ *s2 = (ಧ್ರುವ unsigned ವರ್ಣ *) p2;
	unsigned ವರ್ಣ c1, c2;

	ಕುರು {
		ಕುರು {
			c1 = (unsigned ವರ್ಣ) *s1++;
		} ಯಾವತ್ (c1 != '\0' && !isalnum((unsigned ಪೂರ್ಣಾಙ್ಕ) c1));

		ಕುರು {
			c2 = (unsigned ವರ್ಣ) *s2++;
		} ಯಾವತ್ (c2 != '\0' && !isalnum((unsigned ಪೂರ್ಣಾಙ್ಕ) c2));

		ಯದಿ (c1 != '\0')
			c1 = tolower(c1);
		ಯದಿ (c2 != '\0')
			c2 = tolower(c2);
		ಯದಿ (c1 == '\0')
			ಪ್ರತ್ಯಾಗಮ c1 - c2;
	} ಯಾವತ್ (c1 == c2);

	ಪ್ರತ್ಯಾಗಮ c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
ಪೂರ್ಣಾಙ್ಕ ul_optstr_next(ವರ್ಣ **optstr, ವರ್ಣ **name, size_t *namesz,
		   ವರ್ಣ **ಮೌಲ್ಯ, size_t *valsz)
{
	ಪೂರ್ಣಾಙ್ಕ open_quote = 0;
	ವರ್ಣ *start = ರಿಕ್ತ, *stop = ರಿಕ್ತ, *p, *sep = ರಿಕ್ತ;
	ವರ್ಣ *optstr0;

	ಪ್ರತಿಜ್ಞಾ(optstr);
	ಪ್ರತಿಜ್ಞಾ(*optstr);

	optstr0 = *optstr;

	ಯದಿ (name)
		*name = ರಿಕ್ತ;
	ಯದಿ (namesz)
		*namesz = 0;
	ಯದಿ (ಮೌಲ್ಯ)
		*ಮೌಲ್ಯ = ರಿಕ್ತ;
	ಯದಿ (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	ಯಾವತ್ (optstr0 && *optstr0 == ',')
		optstr0++;

	ಪ್ರತಿ (p = optstr0; p && *p; p++) {
		ಯದಿ (!start && *p == '=')
			ಪ್ರತ್ಯಾಗಮ -EINVAL;
		ಯದಿ (!start)
			start = p;		/* beginning of the option item */
		ಯದಿ (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		ಯದಿ (open_quote)
			ಅನುವರ್ತ;		/* still in quoted block */
		ಯದಿ (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		ಯದಿ (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		ಅನ್ಯಥಾ ಯದಿ (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		ಯದಿ (!start || !stop)
			ಅನುವರ್ತ;
		ಯದಿ (stop <= start)
			ಪ್ರತ್ಯಾಗಮ -EINVAL;

		ಯದಿ (name)
			*name = start;
		ಯದಿ (namesz)
			*namesz = sep ? sep - start : stop - start;
		*optstr = *stop ? stop + 1 : stop;

		ಯದಿ (sep) {
			ಯದಿ (ಮೌಲ್ಯ)
				*ಮೌಲ್ಯ = sep + 1;
			ಯದಿ (valsz)
				*valsz = stop - sep - 1;
		}
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಪ್ರತ್ಯಾಗಮ 1;				/* end of optstr */
}

ಪೂರ್ಣಾಙ್ಕ ul_optstr_is_valid(ಧ್ರುವ ವರ್ಣ *optstr)
{
	ಪೂರ್ಣಾಙ್ಕ rc;
	ವರ್ಣ *p = (ವರ್ಣ *) optstr;

	ಯಾವತ್ ((rc = ul_optstr_next(&p, ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ)) == 0);
	ಪ್ರತ್ಯಾಗಮ rc < 0 ? 0 : 1;
}

ವರ್ಣ *ul_optstr_get_ಮೌಲ್ಯ(ಧ್ರುವ ವರ್ಣ *optstr, ಧ್ರುವ ವರ್ಣ *ಕೀಲಿ)
{
	size_t sz, namesz = 0, valsz = 0;
	ವರ್ಣ *name = ರಿಕ್ತ, *ಮೌಲ್ಯ = ರಿಕ್ತ;
	ವರ್ಣ *p = (ವರ್ಣ *) optstr;

	ಯದಿ (!optstr || !ಕೀಲಿ || !*ಕೀಲಿ)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	sz = strlen(ಕೀಲಿ);
	ಯಾವತ್ (ul_optstr_next(&p, &name, &namesz, &ಮೌಲ್ಯ, &valsz) == 0) {
		ಯದಿ (namesz != sz || !valsz)
			ಅನುವರ್ತ;
		ಯದಿ (strncmp(name, ಕೀಲಿ, namesz) == 0)
			ಪ್ರತ್ಯಾಗಮ strndup(ಮೌಲ್ಯ, valsz);
	}
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(ವರ್ಣ *str)
{
	size_t len;
	ವರ್ಣ q;

	ಯದಿ (!str || !*str)
		ಪ್ರತ್ಯಾಗಮ 0;

	len = strlen(str);
	q = *str;

	ಯದಿ ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		ಪ್ರತ್ಯಾಗಮ len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	ಪ್ರತ್ಯಾಗಮ len - 2;
}

#ifdef TEST_PROGRAM_STRUTILS

ಸಂರಚನಾ testS {
	ವರ್ಣ *name;
	ವರ್ಣ *ಮೌಲ್ಯ;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ test_strdup_to_member(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ಸಂರಚನಾ testS *xx;

	ಯದಿ (argc < 3)
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;

	xx = calloc(1, ಪರಿಮಾಣ(*xx));
	ಯದಿ (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_to_struct_member(xx, name, argv[1]);
	strdup_to_struct_member(xx, ಮೌಲ್ಯ, argv[2]);

	ಯದಿ (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->ಮೌಲ್ಯ, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	printf("1: '%s', 2: '%s'\n", xx->name, xx->ಮೌಲ್ಯ);

	ಮುಕ್ತ(xx->name);
	ಮುಕ್ತ(xx->ಮೌಲ್ಯ);
	ಮುಕ್ತ(xx);
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ test_strutils_sizes(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	uintmax_t size = 0;
	ವರ್ಣ *hum1, *hum2, *hum3;

	ಯದಿ (argc < 2)
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;

	ಯದಿ (strtosize(argv[1], &size))
		errx(EXIT_FAILURE, "invalid size '%s' value", argv[1]);

	hum1 = size_to_human_string(SIZE_SUFFIX_1LETTER, size);
	hum2 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE, size);
	hum3 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE |
				    SIZE_DECIMAL_2DIGITS, size);

	printf("%25s : %20ju : %8s : %12s : %13s\n", argv[1], size, hum1, hum2, hum3);
	ಮುಕ್ತ(hum1);
	ಮುಕ್ತ(hum2);
	ಮುಕ್ತ(hum3);

	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ test_strutils_cmp_paths(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ಪೂರ್ಣಾಙ್ಕ rc = streq_paths(argv[1], argv[2]);

	ಯದಿ (argc < 3)
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;

	printf("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ test_strutils_path_is_within(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ಪೂರ್ಣಾಙ್ಕ rc;

	ಯದಿ (argc < 3)
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;

	rc = ul_path_is_within(argv[1], argv[2]);

	printf("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ test_strutils_normalize(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	unsigned ವರ್ಣ *src, *dst, *org;
	size_t sz, len;

	ಯದಿ (argc < 2)
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;

	org = (unsigned ವರ್ಣ *) strdup(argv[1]);
	src = (unsigned ವರ್ಣ *) strdup((ವರ್ಣ *) org);
	len = strlen((ವರ್ಣ *) src);
	dst = malloc(len + 1);

	ಯದಿ (!org || !src || !dst)
		ಗಚ್ಛ done;

	/* two buffers */
	sz = __normalize_whitespace(src, len, dst, len + 1);
	printf("1: '%s' --> '%s' [sz=%zu]\n", src, dst, sz);

	/* one buffer */
	sz = normalize_whitespace(src);
	printf("2: '%s' --> '%s' [sz=%zu]\n", org, src, sz);

done:
	ಮುಕ್ತ(src);
	ಮುಕ್ತ(dst);
	ಮುಕ್ತ(org);

	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ test_strutils_cstrcasecmp(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ವರ್ಣ *a, *b;

	ಯದಿ (argc < 3)
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	ಯದಿ (!a || !b)
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;

	printf("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	printf("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	printf("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, strlen(a)));

	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ಯದಿ (argc == 3 && strcmp(argv[1], "--size") == 0) {
		ಪ್ರತ್ಯಾಗಮ test_strutils_sizes(argc - 1, argv + 1);

	} ಅನ್ಯಥಾ ಯದಿ (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		printf("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						ರಿಕ್ತ));
		ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
	} ಅನ್ಯಥಾ ಯದಿ (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		ಯದಿ (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		printf("'%s'-->%s\n", argv[2], val ? "true" : "false");
		ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;

	} ಅನ್ಯಥಾ ಯದಿ (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		ಪ್ರತ್ಯಾಗಮ test_strutils_cmp_paths(argc - 1, argv + 1);

	} ಅನ್ಯಥಾ ಯದಿ (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		ಪ್ರತ್ಯಾಗಮ test_strutils_path_is_within(argc - 1, argv + 1);

	} ಅನ್ಯಥಾ ಯದಿ (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		ಪ್ರತ್ಯಾಗಮ test_strdup_to_member(argc - 1, argv + 1);

	} ಅನ್ಯಥಾ ಯದಿ  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		printf("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;

	} ಅನ್ಯಥಾ ಯದಿ (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		ಪ್ರತ್ಯಾಗಮ test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} ಅನ್ಯಥಾ ಯದಿ (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		ಪ್ರತ್ಯಾಗಮ test_strutils_normalize(argc - 1, argv + 1);

	} ಅನ್ಯಥಾ ಯದಿ (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		printf("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
	} ಅನ್ಯಥಾ ಯದಿ (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		printf("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
	} ಅನ್ಯಥಾ ಯದಿ (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		printf("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
	} ಅನ್ಯಥಾ ಯದಿ (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		printf("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
	} ಅನ್ಯಥಾ ಯದಿ (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		printf("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
	} ಅನ್ಯಥಾ ಯದಿ (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		printf("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;

	} ಅನ್ಯಥಾ ಯದಿ (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		printf("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;

	} ಅನ್ಯಥಾ ಯದಿ (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		ವರ್ಣ *buf = "abc\0Y\0\0xyz\0X";
		ವರ್ಣ *end = buf + 12;
		ವರ್ಣ *p = buf;

		ಕುರು {
			printf("str: '%s'\n", p);
		} ಯಾವತ್ ((p = ul_next_string(p, end)));

		ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;

	} ಅನ್ಯಥಾ ಯದಿ (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		ವರ್ಣ *name = ರಿಕ್ತ, *val = ರಿಕ್ತ;
		ವರ್ಣ *p = argv[2];
		ಪೂರ್ಣಾಙ್ಕ rc;

		ಯದಿ (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		ಯಾವತ್ ((rc = ul_optstr_next(&p, &name, &namesz, &val, &valsz)) == 0) {
			printf("'%.*s' : '%.*s'\n", (ಪೂರ್ಣಾಙ್ಕ) namesz, name,
						   (ಪೂರ್ಣಾಙ್ಕ) valsz, val);
		}
		ಯದಿ (rc == 1)
			ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
	} ಅನ್ಯಥಾ ಯದಿ (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		ವರ್ಣ *unquoted = strdup(argv[2]);

		ಯದಿ (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		printf("%s-->%s\n", argv[2], unquoted);
		ಮುಕ್ತ(unquoted);
		ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;

	} ಅನ್ಯಥಾ {
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

	ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_STRUTILS */
