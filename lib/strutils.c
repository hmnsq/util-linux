/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Karel Zak <kzak@redhat.com> [2010]
 *          Davidlohr Bueso <dave@gnu.org> [2010]
 */
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

#include "c.h"
#include "cctype.h"
#include "nls.h"
#include "strutils.h"
#include "timeutils.h"
#include "bitops.h"
#include "pathnames.h"

static puur_naa_nka STRTOXX_EXIT_CODE = EXIT_FAILURE;

void strutils_set_exitcode(puur_naa_nka ex)
{
	STRTOXX_EXIT_CODE = ex;
}

static puur_naa_nka do_scale_by_power (uintmax_t *x, puur_naa_nka base, puur_naa_nka power)
{
	ᮞᮨᮓᮨᮜ (power--) {
		ᮚᮦᮔ᮪ (UINTMAX_MAX / base < *x)
			ᮝᮜᮤ -ERANGE;
		*x *= base;
	}
	ᮝᮜᮤ 0;
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
puur_naa_nka ul_parse_size(const var_na *str, uintmax_t *res, puur_naa_nka *power)
{
	const var_na *p;
	var_na *end;
	uintmax_t x, frac = 0;
	puur_naa_nka base = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	static const var_na *suf  = "KMGTPEZY";
	static const var_na *suf2 = "kmgtpezy";
	const var_na *sp;

	*res = 0;

	ᮚᮦᮔ᮪ (!str || !*str) {
		rc = -EINVAL;
		goto err;
	}

	/* Only positive numbers are acceptable
	 *
	 * Note that this check is not perfect, it would be better to
	 * use lconv->negative_sign. But coreutils use the same solution,
	 * so it's probably good enough...
	 */
	p = str;
	ᮞᮨᮓᮨᮜ (isspace((unsigned var_na) *p))
		p++;
	ᮚᮦᮔ᮪ (*p == '-') {
		rc = -EINVAL;
		goto err;
	}

	errno = 0, end = NULL;
	x = strtoumax(str, &end, 0);

	ᮚᮦᮔ᮪ (end == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		goto err;
	}
	ᮚᮦᮔ᮪ (!end || !*end)
		goto done;			/* without suffix */
	p = end;

	/*
	 * Check size suffixes
	 */
check_suffix:
	ᮚᮦᮔ᮪ (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		base = 1024;			/* XiB, 2^N */
	ᮜᮤᮚ ᮚᮦᮔ᮪ ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		base = 1000;			/* XB, 10^N */
	ᮜᮤᮚ ᮚᮦᮔ᮪ (*(p + 1)) {
		sa_nracanaa lconv const *l = localeconv();
		const var_na *dp = l ? l->decimal_point : NULL;
		size_t dpsz = dp ? strlen(dp) : 0;

		ᮚᮦᮔ᮪ (frac == 0 && *p && dp && strncmp(dp, p, dpsz) == 0) {
			const var_na *fstr = p + dpsz;

			for (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			ᮚᮦᮔ᮪ (isdigit(*fstr)) {
				errno = 0, end = NULL;
				frac = strtoumax(fstr, &end, 0);
				ᮚᮦᮔ᮪ (end == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					goto err;
				}
			} ᮜᮤᮚ
				end = (var_na *) p;

			ᮚᮦᮔ᮪ (!end || !*end) {
				rc = -EINVAL;
				goto err;	/* without suffix, but with fractions */
			}
			p = end;
			goto check_suffix;
		}
		rc = -EINVAL;
		goto err;			/* unexpected suffix */
	}

	sp = strchr(suf, *p);
	ᮚᮦᮔ᮪ (sp)
		pwr = (sp - suf) + 1;
	ᮜᮤᮚ {
		sp = strchr(suf2, *p);
		ᮚᮦᮔ᮪ (sp)
			pwr = (sp - suf2) + 1;
		ᮜᮤᮚ {
			rc = -EINVAL;
			goto err;
		}
	}

	ᮚᮦᮔ᮪ (power)
		*power = pwr;

	rc = do_scale_by_power(&x, base, pwr);
	ᮚᮦᮔ᮪ (rc)
		goto err;

	ᮚᮦᮔ᮪ (frac && pwr) {
		puur_naa_nka i;
		uintmax_t frac_base = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_base, base, pwr);
		ᮚᮦᮔ᮪ (rc)
			goto err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		do {
			frac_val = ((frac % 10) * frac_base + frac_val) / 10;
			frac /= 10;
		} ᮞᮨᮓᮨᮜ (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		for (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		ᮚᮦᮔ᮪ (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			goto err;
		}
		x += frac_val;
	}
done:
	*res = x;
err:
	ᮚᮦᮔ᮪ (rc < 0)
		errno = -rc;
	ᮝᮜᮤ rc;
}

puur_naa_nka strtosize(const var_na *str, uintmax_t *res)
{
	ᮝᮜᮤ ul_parse_size(str, res, NULL);
}

puur_naa_nka isdigit_strend(const var_na *str, const var_na **end)
{
	const var_na *p;

	for (p = str; p && *p && isdigit((unsigned var_na) *p); p++);

	ᮚᮦᮔ᮪ (end)
		*end = p;
	ᮝᮜᮤ p && p > str && !*p;
}

puur_naa_nka isxdigit_strend(const var_na *str, const var_na **end)
{
	const var_na *p;

	for (p = str; p && *p && isxdigit((unsigned var_na) *p); p++);

	ᮚᮦᮔ᮪ (end)
		*end = p;

	ᮝᮜᮤ p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
puur_naa_nka ul_strtobool(const var_na *str, bool *result)
{
	static const var_na *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	static const var_na *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	ᮚᮦᮔ᮪ (!str || !result)
		ᮝᮜᮤ -(errno = EINVAL);

	errno = 0;

	for (i = 0; i < ARRAY_SIZE(bool_true); i++) {
		ᮚᮦᮔ᮪ (c_strcasecmp(str, bool_true[i]) == 0) {
			*result = true;
			ᮝᮜᮤ 0;
		}
	}

	for (i = 0; i < ARRAY_SIZE(bool_false); i++) {
		ᮚᮦᮔ᮪ (c_strcasecmp(str, bool_false[i]) == 0) {
			*result = false;
			ᮝᮜᮤ 0;
		}
	}

	ᮝᮜᮤ -(errno = EINVAL);
}

puur_naa_nka ul_parse_switch(const var_na *arg, ...)
{
	const var_na *a, *b;
	va_list ap;

	va_start(ap, arg);
	do {
		a = va_arg(ap, var_na *);
		ᮚᮦᮔ᮪ (!a)
			bha_nga;
		b = va_arg(ap, var_na *);
		ᮚᮦᮔ᮪ (!b)
			bha_nga;

		ᮚᮦᮔ᮪ (strcmp(arg, a) == 0) {
			va_end(ap);
			ᮝᮜᮤ 1;
		}

		ᮚᮦᮔ᮪ (strcmp(arg, b) == 0) {
			va_end(ap);
			ᮝᮜᮤ 0;
		}
	} ᮞᮨᮓᮨᮜ (1);
	va_end(ap);

	errx(STRTOXX_EXIT_CODE, _("unsupported argument: %s"), arg);
}

#ifndef HAVE_MEMPCPY
void *mempcpy(void *restrict dest, const void *restrict src, size_t n)
{
	ᮝᮜᮤ ((var_na *)memcpy(dest, src, n)) + n;
}
#endif

#ifndef HAVE_STRNLEN
size_t strnlen(const var_na *s, size_t maxlen)
{
	size_t i;

	for (i = 0; i < maxlen; i++) {
		ᮚᮦᮔ᮪ (s[i] == '\0')
			ᮝᮜᮤ i;
	}
	ᮝᮜᮤ maxlen;
}
#endif

#ifndef HAVE_STRNCHR
var_na *strnchr(const var_na *s, size_t maxlen, puur_naa_nka c)
{
	for (; maxlen-- && *s != '\0'; ++s)
		ᮚᮦᮔ᮪ (*s == (var_na)c)
			ᮝᮜᮤ (var_na *)s;
	ᮝᮜᮤ NULL;
}
#endif

#ifndef HAVE_STRNDUP
var_na *strndup(const var_na *s, size_t n)
{
	size_t len = strnlen(s, n);
	var_na *new = malloc((len + 1) * parimaa_na(var_na));
	ᮚᮦᮔ᮪ (!new)
		ᮝᮜᮤ NULL;
	new[len] = '\0';
	ᮝᮜᮤ (var_na *) memcpy(new, s, len);
}
#endif

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
puur_naa_nka ul_strtos64(const var_na *str, int64_t *num, puur_naa_nka base)
{
	var_na *end = NULL;

	ᮚᮦᮔ᮪ (str == NULL || *str == '\0')
		ᮝᮜᮤ -(errno = EINVAL);

	errno = 0;
	*num = (int64_t) strtoimax(str, &end, base);

	ᮚᮦᮔ᮪ (errno != 0)
		ᮝᮜᮤ -errno;
	ᮚᮦᮔ᮪ (str == end || (end && *end))
		ᮝᮜᮤ -(errno = EINVAL);
	ᮝᮜᮤ 0;
}

puur_naa_nka ul_strtou64(const var_na *str, uint64_t *num, puur_naa_nka base)
{
	var_na *end = NULL;
	int64_t tmp;

	ᮚᮦᮔ᮪ (str == NULL || *str == '\0')
		ᮝᮜᮤ -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &end, base);
	ᮚᮦᮔ᮪ (tmp < 0)
		errno = ERANGE;
	ᮜᮤᮚ {
		errno = 0;
		*num = strtoumax(str, &end, base);
	}

	ᮚᮦᮔ᮪ (errno != 0)
		ᮝᮜᮤ -errno;
	ᮚᮦᮔ᮪ (str == end || (end && *end))
		ᮝᮜᮤ -(errno = EINVAL);
	ᮝᮜᮤ 0;
}

puur_naa_nka ul_strtos32(const var_na *str, int32_t *num, puur_naa_nka base)
{
	int64_t tmp;
	puur_naa_nka rc;

	rc = ul_strtos64(str, &tmp, base);
	ᮚᮦᮔ᮪ (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	ᮚᮦᮔ᮪ (rc == 0)
		*num = (int32_t) tmp;
	ᮝᮜᮤ rc;
}

puur_naa_nka ul_strtou32(const var_na *str, uint32_t *num, puur_naa_nka base)
{
	uint64_t tmp;
	puur_naa_nka rc;

	rc = ul_strtou64(str, &tmp, base);
	ᮚᮦᮔ᮪ (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	ᮚᮦᮔ᮪ (rc == 0)
		*num = (uint32_t) tmp;
	ᮝᮜᮤ rc;
}

puur_naa_nka ul_strtou16(const var_na *str, uint16_t *num, puur_naa_nka base)
{
	uint64_t tmp;
	puur_naa_nka rc;

	rc = ul_strtou64(str, &tmp, base);
	ᮚᮦᮔ᮪ (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	ᮚᮦᮔ᮪ (rc == 0)
		*num = (uint16_t) tmp;
	ᮝᮜᮤ rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(const var_na *str, puur_naa_nka base, const var_na *errmesg,
			     int64_t low, int64_t up)
{
	int64_t num = 0;
	puur_naa_nka rc;

	rc = ul_strtos64(str, &num, base);
	ᮚᮦᮔ᮪ (rc == 0 && ((low && num < low) || (up && num > up)))
		rc = -(errno = ERANGE);

	ᮚᮦᮔ᮪ (rc) {
		ᮚᮦᮔ᮪ (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	ᮝᮜᮤ num;
}

uint64_t str2unum_or_err(const var_na *str, puur_naa_nka base, const var_na *errmesg, uint64_t up)
{
	uint64_t num = 0;
	puur_naa_nka rc;

	rc = ul_strtou64(str, &num, base);
	ᮚᮦᮔ᮪ (rc == 0 && (up && num > up))
		rc = -(errno = ERANGE);

	ᮚᮦᮔ᮪ (rc) {
		ᮚᮦᮔ᮪ (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	ᮝᮜᮤ num;
}

dviguu_na strtod_or_err(const var_na *str, const var_na *errmesg)
{
	dviguu_na num;
	var_na *end = NULL;

	errno = 0;
	ᮚᮦᮔ᮪ (str == NULL || *str == '\0')
		goto err;
	num = strtod(str, &end);

	ᮚᮦᮔ᮪ (errno || str == end || (end && *end))
		goto err;

	ᮝᮜᮤ num;
err:
	ᮚᮦᮔ᮪ (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

puur_naa_nka ul_strtold(const var_na *str, long dviguu_na *num)
{
	var_na *end = NULL;

	errno = 0;
	ᮚᮦᮔ᮪ (str == NULL || *str == '\0')
		ᮝᮜᮤ -(errno = EINVAL);
	*num = strtold(str, &end);

	ᮚᮦᮔ᮪ (errno != 0)
		ᮝᮜᮤ -errno;
	ᮚᮦᮔ᮪ (str == end || (end && *end))
		ᮝᮜᮤ -(errno = EINVAL);
	ᮝᮜᮤ 0;
}

long dviguu_na strtold_or_err(const var_na *str, const var_na *errmesg)
{
	long dviguu_na num = 0;

	ᮚᮦᮔ᮪ (ul_strtold(str, &num) == 0)
		ᮝᮜᮤ num;
	ᮚᮦᮔ᮪ (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(const var_na *str, const var_na *errmesg)
{
	uintmax_t num;

	ᮚᮦᮔ᮪ (strtosize(str, &num) == 0)
		ᮝᮜᮤ num;

	ᮚᮦᮔ᮪ (errno)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}


void strtotimeval_or_err(const var_na *str, sa_nracanaa timeval *tv, const var_na *errmesg)
{
	long dviguu_na user_input;

	user_input = strtold_or_err(str, errmesg);
	tv->tv_sec = (time_t) user_input;
	tv->tv_usec = (suseconds_t)((user_input - tv->tv_sec) * USEC_PER_SEC);
}

void strtotimespec_or_err(const var_na *str, sa_nracanaa timespec *ts, const var_na *errmesg)
{
	long dviguu_na user_input;

	user_input = strtold_or_err(str, errmesg);
	ts->tv_sec = (time_t) user_input;
	ts->tv_nsec = (long)((user_input - ts->tv_sec) * NSEC_PER_SEC);
}

time_t strtotime_or_err(const var_na *str, const var_na *errmesg)
{
	int64_t user_input;

	user_input = strtos64_or_err(str, errmesg);
	ᮝᮜᮤ (time_t) user_input;
}

bool hyperlinkwanted(const var_na *mode)
{
	ᮚᮦᮔ᮪ (mode && strcmp(mode, "never") == 0)
		ᮝᮜᮤ false;

	ᮚᮦᮔ᮪ (mode && strcmp(mode, "always") == 0)
		ᮝᮜᮤ true;

	ᮚᮦᮔ᮪ (!mode || strcmp(mode, "auto") == 0)
		ᮝᮜᮤ isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), mode);
}

bool annotationwanted(const var_na *mode)
{
	ᮚᮦᮔ᮪ (mode && strcmp(mode, "never") == 0)
		ᮝᮜᮤ false;

	ᮚᮦᮔ᮪ (mode && strcmp(mode, "always") == 0)
		ᮝᮜᮤ true;

	ᮚᮦᮔ᮪ (!mode || strcmp(mode, "auto") == 0)
		ᮝᮜᮤ isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), mode);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
var_na *xstrmode(mode_t mode, var_na *str)
{
	unsigned short i = 0;

	ᮚᮦᮔ᮪ (S_ISDIR(mode))
		str[i++] = 'd';
	ᮜᮤᮚ ᮚᮦᮔ᮪ (S_ISLNK(mode))
		str[i++] = 'l';
	ᮜᮤᮚ ᮚᮦᮔ᮪ (S_ISCHR(mode))
		str[i++] = 'c';
	ᮜᮤᮚ ᮚᮦᮔ᮪ (S_ISBLK(mode))
		str[i++] = 'b';
	ᮜᮤᮚ ᮚᮦᮔ᮪ (S_ISSOCK(mode))
		str[i++] = 's';
	ᮜᮤᮚ ᮚᮦᮔ᮪ (S_ISFIFO(mode))
		str[i++] = 'p';
	ᮜᮤᮚ ᮚᮦᮔ᮪ (S_ISREG(mode))
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

	ᮝᮜᮤ str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
static puur_naa_nka get_exp(uint64_t n)
{
	puur_naa_nka shft;

	for (shft = 10; shft <= 60; shft += 10) {
		ᮚᮦᮔ᮪ (n < (1ULL << shft))
			bha_nga;
	}
	ᮝᮜᮤ shft - 10;
}

var_na *size_to_human_string(puur_naa_nka options, uint64_t bytes)
{
	var_na buf[32];
	puur_naa_nka dec, exp;
	uint64_t frac;
	const var_na *letters = "BKMGTPE";
	var_na suffix[parimaa_na(" KiB")], *psuf = suffix;
	var_na c;

	ᮚᮦᮔ᮪ (options & SIZE_SUFFIX_SPACE)
		*psuf++ = ' ';


	exp  = get_exp(bytes);
	c    = *(letters + (exp ? exp / 10 : 0));
	dec  = exp ? bytes / (1ULL << exp) : bytes;
	frac = exp ? bytes % (1ULL << exp) : 0;

	*psuf++ = c;

	ᮚᮦᮔ᮪ ((options & SIZE_SUFFIX_3LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/* round */
	ᮚᮦᮔ᮪ (frac) {
		/* get 3 digits after decimal point */
		ᮚᮦᮔ᮪ (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (exp - 10)) ;
		ᮜᮤᮚ
			frac = (frac * 1000) / (1ULL << (exp)) ;

		ᮚᮦᮔ᮪ (options & SIZE_DECIMAL_2DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} ᮜᮤᮚ {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		ᮚᮦᮔ᮪ (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	ᮚᮦᮔ᮪ (frac) {
		sa_nracanaa lconv const *l = localeconv();
		var_na *dp = l ? l->decimal_point : NULL;
		puur_naa_nka len;

		ᮚᮦᮔ᮪ (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, parimaa_na(buf), "%d%s%02" PRIu64, dec, dp, frac);
		ᮚᮦᮔ᮪ (len > 0 && (size_t) len < parimaa_na(buf)) {
			/* remove potential extraneous zero */
			ᮚᮦᮔ᮪ (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, parimaa_na(buf) - len);
		} ᮜᮤᮚ
			*buf = '\0';	/* snprintf error */
	} ᮜᮤᮚ
		snprintf(buf, parimaa_na(buf), "%d%s", dec, suffix);

	ᮝᮜᮤ strdup(buf);
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
puur_naa_nka string_to_idarray(const var_na *list, puur_naa_nka ary[], size_t arysz,
			puur_naa_nka (name2id)(const var_na *, size_t))
{
	const var_na *begin = NULL, *p;
	size_t n = 0;

	ᮚᮦᮔ᮪ (!list || !*list || !ary || !arysz || !name2id)
		ᮝᮜᮤ -1;

	for (p = list; p && *p; p++) {
		const var_na *end = NULL;
		puur_naa_nka id;

		ᮚᮦᮔ᮪ (n >= arysz)
			ᮝᮜᮤ -2;
		ᮚᮦᮔ᮪ (!begin)
			begin = p;		/* begin of the column name */
		ᮚᮦᮔ᮪ (*p == ',')
			end = p;		/* terminate the name */
		ᮚᮦᮔ᮪ (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		ᮚᮦᮔ᮪ (!begin || !end)
			continue;
		ᮚᮦᮔ᮪ (end <= begin)
			ᮝᮜᮤ -1;

		id = name2id(begin, end - begin);
		ᮚᮦᮔ᮪ (id == -1)
			ᮝᮜᮤ -1;
		ary[ n++ ] = id;
		begin = NULL;
		ᮚᮦᮔ᮪ (end && !*end)
			bha_nga;
	}
	ᮝᮜᮤ n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
puur_naa_nka string_add_to_idarray(const var_na *list, puur_naa_nka ary[], size_t arysz,
			size_t *ary_pos, puur_naa_nka (name2id)(const var_na *, size_t))
{
	const var_na *list_add;
	puur_naa_nka r;

	ᮚᮦᮔ᮪ (!list || !*list || !ary_pos || *ary_pos > arysz)
		ᮝᮜᮤ -1;

	ᮚᮦᮔ᮪ (list[0] == '+')
		list_add = &list[1];
	ᮜᮤᮚ {
		list_add = list;
		*ary_pos = 0;
	}

	r = string_to_idarray(list_add, &ary[*ary_pos], arysz - *ary_pos, name2id);
	ᮚᮦᮔ᮪ (r > 0)
		*ary_pos += r;
	ᮝᮜᮤ r;
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
puur_naa_nka string_to_bitarray(const var_na *list,
		     var_na *ary,
		     puur_naa_nka (*name2bit)(const var_na *, size_t),
		     size_t allow_range)
{
	const var_na *begin = NULL, *p;

	ᮚᮦᮔ᮪ (!list || !name2bit || !ary)
		ᮝᮜᮤ -EINVAL;

	for (p = list; p && *p; p++) {
		const var_na *end = NULL;
		puur_naa_nka bit, set_lower = 0, set_higher = 0;

		ᮚᮦᮔ᮪ (!begin)
			begin = p;		/* begin of the level name */
		ᮚᮦᮔ᮪ (*p == ',')
			end = p;		/* terminate the name */
		ᮚᮦᮔ᮪ (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		ᮚᮦᮔ᮪ (!begin || !end)
			continue;
		ᮚᮦᮔ᮪ (end <= begin)
			ᮝᮜᮤ -1;
		ᮚᮦᮔ᮪ (allow_range) {
			ᮚᮦᮔ᮪ (*(end - 1) == '+') {
				end--;
				set_lower = 1;
			} ᮜᮤᮚ ᮚᮦᮔ᮪ (*begin == '+') {
				begin++;
				set_higher = 1;
			}
		}

		bit = name2bit(begin, end - begin);
		ᮚᮦᮔ᮪ (bit < 0)
			ᮝᮜᮤ bit;
		setbit(ary, bit);
		ᮚᮦᮔ᮪ (set_lower)
			ᮞᮨᮓᮨᮜ (--bit >= 0)
				setbit(ary, bit);
		ᮜᮤᮚ ᮚᮦᮔ᮪ (set_higher)
			ᮞᮨᮓᮨᮜ (++bit < (puur_naa_nka) allow_range)
				setbit(ary, bit);
		begin = NULL;
		ᮚᮦᮔ᮪ (end && !*end)
			bha_nga;
	}
	ᮝᮜᮤ 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
puur_naa_nka string_to_bitmask(const var_na *list,
		     unsigned long *mask,
		     long (*name2flag)(const var_na *, size_t))
{
	const var_na *begin = NULL, *p;

	ᮚᮦᮔ᮪ (!list || !name2flag || !mask)
		ᮝᮜᮤ -EINVAL;

	for (p = list; p && *p; p++) {
		const var_na *end = NULL;
		long flag;

		ᮚᮦᮔ᮪ (!begin)
			begin = p;		/* begin of the level name */
		ᮚᮦᮔ᮪ (*p == ',')
			end = p;		/* terminate the name */
		ᮚᮦᮔ᮪ (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		ᮚᮦᮔ᮪ (!begin || !end)
			continue;
		ᮚᮦᮔ᮪ (end <= begin)
			ᮝᮜᮤ -1;

		flag = name2flag(begin, end - begin);
		ᮚᮦᮔ᮪ (flag < 0)
			ᮝᮜᮤ flag;	/* error */
		*mask |= flag;
		begin = NULL;
		ᮚᮦᮔ᮪ (end && !*end)
			bha_nga;
	}
	ᮝᮜᮤ 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
puur_naa_nka ul_parse_range(const var_na *str, puur_naa_nka *lower, puur_naa_nka *upper, puur_naa_nka def)
{
	var_na *end = NULL;

	ᮚᮦᮔ᮪ (!str)
		ᮝᮜᮤ 0;

	*upper = *lower = def;
	errno = 0;

	ᮚᮦᮔ᮪ (*str == ':') {				/* <:N> */
		str++;
		*upper = strtol(str, &end, 10);
		ᮚᮦᮔ᮪ (errno || !end || *end || end == str)
			ᮝᮜᮤ -1;
	} ᮜᮤᮚ {
		*upper = *lower = strtol(str, &end, 10);
		ᮚᮦᮔ᮪ (errno || !end || end == str)
			ᮝᮜᮤ -1;

		ᮚᮦᮔ᮪ (*end == ':' && !*(end + 1))		/* <M:> */
			*upper = def;
		ᮜᮤᮚ ᮚᮦᮔ᮪ (*end == '-' || *end == ':') {	/* <M:N> <M-N> */
			str = end + 1;
			end = NULL;
			errno = 0;
			*upper = strtol(str, &end, 10);

			ᮚᮦᮔ᮪ (errno || !end || *end || end == str)
				ᮝᮜᮤ -1;
		}
	}
	ᮝᮜᮤ 0;
}

const var_na *ul_path_next_segment(const var_na *str, size_t *sz)
{
	const var_na *start, *p;

	start = str;
	*sz = 0;
	ᮞᮨᮓᮨᮜ (start && *start == '/' && *(start + 1) == '/')
		start++;

	ᮚᮦᮔ᮪ (!start || !*start)
		ᮝᮜᮤ NULL;

	for (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	ᮝᮜᮤ start;
}

puur_naa_nka streq_paths(const var_na *a, const var_na *b)
{
	ᮞᮨᮓᮨᮜ (a && b) {
		size_t a_sz, b_sz;
		const var_na *a_seg = ul_path_next_segment(a, &a_sz);
		const var_na *b_seg = ul_path_next_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		ᮚᮦᮔ᮪ (a_sz + b_sz == 0)
			ᮝᮜᮤ 1;

		/* ignore trailing slash */
		ᮚᮦᮔ᮪ (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			ᮝᮜᮤ 1;

		ᮚᮦᮔ᮪ (!a_seg || !b_seg)
			bha_nga;
		ᮚᮦᮔ᮪ (a_sz != b_sz || strncmp(a_seg, b_seg, a_sz) != 0)
			bha_nga;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	ᮝᮜᮤ 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
puur_naa_nka ul_path_is_within(const var_na *path, const var_na *dir)
{
	ᮚᮦᮔ᮪ (!path || !dir || !*path || !*dir)
		ᮝᮜᮤ 0;

	ᮞᮨᮓᮨᮜ (path && dir) {
		size_t p_sz, d_sz;
		const var_na *p_seg = ul_path_next_segment(path, &p_sz);
		const var_na *d_seg = ul_path_next_segment(dir, &d_sz);

		/* all @dir segments matched */
		ᮚᮦᮔ᮪ (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			ᮝᮜᮤ 1;

		ᮚᮦᮔ᮪ (!p_seg || !d_seg)
			bha_nga;
		ᮚᮦᮔ᮪ (p_sz != d_sz || strncmp(p_seg, d_seg, p_sz) != 0)
			bha_nga;

		path = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	ᮝᮜᮤ 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
var_na *ul_strnconcat(const var_na *s, const var_na *suffix, size_t b)
{
	size_t a;
	var_na *r;

	ᮚᮦᮔ᮪ (!s && !suffix)
		ᮝᮜᮤ strdup("");
	ᮚᮦᮔ᮪ (!s)
		ᮝᮜᮤ strndup(suffix, b);
	ᮚᮦᮔ᮪ (!suffix)
		ᮝᮜᮤ strdup(s);

	assert(s);
	assert(suffix);

	a = strlen(s);
	ᮚᮦᮔ᮪ (b > ((size_t) -1) - a)
		ᮝᮜᮤ NULL;

	r = malloc(a + b + 1);
	ᮚᮦᮔ᮪ (!r)
		ᮝᮜᮤ NULL;

	memcpy(r, s, a);
	memcpy(r + a, suffix, b);
	r[a+b] = 0;

	ᮝᮜᮤ r;
}

/* concatenate two strings to a new string */
var_na *ul_strconcat(const var_na *s, const var_na *suffix)
{
	ᮝᮜᮤ ul_strnconcat(s, suffix, suffix ? strlen(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
var_na *ul_strfconcat(const var_na *s, const var_na *format, ...)
{
	va_list ap;
	var_na *val, *res;
	puur_naa_nka sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	ᮚᮦᮔ᮪ (sz < 0)
		ᮝᮜᮤ NULL;

	res = ul_strnconcat(s, val, sz);
	free(val);
	ᮝᮜᮤ res;
}

puur_naa_nka ul_strappend(var_na **a, const var_na *b)
{
	size_t al, bl;
	var_na *tmp;

	ᮚᮦᮔ᮪ (!a)
		ᮝᮜᮤ -EINVAL;
	ᮚᮦᮔ᮪ (!b || !*b)
		ᮝᮜᮤ 0;
	ᮚᮦᮔ᮪ (!*a) {
		*a = strdup(b);
		ᮝᮜᮤ !*a ? -ENOMEM : 0;
	}

	al = strlen(*a);
	bl = strlen(b);

	tmp = realloc(*a, al + bl + 1);
	ᮚᮦᮔ᮪ (!tmp)
		ᮝᮜᮤ -ENOMEM;
	*a = tmp;
	memcpy((*a) + al, b, bl + 1);
	ᮝᮜᮤ 0;
}

/* the hybrid version of strfconcat and strappend. */
puur_naa_nka strfappend(var_na **a, const var_na *format, ...)
{
	va_list ap;
	puur_naa_nka res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	ᮝᮜᮤ res;
}

extern puur_naa_nka ul_strvfappend(var_na **a, const var_na *format, va_list ap)
{
	var_na *val;
	puur_naa_nka sz;
	puur_naa_nka res;

	sz = vasprintf(&val, format, ap);
	ᮚᮦᮔ᮪ (sz < 0)
		ᮝᮜᮤ -errno;

	res = ul_strappend(a, val);
	free(val);
	ᮝᮜᮤ res;
}

static size_t strcspn_escaped(const var_na *s, const var_na *reject)
{
	puur_naa_nka escaped = 0;
	puur_naa_nka n;

	for (n=0; s[n]; n++) {
		ᮚᮦᮔ᮪ (escaped)
			escaped = 0;
		ᮜᮤᮚ ᮚᮦᮔ᮪ (s[n] == '\\')
			escaped = 1;
		ᮜᮤᮚ ᮚᮦᮔ᮪ (strchr(reject, s[n]))
			bha_nga;
	}

	/* if s ends in \, return index of previous char */
	ᮝᮜᮤ n - escaped;
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
var_na *ul_strchr_escaped(const var_na *s, puur_naa_nka c)
{
	var_na *p;
	puur_naa_nka esc = 0;

	for (p = (var_na *) s; p && *p; p++) {
		ᮚᮦᮔ᮪ (!esc && *p == '\\') {
			esc = 1;
			continue;
		}
		ᮚᮦᮔ᮪ (*p == c && (!esc || c == '\\'))
			ᮝᮜᮤ p;
		esc = 0;
	}

	ᮝᮜᮤ NULL;
}

/* Split a string into words. */
const var_na *ul_split(const var_na **state, size_t *l, const var_na *separator, puur_naa_nka quoted)
{
	const var_na *current;

	current = *state;

	ᮚᮦᮔ᮪ (!*current) {
		assert(**state == '\0');
		ᮝᮜᮤ NULL;
	}

	current += strspn(current, separator);
	ᮚᮦᮔ᮪ (!*current) {
		*state = current;
		ᮝᮜᮤ NULL;
	}

	ᮚᮦᮔ᮪ (quoted && strchr("\'\"", *current)) {
		var_na quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		ᮚᮦᮔ᮪ (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			ᮝᮜᮤ NULL;
		}
		*state = current++ + *l + 2;
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (quoted) {
		*l = strcspn_escaped(current, separator);
		ᮚᮦᮔ᮪ (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			ᮝᮜᮤ NULL;
		}
		*state = current + *l;
	} ᮜᮤᮚ {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	ᮝᮜᮤ current;
}

/* Rewind file pointer forward to new line.  */
puur_naa_nka skip_fline(FILE *fp)
{
	puur_naa_nka ch;

	do {
		ᮚᮦᮔ᮪ ((ch = fgetc(fp)) == EOF)
			ᮝᮜᮤ 1;
		ᮚᮦᮔ᮪ (ch == '\n')
			ᮝᮜᮤ 0;
	} ᮞᮨᮓᮨᮜ (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
puur_naa_nka ul_stralnumcmp(const var_na *p1, const var_na *p2)
{
	const unsigned var_na *s1 = (const unsigned var_na *) p1;
	const unsigned var_na *s2 = (const unsigned var_na *) p2;
	unsigned var_na c1, c2;

	do {
		do {
			c1 = (unsigned var_na) *s1++;
		} ᮞᮨᮓᮨᮜ (c1 != '\0' && !isalnum((unsigned puur_naa_nka) c1));

		do {
			c2 = (unsigned var_na) *s2++;
		} ᮞᮨᮓᮨᮜ (c2 != '\0' && !isalnum((unsigned puur_naa_nka) c2));

		ᮚᮦᮔ᮪ (c1 != '\0')
			c1 = tolower(c1);
		ᮚᮦᮔ᮪ (c2 != '\0')
			c2 = tolower(c2);
		ᮚᮦᮔ᮪ (c1 == '\0')
			ᮝᮜᮤ c1 - c2;
	} ᮞᮨᮓᮨᮜ (c1 == c2);

	ᮝᮜᮤ c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
puur_naa_nka ul_optstr_next(var_na **optstr, var_na **name, size_t *namesz,
		   var_na **value, size_t *valsz)
{
	puur_naa_nka open_quote = 0;
	var_na *start = NULL, *stop = NULL, *p, *sep = NULL;
	var_na *optstr0;

	assert(optstr);
	assert(*optstr);

	optstr0 = *optstr;

	ᮚᮦᮔ᮪ (name)
		*name = NULL;
	ᮚᮦᮔ᮪ (namesz)
		*namesz = 0;
	ᮚᮦᮔ᮪ (value)
		*value = NULL;
	ᮚᮦᮔ᮪ (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	ᮞᮨᮓᮨᮜ (optstr0 && *optstr0 == ',')
		optstr0++;

	for (p = optstr0; p && *p; p++) {
		ᮚᮦᮔ᮪ (!start && *p == '=')
			ᮝᮜᮤ -EINVAL;
		ᮚᮦᮔ᮪ (!start)
			start = p;		/* beginning of the option item */
		ᮚᮦᮔ᮪ (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		ᮚᮦᮔ᮪ (open_quote)
			continue;		/* still in quoted block */
		ᮚᮦᮔ᮪ (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		ᮚᮦᮔ᮪ (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		ᮜᮤᮚ ᮚᮦᮔ᮪ (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		ᮚᮦᮔ᮪ (!start || !stop)
			continue;
		ᮚᮦᮔ᮪ (stop <= start)
			ᮝᮜᮤ -EINVAL;

		ᮚᮦᮔ᮪ (name)
			*name = start;
		ᮚᮦᮔ᮪ (namesz)
			*namesz = sep ? sep - start : stop - start;
		*optstr = *stop ? stop + 1 : stop;

		ᮚᮦᮔ᮪ (sep) {
			ᮚᮦᮔ᮪ (value)
				*value = sep + 1;
			ᮚᮦᮔ᮪ (valsz)
				*valsz = stop - sep - 1;
		}
		ᮝᮜᮤ 0;
	}

	ᮝᮜᮤ 1;				/* end of optstr */
}

puur_naa_nka ul_optstr_is_valid(const var_na *optstr)
{
	puur_naa_nka rc;
	var_na *p = (var_na *) optstr;

	ᮞᮨᮓᮨᮜ ((rc = ul_optstr_next(&p, NULL, NULL, NULL, NULL)) == 0);
	ᮝᮜᮤ rc < 0 ? 0 : 1;
}

var_na *ul_optstr_get_value(const var_na *optstr, const var_na *key)
{
	size_t sz, namesz = 0, valsz = 0;
	var_na *name = NULL, *value = NULL;
	var_na *p = (var_na *) optstr;

	ᮚᮦᮔ᮪ (!optstr || !key || !*key)
		ᮝᮜᮤ NULL;

	sz = strlen(key);
	ᮞᮨᮓᮨᮜ (ul_optstr_next(&p, &name, &namesz, &value, &valsz) == 0) {
		ᮚᮦᮔ᮪ (namesz != sz || !valsz)
			continue;
		ᮚᮦᮔ᮪ (strncmp(name, key, namesz) == 0)
			ᮝᮜᮤ strndup(value, valsz);
	}
	ᮝᮜᮤ NULL;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(var_na *str)
{
	size_t len;
	var_na q;

	ᮚᮦᮔ᮪ (!str || !*str)
		ᮝᮜᮤ 0;

	len = strlen(str);
	q = *str;

	ᮚᮦᮔ᮪ ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		ᮝᮜᮤ len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	ᮝᮜᮤ len - 2;
}

#ifdef TEST_PROGRAM_STRUTILS

sa_nracanaa testS {
	var_na *name;
	var_na *value;
};

static puur_naa_nka test_strdup_to_member(puur_naa_nka argc, var_na *argv[])
{
	sa_nracanaa testS *xx;

	ᮚᮦᮔ᮪ (argc < 3)
		ᮝᮜᮤ EXIT_FAILURE;

	xx = calloc(1, parimaa_na(*xx));
	ᮚᮦᮔ᮪ (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_to_struct_member(xx, name, argv[1]);
	strdup_to_struct_member(xx, value, argv[2]);

	ᮚᮦᮔ᮪ (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->value, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	printf("1: '%s', 2: '%s'\n", xx->name, xx->value);

	free(xx->name);
	free(xx->value);
	free(xx);
	ᮝᮜᮤ EXIT_SUCCESS;
}

static puur_naa_nka test_strutils_sizes(puur_naa_nka argc, var_na *argv[])
{
	uintmax_t size = 0;
	var_na *hum1, *hum2, *hum3;

	ᮚᮦᮔ᮪ (argc < 2)
		ᮝᮜᮤ EXIT_FAILURE;

	ᮚᮦᮔ᮪ (strtosize(argv[1], &size))
		errx(EXIT_FAILURE, "invalid size '%s' value", argv[1]);

	hum1 = size_to_human_string(SIZE_SUFFIX_1LETTER, size);
	hum2 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE, size);
	hum3 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE |
				    SIZE_DECIMAL_2DIGITS, size);

	printf("%25s : %20ju : %8s : %12s : %13s\n", argv[1], size, hum1, hum2, hum3);
	free(hum1);
	free(hum2);
	free(hum3);

	ᮝᮜᮤ EXIT_SUCCESS;
}

static puur_naa_nka test_strutils_cmp_paths(puur_naa_nka argc, var_na *argv[])
{
	puur_naa_nka rc = streq_paths(argv[1], argv[2]);

	ᮚᮦᮔ᮪ (argc < 3)
		ᮝᮜᮤ EXIT_FAILURE;

	printf("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	ᮝᮜᮤ EXIT_SUCCESS;
}

static puur_naa_nka test_strutils_path_is_within(puur_naa_nka argc, var_na *argv[])
{
	puur_naa_nka rc;

	ᮚᮦᮔ᮪ (argc < 3)
		ᮝᮜᮤ EXIT_FAILURE;

	rc = ul_path_is_within(argv[1], argv[2]);

	printf("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	ᮝᮜᮤ EXIT_SUCCESS;
}

static puur_naa_nka test_strutils_normalize(puur_naa_nka argc, var_na *argv[])
{
	unsigned var_na *src, *dst, *org;
	size_t sz, len;

	ᮚᮦᮔ᮪ (argc < 2)
		ᮝᮜᮤ EXIT_FAILURE;

	org = (unsigned var_na *) strdup(argv[1]);
	src = (unsigned var_na *) strdup((var_na *) org);
	len = strlen((var_na *) src);
	dst = malloc(len + 1);

	ᮚᮦᮔ᮪ (!org || !src || !dst)
		goto done;

	/* two buffers */
	sz = __normalize_whitespace(src, len, dst, len + 1);
	printf("1: '%s' --> '%s' [sz=%zu]\n", src, dst, sz);

	/* one buffer */
	sz = normalize_whitespace(src);
	printf("2: '%s' --> '%s' [sz=%zu]\n", org, src, sz);

done:
	free(src);
	free(dst);
	free(org);

	ᮝᮜᮤ EXIT_SUCCESS;
}

static puur_naa_nka test_strutils_cstrcasecmp(puur_naa_nka argc, var_na *argv[])
{
	var_na *a, *b;

	ᮚᮦᮔ᮪ (argc < 3)
		ᮝᮜᮤ EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	ᮚᮦᮔ᮪ (!a || !b)
		ᮝᮜᮤ EXIT_FAILURE;

	printf("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	printf("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	printf("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, strlen(a)));

	ᮝᮜᮤ EXIT_SUCCESS;
}

puur_naa_nka main(puur_naa_nka argc, var_na *argv[])
{
	ᮚᮦᮔ᮪ (argc == 3 && strcmp(argv[1], "--size") == 0) {
		ᮝᮜᮤ test_strutils_sizes(argc - 1, argv + 1);

	} ᮜᮤᮚ ᮚᮦᮔ᮪ (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		printf("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						NULL));
		ᮝᮜᮤ EXIT_SUCCESS;
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		ᮚᮦᮔ᮪ (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		printf("'%s'-->%s\n", argv[2], val ? "true" : "false");
		ᮝᮜᮤ EXIT_SUCCESS;

	} ᮜᮤᮚ ᮚᮦᮔ᮪ (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		ᮝᮜᮤ test_strutils_cmp_paths(argc - 1, argv + 1);

	} ᮜᮤᮚ ᮚᮦᮔ᮪ (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		ᮝᮜᮤ test_strutils_path_is_within(argc - 1, argv + 1);

	} ᮜᮤᮚ ᮚᮦᮔ᮪ (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		ᮝᮜᮤ test_strdup_to_member(argc - 1, argv + 1);

	} ᮜᮤᮚ ᮚᮦᮔ᮪  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		printf("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		ᮝᮜᮤ EXIT_SUCCESS;

	} ᮜᮤᮚ ᮚᮦᮔ᮪ (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		ᮝᮜᮤ test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} ᮜᮤᮚ ᮚᮦᮔ᮪ (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		ᮝᮜᮤ test_strutils_normalize(argc - 1, argv + 1);

	} ᮜᮤᮚ ᮚᮦᮔ᮪ (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		printf("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		ᮝᮜᮤ EXIT_SUCCESS;
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		printf("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		ᮝᮜᮤ EXIT_SUCCESS;
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		printf("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		ᮝᮜᮤ EXIT_SUCCESS;
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		printf("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		ᮝᮜᮤ EXIT_SUCCESS;
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		printf("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		ᮝᮜᮤ EXIT_SUCCESS;
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		printf("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		ᮝᮜᮤ EXIT_SUCCESS;

	} ᮜᮤᮚ ᮚᮦᮔ᮪ (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		printf("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		ᮝᮜᮤ EXIT_SUCCESS;

	} ᮜᮤᮚ ᮚᮦᮔ᮪ (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		var_na *buf = "abc\0Y\0\0xyz\0X";
		var_na *end = buf + 12;
		var_na *p = buf;

		do {
			printf("str: '%s'\n", p);
		} ᮞᮨᮓᮨᮜ ((p = ul_next_string(p, end)));

		ᮝᮜᮤ EXIT_SUCCESS;

	} ᮜᮤᮚ ᮚᮦᮔ᮪ (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		var_na *name = NULL, *val = NULL;
		var_na *p = argv[2];
		puur_naa_nka rc;

		ᮚᮦᮔ᮪ (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		ᮞᮨᮓᮨᮜ ((rc = ul_optstr_next(&p, &name, &namesz, &val, &valsz)) == 0) {
			printf("'%.*s' : '%.*s'\n", (puur_naa_nka) namesz, name,
						   (puur_naa_nka) valsz, val);
		}
		ᮚᮦᮔ᮪ (rc == 1)
			ᮝᮜᮤ EXIT_SUCCESS;
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		var_na *unquoted = strdup(argv[2]);

		ᮚᮦᮔ᮪ (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		printf("%s-->%s\n", argv[2], unquoted);
		free(unquoted);
		ᮝᮜᮤ EXIT_SUCCESS;

	} ᮜᮤᮚ {
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

	ᮝᮜᮤ EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_STRUTILS */
