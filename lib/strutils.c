/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Karel Zak <kzak@redhat.com> [2010]
 *          Davidlohr Bueso <dave@gnu.org> [2010]
 */
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <inttypes.h>
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <strings.h>
#அந்தர்பாவ <assert.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "cctype.h"
#அந்தர்பாவ "nls.h"
#அந்தர்பாவ "strutils.h"
#அந்தர்பாவ "timeutils.h"
#அந்தர்பாவ "bitops.h"
#அந்தர்பாவ "pathnames.h"

நிலையான முழு STRTOXX_EXIT_CODE = EXIT_FAILURE;

வெறுமை strutils_set_exitcode(முழு ex)
{
	STRTOXX_EXIT_CODE = ex;
}

நிலையான முழு do_scale_by_power (uintmax_t *x, முழு base, முழு power)
{
	போது (power--) {
		எனில் (UINTMAX_MAX / base < *x)
			திரும்பு -ERANGE;
		*x *= base;
	}
	திரும்பு 0;
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
முழு ul_parse_size(மாறிலி எழுத்து *str, uintmax_t *res, முழு *power)
{
	மாறிலி எழுத்து *p;
	எழுத்து *end;
	uintmax_t x, frac = 0;
	முழு base = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	நிலையான மாறிலி எழுத்து *suf  = "KMGTPEZY";
	நிலையான மாறிலி எழுத்து *suf2 = "kmgtpezy";
	மாறிலி எழுத்து *sp;

	*res = 0;

	எனில் (!str || !*str) {
		rc = -EINVAL;
		செல் err;
	}

	/* Only positive numbers are acceptable
	 *
	 * Note that this check is not perfect, it would be better to
	 * use lconv->negative_sign. But coreutils use the same solution,
	 * so it's probably good enough...
	 */
	p = str;
	போது (isspace((unsigned எழுத்து) *p))
		p++;
	எனில் (*p == '-') {
		rc = -EINVAL;
		செல் err;
	}

	errno = 0, end = வெற்று;
	x = strtoumax(str, &end, 0);

	எனில் (end == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		செல் err;
	}
	எனில் (!end || !*end)
		செல் done;			/* without suffix */
	p = end;

	/*
	 * Check size suffixes
	 */
check_suffix:
	எனில் (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		base = 1024;			/* XiB, 2^N */
	இல்லையெனில் எனில் ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		base = 1000;			/* XB, 10^N */
	இல்லையெனில் எனில் (*(p + 1)) {
		கட்டமைப்பு lconv மாறிலி *l = localeconv();
		மாறிலி எழுத்து *dp = l ? l->decimal_point : வெற்று;
		size_t dpsz = dp ? strlen(dp) : 0;

		எனில் (frac == 0 && *p && dp && strncmp(dp, p, dpsz) == 0) {
			மாறிலி எழுத்து *fstr = p + dpsz;

			க்கு (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			எனில் (isdigit(*fstr)) {
				errno = 0, end = வெற்று;
				frac = strtoumax(fstr, &end, 0);
				எனில் (end == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					செல் err;
				}
			} இல்லையெனில்
				end = (எழுத்து *) p;

			எனில் (!end || !*end) {
				rc = -EINVAL;
				செல் err;	/* without suffix, but with fractions */
			}
			p = end;
			செல் check_suffix;
		}
		rc = -EINVAL;
		செல் err;			/* unexpected suffix */
	}

	sp = strchr(suf, *p);
	எனில் (sp)
		pwr = (sp - suf) + 1;
	இல்லையெனில் {
		sp = strchr(suf2, *p);
		எனில் (sp)
			pwr = (sp - suf2) + 1;
		இல்லையெனில் {
			rc = -EINVAL;
			செல் err;
		}
	}

	எனில் (power)
		*power = pwr;

	rc = do_scale_by_power(&x, base, pwr);
	எனில் (rc)
		செல் err;

	எனில் (frac && pwr) {
		முழு i;
		uintmax_t frac_base = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_base, base, pwr);
		எனில் (rc)
			செல் err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		செய் {
			frac_val = ((frac % 10) * frac_base + frac_val) / 10;
			frac /= 10;
		} போது (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		க்கு (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		எனில் (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			செல் err;
		}
		x += frac_val;
	}
done:
	*res = x;
err:
	எனில் (rc < 0)
		errno = -rc;
	திரும்பு rc;
}

முழு strtosize(மாறிலி எழுத்து *str, uintmax_t *res)
{
	திரும்பு ul_parse_size(str, res, வெற்று);
}

முழு isdigit_strend(மாறிலி எழுத்து *str, மாறிலி எழுத்து **end)
{
	மாறிலி எழுத்து *p;

	க்கு (p = str; p && *p && isdigit((unsigned எழுத்து) *p); p++);

	எனில் (end)
		*end = p;
	திரும்பு p && p > str && !*p;
}

முழு isxdigit_strend(மாறிலி எழுத்து *str, மாறிலி எழுத்து **end)
{
	மாறிலி எழுத்து *p;

	க்கு (p = str; p && *p && isxdigit((unsigned எழுத்து) *p); p++);

	எனில் (end)
		*end = p;

	திரும்பு p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
முழு ul_strtobool(மாறிலி எழுத்து *str, bool *result)
{
	நிலையான மாறிலி எழுத்து *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	நிலையான மாறிலி எழுத்து *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	எனில் (!str || !result)
		திரும்பு -(errno = EINVAL);

	errno = 0;

	க்கு (i = 0; i < ARRAY_SIZE(bool_true); i++) {
		எனில் (c_strcasecmp(str, bool_true[i]) == 0) {
			*result = true;
			திரும்பு 0;
		}
	}

	க்கு (i = 0; i < ARRAY_SIZE(bool_false); i++) {
		எனில் (c_strcasecmp(str, bool_false[i]) == 0) {
			*result = false;
			திரும்பு 0;
		}
	}

	திரும்பு -(errno = EINVAL);
}

முழு ul_parse_switch(மாறிலி எழுத்து *arg, ...)
{
	மாறிலி எழுத்து *a, *b;
	va_list ap;

	va_start(ap, arg);
	செய் {
		a = va_arg(ap, எழுத்து *);
		எனில் (!a)
			நிறுத்து;
		b = va_arg(ap, எழுத்து *);
		எனில் (!b)
			நிறுத்து;

		எனில் (strcmp(arg, a) == 0) {
			va_end(ap);
			திரும்பு 1;
		}

		எனில் (strcmp(arg, b) == 0) {
			va_end(ap);
			திரும்பு 0;
		}
	} போது (1);
	va_end(ap);

	errx(STRTOXX_EXIT_CODE, _("unsupported argument: %s"), arg);
}

#ifndef HAVE_MEMPCPY
வெறுமை *mempcpy(வெறுமை *restrict dest, மாறிலி வெறுமை *restrict src, size_t n)
{
	திரும்பு ((எழுத்து *)memcpy(dest, src, n)) + n;
}
#endif

#ifndef HAVE_STRNLEN
size_t strnlen(மாறிலி எழுத்து *s, size_t maxlen)
{
	size_t i;

	க்கு (i = 0; i < maxlen; i++) {
		எனில் (s[i] == '\0')
			திரும்பு i;
	}
	திரும்பு maxlen;
}
#endif

#ifndef HAVE_STRNCHR
எழுத்து *strnchr(மாறிலி எழுத்து *s, size_t maxlen, முழு c)
{
	க்கு (; maxlen-- && *s != '\0'; ++s)
		எனில் (*s == (எழுத்து)c)
			திரும்பு (எழுத்து *)s;
	திரும்பு வெற்று;
}
#endif

#ifndef HAVE_STRNDUP
எழுத்து *strndup(மாறிலி எழுத்து *s, size_t n)
{
	size_t len = strnlen(s, n);
	எழுத்து *new = malloc((len + 1) * அளவு(எழுத்து));
	எனில் (!new)
		திரும்பு வெற்று;
	new[len] = '\0';
	திரும்பு (எழுத்து *) memcpy(new, s, len);
}
#endif

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
முழு ul_strtos64(மாறிலி எழுத்து *str, int64_t *இலக்கம், முழு base)
{
	எழுத்து *end = வெற்று;

	எனில் (str == வெற்று || *str == '\0')
		திரும்பு -(errno = EINVAL);

	errno = 0;
	*இலக்கம் = (int64_t) strtoimax(str, &end, base);

	எனில் (errno != 0)
		திரும்பு -errno;
	எனில் (str == end || (end && *end))
		திரும்பு -(errno = EINVAL);
	திரும்பு 0;
}

முழு ul_strtou64(மாறிலி எழுத்து *str, uint64_t *இலக்கம், முழு base)
{
	எழுத்து *end = வெற்று;
	int64_t tmp;

	எனில் (str == வெற்று || *str == '\0')
		திரும்பு -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &end, base);
	எனில் (tmp < 0)
		errno = ERANGE;
	இல்லையெனில் {
		errno = 0;
		*இலக்கம் = strtoumax(str, &end, base);
	}

	எனில் (errno != 0)
		திரும்பு -errno;
	எனில் (str == end || (end && *end))
		திரும்பு -(errno = EINVAL);
	திரும்பு 0;
}

முழு ul_strtos32(மாறிலி எழுத்து *str, int32_t *இலக்கம், முழு base)
{
	int64_t tmp;
	முழு rc;

	rc = ul_strtos64(str, &tmp, base);
	எனில் (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	எனில் (rc == 0)
		*இலக்கம் = (int32_t) tmp;
	திரும்பு rc;
}

முழு ul_strtou32(மாறிலி எழுத்து *str, uint32_t *இலக்கம், முழு base)
{
	uint64_t tmp;
	முழு rc;

	rc = ul_strtou64(str, &tmp, base);
	எனில் (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	எனில் (rc == 0)
		*இலக்கம் = (uint32_t) tmp;
	திரும்பு rc;
}

முழு ul_strtou16(மாறிலி எழுத்து *str, uint16_t *இலக்கம், முழு base)
{
	uint64_t tmp;
	முழு rc;

	rc = ul_strtou64(str, &tmp, base);
	எனில் (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	எனில் (rc == 0)
		*இலக்கம் = (uint16_t) tmp;
	திரும்பு rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(மாறிலி எழுத்து *str, முழு base, மாறிலி எழுத்து *errmesg,
			     int64_t கீழ், int64_t up)
{
	int64_t இலக்கம் = 0;
	முழு rc;

	rc = ul_strtos64(str, &இலக்கம், base);
	எனில் (rc == 0 && ((கீழ் && இலக்கம் < கீழ்) || (up && இலக்கம் > up)))
		rc = -(errno = ERANGE);

	எனில் (rc) {
		எனில் (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	திரும்பு இலக்கம்;
}

uint64_t str2unum_or_err(மாறிலி எழுத்து *str, முழு base, மாறிலி எழுத்து *errmesg, uint64_t up)
{
	uint64_t இலக்கம் = 0;
	முழு rc;

	rc = ul_strtou64(str, &இலக்கம், base);
	எனில் (rc == 0 && (up && இலக்கம் > up))
		rc = -(errno = ERANGE);

	எனில் (rc) {
		எனில் (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	திரும்பு இலக்கம்;
}

இரட்டை strtod_or_err(மாறிலி எழுத்து *str, மாறிலி எழுத்து *errmesg)
{
	இரட்டை இலக்கம்;
	எழுத்து *end = வெற்று;

	errno = 0;
	எனில் (str == வெற்று || *str == '\0')
		செல் err;
	இலக்கம் = strtod(str, &end);

	எனில் (errno || str == end || (end && *end))
		செல் err;

	திரும்பு இலக்கம்;
err:
	எனில் (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

முழு ul_strtold(மாறிலி எழுத்து *str, long இரட்டை *இலக்கம்)
{
	எழுத்து *end = வெற்று;

	errno = 0;
	எனில் (str == வெற்று || *str == '\0')
		திரும்பு -(errno = EINVAL);
	*இலக்கம் = strtold(str, &end);

	எனில் (errno != 0)
		திரும்பு -errno;
	எனில் (str == end || (end && *end))
		திரும்பு -(errno = EINVAL);
	திரும்பு 0;
}

long இரட்டை strtold_or_err(மாறிலி எழுத்து *str, மாறிலி எழுத்து *errmesg)
{
	long இரட்டை இலக்கம் = 0;

	எனில் (ul_strtold(str, &இலக்கம்) == 0)
		திரும்பு இலக்கம்;
	எனில் (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(மாறிலி எழுத்து *str, மாறிலி எழுத்து *errmesg)
{
	uintmax_t இலக்கம்;

	எனில் (strtosize(str, &இலக்கம்) == 0)
		திரும்பு இலக்கம்;

	எனில் (errno)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}


வெறுமை strtotimeval_or_err(மாறிலி எழுத்து *str, கட்டமைப்பு timeval *tv, மாறிலி எழுத்து *errmesg)
{
	long இரட்டை user_உள்ளீடு;

	user_உள்ளீடு = strtold_or_err(str, errmesg);
	tv->tv_sec = (time_t) user_உள்ளீடு;
	tv->tv_usec = (suseconds_t)((user_உள்ளீடு - tv->tv_sec) * USEC_PER_SEC);
}

வெறுமை strtotimespec_or_err(மாறிலி எழுத்து *str, கட்டமைப்பு timespec *ts, மாறிலி எழுத்து *errmesg)
{
	long இரட்டை user_உள்ளீடு;

	user_உள்ளீடு = strtold_or_err(str, errmesg);
	ts->tv_sec = (time_t) user_உள்ளீடு;
	ts->tv_nsec = (long)((user_உள்ளீடு - ts->tv_sec) * NSEC_PER_SEC);
}

time_t strtotime_or_err(மாறிலி எழுத்து *str, மாறிலி எழுத்து *errmesg)
{
	int64_t user_உள்ளீடு;

	user_உள்ளீடு = strtos64_or_err(str, errmesg);
	திரும்பு (time_t) user_உள்ளீடு;
}

bool hyperlinkwanted(மாறிலி எழுத்து *mode)
{
	எனில் (mode && strcmp(mode, "never") == 0)
		திரும்பு false;

	எனில் (mode && strcmp(mode, "always") == 0)
		திரும்பு true;

	எனில் (!mode || strcmp(mode, "auto") == 0)
		திரும்பு isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), mode);
}

bool annotationwanted(மாறிலி எழுத்து *mode)
{
	எனில் (mode && strcmp(mode, "never") == 0)
		திரும்பு false;

	எனில் (mode && strcmp(mode, "always") == 0)
		திரும்பு true;

	எனில் (!mode || strcmp(mode, "auto") == 0)
		திரும்பு isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), mode);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
எழுத்து *xstrmode(mode_t mode, எழுத்து *str)
{
	unsigned short i = 0;

	எனில் (S_ISDIR(mode))
		str[i++] = 'd';
	இல்லையெனில் எனில் (S_ISLNK(mode))
		str[i++] = 'l';
	இல்லையெனில் எனில் (S_ISCHR(mode))
		str[i++] = 'c';
	இல்லையெனில் எனில் (S_ISBLK(mode))
		str[i++] = 'b';
	இல்லையெனில் எனில் (S_ISSOCK(mode))
		str[i++] = 's';
	இல்லையெனில் எனில் (S_ISFIFO(mode))
		str[i++] = 'p';
	இல்லையெனில் எனில் (S_ISREG(mode))
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

	திரும்பு str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
நிலையான முழு get_exp(uint64_t n)
{
	முழு shft;

	க்கு (shft = 10; shft <= 60; shft += 10) {
		எனில் (n < (1ULL << shft))
			நிறுத்து;
	}
	திரும்பு shft - 10;
}

எழுத்து *size_to_human_string(முழு options, uint64_t bytes)
{
	எழுத்து buf[32];
	முழு dec, exp;
	uint64_t frac;
	மாறிலி எழுத்து *letters = "BKMGTPE";
	எழுத்து suffix[அளவு(" KiB")], *psuf = suffix;
	எழுத்து c;

	எனில் (options & SIZE_SUFFIX_SPACE)
		*psuf++ = ' ';


	exp  = get_exp(bytes);
	c    = *(letters + (exp ? exp / 10 : 0));
	dec  = exp ? bytes / (1ULL << exp) : bytes;
	frac = exp ? bytes % (1ULL << exp) : 0;

	*psuf++ = c;

	எனில் ((options & SIZE_SUFFIX_3LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/* round */
	எனில் (frac) {
		/* get 3 digits after decimal point */
		எனில் (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (exp - 10)) ;
		இல்லையெனில்
			frac = (frac * 1000) / (1ULL << (exp)) ;

		எனில் (options & SIZE_DECIMAL_2DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} இல்லையெனில் {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		எனில் (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	எனில் (frac) {
		கட்டமைப்பு lconv மாறிலி *l = localeconv();
		எழுத்து *dp = l ? l->decimal_point : வெற்று;
		முழு len;

		எனில் (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, அளவு(buf), "%d%s%02" PRIu64, dec, dp, frac);
		எனில் (len > 0 && (size_t) len < அளவு(buf)) {
			/* remove potential extraneous zero */
			எனில் (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, அளவு(buf) - len);
		} இல்லையெனில்
			*buf = '\0';	/* snprintf error */
	} இல்லையெனில்
		snprintf(buf, அளவு(buf), "%d%s", dec, suffix);

	திரும்பு strdup(buf);
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
முழு string_to_idarray(மாறிலி எழுத்து *list, முழு ary[], size_t arysz,
			முழு (name2id)(மாறிலி எழுத்து *, size_t))
{
	மாறிலி எழுத்து *begin = வெற்று, *p;
	size_t n = 0;

	எனில் (!list || !*list || !ary || !arysz || !name2id)
		திரும்பு -1;

	க்கு (p = list; p && *p; p++) {
		மாறிலி எழுத்து *end = வெற்று;
		முழு id;

		எனில் (n >= arysz)
			திரும்பு -2;
		எனில் (!begin)
			begin = p;		/* begin of the column name */
		எனில் (*p == ',')
			end = p;		/* terminate the name */
		எனில் (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		எனில் (!begin || !end)
			தொடர்;
		எனில் (end <= begin)
			திரும்பு -1;

		id = name2id(begin, end - begin);
		எனில் (id == -1)
			திரும்பு -1;
		ary[ n++ ] = id;
		begin = வெற்று;
		எனில் (end && !*end)
			நிறுத்து;
	}
	திரும்பு n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
முழு string_add_to_idarray(மாறிலி எழுத்து *list, முழு ary[], size_t arysz,
			size_t *ary_pos, முழு (name2id)(மாறிலி எழுத்து *, size_t))
{
	மாறிலி எழுத்து *list_add;
	முழு r;

	எனில் (!list || !*list || !ary_pos || *ary_pos > arysz)
		திரும்பு -1;

	எனில் (list[0] == '+')
		list_add = &list[1];
	இல்லையெனில் {
		list_add = list;
		*ary_pos = 0;
	}

	r = string_to_idarray(list_add, &ary[*ary_pos], arysz - *ary_pos, name2id);
	எனில் (r > 0)
		*ary_pos += r;
	திரும்பு r;
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
முழு string_to_bitarray(மாறிலி எழுத்து *list,
		     எழுத்து *ary,
		     முழு (*name2bit)(மாறிலி எழுத்து *, size_t),
		     size_t allow_range)
{
	மாறிலி எழுத்து *begin = வெற்று, *p;

	எனில் (!list || !name2bit || !ary)
		திரும்பு -EINVAL;

	க்கு (p = list; p && *p; p++) {
		மாறிலி எழுத்து *end = வெற்று;
		முழு bit, set_lower = 0, set_higher = 0;

		எனில் (!begin)
			begin = p;		/* begin of the level name */
		எனில் (*p == ',')
			end = p;		/* terminate the name */
		எனில் (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		எனில் (!begin || !end)
			தொடர்;
		எனில் (end <= begin)
			திரும்பு -1;
		எனில் (allow_range) {
			எனில் (*(end - 1) == '+') {
				end--;
				set_lower = 1;
			} இல்லையெனில் எனில் (*begin == '+') {
				begin++;
				set_higher = 1;
			}
		}

		bit = name2bit(begin, end - begin);
		எனில் (bit < 0)
			திரும்பு bit;
		setbit(ary, bit);
		எனில் (set_lower)
			போது (--bit >= 0)
				setbit(ary, bit);
		இல்லையெனில் எனில் (set_higher)
			போது (++bit < (முழு) allow_range)
				setbit(ary, bit);
		begin = வெற்று;
		எனில் (end && !*end)
			நிறுத்து;
	}
	திரும்பு 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
முழு string_to_bitmask(மாறிலி எழுத்து *list,
		     unsigned long *mask,
		     long (*name2flag)(மாறிலி எழுத்து *, size_t))
{
	மாறிலி எழுத்து *begin = வெற்று, *p;

	எனில் (!list || !name2flag || !mask)
		திரும்பு -EINVAL;

	க்கு (p = list; p && *p; p++) {
		மாறிலி எழுத்து *end = வெற்று;
		long flag;

		எனில் (!begin)
			begin = p;		/* begin of the level name */
		எனில் (*p == ',')
			end = p;		/* terminate the name */
		எனில் (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		எனில் (!begin || !end)
			தொடர்;
		எனில் (end <= begin)
			திரும்பு -1;

		flag = name2flag(begin, end - begin);
		எனில் (flag < 0)
			திரும்பு flag;	/* error */
		*mask |= flag;
		begin = வெற்று;
		எனில் (end && !*end)
			நிறுத்து;
	}
	திரும்பு 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
முழு ul_parse_range(மாறிலி எழுத்து *str, முழு *lower, முழு *upper, முழு def)
{
	எழுத்து *end = வெற்று;

	எனில் (!str)
		திரும்பு 0;

	*upper = *lower = def;
	errno = 0;

	எனில் (*str == ':') {				/* <:N> */
		str++;
		*upper = strtol(str, &end, 10);
		எனில் (errno || !end || *end || end == str)
			திரும்பு -1;
	} இல்லையெனில் {
		*upper = *lower = strtol(str, &end, 10);
		எனில் (errno || !end || end == str)
			திரும்பு -1;

		எனில் (*end == ':' && !*(end + 1))		/* <M:> */
			*upper = def;
		இல்லையெனில் எனில் (*end == '-' || *end == ':') {	/* <M:N> <M-N> */
			str = end + 1;
			end = வெற்று;
			errno = 0;
			*upper = strtol(str, &end, 10);

			எனில் (errno || !end || *end || end == str)
				திரும்பு -1;
		}
	}
	திரும்பு 0;
}

மாறிலி எழுத்து *ul_path_next_segment(மாறிலி எழுத்து *str, size_t *sz)
{
	மாறிலி எழுத்து *start, *p;

	start = str;
	*sz = 0;
	போது (start && *start == '/' && *(start + 1) == '/')
		start++;

	எனில் (!start || !*start)
		திரும்பு வெற்று;

	க்கு (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	திரும்பு start;
}

முழு streq_paths(மாறிலி எழுத்து *a, மாறிலி எழுத்து *b)
{
	போது (a && b) {
		size_t a_sz, b_sz;
		மாறிலி எழுத்து *a_seg = ul_path_next_segment(a, &a_sz);
		மாறிலி எழுத்து *b_seg = ul_path_next_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		எனில் (a_sz + b_sz == 0)
			திரும்பு 1;

		/* ignore trailing slash */
		எனில் (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			திரும்பு 1;

		எனில் (!a_seg || !b_seg)
			நிறுத்து;
		எனில் (a_sz != b_sz || strncmp(a_seg, b_seg, a_sz) != 0)
			நிறுத்து;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	திரும்பு 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
முழு ul_path_is_within(மாறிலி எழுத்து *path, மாறிலி எழுத்து *dir)
{
	எனில் (!path || !dir || !*path || !*dir)
		திரும்பு 0;

	போது (path && dir) {
		size_t p_sz, d_sz;
		மாறிலி எழுத்து *p_seg = ul_path_next_segment(path, &p_sz);
		மாறிலி எழுத்து *d_seg = ul_path_next_segment(dir, &d_sz);

		/* all @dir segments matched */
		எனில் (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			திரும்பு 1;

		எனில் (!p_seg || !d_seg)
			நிறுத்து;
		எனில் (p_sz != d_sz || strncmp(p_seg, d_seg, p_sz) != 0)
			நிறுத்து;

		path = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	திரும்பு 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
எழுத்து *ul_strnconcat(மாறிலி எழுத்து *s, மாறிலி எழுத்து *suffix, size_t b)
{
	size_t a;
	எழுத்து *r;

	எனில் (!s && !suffix)
		திரும்பு strdup("");
	எனில் (!s)
		திரும்பு strndup(suffix, b);
	எனில் (!suffix)
		திரும்பு strdup(s);

	ப்ரதிஜ்ஞா(s);
	ப்ரதிஜ்ஞா(suffix);

	a = strlen(s);
	எனில் (b > ((size_t) -1) - a)
		திரும்பு வெற்று;

	r = malloc(a + b + 1);
	எனில் (!r)
		திரும்பு வெற்று;

	memcpy(r, s, a);
	memcpy(r + a, suffix, b);
	r[a+b] = 0;

	திரும்பு r;
}

/* concatenate two strings to a new string */
எழுத்து *ul_strconcat(மாறிலி எழுத்து *s, மாறிலி எழுத்து *suffix)
{
	திரும்பு ul_strnconcat(s, suffix, suffix ? strlen(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
எழுத்து *ul_strfconcat(மாறிலி எழுத்து *s, மாறிலி எழுத்து *format, ...)
{
	va_list ap;
	எழுத்து *val, *res;
	முழு sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	எனில் (sz < 0)
		திரும்பு வெற்று;

	res = ul_strnconcat(s, val, sz);
	விடுவி(val);
	திரும்பு res;
}

முழு ul_strappend(எழுத்து **a, மாறிலி எழுத்து *b)
{
	size_t al, bl;
	எழுத்து *tmp;

	எனில் (!a)
		திரும்பு -EINVAL;
	எனில் (!b || !*b)
		திரும்பு 0;
	எனில் (!*a) {
		*a = strdup(b);
		திரும்பு !*a ? -ENOMEM : 0;
	}

	al = strlen(*a);
	bl = strlen(b);

	tmp = realloc(*a, al + bl + 1);
	எனில் (!tmp)
		திரும்பு -ENOMEM;
	*a = tmp;
	memcpy((*a) + al, b, bl + 1);
	திரும்பு 0;
}

/* the hybrid version of strfconcat and strappend. */
முழு strfappend(எழுத்து **a, மாறிலி எழுத்து *format, ...)
{
	va_list ap;
	முழு res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	திரும்பு res;
}

வெளி முழு ul_strvfappend(எழுத்து **a, மாறிலி எழுத்து *format, va_list ap)
{
	எழுத்து *val;
	முழு sz;
	முழு res;

	sz = vasprintf(&val, format, ap);
	எனில் (sz < 0)
		திரும்பு -errno;

	res = ul_strappend(a, val);
	விடுவி(val);
	திரும்பு res;
}

நிலையான size_t strcspn_escaped(மாறிலி எழுத்து *s, மாறிலி எழுத்து *reject)
{
	முழு escaped = 0;
	முழு n;

	க்கு (n=0; s[n]; n++) {
		எனில் (escaped)
			escaped = 0;
		இல்லையெனில் எனில் (s[n] == '\\')
			escaped = 1;
		இல்லையெனில் எனில் (strchr(reject, s[n]))
			நிறுத்து;
	}

	/* if s ends in \, return index of previous char */
	திரும்பு n - escaped;
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
எழுத்து *ul_strchr_escaped(மாறிலி எழுத்து *s, முழு c)
{
	எழுத்து *p;
	முழு esc = 0;

	க்கு (p = (எழுத்து *) s; p && *p; p++) {
		எனில் (!esc && *p == '\\') {
			esc = 1;
			தொடர்;
		}
		எனில் (*p == c && (!esc || c == '\\'))
			திரும்பு p;
		esc = 0;
	}

	திரும்பு வெற்று;
}

/* Split a string into words. */
மாறிலி எழுத்து *ul_split(மாறிலி எழுத்து **state, size_t *l, மாறிலி எழுத்து *separator, முழு quoted)
{
	மாறிலி எழுத்து *current;

	current = *state;

	எனில் (!*current) {
		ப்ரதிஜ்ஞா(**state == '\0');
		திரும்பு வெற்று;
	}

	current += strspn(current, separator);
	எனில் (!*current) {
		*state = current;
		திரும்பு வெற்று;
	}

	எனில் (quoted && strchr("\'\"", *current)) {
		எழுத்து quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		எனில் (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			திரும்பு வெற்று;
		}
		*state = current++ + *l + 2;
	} இல்லையெனில் எனில் (quoted) {
		*l = strcspn_escaped(current, separator);
		எனில் (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			திரும்பு வெற்று;
		}
		*state = current + *l;
	} இல்லையெனில் {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	திரும்பு current;
}

/* Rewind file pointer forward to new line.  */
முழு skip_fline(FILE *fp)
{
	முழு ch;

	செய் {
		எனில் ((ch = fgetc(fp)) == EOF)
			திரும்பு 1;
		எனில் (ch == '\n')
			திரும்பு 0;
	} போது (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
முழு ul_stralnumcmp(மாறிலி எழுத்து *p1, மாறிலி எழுத்து *p2)
{
	மாறிலி unsigned எழுத்து *s1 = (மாறிலி unsigned எழுத்து *) p1;
	மாறிலி unsigned எழுத்து *s2 = (மாறிலி unsigned எழுத்து *) p2;
	unsigned எழுத்து c1, c2;

	செய் {
		செய் {
			c1 = (unsigned எழுத்து) *s1++;
		} போது (c1 != '\0' && !isalnum((unsigned முழு) c1));

		செய் {
			c2 = (unsigned எழுத்து) *s2++;
		} போது (c2 != '\0' && !isalnum((unsigned முழு) c2));

		எனில் (c1 != '\0')
			c1 = tolower(c1);
		எனில் (c2 != '\0')
			c2 = tolower(c2);
		எனில் (c1 == '\0')
			திரும்பு c1 - c2;
	} போது (c1 == c2);

	திரும்பு c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
முழு ul_optstr_next(எழுத்து **optstr, எழுத்து **name, size_t *namesz,
		   எழுத்து **மதிப்பு, size_t *valsz)
{
	முழு open_quote = 0;
	எழுத்து *start = வெற்று, *stop = வெற்று, *p, *sep = வெற்று;
	எழுத்து *optstr0;

	ப்ரதிஜ்ஞா(optstr);
	ப்ரதிஜ்ஞா(*optstr);

	optstr0 = *optstr;

	எனில் (name)
		*name = வெற்று;
	எனில் (namesz)
		*namesz = 0;
	எனில் (மதிப்பு)
		*மதிப்பு = வெற்று;
	எனில் (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	போது (optstr0 && *optstr0 == ',')
		optstr0++;

	க்கு (p = optstr0; p && *p; p++) {
		எனில் (!start && *p == '=')
			திரும்பு -EINVAL;
		எனில் (!start)
			start = p;		/* beginning of the option item */
		எனில் (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		எனில் (open_quote)
			தொடர்;		/* still in quoted block */
		எனில் (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		எனில் (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		இல்லையெனில் எனில் (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		எனில் (!start || !stop)
			தொடர்;
		எனில் (stop <= start)
			திரும்பு -EINVAL;

		எனில் (name)
			*name = start;
		எனில் (namesz)
			*namesz = sep ? sep - start : stop - start;
		*optstr = *stop ? stop + 1 : stop;

		எனில் (sep) {
			எனில் (மதிப்பு)
				*மதிப்பு = sep + 1;
			எனில் (valsz)
				*valsz = stop - sep - 1;
		}
		திரும்பு 0;
	}

	திரும்பு 1;				/* end of optstr */
}

முழு ul_optstr_is_valid(மாறிலி எழுத்து *optstr)
{
	முழு rc;
	எழுத்து *p = (எழுத்து *) optstr;

	போது ((rc = ul_optstr_next(&p, வெற்று, வெற்று, வெற்று, வெற்று)) == 0);
	திரும்பு rc < 0 ? 0 : 1;
}

எழுத்து *ul_optstr_get_மதிப்பு(மாறிலி எழுத்து *optstr, மாறிலி எழுத்து *சாவி)
{
	size_t sz, namesz = 0, valsz = 0;
	எழுத்து *name = வெற்று, *மதிப்பு = வெற்று;
	எழுத்து *p = (எழுத்து *) optstr;

	எனில் (!optstr || !சாவி || !*சாவி)
		திரும்பு வெற்று;

	sz = strlen(சாவி);
	போது (ul_optstr_next(&p, &name, &namesz, &மதிப்பு, &valsz) == 0) {
		எனில் (namesz != sz || !valsz)
			தொடர்;
		எனில் (strncmp(name, சாவி, namesz) == 0)
			திரும்பு strndup(மதிப்பு, valsz);
	}
	திரும்பு வெற்று;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(எழுத்து *str)
{
	size_t len;
	எழுத்து q;

	எனில் (!str || !*str)
		திரும்பு 0;

	len = strlen(str);
	q = *str;

	எனில் ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		திரும்பு len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	திரும்பு len - 2;
}

#ifdef TEST_PROGRAM_STRUTILS

கட்டமைப்பு testS {
	எழுத்து *name;
	எழுத்து *மதிப்பு;
};

நிலையான முழு test_strdup_to_member(முழு argc, எழுத்து *argv[])
{
	கட்டமைப்பு testS *xx;

	எனில் (argc < 3)
		திரும்பு EXIT_FAILURE;

	xx = calloc(1, அளவு(*xx));
	எனில் (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_to_struct_member(xx, name, argv[1]);
	strdup_to_struct_member(xx, மதிப்பு, argv[2]);

	எனில் (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->மதிப்பு, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	printf("1: '%s', 2: '%s'\n", xx->name, xx->மதிப்பு);

	விடுவி(xx->name);
	விடுவி(xx->மதிப்பு);
	விடுவி(xx);
	திரும்பு EXIT_SUCCESS;
}

நிலையான முழு test_strutils_sizes(முழு argc, எழுத்து *argv[])
{
	uintmax_t size = 0;
	எழுத்து *hum1, *hum2, *hum3;

	எனில் (argc < 2)
		திரும்பு EXIT_FAILURE;

	எனில் (strtosize(argv[1], &size))
		errx(EXIT_FAILURE, "invalid size '%s' value", argv[1]);

	hum1 = size_to_human_string(SIZE_SUFFIX_1LETTER, size);
	hum2 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE, size);
	hum3 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE |
				    SIZE_DECIMAL_2DIGITS, size);

	printf("%25s : %20ju : %8s : %12s : %13s\n", argv[1], size, hum1, hum2, hum3);
	விடுவி(hum1);
	விடுவி(hum2);
	விடுவி(hum3);

	திரும்பு EXIT_SUCCESS;
}

நிலையான முழு test_strutils_cmp_paths(முழு argc, எழுத்து *argv[])
{
	முழு rc = streq_paths(argv[1], argv[2]);

	எனில் (argc < 3)
		திரும்பு EXIT_FAILURE;

	printf("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	திரும்பு EXIT_SUCCESS;
}

நிலையான முழு test_strutils_path_is_within(முழு argc, எழுத்து *argv[])
{
	முழு rc;

	எனில் (argc < 3)
		திரும்பு EXIT_FAILURE;

	rc = ul_path_is_within(argv[1], argv[2]);

	printf("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	திரும்பு EXIT_SUCCESS;
}

நிலையான முழு test_strutils_normalize(முழு argc, எழுத்து *argv[])
{
	unsigned எழுத்து *src, *dst, *org;
	size_t sz, len;

	எனில் (argc < 2)
		திரும்பு EXIT_FAILURE;

	org = (unsigned எழுத்து *) strdup(argv[1]);
	src = (unsigned எழுத்து *) strdup((எழுத்து *) org);
	len = strlen((எழுத்து *) src);
	dst = malloc(len + 1);

	எனில் (!org || !src || !dst)
		செல் done;

	/* two buffers */
	sz = __normalize_whitespace(src, len, dst, len + 1);
	printf("1: '%s' --> '%s' [sz=%zu]\n", src, dst, sz);

	/* one buffer */
	sz = normalize_whitespace(src);
	printf("2: '%s' --> '%s' [sz=%zu]\n", org, src, sz);

done:
	விடுவி(src);
	விடுவி(dst);
	விடுவி(org);

	திரும்பு EXIT_SUCCESS;
}

நிலையான முழு test_strutils_cstrcasecmp(முழு argc, எழுத்து *argv[])
{
	எழுத்து *a, *b;

	எனில் (argc < 3)
		திரும்பு EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	எனில் (!a || !b)
		திரும்பு EXIT_FAILURE;

	printf("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	printf("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	printf("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, strlen(a)));

	திரும்பு EXIT_SUCCESS;
}

முழு முதன்மை(முழு argc, எழுத்து *argv[])
{
	எனில் (argc == 3 && strcmp(argv[1], "--size") == 0) {
		திரும்பு test_strutils_sizes(argc - 1, argv + 1);

	} இல்லையெனில் எனில் (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		printf("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						வெற்று));
		திரும்பு EXIT_SUCCESS;
	} இல்லையெனில் எனில் (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		எனில் (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		printf("'%s'-->%s\n", argv[2], val ? "true" : "false");
		திரும்பு EXIT_SUCCESS;

	} இல்லையெனில் எனில் (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		திரும்பு test_strutils_cmp_paths(argc - 1, argv + 1);

	} இல்லையெனில் எனில் (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		திரும்பு test_strutils_path_is_within(argc - 1, argv + 1);

	} இல்லையெனில் எனில் (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		திரும்பு test_strdup_to_member(argc - 1, argv + 1);

	} இல்லையெனில் எனில்  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		printf("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		திரும்பு EXIT_SUCCESS;

	} இல்லையெனில் எனில் (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		திரும்பு test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} இல்லையெனில் எனில் (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		திரும்பு test_strutils_normalize(argc - 1, argv + 1);

	} இல்லையெனில் எனில் (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		printf("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		திரும்பு EXIT_SUCCESS;
	} இல்லையெனில் எனில் (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		printf("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		திரும்பு EXIT_SUCCESS;
	} இல்லையெனில் எனில் (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		printf("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		திரும்பு EXIT_SUCCESS;
	} இல்லையெனில் எனில் (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		printf("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		திரும்பு EXIT_SUCCESS;
	} இல்லையெனில் எனில் (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		printf("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		திரும்பு EXIT_SUCCESS;
	} இல்லையெனில் எனில் (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		printf("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		திரும்பு EXIT_SUCCESS;

	} இல்லையெனில் எனில் (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		printf("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		திரும்பு EXIT_SUCCESS;

	} இல்லையெனில் எனில் (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		எழுத்து *buf = "abc\0Y\0\0xyz\0X";
		எழுத்து *end = buf + 12;
		எழுத்து *p = buf;

		செய் {
			printf("str: '%s'\n", p);
		} போது ((p = ul_next_string(p, end)));

		திரும்பு EXIT_SUCCESS;

	} இல்லையெனில் எனில் (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		எழுத்து *name = வெற்று, *val = வெற்று;
		எழுத்து *p = argv[2];
		முழு rc;

		எனில் (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		போது ((rc = ul_optstr_next(&p, &name, &namesz, &val, &valsz)) == 0) {
			printf("'%.*s' : '%.*s'\n", (முழு) namesz, name,
						   (முழு) valsz, val);
		}
		எனில் (rc == 1)
			திரும்பு EXIT_SUCCESS;
	} இல்லையெனில் எனில் (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		எழுத்து *unquoted = strdup(argv[2]);

		எனில் (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		printf("%s-->%s\n", argv[2], unquoted);
		விடுவி(unquoted);
		திரும்பு EXIT_SUCCESS;

	} இல்லையெனில் {
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

	திரும்பு EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_STRUTILS */
