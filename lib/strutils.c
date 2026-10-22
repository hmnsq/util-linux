/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Karel Zak <kzak@redhat.com> [2010]
 *          Davidlohr Bueso <dave@gnu.org> [2010]
 */
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <inttypes.h>
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <strings.h>
#અન્તર્ભાવ <assert.h>

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "cctype.h"
#અન્તર્ભાવ "nls.h"
#અન્તર્ભાવ "strutils.h"
#અન્તર્ભાવ "timeutils.h"
#અન્તર્ભાવ "bitops.h"
#અન્તર્ભાવ "pathnames.h"

સ્થિર પૂર્ણાંક STRTOXX_EXIT_CODE = EXIT_FAILURE;

શૂન્ય strutils_set_exitcode(પૂર્ણાંક ex)
{
	STRTOXX_EXIT_CODE = ex;
}

સ્થિર પૂર્ણાંક do_scale_by_power (uintmax_t *x, પૂર્ણાંક base, પૂર્ણાંક power)
{
	જ્યારે (power--) {
		જો (UINTMAX_MAX / base < *x)
			પાછા -ERANGE;
		*x *= base;
	}
	પાછા 0;
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
પૂર્ણાંક ul_parse_size(અચલ અક્ષર *str, uintmax_t *res, પૂર્ણાંક *power)
{
	અચલ અક્ષર *p;
	અક્ષર *end;
	uintmax_t x, frac = 0;
	પૂર્ણાંક base = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	સ્થિર અચલ અક્ષર *suf  = "KMGTPEZY";
	સ્થિર અચલ અક્ષર *suf2 = "kmgtpezy";
	અચલ અક્ષર *sp;

	*res = 0;

	જો (!str || !*str) {
		rc = -EINVAL;
		જાઓ err;
	}

	/* Only positive numbers are acceptable
	 *
	 * Note that this check is not perfect, it would be better to
	 * use lconv->negative_sign. But coreutils use the same solution,
	 * so it's probably good enough...
	 */
	p = str;
	જ્યારે (isspace((unsigned અક્ષર) *p))
		p++;
	જો (*p == '-') {
		rc = -EINVAL;
		જાઓ err;
	}

	errno = 0, end = ખાલી;
	x = strtoumax(str, &end, 0);

	જો (end == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		જાઓ err;
	}
	જો (!end || !*end)
		જાઓ done;			/* without suffix */
	p = end;

	/*
	 * Check size suffixes
	 */
check_suffix:
	જો (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		base = 1024;			/* XiB, 2^N */
	નહીંતર જો ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		base = 1000;			/* XB, 10^N */
	નહીંતર જો (*(p + 1)) {
		સંરચના lconv અચલ *l = localeconv();
		અચલ અક્ષર *dp = l ? l->decimal_point : ખાલી;
		size_t dpsz = dp ? strlen(dp) : 0;

		જો (frac == 0 && *p && dp && strncmp(dp, p, dpsz) == 0) {
			અચલ અક્ષર *fstr = p + dpsz;

			માટે (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			જો (isdigit(*fstr)) {
				errno = 0, end = ખાલી;
				frac = strtoumax(fstr, &end, 0);
				જો (end == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					જાઓ err;
				}
			} નહીંતર
				end = (અક્ષર *) p;

			જો (!end || !*end) {
				rc = -EINVAL;
				જાઓ err;	/* without suffix, but with fractions */
			}
			p = end;
			જાઓ check_suffix;
		}
		rc = -EINVAL;
		જાઓ err;			/* unexpected suffix */
	}

	sp = strchr(suf, *p);
	જો (sp)
		pwr = (sp - suf) + 1;
	નહીંતર {
		sp = strchr(suf2, *p);
		જો (sp)
			pwr = (sp - suf2) + 1;
		નહીંતર {
			rc = -EINVAL;
			જાઓ err;
		}
	}

	જો (power)
		*power = pwr;

	rc = do_scale_by_power(&x, base, pwr);
	જો (rc)
		જાઓ err;

	જો (frac && pwr) {
		પૂર્ણાંક i;
		uintmax_t frac_base = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_base, base, pwr);
		જો (rc)
			જાઓ err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		કરો {
			frac_val = ((frac % 10) * frac_base + frac_val) / 10;
			frac /= 10;
		} જ્યારે (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		માટે (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		જો (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			જાઓ err;
		}
		x += frac_val;
	}
done:
	*res = x;
err:
	જો (rc < 0)
		errno = -rc;
	પાછા rc;
}

પૂર્ણાંક strtosize(અચલ અક્ષર *str, uintmax_t *res)
{
	પાછા ul_parse_size(str, res, ખાલી);
}

પૂર્ણાંક isdigit_strend(અચલ અક્ષર *str, અચલ અક્ષર **end)
{
	અચલ અક્ષર *p;

	માટે (p = str; p && *p && isdigit((unsigned અક્ષર) *p); p++);

	જો (end)
		*end = p;
	પાછા p && p > str && !*p;
}

પૂર્ણાંક isxdigit_strend(અચલ અક્ષર *str, અચલ અક્ષર **end)
{
	અચલ અક્ષર *p;

	માટે (p = str; p && *p && isxdigit((unsigned અક્ષર) *p); p++);

	જો (end)
		*end = p;

	પાછા p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
પૂર્ણાંક ul_strtobool(અચલ અક્ષર *str, bool *result)
{
	સ્થિર અચલ અક્ષર *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	સ્થિર અચલ અક્ષર *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	જો (!str || !result)
		પાછા -(errno = EINVAL);

	errno = 0;

	માટે (i = 0; i < ARRAY_SIZE(bool_true); i++) {
		જો (c_strcasecmp(str, bool_true[i]) == 0) {
			*result = true;
			પાછા 0;
		}
	}

	માટે (i = 0; i < ARRAY_SIZE(bool_false); i++) {
		જો (c_strcasecmp(str, bool_false[i]) == 0) {
			*result = false;
			પાછા 0;
		}
	}

	પાછા -(errno = EINVAL);
}

પૂર્ણાંક ul_parse_switch(અચલ અક્ષર *arg, ...)
{
	અચલ અક્ષર *a, *b;
	va_list ap;

	va_start(ap, arg);
	કરો {
		a = va_arg(ap, અક્ષર *);
		જો (!a)
			તોડો;
		b = va_arg(ap, અક્ષર *);
		જો (!b)
			તોડો;

		જો (strcmp(arg, a) == 0) {
			va_end(ap);
			પાછા 1;
		}

		જો (strcmp(arg, b) == 0) {
			va_end(ap);
			પાછા 0;
		}
	} જ્યારે (1);
	va_end(ap);

	errx(STRTOXX_EXIT_CODE, _("unsupported argument: %s"), arg);
}

#ifndef HAVE_MEMPCPY
શૂન્ય *mempcpy(શૂન્ય *restrict dest, અચલ શૂન્ય *restrict src, size_t n)
{
	પાછા ((અક્ષર *)memcpy(dest, src, n)) + n;
}
#endif

#ifndef HAVE_STRNLEN
size_t strnlen(અચલ અક્ષર *s, size_t maxlen)
{
	size_t i;

	માટે (i = 0; i < maxlen; i++) {
		જો (s[i] == '\0')
			પાછા i;
	}
	પાછા maxlen;
}
#endif

#ifndef HAVE_STRNCHR
અક્ષર *strnchr(અચલ અક્ષર *s, size_t maxlen, પૂર્ણાંક c)
{
	માટે (; maxlen-- && *s != '\0'; ++s)
		જો (*s == (અક્ષર)c)
			પાછા (અક્ષર *)s;
	પાછા ખાલી;
}
#endif

#ifndef HAVE_STRNDUP
અક્ષર *strndup(અચલ અક્ષર *s, size_t n)
{
	size_t len = strnlen(s, n);
	અક્ષર *new = malloc((len + 1) * કદ(અક્ષર));
	જો (!new)
		પાછા ખાલી;
	new[len] = '\0';
	પાછા (અક્ષર *) memcpy(new, s, len);
}
#endif

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
પૂર્ણાંક ul_strtos64(અચલ અક્ષર *str, int64_t *અંક, પૂર્ણાંક base)
{
	અક્ષર *end = ખાલી;

	જો (str == ખાલી || *str == '\0')
		પાછા -(errno = EINVAL);

	errno = 0;
	*અંક = (int64_t) strtoimax(str, &end, base);

	જો (errno != 0)
		પાછા -errno;
	જો (str == end || (end && *end))
		પાછા -(errno = EINVAL);
	પાછા 0;
}

પૂર્ણાંક ul_strtou64(અચલ અક્ષર *str, uint64_t *અંક, પૂર્ણાંક base)
{
	અક્ષર *end = ખાલી;
	int64_t tmp;

	જો (str == ખાલી || *str == '\0')
		પાછા -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &end, base);
	જો (tmp < 0)
		errno = ERANGE;
	નહીંતર {
		errno = 0;
		*અંક = strtoumax(str, &end, base);
	}

	જો (errno != 0)
		પાછા -errno;
	જો (str == end || (end && *end))
		પાછા -(errno = EINVAL);
	પાછા 0;
}

પૂર્ણાંક ul_strtos32(અચલ અક્ષર *str, int32_t *અંક, પૂર્ણાંક base)
{
	int64_t tmp;
	પૂર્ણાંક rc;

	rc = ul_strtos64(str, &tmp, base);
	જો (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	જો (rc == 0)
		*અંક = (int32_t) tmp;
	પાછા rc;
}

પૂર્ણાંક ul_strtou32(અચલ અક્ષર *str, uint32_t *અંક, પૂર્ણાંક base)
{
	uint64_t tmp;
	પૂર્ણાંક rc;

	rc = ul_strtou64(str, &tmp, base);
	જો (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	જો (rc == 0)
		*અંક = (uint32_t) tmp;
	પાછા rc;
}

પૂર્ણાંક ul_strtou16(અચલ અક્ષર *str, uint16_t *અંક, પૂર્ણાંક base)
{
	uint64_t tmp;
	પૂર્ણાંક rc;

	rc = ul_strtou64(str, &tmp, base);
	જો (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	જો (rc == 0)
		*અંક = (uint16_t) tmp;
	પાછા rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(અચલ અક્ષર *str, પૂર્ણાંક base, અચલ અક્ષર *errmesg,
			     int64_t નીચલું, int64_t up)
{
	int64_t અંક = 0;
	પૂર્ણાંક rc;

	rc = ul_strtos64(str, &અંક, base);
	જો (rc == 0 && ((નીચલું && અંક < નીચલું) || (up && અંક > up)))
		rc = -(errno = ERANGE);

	જો (rc) {
		જો (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	પાછા અંક;
}

uint64_t str2unum_or_err(અચલ અક્ષર *str, પૂર્ણાંક base, અચલ અક્ષર *errmesg, uint64_t up)
{
	uint64_t અંક = 0;
	પૂર્ણાંક rc;

	rc = ul_strtou64(str, &અંક, base);
	જો (rc == 0 && (up && અંક > up))
		rc = -(errno = ERANGE);

	જો (rc) {
		જો (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	પાછા અંક;
}

દ્વિગુણ strtod_or_err(અચલ અક્ષર *str, અચલ અક્ષર *errmesg)
{
	દ્વિગુણ અંક;
	અક્ષર *end = ખાલી;

	errno = 0;
	જો (str == ખાલી || *str == '\0')
		જાઓ err;
	અંક = strtod(str, &end);

	જો (errno || str == end || (end && *end))
		જાઓ err;

	પાછા અંક;
err:
	જો (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

પૂર્ણાંક ul_strtold(અચલ અક્ષર *str, long દ્વિગુણ *અંક)
{
	અક્ષર *end = ખાલી;

	errno = 0;
	જો (str == ખાલી || *str == '\0')
		પાછા -(errno = EINVAL);
	*અંક = strtold(str, &end);

	જો (errno != 0)
		પાછા -errno;
	જો (str == end || (end && *end))
		પાછા -(errno = EINVAL);
	પાછા 0;
}

long દ્વિગુણ strtold_or_err(અચલ અક્ષર *str, અચલ અક્ષર *errmesg)
{
	long દ્વિગુણ અંક = 0;

	જો (ul_strtold(str, &અંક) == 0)
		પાછા અંક;
	જો (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(અચલ અક્ષર *str, અચલ અક્ષર *errmesg)
{
	uintmax_t અંક;

	જો (strtosize(str, &અંક) == 0)
		પાછા અંક;

	જો (errno)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}


શૂન્ય strtotimeval_or_err(અચલ અક્ષર *str, સંરચના timeval *tv, અચલ અક્ષર *errmesg)
{
	long દ્વિગુણ user_આગત;

	user_આગત = strtold_or_err(str, errmesg);
	tv->tv_sec = (time_t) user_આગત;
	tv->tv_usec = (suseconds_t)((user_આગત - tv->tv_sec) * USEC_PER_SEC);
}

શૂન્ય strtotimespec_or_err(અચલ અક્ષર *str, સંરચના timespec *ts, અચલ અક્ષર *errmesg)
{
	long દ્વિગુણ user_આગત;

	user_આગત = strtold_or_err(str, errmesg);
	ts->tv_sec = (time_t) user_આગત;
	ts->tv_nsec = (long)((user_આગત - ts->tv_sec) * NSEC_PER_SEC);
}

time_t strtotime_or_err(અચલ અક્ષર *str, અચલ અક્ષર *errmesg)
{
	int64_t user_આગત;

	user_આગત = strtos64_or_err(str, errmesg);
	પાછા (time_t) user_આગત;
}

bool hyperlinkwanted(અચલ અક્ષર *mode)
{
	જો (mode && strcmp(mode, "never") == 0)
		પાછા false;

	જો (mode && strcmp(mode, "always") == 0)
		પાછા true;

	જો (!mode || strcmp(mode, "auto") == 0)
		પાછા isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), mode);
}

bool annotationwanted(અચલ અક્ષર *mode)
{
	જો (mode && strcmp(mode, "never") == 0)
		પાછા false;

	જો (mode && strcmp(mode, "always") == 0)
		પાછા true;

	જો (!mode || strcmp(mode, "auto") == 0)
		પાછા isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), mode);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
અક્ષર *xstrmode(mode_t mode, અક્ષર *str)
{
	unsigned short i = 0;

	જો (S_ISDIR(mode))
		str[i++] = 'd';
	નહીંતર જો (S_ISLNK(mode))
		str[i++] = 'l';
	નહીંતર જો (S_ISCHR(mode))
		str[i++] = 'c';
	નહીંતર જો (S_ISBLK(mode))
		str[i++] = 'b';
	નહીંતર જો (S_ISSOCK(mode))
		str[i++] = 's';
	નહીંતર જો (S_ISFIFO(mode))
		str[i++] = 'p';
	નહીંતર જો (S_ISREG(mode))
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

	પાછા str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
સ્થિર પૂર્ણાંક get_exp(uint64_t n)
{
	પૂર્ણાંક shft;

	માટે (shft = 10; shft <= 60; shft += 10) {
		જો (n < (1ULL << shft))
			તોડો;
	}
	પાછા shft - 10;
}

અક્ષર *size_to_human_string(પૂર્ણાંક options, uint64_t bytes)
{
	અક્ષર buf[32];
	પૂર્ણાંક dec, exp;
	uint64_t frac;
	અચલ અક્ષર *letters = "BKMGTPE";
	અક્ષર suffix[કદ(" KiB")], *psuf = suffix;
	અક્ષર c;

	જો (options & SIZE_SUFFIX_SPACE)
		*psuf++ = ' ';


	exp  = get_exp(bytes);
	c    = *(letters + (exp ? exp / 10 : 0));
	dec  = exp ? bytes / (1ULL << exp) : bytes;
	frac = exp ? bytes % (1ULL << exp) : 0;

	*psuf++ = c;

	જો ((options & SIZE_SUFFIX_3LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/* round */
	જો (frac) {
		/* get 3 digits after decimal point */
		જો (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (exp - 10)) ;
		નહીંતર
			frac = (frac * 1000) / (1ULL << (exp)) ;

		જો (options & SIZE_DECIMAL_2DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} નહીંતર {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		જો (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	જો (frac) {
		સંરચના lconv અચલ *l = localeconv();
		અક્ષર *dp = l ? l->decimal_point : ખાલી;
		પૂર્ણાંક len;

		જો (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, કદ(buf), "%d%s%02" PRIu64, dec, dp, frac);
		જો (len > 0 && (size_t) len < કદ(buf)) {
			/* remove potential extraneous zero */
			જો (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, કદ(buf) - len);
		} નહીંતર
			*buf = '\0';	/* snprintf error */
	} નહીંતર
		snprintf(buf, કદ(buf), "%d%s", dec, suffix);

	પાછા strdup(buf);
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
પૂર્ણાંક string_to_idarray(અચલ અક્ષર *list, પૂર્ણાંક ary[], size_t arysz,
			પૂર્ણાંક (name2id)(અચલ અક્ષર *, size_t))
{
	અચલ અક્ષર *begin = ખાલી, *p;
	size_t n = 0;

	જો (!list || !*list || !ary || !arysz || !name2id)
		પાછા -1;

	માટે (p = list; p && *p; p++) {
		અચલ અક્ષર *end = ખાલી;
		પૂર્ણાંક id;

		જો (n >= arysz)
			પાછા -2;
		જો (!begin)
			begin = p;		/* begin of the column name */
		જો (*p == ',')
			end = p;		/* terminate the name */
		જો (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		જો (!begin || !end)
			ચાલુ;
		જો (end <= begin)
			પાછા -1;

		id = name2id(begin, end - begin);
		જો (id == -1)
			પાછા -1;
		ary[ n++ ] = id;
		begin = ખાલી;
		જો (end && !*end)
			તોડો;
	}
	પાછા n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
પૂર્ણાંક string_add_to_idarray(અચલ અક્ષર *list, પૂર્ણાંક ary[], size_t arysz,
			size_t *ary_pos, પૂર્ણાંક (name2id)(અચલ અક્ષર *, size_t))
{
	અચલ અક્ષર *list_add;
	પૂર્ણાંક r;

	જો (!list || !*list || !ary_pos || *ary_pos > arysz)
		પાછા -1;

	જો (list[0] == '+')
		list_add = &list[1];
	નહીંતર {
		list_add = list;
		*ary_pos = 0;
	}

	r = string_to_idarray(list_add, &ary[*ary_pos], arysz - *ary_pos, name2id);
	જો (r > 0)
		*ary_pos += r;
	પાછા r;
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
પૂર્ણાંક string_to_bitarray(અચલ અક્ષર *list,
		     અક્ષર *ary,
		     પૂર્ણાંક (*name2bit)(અચલ અક્ષર *, size_t),
		     size_t allow_range)
{
	અચલ અક્ષર *begin = ખાલી, *p;

	જો (!list || !name2bit || !ary)
		પાછા -EINVAL;

	માટે (p = list; p && *p; p++) {
		અચલ અક્ષર *end = ખાલી;
		પૂર્ણાંક bit, set_lower = 0, set_higher = 0;

		જો (!begin)
			begin = p;		/* begin of the level name */
		જો (*p == ',')
			end = p;		/* terminate the name */
		જો (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		જો (!begin || !end)
			ચાલુ;
		જો (end <= begin)
			પાછા -1;
		જો (allow_range) {
			જો (*(end - 1) == '+') {
				end--;
				set_lower = 1;
			} નહીંતર જો (*begin == '+') {
				begin++;
				set_higher = 1;
			}
		}

		bit = name2bit(begin, end - begin);
		જો (bit < 0)
			પાછા bit;
		setbit(ary, bit);
		જો (set_lower)
			જ્યારે (--bit >= 0)
				setbit(ary, bit);
		નહીંતર જો (set_higher)
			જ્યારે (++bit < (પૂર્ણાંક) allow_range)
				setbit(ary, bit);
		begin = ખાલી;
		જો (end && !*end)
			તોડો;
	}
	પાછા 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
પૂર્ણાંક string_to_bitmask(અચલ અક્ષર *list,
		     unsigned long *mask,
		     long (*name2flag)(અચલ અક્ષર *, size_t))
{
	અચલ અક્ષર *begin = ખાલી, *p;

	જો (!list || !name2flag || !mask)
		પાછા -EINVAL;

	માટે (p = list; p && *p; p++) {
		અચલ અક્ષર *end = ખાલી;
		long flag;

		જો (!begin)
			begin = p;		/* begin of the level name */
		જો (*p == ',')
			end = p;		/* terminate the name */
		જો (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		જો (!begin || !end)
			ચાલુ;
		જો (end <= begin)
			પાછા -1;

		flag = name2flag(begin, end - begin);
		જો (flag < 0)
			પાછા flag;	/* error */
		*mask |= flag;
		begin = ખાલી;
		જો (end && !*end)
			તોડો;
	}
	પાછા 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
પૂર્ણાંક ul_parse_range(અચલ અક્ષર *str, પૂર્ણાંક *lower, પૂર્ણાંક *upper, પૂર્ણાંક def)
{
	અક્ષર *end = ખાલી;

	જો (!str)
		પાછા 0;

	*upper = *lower = def;
	errno = 0;

	જો (*str == ':') {				/* <:N> */
		str++;
		*upper = strtol(str, &end, 10);
		જો (errno || !end || *end || end == str)
			પાછા -1;
	} નહીંતર {
		*upper = *lower = strtol(str, &end, 10);
		જો (errno || !end || end == str)
			પાછા -1;

		જો (*end == ':' && !*(end + 1))		/* <M:> */
			*upper = def;
		નહીંતર જો (*end == '-' || *end == ':') {	/* <M:N> <M-N> */
			str = end + 1;
			end = ખાલી;
			errno = 0;
			*upper = strtol(str, &end, 10);

			જો (errno || !end || *end || end == str)
				પાછા -1;
		}
	}
	પાછા 0;
}

અચલ અક્ષર *ul_path_next_segment(અચલ અક્ષર *str, size_t *sz)
{
	અચલ અક્ષર *start, *p;

	start = str;
	*sz = 0;
	જ્યારે (start && *start == '/' && *(start + 1) == '/')
		start++;

	જો (!start || !*start)
		પાછા ખાલી;

	માટે (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	પાછા start;
}

પૂર્ણાંક streq_paths(અચલ અક્ષર *a, અચલ અક્ષર *b)
{
	જ્યારે (a && b) {
		size_t a_sz, b_sz;
		અચલ અક્ષર *a_seg = ul_path_next_segment(a, &a_sz);
		અચલ અક્ષર *b_seg = ul_path_next_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		જો (a_sz + b_sz == 0)
			પાછા 1;

		/* ignore trailing slash */
		જો (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			પાછા 1;

		જો (!a_seg || !b_seg)
			તોડો;
		જો (a_sz != b_sz || strncmp(a_seg, b_seg, a_sz) != 0)
			તોડો;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	પાછા 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
પૂર્ણાંક ul_path_is_within(અચલ અક્ષર *path, અચલ અક્ષર *dir)
{
	જો (!path || !dir || !*path || !*dir)
		પાછા 0;

	જ્યારે (path && dir) {
		size_t p_sz, d_sz;
		અચલ અક્ષર *p_seg = ul_path_next_segment(path, &p_sz);
		અચલ અક્ષર *d_seg = ul_path_next_segment(dir, &d_sz);

		/* all @dir segments matched */
		જો (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			પાછા 1;

		જો (!p_seg || !d_seg)
			તોડો;
		જો (p_sz != d_sz || strncmp(p_seg, d_seg, p_sz) != 0)
			તોડો;

		path = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	પાછા 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
અક્ષર *ul_strnconcat(અચલ અક્ષર *s, અચલ અક્ષર *suffix, size_t b)
{
	size_t a;
	અક્ષર *r;

	જો (!s && !suffix)
		પાછા strdup("");
	જો (!s)
		પાછા strndup(suffix, b);
	જો (!suffix)
		પાછા strdup(s);

	પ્રતિજ્ઞા(s);
	પ્રતિજ્ઞા(suffix);

	a = strlen(s);
	જો (b > ((size_t) -1) - a)
		પાછા ખાલી;

	r = malloc(a + b + 1);
	જો (!r)
		પાછા ખાલી;

	memcpy(r, s, a);
	memcpy(r + a, suffix, b);
	r[a+b] = 0;

	પાછા r;
}

/* concatenate two strings to a new string */
અક્ષર *ul_strconcat(અચલ અક્ષર *s, અચલ અક્ષર *suffix)
{
	પાછા ul_strnconcat(s, suffix, suffix ? strlen(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
અક્ષર *ul_strfconcat(અચલ અક્ષર *s, અચલ અક્ષર *format, ...)
{
	va_list ap;
	અક્ષર *val, *res;
	પૂર્ણાંક sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	જો (sz < 0)
		પાછા ખાલી;

	res = ul_strnconcat(s, val, sz);
	મુક્ત(val);
	પાછા res;
}

પૂર્ણાંક ul_strappend(અક્ષર **a, અચલ અક્ષર *b)
{
	size_t al, bl;
	અક્ષર *tmp;

	જો (!a)
		પાછા -EINVAL;
	જો (!b || !*b)
		પાછા 0;
	જો (!*a) {
		*a = strdup(b);
		પાછા !*a ? -ENOMEM : 0;
	}

	al = strlen(*a);
	bl = strlen(b);

	tmp = realloc(*a, al + bl + 1);
	જો (!tmp)
		પાછા -ENOMEM;
	*a = tmp;
	memcpy((*a) + al, b, bl + 1);
	પાછા 0;
}

/* the hybrid version of strfconcat and strappend. */
પૂર્ણાંક strfappend(અક્ષર **a, અચલ અક્ષર *format, ...)
{
	va_list ap;
	પૂર્ણાંક res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	પાછા res;
}

બાહ્ય પૂર્ણાંક ul_strvfappend(અક્ષર **a, અચલ અક્ષર *format, va_list ap)
{
	અક્ષર *val;
	પૂર્ણાંક sz;
	પૂર્ણાંક res;

	sz = vasprintf(&val, format, ap);
	જો (sz < 0)
		પાછા -errno;

	res = ul_strappend(a, val);
	મુક્ત(val);
	પાછા res;
}

સ્થિર size_t strcspn_escaped(અચલ અક્ષર *s, અચલ અક્ષર *reject)
{
	પૂર્ણાંક escaped = 0;
	પૂર્ણાંક n;

	માટે (n=0; s[n]; n++) {
		જો (escaped)
			escaped = 0;
		નહીંતર જો (s[n] == '\\')
			escaped = 1;
		નહીંતર જો (strchr(reject, s[n]))
			તોડો;
	}

	/* if s ends in \, return index of previous char */
	પાછા n - escaped;
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
અક્ષર *ul_strchr_escaped(અચલ અક્ષર *s, પૂર્ણાંક c)
{
	અક્ષર *p;
	પૂર્ણાંક esc = 0;

	માટે (p = (અક્ષર *) s; p && *p; p++) {
		જો (!esc && *p == '\\') {
			esc = 1;
			ચાલુ;
		}
		જો (*p == c && (!esc || c == '\\'))
			પાછા p;
		esc = 0;
	}

	પાછા ખાલી;
}

/* Split a string into words. */
અચલ અક્ષર *ul_split(અચલ અક્ષર **state, size_t *l, અચલ અક્ષર *separator, પૂર્ણાંક quoted)
{
	અચલ અક્ષર *current;

	current = *state;

	જો (!*current) {
		પ્રતિજ્ઞા(**state == '\0');
		પાછા ખાલી;
	}

	current += strspn(current, separator);
	જો (!*current) {
		*state = current;
		પાછા ખાલી;
	}

	જો (quoted && strchr("\'\"", *current)) {
		અક્ષર quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		જો (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			પાછા ખાલી;
		}
		*state = current++ + *l + 2;
	} નહીંતર જો (quoted) {
		*l = strcspn_escaped(current, separator);
		જો (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			પાછા ખાલી;
		}
		*state = current + *l;
	} નહીંતર {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	પાછા current;
}

/* Rewind file pointer forward to new line.  */
પૂર્ણાંક skip_fline(FILE *fp)
{
	પૂર્ણાંક ch;

	કરો {
		જો ((ch = fgetc(fp)) == EOF)
			પાછા 1;
		જો (ch == '\n')
			પાછા 0;
	} જ્યારે (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
પૂર્ણાંક ul_stralnumcmp(અચલ અક્ષર *p1, અચલ અક્ષર *p2)
{
	અચલ unsigned અક્ષર *s1 = (અચલ unsigned અક્ષર *) p1;
	અચલ unsigned અક્ષર *s2 = (અચલ unsigned અક્ષર *) p2;
	unsigned અક્ષર c1, c2;

	કરો {
		કરો {
			c1 = (unsigned અક્ષર) *s1++;
		} જ્યારે (c1 != '\0' && !isalnum((unsigned પૂર્ણાંક) c1));

		કરો {
			c2 = (unsigned અક્ષર) *s2++;
		} જ્યારે (c2 != '\0' && !isalnum((unsigned પૂર્ણાંક) c2));

		જો (c1 != '\0')
			c1 = tolower(c1);
		જો (c2 != '\0')
			c2 = tolower(c2);
		જો (c1 == '\0')
			પાછા c1 - c2;
	} જ્યારે (c1 == c2);

	પાછા c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
પૂર્ણાંક ul_optstr_next(અક્ષર **optstr, અક્ષર **name, size_t *namesz,
		   અક્ષર **કિંમત, size_t *valsz)
{
	પૂર્ણાંક open_quote = 0;
	અક્ષર *start = ખાલી, *stop = ખાલી, *p, *sep = ખાલી;
	અક્ષર *optstr0;

	પ્રતિજ્ઞા(optstr);
	પ્રતિજ્ઞા(*optstr);

	optstr0 = *optstr;

	જો (name)
		*name = ખાલી;
	જો (namesz)
		*namesz = 0;
	જો (કિંમત)
		*કિંમત = ખાલી;
	જો (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	જ્યારે (optstr0 && *optstr0 == ',')
		optstr0++;

	માટે (p = optstr0; p && *p; p++) {
		જો (!start && *p == '=')
			પાછા -EINVAL;
		જો (!start)
			start = p;		/* beginning of the option item */
		જો (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		જો (open_quote)
			ચાલુ;		/* still in quoted block */
		જો (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		જો (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		નહીંતર જો (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		જો (!start || !stop)
			ચાલુ;
		જો (stop <= start)
			પાછા -EINVAL;

		જો (name)
			*name = start;
		જો (namesz)
			*namesz = sep ? sep - start : stop - start;
		*optstr = *stop ? stop + 1 : stop;

		જો (sep) {
			જો (કિંમત)
				*કિંમત = sep + 1;
			જો (valsz)
				*valsz = stop - sep - 1;
		}
		પાછા 0;
	}

	પાછા 1;				/* end of optstr */
}

પૂર્ણાંક ul_optstr_is_valid(અચલ અક્ષર *optstr)
{
	પૂર્ણાંક rc;
	અક્ષર *p = (અક્ષર *) optstr;

	જ્યારે ((rc = ul_optstr_next(&p, ખાલી, ખાલી, ખાલી, ખાલી)) == 0);
	પાછા rc < 0 ? 0 : 1;
}

અક્ષર *ul_optstr_get_કિંમત(અચલ અક્ષર *optstr, અચલ અક્ષર *ચાવી)
{
	size_t sz, namesz = 0, valsz = 0;
	અક્ષર *name = ખાલી, *કિંમત = ખાલી;
	અક્ષર *p = (અક્ષર *) optstr;

	જો (!optstr || !ચાવી || !*ચાવી)
		પાછા ખાલી;

	sz = strlen(ચાવી);
	જ્યારે (ul_optstr_next(&p, &name, &namesz, &કિંમત, &valsz) == 0) {
		જો (namesz != sz || !valsz)
			ચાલુ;
		જો (strncmp(name, ચાવી, namesz) == 0)
			પાછા strndup(કિંમત, valsz);
	}
	પાછા ખાલી;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(અક્ષર *str)
{
	size_t len;
	અક્ષર q;

	જો (!str || !*str)
		પાછા 0;

	len = strlen(str);
	q = *str;

	જો ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		પાછા len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	પાછા len - 2;
}

#ifdef TEST_PROGRAM_STRUTILS

સંરચના testS {
	અક્ષર *name;
	અક્ષર *કિંમત;
};

સ્થિર પૂર્ણાંક test_strdup_to_member(પૂર્ણાંક argc, અક્ષર *argv[])
{
	સંરચના testS *xx;

	જો (argc < 3)
		પાછા EXIT_FAILURE;

	xx = calloc(1, કદ(*xx));
	જો (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_to_struct_member(xx, name, argv[1]);
	strdup_to_struct_member(xx, કિંમત, argv[2]);

	જો (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->કિંમત, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	printf("1: '%s', 2: '%s'\n", xx->name, xx->કિંમત);

	મુક્ત(xx->name);
	મુક્ત(xx->કિંમત);
	મુક્ત(xx);
	પાછા EXIT_SUCCESS;
}

સ્થિર પૂર્ણાંક test_strutils_sizes(પૂર્ણાંક argc, અક્ષર *argv[])
{
	uintmax_t size = 0;
	અક્ષર *hum1, *hum2, *hum3;

	જો (argc < 2)
		પાછા EXIT_FAILURE;

	જો (strtosize(argv[1], &size))
		errx(EXIT_FAILURE, "invalid size '%s' value", argv[1]);

	hum1 = size_to_human_string(SIZE_SUFFIX_1LETTER, size);
	hum2 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE, size);
	hum3 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE |
				    SIZE_DECIMAL_2DIGITS, size);

	printf("%25s : %20ju : %8s : %12s : %13s\n", argv[1], size, hum1, hum2, hum3);
	મુક્ત(hum1);
	મુક્ત(hum2);
	મુક્ત(hum3);

	પાછા EXIT_SUCCESS;
}

સ્થિર પૂર્ણાંક test_strutils_cmp_paths(પૂર્ણાંક argc, અક્ષર *argv[])
{
	પૂર્ણાંક rc = streq_paths(argv[1], argv[2]);

	જો (argc < 3)
		પાછા EXIT_FAILURE;

	printf("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	પાછા EXIT_SUCCESS;
}

સ્થિર પૂર્ણાંક test_strutils_path_is_within(પૂર્ણાંક argc, અક્ષર *argv[])
{
	પૂર્ણાંક rc;

	જો (argc < 3)
		પાછા EXIT_FAILURE;

	rc = ul_path_is_within(argv[1], argv[2]);

	printf("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	પાછા EXIT_SUCCESS;
}

સ્થિર પૂર્ણાંક test_strutils_normalize(પૂર્ણાંક argc, અક્ષર *argv[])
{
	unsigned અક્ષર *src, *dst, *org;
	size_t sz, len;

	જો (argc < 2)
		પાછા EXIT_FAILURE;

	org = (unsigned અક્ષર *) strdup(argv[1]);
	src = (unsigned અક્ષર *) strdup((અક્ષર *) org);
	len = strlen((અક્ષર *) src);
	dst = malloc(len + 1);

	જો (!org || !src || !dst)
		જાઓ done;

	/* two buffers */
	sz = __normalize_whitespace(src, len, dst, len + 1);
	printf("1: '%s' --> '%s' [sz=%zu]\n", src, dst, sz);

	/* one buffer */
	sz = normalize_whitespace(src);
	printf("2: '%s' --> '%s' [sz=%zu]\n", org, src, sz);

done:
	મુક્ત(src);
	મુક્ત(dst);
	મુક્ત(org);

	પાછા EXIT_SUCCESS;
}

સ્થિર પૂર્ણાંક test_strutils_cstrcasecmp(પૂર્ણાંક argc, અક્ષર *argv[])
{
	અક્ષર *a, *b;

	જો (argc < 3)
		પાછા EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	જો (!a || !b)
		પાછા EXIT_FAILURE;

	printf("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	printf("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	printf("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, strlen(a)));

	પાછા EXIT_SUCCESS;
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[])
{
	જો (argc == 3 && strcmp(argv[1], "--size") == 0) {
		પાછા test_strutils_sizes(argc - 1, argv + 1);

	} નહીંતર જો (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		printf("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						ખાલી));
		પાછા EXIT_SUCCESS;
	} નહીંતર જો (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		જો (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		printf("'%s'-->%s\n", argv[2], val ? "true" : "false");
		પાછા EXIT_SUCCESS;

	} નહીંતર જો (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		પાછા test_strutils_cmp_paths(argc - 1, argv + 1);

	} નહીંતર જો (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		પાછા test_strutils_path_is_within(argc - 1, argv + 1);

	} નહીંતર જો (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		પાછા test_strdup_to_member(argc - 1, argv + 1);

	} નહીંતર જો  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		printf("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		પાછા EXIT_SUCCESS;

	} નહીંતર જો (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		પાછા test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} નહીંતર જો (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		પાછા test_strutils_normalize(argc - 1, argv + 1);

	} નહીંતર જો (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		printf("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		પાછા EXIT_SUCCESS;
	} નહીંતર જો (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		printf("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		પાછા EXIT_SUCCESS;
	} નહીંતર જો (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		printf("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		પાછા EXIT_SUCCESS;
	} નહીંતર જો (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		printf("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		પાછા EXIT_SUCCESS;
	} નહીંતર જો (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		printf("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		પાછા EXIT_SUCCESS;
	} નહીંતર જો (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		printf("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		પાછા EXIT_SUCCESS;

	} નહીંતર જો (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		printf("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		પાછા EXIT_SUCCESS;

	} નહીંતર જો (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		અક્ષર *buf = "abc\0Y\0\0xyz\0X";
		અક્ષર *end = buf + 12;
		અક્ષર *p = buf;

		કરો {
			printf("str: '%s'\n", p);
		} જ્યારે ((p = ul_next_string(p, end)));

		પાછા EXIT_SUCCESS;

	} નહીંતર જો (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		અક્ષર *name = ખાલી, *val = ખાલી;
		અક્ષર *p = argv[2];
		પૂર્ણાંક rc;

		જો (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		જ્યારે ((rc = ul_optstr_next(&p, &name, &namesz, &val, &valsz)) == 0) {
			printf("'%.*s' : '%.*s'\n", (પૂર્ણાંક) namesz, name,
						   (પૂર્ણાંક) valsz, val);
		}
		જો (rc == 1)
			પાછા EXIT_SUCCESS;
	} નહીંતર જો (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		અક્ષર *unquoted = strdup(argv[2]);

		જો (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		printf("%s-->%s\n", argv[2], unquoted);
		મુક્ત(unquoted);
		પાછા EXIT_SUCCESS;

	} નહીંતર {
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

	પાછા EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_STRUTILS */
