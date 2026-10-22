/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Karel Zak <kzak@redhat.com> [2010]
 *          Davidlohr Bueso <dave@gnu.org> [2010]
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <inttypes.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "cctype.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "nls.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "strutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "timeutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "bitops.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "pathnames.h"

ਸਥਿਰ ਪੂਰਨ STRTOXX_EXIT_ਕੋਡ = EXIT_FAILURE;

ਖਾਲੀ strutils_ਸੈੱਟ_exitcode(ਪੂਰਨ ex)
{
	STRTOXX_EXIT_ਕੋਡ = ex;
}

ਸਥਿਰ ਪੂਰਨ do_scale_by_power (uintmax_t *x, ਪੂਰਨ ਅਧਾਰ, ਪੂਰਨ power)
{
	ਜਦੋਂ (power--) {
		ਜੇ (UINTMAX_MAX / ਅਧਾਰ < *x)
			ਵਾਪਸ -ERANGE;
		*x *= ਅਧਾਰ;
	}
	ਵਾਪਸ 0;
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
ਪੂਰਨ ul_parse_ਮਾਪ(ਅਚਲ ਅੱਖਰ *str, uintmax_t *res, ਪੂਰਨ *power)
{
	ਅਚਲ ਅੱਖਰ *p;
	ਅੱਖਰ *ਸਿਰਾ;
	uintmax_t x, frac = 0;
	ਪੂਰਨ ਅਧਾਰ = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	ਸਥਿਰ ਅਚਲ ਅੱਖਰ *suf  = "KMGTPEZY";
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ *suf2 = "kmgtpezy";
	ਅਚਲ ਅੱਖਰ *sp;

	*res = 0;

	ਜੇ (!str || !*str) {
		rc = -EINVAL;
		ਜਾਓ err;
	}

	/* Only positive numbers are acceptable
	 *
	 * Note that this check is not perfect, it would be better to
	 * use lconv->negative_sign. But coreutils use the same solution,
	 * so it's probably good enough...
	 */
	p = str;
	ਜਦੋਂ (ਖਾਲੀ_ਥਾਂ_ਹੈ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *p))
		p++;
	ਜੇ (*p == '-') {
		rc = -EINVAL;
		ਜਾਓ err;
	}

	errno = 0, ਸਿਰਾ = ਨਿਰਾ;
	x = strtoumax(str, &ਸਿਰਾ, 0);

	ਜੇ (ਸਿਰਾ == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		ਜਾਓ err;
	}
	ਜੇ (!ਸਿਰਾ || !*ਸਿਰਾ)
		ਜਾਓ ਪੂਰਾ;			/* without suffix */
	p = ਸਿਰਾ;

	/*
	 * Check size suffixes
	 */
check_suffix:
	ਜੇ (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		ਅਧਾਰ = 1024;			/* XiB, 2^N */
	ਨਹੀਂਤਾਂ ਜੇ ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		ਅਧਾਰ = 1000;			/* XB, 10^N */
	ਨਹੀਂਤਾਂ ਜੇ (*(p + 1)) {
		ਬਣਤਰ lconv ਅਚਲ *l = localeconv();
		ਅਚਲ ਅੱਖਰ *dp = l ? l->decimal_ਬਿੰਦੂ : ਨਿਰਾ;
		size_t dpsz = dp ? ਲੜੀ_ਲੰਬਾਈ(dp) : 0;

		ਜੇ (frac == 0 && *p && dp && ਲੜੀ_ਤੁਲਨਾ(dp, p, dpsz) == 0) {
			ਅਚਲ ਅੱਖਰ *fstr = p + dpsz;

			ਲਈ (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			ਜੇ (ਅੰਕ_ਹੈ(*fstr)) {
				errno = 0, ਸਿਰਾ = ਨਿਰਾ;
				frac = strtoumax(fstr, &ਸਿਰਾ, 0);
				ਜੇ (ਸਿਰਾ == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					ਜਾਓ err;
				}
			} ਨਹੀਂਤਾਂ
				ਸਿਰਾ = (ਅੱਖਰ *) p;

			ਜੇ (!ਸਿਰਾ || !*ਸਿਰਾ) {
				rc = -EINVAL;
				ਜਾਓ err;	/* without suffix, but with fractions */
			}
			p = ਸਿਰਾ;
			ਜਾਓ check_suffix;
		}
		rc = -EINVAL;
		ਜਾਓ err;			/* unexpected suffix */
	}

	sp = strchr(suf, *p);
	ਜੇ (sp)
		pwr = (sp - suf) + 1;
	ਨਹੀਂਤਾਂ {
		sp = strchr(suf2, *p);
		ਜੇ (sp)
			pwr = (sp - suf2) + 1;
		ਨਹੀਂਤਾਂ {
			rc = -EINVAL;
			ਜਾਓ err;
		}
	}

	ਜੇ (power)
		*power = pwr;

	rc = do_scale_by_power(&x, ਅਧਾਰ, pwr);
	ਜੇ (rc)
		ਜਾਓ err;

	ਜੇ (frac && pwr) {
		ਪੂਰਨ i;
		uintmax_t frac_ਅਧਾਰ = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_ਅਧਾਰ, ਅਧਾਰ, pwr);
		ਜੇ (rc)
			ਜਾਓ err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		ਕਰੋ {
			frac_val = ((frac % 10) * frac_ਅਧਾਰ + frac_val) / 10;
			frac /= 10;
		} ਜਦੋਂ (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		ਲਈ (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		ਜੇ (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			ਜਾਓ err;
		}
		x += frac_val;
	}
ਪੂਰਾ:
	*res = x;
err:
	ਜੇ (rc < 0)
		errno = -rc;
	ਵਾਪਸ rc;
}

ਪੂਰਨ strtosize(ਅਚਲ ਅੱਖਰ *str, uintmax_t *res)
{
	ਵਾਪਸ ul_parse_ਮਾਪ(str, res, ਨਿਰਾ);
}

ਪੂਰਨ isdigit_strend(ਅਚਲ ਅੱਖਰ *str, ਅਚਲ ਅੱਖਰ **ਸਿਰਾ)
{
	ਅਚਲ ਅੱਖਰ *p;

	ਲਈ (p = str; p && *p && ਅੰਕ_ਹੈ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *p); p++);

	ਜੇ (ਸਿਰਾ)
		*ਸਿਰਾ = p;
	ਵਾਪਸ p && p > str && !*p;
}

ਪੂਰਨ isxdigit_strend(ਅਚਲ ਅੱਖਰ *str, ਅਚਲ ਅੱਖਰ **ਸਿਰਾ)
{
	ਅਚਲ ਅੱਖਰ *p;

	ਲਈ (p = str; p && *p && isxdigit((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *p); p++);

	ਜੇ (ਸਿਰਾ)
		*ਸਿਰਾ = p;

	ਵਾਪਸ p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
ਪੂਰਨ ul_strtobool(ਅਚਲ ਅੱਖਰ *str, bool *ਨਤੀਜਾ)
{
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	ਜੇ (!str || !ਨਤੀਜਾ)
		ਵਾਪਸ -(errno = EINVAL);

	errno = 0;

	ਲਈ (i = 0; i < ARRAY_ਮਾਪ(bool_true); i++) {
		ਜੇ (c_strcasecmp(str, bool_true[i]) == 0) {
			*ਨਤੀਜਾ = true;
			ਵਾਪਸ 0;
		}
	}

	ਲਈ (i = 0; i < ARRAY_ਮਾਪ(bool_false); i++) {
		ਜੇ (c_strcasecmp(str, bool_false[i]) == 0) {
			*ਨਤੀਜਾ = false;
			ਵਾਪਸ 0;
		}
	}

	ਵਾਪਸ -(errno = EINVAL);
}

ਪੂਰਨ ul_parse_switch(ਅਚਲ ਅੱਖਰ *arg, ...)
{
	ਅਚਲ ਅੱਖਰ *a, *b;
	va_list ap;

	va_start(ap, arg);
	ਕਰੋ {
		a = va_arg(ap, ਅੱਖਰ *);
		ਜੇ (!a)
			ਰੋਕੋ;
		b = va_arg(ap, ਅੱਖਰ *);
		ਜੇ (!b)
			ਰੋਕੋ;

		ਜੇ (strcmp(arg, a) == 0) {
			va_end(ap);
			ਵਾਪਸ 1;
		}

		ਜੇ (strcmp(arg, b) == 0) {
			va_end(ap);
			ਵਾਪਸ 0;
		}
	} ਜਦੋਂ (1);
	va_end(ap);

	errx(STRTOXX_EXIT_ਕੋਡ, _("unsupported argument: %s"), arg);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ HAVE_MEMPCPY
ਖਾਲੀ *mempcpy(ਖਾਲੀ *ਸੀਮਿਤ dest, ਅਚਲ ਖਾਲੀ *ਸੀਮਿਤ src, size_t n)
{
	ਵਾਪਸ ((ਅੱਖਰ *)ਸਮ੍ਰਿਤੀ_ਨਕਲ(dest, src, n)) + n;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ HAVE_STRNLEN
size_t strnlen(ਅਚਲ ਅੱਖਰ *s, size_t maxlen)
{
	size_t i;

	ਲਈ (i = 0; i < maxlen; i++) {
		ਜੇ (s[i] == '\0')
			ਵਾਪਸ i;
	}
	ਵਾਪਸ maxlen;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ HAVE_STRNCHR
ਅੱਖਰ *strnchr(ਅਚਲ ਅੱਖਰ *s, size_t maxlen, ਪੂਰਨ c)
{
	ਲਈ (; maxlen-- && *s != '\0'; ++s)
		ਜੇ (*s == (ਅੱਖਰ)c)
			ਵਾਪਸ (ਅੱਖਰ *)s;
	ਵਾਪਸ ਨਿਰਾ;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ HAVE_STRNDUP
ਅੱਖਰ *strndup(ਅਚਲ ਅੱਖਰ *s, size_t n)
{
	size_t len = strnlen(s, n);
	ਅੱਖਰ *new = ਸਮ੍ਰਿਤੀ_ਦਿਓ((len + 1) * ਆਕਾਰ(ਅੱਖਰ));
	ਜੇ (!new)
		ਵਾਪਸ ਨਿਰਾ;
	new[len] = '\0';
	ਵਾਪਸ (ਅੱਖਰ *) ਸਮ੍ਰਿਤੀ_ਨਕਲ(new, s, len);
}
#ਜੇ_ਅੰਤ

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
ਪੂਰਨ ul_strtos64(ਅਚਲ ਅੱਖਰ *str, int64_t *ਅੰਕ, ਪੂਰਨ ਅਧਾਰ)
{
	ਅੱਖਰ *ਸਿਰਾ = ਨਿਰਾ;

	ਜੇ (str == ਨਿਰਾ || *str == '\0')
		ਵਾਪਸ -(errno = EINVAL);

	errno = 0;
	*ਅੰਕ = (int64_t) strtoimax(str, &ਸਿਰਾ, ਅਧਾਰ);

	ਜੇ (errno != 0)
		ਵਾਪਸ -errno;
	ਜੇ (str == ਸਿਰਾ || (ਸਿਰਾ && *ਸਿਰਾ))
		ਵਾਪਸ -(errno = EINVAL);
	ਵਾਪਸ 0;
}

ਪੂਰਨ ul_strtou64(ਅਚਲ ਅੱਖਰ *str, uint64_t *ਅੰਕ, ਪੂਰਨ ਅਧਾਰ)
{
	ਅੱਖਰ *ਸਿਰਾ = ਨਿਰਾ;
	int64_t tmp;

	ਜੇ (str == ਨਿਰਾ || *str == '\0')
		ਵਾਪਸ -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &ਸਿਰਾ, ਅਧਾਰ);
	ਜੇ (tmp < 0)
		errno = ERANGE;
	ਨਹੀਂਤਾਂ {
		errno = 0;
		*ਅੰਕ = strtoumax(str, &ਸਿਰਾ, ਅਧਾਰ);
	}

	ਜੇ (errno != 0)
		ਵਾਪਸ -errno;
	ਜੇ (str == ਸਿਰਾ || (ਸਿਰਾ && *ਸਿਰਾ))
		ਵਾਪਸ -(errno = EINVAL);
	ਵਾਪਸ 0;
}

ਪੂਰਨ ul_strtos32(ਅਚਲ ਅੱਖਰ *str, int32_t *ਅੰਕ, ਪੂਰਨ ਅਧਾਰ)
{
	int64_t tmp;
	ਪੂਰਨ rc;

	rc = ul_strtos64(str, &tmp, ਅਧਾਰ);
	ਜੇ (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	ਜੇ (rc == 0)
		*ਅੰਕ = (int32_t) tmp;
	ਵਾਪਸ rc;
}

ਪੂਰਨ ul_strtou32(ਅਚਲ ਅੱਖਰ *str, uint32_t *ਅੰਕ, ਪੂਰਨ ਅਧਾਰ)
{
	uint64_t tmp;
	ਪੂਰਨ rc;

	rc = ul_strtou64(str, &tmp, ਅਧਾਰ);
	ਜੇ (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	ਜੇ (rc == 0)
		*ਅੰਕ = (uint32_t) tmp;
	ਵਾਪਸ rc;
}

ਪੂਰਨ ul_strtou16(ਅਚਲ ਅੱਖਰ *str, uint16_t *ਅੰਕ, ਪੂਰਨ ਅਧਾਰ)
{
	uint64_t tmp;
	ਪੂਰਨ rc;

	rc = ul_strtou64(str, &tmp, ਅਧਾਰ);
	ਜੇ (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	ਜੇ (rc == 0)
		*ਅੰਕ = (uint16_t) tmp;
	ਵਾਪਸ rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(ਅਚਲ ਅੱਖਰ *str, ਪੂਰਨ ਅਧਾਰ, ਅਚਲ ਅੱਖਰ *errmesg,
			     int64_t ਨੀਵਾਂ, int64_t ਉੱਪਰ)
{
	int64_t ਅੰਕ = 0;
	ਪੂਰਨ rc;

	rc = ul_strtos64(str, &ਅੰਕ, ਅਧਾਰ);
	ਜੇ (rc == 0 && ((ਨੀਵਾਂ && ਅੰਕ < ਨੀਵਾਂ) || (ਉੱਪਰ && ਅੰਕ > ਉੱਪਰ)))
		rc = -(errno = ERANGE);

	ਜੇ (rc) {
		ਜੇ (errno == ERANGE)
			err(STRTOXX_EXIT_ਕੋਡ, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_ਕੋਡ, "%s: '%s'", errmesg, str);
	}
	ਵਾਪਸ ਅੰਕ;
}

uint64_t str2unum_or_err(ਅਚਲ ਅੱਖਰ *str, ਪੂਰਨ ਅਧਾਰ, ਅਚਲ ਅੱਖਰ *errmesg, uint64_t ਉੱਪਰ)
{
	uint64_t ਅੰਕ = 0;
	ਪੂਰਨ rc;

	rc = ul_strtou64(str, &ਅੰਕ, ਅਧਾਰ);
	ਜੇ (rc == 0 && (ਉੱਪਰ && ਅੰਕ > ਉੱਪਰ))
		rc = -(errno = ERANGE);

	ਜੇ (rc) {
		ਜੇ (errno == ERANGE)
			err(STRTOXX_EXIT_ਕੋਡ, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_ਕੋਡ, "%s: '%s'", errmesg, str);
	}
	ਵਾਪਸ ਅੰਕ;
}

ਦੁੱਗਣਾ strtod_or_err(ਅਚਲ ਅੱਖਰ *str, ਅਚਲ ਅੱਖਰ *errmesg)
{
	ਦੁੱਗਣਾ ਅੰਕ;
	ਅੱਖਰ *ਸਿਰਾ = ਨਿਰਾ;

	errno = 0;
	ਜੇ (str == ਨਿਰਾ || *str == '\0')
		ਜਾਓ err;
	ਅੰਕ = strtod(str, &ਸਿਰਾ);

	ਜੇ (errno || str == ਸਿਰਾ || (ਸਿਰਾ && *ਸਿਰਾ))
		ਜਾਓ err;

	ਵਾਪਸ ਅੰਕ;
err:
	ਜੇ (errno == ERANGE)
		err(STRTOXX_EXIT_ਕੋਡ, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_ਕੋਡ, "%s: '%s'", errmesg, str);
}

ਪੂਰਨ ul_strtold(ਅਚਲ ਅੱਖਰ *str, ਲੰਮਾ ਦੁੱਗਣਾ *ਅੰਕ)
{
	ਅੱਖਰ *ਸਿਰਾ = ਨਿਰਾ;

	errno = 0;
	ਜੇ (str == ਨਿਰਾ || *str == '\0')
		ਵਾਪਸ -(errno = EINVAL);
	*ਅੰਕ = strtold(str, &ਸਿਰਾ);

	ਜੇ (errno != 0)
		ਵਾਪਸ -errno;
	ਜੇ (str == ਸਿਰਾ || (ਸਿਰਾ && *ਸਿਰਾ))
		ਵਾਪਸ -(errno = EINVAL);
	ਵਾਪਸ 0;
}

ਲੰਮਾ ਦੁੱਗਣਾ strtold_or_err(ਅਚਲ ਅੱਖਰ *str, ਅਚਲ ਅੱਖਰ *errmesg)
{
	ਲੰਮਾ ਦੁੱਗਣਾ ਅੰਕ = 0;

	ਜੇ (ul_strtold(str, &ਅੰਕ) == 0)
		ਵਾਪਸ ਅੰਕ;
	ਜੇ (errno == ERANGE)
		err(STRTOXX_EXIT_ਕੋਡ, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_ਕੋਡ, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(ਅਚਲ ਅੱਖਰ *str, ਅਚਲ ਅੱਖਰ *errmesg)
{
	uintmax_t ਅੰਕ;

	ਜੇ (strtosize(str, &ਅੰਕ) == 0)
		ਵਾਪਸ ਅੰਕ;

	ਜੇ (errno)
		err(STRTOXX_EXIT_ਕੋਡ, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_ਕੋਡ, "%s: '%s'", errmesg, str);
}


ਖਾਲੀ strtotimeval_or_err(ਅਚਲ ਅੱਖਰ *str, ਬਣਤਰ timeval *tv, ਅਚਲ ਅੱਖਰ *errmesg)
{
	ਲੰਮਾ ਦੁੱਗਣਾ user_ਨਿਵੇਸ਼;

	user_ਨਿਵੇਸ਼ = strtold_or_err(str, errmesg);
	tv->tv_sec = (time_t) user_ਨਿਵੇਸ਼;
	tv->tv_usec = (suseconds_t)((user_ਨਿਵੇਸ਼ - tv->tv_sec) * USEC_ਪ੍ਰਤੀ_SEC);
}

ਖਾਲੀ strtotimespec_or_err(ਅਚਲ ਅੱਖਰ *str, ਬਣਤਰ timespec *ts, ਅਚਲ ਅੱਖਰ *errmesg)
{
	ਲੰਮਾ ਦੁੱਗਣਾ user_ਨਿਵੇਸ਼;

	user_ਨਿਵੇਸ਼ = strtold_or_err(str, errmesg);
	ts->tv_sec = (time_t) user_ਨਿਵੇਸ਼;
	ts->tv_nsec = (ਲੰਮਾ)((user_ਨਿਵੇਸ਼ - ts->tv_sec) * NSEC_ਪ੍ਰਤੀ_SEC);
}

time_t strtotime_or_err(ਅਚਲ ਅੱਖਰ *str, ਅਚਲ ਅੱਖਰ *errmesg)
{
	int64_t user_ਨਿਵੇਸ਼;

	user_ਨਿਵੇਸ਼ = strtos64_or_err(str, errmesg);
	ਵਾਪਸ (time_t) user_ਨਿਵੇਸ਼;
}

bool hyperlinkwanted(ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ)
{
	ਜੇ (ਬਹੁਲਕ && strcmp(ਬਹੁਲਕ, "never") == 0)
		ਵਾਪਸ false;

	ਜੇ (ਬਹੁਲਕ && strcmp(ਬਹੁਲਕ, "always") == 0)
		ਵਾਪਸ true;

	ਜੇ (!ਬਹੁਲਕ || strcmp(ਬਹੁਲਕ, "auto") == 0)
		ਵਾਪਸ isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), ਬਹੁਲਕ);
}

bool annotationwanted(ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ)
{
	ਜੇ (ਬਹੁਲਕ && strcmp(ਬਹੁਲਕ, "never") == 0)
		ਵਾਪਸ false;

	ਜੇ (ਬਹੁਲਕ && strcmp(ਬਹੁਲਕ, "always") == 0)
		ਵਾਪਸ true;

	ਜੇ (!ਬਹੁਲਕ || strcmp(ਬਹੁਲਕ, "auto") == 0)
		ਵਾਪਸ isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), ਬਹੁਲਕ);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
ਅੱਖਰ *xstrmode(mode_t ਬਹੁਲਕ, ਅੱਖਰ *str)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ i = 0;

	ਜੇ (S_ISDIR(ਬਹੁਲਕ))
		str[i++] = 'd';
	ਨਹੀਂਤਾਂ ਜੇ (S_ISLNK(ਬਹੁਲਕ))
		str[i++] = 'l';
	ਨਹੀਂਤਾਂ ਜੇ (S_ISCHR(ਬਹੁਲਕ))
		str[i++] = 'c';
	ਨਹੀਂਤਾਂ ਜੇ (S_ISBLK(ਬਹੁਲਕ))
		str[i++] = 'b';
	ਨਹੀਂਤਾਂ ਜੇ (S_ISSOCK(ਬਹੁਲਕ))
		str[i++] = 's';
	ਨਹੀਂਤਾਂ ਜੇ (S_ISFIFO(ਬਹੁਲਕ))
		str[i++] = 'p';
	ਨਹੀਂਤਾਂ ਜੇ (S_ISREG(ਬਹੁਲਕ))
		str[i++] = '-';

	str[i++] = ਬਹੁਲਕ & S_IRUSR ? 'r' : '-';
	str[i++] = ਬਹੁਲਕ & S_IWUSR ? 'w' : '-';
	str[i++] = (ਬਹੁਲਕ & S_ISUID
		? (ਬਹੁਲਕ & S_IXUSR ? 's' : 'S')
		: (ਬਹੁਲਕ & S_IXUSR ? 'x' : '-'));
	str[i++] = ਬਹੁਲਕ & S_IRGRP ? 'r' : '-';
	str[i++] = ਬਹੁਲਕ & S_IWGRP ? 'w' : '-';
	str[i++] = (ਬਹੁਲਕ & S_ISGID
		? (ਬਹੁਲਕ & S_IXGRP ? 's' : 'S')
		: (ਬਹੁਲਕ & S_IXGRP ? 'x' : '-'));
	str[i++] = ਬਹੁਲਕ & S_IROTH ? 'r' : '-';
	str[i++] = ਬਹੁਲਕ & S_IWOTH ? 'w' : '-';
	str[i++] = (ਬਹੁਲਕ & S_ISVTX
		? (ਬਹੁਲਕ & S_IXOTH ? 't' : 'T')
		: (ਬਹੁਲਕ & S_IXOTH ? 'x' : '-'));
	str[i] = '\0';

	ਵਾਪਸ str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
ਸਥਿਰ ਪੂਰਨ get_exp(uint64_t n)
{
	ਪੂਰਨ shft;

	ਲਈ (shft = 10; shft <= 60; shft += 10) {
		ਜੇ (n < (1ULL << shft))
			ਰੋਕੋ;
	}
	ਵਾਪਸ shft - 10;
}

ਅੱਖਰ *ਮਾਪ_ਮੰਜ਼ਿਲ_human_string(ਪੂਰਨ options, uint64_t ਬਾਈਟਾਂ)
{
	ਅੱਖਰ buf[32];
	ਪੂਰਨ dec, ਘਾਤਾਂਕ;
	uint64_t frac;
	ਅਚਲ ਅੱਖਰ *letters = "BKMGTPE";
	ਅੱਖਰ suffix[ਆਕਾਰ(" KiB")], *psuf = suffix;
	ਅੱਖਰ c;

	ਜੇ (options & ਮਾਪ_SUFFIX_SPACE)
		*psuf++ = ' ';


	ਘਾਤਾਂਕ  = get_exp(ਬਾਈਟਾਂ);
	c    = *(letters + (ਘਾਤਾਂਕ ? ਘਾਤਾਂਕ / 10 : 0));
	dec  = ਘਾਤਾਂਕ ? ਬਾਈਟਾਂ / (1ULL << ਘਾਤਾਂਕ) : ਬਾਈਟਾਂ;
	frac = ਘਾਤਾਂਕ ? ਬਾਈਟਾਂ % (1ULL << ਘਾਤਾਂਕ) : 0;

	*psuf++ = c;

	ਜੇ ((options & ਮਾਪ_SUFFIX_3_LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/*ਗੇੜਾ*/
	ਜੇ (frac) {
		/* get 3 digits after decimal point */
		ਜੇ (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (ਘਾਤਾਂਕ - 10)) ;
		ਨਹੀਂਤਾਂ
			frac = (frac * 1000) / (1ULL << (ਘਾਤਾਂਕ)) ;

		ਜੇ (options & ਮਾਪ_DECIMAL_2_DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} ਨਹੀਂਤਾਂ {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		ਜੇ (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	ਜੇ (frac) {
		ਬਣਤਰ lconv ਅਚਲ *l = localeconv();
		ਅੱਖਰ *dp = l ? l->decimal_ਬਿੰਦੂ : ਨਿਰਾ;
		ਪੂਰਨ len;

		ਜੇ (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, ਆਕਾਰ(buf), "%d%s%02" PRIu64, dec, dp, frac);
		ਜੇ (len > 0 && (size_t) len < ਆਕਾਰ(buf)) {
			/* remove potential extraneous zero */
			ਜੇ (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, ਆਕਾਰ(buf) - len);
		} ਨਹੀਂਤਾਂ
			*buf = '\0';	/* snprintf error */
	} ਨਹੀਂਤਾਂ
		snprintf(buf, ਆਕਾਰ(buf), "%d%s", dec, suffix);

	ਵਾਪਸ strdup(buf);
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
ਪੂਰਨ string_ਮੰਜ਼ਿਲ_idarray(ਅਚਲ ਅੱਖਰ *list, ਪੂਰਨ ary[], size_t arysz,
			ਪੂਰਨ (name2id)(ਅਚਲ ਅੱਖਰ *, size_t))
{
	ਅਚਲ ਅੱਖਰ *begin = ਨਿਰਾ, *p;
	size_t n = 0;

	ਜੇ (!list || !*list || !ary || !arysz || !name2id)
		ਵਾਪਸ -1;

	ਲਈ (p = list; p && *p; p++) {
		ਅਚਲ ਅੱਖਰ *ਸਿਰਾ = ਨਿਰਾ;
		ਪੂਰਨ id;

		ਜੇ (n >= arysz)
			ਵਾਪਸ -2;
		ਜੇ (!begin)
			begin = p;		/* begin of the column name */
		ਜੇ (*p == ',')
			ਸਿਰਾ = p;		/* terminate the name */
		ਜੇ (*(p + 1) == '\0')
			ਸਿਰਾ = p + 1;		/* end of string */
		ਜੇ (!begin || !ਸਿਰਾ)
			ਜਾਰੀ;
		ਜੇ (ਸਿਰਾ <= begin)
			ਵਾਪਸ -1;

		id = name2id(begin, ਸਿਰਾ - begin);
		ਜੇ (id == -1)
			ਵਾਪਸ -1;
		ary[ n++ ] = id;
		begin = ਨਿਰਾ;
		ਜੇ (ਸਿਰਾ && !*ਸਿਰਾ)
			ਰੋਕੋ;
	}
	ਵਾਪਸ n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
ਪੂਰਨ string_ਜੋੜੋ_ਮੰਜ਼ਿਲ_idarray(ਅਚਲ ਅੱਖਰ *list, ਪੂਰਨ ary[], size_t arysz,
			size_t *ary_pos, ਪੂਰਨ (name2id)(ਅਚਲ ਅੱਖਰ *, size_t))
{
	ਅਚਲ ਅੱਖਰ *list_ਜੋੜੋ;
	ਪੂਰਨ r;

	ਜੇ (!list || !*list || !ary_pos || *ary_pos > arysz)
		ਵਾਪਸ -1;

	ਜੇ (list[0] == '+')
		list_ਜੋੜੋ = &list[1];
	ਨਹੀਂਤਾਂ {
		list_ਜੋੜੋ = list;
		*ary_pos = 0;
	}

	r = string_ਮੰਜ਼ਿਲ_idarray(list_ਜੋੜੋ, &ary[*ary_pos], arysz - *ary_pos, name2id);
	ਜੇ (r > 0)
		*ary_pos += r;
	ਵਾਪਸ r;
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
ਪੂਰਨ string_ਮੰਜ਼ਿਲ_bitarray(ਅਚਲ ਅੱਖਰ *list,
		     ਅੱਖਰ *ary,
		     ਪੂਰਨ (*name2bit)(ਅਚਲ ਅੱਖਰ *, size_t),
		     size_t allow_range)
{
	ਅਚਲ ਅੱਖਰ *begin = ਨਿਰਾ, *p;

	ਜੇ (!list || !name2bit || !ary)
		ਵਾਪਸ -EINVAL;

	ਲਈ (p = list; p && *p; p++) {
		ਅਚਲ ਅੱਖਰ *ਸਿਰਾ = ਨਿਰਾ;
		ਪੂਰਨ ਦੋਆਧਾਰੀ_ਅੰਕ, ਸੈੱਟ_ਹੇਠਲਾ = 0, ਸੈੱਟ_higher = 0;

		ਜੇ (!begin)
			begin = p;		/* begin of the level name */
		ਜੇ (*p == ',')
			ਸਿਰਾ = p;		/* terminate the name */
		ਜੇ (*(p + 1) == '\0')
			ਸਿਰਾ = p + 1;		/* end of string */
		ਜੇ (!begin || !ਸਿਰਾ)
			ਜਾਰੀ;
		ਜੇ (ਸਿਰਾ <= begin)
			ਵਾਪਸ -1;
		ਜੇ (allow_range) {
			ਜੇ (*(ਸਿਰਾ - 1) == '+') {
				ਸਿਰਾ--;
				ਸੈੱਟ_ਹੇਠਲਾ = 1;
			} ਨਹੀਂਤਾਂ ਜੇ (*begin == '+') {
				begin++;
				ਸੈੱਟ_higher = 1;
			}
		}

		ਦੋਆਧਾਰੀ_ਅੰਕ = name2bit(begin, ਸਿਰਾ - begin);
		ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ < 0)
			ਵਾਪਸ ਦੋਆਧਾਰੀ_ਅੰਕ;
		setbit(ary, ਦੋਆਧਾਰੀ_ਅੰਕ);
		ਜੇ (ਸੈੱਟ_ਹੇਠਲਾ)
			ਜਦੋਂ (--ਦੋਆਧਾਰੀ_ਅੰਕ >= 0)
				setbit(ary, ਦੋਆਧਾਰੀ_ਅੰਕ);
		ਨਹੀਂਤਾਂ ਜੇ (ਸੈੱਟ_higher)
			ਜਦੋਂ (++ਦੋਆਧਾਰੀ_ਅੰਕ < (ਪੂਰਨ) allow_range)
				setbit(ary, ਦੋਆਧਾਰੀ_ਅੰਕ);
		begin = ਨਿਰਾ;
		ਜੇ (ਸਿਰਾ && !*ਸਿਰਾ)
			ਰੋਕੋ;
	}
	ਵਾਪਸ 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
ਪੂਰਨ string_ਮੰਜ਼ਿਲ_bitmask(ਅਚਲ ਅੱਖਰ *list,
		     ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *ਨਕਾਬ,
		     ਲੰਮਾ (*name2flag)(ਅਚਲ ਅੱਖਰ *, size_t))
{
	ਅਚਲ ਅੱਖਰ *begin = ਨਿਰਾ, *p;

	ਜੇ (!list || !name2flag || !ਨਕਾਬ)
		ਵਾਪਸ -EINVAL;

	ਲਈ (p = list; p && *p; p++) {
		ਅਚਲ ਅੱਖਰ *ਸਿਰਾ = ਨਿਰਾ;
		ਲੰਮਾ flag;

		ਜੇ (!begin)
			begin = p;		/* begin of the level name */
		ਜੇ (*p == ',')
			ਸਿਰਾ = p;		/* terminate the name */
		ਜੇ (*(p + 1) == '\0')
			ਸਿਰਾ = p + 1;		/* end of string */
		ਜੇ (!begin || !ਸਿਰਾ)
			ਜਾਰੀ;
		ਜੇ (ਸਿਰਾ <= begin)
			ਵਾਪਸ -1;

		flag = name2flag(begin, ਸਿਰਾ - begin);
		ਜੇ (flag < 0)
			ਵਾਪਸ flag;	/* error */
		*ਨਕਾਬ |= flag;
		begin = ਨਿਰਾ;
		ਜੇ (ਸਿਰਾ && !*ਸਿਰਾ)
			ਰੋਕੋ;
	}
	ਵਾਪਸ 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
ਪੂਰਨ ul_parse_range(ਅਚਲ ਅੱਖਰ *str, ਪੂਰਨ *ਹੇਠਲਾ, ਪੂਰਨ *ਉੱਤਲਾ, ਪੂਰਨ def)
{
	ਅੱਖਰ *ਸਿਰਾ = ਨਿਰਾ;

	ਜੇ (!str)
		ਵਾਪਸ 0;

	*ਉੱਤਲਾ = *ਹੇਠਲਾ = def;
	errno = 0;

	ਜੇ (*str == ':') {				/* <:N> */
		str++;
		*ਉੱਤਲਾ = strtol(str, &ਸਿਰਾ, 10);
		ਜੇ (errno || !ਸਿਰਾ || *ਸਿਰਾ || ਸਿਰਾ == str)
			ਵਾਪਸ -1;
	} ਨਹੀਂਤਾਂ {
		*ਉੱਤਲਾ = *ਹੇਠਲਾ = strtol(str, &ਸਿਰਾ, 10);
		ਜੇ (errno || !ਸਿਰਾ || ਸਿਰਾ == str)
			ਵਾਪਸ -1;

		ਜੇ (*ਸਿਰਾ == ':' && !*(ਸਿਰਾ + 1))		/* <M:> */
			*ਉੱਤਲਾ = def;
		ਨਹੀਂਤਾਂ ਜੇ (*ਸਿਰਾ == '-' || *ਸਿਰਾ == ':') {	/* <M:N> <M-N> */
			str = ਸਿਰਾ + 1;
			ਸਿਰਾ = ਨਿਰਾ;
			errno = 0;
			*ਉੱਤਲਾ = strtol(str, &ਸਿਰਾ, 10);

			ਜੇ (errno || !ਸਿਰਾ || *ਸਿਰਾ || ਸਿਰਾ == str)
				ਵਾਪਸ -1;
		}
	}
	ਵਾਪਸ 0;
}

ਅਚਲ ਅੱਖਰ *ul_ਰਸਤਾ_ਅਗਲਾ_segment(ਅਚਲ ਅੱਖਰ *str, size_t *sz)
{
	ਅਚਲ ਅੱਖਰ *ਸ਼ੁਰੂ, *p;

	ਸ਼ੁਰੂ = str;
	*sz = 0;
	ਜਦੋਂ (ਸ਼ੁਰੂ && *ਸ਼ੁਰੂ == '/' && *(ਸ਼ੁਰੂ + 1) == '/')
		ਸ਼ੁਰੂ++;

	ਜੇ (!ਸ਼ੁਰੂ || !*ਸ਼ੁਰੂ)
		ਵਾਪਸ ਨਿਰਾ;

	ਲਈ (*sz = 1, p = ਸ਼ੁਰੂ + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	ਵਾਪਸ ਸ਼ੁਰੂ;
}

ਪੂਰਨ streq_paths(ਅਚਲ ਅੱਖਰ *a, ਅਚਲ ਅੱਖਰ *b)
{
	ਜਦੋਂ (a && b) {
		size_t a_sz, b_sz;
		ਅਚਲ ਅੱਖਰ *a_seg = ul_ਰਸਤਾ_ਅਗਲਾ_segment(a, &a_sz);
		ਅਚਲ ਅੱਖਰ *b_seg = ul_ਰਸਤਾ_ਅਗਲਾ_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		ਜੇ (a_sz + b_sz == 0)
			ਵਾਪਸ 1;

		/* ignore trailing slash */
		ਜੇ (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			ਵਾਪਸ 1;

		ਜੇ (!a_seg || !b_seg)
			ਰੋਕੋ;
		ਜੇ (a_sz != b_sz || ਲੜੀ_ਤੁਲਨਾ(a_seg, b_seg, a_sz) != 0)
			ਰੋਕੋ;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	ਵਾਪਸ 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
ਪੂਰਨ ul_ਰਸਤਾ_is_within(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਅਚਲ ਅੱਖਰ *dir)
{
	ਜੇ (!ਰਸਤਾ || !dir || !*ਰਸਤਾ || !*dir)
		ਵਾਪਸ 0;

	ਜਦੋਂ (ਰਸਤਾ && dir) {
		size_t p_sz, d_sz;
		ਅਚਲ ਅੱਖਰ *p_seg = ul_ਰਸਤਾ_ਅਗਲਾ_segment(ਰਸਤਾ, &p_sz);
		ਅਚਲ ਅੱਖਰ *d_seg = ul_ਰਸਤਾ_ਅਗਲਾ_segment(dir, &d_sz);

		/* all @dir segments matched */
		ਜੇ (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			ਵਾਪਸ 1;

		ਜੇ (!p_seg || !d_seg)
			ਰੋਕੋ;
		ਜੇ (p_sz != d_sz || ਲੜੀ_ਤੁਲਨਾ(p_seg, d_seg, p_sz) != 0)
			ਰੋਕੋ;

		ਰਸਤਾ = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	ਵਾਪਸ 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
ਅੱਖਰ *ul_strnconcat(ਅਚਲ ਅੱਖਰ *s, ਅਚਲ ਅੱਖਰ *suffix, size_t b)
{
	size_t a;
	ਅੱਖਰ *r;

	ਜੇ (!s && !suffix)
		ਵਾਪਸ strdup("");
	ਜੇ (!s)
		ਵਾਪਸ strndup(suffix, b);
	ਜੇ (!suffix)
		ਵਾਪਸ strdup(s);

	ਪ੍ਰਤਿਜ੍ਞਾ(s);
	ਪ੍ਰਤਿਜ੍ਞਾ(suffix);

	a = ਲੜੀ_ਲੰਬਾਈ(s);
	ਜੇ (b > ((size_t) -1) - a)
		ਵਾਪਸ ਨਿਰਾ;

	r = ਸਮ੍ਰਿਤੀ_ਦਿਓ(a + b + 1);
	ਜੇ (!r)
		ਵਾਪਸ ਨਿਰਾ;

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(r, s, a);
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(r + a, suffix, b);
	r[a+b] = 0;

	ਵਾਪਸ r;
}

/* concatenate two strings to a new string */
ਅੱਖਰ *ul_strconcat(ਅਚਲ ਅੱਖਰ *s, ਅਚਲ ਅੱਖਰ *suffix)
{
	ਵਾਪਸ ul_strnconcat(s, suffix, suffix ? ਲੜੀ_ਲੰਬਾਈ(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
ਅੱਖਰ *ul_strfconcat(ਅਚਲ ਅੱਖਰ *s, ਅਚਲ ਅੱਖਰ *format, ...)
{
	va_list ap;
	ਅੱਖਰ *val, *res;
	ਪੂਰਨ sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	ਜੇ (sz < 0)
		ਵਾਪਸ ਨਿਰਾ;

	res = ul_strnconcat(s, val, sz);
	ਮੁਕਤ(val);
	ਵਾਪਸ res;
}

ਪੂਰਨ ul_strappend(ਅੱਖਰ **a, ਅਚਲ ਅੱਖਰ *b)
{
	size_t al, bl;
	ਅੱਖਰ *tmp;

	ਜੇ (!a)
		ਵਾਪਸ -EINVAL;
	ਜੇ (!b || !*b)
		ਵਾਪਸ 0;
	ਜੇ (!*a) {
		*a = strdup(b);
		ਵਾਪਸ !*a ? -ENOMEM : 0;
	}

	al = ਲੜੀ_ਲੰਬਾਈ(*a);
	bl = ਲੜੀ_ਲੰਬਾਈ(b);

	tmp = realloc(*a, al + bl + 1);
	ਜੇ (!tmp)
		ਵਾਪਸ -ENOMEM;
	*a = tmp;
	ਸਮ੍ਰਿਤੀ_ਨਕਲ((*a) + al, b, bl + 1);
	ਵਾਪਸ 0;
}

/* the hybrid version of strfconcat and strappend. */
ਪੂਰਨ strfappend(ਅੱਖਰ **a, ਅਚਲ ਅੱਖਰ *format, ...)
{
	va_list ap;
	ਪੂਰਨ res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	ਵਾਪਸ res;
}

ਬਾਹਰੀ ਪੂਰਨ ul_strvfappend(ਅੱਖਰ **a, ਅਚਲ ਅੱਖਰ *format, va_list ap)
{
	ਅੱਖਰ *val;
	ਪੂਰਨ sz;
	ਪੂਰਨ res;

	sz = vasprintf(&val, format, ap);
	ਜੇ (sz < 0)
		ਵਾਪਸ -errno;

	res = ul_strappend(a, val);
	ਮੁਕਤ(val);
	ਵਾਪਸ res;
}

ਸਥਿਰ size_t strcspn_escaped(ਅਚਲ ਅੱਖਰ *s, ਅਚਲ ਅੱਖਰ *reject)
{
	ਪੂਰਨ escaped = 0;
	ਪੂਰਨ n;

	ਲਈ (n=0; s[n]; n++) {
		ਜੇ (escaped)
			escaped = 0;
		ਨਹੀਂਤਾਂ ਜੇ (s[n] == '\\')
			escaped = 1;
		ਨਹੀਂਤਾਂ ਜੇ (strchr(reject, s[n]))
			ਰੋਕੋ;
	}

	/* if s ends in \, return index of previous char */
	ਵਾਪਸ n - escaped;
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
ਅੱਖਰ *ul_strchr_escaped(ਅਚਲ ਅੱਖਰ *s, ਪੂਰਨ c)
{
	ਅੱਖਰ *p;
	ਪੂਰਨ esc = 0;

	ਲਈ (p = (ਅੱਖਰ *) s; p && *p; p++) {
		ਜੇ (!esc && *p == '\\') {
			esc = 1;
			ਜਾਰੀ;
		}
		ਜੇ (*p == c && (!esc || c == '\\'))
			ਵਾਪਸ p;
		esc = 0;
	}

	ਵਾਪਸ ਨਿਰਾ;
}

/* Split a string into words. */
ਅਚਲ ਅੱਖਰ *ul_ਵੰਡੋ(ਅਚਲ ਅੱਖਰ **state, size_t *l, ਅਚਲ ਅੱਖਰ *separator, ਪੂਰਨ quoted)
{
	ਅਚਲ ਅੱਖਰ *current;

	current = *state;

	ਜੇ (!*current) {
		ਪ੍ਰਤਿਜ੍ਞਾ(**state == '\0');
		ਵਾਪਸ ਨਿਰਾ;
	}

	current += strspn(current, separator);
	ਜੇ (!*current) {
		*state = current;
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਜੇ (quoted && strchr("\'\"", *current)) {
		ਅੱਖਰ quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		ਜੇ (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			ਵਾਪਸ ਨਿਰਾ;
		}
		*state = current++ + *l + 2;
	} ਨਹੀਂਤਾਂ ਜੇ (quoted) {
		*l = strcspn_escaped(current, separator);
		ਜੇ (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			ਵਾਪਸ ਨਿਰਾ;
		}
		*state = current + *l;
	} ਨਹੀਂਤਾਂ {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	ਵਾਪਸ current;
}

/* Rewind file pointer forward to new line.  */
ਪੂਰਨ skip_fline(FILE *fp)
{
	ਪੂਰਨ ch;

	ਕਰੋ {
		ਜੇ ((ch = fgetc(fp)) == EOF)
			ਵਾਪਸ 1;
		ਜੇ (ch == '\n')
			ਵਾਪਸ 0;
	} ਜਦੋਂ (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
ਪੂਰਨ ul_stralnumcmp(ਅਚਲ ਅੱਖਰ *p1, ਅਚਲ ਅੱਖਰ *p2)
{
	ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *s1 = (ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *) p1;
	ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *s2 = (ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *) p2;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ c1, c2;

	ਕਰੋ {
		ਕਰੋ {
			c1 = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *s1++;
		} ਜਦੋਂ (c1 != '\0' && !ਅੱਖਰ_ਜਾਂ_ਅੰਕ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) c1));

		ਕਰੋ {
			c2 = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *s2++;
		} ਜਦੋਂ (c2 != '\0' && !ਅੱਖਰ_ਜਾਂ_ਅੰਕ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) c2));

		ਜੇ (c1 != '\0')
			c1 = tolower(c1);
		ਜੇ (c2 != '\0')
			c2 = tolower(c2);
		ਜੇ (c1 == '\0')
			ਵਾਪਸ c1 - c2;
	} ਜਦੋਂ (c1 == c2);

	ਵਾਪਸ c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
ਪੂਰਨ ul_optstr_ਅਗਲਾ(ਅੱਖਰ **optstr, ਅੱਖਰ **name, size_t *namesz,
		   ਅੱਖਰ **ਮੁੱਲ, size_t *valsz)
{
	ਪੂਰਨ open_quote = 0;
	ਅੱਖਰ *ਸ਼ੁਰੂ = ਨਿਰਾ, *stop = ਨਿਰਾ, *p, *sep = ਨਿਰਾ;
	ਅੱਖਰ *optstr0;

	ਪ੍ਰਤਿਜ੍ਞਾ(optstr);
	ਪ੍ਰਤਿਜ੍ਞਾ(*optstr);

	optstr0 = *optstr;

	ਜੇ (name)
		*name = ਨਿਰਾ;
	ਜੇ (namesz)
		*namesz = 0;
	ਜੇ (ਮੁੱਲ)
		*ਮੁੱਲ = ਨਿਰਾ;
	ਜੇ (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	ਜਦੋਂ (optstr0 && *optstr0 == ',')
		optstr0++;

	ਲਈ (p = optstr0; p && *p; p++) {
		ਜੇ (!ਸ਼ੁਰੂ && *p == '=')
			ਵਾਪਸ -EINVAL;
		ਜੇ (!ਸ਼ੁਰੂ)
			ਸ਼ੁਰੂ = p;		/* beginning of the option item */
		ਜੇ (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		ਜੇ (open_quote)
			ਜਾਰੀ;		/* still in quoted block */
		ਜੇ (!sep && p > ਸ਼ੁਰੂ && *p == '=')
			sep = p;		/* name and value separator */
		ਜੇ (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		ਨਹੀਂਤਾਂ ਜੇ (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		ਜੇ (!ਸ਼ੁਰੂ || !stop)
			ਜਾਰੀ;
		ਜੇ (stop <= ਸ਼ੁਰੂ)
			ਵਾਪਸ -EINVAL;

		ਜੇ (name)
			*name = ਸ਼ੁਰੂ;
		ਜੇ (namesz)
			*namesz = sep ? sep - ਸ਼ੁਰੂ : stop - ਸ਼ੁਰੂ;
		*optstr = *stop ? stop + 1 : stop;

		ਜੇ (sep) {
			ਜੇ (ਮੁੱਲ)
				*ਮੁੱਲ = sep + 1;
			ਜੇ (valsz)
				*valsz = stop - sep - 1;
		}
		ਵਾਪਸ 0;
	}

	ਵਾਪਸ 1;				/* end of optstr */
}

ਪੂਰਨ ul_optstr_is_valid(ਅਚਲ ਅੱਖਰ *optstr)
{
	ਪੂਰਨ rc;
	ਅੱਖਰ *p = (ਅੱਖਰ *) optstr;

	ਜਦੋਂ ((rc = ul_optstr_ਅਗਲਾ(&p, ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ)) == 0);
	ਵਾਪਸ rc < 0 ? 0 : 1;
}

ਅੱਖਰ *ul_optstr_get_ਮੁੱਲ(ਅਚਲ ਅੱਖਰ *optstr, ਅਚਲ ਅੱਖਰ *ਕੁੰਜੀ)
{
	size_t sz, namesz = 0, valsz = 0;
	ਅੱਖਰ *name = ਨਿਰਾ, *ਮੁੱਲ = ਨਿਰਾ;
	ਅੱਖਰ *p = (ਅੱਖਰ *) optstr;

	ਜੇ (!optstr || !ਕੁੰਜੀ || !*ਕੁੰਜੀ)
		ਵਾਪਸ ਨਿਰਾ;

	sz = ਲੜੀ_ਲੰਬਾਈ(ਕੁੰਜੀ);
	ਜਦੋਂ (ul_optstr_ਅਗਲਾ(&p, &name, &namesz, &ਮੁੱਲ, &valsz) == 0) {
		ਜੇ (namesz != sz || !valsz)
			ਜਾਰੀ;
		ਜੇ (ਲੜੀ_ਤੁਲਨਾ(name, ਕੁੰਜੀ, namesz) == 0)
			ਵਾਪਸ strndup(ਮੁੱਲ, valsz);
	}
	ਵਾਪਸ ਨਿਰਾ;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(ਅੱਖਰ *str)
{
	size_t len;
	ਅੱਖਰ q;

	ਜੇ (!str || !*str)
		ਵਾਪਸ 0;

	len = ਲੜੀ_ਲੰਬਾਈ(str);
	q = *str;

	ਜੇ ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		ਵਾਪਸ len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	ਵਾਪਸ len - 2;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_STRUTILS

ਬਣਤਰ testS {
	ਅੱਖਰ *name;
	ਅੱਖਰ *ਮੁੱਲ;
};

ਸਥਿਰ ਪੂਰਨ test_strdup_ਮੰਜ਼ਿਲ_member(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਬਣਤਰ testS *xx;

	ਜੇ (argc < 3)
		ਵਾਪਸ EXIT_FAILURE;

	xx = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*xx));
	ਜੇ (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_ਮੰਜ਼ਿਲ_struct_member(xx, name, argv[1]);
	strdup_ਮੰਜ਼ਿਲ_struct_member(xx, ਮੁੱਲ, argv[2]);

	ਜੇ (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->ਮੁੱਲ, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	ਲਿਖੋ("1: '%s', 2: '%s'\n", xx->name, xx->ਮੁੱਲ);

	ਮੁਕਤ(xx->name);
	ਮੁਕਤ(xx->ਮੁੱਲ);
	ਮੁਕਤ(xx);
	ਵਾਪਸ EXIT_SUCCESS;
}

ਸਥਿਰ ਪੂਰਨ test_strutils_sizes(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	uintmax_t ਮਾਪ = 0;
	ਅੱਖਰ *hum1, *hum2, *hum3;

	ਜੇ (argc < 2)
		ਵਾਪਸ EXIT_FAILURE;

	ਜੇ (strtosize(argv[1], &ਮਾਪ))
		errx(EXIT_FAILURE, "invalid size '%s' value", argv[1]);

	hum1 = ਮਾਪ_ਮੰਜ਼ਿਲ_human_string(ਮਾਪ_SUFFIX_1_LETTER, ਮਾਪ);
	hum2 = ਮਾਪ_ਮੰਜ਼ਿਲ_human_string(ਮਾਪ_SUFFIX_3_LETTER |
				    ਮਾਪ_SUFFIX_SPACE, ਮਾਪ);
	hum3 = ਮਾਪ_ਮੰਜ਼ਿਲ_human_string(ਮਾਪ_SUFFIX_3_LETTER |
				    ਮਾਪ_SUFFIX_SPACE |
				    ਮਾਪ_DECIMAL_2_DIGITS, ਮਾਪ);

	ਲਿਖੋ("%25s : %20ju : %8s : %12s : %13s\n", argv[1], ਮਾਪ, hum1, hum2, hum3);
	ਮੁਕਤ(hum1);
	ਮੁਕਤ(hum2);
	ਮੁਕਤ(hum3);

	ਵਾਪਸ EXIT_SUCCESS;
}

ਸਥਿਰ ਪੂਰਨ test_strutils_cmp_paths(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਪੂਰਨ rc = streq_paths(argv[1], argv[2]);

	ਜੇ (argc < 3)
		ਵਾਪਸ EXIT_FAILURE;

	ਲਿਖੋ("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	ਵਾਪਸ EXIT_SUCCESS;
}

ਸਥਿਰ ਪੂਰਨ test_strutils_ਰਸਤਾ_is_within(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਪੂਰਨ rc;

	ਜੇ (argc < 3)
		ਵਾਪਸ EXIT_FAILURE;

	rc = ul_ਰਸਤਾ_is_within(argv[1], argv[2]);

	ਲਿਖੋ("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	ਵਾਪਸ EXIT_SUCCESS;
}

ਸਥਿਰ ਪੂਰਨ test_strutils_normalize(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *src, *dst, *org;
	size_t sz, len;

	ਜੇ (argc < 2)
		ਵਾਪਸ EXIT_FAILURE;

	org = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *) strdup(argv[1]);
	src = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *) strdup((ਅੱਖਰ *) org);
	len = ਲੜੀ_ਲੰਬਾਈ((ਅੱਖਰ *) src);
	dst = ਸਮ੍ਰਿਤੀ_ਦਿਓ(len + 1);

	ਜੇ (!org || !src || !dst)
		ਜਾਓ ਪੂਰਾ;

	/* two buffers */
	sz = __normalize_whitespace(src, len, dst, len + 1);
	ਲਿਖੋ("1: '%s' --> '%s' [sz=%zu]\n", src, dst, sz);

	/* one buffer */
	sz = normalize_whitespace(src);
	ਲਿਖੋ("2: '%s' --> '%s' [sz=%zu]\n", org, src, sz);

ਪੂਰਾ:
	ਮੁਕਤ(src);
	ਮੁਕਤ(dst);
	ਮੁਕਤ(org);

	ਵਾਪਸ EXIT_SUCCESS;
}

ਸਥਿਰ ਪੂਰਨ test_strutils_cstrcasecmp(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਅੱਖਰ *a, *b;

	ਜੇ (argc < 3)
		ਵਾਪਸ EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	ਜੇ (!a || !b)
		ਵਾਪਸ EXIT_FAILURE;

	ਲਿਖੋ("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	ਲਿਖੋ("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	ਲਿਖੋ("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, ਲੜੀ_ਲੰਬਾਈ(a)));

	ਵਾਪਸ EXIT_SUCCESS;
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਜੇ (argc == 3 && strcmp(argv[1], "--size") == 0) {
		ਵਾਪਸ test_strutils_sizes(argc - 1, argv + 1);

	} ਨਹੀਂਤਾਂ ਜੇ (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		ਲਿਖੋ("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						ਨਿਰਾ));
		ਵਾਪਸ EXIT_SUCCESS;
	} ਨਹੀਂਤਾਂ ਜੇ (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		ਜੇ (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		ਲਿਖੋ("'%s'-->%s\n", argv[2], val ? "true" : "false");
		ਵਾਪਸ EXIT_SUCCESS;

	} ਨਹੀਂਤਾਂ ਜੇ (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		ਵਾਪਸ test_strutils_cmp_paths(argc - 1, argv + 1);

	} ਨਹੀਂਤਾਂ ਜੇ (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		ਵਾਪਸ test_strutils_ਰਸਤਾ_is_within(argc - 1, argv + 1);

	} ਨਹੀਂਤਾਂ ਜੇ (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		ਵਾਪਸ test_strdup_ਮੰਜ਼ਿਲ_member(argc - 1, argv + 1);

	} ਨਹੀਂਤਾਂ ਜੇ  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		ਲਿਖੋ("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		ਵਾਪਸ EXIT_SUCCESS;

	} ਨਹੀਂਤਾਂ ਜੇ (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		ਵਾਪਸ test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} ਨਹੀਂਤਾਂ ਜੇ (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		ਵਾਪਸ test_strutils_normalize(argc - 1, argv + 1);

	} ਨਹੀਂਤਾਂ ਜੇ (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		ਲਿਖੋ("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		ਵਾਪਸ EXIT_SUCCESS;
	} ਨਹੀਂਤਾਂ ਜੇ (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		ਲਿਖੋ("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		ਵਾਪਸ EXIT_SUCCESS;
	} ਨਹੀਂਤਾਂ ਜੇ (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		ਲਿਖੋ("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		ਵਾਪਸ EXIT_SUCCESS;
	} ਨਹੀਂਤਾਂ ਜੇ (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		ਲਿਖੋ("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		ਵਾਪਸ EXIT_SUCCESS;
	} ਨਹੀਂਤਾਂ ਜੇ (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		ਲਿਖੋ("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		ਵਾਪਸ EXIT_SUCCESS;
	} ਨਹੀਂਤਾਂ ਜੇ (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		ਲਿਖੋ("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		ਵਾਪਸ EXIT_SUCCESS;

	} ਨਹੀਂਤਾਂ ਜੇ (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		ਲਿਖੋ("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		ਵਾਪਸ EXIT_SUCCESS;

	} ਨਹੀਂਤਾਂ ਜੇ (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		ਅੱਖਰ *buf = "abc\0Y\0\0xyz\0X";
		ਅੱਖਰ *ਸਿਰਾ = buf + 12;
		ਅੱਖਰ *p = buf;

		ਕਰੋ {
			ਲਿਖੋ("str: '%s'\n", p);
		} ਜਦੋਂ ((p = ul_ਅਗਲਾ_string(p, ਸਿਰਾ)));

		ਵਾਪਸ EXIT_SUCCESS;

	} ਨਹੀਂਤਾਂ ਜੇ (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		ਅੱਖਰ *name = ਨਿਰਾ, *val = ਨਿਰਾ;
		ਅੱਖਰ *p = argv[2];
		ਪੂਰਨ rc;

		ਜੇ (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		ਜਦੋਂ ((rc = ul_optstr_ਅਗਲਾ(&p, &name, &namesz, &val, &valsz)) == 0) {
			ਲਿਖੋ("'%.*s' : '%.*s'\n", (ਪੂਰਨ) namesz, name,
						   (ਪੂਰਨ) valsz, val);
		}
		ਜੇ (rc == 1)
			ਵਾਪਸ EXIT_SUCCESS;
	} ਨਹੀਂਤਾਂ ਜੇ (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		ਅੱਖਰ *unquoted = strdup(argv[2]);

		ਜੇ (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		ਲਿਖੋ("%s-->%s\n", argv[2], unquoted);
		ਮੁਕਤ(unquoted);
		ਵਾਪਸ EXIT_SUCCESS;

	} ਨਹੀਂਤਾਂ {
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
		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
	}

	ਵਾਪਸ EXIT_FAILURE;
}
#ਜੇ_ਅੰਤ /* TEST_PROGRAM_STRUTILS */
