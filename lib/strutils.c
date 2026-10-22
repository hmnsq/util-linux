/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Karel Zak <kzak@redhat.com> [2010]
 *          Davidlohr Bueso <dave@gnu.org> [2010]
 */
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <inttypes.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <strings.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "cctype.h"
#ଅନ୍ତର୍ଭାଵ "nls.h"
#ଅନ୍ତର୍ଭାଵ "strutils.h"
#ଅନ୍ତର୍ଭାଵ "timeutils.h"
#ଅନ୍ତର୍ଭାଵ "bitops.h"
#ଅନ୍ତର୍ଭାଵ "pathnames.h"

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା STRTOXX_EXIT_CODE = EXIT_FAILURE;

ଶୂନ୍ୟ strutils_set_exitcode(ପୂର୍ଣ୍ଣସଂଖ୍ୟା ex)
{
	STRTOXX_EXIT_CODE = ex;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା do_scale_by_power (uintmax_t *x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା base, ପୂର୍ଣ୍ଣସଂଖ୍ୟା power)
{
	ଯେତେବେଳେ (power--) {
		ଯଦି (UINTMAX_MAX / base < *x)
			ଫେରନ୍ତୁ -ERANGE;
		*x *= base;
	}
	ଫେରନ୍ତୁ 0;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_parse_size(ଅଚଳ ଅକ୍ଷର *str, uintmax_t *res, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *power)
{
	ଅଚଳ ଅକ୍ଷର *p;
	ଅକ୍ଷର *end;
	uintmax_t x, frac = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା base = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *suf  = "KMGTPEZY";
	ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *suf2 = "kmgtpezy";
	ଅଚଳ ଅକ୍ଷର *sp;

	*res = 0;

	ଯଦି (!str || !*str) {
		rc = -EINVAL;
		ଯାଅ err;
	}

	/* Only positive numbers are acceptable
	 *
	 * Note that this check is not perfect, it would be better to
	 * use lconv->negative_sign. But coreutils use the same solution,
	 * so it's probably good enough...
	 */
	p = str;
	ଯେତେବେଳେ (isspace((unsigned ଅକ୍ଷର) *p))
		p++;
	ଯଦି (*p == '-') {
		rc = -EINVAL;
		ଯାଅ err;
	}

	errno = 0, end = ଖାଲି;
	x = strtoumax(str, &end, 0);

	ଯଦି (end == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		ଯାଅ err;
	}
	ଯଦି (!end || !*end)
		ଯାଅ done;			/* without suffix */
	p = end;

	/*
	 * Check size suffixes
	 */
check_suffix:
	ଯଦି (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		base = 1024;			/* XiB, 2^N */
	ଅନ୍ୟଥା ଯଦି ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		base = 1000;			/* XB, 10^N */
	ଅନ୍ୟଥା ଯଦି (*(p + 1)) {
		ସଂରଚନା lconv ଅଚଳ *l = localeconv();
		ଅଚଳ ଅକ୍ଷର *dp = l ? l->decimal_point : ଖାଲି;
		size_t dpsz = dp ? strlen(dp) : 0;

		ଯଦି (frac == 0 && *p && dp && strncmp(dp, p, dpsz) == 0) {
			ଅଚଳ ଅକ୍ଷର *fstr = p + dpsz;

			ପାଇଁ (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			ଯଦି (isdigit(*fstr)) {
				errno = 0, end = ଖାଲି;
				frac = strtoumax(fstr, &end, 0);
				ଯଦି (end == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					ଯାଅ err;
				}
			} ଅନ୍ୟଥା
				end = (ଅକ୍ଷର *) p;

			ଯଦି (!end || !*end) {
				rc = -EINVAL;
				ଯାଅ err;	/* without suffix, but with fractions */
			}
			p = end;
			ଯାଅ check_suffix;
		}
		rc = -EINVAL;
		ଯାଅ err;			/* unexpected suffix */
	}

	sp = strchr(suf, *p);
	ଯଦି (sp)
		pwr = (sp - suf) + 1;
	ଅନ୍ୟଥା {
		sp = strchr(suf2, *p);
		ଯଦି (sp)
			pwr = (sp - suf2) + 1;
		ଅନ୍ୟଥା {
			rc = -EINVAL;
			ଯାଅ err;
		}
	}

	ଯଦି (power)
		*power = pwr;

	rc = do_scale_by_power(&x, base, pwr);
	ଯଦି (rc)
		ଯାଅ err;

	ଯଦି (frac && pwr) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
		uintmax_t frac_base = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_base, base, pwr);
		ଯଦି (rc)
			ଯାଅ err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		କର {
			frac_val = ((frac % 10) * frac_base + frac_val) / 10;
			frac /= 10;
		} ଯେତେବେଳେ (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		ପାଇଁ (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		ଯଦି (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			ଯାଅ err;
		}
		x += frac_val;
	}
done:
	*res = x;
err:
	ଯଦି (rc < 0)
		errno = -rc;
	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା strtosize(ଅଚଳ ଅକ୍ଷର *str, uintmax_t *res)
{
	ଫେରନ୍ତୁ ul_parse_size(str, res, ଖାଲି);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା isdigit_strend(ଅଚଳ ଅକ୍ଷର *str, ଅଚଳ ଅକ୍ଷର **end)
{
	ଅଚଳ ଅକ୍ଷର *p;

	ପାଇଁ (p = str; p && *p && isdigit((unsigned ଅକ୍ଷର) *p); p++);

	ଯଦି (end)
		*end = p;
	ଫେରନ୍ତୁ p && p > str && !*p;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା isxdigit_strend(ଅଚଳ ଅକ୍ଷର *str, ଅଚଳ ଅକ୍ଷର **end)
{
	ଅଚଳ ଅକ୍ଷର *p;

	ପାଇଁ (p = str; p && *p && isxdigit((unsigned ଅକ୍ଷର) *p); p++);

	ଯଦି (end)
		*end = p;

	ଫେରନ୍ତୁ p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_strtobool(ଅଚଳ ଅକ୍ଷର *str, bool *result)
{
	ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	ଯଦି (!str || !result)
		ଫେରନ୍ତୁ -(errno = EINVAL);

	errno = 0;

	ପାଇଁ (i = 0; i < ARRAY_SIZE(bool_true); i++) {
		ଯଦି (c_strcasecmp(str, bool_true[i]) == 0) {
			*result = true;
			ଫେରନ୍ତୁ 0;
		}
	}

	ପାଇଁ (i = 0; i < ARRAY_SIZE(bool_false); i++) {
		ଯଦି (c_strcasecmp(str, bool_false[i]) == 0) {
			*result = false;
			ଫେରନ୍ତୁ 0;
		}
	}

	ଫେରନ୍ତୁ -(errno = EINVAL);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_parse_switch(ଅଚଳ ଅକ୍ଷର *arg, ...)
{
	ଅଚଳ ଅକ୍ଷର *a, *b;
	va_list ap;

	va_start(ap, arg);
	କର {
		a = va_arg(ap, ଅକ୍ଷର *);
		ଯଦି (!a)
			ଭାଙ୍ଗ;
		b = va_arg(ap, ଅକ୍ଷର *);
		ଯଦି (!b)
			ଭାଙ୍ଗ;

		ଯଦି (strcmp(arg, a) == 0) {
			va_end(ap);
			ଫେରନ୍ତୁ 1;
		}

		ଯଦି (strcmp(arg, b) == 0) {
			va_end(ap);
			ଫେରନ୍ତୁ 0;
		}
	} ଯେତେବେଳେ (1);
	va_end(ap);

	errx(STRTOXX_EXIT_CODE, _("unsupported argument: %s"), arg);
}

#ifndef HAVE_MEMPCPY
ଶୂନ୍ୟ *mempcpy(ଶୂନ୍ୟ *restrict dest, ଅଚଳ ଶୂନ୍ୟ *restrict src, size_t n)
{
	ଫେରନ୍ତୁ ((ଅକ୍ଷର *)memcpy(dest, src, n)) + n;
}
#endif

#ifndef HAVE_STRNLEN
size_t strnlen(ଅଚଳ ଅକ୍ଷର *s, size_t maxlen)
{
	size_t i;

	ପାଇଁ (i = 0; i < maxlen; i++) {
		ଯଦି (s[i] == '\0')
			ଫେରନ୍ତୁ i;
	}
	ଫେରନ୍ତୁ maxlen;
}
#endif

#ifndef HAVE_STRNCHR
ଅକ୍ଷର *strnchr(ଅଚଳ ଅକ୍ଷର *s, size_t maxlen, ପୂର୍ଣ୍ଣସଂଖ୍ୟା c)
{
	ପାଇଁ (; maxlen-- && *s != '\0'; ++s)
		ଯଦି (*s == (ଅକ୍ଷର)c)
			ଫେରନ୍ତୁ (ଅକ୍ଷର *)s;
	ଫେରନ୍ତୁ ଖାଲି;
}
#endif

#ifndef HAVE_STRNDUP
ଅକ୍ଷର *strndup(ଅଚଳ ଅକ୍ଷର *s, size_t n)
{
	size_t len = strnlen(s, n);
	ଅକ୍ଷର *new = malloc((len + 1) * ଆକାର(ଅକ୍ଷର));
	ଯଦି (!new)
		ଫେରନ୍ତୁ ଖାଲି;
	new[len] = '\0';
	ଫେରନ୍ତୁ (ଅକ୍ଷର *) memcpy(new, s, len);
}
#endif

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_strtos64(ଅଚଳ ଅକ୍ଷର *str, int64_t *ଅଙ୍କ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା base)
{
	ଅକ୍ଷର *end = ଖାଲି;

	ଯଦି (str == ଖାଲି || *str == '\0')
		ଫେରନ୍ତୁ -(errno = EINVAL);

	errno = 0;
	*ଅଙ୍କ = (int64_t) strtoimax(str, &end, base);

	ଯଦି (errno != 0)
		ଫେରନ୍ତୁ -errno;
	ଯଦି (str == end || (end && *end))
		ଫେରନ୍ତୁ -(errno = EINVAL);
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_strtou64(ଅଚଳ ଅକ୍ଷର *str, uint64_t *ଅଙ୍କ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା base)
{
	ଅକ୍ଷର *end = ଖାଲି;
	int64_t tmp;

	ଯଦି (str == ଖାଲି || *str == '\0')
		ଫେରନ୍ତୁ -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &end, base);
	ଯଦି (tmp < 0)
		errno = ERANGE;
	ଅନ୍ୟଥା {
		errno = 0;
		*ଅଙ୍କ = strtoumax(str, &end, base);
	}

	ଯଦି (errno != 0)
		ଫେରନ୍ତୁ -errno;
	ଯଦି (str == end || (end && *end))
		ଫେରନ୍ତୁ -(errno = EINVAL);
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_strtos32(ଅଚଳ ଅକ୍ଷର *str, int32_t *ଅଙ୍କ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା base)
{
	int64_t tmp;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	rc = ul_strtos64(str, &tmp, base);
	ଯଦି (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	ଯଦି (rc == 0)
		*ଅଙ୍କ = (int32_t) tmp;
	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_strtou32(ଅଚଳ ଅକ୍ଷର *str, uint32_t *ଅଙ୍କ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା base)
{
	uint64_t tmp;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	rc = ul_strtou64(str, &tmp, base);
	ଯଦି (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	ଯଦି (rc == 0)
		*ଅଙ୍କ = (uint32_t) tmp;
	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_strtou16(ଅଚଳ ଅକ୍ଷର *str, uint16_t *ଅଙ୍କ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା base)
{
	uint64_t tmp;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	rc = ul_strtou64(str, &tmp, base);
	ଯଦି (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	ଯଦି (rc == 0)
		*ଅଙ୍କ = (uint16_t) tmp;
	ଫେରନ୍ତୁ rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(ଅଚଳ ଅକ୍ଷର *str, ପୂର୍ଣ୍ଣସଂଖ୍ୟା base, ଅଚଳ ଅକ୍ଷର *errmesg,
			     int64_t ତଳ, int64_t up)
{
	int64_t ଅଙ୍କ = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	rc = ul_strtos64(str, &ଅଙ୍କ, base);
	ଯଦି (rc == 0 && ((ତଳ && ଅଙ୍କ < ତଳ) || (up && ଅଙ୍କ > up)))
		rc = -(errno = ERANGE);

	ଯଦି (rc) {
		ଯଦି (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	ଫେରନ୍ତୁ ଅଙ୍କ;
}

uint64_t str2unum_or_err(ଅଚଳ ଅକ୍ଷର *str, ପୂର୍ଣ୍ଣସଂଖ୍ୟା base, ଅଚଳ ଅକ୍ଷର *errmesg, uint64_t up)
{
	uint64_t ଅଙ୍କ = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	rc = ul_strtou64(str, &ଅଙ୍କ, base);
	ଯଦି (rc == 0 && (up && ଅଙ୍କ > up))
		rc = -(errno = ERANGE);

	ଯଦି (rc) {
		ଯଦି (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	ଫେରନ୍ତୁ ଅଙ୍କ;
}

ଦ୍ୱିଗୁଣ strtod_or_err(ଅଚଳ ଅକ୍ଷର *str, ଅଚଳ ଅକ୍ଷର *errmesg)
{
	ଦ୍ୱିଗୁଣ ଅଙ୍କ;
	ଅକ୍ଷର *end = ଖାଲି;

	errno = 0;
	ଯଦି (str == ଖାଲି || *str == '\0')
		ଯାଅ err;
	ଅଙ୍କ = strtod(str, &end);

	ଯଦି (errno || str == end || (end && *end))
		ଯାଅ err;

	ଫେରନ୍ତୁ ଅଙ୍କ;
err:
	ଯଦି (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_strtold(ଅଚଳ ଅକ୍ଷର *str, long ଦ୍ୱିଗୁଣ *ଅଙ୍କ)
{
	ଅକ୍ଷର *end = ଖାଲି;

	errno = 0;
	ଯଦି (str == ଖାଲି || *str == '\0')
		ଫେରନ୍ତୁ -(errno = EINVAL);
	*ଅଙ୍କ = strtold(str, &end);

	ଯଦି (errno != 0)
		ଫେରନ୍ତୁ -errno;
	ଯଦି (str == end || (end && *end))
		ଫେରନ୍ତୁ -(errno = EINVAL);
	ଫେରନ୍ତୁ 0;
}

long ଦ୍ୱିଗୁଣ strtold_or_err(ଅଚଳ ଅକ୍ଷର *str, ଅଚଳ ଅକ୍ଷର *errmesg)
{
	long ଦ୍ୱିଗୁଣ ଅଙ୍କ = 0;

	ଯଦି (ul_strtold(str, &ଅଙ୍କ) == 0)
		ଫେରନ୍ତୁ ଅଙ୍କ;
	ଯଦି (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(ଅଚଳ ଅକ୍ଷର *str, ଅଚଳ ଅକ୍ଷର *errmesg)
{
	uintmax_t ଅଙ୍କ;

	ଯଦି (strtosize(str, &ଅଙ୍କ) == 0)
		ଫେରନ୍ତୁ ଅଙ୍କ;

	ଯଦି (errno)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}


ଶୂନ୍ୟ strtotimeval_or_err(ଅଚଳ ଅକ୍ଷର *str, ସଂରଚନା timeval *tv, ଅଚଳ ଅକ୍ଷର *errmesg)
{
	long ଦ୍ୱିଗୁଣ user_ନିବେଶ;

	user_ନିବେଶ = strtold_or_err(str, errmesg);
	tv->tv_sec = (time_t) user_ନିବେଶ;
	tv->tv_usec = (suseconds_t)((user_ନିବେଶ - tv->tv_sec) * USEC_PER_SEC);
}

ଶୂନ୍ୟ strtotimespec_or_err(ଅଚଳ ଅକ୍ଷର *str, ସଂରଚନା timespec *ts, ଅଚଳ ଅକ୍ଷର *errmesg)
{
	long ଦ୍ୱିଗୁଣ user_ନିବେଶ;

	user_ନିବେଶ = strtold_or_err(str, errmesg);
	ts->tv_sec = (time_t) user_ନିବେଶ;
	ts->tv_nsec = (long)((user_ନିବେଶ - ts->tv_sec) * NSEC_PER_SEC);
}

time_t strtotime_or_err(ଅଚଳ ଅକ୍ଷର *str, ଅଚଳ ଅକ୍ଷର *errmesg)
{
	int64_t user_ନିବେଶ;

	user_ନିବେଶ = strtos64_or_err(str, errmesg);
	ଫେରନ୍ତୁ (time_t) user_ନିବେଶ;
}

bool hyperlinkwanted(ଅଚଳ ଅକ୍ଷର *mode)
{
	ଯଦି (mode && strcmp(mode, "never") == 0)
		ଫେରନ୍ତୁ false;

	ଯଦି (mode && strcmp(mode, "always") == 0)
		ଫେରନ୍ତୁ true;

	ଯଦି (!mode || strcmp(mode, "auto") == 0)
		ଫେରନ୍ତୁ isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), mode);
}

bool annotationwanted(ଅଚଳ ଅକ୍ଷର *mode)
{
	ଯଦି (mode && strcmp(mode, "never") == 0)
		ଫେରନ୍ତୁ false;

	ଯଦି (mode && strcmp(mode, "always") == 0)
		ଫେରନ୍ତୁ true;

	ଯଦି (!mode || strcmp(mode, "auto") == 0)
		ଫେରନ୍ତୁ isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), mode);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
ଅକ୍ଷର *xstrmode(mode_t mode, ଅକ୍ଷର *str)
{
	unsigned short i = 0;

	ଯଦି (S_ISDIR(mode))
		str[i++] = 'd';
	ଅନ୍ୟଥା ଯଦି (S_ISLNK(mode))
		str[i++] = 'l';
	ଅନ୍ୟଥା ଯଦି (S_ISCHR(mode))
		str[i++] = 'c';
	ଅନ୍ୟଥା ଯଦି (S_ISBLK(mode))
		str[i++] = 'b';
	ଅନ୍ୟଥା ଯଦି (S_ISSOCK(mode))
		str[i++] = 's';
	ଅନ୍ୟଥା ଯଦି (S_ISFIFO(mode))
		str[i++] = 'p';
	ଅନ୍ୟଥା ଯଦି (S_ISREG(mode))
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

	ଫେରନ୍ତୁ str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_exp(uint64_t n)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା shft;

	ପାଇଁ (shft = 10; shft <= 60; shft += 10) {
		ଯଦି (n < (1ULL << shft))
			ଭାଙ୍ଗ;
	}
	ଫେରନ୍ତୁ shft - 10;
}

ଅକ୍ଷର *size_to_human_string(ପୂର୍ଣ୍ଣସଂଖ୍ୟା options, uint64_t bytes)
{
	ଅକ୍ଷର buf[32];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dec, exp;
	uint64_t frac;
	ଅଚଳ ଅକ୍ଷର *letters = "BKMGTPE";
	ଅକ୍ଷର suffix[ଆକାର(" KiB")], *psuf = suffix;
	ଅକ୍ଷର c;

	ଯଦି (options & SIZE_SUFFIX_SPACE)
		*psuf++ = ' ';


	exp  = get_exp(bytes);
	c    = *(letters + (exp ? exp / 10 : 0));
	dec  = exp ? bytes / (1ULL << exp) : bytes;
	frac = exp ? bytes % (1ULL << exp) : 0;

	*psuf++ = c;

	ଯଦି ((options & SIZE_SUFFIX_3LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/* round */
	ଯଦି (frac) {
		/* get 3 digits after decimal point */
		ଯଦି (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (exp - 10)) ;
		ଅନ୍ୟଥା
			frac = (frac * 1000) / (1ULL << (exp)) ;

		ଯଦି (options & SIZE_DECIMAL_2DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} ଅନ୍ୟଥା {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		ଯଦି (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	ଯଦି (frac) {
		ସଂରଚନା lconv ଅଚଳ *l = localeconv();
		ଅକ୍ଷର *dp = l ? l->decimal_point : ଖାଲି;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;

		ଯଦି (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, ଆକାର(buf), "%d%s%02" PRIu64, dec, dp, frac);
		ଯଦି (len > 0 && (size_t) len < ଆକାର(buf)) {
			/* remove potential extraneous zero */
			ଯଦି (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, ଆକାର(buf) - len);
		} ଅନ୍ୟଥା
			*buf = '\0';	/* snprintf error */
	} ଅନ୍ୟଥା
		snprintf(buf, ଆକାର(buf), "%d%s", dec, suffix);

	ଫେରନ୍ତୁ strdup(buf);
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା string_to_idarray(ଅଚଳ ଅକ୍ଷର *list, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ary[], size_t arysz,
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା (name2id)(ଅଚଳ ଅକ୍ଷର *, size_t))
{
	ଅଚଳ ଅକ୍ଷର *begin = ଖାଲି, *p;
	size_t n = 0;

	ଯଦି (!list || !*list || !ary || !arysz || !name2id)
		ଫେରନ୍ତୁ -1;

	ପାଇଁ (p = list; p && *p; p++) {
		ଅଚଳ ଅକ୍ଷର *end = ଖାଲି;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା id;

		ଯଦି (n >= arysz)
			ଫେରନ୍ତୁ -2;
		ଯଦି (!begin)
			begin = p;		/* begin of the column name */
		ଯଦି (*p == ',')
			end = p;		/* terminate the name */
		ଯଦି (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		ଯଦି (!begin || !end)
			ଜାରି;
		ଯଦି (end <= begin)
			ଫେରନ୍ତୁ -1;

		id = name2id(begin, end - begin);
		ଯଦି (id == -1)
			ଫେରନ୍ତୁ -1;
		ary[ n++ ] = id;
		begin = ଖାଲି;
		ଯଦି (end && !*end)
			ଭାଙ୍ଗ;
	}
	ଫେରନ୍ତୁ n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା string_add_to_idarray(ଅଚଳ ଅକ୍ଷର *list, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ary[], size_t arysz,
			size_t *ary_pos, ପୂର୍ଣ୍ଣସଂଖ୍ୟା (name2id)(ଅଚଳ ଅକ୍ଷର *, size_t))
{
	ଅଚଳ ଅକ୍ଷର *list_add;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା r;

	ଯଦି (!list || !*list || !ary_pos || *ary_pos > arysz)
		ଫେରନ୍ତୁ -1;

	ଯଦି (list[0] == '+')
		list_add = &list[1];
	ଅନ୍ୟଥା {
		list_add = list;
		*ary_pos = 0;
	}

	r = string_to_idarray(list_add, &ary[*ary_pos], arysz - *ary_pos, name2id);
	ଯଦି (r > 0)
		*ary_pos += r;
	ଫେରନ୍ତୁ r;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା string_to_bitarray(ଅଚଳ ଅକ୍ଷର *list,
		     ଅକ୍ଷର *ary,
		     ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*name2bit)(ଅଚଳ ଅକ୍ଷର *, size_t),
		     size_t allow_range)
{
	ଅଚଳ ଅକ୍ଷର *begin = ଖାଲି, *p;

	ଯଦି (!list || !name2bit || !ary)
		ଫେରନ୍ତୁ -EINVAL;

	ପାଇଁ (p = list; p && *p; p++) {
		ଅଚଳ ଅକ୍ଷର *end = ଖାଲି;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା bit, set_lower = 0, set_higher = 0;

		ଯଦି (!begin)
			begin = p;		/* begin of the level name */
		ଯଦି (*p == ',')
			end = p;		/* terminate the name */
		ଯଦି (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		ଯଦି (!begin || !end)
			ଜାରି;
		ଯଦି (end <= begin)
			ଫେରନ୍ତୁ -1;
		ଯଦି (allow_range) {
			ଯଦି (*(end - 1) == '+') {
				end--;
				set_lower = 1;
			} ଅନ୍ୟଥା ଯଦି (*begin == '+') {
				begin++;
				set_higher = 1;
			}
		}

		bit = name2bit(begin, end - begin);
		ଯଦି (bit < 0)
			ଫେରନ୍ତୁ bit;
		setbit(ary, bit);
		ଯଦି (set_lower)
			ଯେତେବେଳେ (--bit >= 0)
				setbit(ary, bit);
		ଅନ୍ୟଥା ଯଦି (set_higher)
			ଯେତେବେଳେ (++bit < (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) allow_range)
				setbit(ary, bit);
		begin = ଖାଲି;
		ଯଦି (end && !*end)
			ଭାଙ୍ଗ;
	}
	ଫେରନ୍ତୁ 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା string_to_bitmask(ଅଚଳ ଅକ୍ଷର *list,
		     unsigned long *mask,
		     long (*name2flag)(ଅଚଳ ଅକ୍ଷର *, size_t))
{
	ଅଚଳ ଅକ୍ଷର *begin = ଖାଲି, *p;

	ଯଦି (!list || !name2flag || !mask)
		ଫେରନ୍ତୁ -EINVAL;

	ପାଇଁ (p = list; p && *p; p++) {
		ଅଚଳ ଅକ୍ଷର *end = ଖାଲି;
		long flag;

		ଯଦି (!begin)
			begin = p;		/* begin of the level name */
		ଯଦି (*p == ',')
			end = p;		/* terminate the name */
		ଯଦି (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		ଯଦି (!begin || !end)
			ଜାରି;
		ଯଦି (end <= begin)
			ଫେରନ୍ତୁ -1;

		flag = name2flag(begin, end - begin);
		ଯଦି (flag < 0)
			ଫେରନ୍ତୁ flag;	/* error */
		*mask |= flag;
		begin = ଖାଲି;
		ଯଦି (end && !*end)
			ଭାଙ୍ଗ;
	}
	ଫେରନ୍ତୁ 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_parse_range(ଅଚଳ ଅକ୍ଷର *str, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *lower, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *upper, ପୂର୍ଣ୍ଣସଂଖ୍ୟା def)
{
	ଅକ୍ଷର *end = ଖାଲି;

	ଯଦି (!str)
		ଫେରନ୍ତୁ 0;

	*upper = *lower = def;
	errno = 0;

	ଯଦି (*str == ':') {				/* <:N> */
		str++;
		*upper = strtol(str, &end, 10);
		ଯଦି (errno || !end || *end || end == str)
			ଫେରନ୍ତୁ -1;
	} ଅନ୍ୟଥା {
		*upper = *lower = strtol(str, &end, 10);
		ଯଦି (errno || !end || end == str)
			ଫେରନ୍ତୁ -1;

		ଯଦି (*end == ':' && !*(end + 1))		/* <M:> */
			*upper = def;
		ଅନ୍ୟଥା ଯଦି (*end == '-' || *end == ':') {	/* <M:N> <M-N> */
			str = end + 1;
			end = ଖାଲି;
			errno = 0;
			*upper = strtol(str, &end, 10);

			ଯଦି (errno || !end || *end || end == str)
				ଫେରନ୍ତୁ -1;
		}
	}
	ଫେରନ୍ତୁ 0;
}

ଅଚଳ ଅକ୍ଷର *ul_path_next_segment(ଅଚଳ ଅକ୍ଷର *str, size_t *sz)
{
	ଅଚଳ ଅକ୍ଷର *start, *p;

	start = str;
	*sz = 0;
	ଯେତେବେଳେ (start && *start == '/' && *(start + 1) == '/')
		start++;

	ଯଦି (!start || !*start)
		ଫେରନ୍ତୁ ଖାଲି;

	ପାଇଁ (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	ଫେରନ୍ତୁ start;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା streq_paths(ଅଚଳ ଅକ୍ଷର *a, ଅଚଳ ଅକ୍ଷର *b)
{
	ଯେତେବେଳେ (a && b) {
		size_t a_sz, b_sz;
		ଅଚଳ ଅକ୍ଷର *a_seg = ul_path_next_segment(a, &a_sz);
		ଅଚଳ ଅକ୍ଷର *b_seg = ul_path_next_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		ଯଦି (a_sz + b_sz == 0)
			ଫେରନ୍ତୁ 1;

		/* ignore trailing slash */
		ଯଦି (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			ଫେରନ୍ତୁ 1;

		ଯଦି (!a_seg || !b_seg)
			ଭାଙ୍ଗ;
		ଯଦି (a_sz != b_sz || strncmp(a_seg, b_seg, a_sz) != 0)
			ଭାଙ୍ଗ;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	ଫେରନ୍ତୁ 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_path_is_within(ଅଚଳ ଅକ୍ଷର *path, ଅଚଳ ଅକ୍ଷର *dir)
{
	ଯଦି (!path || !dir || !*path || !*dir)
		ଫେରନ୍ତୁ 0;

	ଯେତେବେଳେ (path && dir) {
		size_t p_sz, d_sz;
		ଅଚଳ ଅକ୍ଷର *p_seg = ul_path_next_segment(path, &p_sz);
		ଅଚଳ ଅକ୍ଷର *d_seg = ul_path_next_segment(dir, &d_sz);

		/* all @dir segments matched */
		ଯଦି (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			ଫେରନ୍ତୁ 1;

		ଯଦି (!p_seg || !d_seg)
			ଭାଙ୍ଗ;
		ଯଦି (p_sz != d_sz || strncmp(p_seg, d_seg, p_sz) != 0)
			ଭାଙ୍ଗ;

		path = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	ଫେରନ୍ତୁ 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
ଅକ୍ଷର *ul_strnconcat(ଅଚଳ ଅକ୍ଷର *s, ଅଚଳ ଅକ୍ଷର *suffix, size_t b)
{
	size_t a;
	ଅକ୍ଷର *r;

	ଯଦି (!s && !suffix)
		ଫେରନ୍ତୁ strdup("");
	ଯଦି (!s)
		ଫେରନ୍ତୁ strndup(suffix, b);
	ଯଦି (!suffix)
		ଫେରନ୍ତୁ strdup(s);

	ପ୍ରତିଜ୍ଞା(s);
	ପ୍ରତିଜ୍ଞା(suffix);

	a = strlen(s);
	ଯଦି (b > ((size_t) -1) - a)
		ଫେରନ୍ତୁ ଖାଲି;

	r = malloc(a + b + 1);
	ଯଦି (!r)
		ଫେରନ୍ତୁ ଖାଲି;

	memcpy(r, s, a);
	memcpy(r + a, suffix, b);
	r[a+b] = 0;

	ଫେରନ୍ତୁ r;
}

/* concatenate two strings to a new string */
ଅକ୍ଷର *ul_strconcat(ଅଚଳ ଅକ୍ଷର *s, ଅଚଳ ଅକ୍ଷର *suffix)
{
	ଫେରନ୍ତୁ ul_strnconcat(s, suffix, suffix ? strlen(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
ଅକ୍ଷର *ul_strfconcat(ଅଚଳ ଅକ୍ଷର *s, ଅଚଳ ଅକ୍ଷର *format, ...)
{
	va_list ap;
	ଅକ୍ଷର *val, *res;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	ଯଦି (sz < 0)
		ଫେରନ୍ତୁ ଖାଲି;

	res = ul_strnconcat(s, val, sz);
	ମୁକ୍ତ(val);
	ଫେରନ୍ତୁ res;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_strappend(ଅକ୍ଷର **a, ଅଚଳ ଅକ୍ଷର *b)
{
	size_t al, bl;
	ଅକ୍ଷର *tmp;

	ଯଦି (!a)
		ଫେରନ୍ତୁ -EINVAL;
	ଯଦି (!b || !*b)
		ଫେରନ୍ତୁ 0;
	ଯଦି (!*a) {
		*a = strdup(b);
		ଫେରନ୍ତୁ !*a ? -ENOMEM : 0;
	}

	al = strlen(*a);
	bl = strlen(b);

	tmp = realloc(*a, al + bl + 1);
	ଯଦି (!tmp)
		ଫେରନ୍ତୁ -ENOMEM;
	*a = tmp;
	memcpy((*a) + al, b, bl + 1);
	ଫେରନ୍ତୁ 0;
}

/* the hybrid version of strfconcat and strappend. */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା strfappend(ଅକ୍ଷର **a, ଅଚଳ ଅକ୍ଷର *format, ...)
{
	va_list ap;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	ଫେରନ୍ତୁ res;
}

ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_strvfappend(ଅକ୍ଷର **a, ଅଚଳ ଅକ୍ଷର *format, va_list ap)
{
	ଅକ୍ଷର *val;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;

	sz = vasprintf(&val, format, ap);
	ଯଦି (sz < 0)
		ଫେରନ୍ତୁ -errno;

	res = ul_strappend(a, val);
	ମୁକ୍ତ(val);
	ଫେରନ୍ତୁ res;
}

ସ୍ଥିର size_t strcspn_escaped(ଅଚଳ ଅକ୍ଷର *s, ଅଚଳ ଅକ୍ଷର *reject)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା escaped = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;

	ପାଇଁ (n=0; s[n]; n++) {
		ଯଦି (escaped)
			escaped = 0;
		ଅନ୍ୟଥା ଯଦି (s[n] == '\\')
			escaped = 1;
		ଅନ୍ୟଥା ଯଦି (strchr(reject, s[n]))
			ଭାଙ୍ଗ;
	}

	/* if s ends in \, return index of previous char */
	ଫେରନ୍ତୁ n - escaped;
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
ଅକ୍ଷର *ul_strchr_escaped(ଅଚଳ ଅକ୍ଷର *s, ପୂର୍ଣ୍ଣସଂଖ୍ୟା c)
{
	ଅକ୍ଷର *p;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା esc = 0;

	ପାଇଁ (p = (ଅକ୍ଷର *) s; p && *p; p++) {
		ଯଦି (!esc && *p == '\\') {
			esc = 1;
			ଜାରି;
		}
		ଯଦି (*p == c && (!esc || c == '\\'))
			ଫେରନ୍ତୁ p;
		esc = 0;
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

/* Split a string into words. */
ଅଚଳ ଅକ୍ଷର *ul_split(ଅଚଳ ଅକ୍ଷର **state, size_t *l, ଅଚଳ ଅକ୍ଷର *separator, ପୂର୍ଣ୍ଣସଂଖ୍ୟା quoted)
{
	ଅଚଳ ଅକ୍ଷର *current;

	current = *state;

	ଯଦି (!*current) {
		ପ୍ରତିଜ୍ଞା(**state == '\0');
		ଫେରନ୍ତୁ ଖାଲି;
	}

	current += strspn(current, separator);
	ଯଦି (!*current) {
		*state = current;
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଯଦି (quoted && strchr("\'\"", *current)) {
		ଅକ୍ଷର quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		ଯଦି (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			ଫେରନ୍ତୁ ଖାଲି;
		}
		*state = current++ + *l + 2;
	} ଅନ୍ୟଥା ଯଦି (quoted) {
		*l = strcspn_escaped(current, separator);
		ଯଦି (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			ଫେରନ୍ତୁ ଖାଲି;
		}
		*state = current + *l;
	} ଅନ୍ୟଥା {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	ଫେରନ୍ତୁ current;
}

/* Rewind file pointer forward to new line.  */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା skip_fline(FILE *fp)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch;

	କର {
		ଯଦି ((ch = fgetc(fp)) == EOF)
			ଫେରନ୍ତୁ 1;
		ଯଦି (ch == '\n')
			ଫେରନ୍ତୁ 0;
	} ଯେତେବେଳେ (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_stralnumcmp(ଅଚଳ ଅକ୍ଷର *p1, ଅଚଳ ଅକ୍ଷର *p2)
{
	ଅଚଳ unsigned ଅକ୍ଷର *s1 = (ଅଚଳ unsigned ଅକ୍ଷର *) p1;
	ଅଚଳ unsigned ଅକ୍ଷର *s2 = (ଅଚଳ unsigned ଅକ୍ଷର *) p2;
	unsigned ଅକ୍ଷର c1, c2;

	କର {
		କର {
			c1 = (unsigned ଅକ୍ଷର) *s1++;
		} ଯେତେବେଳେ (c1 != '\0' && !isalnum((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) c1));

		କର {
			c2 = (unsigned ଅକ୍ଷର) *s2++;
		} ଯେତେବେଳେ (c2 != '\0' && !isalnum((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) c2));

		ଯଦି (c1 != '\0')
			c1 = tolower(c1);
		ଯଦି (c2 != '\0')
			c2 = tolower(c2);
		ଯଦି (c1 == '\0')
			ଫେରନ୍ତୁ c1 - c2;
	} ଯେତେବେଳେ (c1 == c2);

	ଫେରନ୍ତୁ c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_optstr_next(ଅକ୍ଷର **optstr, ଅକ୍ଷର **name, size_t *namesz,
		   ଅକ୍ଷର **ମୂଲ୍ୟ, size_t *valsz)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା open_quote = 0;
	ଅକ୍ଷର *start = ଖାଲି, *stop = ଖାଲି, *p, *sep = ଖାଲି;
	ଅକ୍ଷର *optstr0;

	ପ୍ରତିଜ୍ଞା(optstr);
	ପ୍ରତିଜ୍ଞା(*optstr);

	optstr0 = *optstr;

	ଯଦି (name)
		*name = ଖାଲି;
	ଯଦି (namesz)
		*namesz = 0;
	ଯଦି (ମୂଲ୍ୟ)
		*ମୂଲ୍ୟ = ଖାଲି;
	ଯଦି (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	ଯେତେବେଳେ (optstr0 && *optstr0 == ',')
		optstr0++;

	ପାଇଁ (p = optstr0; p && *p; p++) {
		ଯଦି (!start && *p == '=')
			ଫେରନ୍ତୁ -EINVAL;
		ଯଦି (!start)
			start = p;		/* beginning of the option item */
		ଯଦି (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		ଯଦି (open_quote)
			ଜାରି;		/* still in quoted block */
		ଯଦି (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		ଯଦି (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		ଅନ୍ୟଥା ଯଦି (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		ଯଦି (!start || !stop)
			ଜାରି;
		ଯଦି (stop <= start)
			ଫେରନ୍ତୁ -EINVAL;

		ଯଦି (name)
			*name = start;
		ଯଦି (namesz)
			*namesz = sep ? sep - start : stop - start;
		*optstr = *stop ? stop + 1 : stop;

		ଯଦି (sep) {
			ଯଦି (ମୂଲ୍ୟ)
				*ମୂଲ୍ୟ = sep + 1;
			ଯଦି (valsz)
				*valsz = stop - sep - 1;
		}
		ଫେରନ୍ତୁ 0;
	}

	ଫେରନ୍ତୁ 1;				/* end of optstr */
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_optstr_is_valid(ଅଚଳ ଅକ୍ଷର *optstr)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	ଅକ୍ଷର *p = (ଅକ୍ଷର *) optstr;

	ଯେତେବେଳେ ((rc = ul_optstr_next(&p, ଖାଲି, ଖାଲି, ଖାଲି, ଖାଲି)) == 0);
	ଫେରନ୍ତୁ rc < 0 ? 0 : 1;
}

ଅକ୍ଷର *ul_optstr_get_ମୂଲ୍ୟ(ଅଚଳ ଅକ୍ଷର *optstr, ଅଚଳ ଅକ୍ଷର *ଚାବି)
{
	size_t sz, namesz = 0, valsz = 0;
	ଅକ୍ଷର *name = ଖାଲି, *ମୂଲ୍ୟ = ଖାଲି;
	ଅକ୍ଷର *p = (ଅକ୍ଷର *) optstr;

	ଯଦି (!optstr || !ଚାବି || !*ଚାବି)
		ଫେରନ୍ତୁ ଖାଲି;

	sz = strlen(ଚାବି);
	ଯେତେବେଳେ (ul_optstr_next(&p, &name, &namesz, &ମୂଲ୍ୟ, &valsz) == 0) {
		ଯଦି (namesz != sz || !valsz)
			ଜାରି;
		ଯଦି (strncmp(name, ଚାବି, namesz) == 0)
			ଫେରନ୍ତୁ strndup(ମୂଲ୍ୟ, valsz);
	}
	ଫେରନ୍ତୁ ଖାଲି;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(ଅକ୍ଷର *str)
{
	size_t len;
	ଅକ୍ଷର q;

	ଯଦି (!str || !*str)
		ଫେରନ୍ତୁ 0;

	len = strlen(str);
	q = *str;

	ଯଦି ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		ଫେରନ୍ତୁ len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	ଫେରନ୍ତୁ len - 2;
}

#ifdef TEST_PROGRAM_STRUTILS

ସଂରଚନା testS {
	ଅକ୍ଷର *name;
	ଅକ୍ଷର *ମୂଲ୍ୟ;
};

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_strdup_to_member(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ସଂରଚନା testS *xx;

	ଯଦି (argc < 3)
		ଫେରନ୍ତୁ EXIT_FAILURE;

	xx = calloc(1, ଆକାର(*xx));
	ଯଦି (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_to_struct_member(xx, name, argv[1]);
	strdup_to_struct_member(xx, ମୂଲ୍ୟ, argv[2]);

	ଯଦି (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->ମୂଲ୍ୟ, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	printf("1: '%s', 2: '%s'\n", xx->name, xx->ମୂଲ୍ୟ);

	ମୁକ୍ତ(xx->name);
	ମୁକ୍ତ(xx->ମୂଲ୍ୟ);
	ମୁକ୍ତ(xx);
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_strutils_sizes(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	uintmax_t size = 0;
	ଅକ୍ଷର *hum1, *hum2, *hum3;

	ଯଦି (argc < 2)
		ଫେରନ୍ତୁ EXIT_FAILURE;

	ଯଦି (strtosize(argv[1], &size))
		errx(EXIT_FAILURE, "invalid size '%s' value", argv[1]);

	hum1 = size_to_human_string(SIZE_SUFFIX_1LETTER, size);
	hum2 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE, size);
	hum3 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE |
				    SIZE_DECIMAL_2DIGITS, size);

	printf("%25s : %20ju : %8s : %12s : %13s\n", argv[1], size, hum1, hum2, hum3);
	ମୁକ୍ତ(hum1);
	ମୁକ୍ତ(hum2);
	ମୁକ୍ତ(hum3);

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_strutils_cmp_paths(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = streq_paths(argv[1], argv[2]);

	ଯଦି (argc < 3)
		ଫେରନ୍ତୁ EXIT_FAILURE;

	printf("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_strutils_path_is_within(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	ଯଦି (argc < 3)
		ଫେରନ୍ତୁ EXIT_FAILURE;

	rc = ul_path_is_within(argv[1], argv[2]);

	printf("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_strutils_normalize(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	unsigned ଅକ୍ଷର *src, *dst, *org;
	size_t sz, len;

	ଯଦି (argc < 2)
		ଫେରନ୍ତୁ EXIT_FAILURE;

	org = (unsigned ଅକ୍ଷର *) strdup(argv[1]);
	src = (unsigned ଅକ୍ଷର *) strdup((ଅକ୍ଷର *) org);
	len = strlen((ଅକ୍ଷର *) src);
	dst = malloc(len + 1);

	ଯଦି (!org || !src || !dst)
		ଯାଅ done;

	/* two buffers */
	sz = __normalize_whitespace(src, len, dst, len + 1);
	printf("1: '%s' --> '%s' [sz=%zu]\n", src, dst, sz);

	/* one buffer */
	sz = normalize_whitespace(src);
	printf("2: '%s' --> '%s' [sz=%zu]\n", org, src, sz);

done:
	ମୁକ୍ତ(src);
	ମୁକ୍ତ(dst);
	ମୁକ୍ତ(org);

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_strutils_cstrcasecmp(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ଅକ୍ଷର *a, *b;

	ଯଦି (argc < 3)
		ଫେରନ୍ତୁ EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	ଯଦି (!a || !b)
		ଫେରନ୍ତୁ EXIT_FAILURE;

	printf("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	printf("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	printf("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, strlen(a)));

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ଯଦି (argc == 3 && strcmp(argv[1], "--size") == 0) {
		ଫେରନ୍ତୁ test_strutils_sizes(argc - 1, argv + 1);

	} ଅନ୍ୟଥା ଯଦି (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		printf("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						ଖାଲି));
		ଫେରନ୍ତୁ EXIT_SUCCESS;
	} ଅନ୍ୟଥା ଯଦି (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		ଯଦି (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		printf("'%s'-->%s\n", argv[2], val ? "true" : "false");
		ଫେରନ୍ତୁ EXIT_SUCCESS;

	} ଅନ୍ୟଥା ଯଦି (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		ଫେରନ୍ତୁ test_strutils_cmp_paths(argc - 1, argv + 1);

	} ଅନ୍ୟଥା ଯଦି (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		ଫେରନ୍ତୁ test_strutils_path_is_within(argc - 1, argv + 1);

	} ଅନ୍ୟଥା ଯଦି (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		ଫେରନ୍ତୁ test_strdup_to_member(argc - 1, argv + 1);

	} ଅନ୍ୟଥା ଯଦି  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		printf("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		ଫେରନ୍ତୁ EXIT_SUCCESS;

	} ଅନ୍ୟଥା ଯଦି (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		ଫେରନ୍ତୁ test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} ଅନ୍ୟଥା ଯଦି (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		ଫେରନ୍ତୁ test_strutils_normalize(argc - 1, argv + 1);

	} ଅନ୍ୟଥା ଯଦି (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		printf("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		ଫେରନ୍ତୁ EXIT_SUCCESS;
	} ଅନ୍ୟଥା ଯଦି (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		printf("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		ଫେରନ୍ତୁ EXIT_SUCCESS;
	} ଅନ୍ୟଥା ଯଦି (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		printf("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		ଫେରନ୍ତୁ EXIT_SUCCESS;
	} ଅନ୍ୟଥା ଯଦି (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		printf("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		ଫେରନ୍ତୁ EXIT_SUCCESS;
	} ଅନ୍ୟଥା ଯଦି (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		printf("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		ଫେରନ୍ତୁ EXIT_SUCCESS;
	} ଅନ୍ୟଥା ଯଦି (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		printf("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		ଫେରନ୍ତୁ EXIT_SUCCESS;

	} ଅନ୍ୟଥା ଯଦି (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		printf("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		ଫେରନ୍ତୁ EXIT_SUCCESS;

	} ଅନ୍ୟଥା ଯଦି (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		ଅକ୍ଷର *buf = "abc\0Y\0\0xyz\0X";
		ଅକ୍ଷର *end = buf + 12;
		ଅକ୍ଷର *p = buf;

		କର {
			printf("str: '%s'\n", p);
		} ଯେତେବେଳେ ((p = ul_next_string(p, end)));

		ଫେରନ୍ତୁ EXIT_SUCCESS;

	} ଅନ୍ୟଥା ଯଦି (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		ଅକ୍ଷର *name = ଖାଲି, *val = ଖାଲି;
		ଅକ୍ଷର *p = argv[2];
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

		ଯଦି (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		ଯେତେବେଳେ ((rc = ul_optstr_next(&p, &name, &namesz, &val, &valsz)) == 0) {
			printf("'%.*s' : '%.*s'\n", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) namesz, name,
						   (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) valsz, val);
		}
		ଯଦି (rc == 1)
			ଫେରନ୍ତୁ EXIT_SUCCESS;
	} ଅନ୍ୟଥା ଯଦି (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		ଅକ୍ଷର *unquoted = strdup(argv[2]);

		ଯଦି (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		printf("%s-->%s\n", argv[2], unquoted);
		ମୁକ୍ତ(unquoted);
		ଫେରନ୍ତୁ EXIT_SUCCESS;

	} ଅନ୍ୟଥା {
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

	ଫେରନ୍ତୁ EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_STRUTILS */
