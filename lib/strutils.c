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
	ᯀᯬᯝ (power--) {
		ᯀᯮᯝ (UINTMAX_MAX / base < *x)
			ᯀᯮᯞ -ERANGE;
		*x *= base;
	}
	ᯀᯮᯞ 0;
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

	ᯀᯮᯝ (!str || !*str) {
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
	ᯀᯬᯝ (isspace((unsigned var_na) *p))
		p++;
	ᯀᯮᯝ (*p == '-') {
		rc = -EINVAL;
		goto err;
	}

	errno = 0, end = NULL;
	x = strtoumax(str, &end, 0);

	ᯀᯮᯝ (end == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		goto err;
	}
	ᯀᯮᯝ (!end || !*end)
		goto done;			/* without suffix */
	p = end;

	/*
	 * Check size suffixes
	 */
check_suffix:
	ᯀᯮᯝ (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		base = 1024;			/* XiB, 2^N */
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		base = 1000;			/* XB, 10^N */
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (*(p + 1)) {
		sa_nracanaa lconv const *l = localeconv();
		const var_na *dp = l ? l->decimal_point : NULL;
		size_t dpsz = dp ? strlen(dp) : 0;

		ᯀᯮᯝ (frac == 0 && *p && dp && strncmp(dp, p, dpsz) == 0) {
			const var_na *fstr = p + dpsz;

			for (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			ᯀᯮᯝ (isdigit(*fstr)) {
				errno = 0, end = NULL;
				frac = strtoumax(fstr, &end, 0);
				ᯀᯮᯝ (end == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					goto err;
				}
			} ᯀᯮᯝᯂᯪ
				end = (var_na *) p;

			ᯀᯮᯝ (!end || !*end) {
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
	ᯀᯮᯝ (sp)
		pwr = (sp - suf) + 1;
	ᯀᯮᯝᯂᯪ {
		sp = strchr(suf2, *p);
		ᯀᯮᯝ (sp)
			pwr = (sp - suf2) + 1;
		ᯀᯮᯝᯂᯪ {
			rc = -EINVAL;
			goto err;
		}
	}

	ᯀᯮᯝ (power)
		*power = pwr;

	rc = do_scale_by_power(&x, base, pwr);
	ᯀᯮᯝ (rc)
		goto err;

	ᯀᯮᯝ (frac && pwr) {
		puur_naa_nka i;
		uintmax_t frac_base = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_base, base, pwr);
		ᯀᯮᯝ (rc)
			goto err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		do {
			frac_val = ((frac % 10) * frac_base + frac_val) / 10;
			frac /= 10;
		} ᯀᯬᯝ (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		for (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		ᯀᯮᯝ (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			goto err;
		}
		x += frac_val;
	}
done:
	*res = x;
err:
	ᯀᯮᯝ (rc < 0)
		errno = -rc;
	ᯀᯮᯞ rc;
}

puur_naa_nka strtosize(const var_na *str, uintmax_t *res)
{
	ᯀᯮᯞ ul_parse_size(str, res, NULL);
}

puur_naa_nka isdigit_strend(const var_na *str, const var_na **end)
{
	const var_na *p;

	for (p = str; p && *p && isdigit((unsigned var_na) *p); p++);

	ᯀᯮᯝ (end)
		*end = p;
	ᯀᯮᯞ p && p > str && !*p;
}

puur_naa_nka isxdigit_strend(const var_na *str, const var_na **end)
{
	const var_na *p;

	for (p = str; p && *p && isxdigit((unsigned var_na) *p); p++);

	ᯀᯮᯝ (end)
		*end = p;

	ᯀᯮᯞ p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
puur_naa_nka ul_strtobool(const var_na *str, bool *result)
{
	static const var_na *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	static const var_na *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	ᯀᯮᯝ (!str || !result)
		ᯀᯮᯞ -(errno = EINVAL);

	errno = 0;

	for (i = 0; i < ARRAY_SIZE(bool_true); i++) {
		ᯀᯮᯝ (c_strcasecmp(str, bool_true[i]) == 0) {
			*result = true;
			ᯀᯮᯞ 0;
		}
	}

	for (i = 0; i < ARRAY_SIZE(bool_false); i++) {
		ᯀᯮᯝ (c_strcasecmp(str, bool_false[i]) == 0) {
			*result = false;
			ᯀᯮᯞ 0;
		}
	}

	ᯀᯮᯞ -(errno = EINVAL);
}

puur_naa_nka ul_parse_switch(const var_na *arg, ...)
{
	const var_na *a, *b;
	va_list ap;

	va_start(ap, arg);
	do {
		a = va_arg(ap, var_na *);
		ᯀᯮᯝ (!a)
			bha_nga;
		b = va_arg(ap, var_na *);
		ᯀᯮᯝ (!b)
			bha_nga;

		ᯀᯮᯝ (strcmp(arg, a) == 0) {
			va_end(ap);
			ᯀᯮᯞ 1;
		}

		ᯀᯮᯝ (strcmp(arg, b) == 0) {
			va_end(ap);
			ᯀᯮᯞ 0;
		}
	} ᯀᯬᯝ (1);
	va_end(ap);

	errx(STRTOXX_EXIT_CODE, _("unsupported argument: %s"), arg);
}

#ifndef HAVE_MEMPCPY
void *mempcpy(void *restrict dest, const void *restrict src, size_t n)
{
	ᯀᯮᯞ ((var_na *)memcpy(dest, src, n)) + n;
}
#endif

#ifndef HAVE_STRNLEN
size_t strnlen(const var_na *s, size_t maxlen)
{
	size_t i;

	for (i = 0; i < maxlen; i++) {
		ᯀᯮᯝ (s[i] == '\0')
			ᯀᯮᯞ i;
	}
	ᯀᯮᯞ maxlen;
}
#endif

#ifndef HAVE_STRNCHR
var_na *strnchr(const var_na *s, size_t maxlen, puur_naa_nka c)
{
	for (; maxlen-- && *s != '\0'; ++s)
		ᯀᯮᯝ (*s == (var_na)c)
			ᯀᯮᯞ (var_na *)s;
	ᯀᯮᯞ NULL;
}
#endif

#ifndef HAVE_STRNDUP
var_na *strndup(const var_na *s, size_t n)
{
	size_t len = strnlen(s, n);
	var_na *new = malloc((len + 1) * parimaa_na(var_na));
	ᯀᯮᯝ (!new)
		ᯀᯮᯞ NULL;
	new[len] = '\0';
	ᯀᯮᯞ (var_na *) memcpy(new, s, len);
}
#endif

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
puur_naa_nka ul_strtos64(const var_na *str, int64_t *num, puur_naa_nka base)
{
	var_na *end = NULL;

	ᯀᯮᯝ (str == NULL || *str == '\0')
		ᯀᯮᯞ -(errno = EINVAL);

	errno = 0;
	*num = (int64_t) strtoimax(str, &end, base);

	ᯀᯮᯝ (errno != 0)
		ᯀᯮᯞ -errno;
	ᯀᯮᯝ (str == end || (end && *end))
		ᯀᯮᯞ -(errno = EINVAL);
	ᯀᯮᯞ 0;
}

puur_naa_nka ul_strtou64(const var_na *str, uint64_t *num, puur_naa_nka base)
{
	var_na *end = NULL;
	int64_t tmp;

	ᯀᯮᯝ (str == NULL || *str == '\0')
		ᯀᯮᯞ -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &end, base);
	ᯀᯮᯝ (tmp < 0)
		errno = ERANGE;
	ᯀᯮᯝᯂᯪ {
		errno = 0;
		*num = strtoumax(str, &end, base);
	}

	ᯀᯮᯝ (errno != 0)
		ᯀᯮᯞ -errno;
	ᯀᯮᯝ (str == end || (end && *end))
		ᯀᯮᯞ -(errno = EINVAL);
	ᯀᯮᯞ 0;
}

puur_naa_nka ul_strtos32(const var_na *str, int32_t *num, puur_naa_nka base)
{
	int64_t tmp;
	puur_naa_nka rc;

	rc = ul_strtos64(str, &tmp, base);
	ᯀᯮᯝ (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	ᯀᯮᯝ (rc == 0)
		*num = (int32_t) tmp;
	ᯀᯮᯞ rc;
}

puur_naa_nka ul_strtou32(const var_na *str, uint32_t *num, puur_naa_nka base)
{
	uint64_t tmp;
	puur_naa_nka rc;

	rc = ul_strtou64(str, &tmp, base);
	ᯀᯮᯝ (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	ᯀᯮᯝ (rc == 0)
		*num = (uint32_t) tmp;
	ᯀᯮᯞ rc;
}

puur_naa_nka ul_strtou16(const var_na *str, uint16_t *num, puur_naa_nka base)
{
	uint64_t tmp;
	puur_naa_nka rc;

	rc = ul_strtou64(str, &tmp, base);
	ᯀᯮᯝ (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	ᯀᯮᯝ (rc == 0)
		*num = (uint16_t) tmp;
	ᯀᯮᯞ rc;
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
	ᯀᯮᯝ (rc == 0 && ((low && num < low) || (up && num > up)))
		rc = -(errno = ERANGE);

	ᯀᯮᯝ (rc) {
		ᯀᯮᯝ (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	ᯀᯮᯞ num;
}

uint64_t str2unum_or_err(const var_na *str, puur_naa_nka base, const var_na *errmesg, uint64_t up)
{
	uint64_t num = 0;
	puur_naa_nka rc;

	rc = ul_strtou64(str, &num, base);
	ᯀᯮᯝ (rc == 0 && (up && num > up))
		rc = -(errno = ERANGE);

	ᯀᯮᯝ (rc) {
		ᯀᯮᯝ (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	ᯀᯮᯞ num;
}

dviguu_na strtod_or_err(const var_na *str, const var_na *errmesg)
{
	dviguu_na num;
	var_na *end = NULL;

	errno = 0;
	ᯀᯮᯝ (str == NULL || *str == '\0')
		goto err;
	num = strtod(str, &end);

	ᯀᯮᯝ (errno || str == end || (end && *end))
		goto err;

	ᯀᯮᯞ num;
err:
	ᯀᯮᯝ (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

puur_naa_nka ul_strtold(const var_na *str, long dviguu_na *num)
{
	var_na *end = NULL;

	errno = 0;
	ᯀᯮᯝ (str == NULL || *str == '\0')
		ᯀᯮᯞ -(errno = EINVAL);
	*num = strtold(str, &end);

	ᯀᯮᯝ (errno != 0)
		ᯀᯮᯞ -errno;
	ᯀᯮᯝ (str == end || (end && *end))
		ᯀᯮᯞ -(errno = EINVAL);
	ᯀᯮᯞ 0;
}

long dviguu_na strtold_or_err(const var_na *str, const var_na *errmesg)
{
	long dviguu_na num = 0;

	ᯀᯮᯝ (ul_strtold(str, &num) == 0)
		ᯀᯮᯞ num;
	ᯀᯮᯝ (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(const var_na *str, const var_na *errmesg)
{
	uintmax_t num;

	ᯀᯮᯝ (strtosize(str, &num) == 0)
		ᯀᯮᯞ num;

	ᯀᯮᯝ (errno)
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
	ᯀᯮᯞ (time_t) user_input;
}

bool hyperlinkwanted(const var_na *mode)
{
	ᯀᯮᯝ (mode && strcmp(mode, "never") == 0)
		ᯀᯮᯞ false;

	ᯀᯮᯝ (mode && strcmp(mode, "always") == 0)
		ᯀᯮᯞ true;

	ᯀᯮᯝ (!mode || strcmp(mode, "auto") == 0)
		ᯀᯮᯞ isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), mode);
}

bool annotationwanted(const var_na *mode)
{
	ᯀᯮᯝ (mode && strcmp(mode, "never") == 0)
		ᯀᯮᯞ false;

	ᯀᯮᯝ (mode && strcmp(mode, "always") == 0)
		ᯀᯮᯞ true;

	ᯀᯮᯝ (!mode || strcmp(mode, "auto") == 0)
		ᯀᯮᯞ isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), mode);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
var_na *xstrmode(mode_t mode, var_na *str)
{
	unsigned short i = 0;

	ᯀᯮᯝ (S_ISDIR(mode))
		str[i++] = 'd';
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (S_ISLNK(mode))
		str[i++] = 'l';
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (S_ISCHR(mode))
		str[i++] = 'c';
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (S_ISBLK(mode))
		str[i++] = 'b';
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (S_ISSOCK(mode))
		str[i++] = 's';
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (S_ISFIFO(mode))
		str[i++] = 'p';
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (S_ISREG(mode))
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

	ᯀᯮᯞ str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
static puur_naa_nka get_exp(uint64_t n)
{
	puur_naa_nka shft;

	for (shft = 10; shft <= 60; shft += 10) {
		ᯀᯮᯝ (n < (1ULL << shft))
			bha_nga;
	}
	ᯀᯮᯞ shft - 10;
}

var_na *size_to_human_string(puur_naa_nka options, uint64_t bytes)
{
	var_na buf[32];
	puur_naa_nka dec, exp;
	uint64_t frac;
	const var_na *letters = "BKMGTPE";
	var_na suffix[parimaa_na(" KiB")], *psuf = suffix;
	var_na c;

	ᯀᯮᯝ (options & SIZE_SUFFIX_SPACE)
		*psuf++ = ' ';


	exp  = get_exp(bytes);
	c    = *(letters + (exp ? exp / 10 : 0));
	dec  = exp ? bytes / (1ULL << exp) : bytes;
	frac = exp ? bytes % (1ULL << exp) : 0;

	*psuf++ = c;

	ᯀᯮᯝ ((options & SIZE_SUFFIX_3LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/* round */
	ᯀᯮᯝ (frac) {
		/* get 3 digits after decimal point */
		ᯀᯮᯝ (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (exp - 10)) ;
		ᯀᯮᯝᯂᯪ
			frac = (frac * 1000) / (1ULL << (exp)) ;

		ᯀᯮᯝ (options & SIZE_DECIMAL_2DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} ᯀᯮᯝᯂᯪ {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		ᯀᯮᯝ (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	ᯀᯮᯝ (frac) {
		sa_nracanaa lconv const *l = localeconv();
		var_na *dp = l ? l->decimal_point : NULL;
		puur_naa_nka len;

		ᯀᯮᯝ (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, parimaa_na(buf), "%d%s%02" PRIu64, dec, dp, frac);
		ᯀᯮᯝ (len > 0 && (size_t) len < parimaa_na(buf)) {
			/* remove potential extraneous zero */
			ᯀᯮᯝ (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, parimaa_na(buf) - len);
		} ᯀᯮᯝᯂᯪ
			*buf = '\0';	/* snprintf error */
	} ᯀᯮᯝᯂᯪ
		snprintf(buf, parimaa_na(buf), "%d%s", dec, suffix);

	ᯀᯮᯞ strdup(buf);
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

	ᯀᯮᯝ (!list || !*list || !ary || !arysz || !name2id)
		ᯀᯮᯞ -1;

	for (p = list; p && *p; p++) {
		const var_na *end = NULL;
		puur_naa_nka id;

		ᯀᯮᯝ (n >= arysz)
			ᯀᯮᯞ -2;
		ᯀᯮᯝ (!begin)
			begin = p;		/* begin of the column name */
		ᯀᯮᯝ (*p == ',')
			end = p;		/* terminate the name */
		ᯀᯮᯝ (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		ᯀᯮᯝ (!begin || !end)
			continue;
		ᯀᯮᯝ (end <= begin)
			ᯀᯮᯞ -1;

		id = name2id(begin, end - begin);
		ᯀᯮᯝ (id == -1)
			ᯀᯮᯞ -1;
		ary[ n++ ] = id;
		begin = NULL;
		ᯀᯮᯝ (end && !*end)
			bha_nga;
	}
	ᯀᯮᯞ n;
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

	ᯀᯮᯝ (!list || !*list || !ary_pos || *ary_pos > arysz)
		ᯀᯮᯞ -1;

	ᯀᯮᯝ (list[0] == '+')
		list_add = &list[1];
	ᯀᯮᯝᯂᯪ {
		list_add = list;
		*ary_pos = 0;
	}

	r = string_to_idarray(list_add, &ary[*ary_pos], arysz - *ary_pos, name2id);
	ᯀᯮᯝ (r > 0)
		*ary_pos += r;
	ᯀᯮᯞ r;
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

	ᯀᯮᯝ (!list || !name2bit || !ary)
		ᯀᯮᯞ -EINVAL;

	for (p = list; p && *p; p++) {
		const var_na *end = NULL;
		puur_naa_nka bit, set_lower = 0, set_higher = 0;

		ᯀᯮᯝ (!begin)
			begin = p;		/* begin of the level name */
		ᯀᯮᯝ (*p == ',')
			end = p;		/* terminate the name */
		ᯀᯮᯝ (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		ᯀᯮᯝ (!begin || !end)
			continue;
		ᯀᯮᯝ (end <= begin)
			ᯀᯮᯞ -1;
		ᯀᯮᯝ (allow_range) {
			ᯀᯮᯝ (*(end - 1) == '+') {
				end--;
				set_lower = 1;
			} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (*begin == '+') {
				begin++;
				set_higher = 1;
			}
		}

		bit = name2bit(begin, end - begin);
		ᯀᯮᯝ (bit < 0)
			ᯀᯮᯞ bit;
		setbit(ary, bit);
		ᯀᯮᯝ (set_lower)
			ᯀᯬᯝ (--bit >= 0)
				setbit(ary, bit);
		ᯀᯮᯝᯂᯪ ᯀᯮᯝ (set_higher)
			ᯀᯬᯝ (++bit < (puur_naa_nka) allow_range)
				setbit(ary, bit);
		begin = NULL;
		ᯀᯮᯝ (end && !*end)
			bha_nga;
	}
	ᯀᯮᯞ 0;
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

	ᯀᯮᯝ (!list || !name2flag || !mask)
		ᯀᯮᯞ -EINVAL;

	for (p = list; p && *p; p++) {
		const var_na *end = NULL;
		long flag;

		ᯀᯮᯝ (!begin)
			begin = p;		/* begin of the level name */
		ᯀᯮᯝ (*p == ',')
			end = p;		/* terminate the name */
		ᯀᯮᯝ (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		ᯀᯮᯝ (!begin || !end)
			continue;
		ᯀᯮᯝ (end <= begin)
			ᯀᯮᯞ -1;

		flag = name2flag(begin, end - begin);
		ᯀᯮᯝ (flag < 0)
			ᯀᯮᯞ flag;	/* error */
		*mask |= flag;
		begin = NULL;
		ᯀᯮᯝ (end && !*end)
			bha_nga;
	}
	ᯀᯮᯞ 0;
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

	ᯀᯮᯝ (!str)
		ᯀᯮᯞ 0;

	*upper = *lower = def;
	errno = 0;

	ᯀᯮᯝ (*str == ':') {				/* <:N> */
		str++;
		*upper = strtol(str, &end, 10);
		ᯀᯮᯝ (errno || !end || *end || end == str)
			ᯀᯮᯞ -1;
	} ᯀᯮᯝᯂᯪ {
		*upper = *lower = strtol(str, &end, 10);
		ᯀᯮᯝ (errno || !end || end == str)
			ᯀᯮᯞ -1;

		ᯀᯮᯝ (*end == ':' && !*(end + 1))		/* <M:> */
			*upper = def;
		ᯀᯮᯝᯂᯪ ᯀᯮᯝ (*end == '-' || *end == ':') {	/* <M:N> <M-N> */
			str = end + 1;
			end = NULL;
			errno = 0;
			*upper = strtol(str, &end, 10);

			ᯀᯮᯝ (errno || !end || *end || end == str)
				ᯀᯮᯞ -1;
		}
	}
	ᯀᯮᯞ 0;
}

const var_na *ul_path_next_segment(const var_na *str, size_t *sz)
{
	const var_na *start, *p;

	start = str;
	*sz = 0;
	ᯀᯬᯝ (start && *start == '/' && *(start + 1) == '/')
		start++;

	ᯀᯮᯝ (!start || !*start)
		ᯀᯮᯞ NULL;

	for (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	ᯀᯮᯞ start;
}

puur_naa_nka streq_paths(const var_na *a, const var_na *b)
{
	ᯀᯬᯝ (a && b) {
		size_t a_sz, b_sz;
		const var_na *a_seg = ul_path_next_segment(a, &a_sz);
		const var_na *b_seg = ul_path_next_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		ᯀᯮᯝ (a_sz + b_sz == 0)
			ᯀᯮᯞ 1;

		/* ignore trailing slash */
		ᯀᯮᯝ (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			ᯀᯮᯞ 1;

		ᯀᯮᯝ (!a_seg || !b_seg)
			bha_nga;
		ᯀᯮᯝ (a_sz != b_sz || strncmp(a_seg, b_seg, a_sz) != 0)
			bha_nga;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	ᯀᯮᯞ 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
puur_naa_nka ul_path_is_within(const var_na *path, const var_na *dir)
{
	ᯀᯮᯝ (!path || !dir || !*path || !*dir)
		ᯀᯮᯞ 0;

	ᯀᯬᯝ (path && dir) {
		size_t p_sz, d_sz;
		const var_na *p_seg = ul_path_next_segment(path, &p_sz);
		const var_na *d_seg = ul_path_next_segment(dir, &d_sz);

		/* all @dir segments matched */
		ᯀᯮᯝ (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			ᯀᯮᯞ 1;

		ᯀᯮᯝ (!p_seg || !d_seg)
			bha_nga;
		ᯀᯮᯝ (p_sz != d_sz || strncmp(p_seg, d_seg, p_sz) != 0)
			bha_nga;

		path = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	ᯀᯮᯞ 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
var_na *ul_strnconcat(const var_na *s, const var_na *suffix, size_t b)
{
	size_t a;
	var_na *r;

	ᯀᯮᯝ (!s && !suffix)
		ᯀᯮᯞ strdup("");
	ᯀᯮᯝ (!s)
		ᯀᯮᯞ strndup(suffix, b);
	ᯀᯮᯝ (!suffix)
		ᯀᯮᯞ strdup(s);

	assert(s);
	assert(suffix);

	a = strlen(s);
	ᯀᯮᯝ (b > ((size_t) -1) - a)
		ᯀᯮᯞ NULL;

	r = malloc(a + b + 1);
	ᯀᯮᯝ (!r)
		ᯀᯮᯞ NULL;

	memcpy(r, s, a);
	memcpy(r + a, suffix, b);
	r[a+b] = 0;

	ᯀᯮᯞ r;
}

/* concatenate two strings to a new string */
var_na *ul_strconcat(const var_na *s, const var_na *suffix)
{
	ᯀᯮᯞ ul_strnconcat(s, suffix, suffix ? strlen(suffix) : 0);
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

	ᯀᯮᯝ (sz < 0)
		ᯀᯮᯞ NULL;

	res = ul_strnconcat(s, val, sz);
	free(val);
	ᯀᯮᯞ res;
}

puur_naa_nka ul_strappend(var_na **a, const var_na *b)
{
	size_t al, bl;
	var_na *tmp;

	ᯀᯮᯝ (!a)
		ᯀᯮᯞ -EINVAL;
	ᯀᯮᯝ (!b || !*b)
		ᯀᯮᯞ 0;
	ᯀᯮᯝ (!*a) {
		*a = strdup(b);
		ᯀᯮᯞ !*a ? -ENOMEM : 0;
	}

	al = strlen(*a);
	bl = strlen(b);

	tmp = realloc(*a, al + bl + 1);
	ᯀᯮᯝ (!tmp)
		ᯀᯮᯞ -ENOMEM;
	*a = tmp;
	memcpy((*a) + al, b, bl + 1);
	ᯀᯮᯞ 0;
}

/* the hybrid version of strfconcat and strappend. */
puur_naa_nka strfappend(var_na **a, const var_na *format, ...)
{
	va_list ap;
	puur_naa_nka res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	ᯀᯮᯞ res;
}

extern puur_naa_nka ul_strvfappend(var_na **a, const var_na *format, va_list ap)
{
	var_na *val;
	puur_naa_nka sz;
	puur_naa_nka res;

	sz = vasprintf(&val, format, ap);
	ᯀᯮᯝ (sz < 0)
		ᯀᯮᯞ -errno;

	res = ul_strappend(a, val);
	free(val);
	ᯀᯮᯞ res;
}

static size_t strcspn_escaped(const var_na *s, const var_na *reject)
{
	puur_naa_nka escaped = 0;
	puur_naa_nka n;

	for (n=0; s[n]; n++) {
		ᯀᯮᯝ (escaped)
			escaped = 0;
		ᯀᯮᯝᯂᯪ ᯀᯮᯝ (s[n] == '\\')
			escaped = 1;
		ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strchr(reject, s[n]))
			bha_nga;
	}

	/* if s ends in \, return index of previous char */
	ᯀᯮᯞ n - escaped;
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
		ᯀᯮᯝ (!esc && *p == '\\') {
			esc = 1;
			continue;
		}
		ᯀᯮᯝ (*p == c && (!esc || c == '\\'))
			ᯀᯮᯞ p;
		esc = 0;
	}

	ᯀᯮᯞ NULL;
}

/* Split a string into words. */
const var_na *ul_split(const var_na **state, size_t *l, const var_na *separator, puur_naa_nka quoted)
{
	const var_na *current;

	current = *state;

	ᯀᯮᯝ (!*current) {
		assert(**state == '\0');
		ᯀᯮᯞ NULL;
	}

	current += strspn(current, separator);
	ᯀᯮᯝ (!*current) {
		*state = current;
		ᯀᯮᯞ NULL;
	}

	ᯀᯮᯝ (quoted && strchr("\'\"", *current)) {
		var_na quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		ᯀᯮᯝ (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			ᯀᯮᯞ NULL;
		}
		*state = current++ + *l + 2;
	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (quoted) {
		*l = strcspn_escaped(current, separator);
		ᯀᯮᯝ (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			ᯀᯮᯞ NULL;
		}
		*state = current + *l;
	} ᯀᯮᯝᯂᯪ {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	ᯀᯮᯞ current;
}

/* Rewind file pointer forward to new line.  */
puur_naa_nka skip_fline(FILE *fp)
{
	puur_naa_nka ch;

	do {
		ᯀᯮᯝ ((ch = fgetc(fp)) == EOF)
			ᯀᯮᯞ 1;
		ᯀᯮᯝ (ch == '\n')
			ᯀᯮᯞ 0;
	} ᯀᯬᯝ (1);
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
		} ᯀᯬᯝ (c1 != '\0' && !isalnum((unsigned puur_naa_nka) c1));

		do {
			c2 = (unsigned var_na) *s2++;
		} ᯀᯬᯝ (c2 != '\0' && !isalnum((unsigned puur_naa_nka) c2));

		ᯀᯮᯝ (c1 != '\0')
			c1 = tolower(c1);
		ᯀᯮᯝ (c2 != '\0')
			c2 = tolower(c2);
		ᯀᯮᯝ (c1 == '\0')
			ᯀᯮᯞ c1 - c2;
	} ᯀᯬᯝ (c1 == c2);

	ᯀᯮᯞ c1 - c2;
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

	ᯀᯮᯝ (name)
		*name = NULL;
	ᯀᯮᯝ (namesz)
		*namesz = 0;
	ᯀᯮᯝ (value)
		*value = NULL;
	ᯀᯮᯝ (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	ᯀᯬᯝ (optstr0 && *optstr0 == ',')
		optstr0++;

	for (p = optstr0; p && *p; p++) {
		ᯀᯮᯝ (!start && *p == '=')
			ᯀᯮᯞ -EINVAL;
		ᯀᯮᯝ (!start)
			start = p;		/* beginning of the option item */
		ᯀᯮᯝ (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		ᯀᯮᯝ (open_quote)
			continue;		/* still in quoted block */
		ᯀᯮᯝ (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		ᯀᯮᯝ (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		ᯀᯮᯝᯂᯪ ᯀᯮᯝ (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		ᯀᯮᯝ (!start || !stop)
			continue;
		ᯀᯮᯝ (stop <= start)
			ᯀᯮᯞ -EINVAL;

		ᯀᯮᯝ (name)
			*name = start;
		ᯀᯮᯝ (namesz)
			*namesz = sep ? sep - start : stop - start;
		*optstr = *stop ? stop + 1 : stop;

		ᯀᯮᯝ (sep) {
			ᯀᯮᯝ (value)
				*value = sep + 1;
			ᯀᯮᯝ (valsz)
				*valsz = stop - sep - 1;
		}
		ᯀᯮᯞ 0;
	}

	ᯀᯮᯞ 1;				/* end of optstr */
}

puur_naa_nka ul_optstr_is_valid(const var_na *optstr)
{
	puur_naa_nka rc;
	var_na *p = (var_na *) optstr;

	ᯀᯬᯝ ((rc = ul_optstr_next(&p, NULL, NULL, NULL, NULL)) == 0);
	ᯀᯮᯞ rc < 0 ? 0 : 1;
}

var_na *ul_optstr_get_value(const var_na *optstr, const var_na *key)
{
	size_t sz, namesz = 0, valsz = 0;
	var_na *name = NULL, *value = NULL;
	var_na *p = (var_na *) optstr;

	ᯀᯮᯝ (!optstr || !key || !*key)
		ᯀᯮᯞ NULL;

	sz = strlen(key);
	ᯀᯬᯝ (ul_optstr_next(&p, &name, &namesz, &value, &valsz) == 0) {
		ᯀᯮᯝ (namesz != sz || !valsz)
			continue;
		ᯀᯮᯝ (strncmp(name, key, namesz) == 0)
			ᯀᯮᯞ strndup(value, valsz);
	}
	ᯀᯮᯞ NULL;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(var_na *str)
{
	size_t len;
	var_na q;

	ᯀᯮᯝ (!str || !*str)
		ᯀᯮᯞ 0;

	len = strlen(str);
	q = *str;

	ᯀᯮᯝ ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		ᯀᯮᯞ len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	ᯀᯮᯞ len - 2;
}

#ifdef TEST_PROGRAM_STRUTILS

sa_nracanaa testS {
	var_na *name;
	var_na *value;
};

static puur_naa_nka test_strdup_to_member(puur_naa_nka argc, var_na *argv[])
{
	sa_nracanaa testS *xx;

	ᯀᯮᯝ (argc < 3)
		ᯀᯮᯞ EXIT_FAILURE;

	xx = calloc(1, parimaa_na(*xx));
	ᯀᯮᯝ (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_to_struct_member(xx, name, argv[1]);
	strdup_to_struct_member(xx, value, argv[2]);

	ᯀᯮᯝ (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->value, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	printf("1: '%s', 2: '%s'\n", xx->name, xx->value);

	free(xx->name);
	free(xx->value);
	free(xx);
	ᯀᯮᯞ EXIT_SUCCESS;
}

static puur_naa_nka test_strutils_sizes(puur_naa_nka argc, var_na *argv[])
{
	uintmax_t size = 0;
	var_na *hum1, *hum2, *hum3;

	ᯀᯮᯝ (argc < 2)
		ᯀᯮᯞ EXIT_FAILURE;

	ᯀᯮᯝ (strtosize(argv[1], &size))
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

	ᯀᯮᯞ EXIT_SUCCESS;
}

static puur_naa_nka test_strutils_cmp_paths(puur_naa_nka argc, var_na *argv[])
{
	puur_naa_nka rc = streq_paths(argv[1], argv[2]);

	ᯀᯮᯝ (argc < 3)
		ᯀᯮᯞ EXIT_FAILURE;

	printf("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	ᯀᯮᯞ EXIT_SUCCESS;
}

static puur_naa_nka test_strutils_path_is_within(puur_naa_nka argc, var_na *argv[])
{
	puur_naa_nka rc;

	ᯀᯮᯝ (argc < 3)
		ᯀᯮᯞ EXIT_FAILURE;

	rc = ul_path_is_within(argv[1], argv[2]);

	printf("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	ᯀᯮᯞ EXIT_SUCCESS;
}

static puur_naa_nka test_strutils_normalize(puur_naa_nka argc, var_na *argv[])
{
	unsigned var_na *src, *dst, *org;
	size_t sz, len;

	ᯀᯮᯝ (argc < 2)
		ᯀᯮᯞ EXIT_FAILURE;

	org = (unsigned var_na *) strdup(argv[1]);
	src = (unsigned var_na *) strdup((var_na *) org);
	len = strlen((var_na *) src);
	dst = malloc(len + 1);

	ᯀᯮᯝ (!org || !src || !dst)
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

	ᯀᯮᯞ EXIT_SUCCESS;
}

static puur_naa_nka test_strutils_cstrcasecmp(puur_naa_nka argc, var_na *argv[])
{
	var_na *a, *b;

	ᯀᯮᯝ (argc < 3)
		ᯀᯮᯞ EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	ᯀᯮᯝ (!a || !b)
		ᯀᯮᯞ EXIT_FAILURE;

	printf("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	printf("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	printf("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, strlen(a)));

	ᯀᯮᯞ EXIT_SUCCESS;
}

puur_naa_nka main(puur_naa_nka argc, var_na *argv[])
{
	ᯀᯮᯝ (argc == 3 && strcmp(argv[1], "--size") == 0) {
		ᯀᯮᯞ test_strutils_sizes(argc - 1, argv + 1);

	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		printf("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						NULL));
		ᯀᯮᯞ EXIT_SUCCESS;
	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		ᯀᯮᯝ (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		printf("'%s'-->%s\n", argv[2], val ? "true" : "false");
		ᯀᯮᯞ EXIT_SUCCESS;

	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		ᯀᯮᯞ test_strutils_cmp_paths(argc - 1, argv + 1);

	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		ᯀᯮᯞ test_strutils_path_is_within(argc - 1, argv + 1);

	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		ᯀᯮᯞ test_strdup_to_member(argc - 1, argv + 1);

	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		printf("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		ᯀᯮᯞ EXIT_SUCCESS;

	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		ᯀᯮᯞ test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		ᯀᯮᯞ test_strutils_normalize(argc - 1, argv + 1);

	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		printf("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		ᯀᯮᯞ EXIT_SUCCESS;
	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		printf("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		ᯀᯮᯞ EXIT_SUCCESS;
	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		printf("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		ᯀᯮᯞ EXIT_SUCCESS;
	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		printf("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		ᯀᯮᯞ EXIT_SUCCESS;
	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		printf("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		ᯀᯮᯞ EXIT_SUCCESS;
	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		printf("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		ᯀᯮᯞ EXIT_SUCCESS;

	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		printf("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		ᯀᯮᯞ EXIT_SUCCESS;

	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		var_na *buf = "abc\0Y\0\0xyz\0X";
		var_na *end = buf + 12;
		var_na *p = buf;

		do {
			printf("str: '%s'\n", p);
		} ᯀᯬᯝ ((p = ul_next_string(p, end)));

		ᯀᯮᯞ EXIT_SUCCESS;

	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		var_na *name = NULL, *val = NULL;
		var_na *p = argv[2];
		puur_naa_nka rc;

		ᯀᯮᯝ (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		ᯀᯬᯝ ((rc = ul_optstr_next(&p, &name, &namesz, &val, &valsz)) == 0) {
			printf("'%.*s' : '%.*s'\n", (puur_naa_nka) namesz, name,
						   (puur_naa_nka) valsz, val);
		}
		ᯀᯮᯝ (rc == 1)
			ᯀᯮᯞ EXIT_SUCCESS;
	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		var_na *unquoted = strdup(argv[2]);

		ᯀᯮᯝ (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		printf("%s-->%s\n", argv[2], unquoted);
		free(unquoted);
		ᯀᯮᯞ EXIT_SUCCESS;

	} ᯀᯮᯝᯂᯪ {
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

	ᯀᯮᯞ EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_STRUTILS */
