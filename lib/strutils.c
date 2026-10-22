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

static цео STRTOXX_EXIT_CODE = EXIT_FAILURE;

void strutils_set_exitcode(цео ex)
{
	STRTOXX_EXIT_CODE = ex;
}

static цео do_scale_by_power (uintmax_t *x, цео base, цео power)
{
	док (power--) {
		ако (UINTMAX_MAX / base < *x)
			врати -ERANGE;
		*x *= base;
	}
	врати 0;
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
цео ul_parse_size(const char *str, uintmax_t *res, цео *power)
{
	const char *p;
	char *end;
	uintmax_t x, frac = 0;
	цео base = 1024, rc = 0, pwr = 0, frac_zeros = 0;

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
	док (isspace((unsigned char) *p))
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
	инако ако ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		base = 1000;			/* XB, 10^N */
	инако ако (*(p + 1)) {
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
			} инако
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
	инако {
		sp = strchr(suf2, *p);
		ако (sp)
			pwr = (sp - suf2) + 1;
		инако {
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
		цео i;
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
		} док (frac);

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
	врати rc;
}

цео strtosize(const char *str, uintmax_t *res)
{
	врати ul_parse_size(str, res, NULL);
}

цео isdigit_strend(const char *str, const char **end)
{
	const char *p;

	за (p = str; p && *p && isdigit((unsigned char) *p); p++);

	ако (end)
		*end = p;
	врати p && p > str && !*p;
}

цео isxdigit_strend(const char *str, const char **end)
{
	const char *p;

	за (p = str; p && *p && isxdigit((unsigned char) *p); p++);

	ако (end)
		*end = p;

	врати p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
цео ul_strtobool(const char *str, bool *result)
{
	static const char *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	static const char *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	ако (!str || !result)
		врати -(errno = EINVAL);

	errno = 0;

	за (i = 0; i < ARRAY_SIZE(bool_true); i++) {
		ако (c_strcasecmp(str, bool_true[i]) == 0) {
			*result = true;
			врати 0;
		}
	}

	за (i = 0; i < ARRAY_SIZE(bool_false); i++) {
		ако (c_strcasecmp(str, bool_false[i]) == 0) {
			*result = false;
			врати 0;
		}
	}

	врати -(errno = EINVAL);
}

цео ul_parse_switch(const char *arg, ...)
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
			врати 1;
		}

		ако (strcmp(arg, b) == 0) {
			va_end(ap);
			врати 0;
		}
	} док (1);
	va_end(ap);

	errx(STRTOXX_EXIT_CODE, _("unsupported argument: %s"), arg);
}

#ifndef HAVE_MEMPCPY
void *mempcpy(void *restrict dest, const void *restrict src, size_t n)
{
	врати ((char *)memcpy(dest, src, n)) + n;
}
#endif

#ifndef HAVE_STRNLEN
size_t strnlen(const char *s, size_t maxlen)
{
	size_t i;

	за (i = 0; i < maxlen; i++) {
		ако (s[i] == '\0')
			врати i;
	}
	врати maxlen;
}
#endif

#ifndef HAVE_STRNCHR
char *strnchr(const char *s, size_t maxlen, цео c)
{
	за (; maxlen-- && *s != '\0'; ++s)
		ако (*s == (char)c)
			врати (char *)s;
	врати NULL;
}
#endif

#ifndef HAVE_STRNDUP
char *strndup(const char *s, size_t n)
{
	size_t len = strnlen(s, n);
	char *new = malloc((len + 1) * sizeof(char));
	ако (!new)
		врати NULL;
	new[len] = '\0';
	врати (char *) memcpy(new, s, len);
}
#endif

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
цео ul_strtos64(const char *str, int64_t *num, цео base)
{
	char *end = NULL;

	ако (str == NULL || *str == '\0')
		врати -(errno = EINVAL);

	errno = 0;
	*num = (int64_t) strtoimax(str, &end, base);

	ако (errno != 0)
		врати -errno;
	ако (str == end || (end && *end))
		врати -(errno = EINVAL);
	врати 0;
}

цео ul_strtou64(const char *str, uint64_t *num, цео base)
{
	char *end = NULL;
	int64_t tmp;

	ако (str == NULL || *str == '\0')
		врати -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &end, base);
	ако (tmp < 0)
		errno = ERANGE;
	инако {
		errno = 0;
		*num = strtoumax(str, &end, base);
	}

	ако (errno != 0)
		врати -errno;
	ако (str == end || (end && *end))
		врати -(errno = EINVAL);
	врати 0;
}

цео ul_strtos32(const char *str, int32_t *num, цео base)
{
	int64_t tmp;
	цео rc;

	rc = ul_strtos64(str, &tmp, base);
	ако (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	ако (rc == 0)
		*num = (int32_t) tmp;
	врати rc;
}

цео ul_strtou32(const char *str, uint32_t *num, цео base)
{
	uint64_t tmp;
	цео rc;

	rc = ul_strtou64(str, &tmp, base);
	ако (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	ако (rc == 0)
		*num = (uint32_t) tmp;
	врати rc;
}

цео ul_strtou16(const char *str, uint16_t *num, цео base)
{
	uint64_t tmp;
	цео rc;

	rc = ul_strtou64(str, &tmp, base);
	ако (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	ако (rc == 0)
		*num = (uint16_t) tmp;
	врати rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(const char *str, цео base, const char *errmesg,
			     int64_t low, int64_t up)
{
	int64_t num = 0;
	цео rc;

	rc = ul_strtos64(str, &num, base);
	ако (rc == 0 && ((low && num < low) || (up && num > up)))
		rc = -(errno = ERANGE);

	ако (rc) {
		ако (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	врати num;
}

uint64_t str2unum_or_err(const char *str, цео base, const char *errmesg, uint64_t up)
{
	uint64_t num = 0;
	цео rc;

	rc = ul_strtou64(str, &num, base);
	ако (rc == 0 && (up && num > up))
		rc = -(errno = ERANGE);

	ако (rc) {
		ако (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	врати num;
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

	врати num;
err:
	ако (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

цео ul_strtold(const char *str, long double *num)
{
	char *end = NULL;

	errno = 0;
	ако (str == NULL || *str == '\0')
		врати -(errno = EINVAL);
	*num = strtold(str, &end);

	ако (errno != 0)
		врати -errno;
	ако (str == end || (end && *end))
		врати -(errno = EINVAL);
	врати 0;
}

long double strtold_or_err(const char *str, const char *errmesg)
{
	long double num = 0;

	ако (ul_strtold(str, &num) == 0)
		врати num;
	ако (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(const char *str, const char *errmesg)
{
	uintmax_t num;

	ако (strtosize(str, &num) == 0)
		врати num;

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
	врати (time_t) user_input;
}

bool hyperlinkwanted(const char *mode)
{
	ако (mode && strcmp(mode, "never") == 0)
		врати false;

	ако (mode && strcmp(mode, "always") == 0)
		врати true;

	ако (!mode || strcmp(mode, "auto") == 0)
		врати isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), mode);
}

bool annotationwanted(const char *mode)
{
	ако (mode && strcmp(mode, "never") == 0)
		врати false;

	ако (mode && strcmp(mode, "always") == 0)
		врати true;

	ако (!mode || strcmp(mode, "auto") == 0)
		врати isatty(STDOUT_FILENO) ? true : false;

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
	инако ако (S_ISLNK(mode))
		str[i++] = 'l';
	инако ако (S_ISCHR(mode))
		str[i++] = 'c';
	инако ако (S_ISBLK(mode))
		str[i++] = 'b';
	инако ако (S_ISSOCK(mode))
		str[i++] = 's';
	инако ако (S_ISFIFO(mode))
		str[i++] = 'p';
	инако ако (S_ISREG(mode))
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

	врати str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
static цео get_exp(uint64_t n)
{
	цео shft;

	за (shft = 10; shft <= 60; shft += 10) {
		ако (n < (1ULL << shft))
			break;
	}
	врати shft - 10;
}

char *size_to_human_string(цео options, uint64_t bytes)
{
	char buf[32];
	цео dec, exp;
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
		инако
			frac = (frac * 1000) / (1ULL << (exp)) ;

		ако (options & SIZE_DECIMAL_2DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} инако {
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
		цео len;

		ако (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, sizeof(buf), "%d%s%02" PRIu64, dec, dp, frac);
		ако (len > 0 && (size_t) len < sizeof(buf)) {
			/* remove potential extraneous zero */
			ако (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, sizeof(buf) - len);
		} инако
			*buf = '\0';	/* snprintf error */
	} инако
		snprintf(buf, sizeof(buf), "%d%s", dec, suffix);

	врати strdup(buf);
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
цео string_to_idarray(const char *list, цео ary[], size_t arysz,
			цео (name2id)(const char *, size_t))
{
	const char *begin = NULL, *p;
	size_t n = 0;

	ако (!list || !*list || !ary || !arysz || !name2id)
		врати -1;

	за (p = list; p && *p; p++) {
		const char *end = NULL;
		цео id;

		ако (n >= arysz)
			врати -2;
		ако (!begin)
			begin = p;		/* begin of the column name */
		ако (*p == ',')
			end = p;		/* terminate the name */
		ако (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		ако (!begin || !end)
			continue;
		ако (end <= begin)
			врати -1;

		id = name2id(begin, end - begin);
		ако (id == -1)
			врати -1;
		ary[ n++ ] = id;
		begin = NULL;
		ако (end && !*end)
			break;
	}
	врати n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
цео string_add_to_idarray(const char *list, цео ary[], size_t arysz,
			size_t *ary_pos, цео (name2id)(const char *, size_t))
{
	const char *list_add;
	цео r;

	ако (!list || !*list || !ary_pos || *ary_pos > arysz)
		врати -1;

	ако (list[0] == '+')
		list_add = &list[1];
	инако {
		list_add = list;
		*ary_pos = 0;
	}

	r = string_to_idarray(list_add, &ary[*ary_pos], arysz - *ary_pos, name2id);
	ако (r > 0)
		*ary_pos += r;
	врати r;
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
цео string_to_bitarray(const char *list,
		     char *ary,
		     цео (*name2bit)(const char *, size_t),
		     size_t allow_range)
{
	const char *begin = NULL, *p;

	ако (!list || !name2bit || !ary)
		врати -EINVAL;

	за (p = list; p && *p; p++) {
		const char *end = NULL;
		цео bit, set_lower = 0, set_higher = 0;

		ако (!begin)
			begin = p;		/* begin of the level name */
		ако (*p == ',')
			end = p;		/* terminate the name */
		ако (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		ако (!begin || !end)
			continue;
		ако (end <= begin)
			врати -1;
		ако (allow_range) {
			ако (*(end - 1) == '+') {
				end--;
				set_lower = 1;
			} инако ако (*begin == '+') {
				begin++;
				set_higher = 1;
			}
		}

		bit = name2bit(begin, end - begin);
		ако (bit < 0)
			врати bit;
		setbit(ary, bit);
		ако (set_lower)
			док (--bit >= 0)
				setbit(ary, bit);
		инако ако (set_higher)
			док (++bit < (цео) allow_range)
				setbit(ary, bit);
		begin = NULL;
		ако (end && !*end)
			break;
	}
	врати 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
цео string_to_bitmask(const char *list,
		     unsigned long *mask,
		     long (*name2flag)(const char *, size_t))
{
	const char *begin = NULL, *p;

	ако (!list || !name2flag || !mask)
		врати -EINVAL;

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
			врати -1;

		flag = name2flag(begin, end - begin);
		ако (flag < 0)
			врати flag;	/* error */
		*mask |= flag;
		begin = NULL;
		ако (end && !*end)
			break;
	}
	врати 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
цео ul_parse_range(const char *str, цео *lower, цео *upper, цео def)
{
	char *end = NULL;

	ако (!str)
		врати 0;

	*upper = *lower = def;
	errno = 0;

	ако (*str == ':') {				/* <:N> */
		str++;
		*upper = strtol(str, &end, 10);
		ако (errno || !end || *end || end == str)
			врати -1;
	} инако {
		*upper = *lower = strtol(str, &end, 10);
		ако (errno || !end || end == str)
			врати -1;

		ако (*end == ':' && !*(end + 1))		/* <M:> */
			*upper = def;
		инако ако (*end == '-' || *end == ':') {	/* <M:N> <M-N> */
			str = end + 1;
			end = NULL;
			errno = 0;
			*upper = strtol(str, &end, 10);

			ако (errno || !end || *end || end == str)
				врати -1;
		}
	}
	врати 0;
}

const char *ul_path_next_segment(const char *str, size_t *sz)
{
	const char *start, *p;

	start = str;
	*sz = 0;
	док (start && *start == '/' && *(start + 1) == '/')
		start++;

	ако (!start || !*start)
		врати NULL;

	за (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	врати start;
}

цео streq_paths(const char *a, const char *b)
{
	док (a && b) {
		size_t a_sz, b_sz;
		const char *a_seg = ul_path_next_segment(a, &a_sz);
		const char *b_seg = ul_path_next_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		ако (a_sz + b_sz == 0)
			врати 1;

		/* ignore trailing slash */
		ако (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			врати 1;

		ако (!a_seg || !b_seg)
			break;
		ако (a_sz != b_sz || strncmp(a_seg, b_seg, a_sz) != 0)
			break;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	врати 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
цео ul_path_is_within(const char *path, const char *dir)
{
	ако (!path || !dir || !*path || !*dir)
		врати 0;

	док (path && dir) {
		size_t p_sz, d_sz;
		const char *p_seg = ul_path_next_segment(path, &p_sz);
		const char *d_seg = ul_path_next_segment(dir, &d_sz);

		/* all @dir segments matched */
		ако (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			врати 1;

		ако (!p_seg || !d_seg)
			break;
		ако (p_sz != d_sz || strncmp(p_seg, d_seg, p_sz) != 0)
			break;

		path = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	врати 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
char *ul_strnconcat(const char *s, const char *suffix, size_t b)
{
	size_t a;
	char *r;

	ако (!s && !suffix)
		врати strdup("");
	ако (!s)
		врати strndup(suffix, b);
	ако (!suffix)
		врати strdup(s);

	assert(s);
	assert(suffix);

	a = strlen(s);
	ако (b > ((size_t) -1) - a)
		врати NULL;

	r = malloc(a + b + 1);
	ако (!r)
		врати NULL;

	memcpy(r, s, a);
	memcpy(r + a, suffix, b);
	r[a+b] = 0;

	врати r;
}

/* concatenate two strings to a new string */
char *ul_strconcat(const char *s, const char *suffix)
{
	врати ul_strnconcat(s, suffix, suffix ? strlen(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
char *ul_strfconcat(const char *s, const char *format, ...)
{
	va_list ap;
	char *val, *res;
	цео sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	ако (sz < 0)
		врати NULL;

	res = ul_strnconcat(s, val, sz);
	free(val);
	врати res;
}

цео ul_strappend(char **a, const char *b)
{
	size_t al, bl;
	char *tmp;

	ако (!a)
		врати -EINVAL;
	ако (!b || !*b)
		врати 0;
	ако (!*a) {
		*a = strdup(b);
		врати !*a ? -ENOMEM : 0;
	}

	al = strlen(*a);
	bl = strlen(b);

	tmp = realloc(*a, al + bl + 1);
	ако (!tmp)
		врати -ENOMEM;
	*a = tmp;
	memcpy((*a) + al, b, bl + 1);
	врати 0;
}

/* the hybrid version of strfconcat and strappend. */
цео strfappend(char **a, const char *format, ...)
{
	va_list ap;
	цео res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	врати res;
}

extern цео ul_strvfappend(char **a, const char *format, va_list ap)
{
	char *val;
	цео sz;
	цео res;

	sz = vasprintf(&val, format, ap);
	ако (sz < 0)
		врати -errno;

	res = ul_strappend(a, val);
	free(val);
	врати res;
}

static size_t strcspn_escaped(const char *s, const char *reject)
{
	цео escaped = 0;
	цео n;

	за (n=0; s[n]; n++) {
		ако (escaped)
			escaped = 0;
		инако ако (s[n] == '\\')
			escaped = 1;
		инако ако (strchr(reject, s[n]))
			break;
	}

	/* if s ends in \, return index of previous char */
	врати n - escaped;
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
char *ul_strchr_escaped(const char *s, цео c)
{
	char *p;
	цео esc = 0;

	за (p = (char *) s; p && *p; p++) {
		ако (!esc && *p == '\\') {
			esc = 1;
			continue;
		}
		ако (*p == c && (!esc || c == '\\'))
			врати p;
		esc = 0;
	}

	врати NULL;
}

/* Split a string into words. */
const char *ul_split(const char **state, size_t *l, const char *separator, цео quoted)
{
	const char *current;

	current = *state;

	ако (!*current) {
		assert(**state == '\0');
		врати NULL;
	}

	current += strspn(current, separator);
	ако (!*current) {
		*state = current;
		врати NULL;
	}

	ако (quoted && strchr("\'\"", *current)) {
		char quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		ако (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			врати NULL;
		}
		*state = current++ + *l + 2;
	} инако ако (quoted) {
		*l = strcspn_escaped(current, separator);
		ако (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			врати NULL;
		}
		*state = current + *l;
	} инако {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	врати current;
}

/* Rewind file pointer forward to new line.  */
цео skip_fline(FILE *fp)
{
	цео ch;

	do {
		ако ((ch = fgetc(fp)) == EOF)
			врати 1;
		ако (ch == '\n')
			врати 0;
	} док (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
цео ul_stralnumcmp(const char *p1, const char *p2)
{
	const unsigned char *s1 = (const unsigned char *) p1;
	const unsigned char *s2 = (const unsigned char *) p2;
	unsigned char c1, c2;

	do {
		do {
			c1 = (unsigned char) *s1++;
		} док (c1 != '\0' && !isalnum((unsigned цео) c1));

		do {
			c2 = (unsigned char) *s2++;
		} док (c2 != '\0' && !isalnum((unsigned цео) c2));

		ако (c1 != '\0')
			c1 = tolower(c1);
		ако (c2 != '\0')
			c2 = tolower(c2);
		ако (c1 == '\0')
			врати c1 - c2;
	} док (c1 == c2);

	врати c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
цео ul_optstr_next(char **optstr, char **name, size_t *namesz,
		   char **value, size_t *valsz)
{
	цео open_quote = 0;
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
	док (optstr0 && *optstr0 == ',')
		optstr0++;

	за (p = optstr0; p && *p; p++) {
		ако (!start && *p == '=')
			врати -EINVAL;
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
		инако ако (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		ако (!start || !stop)
			continue;
		ако (stop <= start)
			врати -EINVAL;

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
		врати 0;
	}

	врати 1;				/* end of optstr */
}

цео ul_optstr_is_valid(const char *optstr)
{
	цео rc;
	char *p = (char *) optstr;

	док ((rc = ul_optstr_next(&p, NULL, NULL, NULL, NULL)) == 0);
	врати rc < 0 ? 0 : 1;
}

char *ul_optstr_get_value(const char *optstr, const char *key)
{
	size_t sz, namesz = 0, valsz = 0;
	char *name = NULL, *value = NULL;
	char *p = (char *) optstr;

	ако (!optstr || !key || !*key)
		врати NULL;

	sz = strlen(key);
	док (ul_optstr_next(&p, &name, &namesz, &value, &valsz) == 0) {
		ако (namesz != sz || !valsz)
			continue;
		ако (strncmp(name, key, namesz) == 0)
			врати strndup(value, valsz);
	}
	врати NULL;
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
		врати 0;

	len = strlen(str);
	q = *str;

	ако ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		врати len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	врати len - 2;
}

#ifdef TEST_PROGRAM_STRUTILS

struct testS {
	char *name;
	char *value;
};

static цео test_strdup_to_member(цео argc, char *argv[])
{
	struct testS *xx;

	ако (argc < 3)
		врати EXIT_FAILURE;

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
	врати EXIT_SUCCESS;
}

static цео test_strutils_sizes(цео argc, char *argv[])
{
	uintmax_t size = 0;
	char *hum1, *hum2, *hum3;

	ако (argc < 2)
		врати EXIT_FAILURE;

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

	врати EXIT_SUCCESS;
}

static цео test_strutils_cmp_paths(цео argc, char *argv[])
{
	цео rc = streq_paths(argv[1], argv[2]);

	ако (argc < 3)
		врати EXIT_FAILURE;

	printf("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	врати EXIT_SUCCESS;
}

static цео test_strutils_path_is_within(цео argc, char *argv[])
{
	цео rc;

	ако (argc < 3)
		врати EXIT_FAILURE;

	rc = ul_path_is_within(argv[1], argv[2]);

	printf("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	врати EXIT_SUCCESS;
}

static цео test_strutils_normalize(цео argc, char *argv[])
{
	unsigned char *src, *dst, *org;
	size_t sz, len;

	ако (argc < 2)
		врати EXIT_FAILURE;

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

	врати EXIT_SUCCESS;
}

static цео test_strutils_cstrcasecmp(цео argc, char *argv[])
{
	char *a, *b;

	ако (argc < 3)
		врати EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	ако (!a || !b)
		врати EXIT_FAILURE;

	printf("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	printf("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	printf("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, strlen(a)));

	врати EXIT_SUCCESS;
}

цео main(цео argc, char *argv[])
{
	ако (argc == 3 && strcmp(argv[1], "--size") == 0) {
		врати test_strutils_sizes(argc - 1, argv + 1);

	} инако ако (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		printf("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						NULL));
		врати EXIT_SUCCESS;
	} инако ако (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		ако (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		printf("'%s'-->%s\n", argv[2], val ? "true" : "false");
		врати EXIT_SUCCESS;

	} инако ако (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		врати test_strutils_cmp_paths(argc - 1, argv + 1);

	} инако ако (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		врати test_strutils_path_is_within(argc - 1, argv + 1);

	} инако ако (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		врати test_strdup_to_member(argc - 1, argv + 1);

	} инако ако  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		printf("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		врати EXIT_SUCCESS;

	} инако ако (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		врати test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} инако ако (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		врати test_strutils_normalize(argc - 1, argv + 1);

	} инако ако (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		printf("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		врати EXIT_SUCCESS;
	} инако ако (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		printf("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		врати EXIT_SUCCESS;
	} инако ако (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		printf("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		врати EXIT_SUCCESS;
	} инако ако (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		printf("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		врати EXIT_SUCCESS;
	} инако ако (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		printf("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		врати EXIT_SUCCESS;
	} инако ако (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		printf("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		врати EXIT_SUCCESS;

	} инако ако (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		printf("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		врати EXIT_SUCCESS;

	} инако ако (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		char *buf = "abc\0Y\0\0xyz\0X";
		char *end = buf + 12;
		char *p = buf;

		do {
			printf("str: '%s'\n", p);
		} док ((p = ul_next_string(p, end)));

		врати EXIT_SUCCESS;

	} инако ако (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		char *name = NULL, *val = NULL;
		char *p = argv[2];
		цео rc;

		ако (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		док ((rc = ul_optstr_next(&p, &name, &namesz, &val, &valsz)) == 0) {
			printf("'%.*s' : '%.*s'\n", (цео) namesz, name,
						   (цео) valsz, val);
		}
		ако (rc == 1)
			врати EXIT_SUCCESS;
	} инако ако (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		char *unquoted = strdup(argv[2]);

		ако (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		printf("%s-->%s\n", argv[2], unquoted);
		free(unquoted);
		врати EXIT_SUCCESS;

	} инако {
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

	врати EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_STRUTILS */
