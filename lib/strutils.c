/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Karel Zak <kzak@redhat.com> [2010]
 *          Davidlohr Bueso <dave@gnu.org> [2010]
 */
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <inttypes.h>
#شامل <ctype.h>
#شامل <errno.h>
#شامل <sys/stat.h>
#شامل <string.h>
#شامل <strings.h>
#شامل <assert.h>

#شامل "c.h"
#شامل "cctype.h"
#شامل "nls.h"
#شامل "strutils.h"
#شامل "timeutils.h"
#شامل "bitops.h"
#شامل "pathnames.h"

ایستا صحیح STRTOXX_EXIT_CODE = EXIT_FAILURE;

تهی strutils_set_exitcode(صحیح ex)
{
	STRTOXX_EXIT_CODE = ex;
}

ایستا صحیح do_scale_by_power (uintmax_t *x, صحیح base, صحیح power)
{
	وقتی (power--) {
		اگر (UINTMAX_MAX / base < *x)
			بازگشت -ERANGE;
		*x *= base;
	}
	بازگشت 0;
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
صحیح ul_parse_size(ثابت نویسه *str, uintmax_t *res, صحیح *power)
{
	ثابت نویسه *p;
	نویسه *end;
	uintmax_t x, frac = 0;
	صحیح base = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	ایستا ثابت نویسه *suf  = "KMGTPEZY";
	ایستا ثابت نویسه *suf2 = "kmgtpezy";
	ثابت نویسه *sp;

	*res = 0;

	اگر (!str || !*str) {
		rc = -EINVAL;
		برو err;
	}

	/* Only positive numbers are acceptable
	 *
	 * Note that this check is not perfect, it would be better to
	 * use lconv->negative_sign. But coreutils use the same solution,
	 * so it's probably good enough...
	 */
	p = str;
	وقتی (isspace((unsigned نویسه) *p))
		p++;
	اگر (*p == '-') {
		rc = -EINVAL;
		برو err;
	}

	errno = 0, end = پوچ;
	x = strtoumax(str, &end, 0);

	اگر (end == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		برو err;
	}
	اگر (!end || !*end)
		برو done;			/* without suffix */
	p = end;

	/*
	 * Check size suffixes
	 */
check_suffix:
	اگر (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		base = 1024;			/* XiB, 2^N */
	وگرنه اگر ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		base = 1000;			/* XB, 10^N */
	وگرنه اگر (*(p + 1)) {
		ساختار lconv ثابت *l = localeconv();
		ثابت نویسه *dp = l ? l->decimal_point : پوچ;
		size_t dpsz = dp ? strlen(dp) : 0;

		اگر (frac == 0 && *p && dp && strncmp(dp, p, dpsz) == 0) {
			ثابت نویسه *fstr = p + dpsz;

			برای (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			اگر (isdigit(*fstr)) {
				errno = 0, end = پوچ;
				frac = strtoumax(fstr, &end, 0);
				اگر (end == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					برو err;
				}
			} وگرنه
				end = (نویسه *) p;

			اگر (!end || !*end) {
				rc = -EINVAL;
				برو err;	/* without suffix, but with fractions */
			}
			p = end;
			برو check_suffix;
		}
		rc = -EINVAL;
		برو err;			/* unexpected suffix */
	}

	sp = strchr(suf, *p);
	اگر (sp)
		pwr = (sp - suf) + 1;
	وگرنه {
		sp = strchr(suf2, *p);
		اگر (sp)
			pwr = (sp - suf2) + 1;
		وگرنه {
			rc = -EINVAL;
			برو err;
		}
	}

	اگر (power)
		*power = pwr;

	rc = do_scale_by_power(&x, base, pwr);
	اگر (rc)
		برو err;

	اگر (frac && pwr) {
		صحیح i;
		uintmax_t frac_base = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_base, base, pwr);
		اگر (rc)
			برو err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		انجامبده {
			frac_val = ((frac % 10) * frac_base + frac_val) / 10;
			frac /= 10;
		} وقتی (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		برای (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		اگر (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			برو err;
		}
		x += frac_val;
	}
done:
	*res = x;
err:
	اگر (rc < 0)
		errno = -rc;
	بازگشت rc;
}

صحیح strtosize(ثابت نویسه *str, uintmax_t *res)
{
	بازگشت ul_parse_size(str, res, پوچ);
}

صحیح isdigit_strend(ثابت نویسه *str, ثابت نویسه **end)
{
	ثابت نویسه *p;

	برای (p = str; p && *p && isdigit((unsigned نویسه) *p); p++);

	اگر (end)
		*end = p;
	بازگشت p && p > str && !*p;
}

صحیح isxdigit_strend(ثابت نویسه *str, ثابت نویسه **end)
{
	ثابت نویسه *p;

	برای (p = str; p && *p && isxdigit((unsigned نویسه) *p); p++);

	اگر (end)
		*end = p;

	بازگشت p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
صحیح ul_strtobool(ثابت نویسه *str, bool *result)
{
	ایستا ثابت نویسه *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	ایستا ثابت نویسه *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	اگر (!str || !result)
		بازگشت -(errno = EINVAL);

	errno = 0;

	برای (i = 0; i < ARRAY_SIZE(bool_true); i++) {
		اگر (c_strcasecmp(str, bool_true[i]) == 0) {
			*result = true;
			بازگشت 0;
		}
	}

	برای (i = 0; i < ARRAY_SIZE(bool_false); i++) {
		اگر (c_strcasecmp(str, bool_false[i]) == 0) {
			*result = false;
			بازگشت 0;
		}
	}

	بازگشت -(errno = EINVAL);
}

صحیح ul_parse_switch(ثابت نویسه *arg, ...)
{
	ثابت نویسه *a, *b;
	va_list ap;

	va_start(ap, arg);
	انجامبده {
		a = va_arg(ap, نویسه *);
		اگر (!a)
			بشکن;
		b = va_arg(ap, نویسه *);
		اگر (!b)
			بشکن;

		اگر (strcmp(arg, a) == 0) {
			va_end(ap);
			بازگشت 1;
		}

		اگر (strcmp(arg, b) == 0) {
			va_end(ap);
			بازگشت 0;
		}
	} وقتی (1);
	va_end(ap);

	errx(STRTOXX_EXIT_CODE, _("unsupported argument: %s"), arg);
}

#ifndef HAVE_MEMPCPY
تهی *mempcpy(تهی *restrict dest, ثابت تهی *restrict src, size_t n)
{
	بازگشت ((نویسه *)memcpy(dest, src, n)) + n;
}
#endif

#ifndef HAVE_STRNLEN
size_t strnlen(ثابت نویسه *s, size_t maxlen)
{
	size_t i;

	برای (i = 0; i < maxlen; i++) {
		اگر (s[i] == '\0')
			بازگشت i;
	}
	بازگشت maxlen;
}
#endif

#ifndef HAVE_STRNCHR
نویسه *strnchr(ثابت نویسه *s, size_t maxlen, صحیح c)
{
	برای (; maxlen-- && *s != '\0'; ++s)
		اگر (*s == (نویسه)c)
			بازگشت (نویسه *)s;
	بازگشت پوچ;
}
#endif

#ifndef HAVE_STRNDUP
نویسه *strndup(ثابت نویسه *s, size_t n)
{
	size_t len = strnlen(s, n);
	نویسه *new = malloc((len + 1) * اندازه(نویسه));
	اگر (!new)
		بازگشت پوچ;
	new[len] = '\0';
	بازگشت (نویسه *) memcpy(new, s, len);
}
#endif

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
صحیح ul_strtos64(ثابت نویسه *str, int64_t *رقم, صحیح base)
{
	نویسه *end = پوچ;

	اگر (str == پوچ || *str == '\0')
		بازگشت -(errno = EINVAL);

	errno = 0;
	*رقم = (int64_t) strtoimax(str, &end, base);

	اگر (errno != 0)
		بازگشت -errno;
	اگر (str == end || (end && *end))
		بازگشت -(errno = EINVAL);
	بازگشت 0;
}

صحیح ul_strtou64(ثابت نویسه *str, uint64_t *رقم, صحیح base)
{
	نویسه *end = پوچ;
	int64_t tmp;

	اگر (str == پوچ || *str == '\0')
		بازگشت -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &end, base);
	اگر (tmp < 0)
		errno = ERANGE;
	وگرنه {
		errno = 0;
		*رقم = strtoumax(str, &end, base);
	}

	اگر (errno != 0)
		بازگشت -errno;
	اگر (str == end || (end && *end))
		بازگشت -(errno = EINVAL);
	بازگشت 0;
}

صحیح ul_strtos32(ثابت نویسه *str, int32_t *رقم, صحیح base)
{
	int64_t tmp;
	صحیح rc;

	rc = ul_strtos64(str, &tmp, base);
	اگر (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	اگر (rc == 0)
		*رقم = (int32_t) tmp;
	بازگشت rc;
}

صحیح ul_strtou32(ثابت نویسه *str, uint32_t *رقم, صحیح base)
{
	uint64_t tmp;
	صحیح rc;

	rc = ul_strtou64(str, &tmp, base);
	اگر (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	اگر (rc == 0)
		*رقم = (uint32_t) tmp;
	بازگشت rc;
}

صحیح ul_strtou16(ثابت نویسه *str, uint16_t *رقم, صحیح base)
{
	uint64_t tmp;
	صحیح rc;

	rc = ul_strtou64(str, &tmp, base);
	اگر (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	اگر (rc == 0)
		*رقم = (uint16_t) tmp;
	بازگشت rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(ثابت نویسه *str, صحیح base, ثابت نویسه *errmesg,
			     int64_t کم, int64_t up)
{
	int64_t رقم = 0;
	صحیح rc;

	rc = ul_strtos64(str, &رقم, base);
	اگر (rc == 0 && ((کم && رقم < کم) || (up && رقم > up)))
		rc = -(errno = ERANGE);

	اگر (rc) {
		اگر (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	بازگشت رقم;
}

uint64_t str2unum_or_err(ثابت نویسه *str, صحیح base, ثابت نویسه *errmesg, uint64_t up)
{
	uint64_t رقم = 0;
	صحیح rc;

	rc = ul_strtou64(str, &رقم, base);
	اگر (rc == 0 && (up && رقم > up))
		rc = -(errno = ERANGE);

	اگر (rc) {
		اگر (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	بازگشت رقم;
}

دوگانه strtod_or_err(ثابت نویسه *str, ثابت نویسه *errmesg)
{
	دوگانه رقم;
	نویسه *end = پوچ;

	errno = 0;
	اگر (str == پوچ || *str == '\0')
		برو err;
	رقم = strtod(str, &end);

	اگر (errno || str == end || (end && *end))
		برو err;

	بازگشت رقم;
err:
	اگر (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

صحیح ul_strtold(ثابت نویسه *str, long دوگانه *رقم)
{
	نویسه *end = پوچ;

	errno = 0;
	اگر (str == پوچ || *str == '\0')
		بازگشت -(errno = EINVAL);
	*رقم = strtold(str, &end);

	اگر (errno != 0)
		بازگشت -errno;
	اگر (str == end || (end && *end))
		بازگشت -(errno = EINVAL);
	بازگشت 0;
}

long دوگانه strtold_or_err(ثابت نویسه *str, ثابت نویسه *errmesg)
{
	long دوگانه رقم = 0;

	اگر (ul_strtold(str, &رقم) == 0)
		بازگشت رقم;
	اگر (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(ثابت نویسه *str, ثابت نویسه *errmesg)
{
	uintmax_t رقم;

	اگر (strtosize(str, &رقم) == 0)
		بازگشت رقم;

	اگر (errno)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}


تهی strtotimeval_or_err(ثابت نویسه *str, ساختار timeval *tv, ثابت نویسه *errmesg)
{
	long دوگانه user_درون_داد;

	user_درون_داد = strtold_or_err(str, errmesg);
	tv->tv_sec = (time_t) user_درون_داد;
	tv->tv_usec = (suseconds_t)((user_درون_داد - tv->tv_sec) * USEC_PER_SEC);
}

تهی strtotimespec_or_err(ثابت نویسه *str, ساختار timespec *ts, ثابت نویسه *errmesg)
{
	long دوگانه user_درون_داد;

	user_درون_داد = strtold_or_err(str, errmesg);
	ts->tv_sec = (time_t) user_درون_داد;
	ts->tv_nsec = (long)((user_درون_داد - ts->tv_sec) * NSEC_PER_SEC);
}

time_t strtotime_or_err(ثابت نویسه *str, ثابت نویسه *errmesg)
{
	int64_t user_درون_داد;

	user_درون_داد = strtos64_or_err(str, errmesg);
	بازگشت (time_t) user_درون_داد;
}

bool hyperlinkwanted(ثابت نویسه *mode)
{
	اگر (mode && strcmp(mode, "never") == 0)
		بازگشت false;

	اگر (mode && strcmp(mode, "always") == 0)
		بازگشت true;

	اگر (!mode || strcmp(mode, "auto") == 0)
		بازگشت isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), mode);
}

bool annotationwanted(ثابت نویسه *mode)
{
	اگر (mode && strcmp(mode, "never") == 0)
		بازگشت false;

	اگر (mode && strcmp(mode, "always") == 0)
		بازگشت true;

	اگر (!mode || strcmp(mode, "auto") == 0)
		بازگشت isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), mode);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
نویسه *xstrmode(mode_t mode, نویسه *str)
{
	unsigned short i = 0;

	اگر (S_ISDIR(mode))
		str[i++] = 'd';
	وگرنه اگر (S_ISLNK(mode))
		str[i++] = 'l';
	وگرنه اگر (S_ISCHR(mode))
		str[i++] = 'c';
	وگرنه اگر (S_ISBLK(mode))
		str[i++] = 'b';
	وگرنه اگر (S_ISSOCK(mode))
		str[i++] = 's';
	وگرنه اگر (S_ISFIFO(mode))
		str[i++] = 'p';
	وگرنه اگر (S_ISREG(mode))
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

	بازگشت str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
ایستا صحیح get_exp(uint64_t n)
{
	صحیح shft;

	برای (shft = 10; shft <= 60; shft += 10) {
		اگر (n < (1ULL << shft))
			بشکن;
	}
	بازگشت shft - 10;
}

نویسه *size_to_human_string(صحیح options, uint64_t bytes)
{
	نویسه buf[32];
	صحیح dec, exp;
	uint64_t frac;
	ثابت نویسه *letters = "BKMGTPE";
	نویسه suffix[اندازه(" KiB")], *psuf = suffix;
	نویسه c;

	اگر (options & SIZE_SUFFIX_SPACE)
		*psuf++ = ' ';


	exp  = get_exp(bytes);
	c    = *(letters + (exp ? exp / 10 : 0));
	dec  = exp ? bytes / (1ULL << exp) : bytes;
	frac = exp ? bytes % (1ULL << exp) : 0;

	*psuf++ = c;

	اگر ((options & SIZE_SUFFIX_3LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/* round */
	اگر (frac) {
		/* get 3 digits after decimal point */
		اگر (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (exp - 10)) ;
		وگرنه
			frac = (frac * 1000) / (1ULL << (exp)) ;

		اگر (options & SIZE_DECIMAL_2DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} وگرنه {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		اگر (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	اگر (frac) {
		ساختار lconv ثابت *l = localeconv();
		نویسه *dp = l ? l->decimal_point : پوچ;
		صحیح len;

		اگر (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, اندازه(buf), "%d%s%02" PRIu64, dec, dp, frac);
		اگر (len > 0 && (size_t) len < اندازه(buf)) {
			/* remove potential extraneous zero */
			اگر (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, اندازه(buf) - len);
		} وگرنه
			*buf = '\0';	/* snprintf error */
	} وگرنه
		snprintf(buf, اندازه(buf), "%d%s", dec, suffix);

	بازگشت strdup(buf);
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
صحیح string_to_idarray(ثابت نویسه *list, صحیح ary[], size_t arysz,
			صحیح (name2id)(ثابت نویسه *, size_t))
{
	ثابت نویسه *begin = پوچ, *p;
	size_t n = 0;

	اگر (!list || !*list || !ary || !arysz || !name2id)
		بازگشت -1;

	برای (p = list; p && *p; p++) {
		ثابت نویسه *end = پوچ;
		صحیح id;

		اگر (n >= arysz)
			بازگشت -2;
		اگر (!begin)
			begin = p;		/* begin of the column name */
		اگر (*p == ',')
			end = p;		/* terminate the name */
		اگر (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		اگر (!begin || !end)
			ادامه;
		اگر (end <= begin)
			بازگشت -1;

		id = name2id(begin, end - begin);
		اگر (id == -1)
			بازگشت -1;
		ary[ n++ ] = id;
		begin = پوچ;
		اگر (end && !*end)
			بشکن;
	}
	بازگشت n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
صحیح string_add_to_idarray(ثابت نویسه *list, صحیح ary[], size_t arysz,
			size_t *ary_pos, صحیح (name2id)(ثابت نویسه *, size_t))
{
	ثابت نویسه *list_add;
	صحیح r;

	اگر (!list || !*list || !ary_pos || *ary_pos > arysz)
		بازگشت -1;

	اگر (list[0] == '+')
		list_add = &list[1];
	وگرنه {
		list_add = list;
		*ary_pos = 0;
	}

	r = string_to_idarray(list_add, &ary[*ary_pos], arysz - *ary_pos, name2id);
	اگر (r > 0)
		*ary_pos += r;
	بازگشت r;
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
صحیح string_to_bitarray(ثابت نویسه *list,
		     نویسه *ary,
		     صحیح (*name2bit)(ثابت نویسه *, size_t),
		     size_t allow_range)
{
	ثابت نویسه *begin = پوچ, *p;

	اگر (!list || !name2bit || !ary)
		بازگشت -EINVAL;

	برای (p = list; p && *p; p++) {
		ثابت نویسه *end = پوچ;
		صحیح bit, set_lower = 0, set_higher = 0;

		اگر (!begin)
			begin = p;		/* begin of the level name */
		اگر (*p == ',')
			end = p;		/* terminate the name */
		اگر (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		اگر (!begin || !end)
			ادامه;
		اگر (end <= begin)
			بازگشت -1;
		اگر (allow_range) {
			اگر (*(end - 1) == '+') {
				end--;
				set_lower = 1;
			} وگرنه اگر (*begin == '+') {
				begin++;
				set_higher = 1;
			}
		}

		bit = name2bit(begin, end - begin);
		اگر (bit < 0)
			بازگشت bit;
		setbit(ary, bit);
		اگر (set_lower)
			وقتی (--bit >= 0)
				setbit(ary, bit);
		وگرنه اگر (set_higher)
			وقتی (++bit < (صحیح) allow_range)
				setbit(ary, bit);
		begin = پوچ;
		اگر (end && !*end)
			بشکن;
	}
	بازگشت 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
صحیح string_to_bitmask(ثابت نویسه *list,
		     unsigned long *mask,
		     long (*name2flag)(ثابت نویسه *, size_t))
{
	ثابت نویسه *begin = پوچ, *p;

	اگر (!list || !name2flag || !mask)
		بازگشت -EINVAL;

	برای (p = list; p && *p; p++) {
		ثابت نویسه *end = پوچ;
		long flag;

		اگر (!begin)
			begin = p;		/* begin of the level name */
		اگر (*p == ',')
			end = p;		/* terminate the name */
		اگر (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		اگر (!begin || !end)
			ادامه;
		اگر (end <= begin)
			بازگشت -1;

		flag = name2flag(begin, end - begin);
		اگر (flag < 0)
			بازگشت flag;	/* error */
		*mask |= flag;
		begin = پوچ;
		اگر (end && !*end)
			بشکن;
	}
	بازگشت 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
صحیح ul_parse_range(ثابت نویسه *str, صحیح *lower, صحیح *upper, صحیح def)
{
	نویسه *end = پوچ;

	اگر (!str)
		بازگشت 0;

	*upper = *lower = def;
	errno = 0;

	اگر (*str == ':') {				/* <:N> */
		str++;
		*upper = strtol(str, &end, 10);
		اگر (errno || !end || *end || end == str)
			بازگشت -1;
	} وگرنه {
		*upper = *lower = strtol(str, &end, 10);
		اگر (errno || !end || end == str)
			بازگشت -1;

		اگر (*end == ':' && !*(end + 1))		/* <M:> */
			*upper = def;
		وگرنه اگر (*end == '-' || *end == ':') {	/* <M:N> <M-N> */
			str = end + 1;
			end = پوچ;
			errno = 0;
			*upper = strtol(str, &end, 10);

			اگر (errno || !end || *end || end == str)
				بازگشت -1;
		}
	}
	بازگشت 0;
}

ثابت نویسه *ul_path_next_segment(ثابت نویسه *str, size_t *sz)
{
	ثابت نویسه *start, *p;

	start = str;
	*sz = 0;
	وقتی (start && *start == '/' && *(start + 1) == '/')
		start++;

	اگر (!start || !*start)
		بازگشت پوچ;

	برای (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	بازگشت start;
}

صحیح streq_paths(ثابت نویسه *a, ثابت نویسه *b)
{
	وقتی (a && b) {
		size_t a_sz, b_sz;
		ثابت نویسه *a_seg = ul_path_next_segment(a, &a_sz);
		ثابت نویسه *b_seg = ul_path_next_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		اگر (a_sz + b_sz == 0)
			بازگشت 1;

		/* ignore trailing slash */
		اگر (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			بازگشت 1;

		اگر (!a_seg || !b_seg)
			بشکن;
		اگر (a_sz != b_sz || strncmp(a_seg, b_seg, a_sz) != 0)
			بشکن;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	بازگشت 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
صحیح ul_path_is_within(ثابت نویسه *path, ثابت نویسه *dir)
{
	اگر (!path || !dir || !*path || !*dir)
		بازگشت 0;

	وقتی (path && dir) {
		size_t p_sz, d_sz;
		ثابت نویسه *p_seg = ul_path_next_segment(path, &p_sz);
		ثابت نویسه *d_seg = ul_path_next_segment(dir, &d_sz);

		/* all @dir segments matched */
		اگر (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			بازگشت 1;

		اگر (!p_seg || !d_seg)
			بشکن;
		اگر (p_sz != d_sz || strncmp(p_seg, d_seg, p_sz) != 0)
			بشکن;

		path = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	بازگشت 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
نویسه *ul_strnconcat(ثابت نویسه *s, ثابت نویسه *suffix, size_t b)
{
	size_t a;
	نویسه *r;

	اگر (!s && !suffix)
		بازگشت strdup("");
	اگر (!s)
		بازگشت strndup(suffix, b);
	اگر (!suffix)
		بازگشت strdup(s);

	أكد(s);
	أكد(suffix);

	a = strlen(s);
	اگر (b > ((size_t) -1) - a)
		بازگشت پوچ;

	r = malloc(a + b + 1);
	اگر (!r)
		بازگشت پوچ;

	memcpy(r, s, a);
	memcpy(r + a, suffix, b);
	r[a+b] = 0;

	بازگشت r;
}

/* concatenate two strings to a new string */
نویسه *ul_strconcat(ثابت نویسه *s, ثابت نویسه *suffix)
{
	بازگشت ul_strnconcat(s, suffix, suffix ? strlen(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
نویسه *ul_strfconcat(ثابت نویسه *s, ثابت نویسه *format, ...)
{
	va_list ap;
	نویسه *val, *res;
	صحیح sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	اگر (sz < 0)
		بازگشت پوچ;

	res = ul_strnconcat(s, val, sz);
	آزاد(val);
	بازگشت res;
}

صحیح ul_strappend(نویسه **a, ثابت نویسه *b)
{
	size_t al, bl;
	نویسه *tmp;

	اگر (!a)
		بازگشت -EINVAL;
	اگر (!b || !*b)
		بازگشت 0;
	اگر (!*a) {
		*a = strdup(b);
		بازگشت !*a ? -ENOMEM : 0;
	}

	al = strlen(*a);
	bl = strlen(b);

	tmp = realloc(*a, al + bl + 1);
	اگر (!tmp)
		بازگشت -ENOMEM;
	*a = tmp;
	memcpy((*a) + al, b, bl + 1);
	بازگشت 0;
}

/* the hybrid version of strfconcat and strappend. */
صحیح strfappend(نویسه **a, ثابت نویسه *format, ...)
{
	va_list ap;
	صحیح res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	بازگشت res;
}

بیرونی صحیح ul_strvfappend(نویسه **a, ثابت نویسه *format, va_list ap)
{
	نویسه *val;
	صحیح sz;
	صحیح res;

	sz = vasprintf(&val, format, ap);
	اگر (sz < 0)
		بازگشت -errno;

	res = ul_strappend(a, val);
	آزاد(val);
	بازگشت res;
}

ایستا size_t strcspn_escaped(ثابت نویسه *s, ثابت نویسه *reject)
{
	صحیح escaped = 0;
	صحیح n;

	برای (n=0; s[n]; n++) {
		اگر (escaped)
			escaped = 0;
		وگرنه اگر (s[n] == '\\')
			escaped = 1;
		وگرنه اگر (strchr(reject, s[n]))
			بشکن;
	}

	/* if s ends in \, return index of previous char */
	بازگشت n - escaped;
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
نویسه *ul_strchr_escaped(ثابت نویسه *s, صحیح c)
{
	نویسه *p;
	صحیح esc = 0;

	برای (p = (نویسه *) s; p && *p; p++) {
		اگر (!esc && *p == '\\') {
			esc = 1;
			ادامه;
		}
		اگر (*p == c && (!esc || c == '\\'))
			بازگشت p;
		esc = 0;
	}

	بازگشت پوچ;
}

/* Split a string into words. */
ثابت نویسه *ul_split(ثابت نویسه **state, size_t *l, ثابت نویسه *separator, صحیح quoted)
{
	ثابت نویسه *current;

	current = *state;

	اگر (!*current) {
		أكد(**state == '\0');
		بازگشت پوچ;
	}

	current += strspn(current, separator);
	اگر (!*current) {
		*state = current;
		بازگشت پوچ;
	}

	اگر (quoted && strchr("\'\"", *current)) {
		نویسه quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		اگر (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			بازگشت پوچ;
		}
		*state = current++ + *l + 2;
	} وگرنه اگر (quoted) {
		*l = strcspn_escaped(current, separator);
		اگر (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			بازگشت پوچ;
		}
		*state = current + *l;
	} وگرنه {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	بازگشت current;
}

/* Rewind file pointer forward to new line.  */
صحیح skip_fline(FILE *fp)
{
	صحیح ch;

	انجامبده {
		اگر ((ch = fgetc(fp)) == EOF)
			بازگشت 1;
		اگر (ch == '\n')
			بازگشت 0;
	} وقتی (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
صحیح ul_stralnumcmp(ثابت نویسه *p1, ثابت نویسه *p2)
{
	ثابت unsigned نویسه *s1 = (ثابت unsigned نویسه *) p1;
	ثابت unsigned نویسه *s2 = (ثابت unsigned نویسه *) p2;
	unsigned نویسه c1, c2;

	انجامبده {
		انجامبده {
			c1 = (unsigned نویسه) *s1++;
		} وقتی (c1 != '\0' && !isalnum((unsigned صحیح) c1));

		انجامبده {
			c2 = (unsigned نویسه) *s2++;
		} وقتی (c2 != '\0' && !isalnum((unsigned صحیح) c2));

		اگر (c1 != '\0')
			c1 = tolower(c1);
		اگر (c2 != '\0')
			c2 = tolower(c2);
		اگر (c1 == '\0')
			بازگشت c1 - c2;
	} وقتی (c1 == c2);

	بازگشت c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
صحیح ul_optstr_next(نویسه **optstr, نویسه **name, size_t *namesz,
		   نویسه **مقدار, size_t *valsz)
{
	صحیح open_quote = 0;
	نویسه *start = پوچ, *stop = پوچ, *p, *sep = پوچ;
	نویسه *optstr0;

	أكد(optstr);
	أكد(*optstr);

	optstr0 = *optstr;

	اگر (name)
		*name = پوچ;
	اگر (namesz)
		*namesz = 0;
	اگر (مقدار)
		*مقدار = پوچ;
	اگر (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	وقتی (optstr0 && *optstr0 == ',')
		optstr0++;

	برای (p = optstr0; p && *p; p++) {
		اگر (!start && *p == '=')
			بازگشت -EINVAL;
		اگر (!start)
			start = p;		/* beginning of the option item */
		اگر (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		اگر (open_quote)
			ادامه;		/* still in quoted block */
		اگر (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		اگر (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		وگرنه اگر (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		اگر (!start || !stop)
			ادامه;
		اگر (stop <= start)
			بازگشت -EINVAL;

		اگر (name)
			*name = start;
		اگر (namesz)
			*namesz = sep ? sep - start : stop - start;
		*optstr = *stop ? stop + 1 : stop;

		اگر (sep) {
			اگر (مقدار)
				*مقدار = sep + 1;
			اگر (valsz)
				*valsz = stop - sep - 1;
		}
		بازگشت 0;
	}

	بازگشت 1;				/* end of optstr */
}

صحیح ul_optstr_is_valid(ثابت نویسه *optstr)
{
	صحیح rc;
	نویسه *p = (نویسه *) optstr;

	وقتی ((rc = ul_optstr_next(&p, پوچ, پوچ, پوچ, پوچ)) == 0);
	بازگشت rc < 0 ? 0 : 1;
}

نویسه *ul_optstr_get_مقدار(ثابت نویسه *optstr, ثابت نویسه *کلید)
{
	size_t sz, namesz = 0, valsz = 0;
	نویسه *name = پوچ, *مقدار = پوچ;
	نویسه *p = (نویسه *) optstr;

	اگر (!optstr || !کلید || !*کلید)
		بازگشت پوچ;

	sz = strlen(کلید);
	وقتی (ul_optstr_next(&p, &name, &namesz, &مقدار, &valsz) == 0) {
		اگر (namesz != sz || !valsz)
			ادامه;
		اگر (strncmp(name, کلید, namesz) == 0)
			بازگشت strndup(مقدار, valsz);
	}
	بازگشت پوچ;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(نویسه *str)
{
	size_t len;
	نویسه q;

	اگر (!str || !*str)
		بازگشت 0;

	len = strlen(str);
	q = *str;

	اگر ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		بازگشت len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	بازگشت len - 2;
}

#ifdef TEST_PROGRAM_STRUTILS

ساختار testS {
	نویسه *name;
	نویسه *مقدار;
};

ایستا صحیح test_strdup_to_member(صحیح argc, نویسه *argv[])
{
	ساختار testS *xx;

	اگر (argc < 3)
		بازگشت EXIT_FAILURE;

	xx = calloc(1, اندازه(*xx));
	اگر (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_to_struct_member(xx, name, argv[1]);
	strdup_to_struct_member(xx, مقدار, argv[2]);

	اگر (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->مقدار, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	printf("1: '%s', 2: '%s'\n", xx->name, xx->مقدار);

	آزاد(xx->name);
	آزاد(xx->مقدار);
	آزاد(xx);
	بازگشت EXIT_SUCCESS;
}

ایستا صحیح test_strutils_sizes(صحیح argc, نویسه *argv[])
{
	uintmax_t size = 0;
	نویسه *hum1, *hum2, *hum3;

	اگر (argc < 2)
		بازگشت EXIT_FAILURE;

	اگر (strtosize(argv[1], &size))
		errx(EXIT_FAILURE, "invalid size '%s' value", argv[1]);

	hum1 = size_to_human_string(SIZE_SUFFIX_1LETTER, size);
	hum2 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE, size);
	hum3 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE |
				    SIZE_DECIMAL_2DIGITS, size);

	printf("%25s : %20ju : %8s : %12s : %13s\n", argv[1], size, hum1, hum2, hum3);
	آزاد(hum1);
	آزاد(hum2);
	آزاد(hum3);

	بازگشت EXIT_SUCCESS;
}

ایستا صحیح test_strutils_cmp_paths(صحیح argc, نویسه *argv[])
{
	صحیح rc = streq_paths(argv[1], argv[2]);

	اگر (argc < 3)
		بازگشت EXIT_FAILURE;

	printf("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	بازگشت EXIT_SUCCESS;
}

ایستا صحیح test_strutils_path_is_within(صحیح argc, نویسه *argv[])
{
	صحیح rc;

	اگر (argc < 3)
		بازگشت EXIT_FAILURE;

	rc = ul_path_is_within(argv[1], argv[2]);

	printf("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	بازگشت EXIT_SUCCESS;
}

ایستا صحیح test_strutils_normalize(صحیح argc, نویسه *argv[])
{
	unsigned نویسه *src, *dst, *org;
	size_t sz, len;

	اگر (argc < 2)
		بازگشت EXIT_FAILURE;

	org = (unsigned نویسه *) strdup(argv[1]);
	src = (unsigned نویسه *) strdup((نویسه *) org);
	len = strlen((نویسه *) src);
	dst = malloc(len + 1);

	اگر (!org || !src || !dst)
		برو done;

	/* two buffers */
	sz = __normalize_whitespace(src, len, dst, len + 1);
	printf("1: '%s' --> '%s' [sz=%zu]\n", src, dst, sz);

	/* one buffer */
	sz = normalize_whitespace(src);
	printf("2: '%s' --> '%s' [sz=%zu]\n", org, src, sz);

done:
	آزاد(src);
	آزاد(dst);
	آزاد(org);

	بازگشت EXIT_SUCCESS;
}

ایستا صحیح test_strutils_cstrcasecmp(صحیح argc, نویسه *argv[])
{
	نویسه *a, *b;

	اگر (argc < 3)
		بازگشت EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	اگر (!a || !b)
		بازگشت EXIT_FAILURE;

	printf("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	printf("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	printf("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, strlen(a)));

	بازگشت EXIT_SUCCESS;
}

صحیح اصلی(صحیح argc, نویسه *argv[])
{
	اگر (argc == 3 && strcmp(argv[1], "--size") == 0) {
		بازگشت test_strutils_sizes(argc - 1, argv + 1);

	} وگرنه اگر (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		printf("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						پوچ));
		بازگشت EXIT_SUCCESS;
	} وگرنه اگر (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		اگر (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		printf("'%s'-->%s\n", argv[2], val ? "true" : "false");
		بازگشت EXIT_SUCCESS;

	} وگرنه اگر (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		بازگشت test_strutils_cmp_paths(argc - 1, argv + 1);

	} وگرنه اگر (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		بازگشت test_strutils_path_is_within(argc - 1, argv + 1);

	} وگرنه اگر (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		بازگشت test_strdup_to_member(argc - 1, argv + 1);

	} وگرنه اگر  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		printf("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		بازگشت EXIT_SUCCESS;

	} وگرنه اگر (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		بازگشت test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} وگرنه اگر (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		بازگشت test_strutils_normalize(argc - 1, argv + 1);

	} وگرنه اگر (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		printf("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		بازگشت EXIT_SUCCESS;
	} وگرنه اگر (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		printf("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		بازگشت EXIT_SUCCESS;
	} وگرنه اگر (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		printf("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		بازگشت EXIT_SUCCESS;
	} وگرنه اگر (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		printf("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		بازگشت EXIT_SUCCESS;
	} وگرنه اگر (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		printf("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		بازگشت EXIT_SUCCESS;
	} وگرنه اگر (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		printf("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		بازگشت EXIT_SUCCESS;

	} وگرنه اگر (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		printf("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		بازگشت EXIT_SUCCESS;

	} وگرنه اگر (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		نویسه *buf = "abc\0Y\0\0xyz\0X";
		نویسه *end = buf + 12;
		نویسه *p = buf;

		انجامبده {
			printf("str: '%s'\n", p);
		} وقتی ((p = ul_next_string(p, end)));

		بازگشت EXIT_SUCCESS;

	} وگرنه اگر (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		نویسه *name = پوچ, *val = پوچ;
		نویسه *p = argv[2];
		صحیح rc;

		اگر (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		وقتی ((rc = ul_optstr_next(&p, &name, &namesz, &val, &valsz)) == 0) {
			printf("'%.*s' : '%.*s'\n", (صحیح) namesz, name,
						   (صحیح) valsz, val);
		}
		اگر (rc == 1)
			بازگشت EXIT_SUCCESS;
	} وگرنه اگر (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		نویسه *unquoted = strdup(argv[2]);

		اگر (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		printf("%s-->%s\n", argv[2], unquoted);
		آزاد(unquoted);
		بازگشت EXIT_SUCCESS;

	} وگرنه {
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

	بازگشت EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_STRUTILS */
