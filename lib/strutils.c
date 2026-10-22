/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Karel Zak <kzak@redhat.com> [2010]
 *          Davidlohr Bueso <dave@gnu.org> [2010]
 */
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <inttypes.h>
#ضمن <ctype.h>
#ضمن <errno.h>
#ضمن <sys/stat.h>
#ضمن <string.h>
#ضمن <strings.h>
#ضمن <assert.h>

#ضمن "c.h"
#ضمن "cctype.h"
#ضمن "nls.h"
#ضمن "strutils.h"
#ضمن "timeutils.h"
#ضمن "bitops.h"
#ضمن "pathnames.h"

ساكن صحيح STRTOXX_EXIT_CODE = EXIT_FAILURE;

فراغ strutils_set_exitcode(صحيح ex)
{
	STRTOXX_EXIT_CODE = ex;
}

ساكن صحيح do_scale_by_power (uintmax_t *x, صحيح base, صحيح power)
{
	כלעוד (power--) {
		אם (UINTMAX_MAX / base < *x)
			החזר -ERANGE;
		*x *= base;
	}
	החזר 0;
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
صحيح ul_parse_size(ثابت حرف *str, uintmax_t *res, صحيح *power)
{
	ثابت حرف *p;
	حرف *end;
	uintmax_t x, frac = 0;
	صحيح base = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	ساكن ثابت حرف *suf  = "KMGTPEZY";
	ساكن ثابت حرف *suf2 = "kmgtpezy";
	ثابت حرف *sp;

	*res = 0;

	אם (!str || !*str) {
		rc = -EINVAL;
		اذهب err;
	}

	/* Only positive numbers are acceptable
	 *
	 * Note that this check is not perfect, it would be better to
	 * use lconv->negative_sign. But coreutils use the same solution,
	 * so it's probably good enough...
	 */
	p = str;
	כלעוד (isspace((unsigned حرف) *p))
		p++;
	אם (*p == '-') {
		rc = -EINVAL;
		اذهب err;
	}

	errno = 0, end = عدم;
	x = strtoumax(str, &end, 0);

	אם (end == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		اذهب err;
	}
	אם (!end || !*end)
		اذهب done;			/* without suffix */
	p = end;

	/*
	 * Check size suffixes
	 */
check_suffix:
	אם (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		base = 1024;			/* XiB, 2^N */
	אחרת אם ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		base = 1000;			/* XB, 10^N */
	אחרת אם (*(p + 1)) {
		بنية lconv ثابت *l = localeconv();
		ثابت حرف *dp = l ? l->decimal_point : عدم;
		size_t dpsz = dp ? strlen(dp) : 0;

		אם (frac == 0 && *p && dp && strncmp(dp, p, dpsz) == 0) {
			ثابت حرف *fstr = p + dpsz;

			עבור (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			אם (isdigit(*fstr)) {
				errno = 0, end = عدم;
				frac = strtoumax(fstr, &end, 0);
				אם (end == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					اذهب err;
				}
			} אחרת
				end = (حرف *) p;

			אם (!end || !*end) {
				rc = -EINVAL;
				اذهب err;	/* without suffix, but with fractions */
			}
			p = end;
			اذهب check_suffix;
		}
		rc = -EINVAL;
		اذهب err;			/* unexpected suffix */
	}

	sp = strchr(suf, *p);
	אם (sp)
		pwr = (sp - suf) + 1;
	אחרת {
		sp = strchr(suf2, *p);
		אם (sp)
			pwr = (sp - suf2) + 1;
		אחרת {
			rc = -EINVAL;
			اذهب err;
		}
	}

	אם (power)
		*power = pwr;

	rc = do_scale_by_power(&x, base, pwr);
	אם (rc)
		اذهب err;

	אם (frac && pwr) {
		صحيح i;
		uintmax_t frac_base = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_base, base, pwr);
		אם (rc)
			اذهب err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		עשה {
			frac_val = ((frac % 10) * frac_base + frac_val) / 10;
			frac /= 10;
		} כלעוד (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		עבור (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		אם (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			اذهب err;
		}
		x += frac_val;
	}
done:
	*res = x;
err:
	אם (rc < 0)
		errno = -rc;
	החזר rc;
}

صحيح strtosize(ثابت حرف *str, uintmax_t *res)
{
	החזר ul_parse_size(str, res, عدم);
}

صحيح isdigit_strend(ثابت حرف *str, ثابت حرف **end)
{
	ثابت حرف *p;

	עבור (p = str; p && *p && isdigit((unsigned حرف) *p); p++);

	אם (end)
		*end = p;
	החזר p && p > str && !*p;
}

صحيح isxdigit_strend(ثابت حرف *str, ثابت حرف **end)
{
	ثابت حرف *p;

	עבור (p = str; p && *p && isxdigit((unsigned حرف) *p); p++);

	אם (end)
		*end = p;

	החזר p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
صحيح ul_strtobool(ثابت حرف *str, bool *result)
{
	ساكن ثابت حرف *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	ساكن ثابت حرف *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	אם (!str || !result)
		החזר -(errno = EINVAL);

	errno = 0;

	עבור (i = 0; i < ARRAY_SIZE(bool_true); i++) {
		אם (c_strcasecmp(str, bool_true[i]) == 0) {
			*result = true;
			החזר 0;
		}
	}

	עבור (i = 0; i < ARRAY_SIZE(bool_false); i++) {
		אם (c_strcasecmp(str, bool_false[i]) == 0) {
			*result = false;
			החזר 0;
		}
	}

	החזר -(errno = EINVAL);
}

صحيح ul_parse_switch(ثابت حرف *arg, ...)
{
	ثابت حرف *a, *b;
	va_list ap;

	va_start(ap, arg);
	עשה {
		a = va_arg(ap, حرف *);
		אם (!a)
			اكسر;
		b = va_arg(ap, حرف *);
		אם (!b)
			اكسر;

		אם (strcmp(arg, a) == 0) {
			va_end(ap);
			החזר 1;
		}

		אם (strcmp(arg, b) == 0) {
			va_end(ap);
			החזר 0;
		}
	} כלעוד (1);
	va_end(ap);

	errx(STRTOXX_EXIT_CODE, _("unsupported argument: %s"), arg);
}

#ifndef HAVE_MEMPCPY
فراغ *mempcpy(فراغ *restrict dest, ثابت فراغ *restrict src, size_t n)
{
	החזר ((حرف *)memcpy(dest, src, n)) + n;
}
#endif

#ifndef HAVE_STRNLEN
size_t strnlen(ثابت حرف *s, size_t maxlen)
{
	size_t i;

	עבור (i = 0; i < maxlen; i++) {
		אם (s[i] == '\0')
			החזר i;
	}
	החזר maxlen;
}
#endif

#ifndef HAVE_STRNCHR
حرف *strnchr(ثابت حرف *s, size_t maxlen, صحيح c)
{
	עבור (; maxlen-- && *s != '\0'; ++s)
		אם (*s == (حرف)c)
			החזר (حرف *)s;
	החזר عدم;
}
#endif

#ifndef HAVE_STRNDUP
حرف *strndup(ثابت حرف *s, size_t n)
{
	size_t len = strnlen(s, n);
	حرف *new = malloc((len + 1) * حجم(حرف));
	אם (!new)
		החזר عدم;
	new[len] = '\0';
	החזר (حرف *) memcpy(new, s, len);
}
#endif

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
صحيح ul_strtos64(ثابت حرف *str, int64_t *num, صحيح base)
{
	حرف *end = عدم;

	אם (str == عدم || *str == '\0')
		החזר -(errno = EINVAL);

	errno = 0;
	*num = (int64_t) strtoimax(str, &end, base);

	אם (errno != 0)
		החזר -errno;
	אם (str == end || (end && *end))
		החזר -(errno = EINVAL);
	החזר 0;
}

صحيح ul_strtou64(ثابت حرف *str, uint64_t *num, صحيح base)
{
	حرف *end = عدم;
	int64_t tmp;

	אם (str == عدم || *str == '\0')
		החזר -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &end, base);
	אם (tmp < 0)
		errno = ERANGE;
	אחרת {
		errno = 0;
		*num = strtoumax(str, &end, base);
	}

	אם (errno != 0)
		החזר -errno;
	אם (str == end || (end && *end))
		החזר -(errno = EINVAL);
	החזר 0;
}

صحيح ul_strtos32(ثابت حرف *str, int32_t *num, صحيح base)
{
	int64_t tmp;
	صحيح rc;

	rc = ul_strtos64(str, &tmp, base);
	אם (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	אם (rc == 0)
		*num = (int32_t) tmp;
	החזר rc;
}

صحيح ul_strtou32(ثابت حرف *str, uint32_t *num, صحيح base)
{
	uint64_t tmp;
	صحيح rc;

	rc = ul_strtou64(str, &tmp, base);
	אם (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	אם (rc == 0)
		*num = (uint32_t) tmp;
	החזר rc;
}

صحيح ul_strtou16(ثابت حرف *str, uint16_t *num, صحيح base)
{
	uint64_t tmp;
	صحيح rc;

	rc = ul_strtou64(str, &tmp, base);
	אם (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	אם (rc == 0)
		*num = (uint16_t) tmp;
	החזר rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(ثابت حرف *str, صحيح base, ثابت حرف *errmesg,
			     int64_t low, int64_t up)
{
	int64_t num = 0;
	صحيح rc;

	rc = ul_strtos64(str, &num, base);
	אם (rc == 0 && ((low && num < low) || (up && num > up)))
		rc = -(errno = ERANGE);

	אם (rc) {
		אם (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	החזר num;
}

uint64_t str2unum_or_err(ثابت حرف *str, صحيح base, ثابت حرف *errmesg, uint64_t up)
{
	uint64_t num = 0;
	صحيح rc;

	rc = ul_strtou64(str, &num, base);
	אם (rc == 0 && (up && num > up))
		rc = -(errno = ERANGE);

	אם (rc) {
		אם (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	החזר num;
}

مزدوج strtod_or_err(ثابت حرف *str, ثابت حرف *errmesg)
{
	مزدوج num;
	حرف *end = عدم;

	errno = 0;
	אם (str == عدم || *str == '\0')
		اذهب err;
	num = strtod(str, &end);

	אם (errno || str == end || (end && *end))
		اذهب err;

	החזר num;
err:
	אם (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

صحيح ul_strtold(ثابت حرف *str, long مزدوج *num)
{
	حرف *end = عدم;

	errno = 0;
	אם (str == عدم || *str == '\0')
		החזר -(errno = EINVAL);
	*num = strtold(str, &end);

	אם (errno != 0)
		החזר -errno;
	אם (str == end || (end && *end))
		החזר -(errno = EINVAL);
	החזר 0;
}

long مزدوج strtold_or_err(ثابت حرف *str, ثابت حرف *errmesg)
{
	long مزدوج num = 0;

	אם (ul_strtold(str, &num) == 0)
		החזר num;
	אם (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(ثابت حرف *str, ثابت حرف *errmesg)
{
	uintmax_t num;

	אם (strtosize(str, &num) == 0)
		החזר num;

	אם (errno)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}


فراغ strtotimeval_or_err(ثابت حرف *str, بنية timeval *tv, ثابت حرف *errmesg)
{
	long مزدوج user_input;

	user_input = strtold_or_err(str, errmesg);
	tv->tv_sec = (time_t) user_input;
	tv->tv_usec = (suseconds_t)((user_input - tv->tv_sec) * USEC_PER_SEC);
}

فراغ strtotimespec_or_err(ثابت حرف *str, بنية timespec *ts, ثابت حرف *errmesg)
{
	long مزدوج user_input;

	user_input = strtold_or_err(str, errmesg);
	ts->tv_sec = (time_t) user_input;
	ts->tv_nsec = (long)((user_input - ts->tv_sec) * NSEC_PER_SEC);
}

time_t strtotime_or_err(ثابت حرف *str, ثابت حرف *errmesg)
{
	int64_t user_input;

	user_input = strtos64_or_err(str, errmesg);
	החזר (time_t) user_input;
}

bool hyperlinkwanted(ثابت حرف *mode)
{
	אם (mode && strcmp(mode, "never") == 0)
		החזר false;

	אם (mode && strcmp(mode, "always") == 0)
		החזר true;

	אם (!mode || strcmp(mode, "auto") == 0)
		החזר isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), mode);
}

bool annotationwanted(ثابت حرف *mode)
{
	אם (mode && strcmp(mode, "never") == 0)
		החזר false;

	אם (mode && strcmp(mode, "always") == 0)
		החזר true;

	אם (!mode || strcmp(mode, "auto") == 0)
		החזר isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), mode);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
حرف *xstrmode(mode_t mode, حرف *str)
{
	unsigned short i = 0;

	אם (S_ISDIR(mode))
		str[i++] = 'd';
	אחרת אם (S_ISLNK(mode))
		str[i++] = 'l';
	אחרת אם (S_ISCHR(mode))
		str[i++] = 'c';
	אחרת אם (S_ISBLK(mode))
		str[i++] = 'b';
	אחרת אם (S_ISSOCK(mode))
		str[i++] = 's';
	אחרת אם (S_ISFIFO(mode))
		str[i++] = 'p';
	אחרת אם (S_ISREG(mode))
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

	החזר str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
ساكن صحيح get_exp(uint64_t n)
{
	صحيح shft;

	עבור (shft = 10; shft <= 60; shft += 10) {
		אם (n < (1ULL << shft))
			اكسر;
	}
	החזר shft - 10;
}

حرف *size_to_human_string(صحيح options, uint64_t bytes)
{
	حرف buf[32];
	صحيح dec, exp;
	uint64_t frac;
	ثابت حرف *letters = "BKMGTPE";
	حرف suffix[حجم(" KiB")], *psuf = suffix;
	حرف c;

	אם (options & SIZE_SUFFIX_SPACE)
		*psuf++ = ' ';


	exp  = get_exp(bytes);
	c    = *(letters + (exp ? exp / 10 : 0));
	dec  = exp ? bytes / (1ULL << exp) : bytes;
	frac = exp ? bytes % (1ULL << exp) : 0;

	*psuf++ = c;

	אם ((options & SIZE_SUFFIX_3LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/* round */
	אם (frac) {
		/* get 3 digits after decimal point */
		אם (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (exp - 10)) ;
		אחרת
			frac = (frac * 1000) / (1ULL << (exp)) ;

		אם (options & SIZE_DECIMAL_2DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} אחרת {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		אם (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	אם (frac) {
		بنية lconv ثابت *l = localeconv();
		حرف *dp = l ? l->decimal_point : عدم;
		صحيح len;

		אם (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, حجم(buf), "%d%s%02" PRIu64, dec, dp, frac);
		אם (len > 0 && (size_t) len < حجم(buf)) {
			/* remove potential extraneous zero */
			אם (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, حجم(buf) - len);
		} אחרת
			*buf = '\0';	/* snprintf error */
	} אחרת
		snprintf(buf, حجم(buf), "%d%s", dec, suffix);

	החזר strdup(buf);
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
صحيح string_to_idarray(ثابت حرف *list, صحيح ary[], size_t arysz,
			صحيح (name2id)(ثابت حرف *, size_t))
{
	ثابت حرف *begin = عدم, *p;
	size_t n = 0;

	אם (!list || !*list || !ary || !arysz || !name2id)
		החזר -1;

	עבור (p = list; p && *p; p++) {
		ثابت حرف *end = عدم;
		صحيح id;

		אם (n >= arysz)
			החזר -2;
		אם (!begin)
			begin = p;		/* begin of the column name */
		אם (*p == ',')
			end = p;		/* terminate the name */
		אם (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		אם (!begin || !end)
			استمر;
		אם (end <= begin)
			החזר -1;

		id = name2id(begin, end - begin);
		אם (id == -1)
			החזר -1;
		ary[ n++ ] = id;
		begin = عدم;
		אם (end && !*end)
			اكسر;
	}
	החזר n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
صحيح string_add_to_idarray(ثابت حرف *list, صحيح ary[], size_t arysz,
			size_t *ary_pos, صحيح (name2id)(ثابت حرف *, size_t))
{
	ثابت حرف *list_add;
	صحيح r;

	אם (!list || !*list || !ary_pos || *ary_pos > arysz)
		החזר -1;

	אם (list[0] == '+')
		list_add = &list[1];
	אחרת {
		list_add = list;
		*ary_pos = 0;
	}

	r = string_to_idarray(list_add, &ary[*ary_pos], arysz - *ary_pos, name2id);
	אם (r > 0)
		*ary_pos += r;
	החזר r;
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
صحيح string_to_bitarray(ثابت حرف *list,
		     حرف *ary,
		     صحيح (*name2bit)(ثابت حرف *, size_t),
		     size_t allow_range)
{
	ثابت حرف *begin = عدم, *p;

	אם (!list || !name2bit || !ary)
		החזר -EINVAL;

	עבור (p = list; p && *p; p++) {
		ثابت حرف *end = عدم;
		صحيح bit, set_lower = 0, set_higher = 0;

		אם (!begin)
			begin = p;		/* begin of the level name */
		אם (*p == ',')
			end = p;		/* terminate the name */
		אם (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		אם (!begin || !end)
			استمر;
		אם (end <= begin)
			החזר -1;
		אם (allow_range) {
			אם (*(end - 1) == '+') {
				end--;
				set_lower = 1;
			} אחרת אם (*begin == '+') {
				begin++;
				set_higher = 1;
			}
		}

		bit = name2bit(begin, end - begin);
		אם (bit < 0)
			החזר bit;
		setbit(ary, bit);
		אם (set_lower)
			כלעוד (--bit >= 0)
				setbit(ary, bit);
		אחרת אם (set_higher)
			כלעוד (++bit < (صحيح) allow_range)
				setbit(ary, bit);
		begin = عدم;
		אם (end && !*end)
			اكسر;
	}
	החזר 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
صحيح string_to_bitmask(ثابت حرف *list,
		     unsigned long *mask,
		     long (*name2flag)(ثابت حرف *, size_t))
{
	ثابت حرف *begin = عدم, *p;

	אם (!list || !name2flag || !mask)
		החזר -EINVAL;

	עבור (p = list; p && *p; p++) {
		ثابت حرف *end = عدم;
		long flag;

		אם (!begin)
			begin = p;		/* begin of the level name */
		אם (*p == ',')
			end = p;		/* terminate the name */
		אם (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		אם (!begin || !end)
			استمر;
		אם (end <= begin)
			החזר -1;

		flag = name2flag(begin, end - begin);
		אם (flag < 0)
			החזר flag;	/* error */
		*mask |= flag;
		begin = عدم;
		אם (end && !*end)
			اكسر;
	}
	החזר 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
صحيح ul_parse_range(ثابت حرف *str, صحيح *lower, صحيح *upper, صحيح def)
{
	حرف *end = عدم;

	אם (!str)
		החזר 0;

	*upper = *lower = def;
	errno = 0;

	אם (*str == ':') {				/* <:N> */
		str++;
		*upper = strtol(str, &end, 10);
		אם (errno || !end || *end || end == str)
			החזר -1;
	} אחרת {
		*upper = *lower = strtol(str, &end, 10);
		אם (errno || !end || end == str)
			החזר -1;

		אם (*end == ':' && !*(end + 1))		/* <M:> */
			*upper = def;
		אחרת אם (*end == '-' || *end == ':') {	/* <M:N> <M-N> */
			str = end + 1;
			end = عدم;
			errno = 0;
			*upper = strtol(str, &end, 10);

			אם (errno || !end || *end || end == str)
				החזר -1;
		}
	}
	החזר 0;
}

ثابت حرف *ul_path_next_segment(ثابت حرف *str, size_t *sz)
{
	ثابت حرف *start, *p;

	start = str;
	*sz = 0;
	כלעוד (start && *start == '/' && *(start + 1) == '/')
		start++;

	אם (!start || !*start)
		החזר عدم;

	עבור (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	החזר start;
}

صحيح streq_paths(ثابت حرف *a, ثابت حرف *b)
{
	כלעוד (a && b) {
		size_t a_sz, b_sz;
		ثابت حرف *a_seg = ul_path_next_segment(a, &a_sz);
		ثابت حرف *b_seg = ul_path_next_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		אם (a_sz + b_sz == 0)
			החזר 1;

		/* ignore trailing slash */
		אם (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			החזר 1;

		אם (!a_seg || !b_seg)
			اكسر;
		אם (a_sz != b_sz || strncmp(a_seg, b_seg, a_sz) != 0)
			اكسر;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	החזר 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
صحيح ul_path_is_within(ثابت حرف *path, ثابت حرف *dir)
{
	אם (!path || !dir || !*path || !*dir)
		החזר 0;

	כלעוד (path && dir) {
		size_t p_sz, d_sz;
		ثابت حرف *p_seg = ul_path_next_segment(path, &p_sz);
		ثابت حرف *d_seg = ul_path_next_segment(dir, &d_sz);

		/* all @dir segments matched */
		אם (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			החזר 1;

		אם (!p_seg || !d_seg)
			اكسر;
		אם (p_sz != d_sz || strncmp(p_seg, d_seg, p_sz) != 0)
			اكسر;

		path = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	החזר 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
حرف *ul_strnconcat(ثابت حرف *s, ثابت حرف *suffix, size_t b)
{
	size_t a;
	حرف *r;

	אם (!s && !suffix)
		החזר strdup("");
	אם (!s)
		החזר strndup(suffix, b);
	אם (!suffix)
		החזר strdup(s);

	أكد(s);
	أكد(suffix);

	a = strlen(s);
	אם (b > ((size_t) -1) - a)
		החזר عدم;

	r = malloc(a + b + 1);
	אם (!r)
		החזר عدم;

	memcpy(r, s, a);
	memcpy(r + a, suffix, b);
	r[a+b] = 0;

	החזר r;
}

/* concatenate two strings to a new string */
حرف *ul_strconcat(ثابت حرف *s, ثابت حرف *suffix)
{
	החזר ul_strnconcat(s, suffix, suffix ? strlen(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
حرف *ul_strfconcat(ثابت حرف *s, ثابت حرف *format, ...)
{
	va_list ap;
	حرف *val, *res;
	صحيح sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	אם (sz < 0)
		החזר عدم;

	res = ul_strnconcat(s, val, sz);
	حرر(val);
	החזר res;
}

صحيح ul_strappend(حرف **a, ثابت حرف *b)
{
	size_t al, bl;
	حرف *tmp;

	אם (!a)
		החזר -EINVAL;
	אם (!b || !*b)
		החזר 0;
	אם (!*a) {
		*a = strdup(b);
		החזר !*a ? -ENOMEM : 0;
	}

	al = strlen(*a);
	bl = strlen(b);

	tmp = realloc(*a, al + bl + 1);
	אם (!tmp)
		החזר -ENOMEM;
	*a = tmp;
	memcpy((*a) + al, b, bl + 1);
	החזר 0;
}

/* the hybrid version of strfconcat and strappend. */
صحيح strfappend(حرف **a, ثابت حرف *format, ...)
{
	va_list ap;
	صحيح res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	החזר res;
}

خارجي صحيح ul_strvfappend(حرف **a, ثابت حرف *format, va_list ap)
{
	حرف *val;
	صحيح sz;
	صحيح res;

	sz = vasprintf(&val, format, ap);
	אם (sz < 0)
		החזר -errno;

	res = ul_strappend(a, val);
	حرر(val);
	החזר res;
}

ساكن size_t strcspn_escaped(ثابت حرف *s, ثابت حرف *reject)
{
	صحيح escaped = 0;
	صحيح n;

	עבור (n=0; s[n]; n++) {
		אם (escaped)
			escaped = 0;
		אחרת אם (s[n] == '\\')
			escaped = 1;
		אחרת אם (strchr(reject, s[n]))
			اكسر;
	}

	/* if s ends in \, return index of previous char */
	החזר n - escaped;
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
حرف *ul_strchr_escaped(ثابت حرف *s, صحيح c)
{
	حرف *p;
	صحيح esc = 0;

	עבור (p = (حرف *) s; p && *p; p++) {
		אם (!esc && *p == '\\') {
			esc = 1;
			استمر;
		}
		אם (*p == c && (!esc || c == '\\'))
			החזר p;
		esc = 0;
	}

	החזר عدم;
}

/* Split a string into words. */
ثابت حرف *ul_split(ثابت حرف **state, size_t *l, ثابت حرف *separator, صحيح quoted)
{
	ثابت حرف *current;

	current = *state;

	אם (!*current) {
		أكد(**state == '\0');
		החזר عدم;
	}

	current += strspn(current, separator);
	אם (!*current) {
		*state = current;
		החזר عدم;
	}

	אם (quoted && strchr("\'\"", *current)) {
		حرف quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		אם (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			החזר عدم;
		}
		*state = current++ + *l + 2;
	} אחרת אם (quoted) {
		*l = strcspn_escaped(current, separator);
		אם (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			החזר عدم;
		}
		*state = current + *l;
	} אחרת {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	החזר current;
}

/* Rewind file pointer forward to new line.  */
صحيح skip_fline(FILE *fp)
{
	صحيح ch;

	עשה {
		אם ((ch = fgetc(fp)) == EOF)
			החזר 1;
		אם (ch == '\n')
			החזר 0;
	} כלעוד (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
صحيح ul_stralnumcmp(ثابت حرف *p1, ثابت حرف *p2)
{
	ثابت unsigned حرف *s1 = (ثابت unsigned حرف *) p1;
	ثابت unsigned حرف *s2 = (ثابت unsigned حرف *) p2;
	unsigned حرف c1, c2;

	עשה {
		עשה {
			c1 = (unsigned حرف) *s1++;
		} כלעוד (c1 != '\0' && !isalnum((unsigned صحيح) c1));

		עשה {
			c2 = (unsigned حرف) *s2++;
		} כלעוד (c2 != '\0' && !isalnum((unsigned صحيح) c2));

		אם (c1 != '\0')
			c1 = tolower(c1);
		אם (c2 != '\0')
			c2 = tolower(c2);
		אם (c1 == '\0')
			החזר c1 - c2;
	} כלעוד (c1 == c2);

	החזר c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
صحيح ul_optstr_next(حرف **optstr, حرف **name, size_t *namesz,
		   حرف **value, size_t *valsz)
{
	صحيح open_quote = 0;
	حرف *start = عدم, *stop = عدم, *p, *sep = عدم;
	حرف *optstr0;

	أكد(optstr);
	أكد(*optstr);

	optstr0 = *optstr;

	אם (name)
		*name = عدم;
	אם (namesz)
		*namesz = 0;
	אם (value)
		*value = عدم;
	אם (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	כלעוד (optstr0 && *optstr0 == ',')
		optstr0++;

	עבור (p = optstr0; p && *p; p++) {
		אם (!start && *p == '=')
			החזר -EINVAL;
		אם (!start)
			start = p;		/* beginning of the option item */
		אם (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		אם (open_quote)
			استمر;		/* still in quoted block */
		אם (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		אם (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		אחרת אם (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		אם (!start || !stop)
			استمر;
		אם (stop <= start)
			החזר -EINVAL;

		אם (name)
			*name = start;
		אם (namesz)
			*namesz = sep ? sep - start : stop - start;
		*optstr = *stop ? stop + 1 : stop;

		אם (sep) {
			אם (value)
				*value = sep + 1;
			אם (valsz)
				*valsz = stop - sep - 1;
		}
		החזר 0;
	}

	החזר 1;				/* end of optstr */
}

صحيح ul_optstr_is_valid(ثابت حرف *optstr)
{
	صحيح rc;
	حرف *p = (حرف *) optstr;

	כלעוד ((rc = ul_optstr_next(&p, عدم, عدم, عدم, عدم)) == 0);
	החזר rc < 0 ? 0 : 1;
}

حرف *ul_optstr_get_value(ثابت حرف *optstr, ثابت حرف *key)
{
	size_t sz, namesz = 0, valsz = 0;
	حرف *name = عدم, *value = عدم;
	حرف *p = (حرف *) optstr;

	אם (!optstr || !key || !*key)
		החזר عدم;

	sz = strlen(key);
	כלעוד (ul_optstr_next(&p, &name, &namesz, &value, &valsz) == 0) {
		אם (namesz != sz || !valsz)
			استمر;
		אם (strncmp(name, key, namesz) == 0)
			החזר strndup(value, valsz);
	}
	החזר عدم;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(حرف *str)
{
	size_t len;
	حرف q;

	אם (!str || !*str)
		החזר 0;

	len = strlen(str);
	q = *str;

	אם ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		החזר len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	החזר len - 2;
}

#ifdef TEST_PROGRAM_STRUTILS

بنية testS {
	حرف *name;
	حرف *value;
};

ساكن صحيح test_strdup_to_member(صحيح argc, حرف *argv[])
{
	بنية testS *xx;

	אם (argc < 3)
		החזר EXIT_FAILURE;

	xx = calloc(1, حجم(*xx));
	אם (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_to_struct_member(xx, name, argv[1]);
	strdup_to_struct_member(xx, value, argv[2]);

	אם (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->value, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	printf("1: '%s', 2: '%s'\n", xx->name, xx->value);

	حرر(xx->name);
	حرر(xx->value);
	حرر(xx);
	החזר EXIT_SUCCESS;
}

ساكن صحيح test_strutils_sizes(صحيح argc, حرف *argv[])
{
	uintmax_t size = 0;
	حرف *hum1, *hum2, *hum3;

	אם (argc < 2)
		החזר EXIT_FAILURE;

	אם (strtosize(argv[1], &size))
		errx(EXIT_FAILURE, "invalid size '%s' value", argv[1]);

	hum1 = size_to_human_string(SIZE_SUFFIX_1LETTER, size);
	hum2 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE, size);
	hum3 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE |
				    SIZE_DECIMAL_2DIGITS, size);

	printf("%25s : %20ju : %8s : %12s : %13s\n", argv[1], size, hum1, hum2, hum3);
	حرر(hum1);
	حرر(hum2);
	حرر(hum3);

	החזר EXIT_SUCCESS;
}

ساكن صحيح test_strutils_cmp_paths(صحيح argc, حرف *argv[])
{
	صحيح rc = streq_paths(argv[1], argv[2]);

	אם (argc < 3)
		החזר EXIT_FAILURE;

	printf("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	החזר EXIT_SUCCESS;
}

ساكن صحيح test_strutils_path_is_within(صحيح argc, حرف *argv[])
{
	صحيح rc;

	אם (argc < 3)
		החזר EXIT_FAILURE;

	rc = ul_path_is_within(argv[1], argv[2]);

	printf("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	החזר EXIT_SUCCESS;
}

ساكن صحيح test_strutils_normalize(صحيح argc, حرف *argv[])
{
	unsigned حرف *src, *dst, *org;
	size_t sz, len;

	אם (argc < 2)
		החזר EXIT_FAILURE;

	org = (unsigned حرف *) strdup(argv[1]);
	src = (unsigned حرف *) strdup((حرف *) org);
	len = strlen((حرف *) src);
	dst = malloc(len + 1);

	אם (!org || !src || !dst)
		اذهب done;

	/* two buffers */
	sz = __normalize_whitespace(src, len, dst, len + 1);
	printf("1: '%s' --> '%s' [sz=%zu]\n", src, dst, sz);

	/* one buffer */
	sz = normalize_whitespace(src);
	printf("2: '%s' --> '%s' [sz=%zu]\n", org, src, sz);

done:
	حرر(src);
	حرر(dst);
	حرر(org);

	החזר EXIT_SUCCESS;
}

ساكن صحيح test_strutils_cstrcasecmp(صحيح argc, حرف *argv[])
{
	حرف *a, *b;

	אם (argc < 3)
		החזר EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	אם (!a || !b)
		החזר EXIT_FAILURE;

	printf("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	printf("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	printf("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, strlen(a)));

	החזר EXIT_SUCCESS;
}

صحيح main(صحيح argc, حرف *argv[])
{
	אם (argc == 3 && strcmp(argv[1], "--size") == 0) {
		החזר test_strutils_sizes(argc - 1, argv + 1);

	} אחרת אם (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		printf("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						عدم));
		החזר EXIT_SUCCESS;
	} אחרת אם (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		אם (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		printf("'%s'-->%s\n", argv[2], val ? "true" : "false");
		החזר EXIT_SUCCESS;

	} אחרת אם (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		החזר test_strutils_cmp_paths(argc - 1, argv + 1);

	} אחרת אם (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		החזר test_strutils_path_is_within(argc - 1, argv + 1);

	} אחרת אם (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		החזר test_strdup_to_member(argc - 1, argv + 1);

	} אחרת אם  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		printf("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		החזר EXIT_SUCCESS;

	} אחרת אם (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		החזר test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} אחרת אם (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		החזר test_strutils_normalize(argc - 1, argv + 1);

	} אחרת אם (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		printf("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		החזר EXIT_SUCCESS;
	} אחרת אם (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		printf("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		החזר EXIT_SUCCESS;
	} אחרת אם (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		printf("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		החזר EXIT_SUCCESS;
	} אחרת אם (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		printf("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		החזר EXIT_SUCCESS;
	} אחרת אם (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		printf("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		החזר EXIT_SUCCESS;
	} אחרת אם (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		printf("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		החזר EXIT_SUCCESS;

	} אחרת אם (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		printf("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		החזר EXIT_SUCCESS;

	} אחרת אם (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		حرف *buf = "abc\0Y\0\0xyz\0X";
		حرف *end = buf + 12;
		حرف *p = buf;

		עשה {
			printf("str: '%s'\n", p);
		} כלעוד ((p = ul_next_string(p, end)));

		החזר EXIT_SUCCESS;

	} אחרת אם (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		حرف *name = عدم, *val = عدم;
		حرف *p = argv[2];
		صحيح rc;

		אם (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		כלעוד ((rc = ul_optstr_next(&p, &name, &namesz, &val, &valsz)) == 0) {
			printf("'%.*s' : '%.*s'\n", (صحيح) namesz, name,
						   (صحيح) valsz, val);
		}
		אם (rc == 1)
			החזר EXIT_SUCCESS;
	} אחרת אם (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		حرف *unquoted = strdup(argv[2]);

		אם (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		printf("%s-->%s\n", argv[2], unquoted);
		حرر(unquoted);
		החזר EXIT_SUCCESS;

	} אחרת {
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

	החזר EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_STRUTILS */
