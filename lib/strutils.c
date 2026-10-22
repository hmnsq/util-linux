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

static 整数 STRTOXX_EXIT_CODE = EXIT_FAILURE;

void strutils_set_exitcode(整数 ex)
{
	STRTOXX_EXIT_CODE = ex;
}

static 整数 do_scale_by_power (uintmax_t *x, 整数 base, 整数 power)
{
	当 (power--) {
		如果 (UINTMAX_MAX / base < *x)
			返回 -ERANGE;
		*x *= base;
	}
	返回 0;
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
整数 ul_parse_size(const char *str, uintmax_t *res, 整数 *power)
{
	const char *p;
	char *end;
	uintmax_t x, frac = 0;
	整数 base = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	static const char *suf  = "KMGTPEZY";
	static const char *suf2 = "kmgtpezy";
	const char *sp;

	*res = 0;

	如果 (!str || !*str) {
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
	当 (isspace((unsigned char) *p))
		p++;
	如果 (*p == '-') {
		rc = -EINVAL;
		goto err;
	}

	errno = 0, end = NULL;
	x = strtoumax(str, &end, 0);

	如果 (end == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		goto err;
	}
	如果 (!end || !*end)
		goto done;			/* without suffix */
	p = end;

	/*
	 * Check size suffixes
	 */
check_suffix:
	如果 (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		base = 1024;			/* XiB, 2^N */
	否则 如果 ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		base = 1000;			/* XB, 10^N */
	否则 如果 (*(p + 1)) {
		struct lconv const *l = localeconv();
		const char *dp = l ? l->decimal_point : NULL;
		size_t dpsz = dp ? strlen(dp) : 0;

		如果 (frac == 0 && *p && dp && strncmp(dp, p, dpsz) == 0) {
			const char *fstr = p + dpsz;

			为 (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			如果 (isdigit(*fstr)) {
				errno = 0, end = NULL;
				frac = strtoumax(fstr, &end, 0);
				如果 (end == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					goto err;
				}
			} 否则
				end = (char *) p;

			如果 (!end || !*end) {
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
	如果 (sp)
		pwr = (sp - suf) + 1;
	否则 {
		sp = strchr(suf2, *p);
		如果 (sp)
			pwr = (sp - suf2) + 1;
		否则 {
			rc = -EINVAL;
			goto err;
		}
	}

	如果 (power)
		*power = pwr;

	rc = do_scale_by_power(&x, base, pwr);
	如果 (rc)
		goto err;

	如果 (frac && pwr) {
		整数 i;
		uintmax_t frac_base = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_base, base, pwr);
		如果 (rc)
			goto err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		do {
			frac_val = ((frac % 10) * frac_base + frac_val) / 10;
			frac /= 10;
		} 当 (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		为 (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		如果 (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			goto err;
		}
		x += frac_val;
	}
done:
	*res = x;
err:
	如果 (rc < 0)
		errno = -rc;
	返回 rc;
}

整数 strtosize(const char *str, uintmax_t *res)
{
	返回 ul_parse_size(str, res, NULL);
}

整数 isdigit_strend(const char *str, const char **end)
{
	const char *p;

	为 (p = str; p && *p && isdigit((unsigned char) *p); p++);

	如果 (end)
		*end = p;
	返回 p && p > str && !*p;
}

整数 isxdigit_strend(const char *str, const char **end)
{
	const char *p;

	为 (p = str; p && *p && isxdigit((unsigned char) *p); p++);

	如果 (end)
		*end = p;

	返回 p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
整数 ul_strtobool(const char *str, bool *result)
{
	static const char *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	static const char *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	如果 (!str || !result)
		返回 -(errno = EINVAL);

	errno = 0;

	为 (i = 0; i < ARRAY_SIZE(bool_true); i++) {
		如果 (c_strcasecmp(str, bool_true[i]) == 0) {
			*result = true;
			返回 0;
		}
	}

	为 (i = 0; i < ARRAY_SIZE(bool_false); i++) {
		如果 (c_strcasecmp(str, bool_false[i]) == 0) {
			*result = false;
			返回 0;
		}
	}

	返回 -(errno = EINVAL);
}

整数 ul_parse_switch(const char *arg, ...)
{
	const char *a, *b;
	va_list ap;

	va_start(ap, arg);
	do {
		a = va_arg(ap, char *);
		如果 (!a)
			break;
		b = va_arg(ap, char *);
		如果 (!b)
			break;

		如果 (strcmp(arg, a) == 0) {
			va_end(ap);
			返回 1;
		}

		如果 (strcmp(arg, b) == 0) {
			va_end(ap);
			返回 0;
		}
	} 当 (1);
	va_end(ap);

	errx(STRTOXX_EXIT_CODE, _("unsupported argument: %s"), arg);
}

#ifndef HAVE_MEMPCPY
void *mempcpy(void *restrict dest, const void *restrict src, size_t n)
{
	返回 ((char *)memcpy(dest, src, n)) + n;
}
#endif

#ifndef HAVE_STRNLEN
size_t strnlen(const char *s, size_t maxlen)
{
	size_t i;

	为 (i = 0; i < maxlen; i++) {
		如果 (s[i] == '\0')
			返回 i;
	}
	返回 maxlen;
}
#endif

#ifndef HAVE_STRNCHR
char *strnchr(const char *s, size_t maxlen, 整数 c)
{
	为 (; maxlen-- && *s != '\0'; ++s)
		如果 (*s == (char)c)
			返回 (char *)s;
	返回 NULL;
}
#endif

#ifndef HAVE_STRNDUP
char *strndup(const char *s, size_t n)
{
	size_t len = strnlen(s, n);
	char *new = malloc((len + 1) * sizeof(char));
	如果 (!new)
		返回 NULL;
	new[len] = '\0';
	返回 (char *) memcpy(new, s, len);
}
#endif

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
整数 ul_strtos64(const char *str, int64_t *num, 整数 base)
{
	char *end = NULL;

	如果 (str == NULL || *str == '\0')
		返回 -(errno = EINVAL);

	errno = 0;
	*num = (int64_t) strtoimax(str, &end, base);

	如果 (errno != 0)
		返回 -errno;
	如果 (str == end || (end && *end))
		返回 -(errno = EINVAL);
	返回 0;
}

整数 ul_strtou64(const char *str, uint64_t *num, 整数 base)
{
	char *end = NULL;
	int64_t tmp;

	如果 (str == NULL || *str == '\0')
		返回 -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &end, base);
	如果 (tmp < 0)
		errno = ERANGE;
	否则 {
		errno = 0;
		*num = strtoumax(str, &end, base);
	}

	如果 (errno != 0)
		返回 -errno;
	如果 (str == end || (end && *end))
		返回 -(errno = EINVAL);
	返回 0;
}

整数 ul_strtos32(const char *str, int32_t *num, 整数 base)
{
	int64_t tmp;
	整数 rc;

	rc = ul_strtos64(str, &tmp, base);
	如果 (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	如果 (rc == 0)
		*num = (int32_t) tmp;
	返回 rc;
}

整数 ul_strtou32(const char *str, uint32_t *num, 整数 base)
{
	uint64_t tmp;
	整数 rc;

	rc = ul_strtou64(str, &tmp, base);
	如果 (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	如果 (rc == 0)
		*num = (uint32_t) tmp;
	返回 rc;
}

整数 ul_strtou16(const char *str, uint16_t *num, 整数 base)
{
	uint64_t tmp;
	整数 rc;

	rc = ul_strtou64(str, &tmp, base);
	如果 (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	如果 (rc == 0)
		*num = (uint16_t) tmp;
	返回 rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(const char *str, 整数 base, const char *errmesg,
			     int64_t low, int64_t up)
{
	int64_t num = 0;
	整数 rc;

	rc = ul_strtos64(str, &num, base);
	如果 (rc == 0 && ((low && num < low) || (up && num > up)))
		rc = -(errno = ERANGE);

	如果 (rc) {
		如果 (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	返回 num;
}

uint64_t str2unum_or_err(const char *str, 整数 base, const char *errmesg, uint64_t up)
{
	uint64_t num = 0;
	整数 rc;

	rc = ul_strtou64(str, &num, base);
	如果 (rc == 0 && (up && num > up))
		rc = -(errno = ERANGE);

	如果 (rc) {
		如果 (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	返回 num;
}

double strtod_or_err(const char *str, const char *errmesg)
{
	double num;
	char *end = NULL;

	errno = 0;
	如果 (str == NULL || *str == '\0')
		goto err;
	num = strtod(str, &end);

	如果 (errno || str == end || (end && *end))
		goto err;

	返回 num;
err:
	如果 (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

整数 ul_strtold(const char *str, long double *num)
{
	char *end = NULL;

	errno = 0;
	如果 (str == NULL || *str == '\0')
		返回 -(errno = EINVAL);
	*num = strtold(str, &end);

	如果 (errno != 0)
		返回 -errno;
	如果 (str == end || (end && *end))
		返回 -(errno = EINVAL);
	返回 0;
}

long double strtold_or_err(const char *str, const char *errmesg)
{
	long double num = 0;

	如果 (ul_strtold(str, &num) == 0)
		返回 num;
	如果 (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(const char *str, const char *errmesg)
{
	uintmax_t num;

	如果 (strtosize(str, &num) == 0)
		返回 num;

	如果 (errno)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}


void strtotimeval_or_err(const char *str, struct timeval *tv, const char *errmesg)
{
	long double user_input;

	user_input = strtold_or_err(str, errmesg);
	tv->tv_sec = (time_t) user_input;
	tv->tv_usec = (suseconds_t)((user_input - tv->tv_sec) * USEC_PER_SEC);
}

void strtotimespec_or_err(const char *str, struct timespec *ts, const char *errmesg)
{
	long double user_input;

	user_input = strtold_or_err(str, errmesg);
	ts->tv_sec = (time_t) user_input;
	ts->tv_nsec = (long)((user_input - ts->tv_sec) * NSEC_PER_SEC);
}

time_t strtotime_or_err(const char *str, const char *errmesg)
{
	int64_t user_input;

	user_input = strtos64_or_err(str, errmesg);
	返回 (time_t) user_input;
}

bool hyperlinkwanted(const char *mode)
{
	如果 (mode && strcmp(mode, "never") == 0)
		返回 false;

	如果 (mode && strcmp(mode, "always") == 0)
		返回 true;

	如果 (!mode || strcmp(mode, "auto") == 0)
		返回 isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), mode);
}

bool annotationwanted(const char *mode)
{
	如果 (mode && strcmp(mode, "never") == 0)
		返回 false;

	如果 (mode && strcmp(mode, "always") == 0)
		返回 true;

	如果 (!mode || strcmp(mode, "auto") == 0)
		返回 isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), mode);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
char *xstrmode(mode_t mode, char *str)
{
	unsigned short i = 0;

	如果 (S_ISDIR(mode))
		str[i++] = 'd';
	否则 如果 (S_ISLNK(mode))
		str[i++] = 'l';
	否则 如果 (S_ISCHR(mode))
		str[i++] = 'c';
	否则 如果 (S_ISBLK(mode))
		str[i++] = 'b';
	否则 如果 (S_ISSOCK(mode))
		str[i++] = 's';
	否则 如果 (S_ISFIFO(mode))
		str[i++] = 'p';
	否则 如果 (S_ISREG(mode))
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

	返回 str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
static 整数 get_exp(uint64_t n)
{
	整数 shft;

	为 (shft = 10; shft <= 60; shft += 10) {
		如果 (n < (1ULL << shft))
			break;
	}
	返回 shft - 10;
}

char *size_to_human_string(整数 options, uint64_t bytes)
{
	char buf[32];
	整数 dec, exp;
	uint64_t frac;
	const char *letters = "BKMGTPE";
	char suffix[sizeof(" KiB")], *psuf = suffix;
	char c;

	如果 (options & SIZE_SUFFIX_SPACE)
		*psuf++ = ' ';


	exp  = get_exp(bytes);
	c    = *(letters + (exp ? exp / 10 : 0));
	dec  = exp ? bytes / (1ULL << exp) : bytes;
	frac = exp ? bytes % (1ULL << exp) : 0;

	*psuf++ = c;

	如果 ((options & SIZE_SUFFIX_3LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/* round */
	如果 (frac) {
		/* get 3 digits after decimal point */
		如果 (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (exp - 10)) ;
		否则
			frac = (frac * 1000) / (1ULL << (exp)) ;

		如果 (options & SIZE_DECIMAL_2DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} 否则 {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		如果 (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	如果 (frac) {
		struct lconv const *l = localeconv();
		char *dp = l ? l->decimal_point : NULL;
		整数 len;

		如果 (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, sizeof(buf), "%d%s%02" PRIu64, dec, dp, frac);
		如果 (len > 0 && (size_t) len < sizeof(buf)) {
			/* remove potential extraneous zero */
			如果 (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, sizeof(buf) - len);
		} 否则
			*buf = '\0';	/* snprintf error */
	} 否则
		snprintf(buf, sizeof(buf), "%d%s", dec, suffix);

	返回 strdup(buf);
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
整数 string_to_idarray(const char *list, 整数 ary[], size_t arysz,
			整数 (name2id)(const char *, size_t))
{
	const char *begin = NULL, *p;
	size_t n = 0;

	如果 (!list || !*list || !ary || !arysz || !name2id)
		返回 -1;

	为 (p = list; p && *p; p++) {
		const char *end = NULL;
		整数 id;

		如果 (n >= arysz)
			返回 -2;
		如果 (!begin)
			begin = p;		/* begin of the column name */
		如果 (*p == ',')
			end = p;		/* terminate the name */
		如果 (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		如果 (!begin || !end)
			continue;
		如果 (end <= begin)
			返回 -1;

		id = name2id(begin, end - begin);
		如果 (id == -1)
			返回 -1;
		ary[ n++ ] = id;
		begin = NULL;
		如果 (end && !*end)
			break;
	}
	返回 n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
整数 string_add_to_idarray(const char *list, 整数 ary[], size_t arysz,
			size_t *ary_pos, 整数 (name2id)(const char *, size_t))
{
	const char *list_add;
	整数 r;

	如果 (!list || !*list || !ary_pos || *ary_pos > arysz)
		返回 -1;

	如果 (list[0] == '+')
		list_add = &list[1];
	否则 {
		list_add = list;
		*ary_pos = 0;
	}

	r = string_to_idarray(list_add, &ary[*ary_pos], arysz - *ary_pos, name2id);
	如果 (r > 0)
		*ary_pos += r;
	返回 r;
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
整数 string_to_bitarray(const char *list,
		     char *ary,
		     整数 (*name2bit)(const char *, size_t),
		     size_t allow_range)
{
	const char *begin = NULL, *p;

	如果 (!list || !name2bit || !ary)
		返回 -EINVAL;

	为 (p = list; p && *p; p++) {
		const char *end = NULL;
		整数 bit, set_lower = 0, set_higher = 0;

		如果 (!begin)
			begin = p;		/* begin of the level name */
		如果 (*p == ',')
			end = p;		/* terminate the name */
		如果 (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		如果 (!begin || !end)
			continue;
		如果 (end <= begin)
			返回 -1;
		如果 (allow_range) {
			如果 (*(end - 1) == '+') {
				end--;
				set_lower = 1;
			} 否则 如果 (*begin == '+') {
				begin++;
				set_higher = 1;
			}
		}

		bit = name2bit(begin, end - begin);
		如果 (bit < 0)
			返回 bit;
		setbit(ary, bit);
		如果 (set_lower)
			当 (--bit >= 0)
				setbit(ary, bit);
		否则 如果 (set_higher)
			当 (++bit < (整数) allow_range)
				setbit(ary, bit);
		begin = NULL;
		如果 (end && !*end)
			break;
	}
	返回 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
整数 string_to_bitmask(const char *list,
		     unsigned long *mask,
		     long (*name2flag)(const char *, size_t))
{
	const char *begin = NULL, *p;

	如果 (!list || !name2flag || !mask)
		返回 -EINVAL;

	为 (p = list; p && *p; p++) {
		const char *end = NULL;
		long flag;

		如果 (!begin)
			begin = p;		/* begin of the level name */
		如果 (*p == ',')
			end = p;		/* terminate the name */
		如果 (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		如果 (!begin || !end)
			continue;
		如果 (end <= begin)
			返回 -1;

		flag = name2flag(begin, end - begin);
		如果 (flag < 0)
			返回 flag;	/* error */
		*mask |= flag;
		begin = NULL;
		如果 (end && !*end)
			break;
	}
	返回 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
整数 ul_parse_range(const char *str, 整数 *lower, 整数 *upper, 整数 def)
{
	char *end = NULL;

	如果 (!str)
		返回 0;

	*upper = *lower = def;
	errno = 0;

	如果 (*str == ':') {				/* <:N> */
		str++;
		*upper = strtol(str, &end, 10);
		如果 (errno || !end || *end || end == str)
			返回 -1;
	} 否则 {
		*upper = *lower = strtol(str, &end, 10);
		如果 (errno || !end || end == str)
			返回 -1;

		如果 (*end == ':' && !*(end + 1))		/* <M:> */
			*upper = def;
		否则 如果 (*end == '-' || *end == ':') {	/* <M:N> <M-N> */
			str = end + 1;
			end = NULL;
			errno = 0;
			*upper = strtol(str, &end, 10);

			如果 (errno || !end || *end || end == str)
				返回 -1;
		}
	}
	返回 0;
}

const char *ul_path_next_segment(const char *str, size_t *sz)
{
	const char *start, *p;

	start = str;
	*sz = 0;
	当 (start && *start == '/' && *(start + 1) == '/')
		start++;

	如果 (!start || !*start)
		返回 NULL;

	为 (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	返回 start;
}

整数 streq_paths(const char *a, const char *b)
{
	当 (a && b) {
		size_t a_sz, b_sz;
		const char *a_seg = ul_path_next_segment(a, &a_sz);
		const char *b_seg = ul_path_next_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		如果 (a_sz + b_sz == 0)
			返回 1;

		/* ignore trailing slash */
		如果 (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			返回 1;

		如果 (!a_seg || !b_seg)
			break;
		如果 (a_sz != b_sz || strncmp(a_seg, b_seg, a_sz) != 0)
			break;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	返回 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
整数 ul_path_is_within(const char *path, const char *dir)
{
	如果 (!path || !dir || !*path || !*dir)
		返回 0;

	当 (path && dir) {
		size_t p_sz, d_sz;
		const char *p_seg = ul_path_next_segment(path, &p_sz);
		const char *d_seg = ul_path_next_segment(dir, &d_sz);

		/* all @dir segments matched */
		如果 (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			返回 1;

		如果 (!p_seg || !d_seg)
			break;
		如果 (p_sz != d_sz || strncmp(p_seg, d_seg, p_sz) != 0)
			break;

		path = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	返回 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
char *ul_strnconcat(const char *s, const char *suffix, size_t b)
{
	size_t a;
	char *r;

	如果 (!s && !suffix)
		返回 strdup("");
	如果 (!s)
		返回 strndup(suffix, b);
	如果 (!suffix)
		返回 strdup(s);

	assert(s);
	assert(suffix);

	a = strlen(s);
	如果 (b > ((size_t) -1) - a)
		返回 NULL;

	r = malloc(a + b + 1);
	如果 (!r)
		返回 NULL;

	memcpy(r, s, a);
	memcpy(r + a, suffix, b);
	r[a+b] = 0;

	返回 r;
}

/* concatenate two strings to a new string */
char *ul_strconcat(const char *s, const char *suffix)
{
	返回 ul_strnconcat(s, suffix, suffix ? strlen(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
char *ul_strfconcat(const char *s, const char *format, ...)
{
	va_list ap;
	char *val, *res;
	整数 sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	如果 (sz < 0)
		返回 NULL;

	res = ul_strnconcat(s, val, sz);
	free(val);
	返回 res;
}

整数 ul_strappend(char **a, const char *b)
{
	size_t al, bl;
	char *tmp;

	如果 (!a)
		返回 -EINVAL;
	如果 (!b || !*b)
		返回 0;
	如果 (!*a) {
		*a = strdup(b);
		返回 !*a ? -ENOMEM : 0;
	}

	al = strlen(*a);
	bl = strlen(b);

	tmp = realloc(*a, al + bl + 1);
	如果 (!tmp)
		返回 -ENOMEM;
	*a = tmp;
	memcpy((*a) + al, b, bl + 1);
	返回 0;
}

/* the hybrid version of strfconcat and strappend. */
整数 strfappend(char **a, const char *format, ...)
{
	va_list ap;
	整数 res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	返回 res;
}

extern 整数 ul_strvfappend(char **a, const char *format, va_list ap)
{
	char *val;
	整数 sz;
	整数 res;

	sz = vasprintf(&val, format, ap);
	如果 (sz < 0)
		返回 -errno;

	res = ul_strappend(a, val);
	free(val);
	返回 res;
}

static size_t strcspn_escaped(const char *s, const char *reject)
{
	整数 escaped = 0;
	整数 n;

	为 (n=0; s[n]; n++) {
		如果 (escaped)
			escaped = 0;
		否则 如果 (s[n] == '\\')
			escaped = 1;
		否则 如果 (strchr(reject, s[n]))
			break;
	}

	/* if s ends in \, return index of previous char */
	返回 n - escaped;
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
char *ul_strchr_escaped(const char *s, 整数 c)
{
	char *p;
	整数 esc = 0;

	为 (p = (char *) s; p && *p; p++) {
		如果 (!esc && *p == '\\') {
			esc = 1;
			continue;
		}
		如果 (*p == c && (!esc || c == '\\'))
			返回 p;
		esc = 0;
	}

	返回 NULL;
}

/* Split a string into words. */
const char *ul_split(const char **state, size_t *l, const char *separator, 整数 quoted)
{
	const char *current;

	current = *state;

	如果 (!*current) {
		assert(**state == '\0');
		返回 NULL;
	}

	current += strspn(current, separator);
	如果 (!*current) {
		*state = current;
		返回 NULL;
	}

	如果 (quoted && strchr("\'\"", *current)) {
		char quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		如果 (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			返回 NULL;
		}
		*state = current++ + *l + 2;
	} 否则 如果 (quoted) {
		*l = strcspn_escaped(current, separator);
		如果 (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			返回 NULL;
		}
		*state = current + *l;
	} 否则 {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	返回 current;
}

/* Rewind file pointer forward to new line.  */
整数 skip_fline(FILE *fp)
{
	整数 ch;

	do {
		如果 ((ch = fgetc(fp)) == EOF)
			返回 1;
		如果 (ch == '\n')
			返回 0;
	} 当 (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
整数 ul_stralnumcmp(const char *p1, const char *p2)
{
	const unsigned char *s1 = (const unsigned char *) p1;
	const unsigned char *s2 = (const unsigned char *) p2;
	unsigned char c1, c2;

	do {
		do {
			c1 = (unsigned char) *s1++;
		} 当 (c1 != '\0' && !isalnum((unsigned 整数) c1));

		do {
			c2 = (unsigned char) *s2++;
		} 当 (c2 != '\0' && !isalnum((unsigned 整数) c2));

		如果 (c1 != '\0')
			c1 = tolower(c1);
		如果 (c2 != '\0')
			c2 = tolower(c2);
		如果 (c1 == '\0')
			返回 c1 - c2;
	} 当 (c1 == c2);

	返回 c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
整数 ul_optstr_next(char **optstr, char **name, size_t *namesz,
		   char **value, size_t *valsz)
{
	整数 open_quote = 0;
	char *start = NULL, *stop = NULL, *p, *sep = NULL;
	char *optstr0;

	assert(optstr);
	assert(*optstr);

	optstr0 = *optstr;

	如果 (name)
		*name = NULL;
	如果 (namesz)
		*namesz = 0;
	如果 (value)
		*value = NULL;
	如果 (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	当 (optstr0 && *optstr0 == ',')
		optstr0++;

	为 (p = optstr0; p && *p; p++) {
		如果 (!start && *p == '=')
			返回 -EINVAL;
		如果 (!start)
			start = p;		/* beginning of the option item */
		如果 (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		如果 (open_quote)
			continue;		/* still in quoted block */
		如果 (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		如果 (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		否则 如果 (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		如果 (!start || !stop)
			continue;
		如果 (stop <= start)
			返回 -EINVAL;

		如果 (name)
			*name = start;
		如果 (namesz)
			*namesz = sep ? sep - start : stop - start;
		*optstr = *stop ? stop + 1 : stop;

		如果 (sep) {
			如果 (value)
				*value = sep + 1;
			如果 (valsz)
				*valsz = stop - sep - 1;
		}
		返回 0;
	}

	返回 1;				/* end of optstr */
}

整数 ul_optstr_is_valid(const char *optstr)
{
	整数 rc;
	char *p = (char *) optstr;

	当 ((rc = ul_optstr_next(&p, NULL, NULL, NULL, NULL)) == 0);
	返回 rc < 0 ? 0 : 1;
}

char *ul_optstr_get_value(const char *optstr, const char *key)
{
	size_t sz, namesz = 0, valsz = 0;
	char *name = NULL, *value = NULL;
	char *p = (char *) optstr;

	如果 (!optstr || !key || !*key)
		返回 NULL;

	sz = strlen(key);
	当 (ul_optstr_next(&p, &name, &namesz, &value, &valsz) == 0) {
		如果 (namesz != sz || !valsz)
			continue;
		如果 (strncmp(name, key, namesz) == 0)
			返回 strndup(value, valsz);
	}
	返回 NULL;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(char *str)
{
	size_t len;
	char q;

	如果 (!str || !*str)
		返回 0;

	len = strlen(str);
	q = *str;

	如果 ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		返回 len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	返回 len - 2;
}

#ifdef TEST_PROGRAM_STRUTILS

struct testS {
	char *name;
	char *value;
};

static 整数 test_strdup_to_member(整数 argc, char *argv[])
{
	struct testS *xx;

	如果 (argc < 3)
		返回 EXIT_FAILURE;

	xx = calloc(1, sizeof(*xx));
	如果 (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_to_struct_member(xx, name, argv[1]);
	strdup_to_struct_member(xx, value, argv[2]);

	如果 (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->value, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	printf("1: '%s', 2: '%s'\n", xx->name, xx->value);

	free(xx->name);
	free(xx->value);
	free(xx);
	返回 EXIT_SUCCESS;
}

static 整数 test_strutils_sizes(整数 argc, char *argv[])
{
	uintmax_t size = 0;
	char *hum1, *hum2, *hum3;

	如果 (argc < 2)
		返回 EXIT_FAILURE;

	如果 (strtosize(argv[1], &size))
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

	返回 EXIT_SUCCESS;
}

static 整数 test_strutils_cmp_paths(整数 argc, char *argv[])
{
	整数 rc = streq_paths(argv[1], argv[2]);

	如果 (argc < 3)
		返回 EXIT_FAILURE;

	printf("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	返回 EXIT_SUCCESS;
}

static 整数 test_strutils_path_is_within(整数 argc, char *argv[])
{
	整数 rc;

	如果 (argc < 3)
		返回 EXIT_FAILURE;

	rc = ul_path_is_within(argv[1], argv[2]);

	printf("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	返回 EXIT_SUCCESS;
}

static 整数 test_strutils_normalize(整数 argc, char *argv[])
{
	unsigned char *src, *dst, *org;
	size_t sz, len;

	如果 (argc < 2)
		返回 EXIT_FAILURE;

	org = (unsigned char *) strdup(argv[1]);
	src = (unsigned char *) strdup((char *) org);
	len = strlen((char *) src);
	dst = malloc(len + 1);

	如果 (!org || !src || !dst)
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

	返回 EXIT_SUCCESS;
}

static 整数 test_strutils_cstrcasecmp(整数 argc, char *argv[])
{
	char *a, *b;

	如果 (argc < 3)
		返回 EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	如果 (!a || !b)
		返回 EXIT_FAILURE;

	printf("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	printf("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	printf("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, strlen(a)));

	返回 EXIT_SUCCESS;
}

整数 主要(整数 argc, char *argv[])
{
	如果 (argc == 3 && strcmp(argv[1], "--size") == 0) {
		返回 test_strutils_sizes(argc - 1, argv + 1);

	} 否则 如果 (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		printf("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						NULL));
		返回 EXIT_SUCCESS;
	} 否则 如果 (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		如果 (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		printf("'%s'-->%s\n", argv[2], val ? "true" : "false");
		返回 EXIT_SUCCESS;

	} 否则 如果 (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		返回 test_strutils_cmp_paths(argc - 1, argv + 1);

	} 否则 如果 (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		返回 test_strutils_path_is_within(argc - 1, argv + 1);

	} 否则 如果 (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		返回 test_strdup_to_member(argc - 1, argv + 1);

	} 否则 如果  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		printf("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		返回 EXIT_SUCCESS;

	} 否则 如果 (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		返回 test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} 否则 如果 (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		返回 test_strutils_normalize(argc - 1, argv + 1);

	} 否则 如果 (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		printf("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		返回 EXIT_SUCCESS;
	} 否则 如果 (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		printf("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		返回 EXIT_SUCCESS;
	} 否则 如果 (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		printf("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		返回 EXIT_SUCCESS;
	} 否则 如果 (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		printf("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		返回 EXIT_SUCCESS;
	} 否则 如果 (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		printf("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		返回 EXIT_SUCCESS;
	} 否则 如果 (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		printf("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		返回 EXIT_SUCCESS;

	} 否则 如果 (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		printf("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		返回 EXIT_SUCCESS;

	} 否则 如果 (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		char *buf = "abc\0Y\0\0xyz\0X";
		char *end = buf + 12;
		char *p = buf;

		do {
			printf("str: '%s'\n", p);
		} 当 ((p = ul_next_string(p, end)));

		返回 EXIT_SUCCESS;

	} 否则 如果 (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		char *name = NULL, *val = NULL;
		char *p = argv[2];
		整数 rc;

		如果 (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		当 ((rc = ul_optstr_next(&p, &name, &namesz, &val, &valsz)) == 0) {
			printf("'%.*s' : '%.*s'\n", (整数) namesz, name,
						   (整数) valsz, val);
		}
		如果 (rc == 1)
			返回 EXIT_SUCCESS;
	} 否则 如果 (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		char *unquoted = strdup(argv[2]);

		如果 (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		printf("%s-->%s\n", argv[2], unquoted);
		free(unquoted);
		返回 EXIT_SUCCESS;

	} 否则 {
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

	返回 EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_STRUTILS */
