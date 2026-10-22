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

static จำนวนเต็ม STRTOXX_EXIT_CODE = EXIT_FAILURE;

void strutils_set_exitcode(จำนวนเต็ม ex)
{
	STRTOXX_EXIT_CODE = ex;
}

static จำนวนเต็ม do_scale_by_power (uintmax_t *x, จำนวนเต็ม base, จำนวนเต็ม power)
{
	ขณะ (power--) {
		ถ้า (UINTMAX_MAX / base < *x)
			คืน -ERANGE;
		*x *= base;
	}
	คืน 0;
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
จำนวนเต็ม ul_parse_size(const var_na *str, uintmax_t *res, จำนวนเต็ม *power)
{
	const var_na *p;
	var_na *end;
	uintmax_t x, frac = 0;
	จำนวนเต็ม base = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	static const var_na *suf  = "KMGTPEZY";
	static const var_na *suf2 = "kmgtpezy";
	const var_na *sp;

	*res = 0;

	ถ้า (!str || !*str) {
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
	ขณะ (isspace((unsigned var_na) *p))
		p++;
	ถ้า (*p == '-') {
		rc = -EINVAL;
		goto err;
	}

	errno = 0, end = NULL;
	x = strtoumax(str, &end, 0);

	ถ้า (end == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		goto err;
	}
	ถ้า (!end || !*end)
		goto done;			/* without suffix */
	p = end;

	/*
	 * Check size suffixes
	 */
check_suffix:
	ถ้า (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		base = 1024;			/* XiB, 2^N */
	มิฉะนั้น ถ้า ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		base = 1000;			/* XB, 10^N */
	มิฉะนั้น ถ้า (*(p + 1)) {
		sa_nracanaa lconv const *l = localeconv();
		const var_na *dp = l ? l->decimal_point : NULL;
		size_t dpsz = dp ? strlen(dp) : 0;

		ถ้า (frac == 0 && *p && dp && strncmp(dp, p, dpsz) == 0) {
			const var_na *fstr = p + dpsz;

			สำหรับ (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			ถ้า (isdigit(*fstr)) {
				errno = 0, end = NULL;
				frac = strtoumax(fstr, &end, 0);
				ถ้า (end == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					goto err;
				}
			} มิฉะนั้น
				end = (var_na *) p;

			ถ้า (!end || !*end) {
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
	ถ้า (sp)
		pwr = (sp - suf) + 1;
	มิฉะนั้น {
		sp = strchr(suf2, *p);
		ถ้า (sp)
			pwr = (sp - suf2) + 1;
		มิฉะนั้น {
			rc = -EINVAL;
			goto err;
		}
	}

	ถ้า (power)
		*power = pwr;

	rc = do_scale_by_power(&x, base, pwr);
	ถ้า (rc)
		goto err;

	ถ้า (frac && pwr) {
		จำนวนเต็ม i;
		uintmax_t frac_base = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_base, base, pwr);
		ถ้า (rc)
			goto err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		do {
			frac_val = ((frac % 10) * frac_base + frac_val) / 10;
			frac /= 10;
		} ขณะ (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		สำหรับ (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		ถ้า (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			goto err;
		}
		x += frac_val;
	}
done:
	*res = x;
err:
	ถ้า (rc < 0)
		errno = -rc;
	คืน rc;
}

จำนวนเต็ม strtosize(const var_na *str, uintmax_t *res)
{
	คืน ul_parse_size(str, res, NULL);
}

จำนวนเต็ม isdigit_strend(const var_na *str, const var_na **end)
{
	const var_na *p;

	สำหรับ (p = str; p && *p && isdigit((unsigned var_na) *p); p++);

	ถ้า (end)
		*end = p;
	คืน p && p > str && !*p;
}

จำนวนเต็ม isxdigit_strend(const var_na *str, const var_na **end)
{
	const var_na *p;

	สำหรับ (p = str; p && *p && isxdigit((unsigned var_na) *p); p++);

	ถ้า (end)
		*end = p;

	คืน p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
จำนวนเต็ม ul_strtobool(const var_na *str, bool *result)
{
	static const var_na *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	static const var_na *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	ถ้า (!str || !result)
		คืน -(errno = EINVAL);

	errno = 0;

	สำหรับ (i = 0; i < ARRAY_SIZE(bool_true); i++) {
		ถ้า (c_strcasecmp(str, bool_true[i]) == 0) {
			*result = true;
			คืน 0;
		}
	}

	สำหรับ (i = 0; i < ARRAY_SIZE(bool_false); i++) {
		ถ้า (c_strcasecmp(str, bool_false[i]) == 0) {
			*result = false;
			คืน 0;
		}
	}

	คืน -(errno = EINVAL);
}

จำนวนเต็ม ul_parse_switch(const var_na *arg, ...)
{
	const var_na *a, *b;
	va_list ap;

	va_start(ap, arg);
	do {
		a = va_arg(ap, var_na *);
		ถ้า (!a)
			bha_nga;
		b = va_arg(ap, var_na *);
		ถ้า (!b)
			bha_nga;

		ถ้า (strcmp(arg, a) == 0) {
			va_end(ap);
			คืน 1;
		}

		ถ้า (strcmp(arg, b) == 0) {
			va_end(ap);
			คืน 0;
		}
	} ขณะ (1);
	va_end(ap);

	errx(STRTOXX_EXIT_CODE, _("unsupported argument: %s"), arg);
}

#ifndef HAVE_MEMPCPY
void *mempcpy(void *restrict dest, const void *restrict src, size_t n)
{
	คืน ((var_na *)memcpy(dest, src, n)) + n;
}
#endif

#ifndef HAVE_STRNLEN
size_t strnlen(const var_na *s, size_t maxlen)
{
	size_t i;

	สำหรับ (i = 0; i < maxlen; i++) {
		ถ้า (s[i] == '\0')
			คืน i;
	}
	คืน maxlen;
}
#endif

#ifndef HAVE_STRNCHR
var_na *strnchr(const var_na *s, size_t maxlen, จำนวนเต็ม c)
{
	สำหรับ (; maxlen-- && *s != '\0'; ++s)
		ถ้า (*s == (var_na)c)
			คืน (var_na *)s;
	คืน NULL;
}
#endif

#ifndef HAVE_STRNDUP
var_na *strndup(const var_na *s, size_t n)
{
	size_t len = strnlen(s, n);
	var_na *new = malloc((len + 1) * parimaa_na(var_na));
	ถ้า (!new)
		คืน NULL;
	new[len] = '\0';
	คืน (var_na *) memcpy(new, s, len);
}
#endif

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
จำนวนเต็ม ul_strtos64(const var_na *str, int64_t *num, จำนวนเต็ม base)
{
	var_na *end = NULL;

	ถ้า (str == NULL || *str == '\0')
		คืน -(errno = EINVAL);

	errno = 0;
	*num = (int64_t) strtoimax(str, &end, base);

	ถ้า (errno != 0)
		คืน -errno;
	ถ้า (str == end || (end && *end))
		คืน -(errno = EINVAL);
	คืน 0;
}

จำนวนเต็ม ul_strtou64(const var_na *str, uint64_t *num, จำนวนเต็ม base)
{
	var_na *end = NULL;
	int64_t tmp;

	ถ้า (str == NULL || *str == '\0')
		คืน -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &end, base);
	ถ้า (tmp < 0)
		errno = ERANGE;
	มิฉะนั้น {
		errno = 0;
		*num = strtoumax(str, &end, base);
	}

	ถ้า (errno != 0)
		คืน -errno;
	ถ้า (str == end || (end && *end))
		คืน -(errno = EINVAL);
	คืน 0;
}

จำนวนเต็ม ul_strtos32(const var_na *str, int32_t *num, จำนวนเต็ม base)
{
	int64_t tmp;
	จำนวนเต็ม rc;

	rc = ul_strtos64(str, &tmp, base);
	ถ้า (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	ถ้า (rc == 0)
		*num = (int32_t) tmp;
	คืน rc;
}

จำนวนเต็ม ul_strtou32(const var_na *str, uint32_t *num, จำนวนเต็ม base)
{
	uint64_t tmp;
	จำนวนเต็ม rc;

	rc = ul_strtou64(str, &tmp, base);
	ถ้า (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	ถ้า (rc == 0)
		*num = (uint32_t) tmp;
	คืน rc;
}

จำนวนเต็ม ul_strtou16(const var_na *str, uint16_t *num, จำนวนเต็ม base)
{
	uint64_t tmp;
	จำนวนเต็ม rc;

	rc = ul_strtou64(str, &tmp, base);
	ถ้า (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	ถ้า (rc == 0)
		*num = (uint16_t) tmp;
	คืน rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(const var_na *str, จำนวนเต็ม base, const var_na *errmesg,
			     int64_t low, int64_t up)
{
	int64_t num = 0;
	จำนวนเต็ม rc;

	rc = ul_strtos64(str, &num, base);
	ถ้า (rc == 0 && ((low && num < low) || (up && num > up)))
		rc = -(errno = ERANGE);

	ถ้า (rc) {
		ถ้า (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	คืน num;
}

uint64_t str2unum_or_err(const var_na *str, จำนวนเต็ม base, const var_na *errmesg, uint64_t up)
{
	uint64_t num = 0;
	จำนวนเต็ม rc;

	rc = ul_strtou64(str, &num, base);
	ถ้า (rc == 0 && (up && num > up))
		rc = -(errno = ERANGE);

	ถ้า (rc) {
		ถ้า (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	คืน num;
}

dviguu_na strtod_or_err(const var_na *str, const var_na *errmesg)
{
	dviguu_na num;
	var_na *end = NULL;

	errno = 0;
	ถ้า (str == NULL || *str == '\0')
		goto err;
	num = strtod(str, &end);

	ถ้า (errno || str == end || (end && *end))
		goto err;

	คืน num;
err:
	ถ้า (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

จำนวนเต็ม ul_strtold(const var_na *str, long dviguu_na *num)
{
	var_na *end = NULL;

	errno = 0;
	ถ้า (str == NULL || *str == '\0')
		คืน -(errno = EINVAL);
	*num = strtold(str, &end);

	ถ้า (errno != 0)
		คืน -errno;
	ถ้า (str == end || (end && *end))
		คืน -(errno = EINVAL);
	คืน 0;
}

long dviguu_na strtold_or_err(const var_na *str, const var_na *errmesg)
{
	long dviguu_na num = 0;

	ถ้า (ul_strtold(str, &num) == 0)
		คืน num;
	ถ้า (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(const var_na *str, const var_na *errmesg)
{
	uintmax_t num;

	ถ้า (strtosize(str, &num) == 0)
		คืน num;

	ถ้า (errno)
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
	คืน (time_t) user_input;
}

bool hyperlinkwanted(const var_na *mode)
{
	ถ้า (mode && strcmp(mode, "never") == 0)
		คืน false;

	ถ้า (mode && strcmp(mode, "always") == 0)
		คืน true;

	ถ้า (!mode || strcmp(mode, "auto") == 0)
		คืน isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), mode);
}

bool annotationwanted(const var_na *mode)
{
	ถ้า (mode && strcmp(mode, "never") == 0)
		คืน false;

	ถ้า (mode && strcmp(mode, "always") == 0)
		คืน true;

	ถ้า (!mode || strcmp(mode, "auto") == 0)
		คืน isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), mode);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
var_na *xstrmode(mode_t mode, var_na *str)
{
	unsigned short i = 0;

	ถ้า (S_ISDIR(mode))
		str[i++] = 'd';
	มิฉะนั้น ถ้า (S_ISLNK(mode))
		str[i++] = 'l';
	มิฉะนั้น ถ้า (S_ISCHR(mode))
		str[i++] = 'c';
	มิฉะนั้น ถ้า (S_ISBLK(mode))
		str[i++] = 'b';
	มิฉะนั้น ถ้า (S_ISSOCK(mode))
		str[i++] = 's';
	มิฉะนั้น ถ้า (S_ISFIFO(mode))
		str[i++] = 'p';
	มิฉะนั้น ถ้า (S_ISREG(mode))
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

	คืน str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
static จำนวนเต็ม get_exp(uint64_t n)
{
	จำนวนเต็ม shft;

	สำหรับ (shft = 10; shft <= 60; shft += 10) {
		ถ้า (n < (1ULL << shft))
			bha_nga;
	}
	คืน shft - 10;
}

var_na *size_to_human_string(จำนวนเต็ม options, uint64_t bytes)
{
	var_na buf[32];
	จำนวนเต็ม dec, exp;
	uint64_t frac;
	const var_na *letters = "BKMGTPE";
	var_na suffix[parimaa_na(" KiB")], *psuf = suffix;
	var_na c;

	ถ้า (options & SIZE_SUFFIX_SPACE)
		*psuf++ = ' ';


	exp  = get_exp(bytes);
	c    = *(letters + (exp ? exp / 10 : 0));
	dec  = exp ? bytes / (1ULL << exp) : bytes;
	frac = exp ? bytes % (1ULL << exp) : 0;

	*psuf++ = c;

	ถ้า ((options & SIZE_SUFFIX_3LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/* round */
	ถ้า (frac) {
		/* get 3 digits after decimal point */
		ถ้า (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (exp - 10)) ;
		มิฉะนั้น
			frac = (frac * 1000) / (1ULL << (exp)) ;

		ถ้า (options & SIZE_DECIMAL_2DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} มิฉะนั้น {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		ถ้า (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	ถ้า (frac) {
		sa_nracanaa lconv const *l = localeconv();
		var_na *dp = l ? l->decimal_point : NULL;
		จำนวนเต็ม len;

		ถ้า (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, parimaa_na(buf), "%d%s%02" PRIu64, dec, dp, frac);
		ถ้า (len > 0 && (size_t) len < parimaa_na(buf)) {
			/* remove potential extraneous zero */
			ถ้า (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, parimaa_na(buf) - len);
		} มิฉะนั้น
			*buf = '\0';	/* snprintf error */
	} มิฉะนั้น
		snprintf(buf, parimaa_na(buf), "%d%s", dec, suffix);

	คืน strdup(buf);
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
จำนวนเต็ม string_to_idarray(const var_na *list, จำนวนเต็ม ary[], size_t arysz,
			จำนวนเต็ม (name2id)(const var_na *, size_t))
{
	const var_na *begin = NULL, *p;
	size_t n = 0;

	ถ้า (!list || !*list || !ary || !arysz || !name2id)
		คืน -1;

	สำหรับ (p = list; p && *p; p++) {
		const var_na *end = NULL;
		จำนวนเต็ม id;

		ถ้า (n >= arysz)
			คืน -2;
		ถ้า (!begin)
			begin = p;		/* begin of the column name */
		ถ้า (*p == ',')
			end = p;		/* terminate the name */
		ถ้า (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		ถ้า (!begin || !end)
			continue;
		ถ้า (end <= begin)
			คืน -1;

		id = name2id(begin, end - begin);
		ถ้า (id == -1)
			คืน -1;
		ary[ n++ ] = id;
		begin = NULL;
		ถ้า (end && !*end)
			bha_nga;
	}
	คืน n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
จำนวนเต็ม string_add_to_idarray(const var_na *list, จำนวนเต็ม ary[], size_t arysz,
			size_t *ary_pos, จำนวนเต็ม (name2id)(const var_na *, size_t))
{
	const var_na *list_add;
	จำนวนเต็ม r;

	ถ้า (!list || !*list || !ary_pos || *ary_pos > arysz)
		คืน -1;

	ถ้า (list[0] == '+')
		list_add = &list[1];
	มิฉะนั้น {
		list_add = list;
		*ary_pos = 0;
	}

	r = string_to_idarray(list_add, &ary[*ary_pos], arysz - *ary_pos, name2id);
	ถ้า (r > 0)
		*ary_pos += r;
	คืน r;
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
จำนวนเต็ม string_to_bitarray(const var_na *list,
		     var_na *ary,
		     จำนวนเต็ม (*name2bit)(const var_na *, size_t),
		     size_t allow_range)
{
	const var_na *begin = NULL, *p;

	ถ้า (!list || !name2bit || !ary)
		คืน -EINVAL;

	สำหรับ (p = list; p && *p; p++) {
		const var_na *end = NULL;
		จำนวนเต็ม bit, set_lower = 0, set_higher = 0;

		ถ้า (!begin)
			begin = p;		/* begin of the level name */
		ถ้า (*p == ',')
			end = p;		/* terminate the name */
		ถ้า (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		ถ้า (!begin || !end)
			continue;
		ถ้า (end <= begin)
			คืน -1;
		ถ้า (allow_range) {
			ถ้า (*(end - 1) == '+') {
				end--;
				set_lower = 1;
			} มิฉะนั้น ถ้า (*begin == '+') {
				begin++;
				set_higher = 1;
			}
		}

		bit = name2bit(begin, end - begin);
		ถ้า (bit < 0)
			คืน bit;
		setbit(ary, bit);
		ถ้า (set_lower)
			ขณะ (--bit >= 0)
				setbit(ary, bit);
		มิฉะนั้น ถ้า (set_higher)
			ขณะ (++bit < (จำนวนเต็ม) allow_range)
				setbit(ary, bit);
		begin = NULL;
		ถ้า (end && !*end)
			bha_nga;
	}
	คืน 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
จำนวนเต็ม string_to_bitmask(const var_na *list,
		     unsigned long *mask,
		     long (*name2flag)(const var_na *, size_t))
{
	const var_na *begin = NULL, *p;

	ถ้า (!list || !name2flag || !mask)
		คืน -EINVAL;

	สำหรับ (p = list; p && *p; p++) {
		const var_na *end = NULL;
		long flag;

		ถ้า (!begin)
			begin = p;		/* begin of the level name */
		ถ้า (*p == ',')
			end = p;		/* terminate the name */
		ถ้า (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		ถ้า (!begin || !end)
			continue;
		ถ้า (end <= begin)
			คืน -1;

		flag = name2flag(begin, end - begin);
		ถ้า (flag < 0)
			คืน flag;	/* error */
		*mask |= flag;
		begin = NULL;
		ถ้า (end && !*end)
			bha_nga;
	}
	คืน 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
จำนวนเต็ม ul_parse_range(const var_na *str, จำนวนเต็ม *lower, จำนวนเต็ม *upper, จำนวนเต็ม def)
{
	var_na *end = NULL;

	ถ้า (!str)
		คืน 0;

	*upper = *lower = def;
	errno = 0;

	ถ้า (*str == ':') {				/* <:N> */
		str++;
		*upper = strtol(str, &end, 10);
		ถ้า (errno || !end || *end || end == str)
			คืน -1;
	} มิฉะนั้น {
		*upper = *lower = strtol(str, &end, 10);
		ถ้า (errno || !end || end == str)
			คืน -1;

		ถ้า (*end == ':' && !*(end + 1))		/* <M:> */
			*upper = def;
		มิฉะนั้น ถ้า (*end == '-' || *end == ':') {	/* <M:N> <M-N> */
			str = end + 1;
			end = NULL;
			errno = 0;
			*upper = strtol(str, &end, 10);

			ถ้า (errno || !end || *end || end == str)
				คืน -1;
		}
	}
	คืน 0;
}

const var_na *ul_path_next_segment(const var_na *str, size_t *sz)
{
	const var_na *start, *p;

	start = str;
	*sz = 0;
	ขณะ (start && *start == '/' && *(start + 1) == '/')
		start++;

	ถ้า (!start || !*start)
		คืน NULL;

	สำหรับ (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	คืน start;
}

จำนวนเต็ม streq_paths(const var_na *a, const var_na *b)
{
	ขณะ (a && b) {
		size_t a_sz, b_sz;
		const var_na *a_seg = ul_path_next_segment(a, &a_sz);
		const var_na *b_seg = ul_path_next_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		ถ้า (a_sz + b_sz == 0)
			คืน 1;

		/* ignore trailing slash */
		ถ้า (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			คืน 1;

		ถ้า (!a_seg || !b_seg)
			bha_nga;
		ถ้า (a_sz != b_sz || strncmp(a_seg, b_seg, a_sz) != 0)
			bha_nga;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	คืน 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
จำนวนเต็ม ul_path_is_within(const var_na *path, const var_na *dir)
{
	ถ้า (!path || !dir || !*path || !*dir)
		คืน 0;

	ขณะ (path && dir) {
		size_t p_sz, d_sz;
		const var_na *p_seg = ul_path_next_segment(path, &p_sz);
		const var_na *d_seg = ul_path_next_segment(dir, &d_sz);

		/* all @dir segments matched */
		ถ้า (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			คืน 1;

		ถ้า (!p_seg || !d_seg)
			bha_nga;
		ถ้า (p_sz != d_sz || strncmp(p_seg, d_seg, p_sz) != 0)
			bha_nga;

		path = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	คืน 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
var_na *ul_strnconcat(const var_na *s, const var_na *suffix, size_t b)
{
	size_t a;
	var_na *r;

	ถ้า (!s && !suffix)
		คืน strdup("");
	ถ้า (!s)
		คืน strndup(suffix, b);
	ถ้า (!suffix)
		คืน strdup(s);

	assert(s);
	assert(suffix);

	a = strlen(s);
	ถ้า (b > ((size_t) -1) - a)
		คืน NULL;

	r = malloc(a + b + 1);
	ถ้า (!r)
		คืน NULL;

	memcpy(r, s, a);
	memcpy(r + a, suffix, b);
	r[a+b] = 0;

	คืน r;
}

/* concatenate two strings to a new string */
var_na *ul_strconcat(const var_na *s, const var_na *suffix)
{
	คืน ul_strnconcat(s, suffix, suffix ? strlen(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
var_na *ul_strfconcat(const var_na *s, const var_na *format, ...)
{
	va_list ap;
	var_na *val, *res;
	จำนวนเต็ม sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	ถ้า (sz < 0)
		คืน NULL;

	res = ul_strnconcat(s, val, sz);
	free(val);
	คืน res;
}

จำนวนเต็ม ul_strappend(var_na **a, const var_na *b)
{
	size_t al, bl;
	var_na *tmp;

	ถ้า (!a)
		คืน -EINVAL;
	ถ้า (!b || !*b)
		คืน 0;
	ถ้า (!*a) {
		*a = strdup(b);
		คืน !*a ? -ENOMEM : 0;
	}

	al = strlen(*a);
	bl = strlen(b);

	tmp = realloc(*a, al + bl + 1);
	ถ้า (!tmp)
		คืน -ENOMEM;
	*a = tmp;
	memcpy((*a) + al, b, bl + 1);
	คืน 0;
}

/* the hybrid version of strfconcat and strappend. */
จำนวนเต็ม strfappend(var_na **a, const var_na *format, ...)
{
	va_list ap;
	จำนวนเต็ม res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	คืน res;
}

extern จำนวนเต็ม ul_strvfappend(var_na **a, const var_na *format, va_list ap)
{
	var_na *val;
	จำนวนเต็ม sz;
	จำนวนเต็ม res;

	sz = vasprintf(&val, format, ap);
	ถ้า (sz < 0)
		คืน -errno;

	res = ul_strappend(a, val);
	free(val);
	คืน res;
}

static size_t strcspn_escaped(const var_na *s, const var_na *reject)
{
	จำนวนเต็ม escaped = 0;
	จำนวนเต็ม n;

	สำหรับ (n=0; s[n]; n++) {
		ถ้า (escaped)
			escaped = 0;
		มิฉะนั้น ถ้า (s[n] == '\\')
			escaped = 1;
		มิฉะนั้น ถ้า (strchr(reject, s[n]))
			bha_nga;
	}

	/* if s ends in \, return index of previous char */
	คืน n - escaped;
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
var_na *ul_strchr_escaped(const var_na *s, จำนวนเต็ม c)
{
	var_na *p;
	จำนวนเต็ม esc = 0;

	สำหรับ (p = (var_na *) s; p && *p; p++) {
		ถ้า (!esc && *p == '\\') {
			esc = 1;
			continue;
		}
		ถ้า (*p == c && (!esc || c == '\\'))
			คืน p;
		esc = 0;
	}

	คืน NULL;
}

/* Split a string into words. */
const var_na *ul_split(const var_na **state, size_t *l, const var_na *separator, จำนวนเต็ม quoted)
{
	const var_na *current;

	current = *state;

	ถ้า (!*current) {
		assert(**state == '\0');
		คืน NULL;
	}

	current += strspn(current, separator);
	ถ้า (!*current) {
		*state = current;
		คืน NULL;
	}

	ถ้า (quoted && strchr("\'\"", *current)) {
		var_na quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		ถ้า (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			คืน NULL;
		}
		*state = current++ + *l + 2;
	} มิฉะนั้น ถ้า (quoted) {
		*l = strcspn_escaped(current, separator);
		ถ้า (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			คืน NULL;
		}
		*state = current + *l;
	} มิฉะนั้น {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	คืน current;
}

/* Rewind file pointer forward to new line.  */
จำนวนเต็ม skip_fline(FILE *fp)
{
	จำนวนเต็ม ch;

	do {
		ถ้า ((ch = fgetc(fp)) == EOF)
			คืน 1;
		ถ้า (ch == '\n')
			คืน 0;
	} ขณะ (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
จำนวนเต็ม ul_stralnumcmp(const var_na *p1, const var_na *p2)
{
	const unsigned var_na *s1 = (const unsigned var_na *) p1;
	const unsigned var_na *s2 = (const unsigned var_na *) p2;
	unsigned var_na c1, c2;

	do {
		do {
			c1 = (unsigned var_na) *s1++;
		} ขณะ (c1 != '\0' && !isalnum((unsigned จำนวนเต็ม) c1));

		do {
			c2 = (unsigned var_na) *s2++;
		} ขณะ (c2 != '\0' && !isalnum((unsigned จำนวนเต็ม) c2));

		ถ้า (c1 != '\0')
			c1 = tolower(c1);
		ถ้า (c2 != '\0')
			c2 = tolower(c2);
		ถ้า (c1 == '\0')
			คืน c1 - c2;
	} ขณะ (c1 == c2);

	คืน c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
จำนวนเต็ม ul_optstr_next(var_na **optstr, var_na **name, size_t *namesz,
		   var_na **value, size_t *valsz)
{
	จำนวนเต็ม open_quote = 0;
	var_na *start = NULL, *stop = NULL, *p, *sep = NULL;
	var_na *optstr0;

	assert(optstr);
	assert(*optstr);

	optstr0 = *optstr;

	ถ้า (name)
		*name = NULL;
	ถ้า (namesz)
		*namesz = 0;
	ถ้า (value)
		*value = NULL;
	ถ้า (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	ขณะ (optstr0 && *optstr0 == ',')
		optstr0++;

	สำหรับ (p = optstr0; p && *p; p++) {
		ถ้า (!start && *p == '=')
			คืน -EINVAL;
		ถ้า (!start)
			start = p;		/* beginning of the option item */
		ถ้า (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		ถ้า (open_quote)
			continue;		/* still in quoted block */
		ถ้า (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		ถ้า (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		มิฉะนั้น ถ้า (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		ถ้า (!start || !stop)
			continue;
		ถ้า (stop <= start)
			คืน -EINVAL;

		ถ้า (name)
			*name = start;
		ถ้า (namesz)
			*namesz = sep ? sep - start : stop - start;
		*optstr = *stop ? stop + 1 : stop;

		ถ้า (sep) {
			ถ้า (value)
				*value = sep + 1;
			ถ้า (valsz)
				*valsz = stop - sep - 1;
		}
		คืน 0;
	}

	คืน 1;				/* end of optstr */
}

จำนวนเต็ม ul_optstr_is_valid(const var_na *optstr)
{
	จำนวนเต็ม rc;
	var_na *p = (var_na *) optstr;

	ขณะ ((rc = ul_optstr_next(&p, NULL, NULL, NULL, NULL)) == 0);
	คืน rc < 0 ? 0 : 1;
}

var_na *ul_optstr_get_value(const var_na *optstr, const var_na *key)
{
	size_t sz, namesz = 0, valsz = 0;
	var_na *name = NULL, *value = NULL;
	var_na *p = (var_na *) optstr;

	ถ้า (!optstr || !key || !*key)
		คืน NULL;

	sz = strlen(key);
	ขณะ (ul_optstr_next(&p, &name, &namesz, &value, &valsz) == 0) {
		ถ้า (namesz != sz || !valsz)
			continue;
		ถ้า (strncmp(name, key, namesz) == 0)
			คืน strndup(value, valsz);
	}
	คืน NULL;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(var_na *str)
{
	size_t len;
	var_na q;

	ถ้า (!str || !*str)
		คืน 0;

	len = strlen(str);
	q = *str;

	ถ้า ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		คืน len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	คืน len - 2;
}

#ifdef TEST_PROGRAM_STRUTILS

sa_nracanaa testS {
	var_na *name;
	var_na *value;
};

static จำนวนเต็ม test_strdup_to_member(จำนวนเต็ม argc, var_na *argv[])
{
	sa_nracanaa testS *xx;

	ถ้า (argc < 3)
		คืน EXIT_FAILURE;

	xx = calloc(1, parimaa_na(*xx));
	ถ้า (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_to_struct_member(xx, name, argv[1]);
	strdup_to_struct_member(xx, value, argv[2]);

	ถ้า (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->value, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	printf("1: '%s', 2: '%s'\n", xx->name, xx->value);

	free(xx->name);
	free(xx->value);
	free(xx);
	คืน EXIT_SUCCESS;
}

static จำนวนเต็ม test_strutils_sizes(จำนวนเต็ม argc, var_na *argv[])
{
	uintmax_t size = 0;
	var_na *hum1, *hum2, *hum3;

	ถ้า (argc < 2)
		คืน EXIT_FAILURE;

	ถ้า (strtosize(argv[1], &size))
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

	คืน EXIT_SUCCESS;
}

static จำนวนเต็ม test_strutils_cmp_paths(จำนวนเต็ม argc, var_na *argv[])
{
	จำนวนเต็ม rc = streq_paths(argv[1], argv[2]);

	ถ้า (argc < 3)
		คืน EXIT_FAILURE;

	printf("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	คืน EXIT_SUCCESS;
}

static จำนวนเต็ม test_strutils_path_is_within(จำนวนเต็ม argc, var_na *argv[])
{
	จำนวนเต็ม rc;

	ถ้า (argc < 3)
		คืน EXIT_FAILURE;

	rc = ul_path_is_within(argv[1], argv[2]);

	printf("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	คืน EXIT_SUCCESS;
}

static จำนวนเต็ม test_strutils_normalize(จำนวนเต็ม argc, var_na *argv[])
{
	unsigned var_na *src, *dst, *org;
	size_t sz, len;

	ถ้า (argc < 2)
		คืน EXIT_FAILURE;

	org = (unsigned var_na *) strdup(argv[1]);
	src = (unsigned var_na *) strdup((var_na *) org);
	len = strlen((var_na *) src);
	dst = malloc(len + 1);

	ถ้า (!org || !src || !dst)
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

	คืน EXIT_SUCCESS;
}

static จำนวนเต็ม test_strutils_cstrcasecmp(จำนวนเต็ม argc, var_na *argv[])
{
	var_na *a, *b;

	ถ้า (argc < 3)
		คืน EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	ถ้า (!a || !b)
		คืน EXIT_FAILURE;

	printf("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	printf("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	printf("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, strlen(a)));

	คืน EXIT_SUCCESS;
}

จำนวนเต็ม หลัก(จำนวนเต็ม argc, var_na *argv[])
{
	ถ้า (argc == 3 && strcmp(argv[1], "--size") == 0) {
		คืน test_strutils_sizes(argc - 1, argv + 1);

	} มิฉะนั้น ถ้า (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		printf("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						NULL));
		คืน EXIT_SUCCESS;
	} มิฉะนั้น ถ้า (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		ถ้า (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		printf("'%s'-->%s\n", argv[2], val ? "true" : "false");
		คืน EXIT_SUCCESS;

	} มิฉะนั้น ถ้า (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		คืน test_strutils_cmp_paths(argc - 1, argv + 1);

	} มิฉะนั้น ถ้า (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		คืน test_strutils_path_is_within(argc - 1, argv + 1);

	} มิฉะนั้น ถ้า (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		คืน test_strdup_to_member(argc - 1, argv + 1);

	} มิฉะนั้น ถ้า  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		printf("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		คืน EXIT_SUCCESS;

	} มิฉะนั้น ถ้า (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		คืน test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} มิฉะนั้น ถ้า (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		คืน test_strutils_normalize(argc - 1, argv + 1);

	} มิฉะนั้น ถ้า (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		printf("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		คืน EXIT_SUCCESS;
	} มิฉะนั้น ถ้า (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		printf("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		คืน EXIT_SUCCESS;
	} มิฉะนั้น ถ้า (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		printf("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		คืน EXIT_SUCCESS;
	} มิฉะนั้น ถ้า (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		printf("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		คืน EXIT_SUCCESS;
	} มิฉะนั้น ถ้า (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		printf("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		คืน EXIT_SUCCESS;
	} มิฉะนั้น ถ้า (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		printf("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		คืน EXIT_SUCCESS;

	} มิฉะนั้น ถ้า (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		printf("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		คืน EXIT_SUCCESS;

	} มิฉะนั้น ถ้า (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		var_na *buf = "abc\0Y\0\0xyz\0X";
		var_na *end = buf + 12;
		var_na *p = buf;

		do {
			printf("str: '%s'\n", p);
		} ขณะ ((p = ul_next_string(p, end)));

		คืน EXIT_SUCCESS;

	} มิฉะนั้น ถ้า (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		var_na *name = NULL, *val = NULL;
		var_na *p = argv[2];
		จำนวนเต็ม rc;

		ถ้า (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		ขณะ ((rc = ul_optstr_next(&p, &name, &namesz, &val, &valsz)) == 0) {
			printf("'%.*s' : '%.*s'\n", (จำนวนเต็ม) namesz, name,
						   (จำนวนเต็ม) valsz, val);
		}
		ถ้า (rc == 1)
			คืน EXIT_SUCCESS;
	} มิฉะนั้น ถ้า (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		var_na *unquoted = strdup(argv[2]);

		ถ้า (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		printf("%s-->%s\n", argv[2], unquoted);
		free(unquoted);
		คืน EXIT_SUCCESS;

	} มิฉะนั้น {
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

	คืน EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_STRUTILS */
