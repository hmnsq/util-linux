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

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ STRTOXX_EXIT_ಸಂಕೇತ = EXIT_FAILURE;

ಶೂನ್ಯ strutils_ಗಣ_exitcode(ಪೂರ್ಣಾಂಕ ex)
{
	STRTOXX_EXIT_ಸಂಕೇತ = ex;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ do_scale_by_power (uintmax_t *x, ಪೂರ್ಣಾಂಕ ತಳ, ಪೂರ್ಣಾಂಕ power)
{
	ಆಗ (power--) {
		ಆದರೆ (UINTMAX_MAX / ತಳ < *x)
			ಹಿಂತಿರುಗಿ -ERANGE;
		*x *= ತಳ;
	}
	ಹಿಂತಿರುಗಿ 0;
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
ಪೂರ್ಣಾಂಕ ul_parse_ಗಾತ್ರ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, uintmax_t *res, ಪೂರ್ಣಾಂಕ *power)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
	ಅಕ್ಷರ *ಕೊನೆ;
	uintmax_t x, frac = 0;
	ಪೂರ್ಣಾಂಕ ತಳ = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *suf  = "KMGTPEZY";
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *suf2 = "kmgtpezy";
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sp;

	*res = 0;

	ಆದರೆ (!str || !*str) {
		rc = -EINVAL;
		ಹೋಗು err;
	}

	/* Only positive numbers are acceptable
	 *
	 * Note that this check is not perfect, it would be better to
	 * use lconv->negative_sign. But coreutils use the same solution,
	 * so it's probably good enough...
	 */
	p = str;
	ಆಗ (ಖಾಲಿಯೇ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *p))
		p++;
	ಆದರೆ (*p == '-') {
		rc = -EINVAL;
		ಹೋಗು err;
	}

	errno = 0, ಕೊನೆ = ಖಾಲಿ;
	x = strtoumax(str, &ಕೊನೆ, 0);

	ಆದರೆ (ಕೊನೆ == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		ಹೋಗು err;
	}
	ಆದರೆ (!ಕೊನೆ || !*ಕೊನೆ)
		ಹೋಗು ಮುಗಿದ;			/* without suffix */
	p = ಕೊನೆ;

	/*
	 * Check size suffixes
	 */
check_suffix:
	ಆದರೆ (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		ತಳ = 1024;			/* XiB, 2^N */
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		ತಳ = 1000;			/* XB, 10^N */
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*(p + 1)) {
		ರಚನೆ lconv ಸ್ಥಿರಾಂಕ *l = localeconv();
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dp = l ? l->decimal_ಬಿಂದು : ಖಾಲಿ;
		size_t dpsz = dp ? ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(dp) : 0;

		ಆದರೆ (frac == 0 && *p && dp && ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(dp, p, dpsz) == 0) {
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fstr = p + dpsz;

			ಗಾಗಿ (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			ಆದರೆ (ಅಂಕಿಯೇ(*fstr)) {
				errno = 0, ಕೊನೆ = ಖಾಲಿ;
				frac = strtoumax(fstr, &ಕೊನೆ, 0);
				ಆದರೆ (ಕೊನೆ == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					ಹೋಗು err;
				}
			} ಇಲ್ಲದಿದ್ದರೆ
				ಕೊನೆ = (ಅಕ್ಷರ *) p;

			ಆದರೆ (!ಕೊನೆ || !*ಕೊನೆ) {
				rc = -EINVAL;
				ಹೋಗು err;	/* without suffix, but with fractions */
			}
			p = ಕೊನೆ;
			ಹೋಗು check_suffix;
		}
		rc = -EINVAL;
		ಹೋಗು err;			/* unexpected suffix */
	}

	sp = strchr(suf, *p);
	ಆದರೆ (sp)
		pwr = (sp - suf) + 1;
	ಇಲ್ಲದಿದ್ದರೆ {
		sp = strchr(suf2, *p);
		ಆದರೆ (sp)
			pwr = (sp - suf2) + 1;
		ಇಲ್ಲದಿದ್ದರೆ {
			rc = -EINVAL;
			ಹೋಗು err;
		}
	}

	ಆದರೆ (power)
		*power = pwr;

	rc = do_scale_by_power(&x, ತಳ, pwr);
	ಆದರೆ (rc)
		ಹೋಗು err;

	ಆದರೆ (frac && pwr) {
		ಪೂರ್ಣಾಂಕ i;
		uintmax_t frac_ತಳ = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_ತಳ, ತಳ, pwr);
		ಆದರೆ (rc)
			ಹೋಗು err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		ಮಾಡು {
			frac_val = ((frac % 10) * frac_ತಳ + frac_val) / 10;
			frac /= 10;
		} ಆಗ (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		ಗಾಗಿ (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		ಆದರೆ (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			ಹೋಗು err;
		}
		x += frac_val;
	}
ಮುಗಿದ:
	*res = x;
err:
	ಆದರೆ (rc < 0)
		errno = -rc;
	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ strtosize(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, uintmax_t *res)
{
	ಹಿಂತಿರುಗಿ ul_parse_ಗಾತ್ರ(str, res, ಖಾಲಿ);
}

ಪೂರ್ಣಾಂಕ isdigit_strend(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **ಕೊನೆ)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;

	ಗಾಗಿ (p = str; p && *p && ಅಂಕಿಯೇ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *p); p++);

	ಆದರೆ (ಕೊನೆ)
		*ಕೊನೆ = p;
	ಹಿಂತಿರುಗಿ p && p > str && !*p;
}

ಪೂರ್ಣಾಂಕ isxdigit_strend(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **ಕೊನೆ)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;

	ಗಾಗಿ (p = str; p && *p && isxdigit((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *p); p++);

	ಆದರೆ (ಕೊನೆ)
		*ಕೊನೆ = p;

	ಹಿಂತಿರುಗಿ p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
ಪೂರ್ಣಾಂಕ ul_strtobool(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, bool *ಫಲಿತಾಂಶ)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	ಆದರೆ (!str || !ಫಲಿತಾಂಶ)
		ಹಿಂತಿರುಗಿ -(errno = EINVAL);

	errno = 0;

	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(bool_true); i++) {
		ಆದರೆ (c_strcasecmp(str, bool_true[i]) == 0) {
			*ಫಲಿತಾಂಶ = true;
			ಹಿಂತಿರುಗಿ 0;
		}
	}

	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(bool_false); i++) {
		ಆದರೆ (c_strcasecmp(str, bool_false[i]) == 0) {
			*ಫಲಿತಾಂಶ = false;
			ಹಿಂತಿರುಗಿ 0;
		}
	}

	ಹಿಂತಿರುಗಿ -(errno = EINVAL);
}

ಪೂರ್ಣಾಂಕ ul_parse_switch(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg, ...)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *a, *b;
	va_list ap;

	va_start(ap, arg);
	ಮಾಡು {
		a = va_arg(ap, ಅಕ್ಷರ *);
		ಆದರೆ (!a)
			ನಿಲ್ಲಿಸು;
		b = va_arg(ap, ಅಕ್ಷರ *);
		ಆದರೆ (!b)
			ನಿಲ್ಲಿಸು;

		ಆದರೆ (strcmp(arg, a) == 0) {
			va_end(ap);
			ಹಿಂತಿರುಗಿ 1;
		}

		ಆದರೆ (strcmp(arg, b) == 0) {
			va_end(ap);
			ಹಿಂತಿರುಗಿ 0;
		}
	} ಆಗ (1);
	va_end(ap);

	errx(STRTOXX_EXIT_ಸಂಕೇತ, _("unsupported argument: %s"), arg);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ HAVE_MEMPCPY
ಶೂನ್ಯ *mempcpy(ಶೂನ್ಯ *ಸೀಮಿತ dest, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಸೀಮಿತ src, size_t n)
{
	ಹಿಂತಿರುಗಿ ((ಅಕ್ಷರ *)ಸ್ಮರಣೆ_ನಕಲು(dest, src, n)) + n;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ HAVE_STRNLEN
size_t strnlen(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, size_t maxlen)
{
	size_t i;

	ಗಾಗಿ (i = 0; i < maxlen; i++) {
		ಆದರೆ (s[i] == '\0')
			ಹಿಂತಿರುಗಿ i;
	}
	ಹಿಂತಿರುಗಿ maxlen;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ HAVE_STRNCHR
ಅಕ್ಷರ *strnchr(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, size_t maxlen, ಪೂರ್ಣಾಂಕ c)
{
	ಗಾಗಿ (; maxlen-- && *s != '\0'; ++s)
		ಆದರೆ (*s == (ಅಕ್ಷರ)c)
			ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ *)s;
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ HAVE_STRNDUP
ಅಕ್ಷರ *strndup(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, size_t n)
{
	size_t len = strnlen(s, n);
	ಅಕ್ಷರ *new = ಸ್ಮರಣೆ_ಕೊಡು((len + 1) * ಗಾತ್ರ(ಅಕ್ಷರ));
	ಆದರೆ (!new)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	new[len] = '\0';
	ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ *) ಸ್ಮರಣೆ_ನಕಲು(new, s, len);
}
#ಆದರೆ_ಅಂತ್ಯ

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
ಪೂರ್ಣಾಂಕ ul_strtos64(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, int64_t *ಅಂಕೆ, ಪೂರ್ಣಾಂಕ ತಳ)
{
	ಅಕ್ಷರ *ಕೊನೆ = ಖಾಲಿ;

	ಆದರೆ (str == ಖಾಲಿ || *str == '\0')
		ಹಿಂತಿರುಗಿ -(errno = EINVAL);

	errno = 0;
	*ಅಂಕೆ = (int64_t) strtoimax(str, &ಕೊನೆ, ತಳ);

	ಆದರೆ (errno != 0)
		ಹಿಂತಿರುಗಿ -errno;
	ಆದರೆ (str == ಕೊನೆ || (ಕೊನೆ && *ಕೊನೆ))
		ಹಿಂತಿರುಗಿ -(errno = EINVAL);
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ ul_strtou64(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, uint64_t *ಅಂಕೆ, ಪೂರ್ಣಾಂಕ ತಳ)
{
	ಅಕ್ಷರ *ಕೊನೆ = ಖಾಲಿ;
	int64_t tmp;

	ಆದರೆ (str == ಖಾಲಿ || *str == '\0')
		ಹಿಂತಿರುಗಿ -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &ಕೊನೆ, ತಳ);
	ಆದರೆ (tmp < 0)
		errno = ERANGE;
	ಇಲ್ಲದಿದ್ದರೆ {
		errno = 0;
		*ಅಂಕೆ = strtoumax(str, &ಕೊನೆ, ತಳ);
	}

	ಆದರೆ (errno != 0)
		ಹಿಂತಿರುಗಿ -errno;
	ಆದರೆ (str == ಕೊನೆ || (ಕೊನೆ && *ಕೊನೆ))
		ಹಿಂತಿರುಗಿ -(errno = EINVAL);
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ ul_strtos32(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, int32_t *ಅಂಕೆ, ಪೂರ್ಣಾಂಕ ತಳ)
{
	int64_t tmp;
	ಪೂರ್ಣಾಂಕ rc;

	rc = ul_strtos64(str, &tmp, ತಳ);
	ಆದರೆ (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	ಆದರೆ (rc == 0)
		*ಅಂಕೆ = (int32_t) tmp;
	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ ul_strtou32(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, uint32_t *ಅಂಕೆ, ಪೂರ್ಣಾಂಕ ತಳ)
{
	uint64_t tmp;
	ಪೂರ್ಣಾಂಕ rc;

	rc = ul_strtou64(str, &tmp, ತಳ);
	ಆದರೆ (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	ಆದರೆ (rc == 0)
		*ಅಂಕೆ = (uint32_t) tmp;
	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ ul_strtou16(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, uint16_t *ಅಂಕೆ, ಪೂರ್ಣಾಂಕ ತಳ)
{
	uint64_t tmp;
	ಪೂರ್ಣಾಂಕ rc;

	rc = ul_strtou64(str, &tmp, ತಳ);
	ಆದರೆ (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	ಆದರೆ (rc == 0)
		*ಅಂಕೆ = (uint16_t) tmp;
	ಹಿಂತಿರುಗಿ rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಪೂರ್ಣಾಂಕ ತಳ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *errmesg,
			     int64_t ಕೆಳಗಿನ, int64_t ಮೇಲೆ)
{
	int64_t ಅಂಕೆ = 0;
	ಪೂರ್ಣಾಂಕ rc;

	rc = ul_strtos64(str, &ಅಂಕೆ, ತಳ);
	ಆದರೆ (rc == 0 && ((ಕೆಳಗಿನ && ಅಂಕೆ < ಕೆಳಗಿನ) || (ಮೇಲೆ && ಅಂಕೆ > ಮೇಲೆ)))
		rc = -(errno = ERANGE);

	ಆದರೆ (rc) {
		ಆದರೆ (errno == ERANGE)
			err(STRTOXX_EXIT_ಸಂಕೇತ, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_ಸಂಕೇತ, "%s: '%s'", errmesg, str);
	}
	ಹಿಂತಿರುಗಿ ಅಂಕೆ;
}

uint64_t str2unum_or_err(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಪೂರ್ಣಾಂಕ ತಳ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *errmesg, uint64_t ಮೇಲೆ)
{
	uint64_t ಅಂಕೆ = 0;
	ಪೂರ್ಣಾಂಕ rc;

	rc = ul_strtou64(str, &ಅಂಕೆ, ತಳ);
	ಆದರೆ (rc == 0 && (ಮೇಲೆ && ಅಂಕೆ > ಮೇಲೆ))
		rc = -(errno = ERANGE);

	ಆದರೆ (rc) {
		ಆದರೆ (errno == ERANGE)
			err(STRTOXX_EXIT_ಸಂಕೇತ, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_ಸಂಕೇತ, "%s: '%s'", errmesg, str);
	}
	ಹಿಂತಿರುಗಿ ಅಂಕೆ;
}

ದ್ವಿಗುಣ strtod_or_err(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *errmesg)
{
	ದ್ವಿಗುಣ ಅಂಕೆ;
	ಅಕ್ಷರ *ಕೊನೆ = ಖಾಲಿ;

	errno = 0;
	ಆದರೆ (str == ಖಾಲಿ || *str == '\0')
		ಹೋಗು err;
	ಅಂಕೆ = strtod(str, &ಕೊನೆ);

	ಆದರೆ (errno || str == ಕೊನೆ || (ಕೊನೆ && *ಕೊನೆ))
		ಹೋಗು err;

	ಹಿಂತಿರುಗಿ ಅಂಕೆ;
err:
	ಆದರೆ (errno == ERANGE)
		err(STRTOXX_EXIT_ಸಂಕೇತ, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_ಸಂಕೇತ, "%s: '%s'", errmesg, str);
}

ಪೂರ್ಣಾಂಕ ul_strtold(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ದೀರ್ಘ ದ್ವಿಗುಣ *ಅಂಕೆ)
{
	ಅಕ್ಷರ *ಕೊನೆ = ಖಾಲಿ;

	errno = 0;
	ಆದರೆ (str == ಖಾಲಿ || *str == '\0')
		ಹಿಂತಿರುಗಿ -(errno = EINVAL);
	*ಅಂಕೆ = strtold(str, &ಕೊನೆ);

	ಆದರೆ (errno != 0)
		ಹಿಂತಿರುಗಿ -errno;
	ಆದರೆ (str == ಕೊನೆ || (ಕೊನೆ && *ಕೊನೆ))
		ಹಿಂತಿರುಗಿ -(errno = EINVAL);
	ಹಿಂತಿರುಗಿ 0;
}

ದೀರ್ಘ ದ್ವಿಗುಣ strtold_or_err(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *errmesg)
{
	ದೀರ್ಘ ದ್ವಿಗುಣ ಅಂಕೆ = 0;

	ಆದರೆ (ul_strtold(str, &ಅಂಕೆ) == 0)
		ಹಿಂತಿರುಗಿ ಅಂಕೆ;
	ಆದರೆ (errno == ERANGE)
		err(STRTOXX_EXIT_ಸಂಕೇತ, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_ಸಂಕೇತ, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *errmesg)
{
	uintmax_t ಅಂಕೆ;

	ಆದರೆ (strtosize(str, &ಅಂಕೆ) == 0)
		ಹಿಂತಿರುಗಿ ಅಂಕೆ;

	ಆದರೆ (errno)
		err(STRTOXX_EXIT_ಸಂಕೇತ, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_ಸಂಕೇತ, "%s: '%s'", errmesg, str);
}


ಶೂನ್ಯ strtotimeval_or_err(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ರಚನೆ timeval *tv, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *errmesg)
{
	ದೀರ್ಘ ದ್ವಿಗುಣ user_ಒಳಹರಿವು;

	user_ಒಳಹರಿವು = strtold_or_err(str, errmesg);
	tv->tv_sec = (time_t) user_ಒಳಹರಿವು;
	tv->tv_usec = (suseconds_t)((user_ಒಳಹರಿವು - tv->tv_sec) * USEC_ಪ್ರತಿ_SEC);
}

ಶೂನ್ಯ strtotimespec_or_err(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ರಚನೆ timespec *ts, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *errmesg)
{
	ದೀರ್ಘ ದ್ವಿಗುಣ user_ಒಳಹರಿವು;

	user_ಒಳಹರಿವು = strtold_or_err(str, errmesg);
	ts->tv_sec = (time_t) user_ಒಳಹರಿವು;
	ts->tv_nsec = (ದೀರ್ಘ)((user_ಒಳಹರಿವು - ts->tv_sec) * NSEC_ಪ್ರತಿ_SEC);
}

time_t strtotime_or_err(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *errmesg)
{
	int64_t user_ಒಳಹರಿವು;

	user_ಒಳಹರಿವು = strtos64_or_err(str, errmesg);
	ಹಿಂತಿರುಗಿ (time_t) user_ಒಳಹರಿವು;
}

bool hyperlinkwanted(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ)
{
	ಆದರೆ (ಬಹುಲಕ && strcmp(ಬಹುಲಕ, "never") == 0)
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (ಬಹುಲಕ && strcmp(ಬಹುಲಕ, "always") == 0)
		ಹಿಂತಿರುಗಿ true;

	ಆದರೆ (!ಬಹುಲಕ || strcmp(ಬಹುಲಕ, "auto") == 0)
		ಹಿಂತಿರುಗಿ isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), ಬಹುಲಕ);
}

bool annotationwanted(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ)
{
	ಆದರೆ (ಬಹುಲಕ && strcmp(ಬಹುಲಕ, "never") == 0)
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (ಬಹುಲಕ && strcmp(ಬಹುಲಕ, "always") == 0)
		ಹಿಂತಿರುಗಿ true;

	ಆದರೆ (!ಬಹುಲಕ || strcmp(ಬಹುಲಕ, "auto") == 0)
		ಹಿಂತಿರುಗಿ isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), ಬಹುಲಕ);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
ಅಕ್ಷರ *xstrmode(mode_t ಬಹುಲಕ, ಅಕ್ಷರ *str)
{
	ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ i = 0;

	ಆದರೆ (S_ISDIR(ಬಹುಲಕ))
		str[i++] = 'd';
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (S_ISLNK(ಬಹುಲಕ))
		str[i++] = 'l';
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (S_ISCHR(ಬಹುಲಕ))
		str[i++] = 'c';
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (S_ISBLK(ಬಹುಲಕ))
		str[i++] = 'b';
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (S_ISSOCK(ಬಹುಲಕ))
		str[i++] = 's';
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (S_ISFIFO(ಬಹುಲಕ))
		str[i++] = 'p';
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (S_ISREG(ಬಹುಲಕ))
		str[i++] = '-';

	str[i++] = ಬಹುಲಕ & S_IRUSR ? 'r' : '-';
	str[i++] = ಬಹುಲಕ & S_IWUSR ? 'w' : '-';
	str[i++] = (ಬಹುಲಕ & S_ISUID
		? (ಬಹುಲಕ & S_IXUSR ? 's' : 'S')
		: (ಬಹುಲಕ & S_IXUSR ? 'x' : '-'));
	str[i++] = ಬಹುಲಕ & S_IRGRP ? 'r' : '-';
	str[i++] = ಬಹುಲಕ & S_IWGRP ? 'w' : '-';
	str[i++] = (ಬಹುಲಕ & S_ISGID
		? (ಬಹುಲಕ & S_IXGRP ? 's' : 'S')
		: (ಬಹುಲಕ & S_IXGRP ? 'x' : '-'));
	str[i++] = ಬಹುಲಕ & S_IROTH ? 'r' : '-';
	str[i++] = ಬಹುಲಕ & S_IWOTH ? 'w' : '-';
	str[i++] = (ಬಹುಲಕ & S_ISVTX
		? (ಬಹುಲಕ & S_IXOTH ? 't' : 'T')
		: (ಬಹುಲಕ & S_IXOTH ? 'x' : '-'));
	str[i] = '\0';

	ಹಿಂತಿರುಗಿ str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_exp(uint64_t n)
{
	ಪೂರ್ಣಾಂಕ shft;

	ಗಾಗಿ (shft = 10; shft <= 60; shft += 10) {
		ಆದರೆ (n < (1ULL << shft))
			ನಿಲ್ಲಿಸು;
	}
	ಹಿಂತಿರುಗಿ shft - 10;
}

ಅಕ್ಷರ *ಗಾತ್ರ_ಗಮ್ಯ_human_string(ಪೂರ್ಣಾಂಕ options, uint64_t ಬೈಟುಗಳು)
{
	ಅಕ್ಷರ buf[32];
	ಪೂರ್ಣಾಂಕ dec, ಘಾತಾಂಕ;
	uint64_t frac;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *letters = "BKMGTPE";
	ಅಕ್ಷರ suffix[ಗಾತ್ರ(" KiB")], *psuf = suffix;
	ಅಕ್ಷರ c;

	ಆದರೆ (options & ಗಾತ್ರ_SUFFIX_SPACE)
		*psuf++ = ' ';


	ಘಾತಾಂಕ  = get_exp(ಬೈಟುಗಳು);
	c    = *(letters + (ಘಾತಾಂಕ ? ಘಾತಾಂಕ / 10 : 0));
	dec  = ಘಾತಾಂಕ ? ಬೈಟುಗಳು / (1ULL << ಘಾತಾಂಕ) : ಬೈಟುಗಳು;
	frac = ಘಾತಾಂಕ ? ಬೈಟುಗಳು % (1ULL << ಘಾತಾಂಕ) : 0;

	*psuf++ = c;

	ಆದರೆ ((options & ಗಾತ್ರ_SUFFIX_3_LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/*ಸುತ್ತು*/
	ಆದರೆ (frac) {
		/* get 3 digits after decimal point */
		ಆದರೆ (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (ಘಾತಾಂಕ - 10)) ;
		ಇಲ್ಲದಿದ್ದರೆ
			frac = (frac * 1000) / (1ULL << (ಘಾತಾಂಕ)) ;

		ಆದರೆ (options & ಗಾತ್ರ_DECIMAL_2_DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		ಆದರೆ (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	ಆದರೆ (frac) {
		ರಚನೆ lconv ಸ್ಥಿರಾಂಕ *l = localeconv();
		ಅಕ್ಷರ *dp = l ? l->decimal_ಬಿಂದು : ಖಾಲಿ;
		ಪೂರ್ಣಾಂಕ len;

		ಆದರೆ (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, ಗಾತ್ರ(buf), "%d%s%02" PRIu64, dec, dp, frac);
		ಆದರೆ (len > 0 && (size_t) len < ಗಾತ್ರ(buf)) {
			/* remove potential extraneous zero */
			ಆದರೆ (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, ಗಾತ್ರ(buf) - len);
		} ಇಲ್ಲದಿದ್ದರೆ
			*buf = '\0';	/* snprintf error */
	} ಇಲ್ಲದಿದ್ದರೆ
		snprintf(buf, ಗಾತ್ರ(buf), "%d%s", dec, suffix);

	ಹಿಂತಿರುಗಿ strdup(buf);
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
ಪೂರ್ಣಾಂಕ string_ಗಮ್ಯ_idarray(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *list, ಪೂರ್ಣಾಂಕ ary[], size_t arysz,
			ಪೂರ್ಣಾಂಕ (name2id)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, size_t))
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *begin = ಖಾಲಿ, *p;
	size_t n = 0;

	ಆದರೆ (!list || !*list || !ary || !arysz || !name2id)
		ಹಿಂತಿರುಗಿ -1;

	ಗಾಗಿ (p = list; p && *p; p++) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೊನೆ = ಖಾಲಿ;
		ಪೂರ್ಣಾಂಕ id;

		ಆದರೆ (n >= arysz)
			ಹಿಂತಿರುಗಿ -2;
		ಆದರೆ (!begin)
			begin = p;		/* begin of the column name */
		ಆದರೆ (*p == ',')
			ಕೊನೆ = p;		/* terminate the name */
		ಆದರೆ (*(p + 1) == '\0')
			ಕೊನೆ = p + 1;		/* end of string */
		ಆದರೆ (!begin || !ಕೊನೆ)
			ಮುಂದುವರಿ;
		ಆದರೆ (ಕೊನೆ <= begin)
			ಹಿಂತಿರುಗಿ -1;

		id = name2id(begin, ಕೊನೆ - begin);
		ಆದರೆ (id == -1)
			ಹಿಂತಿರುಗಿ -1;
		ary[ n++ ] = id;
		begin = ಖಾಲಿ;
		ಆದರೆ (ಕೊನೆ && !*ಕೊನೆ)
			ನಿಲ್ಲಿಸು;
	}
	ಹಿಂತಿರುಗಿ n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
ಪೂರ್ಣಾಂಕ string_ಕೂಡಿಸು_ಗಮ್ಯ_idarray(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *list, ಪೂರ್ಣಾಂಕ ary[], size_t arysz,
			size_t *ary_pos, ಪೂರ್ಣಾಂಕ (name2id)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, size_t))
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *list_ಕೂಡಿಸು;
	ಪೂರ್ಣಾಂಕ r;

	ಆದರೆ (!list || !*list || !ary_pos || *ary_pos > arysz)
		ಹಿಂತಿರುಗಿ -1;

	ಆದರೆ (list[0] == '+')
		list_ಕೂಡಿಸು = &list[1];
	ಇಲ್ಲದಿದ್ದರೆ {
		list_ಕೂಡಿಸು = list;
		*ary_pos = 0;
	}

	r = string_ಗಮ್ಯ_idarray(list_ಕೂಡಿಸು, &ary[*ary_pos], arysz - *ary_pos, name2id);
	ಆದರೆ (r > 0)
		*ary_pos += r;
	ಹಿಂತಿರುಗಿ r;
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
ಪೂರ್ಣಾಂಕ string_ಗಮ್ಯ_bitarray(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *list,
		     ಅಕ್ಷರ *ary,
		     ಪೂರ್ಣಾಂಕ (*name2bit)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, size_t),
		     size_t allow_range)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *begin = ಖಾಲಿ, *p;

	ಆದರೆ (!list || !name2bit || !ary)
		ಹಿಂತಿರುಗಿ -EINVAL;

	ಗಾಗಿ (p = list; p && *p; p++) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೊನೆ = ಖಾಲಿ;
		ಪೂರ್ಣಾಂಕ ಬಿಟ್, ಗಣ_ಅಧೋ = 0, ಗಣ_higher = 0;

		ಆದರೆ (!begin)
			begin = p;		/* begin of the level name */
		ಆದರೆ (*p == ',')
			ಕೊನೆ = p;		/* terminate the name */
		ಆದರೆ (*(p + 1) == '\0')
			ಕೊನೆ = p + 1;		/* end of string */
		ಆದರೆ (!begin || !ಕೊನೆ)
			ಮುಂದುವರಿ;
		ಆದರೆ (ಕೊನೆ <= begin)
			ಹಿಂತಿರುಗಿ -1;
		ಆದರೆ (allow_range) {
			ಆದರೆ (*(ಕೊನೆ - 1) == '+') {
				ಕೊನೆ--;
				ಗಣ_ಅಧೋ = 1;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*begin == '+') {
				begin++;
				ಗಣ_higher = 1;
			}
		}

		ಬಿಟ್ = name2bit(begin, ಕೊನೆ - begin);
		ಆದರೆ (ಬಿಟ್ < 0)
			ಹಿಂತಿರುಗಿ ಬಿಟ್;
		setbit(ary, ಬಿಟ್);
		ಆದರೆ (ಗಣ_ಅಧೋ)
			ಆಗ (--ಬಿಟ್ >= 0)
				setbit(ary, ಬಿಟ್);
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಣ_higher)
			ಆಗ (++ಬಿಟ್ < (ಪೂರ್ಣಾಂಕ) allow_range)
				setbit(ary, ಬಿಟ್);
		begin = ಖಾಲಿ;
		ಆದರೆ (ಕೊನೆ && !*ಕೊನೆ)
			ನಿಲ್ಲಿಸು;
	}
	ಹಿಂತಿರುಗಿ 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
ಪೂರ್ಣಾಂಕ string_ಗಮ್ಯ_bitmask(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *list,
		     ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *ಮುಖವಾಡ,
		     ದೀರ್ಘ (*name2flag)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, size_t))
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *begin = ಖಾಲಿ, *p;

	ಆದರೆ (!list || !name2flag || !ಮುಖವಾಡ)
		ಹಿಂತಿರುಗಿ -EINVAL;

	ಗಾಗಿ (p = list; p && *p; p++) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೊನೆ = ಖಾಲಿ;
		ದೀರ್ಘ flag;

		ಆದರೆ (!begin)
			begin = p;		/* begin of the level name */
		ಆದರೆ (*p == ',')
			ಕೊನೆ = p;		/* terminate the name */
		ಆದರೆ (*(p + 1) == '\0')
			ಕೊನೆ = p + 1;		/* end of string */
		ಆದರೆ (!begin || !ಕೊನೆ)
			ಮುಂದುವರಿ;
		ಆದರೆ (ಕೊನೆ <= begin)
			ಹಿಂತಿರುಗಿ -1;

		flag = name2flag(begin, ಕೊನೆ - begin);
		ಆದರೆ (flag < 0)
			ಹಿಂತಿರುಗಿ flag;	/* error */
		*ಮುಖವಾಡ |= flag;
		begin = ಖಾಲಿ;
		ಆದರೆ (ಕೊನೆ && !*ಕೊನೆ)
			ನಿಲ್ಲಿಸು;
	}
	ಹಿಂತಿರುಗಿ 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
ಪೂರ್ಣಾಂಕ ul_parse_range(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಪೂರ್ಣಾಂಕ *ಅಧೋ, ಪೂರ್ಣಾಂಕ *ಊರ್ಧ್ವ, ಪೂರ್ಣಾಂಕ def)
{
	ಅಕ್ಷರ *ಕೊನೆ = ಖಾಲಿ;

	ಆದರೆ (!str)
		ಹಿಂತಿರುಗಿ 0;

	*ಊರ್ಧ್ವ = *ಅಧೋ = def;
	errno = 0;

	ಆದರೆ (*str == ':') {				/* <:N> */
		str++;
		*ಊರ್ಧ್ವ = strtol(str, &ಕೊನೆ, 10);
		ಆದರೆ (errno || !ಕೊನೆ || *ಕೊನೆ || ಕೊನೆ == str)
			ಹಿಂತಿರುಗಿ -1;
	} ಇಲ್ಲದಿದ್ದರೆ {
		*ಊರ್ಧ್ವ = *ಅಧೋ = strtol(str, &ಕೊನೆ, 10);
		ಆದರೆ (errno || !ಕೊನೆ || ಕೊನೆ == str)
			ಹಿಂತಿರುಗಿ -1;

		ಆದರೆ (*ಕೊನೆ == ':' && !*(ಕೊನೆ + 1))		/* <M:> */
			*ಊರ್ಧ್ವ = def;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*ಕೊನೆ == '-' || *ಕೊನೆ == ':') {	/* <M:N> <M-N> */
			str = ಕೊನೆ + 1;
			ಕೊನೆ = ಖಾಲಿ;
			errno = 0;
			*ಊರ್ಧ್ವ = strtol(str, &ಕೊನೆ, 10);

			ಆದರೆ (errno || !ಕೊನೆ || *ಕೊನೆ || ಕೊನೆ == str)
				ಹಿಂತಿರುಗಿ -1;
		}
	}
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ul_ಮಾರ್ಗ_ಮುಂದಿನ_segment(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, size_t *sz)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆರಂಭ, *p;

	ಆರಂಭ = str;
	*sz = 0;
	ಆಗ (ಆರಂಭ && *ಆರಂಭ == '/' && *(ಆರಂಭ + 1) == '/')
		ಆರಂಭ++;

	ಆದರೆ (!ಆರಂಭ || !*ಆರಂಭ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಗಾಗಿ (*sz = 1, p = ಆರಂಭ + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	ಹಿಂತಿರುಗಿ ಆರಂಭ;
}

ಪೂರ್ಣಾಂಕ streq_paths(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *a, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *b)
{
	ಆಗ (a && b) {
		size_t a_sz, b_sz;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *a_seg = ul_ಮಾರ್ಗ_ಮುಂದಿನ_segment(a, &a_sz);
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *b_seg = ul_ಮಾರ್ಗ_ಮುಂದಿನ_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		ಆದರೆ (a_sz + b_sz == 0)
			ಹಿಂತಿರುಗಿ 1;

		/* ignore trailing slash */
		ಆದರೆ (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			ಹಿಂತಿರುಗಿ 1;

		ಆದರೆ (!a_seg || !b_seg)
			ನಿಲ್ಲಿಸು;
		ಆದರೆ (a_sz != b_sz || ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(a_seg, b_seg, a_sz) != 0)
			ನಿಲ್ಲಿಸು;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	ಹಿಂತಿರುಗಿ 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
ಪೂರ್ಣಾಂಕ ul_ಮಾರ್ಗ_is_within(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dir)
{
	ಆದರೆ (!ಮಾರ್ಗ || !dir || !*ಮಾರ್ಗ || !*dir)
		ಹಿಂತಿರುಗಿ 0;

	ಆಗ (ಮಾರ್ಗ && dir) {
		size_t p_sz, d_sz;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p_seg = ul_ಮಾರ್ಗ_ಮುಂದಿನ_segment(ಮಾರ್ಗ, &p_sz);
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *d_seg = ul_ಮಾರ್ಗ_ಮುಂದಿನ_segment(dir, &d_sz);

		/* all @dir segments matched */
		ಆದರೆ (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			ಹಿಂತಿರುಗಿ 1;

		ಆದರೆ (!p_seg || !d_seg)
			ನಿಲ್ಲಿಸು;
		ಆದರೆ (p_sz != d_sz || ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(p_seg, d_seg, p_sz) != 0)
			ನಿಲ್ಲಿಸು;

		ಮಾರ್ಗ = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	ಹಿಂತಿರುಗಿ 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
ಅಕ್ಷರ *ul_strnconcat(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *suffix, size_t b)
{
	size_t a;
	ಅಕ್ಷರ *r;

	ಆದರೆ (!s && !suffix)
		ಹಿಂತಿರುಗಿ strdup("");
	ಆದರೆ (!s)
		ಹಿಂತಿರುಗಿ strndup(suffix, b);
	ಆದರೆ (!suffix)
		ಹಿಂತಿರುಗಿ strdup(s);

	ಪ್ರತಿಜ್ಞಾ(s);
	ಪ್ರತಿಜ್ಞಾ(suffix);

	a = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s);
	ಆದರೆ (b > ((size_t) -1) - a)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	r = ಸ್ಮರಣೆ_ಕೊಡು(a + b + 1);
	ಆದರೆ (!r)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಸ್ಮರಣೆ_ನಕಲು(r, s, a);
	ಸ್ಮರಣೆ_ನಕಲು(r + a, suffix, b);
	r[a+b] = 0;

	ಹಿಂತಿರುಗಿ r;
}

/* concatenate two strings to a new string */
ಅಕ್ಷರ *ul_strconcat(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *suffix)
{
	ಹಿಂತಿರುಗಿ ul_strnconcat(s, suffix, suffix ? ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
ಅಕ್ಷರ *ul_strfconcat(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *format, ...)
{
	va_list ap;
	ಅಕ್ಷರ *val, *res;
	ಪೂರ್ಣಾಂಕ sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	ಆದರೆ (sz < 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	res = ul_strnconcat(s, val, sz);
	ಬಿಡುಗಡೆ(val);
	ಹಿಂತಿರುಗಿ res;
}

ಪೂರ್ಣಾಂಕ ul_strappend(ಅಕ್ಷರ **a, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *b)
{
	size_t al, bl;
	ಅಕ್ಷರ *tmp;

	ಆದರೆ (!a)
		ಹಿಂತಿರುಗಿ -EINVAL;
	ಆದರೆ (!b || !*b)
		ಹಿಂತಿರುಗಿ 0;
	ಆದರೆ (!*a) {
		*a = strdup(b);
		ಹಿಂತಿರುಗಿ !*a ? -ENOMEM : 0;
	}

	al = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(*a);
	bl = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(b);

	tmp = realloc(*a, al + bl + 1);
	ಆದರೆ (!tmp)
		ಹಿಂತಿರುಗಿ -ENOMEM;
	*a = tmp;
	ಸ್ಮರಣೆ_ನಕಲು((*a) + al, b, bl + 1);
	ಹಿಂತಿರುಗಿ 0;
}

/* the hybrid version of strfconcat and strappend. */
ಪೂರ್ಣಾಂಕ strfappend(ಅಕ್ಷರ **a, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *format, ...)
{
	va_list ap;
	ಪೂರ್ಣಾಂಕ res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	ಹಿಂತಿರುಗಿ res;
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ul_strvfappend(ಅಕ್ಷರ **a, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *format, va_list ap)
{
	ಅಕ್ಷರ *val;
	ಪೂರ್ಣಾಂಕ sz;
	ಪೂರ್ಣಾಂಕ res;

	sz = vasprintf(&val, format, ap);
	ಆದರೆ (sz < 0)
		ಹಿಂತಿರುಗಿ -errno;

	res = ul_strappend(a, val);
	ಬಿಡುಗಡೆ(val);
	ಹಿಂತಿರುಗಿ res;
}

ಸ್ಥಿರ size_t strcspn_escaped(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *reject)
{
	ಪೂರ್ಣಾಂಕ escaped = 0;
	ಪೂರ್ಣಾಂಕ n;

	ಗಾಗಿ (n=0; s[n]; n++) {
		ಆದರೆ (escaped)
			escaped = 0;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (s[n] == '\\')
			escaped = 1;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strchr(reject, s[n]))
			ನಿಲ್ಲಿಸು;
	}

	/* if s ends in \, return index of previous char */
	ಹಿಂತಿರುಗಿ n - escaped;
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
ಅಕ್ಷರ *ul_strchr_escaped(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಪೂರ್ಣಾಂಕ c)
{
	ಅಕ್ಷರ *p;
	ಪೂರ್ಣಾಂಕ esc = 0;

	ಗಾಗಿ (p = (ಅಕ್ಷರ *) s; p && *p; p++) {
		ಆದರೆ (!esc && *p == '\\') {
			esc = 1;
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (*p == c && (!esc || c == '\\'))
			ಹಿಂತಿರುಗಿ p;
		esc = 0;
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/* Split a string into words. */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ul_ವಿಭಜಿಸು(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **state, size_t *l, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *separator, ಪೂರ್ಣಾಂಕ quoted)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *current;

	current = *state;

	ಆದರೆ (!*current) {
		ಪ್ರತಿಜ್ಞಾ(**state == '\0');
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	current += strspn(current, separator);
	ಆದರೆ (!*current) {
		*state = current;
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಆದರೆ (quoted && strchr("\'\"", *current)) {
		ಅಕ್ಷರ quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		ಆದರೆ (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
		*state = current++ + *l + 2;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (quoted) {
		*l = strcspn_escaped(current, separator);
		ಆದರೆ (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
		*state = current + *l;
	} ಇಲ್ಲದಿದ್ದರೆ {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	ಹಿಂತಿರುಗಿ current;
}

/* Rewind file pointer forward to new line.  */
ಪೂರ್ಣಾಂಕ skip_fline(FILE *fp)
{
	ಪೂರ್ಣಾಂಕ ch;

	ಮಾಡು {
		ಆದರೆ ((ch = fgetc(fp)) == EOF)
			ಹಿಂತಿರುಗಿ 1;
		ಆದರೆ (ch == '\n')
			ಹಿಂತಿರುಗಿ 0;
	} ಆಗ (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
ಪೂರ್ಣಾಂಕ ul_stralnumcmp(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p1, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p2)
{
	ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *s1 = (ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *) p1;
	ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *s2 = (ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *) p2;
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ c1, c2;

	ಮಾಡು {
		ಮಾಡು {
			c1 = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *s1++;
		} ಆಗ (c1 != '\0' && !ಅಕ್ಷರವೇ_ಅಂಕಿಯೇ((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) c1));

		ಮಾಡು {
			c2 = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *s2++;
		} ಆಗ (c2 != '\0' && !ಅಕ್ಷರವೇ_ಅಂಕಿಯೇ((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) c2));

		ಆದರೆ (c1 != '\0')
			c1 = tolower(c1);
		ಆದರೆ (c2 != '\0')
			c2 = tolower(c2);
		ಆದರೆ (c1 == '\0')
			ಹಿಂತಿರುಗಿ c1 - c2;
	} ಆಗ (c1 == c2);

	ಹಿಂತಿರುಗಿ c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
ಪೂರ್ಣಾಂಕ ul_optstr_ಮುಂದಿನ(ಅಕ್ಷರ **optstr, ಅಕ್ಷರ **name, size_t *namesz,
		   ಅಕ್ಷರ **ಮೌಲ್ಯ, size_t *valsz)
{
	ಪೂರ್ಣಾಂಕ open_quote = 0;
	ಅಕ್ಷರ *ಆರಂಭ = ಖಾಲಿ, *stop = ಖಾಲಿ, *p, *sep = ಖಾಲಿ;
	ಅಕ್ಷರ *optstr0;

	ಪ್ರತಿಜ್ಞಾ(optstr);
	ಪ್ರತಿಜ್ಞಾ(*optstr);

	optstr0 = *optstr;

	ಆದರೆ (name)
		*name = ಖಾಲಿ;
	ಆದರೆ (namesz)
		*namesz = 0;
	ಆದರೆ (ಮೌಲ್ಯ)
		*ಮೌಲ್ಯ = ಖಾಲಿ;
	ಆದರೆ (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	ಆಗ (optstr0 && *optstr0 == ',')
		optstr0++;

	ಗಾಗಿ (p = optstr0; p && *p; p++) {
		ಆದರೆ (!ಆರಂಭ && *p == '=')
			ಹಿಂತಿರುಗಿ -EINVAL;
		ಆದರೆ (!ಆರಂಭ)
			ಆರಂಭ = p;		/* beginning of the option item */
		ಆದರೆ (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		ಆದರೆ (open_quote)
			ಮುಂದುವರಿ;		/* still in quoted block */
		ಆದರೆ (!sep && p > ಆರಂಭ && *p == '=')
			sep = p;		/* name and value separator */
		ಆದರೆ (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		ಆದರೆ (!ಆರಂಭ || !stop)
			ಮುಂದುವರಿ;
		ಆದರೆ (stop <= ಆರಂಭ)
			ಹಿಂತಿರುಗಿ -EINVAL;

		ಆದರೆ (name)
			*name = ಆರಂಭ;
		ಆದರೆ (namesz)
			*namesz = sep ? sep - ಆರಂಭ : stop - ಆರಂಭ;
		*optstr = *stop ? stop + 1 : stop;

		ಆದರೆ (sep) {
			ಆದರೆ (ಮೌಲ್ಯ)
				*ಮೌಲ್ಯ = sep + 1;
			ಆದರೆ (valsz)
				*valsz = stop - sep - 1;
		}
		ಹಿಂತಿರುಗಿ 0;
	}

	ಹಿಂತಿರುಗಿ 1;				/* end of optstr */
}

ಪೂರ್ಣಾಂಕ ul_optstr_is_valid(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *optstr)
{
	ಪೂರ್ಣಾಂಕ rc;
	ಅಕ್ಷರ *p = (ಅಕ್ಷರ *) optstr;

	ಆಗ ((rc = ul_optstr_ಮುಂದಿನ(&p, ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ)) == 0);
	ಹಿಂತಿರುಗಿ rc < 0 ? 0 : 1;
}

ಅಕ್ಷರ *ul_optstr_get_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *optstr, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೀಲಿ)
{
	size_t sz, namesz = 0, valsz = 0;
	ಅಕ್ಷರ *name = ಖಾಲಿ, *ಮೌಲ್ಯ = ಖಾಲಿ;
	ಅಕ್ಷರ *p = (ಅಕ್ಷರ *) optstr;

	ಆದರೆ (!optstr || !ಕೀಲಿ || !*ಕೀಲಿ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	sz = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಕೀಲಿ);
	ಆಗ (ul_optstr_ಮುಂದಿನ(&p, &name, &namesz, &ಮೌಲ್ಯ, &valsz) == 0) {
		ಆದರೆ (namesz != sz || !valsz)
			ಮುಂದುವರಿ;
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(name, ಕೀಲಿ, namesz) == 0)
			ಹಿಂತಿರುಗಿ strndup(ಮೌಲ್ಯ, valsz);
	}
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(ಅಕ್ಷರ *str)
{
	size_t len;
	ಅಕ್ಷರ q;

	ಆದರೆ (!str || !*str)
		ಹಿಂತಿರುಗಿ 0;

	len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str);
	q = *str;

	ಆದರೆ ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		ಹಿಂತಿರುಗಿ len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	ಹಿಂತಿರುಗಿ len - 2;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_STRUTILS

ರಚನೆ testS {
	ಅಕ್ಷರ *name;
	ಅಕ್ಷರ *ಮೌಲ್ಯ;
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ test_strdup_ಗಮ್ಯ_member(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ರಚನೆ testS *xx;

	ಆದರೆ (argc < 3)
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;

	xx = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*xx));
	ಆದರೆ (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_ಗಮ್ಯ_struct_member(xx, name, argv[1]);
	strdup_ಗಮ್ಯ_struct_member(xx, ಮೌಲ್ಯ, argv[2]);

	ಆದರೆ (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->ಮೌಲ್ಯ, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	ಬರೆ("1: '%s', 2: '%s'\n", xx->name, xx->ಮೌಲ್ಯ);

	ಬಿಡುಗಡೆ(xx->name);
	ಬಿಡುಗಡೆ(xx->ಮೌಲ್ಯ);
	ಬಿಡುಗಡೆ(xx);
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ test_strutils_sizes(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	uintmax_t ಗಾತ್ರ_2 = 0;
	ಅಕ್ಷರ *hum1, *hum2, *hum3;

	ಆದರೆ (argc < 2)
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;

	ಆದರೆ (strtosize(argv[1], &ಗಾತ್ರ_2))
		errx(EXIT_FAILURE, "invalid size '%s' value", argv[1]);

	hum1 = ಗಾತ್ರ_ಗಮ್ಯ_human_string(ಗಾತ್ರ_SUFFIX_1_LETTER, ಗಾತ್ರ_2);
	hum2 = ಗಾತ್ರ_ಗಮ್ಯ_human_string(ಗಾತ್ರ_SUFFIX_3_LETTER |
				    ಗಾತ್ರ_SUFFIX_SPACE, ಗಾತ್ರ_2);
	hum3 = ಗಾತ್ರ_ಗಮ್ಯ_human_string(ಗಾತ್ರ_SUFFIX_3_LETTER |
				    ಗಾತ್ರ_SUFFIX_SPACE |
				    ಗಾತ್ರ_DECIMAL_2_DIGITS, ಗಾತ್ರ_2);

	ಬರೆ("%25s : %20ju : %8s : %12s : %13s\n", argv[1], ಗಾತ್ರ_2, hum1, hum2, hum3);
	ಬಿಡುಗಡೆ(hum1);
	ಬಿಡುಗಡೆ(hum2);
	ಬಿಡುಗಡೆ(hum3);

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ test_strutils_cmp_paths(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ಪೂರ್ಣಾಂಕ rc = streq_paths(argv[1], argv[2]);

	ಆದರೆ (argc < 3)
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;

	ಬರೆ("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ test_strutils_ಮಾರ್ಗ_is_within(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ಪೂರ್ಣಾಂಕ rc;

	ಆದರೆ (argc < 3)
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;

	rc = ul_ಮಾರ್ಗ_is_within(argv[1], argv[2]);

	ಬರೆ("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ test_strutils_normalize(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *src, *dst, *org;
	size_t sz, len;

	ಆದರೆ (argc < 2)
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;

	org = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *) strdup(argv[1]);
	src = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *) strdup((ಅಕ್ಷರ *) org);
	len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ((ಅಕ್ಷರ *) src);
	dst = ಸ್ಮರಣೆ_ಕೊಡು(len + 1);

	ಆದರೆ (!org || !src || !dst)
		ಹೋಗು ಮುಗಿದ;

	/* two buffers */
	sz = __normalize_whitespace(src, len, dst, len + 1);
	ಬರೆ("1: '%s' --> '%s' [sz=%zu]\n", src, dst, sz);

	/* one buffer */
	sz = normalize_whitespace(src);
	ಬರೆ("2: '%s' --> '%s' [sz=%zu]\n", org, src, sz);

ಮುಗಿದ:
	ಬಿಡುಗಡೆ(src);
	ಬಿಡುಗಡೆ(dst);
	ಬಿಡುಗಡೆ(org);

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ test_strutils_cstrcasecmp(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ಅಕ್ಷರ *a, *b;

	ಆದರೆ (argc < 3)
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	ಆದರೆ (!a || !b)
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;

	ಬರೆ("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	ಬರೆ("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	ಬರೆ("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(a)));

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ಆದರೆ (argc == 3 && strcmp(argv[1], "--size") == 0) {
		ಹಿಂತಿರುಗಿ test_strutils_sizes(argc - 1, argv + 1);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		ಬರೆ("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						ಖಾಲಿ));
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		ಆದರೆ (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		ಬರೆ("'%s'-->%s\n", argv[2], val ? "true" : "false");
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		ಹಿಂತಿರುಗಿ test_strutils_cmp_paths(argc - 1, argv + 1);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		ಹಿಂತಿರುಗಿ test_strutils_ಮಾರ್ಗ_is_within(argc - 1, argv + 1);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		ಹಿಂತಿರುಗಿ test_strdup_ಗಮ್ಯ_member(argc - 1, argv + 1);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		ಬರೆ("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		ಹಿಂತಿರುಗಿ test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		ಹಿಂತಿರುಗಿ test_strutils_normalize(argc - 1, argv + 1);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		ಬರೆ("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		ಬರೆ("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		ಬರೆ("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		ಬರೆ("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		ಬರೆ("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		ಬರೆ("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		ಬರೆ("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		ಅಕ್ಷರ *buf = "abc\0Y\0\0xyz\0X";
		ಅಕ್ಷರ *ಕೊನೆ = buf + 12;
		ಅಕ್ಷರ *p = buf;

		ಮಾಡು {
			ಬರೆ("str: '%s'\n", p);
		} ಆಗ ((p = ul_ಮುಂದಿನ_string(p, ಕೊನೆ)));

		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		ಅಕ್ಷರ *name = ಖಾಲಿ, *val = ಖಾಲಿ;
		ಅಕ್ಷರ *p = argv[2];
		ಪೂರ್ಣಾಂಕ rc;

		ಆದರೆ (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		ಆಗ ((rc = ul_optstr_ಮುಂದಿನ(&p, &name, &namesz, &val, &valsz)) == 0) {
			ಬರೆ("'%.*s' : '%.*s'\n", (ಪೂರ್ಣಾಂಕ) namesz, name,
						   (ಪೂರ್ಣಾಂಕ) valsz, val);
		}
		ಆದರೆ (rc == 1)
			ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		ಅಕ್ಷರ *unquoted = strdup(argv[2]);

		ಆದರೆ (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		ಬರೆ("%s-->%s\n", argv[2], unquoted);
		ಬಿಡುಗಡೆ(unquoted);
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;

	} ಇಲ್ಲದಿದ್ದರೆ {
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
		ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
	}

	ಹಿಂತಿರುಗಿ EXIT_FAILURE;
}
#ಆದರೆ_ಅಂತ್ಯ /* TEST_PROGRAM_STRUTILS */
