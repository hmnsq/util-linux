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

static số_nguyên STRTOXX_EXIT_CODE = EXIT_FAILURE;

void strutils_set_exitcode(số_nguyên ex)
{
	STRTOXX_EXIT_CODE = ex;
}

static số_nguyên do_scale_by_power (uintmax_t *x, số_nguyên base, số_nguyên power)
{
	trong_khi (power--) {
		nếu (UINTMAX_MAX / base < *x)
			trả_về -ERANGE;
		*x *= base;
	}
	trả_về 0;
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
số_nguyên ul_parse_size(const char *str, uintmax_t *res, số_nguyên *power)
{
	const char *p;
	char *end;
	uintmax_t x, frac = 0;
	số_nguyên base = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	static const char *suf  = "KMGTPEZY";
	static const char *suf2 = "kmgtpezy";
	const char *sp;

	*res = 0;

	nếu (!str || !*str) {
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
	trong_khi (isspace((unsigned char) *p))
		p++;
	nếu (*p == '-') {
		rc = -EINVAL;
		goto err;
	}

	errno = 0, end = NULL;
	x = strtoumax(str, &end, 0);

	nếu (end == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		goto err;
	}
	nếu (!end || !*end)
		goto done;			/* without suffix */
	p = end;

	/*
	 * Check size suffixes
	 */
check_suffix:
	nếu (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		base = 1024;			/* XiB, 2^N */
	không_thì nếu ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		base = 1000;			/* XB, 10^N */
	không_thì nếu (*(p + 1)) {
		struct lconv const *l = localeconv();
		const char *dp = l ? l->decimal_point : NULL;
		size_t dpsz = dp ? strlen(dp) : 0;

		nếu (frac == 0 && *p && dp && strncmp(dp, p, dpsz) == 0) {
			const char *fstr = p + dpsz;

			cho (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			nếu (isdigit(*fstr)) {
				errno = 0, end = NULL;
				frac = strtoumax(fstr, &end, 0);
				nếu (end == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					goto err;
				}
			} không_thì
				end = (char *) p;

			nếu (!end || !*end) {
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
	nếu (sp)
		pwr = (sp - suf) + 1;
	không_thì {
		sp = strchr(suf2, *p);
		nếu (sp)
			pwr = (sp - suf2) + 1;
		không_thì {
			rc = -EINVAL;
			goto err;
		}
	}

	nếu (power)
		*power = pwr;

	rc = do_scale_by_power(&x, base, pwr);
	nếu (rc)
		goto err;

	nếu (frac && pwr) {
		số_nguyên i;
		uintmax_t frac_base = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_base, base, pwr);
		nếu (rc)
			goto err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		do {
			frac_val = ((frac % 10) * frac_base + frac_val) / 10;
			frac /= 10;
		} trong_khi (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		cho (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		nếu (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			goto err;
		}
		x += frac_val;
	}
done:
	*res = x;
err:
	nếu (rc < 0)
		errno = -rc;
	trả_về rc;
}

số_nguyên strtosize(const char *str, uintmax_t *res)
{
	trả_về ul_parse_size(str, res, NULL);
}

số_nguyên isdigit_strend(const char *str, const char **end)
{
	const char *p;

	cho (p = str; p && *p && isdigit((unsigned char) *p); p++);

	nếu (end)
		*end = p;
	trả_về p && p > str && !*p;
}

số_nguyên isxdigit_strend(const char *str, const char **end)
{
	const char *p;

	cho (p = str; p && *p && isxdigit((unsigned char) *p); p++);

	nếu (end)
		*end = p;

	trả_về p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
số_nguyên ul_strtobool(const char *str, bool *result)
{
	static const char *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	static const char *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	nếu (!str || !result)
		trả_về -(errno = EINVAL);

	errno = 0;

	cho (i = 0; i < ARRAY_SIZE(bool_true); i++) {
		nếu (c_strcasecmp(str, bool_true[i]) == 0) {
			*result = true;
			trả_về 0;
		}
	}

	cho (i = 0; i < ARRAY_SIZE(bool_false); i++) {
		nếu (c_strcasecmp(str, bool_false[i]) == 0) {
			*result = false;
			trả_về 0;
		}
	}

	trả_về -(errno = EINVAL);
}

số_nguyên ul_parse_switch(const char *arg, ...)
{
	const char *a, *b;
	va_list ap;

	va_start(ap, arg);
	do {
		a = va_arg(ap, char *);
		nếu (!a)
			break;
		b = va_arg(ap, char *);
		nếu (!b)
			break;

		nếu (strcmp(arg, a) == 0) {
			va_end(ap);
			trả_về 1;
		}

		nếu (strcmp(arg, b) == 0) {
			va_end(ap);
			trả_về 0;
		}
	} trong_khi (1);
	va_end(ap);

	errx(STRTOXX_EXIT_CODE, _("unsupported argument: %s"), arg);
}

#ifndef HAVE_MEMPCPY
void *mempcpy(void *restrict dest, const void *restrict src, size_t n)
{
	trả_về ((char *)memcpy(dest, src, n)) + n;
}
#endif

#ifndef HAVE_STRNLEN
size_t strnlen(const char *s, size_t maxlen)
{
	size_t i;

	cho (i = 0; i < maxlen; i++) {
		nếu (s[i] == '\0')
			trả_về i;
	}
	trả_về maxlen;
}
#endif

#ifndef HAVE_STRNCHR
char *strnchr(const char *s, size_t maxlen, số_nguyên c)
{
	cho (; maxlen-- && *s != '\0'; ++s)
		nếu (*s == (char)c)
			trả_về (char *)s;
	trả_về NULL;
}
#endif

#ifndef HAVE_STRNDUP
char *strndup(const char *s, size_t n)
{
	size_t len = strnlen(s, n);
	char *new = malloc((len + 1) * sizeof(char));
	nếu (!new)
		trả_về NULL;
	new[len] = '\0';
	trả_về (char *) memcpy(new, s, len);
}
#endif

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
số_nguyên ul_strtos64(const char *str, int64_t *num, số_nguyên base)
{
	char *end = NULL;

	nếu (str == NULL || *str == '\0')
		trả_về -(errno = EINVAL);

	errno = 0;
	*num = (int64_t) strtoimax(str, &end, base);

	nếu (errno != 0)
		trả_về -errno;
	nếu (str == end || (end && *end))
		trả_về -(errno = EINVAL);
	trả_về 0;
}

số_nguyên ul_strtou64(const char *str, uint64_t *num, số_nguyên base)
{
	char *end = NULL;
	int64_t tmp;

	nếu (str == NULL || *str == '\0')
		trả_về -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &end, base);
	nếu (tmp < 0)
		errno = ERANGE;
	không_thì {
		errno = 0;
		*num = strtoumax(str, &end, base);
	}

	nếu (errno != 0)
		trả_về -errno;
	nếu (str == end || (end && *end))
		trả_về -(errno = EINVAL);
	trả_về 0;
}

số_nguyên ul_strtos32(const char *str, int32_t *num, số_nguyên base)
{
	int64_t tmp;
	số_nguyên rc;

	rc = ul_strtos64(str, &tmp, base);
	nếu (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	nếu (rc == 0)
		*num = (int32_t) tmp;
	trả_về rc;
}

số_nguyên ul_strtou32(const char *str, uint32_t *num, số_nguyên base)
{
	uint64_t tmp;
	số_nguyên rc;

	rc = ul_strtou64(str, &tmp, base);
	nếu (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	nếu (rc == 0)
		*num = (uint32_t) tmp;
	trả_về rc;
}

số_nguyên ul_strtou16(const char *str, uint16_t *num, số_nguyên base)
{
	uint64_t tmp;
	số_nguyên rc;

	rc = ul_strtou64(str, &tmp, base);
	nếu (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	nếu (rc == 0)
		*num = (uint16_t) tmp;
	trả_về rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(const char *str, số_nguyên base, const char *errmesg,
			     int64_t low, int64_t up)
{
	int64_t num = 0;
	số_nguyên rc;

	rc = ul_strtos64(str, &num, base);
	nếu (rc == 0 && ((low && num < low) || (up && num > up)))
		rc = -(errno = ERANGE);

	nếu (rc) {
		nếu (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	trả_về num;
}

uint64_t str2unum_or_err(const char *str, số_nguyên base, const char *errmesg, uint64_t up)
{
	uint64_t num = 0;
	số_nguyên rc;

	rc = ul_strtou64(str, &num, base);
	nếu (rc == 0 && (up && num > up))
		rc = -(errno = ERANGE);

	nếu (rc) {
		nếu (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	trả_về num;
}

double strtod_or_err(const char *str, const char *errmesg)
{
	double num;
	char *end = NULL;

	errno = 0;
	nếu (str == NULL || *str == '\0')
		goto err;
	num = strtod(str, &end);

	nếu (errno || str == end || (end && *end))
		goto err;

	trả_về num;
err:
	nếu (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

số_nguyên ul_strtold(const char *str, long double *num)
{
	char *end = NULL;

	errno = 0;
	nếu (str == NULL || *str == '\0')
		trả_về -(errno = EINVAL);
	*num = strtold(str, &end);

	nếu (errno != 0)
		trả_về -errno;
	nếu (str == end || (end && *end))
		trả_về -(errno = EINVAL);
	trả_về 0;
}

long double strtold_or_err(const char *str, const char *errmesg)
{
	long double num = 0;

	nếu (ul_strtold(str, &num) == 0)
		trả_về num;
	nếu (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(const char *str, const char *errmesg)
{
	uintmax_t num;

	nếu (strtosize(str, &num) == 0)
		trả_về num;

	nếu (errno)
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
	trả_về (time_t) user_input;
}

bool hyperlinkwanted(const char *mode)
{
	nếu (mode && strcmp(mode, "never") == 0)
		trả_về false;

	nếu (mode && strcmp(mode, "always") == 0)
		trả_về true;

	nếu (!mode || strcmp(mode, "auto") == 0)
		trả_về isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), mode);
}

bool annotationwanted(const char *mode)
{
	nếu (mode && strcmp(mode, "never") == 0)
		trả_về false;

	nếu (mode && strcmp(mode, "always") == 0)
		trả_về true;

	nếu (!mode || strcmp(mode, "auto") == 0)
		trả_về isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), mode);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
char *xstrmode(mode_t mode, char *str)
{
	unsigned short i = 0;

	nếu (S_ISDIR(mode))
		str[i++] = 'd';
	không_thì nếu (S_ISLNK(mode))
		str[i++] = 'l';
	không_thì nếu (S_ISCHR(mode))
		str[i++] = 'c';
	không_thì nếu (S_ISBLK(mode))
		str[i++] = 'b';
	không_thì nếu (S_ISSOCK(mode))
		str[i++] = 's';
	không_thì nếu (S_ISFIFO(mode))
		str[i++] = 'p';
	không_thì nếu (S_ISREG(mode))
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

	trả_về str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
static số_nguyên get_exp(uint64_t n)
{
	số_nguyên shft;

	cho (shft = 10; shft <= 60; shft += 10) {
		nếu (n < (1ULL << shft))
			break;
	}
	trả_về shft - 10;
}

char *size_to_human_string(số_nguyên options, uint64_t bytes)
{
	char buf[32];
	số_nguyên dec, exp;
	uint64_t frac;
	const char *letters = "BKMGTPE";
	char suffix[sizeof(" KiB")], *psuf = suffix;
	char c;

	nếu (options & SIZE_SUFFIX_SPACE)
		*psuf++ = ' ';


	exp  = get_exp(bytes);
	c    = *(letters + (exp ? exp / 10 : 0));
	dec  = exp ? bytes / (1ULL << exp) : bytes;
	frac = exp ? bytes % (1ULL << exp) : 0;

	*psuf++ = c;

	nếu ((options & SIZE_SUFFIX_3LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/* round */
	nếu (frac) {
		/* get 3 digits after decimal point */
		nếu (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (exp - 10)) ;
		không_thì
			frac = (frac * 1000) / (1ULL << (exp)) ;

		nếu (options & SIZE_DECIMAL_2DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} không_thì {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		nếu (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	nếu (frac) {
		struct lconv const *l = localeconv();
		char *dp = l ? l->decimal_point : NULL;
		số_nguyên len;

		nếu (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, sizeof(buf), "%d%s%02" PRIu64, dec, dp, frac);
		nếu (len > 0 && (size_t) len < sizeof(buf)) {
			/* remove potential extraneous zero */
			nếu (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, sizeof(buf) - len);
		} không_thì
			*buf = '\0';	/* snprintf error */
	} không_thì
		snprintf(buf, sizeof(buf), "%d%s", dec, suffix);

	trả_về strdup(buf);
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
số_nguyên string_to_idarray(const char *list, số_nguyên ary[], size_t arysz,
			số_nguyên (name2id)(const char *, size_t))
{
	const char *begin = NULL, *p;
	size_t n = 0;

	nếu (!list || !*list || !ary || !arysz || !name2id)
		trả_về -1;

	cho (p = list; p && *p; p++) {
		const char *end = NULL;
		số_nguyên id;

		nếu (n >= arysz)
			trả_về -2;
		nếu (!begin)
			begin = p;		/* begin of the column name */
		nếu (*p == ',')
			end = p;		/* terminate the name */
		nếu (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		nếu (!begin || !end)
			continue;
		nếu (end <= begin)
			trả_về -1;

		id = name2id(begin, end - begin);
		nếu (id == -1)
			trả_về -1;
		ary[ n++ ] = id;
		begin = NULL;
		nếu (end && !*end)
			break;
	}
	trả_về n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
số_nguyên string_add_to_idarray(const char *list, số_nguyên ary[], size_t arysz,
			size_t *ary_pos, số_nguyên (name2id)(const char *, size_t))
{
	const char *list_add;
	số_nguyên r;

	nếu (!list || !*list || !ary_pos || *ary_pos > arysz)
		trả_về -1;

	nếu (list[0] == '+')
		list_add = &list[1];
	không_thì {
		list_add = list;
		*ary_pos = 0;
	}

	r = string_to_idarray(list_add, &ary[*ary_pos], arysz - *ary_pos, name2id);
	nếu (r > 0)
		*ary_pos += r;
	trả_về r;
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
số_nguyên string_to_bitarray(const char *list,
		     char *ary,
		     số_nguyên (*name2bit)(const char *, size_t),
		     size_t allow_range)
{
	const char *begin = NULL, *p;

	nếu (!list || !name2bit || !ary)
		trả_về -EINVAL;

	cho (p = list; p && *p; p++) {
		const char *end = NULL;
		số_nguyên bit, set_lower = 0, set_higher = 0;

		nếu (!begin)
			begin = p;		/* begin of the level name */
		nếu (*p == ',')
			end = p;		/* terminate the name */
		nếu (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		nếu (!begin || !end)
			continue;
		nếu (end <= begin)
			trả_về -1;
		nếu (allow_range) {
			nếu (*(end - 1) == '+') {
				end--;
				set_lower = 1;
			} không_thì nếu (*begin == '+') {
				begin++;
				set_higher = 1;
			}
		}

		bit = name2bit(begin, end - begin);
		nếu (bit < 0)
			trả_về bit;
		setbit(ary, bit);
		nếu (set_lower)
			trong_khi (--bit >= 0)
				setbit(ary, bit);
		không_thì nếu (set_higher)
			trong_khi (++bit < (số_nguyên) allow_range)
				setbit(ary, bit);
		begin = NULL;
		nếu (end && !*end)
			break;
	}
	trả_về 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
số_nguyên string_to_bitmask(const char *list,
		     unsigned long *mask,
		     long (*name2flag)(const char *, size_t))
{
	const char *begin = NULL, *p;

	nếu (!list || !name2flag || !mask)
		trả_về -EINVAL;

	cho (p = list; p && *p; p++) {
		const char *end = NULL;
		long flag;

		nếu (!begin)
			begin = p;		/* begin of the level name */
		nếu (*p == ',')
			end = p;		/* terminate the name */
		nếu (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		nếu (!begin || !end)
			continue;
		nếu (end <= begin)
			trả_về -1;

		flag = name2flag(begin, end - begin);
		nếu (flag < 0)
			trả_về flag;	/* error */
		*mask |= flag;
		begin = NULL;
		nếu (end && !*end)
			break;
	}
	trả_về 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
số_nguyên ul_parse_range(const char *str, số_nguyên *lower, số_nguyên *upper, số_nguyên def)
{
	char *end = NULL;

	nếu (!str)
		trả_về 0;

	*upper = *lower = def;
	errno = 0;

	nếu (*str == ':') {				/* <:N> */
		str++;
		*upper = strtol(str, &end, 10);
		nếu (errno || !end || *end || end == str)
			trả_về -1;
	} không_thì {
		*upper = *lower = strtol(str, &end, 10);
		nếu (errno || !end || end == str)
			trả_về -1;

		nếu (*end == ':' && !*(end + 1))		/* <M:> */
			*upper = def;
		không_thì nếu (*end == '-' || *end == ':') {	/* <M:N> <M-N> */
			str = end + 1;
			end = NULL;
			errno = 0;
			*upper = strtol(str, &end, 10);

			nếu (errno || !end || *end || end == str)
				trả_về -1;
		}
	}
	trả_về 0;
}

const char *ul_path_next_segment(const char *str, size_t *sz)
{
	const char *start, *p;

	start = str;
	*sz = 0;
	trong_khi (start && *start == '/' && *(start + 1) == '/')
		start++;

	nếu (!start || !*start)
		trả_về NULL;

	cho (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	trả_về start;
}

số_nguyên streq_paths(const char *a, const char *b)
{
	trong_khi (a && b) {
		size_t a_sz, b_sz;
		const char *a_seg = ul_path_next_segment(a, &a_sz);
		const char *b_seg = ul_path_next_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		nếu (a_sz + b_sz == 0)
			trả_về 1;

		/* ignore trailing slash */
		nếu (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			trả_về 1;

		nếu (!a_seg || !b_seg)
			break;
		nếu (a_sz != b_sz || strncmp(a_seg, b_seg, a_sz) != 0)
			break;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	trả_về 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
số_nguyên ul_path_is_within(const char *path, const char *dir)
{
	nếu (!path || !dir || !*path || !*dir)
		trả_về 0;

	trong_khi (path && dir) {
		size_t p_sz, d_sz;
		const char *p_seg = ul_path_next_segment(path, &p_sz);
		const char *d_seg = ul_path_next_segment(dir, &d_sz);

		/* all @dir segments matched */
		nếu (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			trả_về 1;

		nếu (!p_seg || !d_seg)
			break;
		nếu (p_sz != d_sz || strncmp(p_seg, d_seg, p_sz) != 0)
			break;

		path = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	trả_về 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
char *ul_strnconcat(const char *s, const char *suffix, size_t b)
{
	size_t a;
	char *r;

	nếu (!s && !suffix)
		trả_về strdup("");
	nếu (!s)
		trả_về strndup(suffix, b);
	nếu (!suffix)
		trả_về strdup(s);

	assert(s);
	assert(suffix);

	a = strlen(s);
	nếu (b > ((size_t) -1) - a)
		trả_về NULL;

	r = malloc(a + b + 1);
	nếu (!r)
		trả_về NULL;

	memcpy(r, s, a);
	memcpy(r + a, suffix, b);
	r[a+b] = 0;

	trả_về r;
}

/* concatenate two strings to a new string */
char *ul_strconcat(const char *s, const char *suffix)
{
	trả_về ul_strnconcat(s, suffix, suffix ? strlen(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
char *ul_strfconcat(const char *s, const char *format, ...)
{
	va_list ap;
	char *val, *res;
	số_nguyên sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	nếu (sz < 0)
		trả_về NULL;

	res = ul_strnconcat(s, val, sz);
	free(val);
	trả_về res;
}

số_nguyên ul_strappend(char **a, const char *b)
{
	size_t al, bl;
	char *tmp;

	nếu (!a)
		trả_về -EINVAL;
	nếu (!b || !*b)
		trả_về 0;
	nếu (!*a) {
		*a = strdup(b);
		trả_về !*a ? -ENOMEM : 0;
	}

	al = strlen(*a);
	bl = strlen(b);

	tmp = realloc(*a, al + bl + 1);
	nếu (!tmp)
		trả_về -ENOMEM;
	*a = tmp;
	memcpy((*a) + al, b, bl + 1);
	trả_về 0;
}

/* the hybrid version of strfconcat and strappend. */
số_nguyên strfappend(char **a, const char *format, ...)
{
	va_list ap;
	số_nguyên res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	trả_về res;
}

extern số_nguyên ul_strvfappend(char **a, const char *format, va_list ap)
{
	char *val;
	số_nguyên sz;
	số_nguyên res;

	sz = vasprintf(&val, format, ap);
	nếu (sz < 0)
		trả_về -errno;

	res = ul_strappend(a, val);
	free(val);
	trả_về res;
}

static size_t strcspn_escaped(const char *s, const char *reject)
{
	số_nguyên escaped = 0;
	số_nguyên n;

	cho (n=0; s[n]; n++) {
		nếu (escaped)
			escaped = 0;
		không_thì nếu (s[n] == '\\')
			escaped = 1;
		không_thì nếu (strchr(reject, s[n]))
			break;
	}

	/* if s ends in \, return index of previous char */
	trả_về n - escaped;
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
char *ul_strchr_escaped(const char *s, số_nguyên c)
{
	char *p;
	số_nguyên esc = 0;

	cho (p = (char *) s; p && *p; p++) {
		nếu (!esc && *p == '\\') {
			esc = 1;
			continue;
		}
		nếu (*p == c && (!esc || c == '\\'))
			trả_về p;
		esc = 0;
	}

	trả_về NULL;
}

/* Split a string into words. */
const char *ul_split(const char **state, size_t *l, const char *separator, số_nguyên quoted)
{
	const char *current;

	current = *state;

	nếu (!*current) {
		assert(**state == '\0');
		trả_về NULL;
	}

	current += strspn(current, separator);
	nếu (!*current) {
		*state = current;
		trả_về NULL;
	}

	nếu (quoted && strchr("\'\"", *current)) {
		char quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		nếu (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			trả_về NULL;
		}
		*state = current++ + *l + 2;
	} không_thì nếu (quoted) {
		*l = strcspn_escaped(current, separator);
		nếu (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			trả_về NULL;
		}
		*state = current + *l;
	} không_thì {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	trả_về current;
}

/* Rewind file pointer forward to new line.  */
số_nguyên skip_fline(FILE *fp)
{
	số_nguyên ch;

	do {
		nếu ((ch = fgetc(fp)) == EOF)
			trả_về 1;
		nếu (ch == '\n')
			trả_về 0;
	} trong_khi (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
số_nguyên ul_stralnumcmp(const char *p1, const char *p2)
{
	const unsigned char *s1 = (const unsigned char *) p1;
	const unsigned char *s2 = (const unsigned char *) p2;
	unsigned char c1, c2;

	do {
		do {
			c1 = (unsigned char) *s1++;
		} trong_khi (c1 != '\0' && !isalnum((unsigned số_nguyên) c1));

		do {
			c2 = (unsigned char) *s2++;
		} trong_khi (c2 != '\0' && !isalnum((unsigned số_nguyên) c2));

		nếu (c1 != '\0')
			c1 = tolower(c1);
		nếu (c2 != '\0')
			c2 = tolower(c2);
		nếu (c1 == '\0')
			trả_về c1 - c2;
	} trong_khi (c1 == c2);

	trả_về c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
số_nguyên ul_optstr_next(char **optstr, char **name, size_t *namesz,
		   char **value, size_t *valsz)
{
	số_nguyên open_quote = 0;
	char *start = NULL, *stop = NULL, *p, *sep = NULL;
	char *optstr0;

	assert(optstr);
	assert(*optstr);

	optstr0 = *optstr;

	nếu (name)
		*name = NULL;
	nếu (namesz)
		*namesz = 0;
	nếu (value)
		*value = NULL;
	nếu (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	trong_khi (optstr0 && *optstr0 == ',')
		optstr0++;

	cho (p = optstr0; p && *p; p++) {
		nếu (!start && *p == '=')
			trả_về -EINVAL;
		nếu (!start)
			start = p;		/* beginning of the option item */
		nếu (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		nếu (open_quote)
			continue;		/* still in quoted block */
		nếu (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		nếu (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		không_thì nếu (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		nếu (!start || !stop)
			continue;
		nếu (stop <= start)
			trả_về -EINVAL;

		nếu (name)
			*name = start;
		nếu (namesz)
			*namesz = sep ? sep - start : stop - start;
		*optstr = *stop ? stop + 1 : stop;

		nếu (sep) {
			nếu (value)
				*value = sep + 1;
			nếu (valsz)
				*valsz = stop - sep - 1;
		}
		trả_về 0;
	}

	trả_về 1;				/* end of optstr */
}

số_nguyên ul_optstr_is_valid(const char *optstr)
{
	số_nguyên rc;
	char *p = (char *) optstr;

	trong_khi ((rc = ul_optstr_next(&p, NULL, NULL, NULL, NULL)) == 0);
	trả_về rc < 0 ? 0 : 1;
}

char *ul_optstr_get_value(const char *optstr, const char *key)
{
	size_t sz, namesz = 0, valsz = 0;
	char *name = NULL, *value = NULL;
	char *p = (char *) optstr;

	nếu (!optstr || !key || !*key)
		trả_về NULL;

	sz = strlen(key);
	trong_khi (ul_optstr_next(&p, &name, &namesz, &value, &valsz) == 0) {
		nếu (namesz != sz || !valsz)
			continue;
		nếu (strncmp(name, key, namesz) == 0)
			trả_về strndup(value, valsz);
	}
	trả_về NULL;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(char *str)
{
	size_t len;
	char q;

	nếu (!str || !*str)
		trả_về 0;

	len = strlen(str);
	q = *str;

	nếu ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		trả_về len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	trả_về len - 2;
}

#ifdef TEST_PROGRAM_STRUTILS

struct testS {
	char *name;
	char *value;
};

static số_nguyên test_strdup_to_member(số_nguyên argc, char *argv[])
{
	struct testS *xx;

	nếu (argc < 3)
		trả_về EXIT_FAILURE;

	xx = calloc(1, sizeof(*xx));
	nếu (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_to_struct_member(xx, name, argv[1]);
	strdup_to_struct_member(xx, value, argv[2]);

	nếu (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->value, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	printf("1: '%s', 2: '%s'\n", xx->name, xx->value);

	free(xx->name);
	free(xx->value);
	free(xx);
	trả_về EXIT_SUCCESS;
}

static số_nguyên test_strutils_sizes(số_nguyên argc, char *argv[])
{
	uintmax_t size = 0;
	char *hum1, *hum2, *hum3;

	nếu (argc < 2)
		trả_về EXIT_FAILURE;

	nếu (strtosize(argv[1], &size))
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

	trả_về EXIT_SUCCESS;
}

static số_nguyên test_strutils_cmp_paths(số_nguyên argc, char *argv[])
{
	số_nguyên rc = streq_paths(argv[1], argv[2]);

	nếu (argc < 3)
		trả_về EXIT_FAILURE;

	printf("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	trả_về EXIT_SUCCESS;
}

static số_nguyên test_strutils_path_is_within(số_nguyên argc, char *argv[])
{
	số_nguyên rc;

	nếu (argc < 3)
		trả_về EXIT_FAILURE;

	rc = ul_path_is_within(argv[1], argv[2]);

	printf("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	trả_về EXIT_SUCCESS;
}

static số_nguyên test_strutils_normalize(số_nguyên argc, char *argv[])
{
	unsigned char *src, *dst, *org;
	size_t sz, len;

	nếu (argc < 2)
		trả_về EXIT_FAILURE;

	org = (unsigned char *) strdup(argv[1]);
	src = (unsigned char *) strdup((char *) org);
	len = strlen((char *) src);
	dst = malloc(len + 1);

	nếu (!org || !src || !dst)
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

	trả_về EXIT_SUCCESS;
}

static số_nguyên test_strutils_cstrcasecmp(số_nguyên argc, char *argv[])
{
	char *a, *b;

	nếu (argc < 3)
		trả_về EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	nếu (!a || !b)
		trả_về EXIT_FAILURE;

	printf("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	printf("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	printf("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, strlen(a)));

	trả_về EXIT_SUCCESS;
}

số_nguyên main(số_nguyên argc, char *argv[])
{
	nếu (argc == 3 && strcmp(argv[1], "--size") == 0) {
		trả_về test_strutils_sizes(argc - 1, argv + 1);

	} không_thì nếu (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		printf("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						NULL));
		trả_về EXIT_SUCCESS;
	} không_thì nếu (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		nếu (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		printf("'%s'-->%s\n", argv[2], val ? "true" : "false");
		trả_về EXIT_SUCCESS;

	} không_thì nếu (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		trả_về test_strutils_cmp_paths(argc - 1, argv + 1);

	} không_thì nếu (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		trả_về test_strutils_path_is_within(argc - 1, argv + 1);

	} không_thì nếu (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		trả_về test_strdup_to_member(argc - 1, argv + 1);

	} không_thì nếu  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		printf("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		trả_về EXIT_SUCCESS;

	} không_thì nếu (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		trả_về test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} không_thì nếu (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		trả_về test_strutils_normalize(argc - 1, argv + 1);

	} không_thì nếu (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		printf("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		trả_về EXIT_SUCCESS;
	} không_thì nếu (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		printf("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		trả_về EXIT_SUCCESS;
	} không_thì nếu (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		printf("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		trả_về EXIT_SUCCESS;
	} không_thì nếu (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		printf("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		trả_về EXIT_SUCCESS;
	} không_thì nếu (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		printf("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		trả_về EXIT_SUCCESS;
	} không_thì nếu (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		printf("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		trả_về EXIT_SUCCESS;

	} không_thì nếu (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		printf("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		trả_về EXIT_SUCCESS;

	} không_thì nếu (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		char *buf = "abc\0Y\0\0xyz\0X";
		char *end = buf + 12;
		char *p = buf;

		do {
			printf("str: '%s'\n", p);
		} trong_khi ((p = ul_next_string(p, end)));

		trả_về EXIT_SUCCESS;

	} không_thì nếu (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		char *name = NULL, *val = NULL;
		char *p = argv[2];
		số_nguyên rc;

		nếu (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		trong_khi ((rc = ul_optstr_next(&p, &name, &namesz, &val, &valsz)) == 0) {
			printf("'%.*s' : '%.*s'\n", (số_nguyên) namesz, name,
						   (số_nguyên) valsz, val);
		}
		nếu (rc == 1)
			trả_về EXIT_SUCCESS;
	} không_thì nếu (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		char *unquoted = strdup(argv[2]);

		nếu (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		printf("%s-->%s\n", argv[2], unquoted);
		free(unquoted);
		trả_về EXIT_SUCCESS;

	} không_thì {
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

	trả_về EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_STRUTILS */
