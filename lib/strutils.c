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

static 정수 STRTOXX_EXIT_CODE = EXIT_FAILURE;

void strutils_set_exitcode(정수 ex)
{
	STRTOXX_EXIT_CODE = ex;
}

static 정수 do_scale_by_power (uintmax_t *x, 정수 base, 정수 power)
{
	동안 (power--) {
		만약 (UINTMAX_MAX / base < *x)
			반환 -ERANGE;
		*x *= base;
	}
	반환 0;
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
정수 ul_parse_size(const char *str, uintmax_t *res, 정수 *power)
{
	const char *p;
	char *end;
	uintmax_t x, frac = 0;
	정수 base = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	static const char *suf  = "KMGTPEZY";
	static const char *suf2 = "kmgtpezy";
	const char *sp;

	*res = 0;

	만약 (!str || !*str) {
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
	동안 (isspace((unsigned char) *p))
		p++;
	만약 (*p == '-') {
		rc = -EINVAL;
		goto err;
	}

	errno = 0, end = NULL;
	x = strtoumax(str, &end, 0);

	만약 (end == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		goto err;
	}
	만약 (!end || !*end)
		goto done;			/* without suffix */
	p = end;

	/*
	 * Check size suffixes
	 */
check_suffix:
	만약 (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		base = 1024;			/* XiB, 2^N */
	아니면 만약 ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		base = 1000;			/* XB, 10^N */
	아니면 만약 (*(p + 1)) {
		struct lconv const *l = localeconv();
		const char *dp = l ? l->decimal_point : NULL;
		size_t dpsz = dp ? strlen(dp) : 0;

		만약 (frac == 0 && *p && dp && strncmp(dp, p, dpsz) == 0) {
			const char *fstr = p + dpsz;

			위해 (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			만약 (isdigit(*fstr)) {
				errno = 0, end = NULL;
				frac = strtoumax(fstr, &end, 0);
				만약 (end == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					goto err;
				}
			} 아니면
				end = (char *) p;

			만약 (!end || !*end) {
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
	만약 (sp)
		pwr = (sp - suf) + 1;
	아니면 {
		sp = strchr(suf2, *p);
		만약 (sp)
			pwr = (sp - suf2) + 1;
		아니면 {
			rc = -EINVAL;
			goto err;
		}
	}

	만약 (power)
		*power = pwr;

	rc = do_scale_by_power(&x, base, pwr);
	만약 (rc)
		goto err;

	만약 (frac && pwr) {
		정수 i;
		uintmax_t frac_base = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_base, base, pwr);
		만약 (rc)
			goto err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		do {
			frac_val = ((frac % 10) * frac_base + frac_val) / 10;
			frac /= 10;
		} 동안 (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		위해 (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		만약 (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			goto err;
		}
		x += frac_val;
	}
done:
	*res = x;
err:
	만약 (rc < 0)
		errno = -rc;
	반환 rc;
}

정수 strtosize(const char *str, uintmax_t *res)
{
	반환 ul_parse_size(str, res, NULL);
}

정수 isdigit_strend(const char *str, const char **end)
{
	const char *p;

	위해 (p = str; p && *p && isdigit((unsigned char) *p); p++);

	만약 (end)
		*end = p;
	반환 p && p > str && !*p;
}

정수 isxdigit_strend(const char *str, const char **end)
{
	const char *p;

	위해 (p = str; p && *p && isxdigit((unsigned char) *p); p++);

	만약 (end)
		*end = p;

	반환 p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
정수 ul_strtobool(const char *str, bool *result)
{
	static const char *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	static const char *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	만약 (!str || !result)
		반환 -(errno = EINVAL);

	errno = 0;

	위해 (i = 0; i < ARRAY_SIZE(bool_true); i++) {
		만약 (c_strcasecmp(str, bool_true[i]) == 0) {
			*result = true;
			반환 0;
		}
	}

	위해 (i = 0; i < ARRAY_SIZE(bool_false); i++) {
		만약 (c_strcasecmp(str, bool_false[i]) == 0) {
			*result = false;
			반환 0;
		}
	}

	반환 -(errno = EINVAL);
}

정수 ul_parse_switch(const char *arg, ...)
{
	const char *a, *b;
	va_list ap;

	va_start(ap, arg);
	do {
		a = va_arg(ap, char *);
		만약 (!a)
			break;
		b = va_arg(ap, char *);
		만약 (!b)
			break;

		만약 (strcmp(arg, a) == 0) {
			va_end(ap);
			반환 1;
		}

		만약 (strcmp(arg, b) == 0) {
			va_end(ap);
			반환 0;
		}
	} 동안 (1);
	va_end(ap);

	errx(STRTOXX_EXIT_CODE, _("unsupported argument: %s"), arg);
}

#ifndef HAVE_MEMPCPY
void *mempcpy(void *restrict dest, const void *restrict src, size_t n)
{
	반환 ((char *)memcpy(dest, src, n)) + n;
}
#endif

#ifndef HAVE_STRNLEN
size_t strnlen(const char *s, size_t maxlen)
{
	size_t i;

	위해 (i = 0; i < maxlen; i++) {
		만약 (s[i] == '\0')
			반환 i;
	}
	반환 maxlen;
}
#endif

#ifndef HAVE_STRNCHR
char *strnchr(const char *s, size_t maxlen, 정수 c)
{
	위해 (; maxlen-- && *s != '\0'; ++s)
		만약 (*s == (char)c)
			반환 (char *)s;
	반환 NULL;
}
#endif

#ifndef HAVE_STRNDUP
char *strndup(const char *s, size_t n)
{
	size_t len = strnlen(s, n);
	char *new = malloc((len + 1) * sizeof(char));
	만약 (!new)
		반환 NULL;
	new[len] = '\0';
	반환 (char *) memcpy(new, s, len);
}
#endif

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
정수 ul_strtos64(const char *str, int64_t *num, 정수 base)
{
	char *end = NULL;

	만약 (str == NULL || *str == '\0')
		반환 -(errno = EINVAL);

	errno = 0;
	*num = (int64_t) strtoimax(str, &end, base);

	만약 (errno != 0)
		반환 -errno;
	만약 (str == end || (end && *end))
		반환 -(errno = EINVAL);
	반환 0;
}

정수 ul_strtou64(const char *str, uint64_t *num, 정수 base)
{
	char *end = NULL;
	int64_t tmp;

	만약 (str == NULL || *str == '\0')
		반환 -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &end, base);
	만약 (tmp < 0)
		errno = ERANGE;
	아니면 {
		errno = 0;
		*num = strtoumax(str, &end, base);
	}

	만약 (errno != 0)
		반환 -errno;
	만약 (str == end || (end && *end))
		반환 -(errno = EINVAL);
	반환 0;
}

정수 ul_strtos32(const char *str, int32_t *num, 정수 base)
{
	int64_t tmp;
	정수 rc;

	rc = ul_strtos64(str, &tmp, base);
	만약 (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	만약 (rc == 0)
		*num = (int32_t) tmp;
	반환 rc;
}

정수 ul_strtou32(const char *str, uint32_t *num, 정수 base)
{
	uint64_t tmp;
	정수 rc;

	rc = ul_strtou64(str, &tmp, base);
	만약 (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	만약 (rc == 0)
		*num = (uint32_t) tmp;
	반환 rc;
}

정수 ul_strtou16(const char *str, uint16_t *num, 정수 base)
{
	uint64_t tmp;
	정수 rc;

	rc = ul_strtou64(str, &tmp, base);
	만약 (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	만약 (rc == 0)
		*num = (uint16_t) tmp;
	반환 rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(const char *str, 정수 base, const char *errmesg,
			     int64_t low, int64_t up)
{
	int64_t num = 0;
	정수 rc;

	rc = ul_strtos64(str, &num, base);
	만약 (rc == 0 && ((low && num < low) || (up && num > up)))
		rc = -(errno = ERANGE);

	만약 (rc) {
		만약 (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	반환 num;
}

uint64_t str2unum_or_err(const char *str, 정수 base, const char *errmesg, uint64_t up)
{
	uint64_t num = 0;
	정수 rc;

	rc = ul_strtou64(str, &num, base);
	만약 (rc == 0 && (up && num > up))
		rc = -(errno = ERANGE);

	만약 (rc) {
		만약 (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	반환 num;
}

double strtod_or_err(const char *str, const char *errmesg)
{
	double num;
	char *end = NULL;

	errno = 0;
	만약 (str == NULL || *str == '\0')
		goto err;
	num = strtod(str, &end);

	만약 (errno || str == end || (end && *end))
		goto err;

	반환 num;
err:
	만약 (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

정수 ul_strtold(const char *str, long double *num)
{
	char *end = NULL;

	errno = 0;
	만약 (str == NULL || *str == '\0')
		반환 -(errno = EINVAL);
	*num = strtold(str, &end);

	만약 (errno != 0)
		반환 -errno;
	만약 (str == end || (end && *end))
		반환 -(errno = EINVAL);
	반환 0;
}

long double strtold_or_err(const char *str, const char *errmesg)
{
	long double num = 0;

	만약 (ul_strtold(str, &num) == 0)
		반환 num;
	만약 (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(const char *str, const char *errmesg)
{
	uintmax_t num;

	만약 (strtosize(str, &num) == 0)
		반환 num;

	만약 (errno)
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
	반환 (time_t) user_input;
}

bool hyperlinkwanted(const char *mode)
{
	만약 (mode && strcmp(mode, "never") == 0)
		반환 false;

	만약 (mode && strcmp(mode, "always") == 0)
		반환 true;

	만약 (!mode || strcmp(mode, "auto") == 0)
		반환 isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), mode);
}

bool annotationwanted(const char *mode)
{
	만약 (mode && strcmp(mode, "never") == 0)
		반환 false;

	만약 (mode && strcmp(mode, "always") == 0)
		반환 true;

	만약 (!mode || strcmp(mode, "auto") == 0)
		반환 isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), mode);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
char *xstrmode(mode_t mode, char *str)
{
	unsigned short i = 0;

	만약 (S_ISDIR(mode))
		str[i++] = 'd';
	아니면 만약 (S_ISLNK(mode))
		str[i++] = 'l';
	아니면 만약 (S_ISCHR(mode))
		str[i++] = 'c';
	아니면 만약 (S_ISBLK(mode))
		str[i++] = 'b';
	아니면 만약 (S_ISSOCK(mode))
		str[i++] = 's';
	아니면 만약 (S_ISFIFO(mode))
		str[i++] = 'p';
	아니면 만약 (S_ISREG(mode))
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

	반환 str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
static 정수 get_exp(uint64_t n)
{
	정수 shft;

	위해 (shft = 10; shft <= 60; shft += 10) {
		만약 (n < (1ULL << shft))
			break;
	}
	반환 shft - 10;
}

char *size_to_human_string(정수 options, uint64_t bytes)
{
	char buf[32];
	정수 dec, exp;
	uint64_t frac;
	const char *letters = "BKMGTPE";
	char suffix[sizeof(" KiB")], *psuf = suffix;
	char c;

	만약 (options & SIZE_SUFFIX_SPACE)
		*psuf++ = ' ';


	exp  = get_exp(bytes);
	c    = *(letters + (exp ? exp / 10 : 0));
	dec  = exp ? bytes / (1ULL << exp) : bytes;
	frac = exp ? bytes % (1ULL << exp) : 0;

	*psuf++ = c;

	만약 ((options & SIZE_SUFFIX_3LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/* round */
	만약 (frac) {
		/* get 3 digits after decimal point */
		만약 (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (exp - 10)) ;
		아니면
			frac = (frac * 1000) / (1ULL << (exp)) ;

		만약 (options & SIZE_DECIMAL_2DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} 아니면 {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		만약 (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	만약 (frac) {
		struct lconv const *l = localeconv();
		char *dp = l ? l->decimal_point : NULL;
		정수 len;

		만약 (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, sizeof(buf), "%d%s%02" PRIu64, dec, dp, frac);
		만약 (len > 0 && (size_t) len < sizeof(buf)) {
			/* remove potential extraneous zero */
			만약 (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, sizeof(buf) - len);
		} 아니면
			*buf = '\0';	/* snprintf error */
	} 아니면
		snprintf(buf, sizeof(buf), "%d%s", dec, suffix);

	반환 strdup(buf);
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
정수 string_to_idarray(const char *list, 정수 ary[], size_t arysz,
			정수 (name2id)(const char *, size_t))
{
	const char *begin = NULL, *p;
	size_t n = 0;

	만약 (!list || !*list || !ary || !arysz || !name2id)
		반환 -1;

	위해 (p = list; p && *p; p++) {
		const char *end = NULL;
		정수 id;

		만약 (n >= arysz)
			반환 -2;
		만약 (!begin)
			begin = p;		/* begin of the column name */
		만약 (*p == ',')
			end = p;		/* terminate the name */
		만약 (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		만약 (!begin || !end)
			continue;
		만약 (end <= begin)
			반환 -1;

		id = name2id(begin, end - begin);
		만약 (id == -1)
			반환 -1;
		ary[ n++ ] = id;
		begin = NULL;
		만약 (end && !*end)
			break;
	}
	반환 n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
정수 string_add_to_idarray(const char *list, 정수 ary[], size_t arysz,
			size_t *ary_pos, 정수 (name2id)(const char *, size_t))
{
	const char *list_add;
	정수 r;

	만약 (!list || !*list || !ary_pos || *ary_pos > arysz)
		반환 -1;

	만약 (list[0] == '+')
		list_add = &list[1];
	아니면 {
		list_add = list;
		*ary_pos = 0;
	}

	r = string_to_idarray(list_add, &ary[*ary_pos], arysz - *ary_pos, name2id);
	만약 (r > 0)
		*ary_pos += r;
	반환 r;
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
정수 string_to_bitarray(const char *list,
		     char *ary,
		     정수 (*name2bit)(const char *, size_t),
		     size_t allow_range)
{
	const char *begin = NULL, *p;

	만약 (!list || !name2bit || !ary)
		반환 -EINVAL;

	위해 (p = list; p && *p; p++) {
		const char *end = NULL;
		정수 bit, set_lower = 0, set_higher = 0;

		만약 (!begin)
			begin = p;		/* begin of the level name */
		만약 (*p == ',')
			end = p;		/* terminate the name */
		만약 (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		만약 (!begin || !end)
			continue;
		만약 (end <= begin)
			반환 -1;
		만약 (allow_range) {
			만약 (*(end - 1) == '+') {
				end--;
				set_lower = 1;
			} 아니면 만약 (*begin == '+') {
				begin++;
				set_higher = 1;
			}
		}

		bit = name2bit(begin, end - begin);
		만약 (bit < 0)
			반환 bit;
		setbit(ary, bit);
		만약 (set_lower)
			동안 (--bit >= 0)
				setbit(ary, bit);
		아니면 만약 (set_higher)
			동안 (++bit < (정수) allow_range)
				setbit(ary, bit);
		begin = NULL;
		만약 (end && !*end)
			break;
	}
	반환 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
정수 string_to_bitmask(const char *list,
		     unsigned long *mask,
		     long (*name2flag)(const char *, size_t))
{
	const char *begin = NULL, *p;

	만약 (!list || !name2flag || !mask)
		반환 -EINVAL;

	위해 (p = list; p && *p; p++) {
		const char *end = NULL;
		long flag;

		만약 (!begin)
			begin = p;		/* begin of the level name */
		만약 (*p == ',')
			end = p;		/* terminate the name */
		만약 (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		만약 (!begin || !end)
			continue;
		만약 (end <= begin)
			반환 -1;

		flag = name2flag(begin, end - begin);
		만약 (flag < 0)
			반환 flag;	/* error */
		*mask |= flag;
		begin = NULL;
		만약 (end && !*end)
			break;
	}
	반환 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
정수 ul_parse_range(const char *str, 정수 *lower, 정수 *upper, 정수 def)
{
	char *end = NULL;

	만약 (!str)
		반환 0;

	*upper = *lower = def;
	errno = 0;

	만약 (*str == ':') {				/* <:N> */
		str++;
		*upper = strtol(str, &end, 10);
		만약 (errno || !end || *end || end == str)
			반환 -1;
	} 아니면 {
		*upper = *lower = strtol(str, &end, 10);
		만약 (errno || !end || end == str)
			반환 -1;

		만약 (*end == ':' && !*(end + 1))		/* <M:> */
			*upper = def;
		아니면 만약 (*end == '-' || *end == ':') {	/* <M:N> <M-N> */
			str = end + 1;
			end = NULL;
			errno = 0;
			*upper = strtol(str, &end, 10);

			만약 (errno || !end || *end || end == str)
				반환 -1;
		}
	}
	반환 0;
}

const char *ul_path_next_segment(const char *str, size_t *sz)
{
	const char *start, *p;

	start = str;
	*sz = 0;
	동안 (start && *start == '/' && *(start + 1) == '/')
		start++;

	만약 (!start || !*start)
		반환 NULL;

	위해 (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	반환 start;
}

정수 streq_paths(const char *a, const char *b)
{
	동안 (a && b) {
		size_t a_sz, b_sz;
		const char *a_seg = ul_path_next_segment(a, &a_sz);
		const char *b_seg = ul_path_next_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		만약 (a_sz + b_sz == 0)
			반환 1;

		/* ignore trailing slash */
		만약 (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			반환 1;

		만약 (!a_seg || !b_seg)
			break;
		만약 (a_sz != b_sz || strncmp(a_seg, b_seg, a_sz) != 0)
			break;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	반환 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
정수 ul_path_is_within(const char *path, const char *dir)
{
	만약 (!path || !dir || !*path || !*dir)
		반환 0;

	동안 (path && dir) {
		size_t p_sz, d_sz;
		const char *p_seg = ul_path_next_segment(path, &p_sz);
		const char *d_seg = ul_path_next_segment(dir, &d_sz);

		/* all @dir segments matched */
		만약 (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			반환 1;

		만약 (!p_seg || !d_seg)
			break;
		만약 (p_sz != d_sz || strncmp(p_seg, d_seg, p_sz) != 0)
			break;

		path = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	반환 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
char *ul_strnconcat(const char *s, const char *suffix, size_t b)
{
	size_t a;
	char *r;

	만약 (!s && !suffix)
		반환 strdup("");
	만약 (!s)
		반환 strndup(suffix, b);
	만약 (!suffix)
		반환 strdup(s);

	assert(s);
	assert(suffix);

	a = strlen(s);
	만약 (b > ((size_t) -1) - a)
		반환 NULL;

	r = malloc(a + b + 1);
	만약 (!r)
		반환 NULL;

	memcpy(r, s, a);
	memcpy(r + a, suffix, b);
	r[a+b] = 0;

	반환 r;
}

/* concatenate two strings to a new string */
char *ul_strconcat(const char *s, const char *suffix)
{
	반환 ul_strnconcat(s, suffix, suffix ? strlen(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
char *ul_strfconcat(const char *s, const char *format, ...)
{
	va_list ap;
	char *val, *res;
	정수 sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	만약 (sz < 0)
		반환 NULL;

	res = ul_strnconcat(s, val, sz);
	free(val);
	반환 res;
}

정수 ul_strappend(char **a, const char *b)
{
	size_t al, bl;
	char *tmp;

	만약 (!a)
		반환 -EINVAL;
	만약 (!b || !*b)
		반환 0;
	만약 (!*a) {
		*a = strdup(b);
		반환 !*a ? -ENOMEM : 0;
	}

	al = strlen(*a);
	bl = strlen(b);

	tmp = realloc(*a, al + bl + 1);
	만약 (!tmp)
		반환 -ENOMEM;
	*a = tmp;
	memcpy((*a) + al, b, bl + 1);
	반환 0;
}

/* the hybrid version of strfconcat and strappend. */
정수 strfappend(char **a, const char *format, ...)
{
	va_list ap;
	정수 res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	반환 res;
}

extern 정수 ul_strvfappend(char **a, const char *format, va_list ap)
{
	char *val;
	정수 sz;
	정수 res;

	sz = vasprintf(&val, format, ap);
	만약 (sz < 0)
		반환 -errno;

	res = ul_strappend(a, val);
	free(val);
	반환 res;
}

static size_t strcspn_escaped(const char *s, const char *reject)
{
	정수 escaped = 0;
	정수 n;

	위해 (n=0; s[n]; n++) {
		만약 (escaped)
			escaped = 0;
		아니면 만약 (s[n] == '\\')
			escaped = 1;
		아니면 만약 (strchr(reject, s[n]))
			break;
	}

	/* if s ends in \, return index of previous char */
	반환 n - escaped;
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
char *ul_strchr_escaped(const char *s, 정수 c)
{
	char *p;
	정수 esc = 0;

	위해 (p = (char *) s; p && *p; p++) {
		만약 (!esc && *p == '\\') {
			esc = 1;
			continue;
		}
		만약 (*p == c && (!esc || c == '\\'))
			반환 p;
		esc = 0;
	}

	반환 NULL;
}

/* Split a string into words. */
const char *ul_split(const char **state, size_t *l, const char *separator, 정수 quoted)
{
	const char *current;

	current = *state;

	만약 (!*current) {
		assert(**state == '\0');
		반환 NULL;
	}

	current += strspn(current, separator);
	만약 (!*current) {
		*state = current;
		반환 NULL;
	}

	만약 (quoted && strchr("\'\"", *current)) {
		char quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		만약 (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			반환 NULL;
		}
		*state = current++ + *l + 2;
	} 아니면 만약 (quoted) {
		*l = strcspn_escaped(current, separator);
		만약 (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			반환 NULL;
		}
		*state = current + *l;
	} 아니면 {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	반환 current;
}

/* Rewind file pointer forward to new line.  */
정수 skip_fline(FILE *fp)
{
	정수 ch;

	do {
		만약 ((ch = fgetc(fp)) == EOF)
			반환 1;
		만약 (ch == '\n')
			반환 0;
	} 동안 (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
정수 ul_stralnumcmp(const char *p1, const char *p2)
{
	const unsigned char *s1 = (const unsigned char *) p1;
	const unsigned char *s2 = (const unsigned char *) p2;
	unsigned char c1, c2;

	do {
		do {
			c1 = (unsigned char) *s1++;
		} 동안 (c1 != '\0' && !isalnum((unsigned 정수) c1));

		do {
			c2 = (unsigned char) *s2++;
		} 동안 (c2 != '\0' && !isalnum((unsigned 정수) c2));

		만약 (c1 != '\0')
			c1 = tolower(c1);
		만약 (c2 != '\0')
			c2 = tolower(c2);
		만약 (c1 == '\0')
			반환 c1 - c2;
	} 동안 (c1 == c2);

	반환 c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
정수 ul_optstr_next(char **optstr, char **name, size_t *namesz,
		   char **value, size_t *valsz)
{
	정수 open_quote = 0;
	char *start = NULL, *stop = NULL, *p, *sep = NULL;
	char *optstr0;

	assert(optstr);
	assert(*optstr);

	optstr0 = *optstr;

	만약 (name)
		*name = NULL;
	만약 (namesz)
		*namesz = 0;
	만약 (value)
		*value = NULL;
	만약 (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	동안 (optstr0 && *optstr0 == ',')
		optstr0++;

	위해 (p = optstr0; p && *p; p++) {
		만약 (!start && *p == '=')
			반환 -EINVAL;
		만약 (!start)
			start = p;		/* beginning of the option item */
		만약 (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		만약 (open_quote)
			continue;		/* still in quoted block */
		만약 (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		만약 (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		아니면 만약 (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		만약 (!start || !stop)
			continue;
		만약 (stop <= start)
			반환 -EINVAL;

		만약 (name)
			*name = start;
		만약 (namesz)
			*namesz = sep ? sep - start : stop - start;
		*optstr = *stop ? stop + 1 : stop;

		만약 (sep) {
			만약 (value)
				*value = sep + 1;
			만약 (valsz)
				*valsz = stop - sep - 1;
		}
		반환 0;
	}

	반환 1;				/* end of optstr */
}

정수 ul_optstr_is_valid(const char *optstr)
{
	정수 rc;
	char *p = (char *) optstr;

	동안 ((rc = ul_optstr_next(&p, NULL, NULL, NULL, NULL)) == 0);
	반환 rc < 0 ? 0 : 1;
}

char *ul_optstr_get_value(const char *optstr, const char *key)
{
	size_t sz, namesz = 0, valsz = 0;
	char *name = NULL, *value = NULL;
	char *p = (char *) optstr;

	만약 (!optstr || !key || !*key)
		반환 NULL;

	sz = strlen(key);
	동안 (ul_optstr_next(&p, &name, &namesz, &value, &valsz) == 0) {
		만약 (namesz != sz || !valsz)
			continue;
		만약 (strncmp(name, key, namesz) == 0)
			반환 strndup(value, valsz);
	}
	반환 NULL;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(char *str)
{
	size_t len;
	char q;

	만약 (!str || !*str)
		반환 0;

	len = strlen(str);
	q = *str;

	만약 ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		반환 len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	반환 len - 2;
}

#ifdef TEST_PROGRAM_STRUTILS

struct testS {
	char *name;
	char *value;
};

static 정수 test_strdup_to_member(정수 argc, char *argv[])
{
	struct testS *xx;

	만약 (argc < 3)
		반환 EXIT_FAILURE;

	xx = calloc(1, sizeof(*xx));
	만약 (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_to_struct_member(xx, name, argv[1]);
	strdup_to_struct_member(xx, value, argv[2]);

	만약 (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->value, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	printf("1: '%s', 2: '%s'\n", xx->name, xx->value);

	free(xx->name);
	free(xx->value);
	free(xx);
	반환 EXIT_SUCCESS;
}

static 정수 test_strutils_sizes(정수 argc, char *argv[])
{
	uintmax_t size = 0;
	char *hum1, *hum2, *hum3;

	만약 (argc < 2)
		반환 EXIT_FAILURE;

	만약 (strtosize(argv[1], &size))
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

	반환 EXIT_SUCCESS;
}

static 정수 test_strutils_cmp_paths(정수 argc, char *argv[])
{
	정수 rc = streq_paths(argv[1], argv[2]);

	만약 (argc < 3)
		반환 EXIT_FAILURE;

	printf("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	반환 EXIT_SUCCESS;
}

static 정수 test_strutils_path_is_within(정수 argc, char *argv[])
{
	정수 rc;

	만약 (argc < 3)
		반환 EXIT_FAILURE;

	rc = ul_path_is_within(argv[1], argv[2]);

	printf("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	반환 EXIT_SUCCESS;
}

static 정수 test_strutils_normalize(정수 argc, char *argv[])
{
	unsigned char *src, *dst, *org;
	size_t sz, len;

	만약 (argc < 2)
		반환 EXIT_FAILURE;

	org = (unsigned char *) strdup(argv[1]);
	src = (unsigned char *) strdup((char *) org);
	len = strlen((char *) src);
	dst = malloc(len + 1);

	만약 (!org || !src || !dst)
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

	반환 EXIT_SUCCESS;
}

static 정수 test_strutils_cstrcasecmp(정수 argc, char *argv[])
{
	char *a, *b;

	만약 (argc < 3)
		반환 EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	만약 (!a || !b)
		반환 EXIT_FAILURE;

	printf("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	printf("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	printf("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, strlen(a)));

	반환 EXIT_SUCCESS;
}

정수 main(정수 argc, char *argv[])
{
	만약 (argc == 3 && strcmp(argv[1], "--size") == 0) {
		반환 test_strutils_sizes(argc - 1, argv + 1);

	} 아니면 만약 (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		printf("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						NULL));
		반환 EXIT_SUCCESS;
	} 아니면 만약 (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		만약 (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		printf("'%s'-->%s\n", argv[2], val ? "true" : "false");
		반환 EXIT_SUCCESS;

	} 아니면 만약 (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		반환 test_strutils_cmp_paths(argc - 1, argv + 1);

	} 아니면 만약 (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		반환 test_strutils_path_is_within(argc - 1, argv + 1);

	} 아니면 만약 (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		반환 test_strdup_to_member(argc - 1, argv + 1);

	} 아니면 만약  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		printf("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		반환 EXIT_SUCCESS;

	} 아니면 만약 (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		반환 test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} 아니면 만약 (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		반환 test_strutils_normalize(argc - 1, argv + 1);

	} 아니면 만약 (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		printf("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		반환 EXIT_SUCCESS;
	} 아니면 만약 (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		printf("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		반환 EXIT_SUCCESS;
	} 아니면 만약 (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		printf("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		반환 EXIT_SUCCESS;
	} 아니면 만약 (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		printf("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		반환 EXIT_SUCCESS;
	} 아니면 만약 (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		printf("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		반환 EXIT_SUCCESS;
	} 아니면 만약 (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		printf("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		반환 EXIT_SUCCESS;

	} 아니면 만약 (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		printf("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		반환 EXIT_SUCCESS;

	} 아니면 만약 (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		char *buf = "abc\0Y\0\0xyz\0X";
		char *end = buf + 12;
		char *p = buf;

		do {
			printf("str: '%s'\n", p);
		} 동안 ((p = ul_next_string(p, end)));

		반환 EXIT_SUCCESS;

	} 아니면 만약 (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		char *name = NULL, *val = NULL;
		char *p = argv[2];
		정수 rc;

		만약 (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		동안 ((rc = ul_optstr_next(&p, &name, &namesz, &val, &valsz)) == 0) {
			printf("'%.*s' : '%.*s'\n", (정수) namesz, name,
						   (정수) valsz, val);
		}
		만약 (rc == 1)
			반환 EXIT_SUCCESS;
	} 아니면 만약 (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		char *unquoted = strdup(argv[2]);

		만약 (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		printf("%s-->%s\n", argv[2], unquoted);
		free(unquoted);
		반환 EXIT_SUCCESS;

	} 아니면 {
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

	반환 EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_STRUTILS */
