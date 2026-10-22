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

static цяло STRTOXX_EXIT_CODE = EXIT_FAILURE;

void strutils_set_exitcode(цяло ex)
{
	STRTOXX_EXIT_CODE = ex;
}

static цяло do_scale_by_power (uintmax_t *x, цяло base, цяло power)
{
	докато (power--) {
		ако (UINTMAX_MAX / base < *x)
			върни -ERANGE;
		*x *= base;
	}
	върни 0;
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
цяло ul_parse_size(const char *str, uintmax_t *res, цяло *power)
{
	const char *p;
	char *end;
	uintmax_t x, frac = 0;
	цяло base = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	static const char *suf  = "KMGTPEZY";
	static const char *suf2 = "kmgtpezy";
	const char *sp;

	*res = 0;

	ако (!str || !*str) {
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
	докато (isspace((unsigned char) *p))
		p++;
	ако (*p == '-') {
		rc = -EINVAL;
		goto err;
	}

	errno = 0, end = NULL;
	x = strtoumax(str, &end, 0);

	ако (end == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		goto err;
	}
	ако (!end || !*end)
		goto done;			/* without suffix */
	p = end;

	/*
	 * Check size suffixes
	 */
check_suffix:
	ако (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		base = 1024;			/* XiB, 2^N */
	иначе ако ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		base = 1000;			/* XB, 10^N */
	иначе ако (*(p + 1)) {
		struct lconv const *l = localeconv();
		const char *dp = l ? l->decimal_point : NULL;
		size_t dpsz = dp ? strlen(dp) : 0;

		ако (frac == 0 && *p && dp && strncmp(dp, p, dpsz) == 0) {
			const char *fstr = p + dpsz;

			за (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			ако (isdigit(*fstr)) {
				errno = 0, end = NULL;
				frac = strtoumax(fstr, &end, 0);
				ако (end == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					goto err;
				}
			} иначе
				end = (char *) p;

			ако (!end || !*end) {
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
	ако (sp)
		pwr = (sp - suf) + 1;
	иначе {
		sp = strchr(suf2, *p);
		ако (sp)
			pwr = (sp - suf2) + 1;
		иначе {
			rc = -EINVAL;
			goto err;
		}
	}

	ако (power)
		*power = pwr;

	rc = do_scale_by_power(&x, base, pwr);
	ако (rc)
		goto err;

	ако (frac && pwr) {
		цяло i;
		uintmax_t frac_base = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_base, base, pwr);
		ако (rc)
			goto err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		do {
			frac_val = ((frac % 10) * frac_base + frac_val) / 10;
			frac /= 10;
		} докато (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		за (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		ако (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			goto err;
		}
		x += frac_val;
	}
done:
	*res = x;
err:
	ако (rc < 0)
		errno = -rc;
	върни rc;
}

цяло strtosize(const char *str, uintmax_t *res)
{
	върни ul_parse_size(str, res, NULL);
}

цяло isdigit_strend(const char *str, const char **end)
{
	const char *p;

	за (p = str; p && *p && isdigit((unsigned char) *p); p++);

	ако (end)
		*end = p;
	върни p && p > str && !*p;
}

цяло isxdigit_strend(const char *str, const char **end)
{
	const char *p;

	за (p = str; p && *p && isxdigit((unsigned char) *p); p++);

	ако (end)
		*end = p;

	върни p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
цяло ul_strtobool(const char *str, bool *result)
{
	static const char *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	static const char *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	ако (!str || !result)
		върни -(errno = EINVAL);

	errno = 0;

	за (i = 0; i < ARRAY_SIZE(bool_true); i++) {
		ако (c_strcasecmp(str, bool_true[i]) == 0) {
			*result = true;
			върни 0;
		}
	}

	за (i = 0; i < ARRAY_SIZE(bool_false); i++) {
		ако (c_strcasecmp(str, bool_false[i]) == 0) {
			*result = false;
			върни 0;
		}
	}

	върни -(errno = EINVAL);
}

цяло ul_parse_switch(const char *arg, ...)
{
	const char *a, *b;
	va_list ap;

	va_start(ap, arg);
	do {
		a = va_arg(ap, char *);
		ако (!a)
			break;
		b = va_arg(ap, char *);
		ако (!b)
			break;

		ако (strcmp(arg, a) == 0) {
			va_end(ap);
			върни 1;
		}

		ако (strcmp(arg, b) == 0) {
			va_end(ap);
			върни 0;
		}
	} докато (1);
	va_end(ap);

	errx(STRTOXX_EXIT_CODE, _("unsupported argument: %s"), arg);
}

#ifndef HAVE_MEMPCPY
void *mempcpy(void *restrict dest, const void *restrict src, size_t n)
{
	върни ((char *)memcpy(dest, src, n)) + n;
}
#endif

#ifndef HAVE_STRNLEN
size_t strnlen(const char *s, size_t maxlen)
{
	size_t i;

	за (i = 0; i < maxlen; i++) {
		ако (s[i] == '\0')
			върни i;
	}
	върни maxlen;
}
#endif

#ifndef HAVE_STRNCHR
char *strnchr(const char *s, size_t maxlen, цяло c)
{
	за (; maxlen-- && *s != '\0'; ++s)
		ако (*s == (char)c)
			върни (char *)s;
	върни NULL;
}
#endif

#ifndef HAVE_STRNDUP
char *strndup(const char *s, size_t n)
{
	size_t len = strnlen(s, n);
	char *new = malloc((len + 1) * sizeof(char));
	ако (!new)
		върни NULL;
	new[len] = '\0';
	върни (char *) memcpy(new, s, len);
}
#endif

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
цяло ul_strtos64(const char *str, int64_t *num, цяло base)
{
	char *end = NULL;

	ако (str == NULL || *str == '\0')
		върни -(errno = EINVAL);

	errno = 0;
	*num = (int64_t) strtoimax(str, &end, base);

	ако (errno != 0)
		върни -errno;
	ако (str == end || (end && *end))
		върни -(errno = EINVAL);
	върни 0;
}

цяло ul_strtou64(const char *str, uint64_t *num, цяло base)
{
	char *end = NULL;
	int64_t tmp;

	ако (str == NULL || *str == '\0')
		върни -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &end, base);
	ако (tmp < 0)
		errno = ERANGE;
	иначе {
		errno = 0;
		*num = strtoumax(str, &end, base);
	}

	ако (errno != 0)
		върни -errno;
	ако (str == end || (end && *end))
		върни -(errno = EINVAL);
	върни 0;
}

цяло ul_strtos32(const char *str, int32_t *num, цяло base)
{
	int64_t tmp;
	цяло rc;

	rc = ul_strtos64(str, &tmp, base);
	ако (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	ако (rc == 0)
		*num = (int32_t) tmp;
	върни rc;
}

цяло ul_strtou32(const char *str, uint32_t *num, цяло base)
{
	uint64_t tmp;
	цяло rc;

	rc = ul_strtou64(str, &tmp, base);
	ако (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	ако (rc == 0)
		*num = (uint32_t) tmp;
	върни rc;
}

цяло ul_strtou16(const char *str, uint16_t *num, цяло base)
{
	uint64_t tmp;
	цяло rc;

	rc = ul_strtou64(str, &tmp, base);
	ако (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	ако (rc == 0)
		*num = (uint16_t) tmp;
	върни rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(const char *str, цяло base, const char *errmesg,
			     int64_t low, int64_t up)
{
	int64_t num = 0;
	цяло rc;

	rc = ul_strtos64(str, &num, base);
	ако (rc == 0 && ((low && num < low) || (up && num > up)))
		rc = -(errno = ERANGE);

	ако (rc) {
		ако (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	върни num;
}

uint64_t str2unum_or_err(const char *str, цяло base, const char *errmesg, uint64_t up)
{
	uint64_t num = 0;
	цяло rc;

	rc = ul_strtou64(str, &num, base);
	ако (rc == 0 && (up && num > up))
		rc = -(errno = ERANGE);

	ако (rc) {
		ако (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	върни num;
}

double strtod_or_err(const char *str, const char *errmesg)
{
	double num;
	char *end = NULL;

	errno = 0;
	ако (str == NULL || *str == '\0')
		goto err;
	num = strtod(str, &end);

	ако (errno || str == end || (end && *end))
		goto err;

	върни num;
err:
	ако (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

цяло ul_strtold(const char *str, long double *num)
{
	char *end = NULL;

	errno = 0;
	ако (str == NULL || *str == '\0')
		върни -(errno = EINVAL);
	*num = strtold(str, &end);

	ако (errno != 0)
		върни -errno;
	ако (str == end || (end && *end))
		върни -(errno = EINVAL);
	върни 0;
}

long double strtold_or_err(const char *str, const char *errmesg)
{
	long double num = 0;

	ако (ul_strtold(str, &num) == 0)
		върни num;
	ако (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(const char *str, const char *errmesg)
{
	uintmax_t num;

	ако (strtosize(str, &num) == 0)
		върни num;

	ако (errno)
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
	върни (time_t) user_input;
}

bool hyperlinkwanted(const char *mode)
{
	ако (mode && strcmp(mode, "never") == 0)
		върни false;

	ако (mode && strcmp(mode, "always") == 0)
		върни true;

	ако (!mode || strcmp(mode, "auto") == 0)
		върни isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), mode);
}

bool annotationwanted(const char *mode)
{
	ако (mode && strcmp(mode, "never") == 0)
		върни false;

	ако (mode && strcmp(mode, "always") == 0)
		върни true;

	ако (!mode || strcmp(mode, "auto") == 0)
		върни isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), mode);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
char *xstrmode(mode_t mode, char *str)
{
	unsigned short i = 0;

	ако (S_ISDIR(mode))
		str[i++] = 'd';
	иначе ако (S_ISLNK(mode))
		str[i++] = 'l';
	иначе ако (S_ISCHR(mode))
		str[i++] = 'c';
	иначе ако (S_ISBLK(mode))
		str[i++] = 'b';
	иначе ако (S_ISSOCK(mode))
		str[i++] = 's';
	иначе ако (S_ISFIFO(mode))
		str[i++] = 'p';
	иначе ако (S_ISREG(mode))
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

	върни str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
static цяло get_exp(uint64_t n)
{
	цяло shft;

	за (shft = 10; shft <= 60; shft += 10) {
		ако (n < (1ULL << shft))
			break;
	}
	върни shft - 10;
}

char *size_to_human_string(цяло options, uint64_t bytes)
{
	char buf[32];
	цяло dec, exp;
	uint64_t frac;
	const char *letters = "BKMGTPE";
	char suffix[sizeof(" KiB")], *psuf = suffix;
	char c;

	ако (options & SIZE_SUFFIX_SPACE)
		*psuf++ = ' ';


	exp  = get_exp(bytes);
	c    = *(letters + (exp ? exp / 10 : 0));
	dec  = exp ? bytes / (1ULL << exp) : bytes;
	frac = exp ? bytes % (1ULL << exp) : 0;

	*psuf++ = c;

	ако ((options & SIZE_SUFFIX_3LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/* round */
	ако (frac) {
		/* get 3 digits after decimal point */
		ако (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (exp - 10)) ;
		иначе
			frac = (frac * 1000) / (1ULL << (exp)) ;

		ако (options & SIZE_DECIMAL_2DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} иначе {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		ако (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	ако (frac) {
		struct lconv const *l = localeconv();
		char *dp = l ? l->decimal_point : NULL;
		цяло len;

		ако (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, sizeof(buf), "%d%s%02" PRIu64, dec, dp, frac);
		ако (len > 0 && (size_t) len < sizeof(buf)) {
			/* remove potential extraneous zero */
			ако (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, sizeof(buf) - len);
		} иначе
			*buf = '\0';	/* snprintf error */
	} иначе
		snprintf(buf, sizeof(buf), "%d%s", dec, suffix);

	върни strdup(buf);
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
цяло string_to_idarray(const char *list, цяло ary[], size_t arysz,
			цяло (name2id)(const char *, size_t))
{
	const char *begin = NULL, *p;
	size_t n = 0;

	ако (!list || !*list || !ary || !arysz || !name2id)
		върни -1;

	за (p = list; p && *p; p++) {
		const char *end = NULL;
		цяло id;

		ако (n >= arysz)
			върни -2;
		ако (!begin)
			begin = p;		/* begin of the column name */
		ако (*p == ',')
			end = p;		/* terminate the name */
		ако (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		ако (!begin || !end)
			continue;
		ако (end <= begin)
			върни -1;

		id = name2id(begin, end - begin);
		ако (id == -1)
			върни -1;
		ary[ n++ ] = id;
		begin = NULL;
		ако (end && !*end)
			break;
	}
	върни n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
цяло string_add_to_idarray(const char *list, цяло ary[], size_t arysz,
			size_t *ary_pos, цяло (name2id)(const char *, size_t))
{
	const char *list_add;
	цяло r;

	ако (!list || !*list || !ary_pos || *ary_pos > arysz)
		върни -1;

	ако (list[0] == '+')
		list_add = &list[1];
	иначе {
		list_add = list;
		*ary_pos = 0;
	}

	r = string_to_idarray(list_add, &ary[*ary_pos], arysz - *ary_pos, name2id);
	ако (r > 0)
		*ary_pos += r;
	върни r;
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
цяло string_to_bitarray(const char *list,
		     char *ary,
		     цяло (*name2bit)(const char *, size_t),
		     size_t allow_range)
{
	const char *begin = NULL, *p;

	ако (!list || !name2bit || !ary)
		върни -EINVAL;

	за (p = list; p && *p; p++) {
		const char *end = NULL;
		цяло bit, set_lower = 0, set_higher = 0;

		ако (!begin)
			begin = p;		/* begin of the level name */
		ако (*p == ',')
			end = p;		/* terminate the name */
		ако (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		ако (!begin || !end)
			continue;
		ако (end <= begin)
			върни -1;
		ако (allow_range) {
			ако (*(end - 1) == '+') {
				end--;
				set_lower = 1;
			} иначе ако (*begin == '+') {
				begin++;
				set_higher = 1;
			}
		}

		bit = name2bit(begin, end - begin);
		ако (bit < 0)
			върни bit;
		setbit(ary, bit);
		ако (set_lower)
			докато (--bit >= 0)
				setbit(ary, bit);
		иначе ако (set_higher)
			докато (++bit < (цяло) allow_range)
				setbit(ary, bit);
		begin = NULL;
		ако (end && !*end)
			break;
	}
	върни 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
цяло string_to_bitmask(const char *list,
		     unsigned long *mask,
		     long (*name2flag)(const char *, size_t))
{
	const char *begin = NULL, *p;

	ако (!list || !name2flag || !mask)
		върни -EINVAL;

	за (p = list; p && *p; p++) {
		const char *end = NULL;
		long flag;

		ако (!begin)
			begin = p;		/* begin of the level name */
		ако (*p == ',')
			end = p;		/* terminate the name */
		ако (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		ако (!begin || !end)
			continue;
		ако (end <= begin)
			върни -1;

		flag = name2flag(begin, end - begin);
		ако (flag < 0)
			върни flag;	/* error */
		*mask |= flag;
		begin = NULL;
		ако (end && !*end)
			break;
	}
	върни 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
цяло ul_parse_range(const char *str, цяло *lower, цяло *upper, цяло def)
{
	char *end = NULL;

	ако (!str)
		върни 0;

	*upper = *lower = def;
	errno = 0;

	ако (*str == ':') {				/* <:N> */
		str++;
		*upper = strtol(str, &end, 10);
		ако (errno || !end || *end || end == str)
			върни -1;
	} иначе {
		*upper = *lower = strtol(str, &end, 10);
		ако (errno || !end || end == str)
			върни -1;

		ако (*end == ':' && !*(end + 1))		/* <M:> */
			*upper = def;
		иначе ако (*end == '-' || *end == ':') {	/* <M:N> <M-N> */
			str = end + 1;
			end = NULL;
			errno = 0;
			*upper = strtol(str, &end, 10);

			ако (errno || !end || *end || end == str)
				върни -1;
		}
	}
	върни 0;
}

const char *ul_path_next_segment(const char *str, size_t *sz)
{
	const char *start, *p;

	start = str;
	*sz = 0;
	докато (start && *start == '/' && *(start + 1) == '/')
		start++;

	ако (!start || !*start)
		върни NULL;

	за (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	върни start;
}

цяло streq_paths(const char *a, const char *b)
{
	докато (a && b) {
		size_t a_sz, b_sz;
		const char *a_seg = ul_path_next_segment(a, &a_sz);
		const char *b_seg = ul_path_next_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		ако (a_sz + b_sz == 0)
			върни 1;

		/* ignore trailing slash */
		ако (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			върни 1;

		ако (!a_seg || !b_seg)
			break;
		ако (a_sz != b_sz || strncmp(a_seg, b_seg, a_sz) != 0)
			break;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	върни 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
цяло ul_path_is_within(const char *path, const char *dir)
{
	ако (!path || !dir || !*path || !*dir)
		върни 0;

	докато (path && dir) {
		size_t p_sz, d_sz;
		const char *p_seg = ul_path_next_segment(path, &p_sz);
		const char *d_seg = ul_path_next_segment(dir, &d_sz);

		/* all @dir segments matched */
		ако (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			върни 1;

		ако (!p_seg || !d_seg)
			break;
		ако (p_sz != d_sz || strncmp(p_seg, d_seg, p_sz) != 0)
			break;

		path = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	върни 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
char *ul_strnconcat(const char *s, const char *suffix, size_t b)
{
	size_t a;
	char *r;

	ако (!s && !suffix)
		върни strdup("");
	ако (!s)
		върни strndup(suffix, b);
	ако (!suffix)
		върни strdup(s);

	assert(s);
	assert(suffix);

	a = strlen(s);
	ако (b > ((size_t) -1) - a)
		върни NULL;

	r = malloc(a + b + 1);
	ако (!r)
		върни NULL;

	memcpy(r, s, a);
	memcpy(r + a, suffix, b);
	r[a+b] = 0;

	върни r;
}

/* concatenate two strings to a new string */
char *ul_strconcat(const char *s, const char *suffix)
{
	върни ul_strnconcat(s, suffix, suffix ? strlen(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
char *ul_strfconcat(const char *s, const char *format, ...)
{
	va_list ap;
	char *val, *res;
	цяло sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	ако (sz < 0)
		върни NULL;

	res = ul_strnconcat(s, val, sz);
	free(val);
	върни res;
}

цяло ul_strappend(char **a, const char *b)
{
	size_t al, bl;
	char *tmp;

	ако (!a)
		върни -EINVAL;
	ако (!b || !*b)
		върни 0;
	ако (!*a) {
		*a = strdup(b);
		върни !*a ? -ENOMEM : 0;
	}

	al = strlen(*a);
	bl = strlen(b);

	tmp = realloc(*a, al + bl + 1);
	ако (!tmp)
		върни -ENOMEM;
	*a = tmp;
	memcpy((*a) + al, b, bl + 1);
	върни 0;
}

/* the hybrid version of strfconcat and strappend. */
цяло strfappend(char **a, const char *format, ...)
{
	va_list ap;
	цяло res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	върни res;
}

extern цяло ul_strvfappend(char **a, const char *format, va_list ap)
{
	char *val;
	цяло sz;
	цяло res;

	sz = vasprintf(&val, format, ap);
	ако (sz < 0)
		върни -errno;

	res = ul_strappend(a, val);
	free(val);
	върни res;
}

static size_t strcspn_escaped(const char *s, const char *reject)
{
	цяло escaped = 0;
	цяло n;

	за (n=0; s[n]; n++) {
		ако (escaped)
			escaped = 0;
		иначе ако (s[n] == '\\')
			escaped = 1;
		иначе ако (strchr(reject, s[n]))
			break;
	}

	/* if s ends in \, return index of previous char */
	върни n - escaped;
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
char *ul_strchr_escaped(const char *s, цяло c)
{
	char *p;
	цяло esc = 0;

	за (p = (char *) s; p && *p; p++) {
		ако (!esc && *p == '\\') {
			esc = 1;
			continue;
		}
		ако (*p == c && (!esc || c == '\\'))
			върни p;
		esc = 0;
	}

	върни NULL;
}

/* Split a string into words. */
const char *ul_split(const char **state, size_t *l, const char *separator, цяло quoted)
{
	const char *current;

	current = *state;

	ако (!*current) {
		assert(**state == '\0');
		върни NULL;
	}

	current += strspn(current, separator);
	ако (!*current) {
		*state = current;
		върни NULL;
	}

	ако (quoted && strchr("\'\"", *current)) {
		char quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		ако (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			върни NULL;
		}
		*state = current++ + *l + 2;
	} иначе ако (quoted) {
		*l = strcspn_escaped(current, separator);
		ако (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			върни NULL;
		}
		*state = current + *l;
	} иначе {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	върни current;
}

/* Rewind file pointer forward to new line.  */
цяло skip_fline(FILE *fp)
{
	цяло ch;

	do {
		ако ((ch = fgetc(fp)) == EOF)
			върни 1;
		ако (ch == '\n')
			върни 0;
	} докато (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
цяло ul_stralnumcmp(const char *p1, const char *p2)
{
	const unsigned char *s1 = (const unsigned char *) p1;
	const unsigned char *s2 = (const unsigned char *) p2;
	unsigned char c1, c2;

	do {
		do {
			c1 = (unsigned char) *s1++;
		} докато (c1 != '\0' && !isalnum((unsigned цяло) c1));

		do {
			c2 = (unsigned char) *s2++;
		} докато (c2 != '\0' && !isalnum((unsigned цяло) c2));

		ако (c1 != '\0')
			c1 = tolower(c1);
		ако (c2 != '\0')
			c2 = tolower(c2);
		ако (c1 == '\0')
			върни c1 - c2;
	} докато (c1 == c2);

	върни c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
цяло ul_optstr_next(char **optstr, char **name, size_t *namesz,
		   char **value, size_t *valsz)
{
	цяло open_quote = 0;
	char *start = NULL, *stop = NULL, *p, *sep = NULL;
	char *optstr0;

	assert(optstr);
	assert(*optstr);

	optstr0 = *optstr;

	ако (name)
		*name = NULL;
	ако (namesz)
		*namesz = 0;
	ако (value)
		*value = NULL;
	ако (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	докато (optstr0 && *optstr0 == ',')
		optstr0++;

	за (p = optstr0; p && *p; p++) {
		ако (!start && *p == '=')
			върни -EINVAL;
		ако (!start)
			start = p;		/* beginning of the option item */
		ако (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		ако (open_quote)
			continue;		/* still in quoted block */
		ако (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		ако (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		иначе ако (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		ако (!start || !stop)
			continue;
		ако (stop <= start)
			върни -EINVAL;

		ако (name)
			*name = start;
		ако (namesz)
			*namesz = sep ? sep - start : stop - start;
		*optstr = *stop ? stop + 1 : stop;

		ако (sep) {
			ако (value)
				*value = sep + 1;
			ако (valsz)
				*valsz = stop - sep - 1;
		}
		върни 0;
	}

	върни 1;				/* end of optstr */
}

цяло ul_optstr_is_valid(const char *optstr)
{
	цяло rc;
	char *p = (char *) optstr;

	докато ((rc = ul_optstr_next(&p, NULL, NULL, NULL, NULL)) == 0);
	върни rc < 0 ? 0 : 1;
}

char *ul_optstr_get_value(const char *optstr, const char *key)
{
	size_t sz, namesz = 0, valsz = 0;
	char *name = NULL, *value = NULL;
	char *p = (char *) optstr;

	ако (!optstr || !key || !*key)
		върни NULL;

	sz = strlen(key);
	докато (ul_optstr_next(&p, &name, &namesz, &value, &valsz) == 0) {
		ако (namesz != sz || !valsz)
			continue;
		ако (strncmp(name, key, namesz) == 0)
			върни strndup(value, valsz);
	}
	върни NULL;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(char *str)
{
	size_t len;
	char q;

	ако (!str || !*str)
		върни 0;

	len = strlen(str);
	q = *str;

	ако ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		върни len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	върни len - 2;
}

#ifdef TEST_PROGRAM_STRUTILS

struct testS {
	char *name;
	char *value;
};

static цяло test_strdup_to_member(цяло argc, char *argv[])
{
	struct testS *xx;

	ако (argc < 3)
		върни EXIT_FAILURE;

	xx = calloc(1, sizeof(*xx));
	ако (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_to_struct_member(xx, name, argv[1]);
	strdup_to_struct_member(xx, value, argv[2]);

	ако (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->value, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	printf("1: '%s', 2: '%s'\n", xx->name, xx->value);

	free(xx->name);
	free(xx->value);
	free(xx);
	върни EXIT_SUCCESS;
}

static цяло test_strutils_sizes(цяло argc, char *argv[])
{
	uintmax_t size = 0;
	char *hum1, *hum2, *hum3;

	ако (argc < 2)
		върни EXIT_FAILURE;

	ако (strtosize(argv[1], &size))
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

	върни EXIT_SUCCESS;
}

static цяло test_strutils_cmp_paths(цяло argc, char *argv[])
{
	цяло rc = streq_paths(argv[1], argv[2]);

	ако (argc < 3)
		върни EXIT_FAILURE;

	printf("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	върни EXIT_SUCCESS;
}

static цяло test_strutils_path_is_within(цяло argc, char *argv[])
{
	цяло rc;

	ако (argc < 3)
		върни EXIT_FAILURE;

	rc = ul_path_is_within(argv[1], argv[2]);

	printf("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	върни EXIT_SUCCESS;
}

static цяло test_strutils_normalize(цяло argc, char *argv[])
{
	unsigned char *src, *dst, *org;
	size_t sz, len;

	ако (argc < 2)
		върни EXIT_FAILURE;

	org = (unsigned char *) strdup(argv[1]);
	src = (unsigned char *) strdup((char *) org);
	len = strlen((char *) src);
	dst = malloc(len + 1);

	ако (!org || !src || !dst)
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

	върни EXIT_SUCCESS;
}

static цяло test_strutils_cstrcasecmp(цяло argc, char *argv[])
{
	char *a, *b;

	ако (argc < 3)
		върни EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	ако (!a || !b)
		върни EXIT_FAILURE;

	printf("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	printf("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	printf("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, strlen(a)));

	върни EXIT_SUCCESS;
}

цяло main(цяло argc, char *argv[])
{
	ако (argc == 3 && strcmp(argv[1], "--size") == 0) {
		върни test_strutils_sizes(argc - 1, argv + 1);

	} иначе ако (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		printf("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						NULL));
		върни EXIT_SUCCESS;
	} иначе ако (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		ако (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		printf("'%s'-->%s\n", argv[2], val ? "true" : "false");
		върни EXIT_SUCCESS;

	} иначе ако (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		върни test_strutils_cmp_paths(argc - 1, argv + 1);

	} иначе ако (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		върни test_strutils_path_is_within(argc - 1, argv + 1);

	} иначе ако (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		върни test_strdup_to_member(argc - 1, argv + 1);

	} иначе ако  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		printf("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		върни EXIT_SUCCESS;

	} иначе ако (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		върни test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} иначе ако (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		върни test_strutils_normalize(argc - 1, argv + 1);

	} иначе ако (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		printf("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		върни EXIT_SUCCESS;
	} иначе ако (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		printf("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		върни EXIT_SUCCESS;
	} иначе ако (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		printf("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		върни EXIT_SUCCESS;
	} иначе ако (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		printf("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		върни EXIT_SUCCESS;
	} иначе ако (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		printf("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		върни EXIT_SUCCESS;
	} иначе ако (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		printf("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		върни EXIT_SUCCESS;

	} иначе ако (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		printf("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		върни EXIT_SUCCESS;

	} иначе ако (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		char *buf = "abc\0Y\0\0xyz\0X";
		char *end = buf + 12;
		char *p = buf;

		do {
			printf("str: '%s'\n", p);
		} докато ((p = ul_next_string(p, end)));

		върни EXIT_SUCCESS;

	} иначе ако (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		char *name = NULL, *val = NULL;
		char *p = argv[2];
		цяло rc;

		ако (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		докато ((rc = ul_optstr_next(&p, &name, &namesz, &val, &valsz)) == 0) {
			printf("'%.*s' : '%.*s'\n", (цяло) namesz, name,
						   (цяло) valsz, val);
		}
		ако (rc == 1)
			върни EXIT_SUCCESS;
	} иначе ако (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		char *unquoted = strdup(argv[2]);

		ако (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		printf("%s-->%s\n", argv[2], unquoted);
		free(unquoted);
		върни EXIT_SUCCESS;

	} иначе {
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

	върни EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_STRUTILS */
