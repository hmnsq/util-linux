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

ساكن صحيح STRTOXX_EXIT_ڪوڊ = EXIT_FAILURE;

فراغ strutils_سيٽ_exitcode(صحيح ex)
{
	STRTOXX_EXIT_ڪوڊ = ex;
}

ساكن صحيح do_scale_by_power (uintmax_t *x, صحيح بنياد, صحيح power)
{
	طالما (power--) {
		إذا (UINTMAX_MAX / بنياد < *x)
			أرجع -ERANGE;
		*x *= بنياد;
	}
	أرجع 0;
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
صحيح ul_parse_سائيز(ثابت حرف *str, uintmax_t *res, صحيح *power)
{
	ثابت حرف *p;
	حرف *پڇاڙي;
	uintmax_t x, frac = 0;
	صحيح بنياد = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	ساكن ثابت حرف *suf  = "KMGTPEZY";
	ساكن ثابت حرف *suf2 = "kmgtpezy";
	ثابت حرف *sp;

	*res = 0;

	إذا (!str || !*str) {
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
	طالما (خال_آهي((بنانشان حرف) *p))
		p++;
	إذا (*p == '-') {
		rc = -EINVAL;
		اذهب err;
	}

	errno = 0, پڇاڙي = عدم;
	x = strtoumax(str, &پڇاڙي, 0);

	إذا (پڇاڙي == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		اذهب err;
	}
	إذا (!پڇاڙي || !*پڇاڙي)
		اذهب پورو;			/* without suffix */
	p = پڇاڙي;

	/*
	 * Check size suffixes
	 */
check_suffix:
	إذا (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		بنياد = 1024;			/* XiB, 2^N */
	وإلا إذا ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		بنياد = 1000;			/* XB, 10^N */
	وإلا إذا (*(p + 1)) {
		بنية lconv ثابت *l = localeconv();
		ثابت حرف *dp = l ? l->decimal_نقطو : عدم;
		size_t dpsz = dp ? لڙي_ڊيگهه(dp) : 0;

		إذا (frac == 0 && *p && dp && لڙي_ڀيٽ(dp, p, dpsz) == 0) {
			ثابت حرف *fstr = p + dpsz;

			لكل (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			إذا (انگ_آهي(*fstr)) {
				errno = 0, پڇاڙي = عدم;
				frac = strtoumax(fstr, &پڇاڙي, 0);
				إذا (پڇاڙي == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					اذهب err;
				}
			} وإلا
				پڇاڙي = (حرف *) p;

			إذا (!پڇاڙي || !*پڇاڙي) {
				rc = -EINVAL;
				اذهب err;	/* without suffix, but with fractions */
			}
			p = پڇاڙي;
			اذهب check_suffix;
		}
		rc = -EINVAL;
		اذهب err;			/* unexpected suffix */
	}

	sp = strchr(suf, *p);
	إذا (sp)
		pwr = (sp - suf) + 1;
	وإلا {
		sp = strchr(suf2, *p);
		إذا (sp)
			pwr = (sp - suf2) + 1;
		وإلا {
			rc = -EINVAL;
			اذهب err;
		}
	}

	إذا (power)
		*power = pwr;

	rc = do_scale_by_power(&x, بنياد, pwr);
	إذا (rc)
		اذهب err;

	إذا (frac && pwr) {
		صحيح i;
		uintmax_t frac_بنياد = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_بنياد, بنياد, pwr);
		إذا (rc)
			اذهب err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		افعل {
			frac_val = ((frac % 10) * frac_بنياد + frac_val) / 10;
			frac /= 10;
		} طالما (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		لكل (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		إذا (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			اذهب err;
		}
		x += frac_val;
	}
پورو:
	*res = x;
err:
	إذا (rc < 0)
		errno = -rc;
	أرجع rc;
}

صحيح strtosize(ثابت حرف *str, uintmax_t *res)
{
	أرجع ul_parse_سائيز(str, res, عدم);
}

صحيح isdigit_strend(ثابت حرف *str, ثابت حرف **پڇاڙي)
{
	ثابت حرف *p;

	لكل (p = str; p && *p && انگ_آهي((بنانشان حرف) *p); p++);

	إذا (پڇاڙي)
		*پڇاڙي = p;
	أرجع p && p > str && !*p;
}

صحيح isxdigit_strend(ثابت حرف *str, ثابت حرف **پڇاڙي)
{
	ثابت حرف *p;

	لكل (p = str; p && *p && isxdigit((بنانشان حرف) *p); p++);

	إذا (پڇاڙي)
		*پڇاڙي = p;

	أرجع p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
صحيح ul_strtobool(ثابت حرف *str, bool *نتيجو)
{
	ساكن ثابت حرف *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	ساكن ثابت حرف *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	إذا (!str || !نتيجو)
		أرجع -(errno = EINVAL);

	errno = 0;

	لكل (i = 0; i < ARRAY_سائيز(bool_true); i++) {
		إذا (c_strcasecmp(str, bool_true[i]) == 0) {
			*نتيجو = true;
			أرجع 0;
		}
	}

	لكل (i = 0; i < ARRAY_سائيز(bool_false); i++) {
		إذا (c_strcasecmp(str, bool_false[i]) == 0) {
			*نتيجو = false;
			أرجع 0;
		}
	}

	أرجع -(errno = EINVAL);
}

صحيح ul_parse_switch(ثابت حرف *arg, ...)
{
	ثابت حرف *a, *b;
	va_list ap;

	va_start(ap, arg);
	افعل {
		a = va_arg(ap, حرف *);
		إذا (!a)
			اكسر;
		b = va_arg(ap, حرف *);
		إذا (!b)
			اكسر;

		إذا (strcmp(arg, a) == 0) {
			va_end(ap);
			أرجع 1;
		}

		إذا (strcmp(arg, b) == 0) {
			va_end(ap);
			أرجع 0;
		}
	} طالما (1);
	va_end(ap);

	errx(STRTOXX_EXIT_ڪوڊ, _("unsupported argument: %s"), arg);
}

#جيڪڏهن_وصف_نه_ٿيل HAVE_MEMPCPY
فراغ *mempcpy(فراغ *محدود dest, ثابت فراغ *محدود src, size_t n)
{
	أرجع ((حرف *)يادگيري_نقل(dest, src, n)) + n;
}
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل HAVE_STRNLEN
size_t strnlen(ثابت حرف *s, size_t maxlen)
{
	size_t i;

	لكل (i = 0; i < maxlen; i++) {
		إذا (s[i] == '\0')
			أرجع i;
	}
	أرجع maxlen;
}
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل HAVE_STRNCHR
حرف *strnchr(ثابت حرف *s, size_t maxlen, صحيح c)
{
	لكل (; maxlen-- && *s != '\0'; ++s)
		إذا (*s == (حرف)c)
			أرجع (حرف *)s;
	أرجع عدم;
}
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل HAVE_STRNDUP
حرف *strndup(ثابت حرف *s, size_t n)
{
	size_t len = strnlen(s, n);
	حرف *new = يادگيري_ڏيو((len + 1) * حجم(حرف));
	إذا (!new)
		أرجع عدم;
	new[len] = '\0';
	أرجع (حرف *) يادگيري_نقل(new, s, len);
}
#جيڪڏهن_پورو

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
صحيح ul_strtos64(ثابت حرف *str, int64_t *انگ, صحيح بنياد)
{
	حرف *پڇاڙي = عدم;

	إذا (str == عدم || *str == '\0')
		أرجع -(errno = EINVAL);

	errno = 0;
	*انگ = (int64_t) strtoimax(str, &پڇاڙي, بنياد);

	إذا (errno != 0)
		أرجع -errno;
	إذا (str == پڇاڙي || (پڇاڙي && *پڇاڙي))
		أرجع -(errno = EINVAL);
	أرجع 0;
}

صحيح ul_strtou64(ثابت حرف *str, uint64_t *انگ, صحيح بنياد)
{
	حرف *پڇاڙي = عدم;
	int64_t tmp;

	إذا (str == عدم || *str == '\0')
		أرجع -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &پڇاڙي, بنياد);
	إذا (tmp < 0)
		errno = ERANGE;
	وإلا {
		errno = 0;
		*انگ = strtoumax(str, &پڇاڙي, بنياد);
	}

	إذا (errno != 0)
		أرجع -errno;
	إذا (str == پڇاڙي || (پڇاڙي && *پڇاڙي))
		أرجع -(errno = EINVAL);
	أرجع 0;
}

صحيح ul_strtos32(ثابت حرف *str, int32_t *انگ, صحيح بنياد)
{
	int64_t tmp;
	صحيح rc;

	rc = ul_strtos64(str, &tmp, بنياد);
	إذا (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	إذا (rc == 0)
		*انگ = (int32_t) tmp;
	أرجع rc;
}

صحيح ul_strtou32(ثابت حرف *str, uint32_t *انگ, صحيح بنياد)
{
	uint64_t tmp;
	صحيح rc;

	rc = ul_strtou64(str, &tmp, بنياد);
	إذا (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	إذا (rc == 0)
		*انگ = (uint32_t) tmp;
	أرجع rc;
}

صحيح ul_strtou16(ثابت حرف *str, uint16_t *انگ, صحيح بنياد)
{
	uint64_t tmp;
	صحيح rc;

	rc = ul_strtou64(str, &tmp, بنياد);
	إذا (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	إذا (rc == 0)
		*انگ = (uint16_t) tmp;
	أرجع rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(ثابت حرف *str, صحيح بنياد, ثابت حرف *errmesg,
			     int64_t هيٺيون, int64_t مٿي)
{
	int64_t انگ = 0;
	صحيح rc;

	rc = ul_strtos64(str, &انگ, بنياد);
	إذا (rc == 0 && ((هيٺيون && انگ < هيٺيون) || (مٿي && انگ > مٿي)))
		rc = -(errno = ERANGE);

	إذا (rc) {
		إذا (errno == ERANGE)
			err(STRTOXX_EXIT_ڪوڊ, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_ڪوڊ, "%s: '%s'", errmesg, str);
	}
	أرجع انگ;
}

uint64_t str2unum_or_err(ثابت حرف *str, صحيح بنياد, ثابت حرف *errmesg, uint64_t مٿي)
{
	uint64_t انگ = 0;
	صحيح rc;

	rc = ul_strtou64(str, &انگ, بنياد);
	إذا (rc == 0 && (مٿي && انگ > مٿي))
		rc = -(errno = ERANGE);

	إذا (rc) {
		إذا (errno == ERANGE)
			err(STRTOXX_EXIT_ڪوڊ, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_ڪوڊ, "%s: '%s'", errmesg, str);
	}
	أرجع انگ;
}

مزدوج strtod_or_err(ثابت حرف *str, ثابت حرف *errmesg)
{
	مزدوج انگ;
	حرف *پڇاڙي = عدم;

	errno = 0;
	إذا (str == عدم || *str == '\0')
		اذهب err;
	انگ = strtod(str, &پڇاڙي);

	إذا (errno || str == پڇاڙي || (پڇاڙي && *پڇاڙي))
		اذهب err;

	أرجع انگ;
err:
	إذا (errno == ERANGE)
		err(STRTOXX_EXIT_ڪوڊ, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_ڪوڊ, "%s: '%s'", errmesg, str);
}

صحيح ul_strtold(ثابت حرف *str, ڊگهو مزدوج *انگ)
{
	حرف *پڇاڙي = عدم;

	errno = 0;
	إذا (str == عدم || *str == '\0')
		أرجع -(errno = EINVAL);
	*انگ = strtold(str, &پڇاڙي);

	إذا (errno != 0)
		أرجع -errno;
	إذا (str == پڇاڙي || (پڇاڙي && *پڇاڙي))
		أرجع -(errno = EINVAL);
	أرجع 0;
}

ڊگهو مزدوج strtold_or_err(ثابت حرف *str, ثابت حرف *errmesg)
{
	ڊگهو مزدوج انگ = 0;

	إذا (ul_strtold(str, &انگ) == 0)
		أرجع انگ;
	إذا (errno == ERANGE)
		err(STRTOXX_EXIT_ڪوڊ, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_ڪوڊ, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(ثابت حرف *str, ثابت حرف *errmesg)
{
	uintmax_t انگ;

	إذا (strtosize(str, &انگ) == 0)
		أرجع انگ;

	إذا (errno)
		err(STRTOXX_EXIT_ڪوڊ, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_ڪوڊ, "%s: '%s'", errmesg, str);
}


فراغ strtotimeval_or_err(ثابت حرف *str, بنية timeval *tv, ثابت حرف *errmesg)
{
	ڊگهو مزدوج user_داخلا;

	user_داخلا = strtold_or_err(str, errmesg);
	tv->tv_sec = (time_t) user_داخلا;
	tv->tv_usec = (suseconds_t)((user_داخلا - tv->tv_sec) * USEC_في_SEC);
}

فراغ strtotimespec_or_err(ثابت حرف *str, بنية timespec *ts, ثابت حرف *errmesg)
{
	ڊگهو مزدوج user_داخلا;

	user_داخلا = strtold_or_err(str, errmesg);
	ts->tv_sec = (time_t) user_داخلا;
	ts->tv_nsec = (ڊگهو)((user_داخلا - ts->tv_sec) * NSEC_في_SEC);
}

time_t strtotime_or_err(ثابت حرف *str, ثابت حرف *errmesg)
{
	int64_t user_داخلا;

	user_داخلا = strtos64_or_err(str, errmesg);
	أرجع (time_t) user_داخلا;
}

bool hyperlinkwanted(ثابت حرف *گهڻائي)
{
	إذا (گهڻائي && strcmp(گهڻائي, "never") == 0)
		أرجع false;

	إذا (گهڻائي && strcmp(گهڻائي, "always") == 0)
		أرجع true;

	إذا (!گهڻائي || strcmp(گهڻائي, "auto") == 0)
		أرجع isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), گهڻائي);
}

bool annotationwanted(ثابت حرف *گهڻائي)
{
	إذا (گهڻائي && strcmp(گهڻائي, "never") == 0)
		أرجع false;

	إذا (گهڻائي && strcmp(گهڻائي, "always") == 0)
		أرجع true;

	إذا (!گهڻائي || strcmp(گهڻائي, "auto") == 0)
		أرجع isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), گهڻائي);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
حرف *xstrmode(mode_t گهڻائي, حرف *str)
{
	بنانشان ننڍو i = 0;

	إذا (S_ISDIR(گهڻائي))
		str[i++] = 'd';
	وإلا إذا (S_ISLNK(گهڻائي))
		str[i++] = 'l';
	وإلا إذا (S_ISCHR(گهڻائي))
		str[i++] = 'c';
	وإلا إذا (S_ISBLK(گهڻائي))
		str[i++] = 'b';
	وإلا إذا (S_ISSOCK(گهڻائي))
		str[i++] = 's';
	وإلا إذا (S_ISFIFO(گهڻائي))
		str[i++] = 'p';
	وإلا إذا (S_ISREG(گهڻائي))
		str[i++] = '-';

	str[i++] = گهڻائي & S_IRUSR ? 'r' : '-';
	str[i++] = گهڻائي & S_IWUSR ? 'w' : '-';
	str[i++] = (گهڻائي & S_ISUID
		? (گهڻائي & S_IXUSR ? 's' : 'S')
		: (گهڻائي & S_IXUSR ? 'x' : '-'));
	str[i++] = گهڻائي & S_IRGRP ? 'r' : '-';
	str[i++] = گهڻائي & S_IWGRP ? 'w' : '-';
	str[i++] = (گهڻائي & S_ISGID
		? (گهڻائي & S_IXGRP ? 's' : 'S')
		: (گهڻائي & S_IXGRP ? 'x' : '-'));
	str[i++] = گهڻائي & S_IROTH ? 'r' : '-';
	str[i++] = گهڻائي & S_IWOTH ? 'w' : '-';
	str[i++] = (گهڻائي & S_ISVTX
		? (گهڻائي & S_IXOTH ? 't' : 'T')
		: (گهڻائي & S_IXOTH ? 'x' : '-'));
	str[i] = '\0';

	أرجع str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
ساكن صحيح get_exp(uint64_t n)
{
	صحيح shft;

	لكل (shft = 10; shft <= 60; shft += 10) {
		إذا (n < (1ULL << shft))
			اكسر;
	}
	أرجع shft - 10;
}

حرف *سائيز_منزل_human_string(صحيح options, uint64_t بائيٽون)
{
	حرف buf[32];
	صحيح dec, قوت_نما;
	uint64_t frac;
	ثابت حرف *letters = "BKMGTPE";
	حرف suffix[حجم(" KiB")], *psuf = suffix;
	حرف c;

	إذا (options & سائيز_SUFFIX_SPACE)
		*psuf++ = ' ';


	قوت_نما  = get_exp(بائيٽون);
	c    = *(letters + (قوت_نما ? قوت_نما / 10 : 0));
	dec  = قوت_نما ? بائيٽون / (1ULL << قوت_نما) : بائيٽون;
	frac = قوت_نما ? بائيٽون % (1ULL << قوت_نما) : 0;

	*psuf++ = c;

	إذا ((options & سائيز_SUFFIX_3_LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/*ڦيرو*/
	إذا (frac) {
		/* get 3 digits after decimal point */
		إذا (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (قوت_نما - 10)) ;
		وإلا
			frac = (frac * 1000) / (1ULL << (قوت_نما)) ;

		إذا (options & سائيز_DECIMAL_2_DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} وإلا {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		إذا (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	إذا (frac) {
		بنية lconv ثابت *l = localeconv();
		حرف *dp = l ? l->decimal_نقطو : عدم;
		صحيح len;

		إذا (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, حجم(buf), "%d%s%02" PRIu64, dec, dp, frac);
		إذا (len > 0 && (size_t) len < حجم(buf)) {
			/* remove potential extraneous zero */
			إذا (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, حجم(buf) - len);
		} وإلا
			*buf = '\0';	/* snprintf error */
	} وإلا
		snprintf(buf, حجم(buf), "%d%s", dec, suffix);

	أرجع strdup(buf);
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
صحيح string_منزل_idarray(ثابت حرف *list, صحيح ary[], size_t arysz,
			صحيح (name2id)(ثابت حرف *, size_t))
{
	ثابت حرف *begin = عدم, *p;
	size_t n = 0;

	إذا (!list || !*list || !ary || !arysz || !name2id)
		أرجع -1;

	لكل (p = list; p && *p; p++) {
		ثابت حرف *پڇاڙي = عدم;
		صحيح id;

		إذا (n >= arysz)
			أرجع -2;
		إذا (!begin)
			begin = p;		/* begin of the column name */
		إذا (*p == ',')
			پڇاڙي = p;		/* terminate the name */
		إذا (*(p + 1) == '\0')
			پڇاڙي = p + 1;		/* end of string */
		إذا (!begin || !پڇاڙي)
			استمر;
		إذا (پڇاڙي <= begin)
			أرجع -1;

		id = name2id(begin, پڇاڙي - begin);
		إذا (id == -1)
			أرجع -1;
		ary[ n++ ] = id;
		begin = عدم;
		إذا (پڇاڙي && !*پڇاڙي)
			اكسر;
	}
	أرجع n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
صحيح string_جوڙيو_منزل_idarray(ثابت حرف *list, صحيح ary[], size_t arysz,
			size_t *ary_pos, صحيح (name2id)(ثابت حرف *, size_t))
{
	ثابت حرف *list_جوڙيو;
	صحيح r;

	إذا (!list || !*list || !ary_pos || *ary_pos > arysz)
		أرجع -1;

	إذا (list[0] == '+')
		list_جوڙيو = &list[1];
	وإلا {
		list_جوڙيو = list;
		*ary_pos = 0;
	}

	r = string_منزل_idarray(list_جوڙيو, &ary[*ary_pos], arysz - *ary_pos, name2id);
	إذا (r > 0)
		*ary_pos += r;
	أرجع r;
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
صحيح string_منزل_bitarray(ثابت حرف *list,
		     حرف *ary,
		     صحيح (*name2bit)(ثابت حرف *, size_t),
		     size_t allow_range)
{
	ثابت حرف *begin = عدم, *p;

	إذا (!list || !name2bit || !ary)
		أرجع -EINVAL;

	لكل (p = list; p && *p; p++) {
		ثابت حرف *پڇاڙي = عدم;
		صحيح ٻٽو_انگ, سيٽ_هيٺاهون = 0, سيٽ_higher = 0;

		إذا (!begin)
			begin = p;		/* begin of the level name */
		إذا (*p == ',')
			پڇاڙي = p;		/* terminate the name */
		إذا (*(p + 1) == '\0')
			پڇاڙي = p + 1;		/* end of string */
		إذا (!begin || !پڇاڙي)
			استمر;
		إذا (پڇاڙي <= begin)
			أرجع -1;
		إذا (allow_range) {
			إذا (*(پڇاڙي - 1) == '+') {
				پڇاڙي--;
				سيٽ_هيٺاهون = 1;
			} وإلا إذا (*begin == '+') {
				begin++;
				سيٽ_higher = 1;
			}
		}

		ٻٽو_انگ = name2bit(begin, پڇاڙي - begin);
		إذا (ٻٽو_انگ < 0)
			أرجع ٻٽو_انگ;
		setbit(ary, ٻٽو_انگ);
		إذا (سيٽ_هيٺاهون)
			طالما (--ٻٽو_انگ >= 0)
				setbit(ary, ٻٽو_انگ);
		وإلا إذا (سيٽ_higher)
			طالما (++ٻٽو_انگ < (صحيح) allow_range)
				setbit(ary, ٻٽو_انگ);
		begin = عدم;
		إذا (پڇاڙي && !*پڇاڙي)
			اكسر;
	}
	أرجع 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
صحيح string_منزل_bitmask(ثابت حرف *list,
		     بنانشان ڊگهو *نقاب,
		     ڊگهو (*name2flag)(ثابت حرف *, size_t))
{
	ثابت حرف *begin = عدم, *p;

	إذا (!list || !name2flag || !نقاب)
		أرجع -EINVAL;

	لكل (p = list; p && *p; p++) {
		ثابت حرف *پڇاڙي = عدم;
		ڊگهو flag;

		إذا (!begin)
			begin = p;		/* begin of the level name */
		إذا (*p == ',')
			پڇاڙي = p;		/* terminate the name */
		إذا (*(p + 1) == '\0')
			پڇاڙي = p + 1;		/* end of string */
		إذا (!begin || !پڇاڙي)
			استمر;
		إذا (پڇاڙي <= begin)
			أرجع -1;

		flag = name2flag(begin, پڇاڙي - begin);
		إذا (flag < 0)
			أرجع flag;	/* error */
		*نقاب |= flag;
		begin = عدم;
		إذا (پڇاڙي && !*پڇاڙي)
			اكسر;
	}
	أرجع 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
صحيح ul_parse_range(ثابت حرف *str, صحيح *هيٺاهون, صحيح *مٿانهون, صحيح def)
{
	حرف *پڇاڙي = عدم;

	إذا (!str)
		أرجع 0;

	*مٿانهون = *هيٺاهون = def;
	errno = 0;

	إذا (*str == ':') {				/* <:N> */
		str++;
		*مٿانهون = strtol(str, &پڇاڙي, 10);
		إذا (errno || !پڇاڙي || *پڇاڙي || پڇاڙي == str)
			أرجع -1;
	} وإلا {
		*مٿانهون = *هيٺاهون = strtol(str, &پڇاڙي, 10);
		إذا (errno || !پڇاڙي || پڇاڙي == str)
			أرجع -1;

		إذا (*پڇاڙي == ':' && !*(پڇاڙي + 1))		/* <M:> */
			*مٿانهون = def;
		وإلا إذا (*پڇاڙي == '-' || *پڇاڙي == ':') {	/* <M:N> <M-N> */
			str = پڇاڙي + 1;
			پڇاڙي = عدم;
			errno = 0;
			*مٿانهون = strtol(str, &پڇاڙي, 10);

			إذا (errno || !پڇاڙي || *پڇاڙي || پڇاڙي == str)
				أرجع -1;
		}
	}
	أرجع 0;
}

ثابت حرف *ul_رستو_ايندڙ_segment(ثابت حرف *str, size_t *sz)
{
	ثابت حرف *شروعات, *p;

	شروعات = str;
	*sz = 0;
	طالما (شروعات && *شروعات == '/' && *(شروعات + 1) == '/')
		شروعات++;

	إذا (!شروعات || !*شروعات)
		أرجع عدم;

	لكل (*sz = 1, p = شروعات + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	أرجع شروعات;
}

صحيح streq_paths(ثابت حرف *a, ثابت حرف *b)
{
	طالما (a && b) {
		size_t a_sz, b_sz;
		ثابت حرف *a_seg = ul_رستو_ايندڙ_segment(a, &a_sz);
		ثابت حرف *b_seg = ul_رستو_ايندڙ_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		إذا (a_sz + b_sz == 0)
			أرجع 1;

		/* ignore trailing slash */
		إذا (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			أرجع 1;

		إذا (!a_seg || !b_seg)
			اكسر;
		إذا (a_sz != b_sz || لڙي_ڀيٽ(a_seg, b_seg, a_sz) != 0)
			اكسر;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	أرجع 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
صحيح ul_رستو_is_within(ثابت حرف *رستو, ثابت حرف *dir)
{
	إذا (!رستو || !dir || !*رستو || !*dir)
		أرجع 0;

	طالما (رستو && dir) {
		size_t p_sz, d_sz;
		ثابت حرف *p_seg = ul_رستو_ايندڙ_segment(رستو, &p_sz);
		ثابت حرف *d_seg = ul_رستو_ايندڙ_segment(dir, &d_sz);

		/* all @dir segments matched */
		إذا (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			أرجع 1;

		إذا (!p_seg || !d_seg)
			اكسر;
		إذا (p_sz != d_sz || لڙي_ڀيٽ(p_seg, d_seg, p_sz) != 0)
			اكسر;

		رستو = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	أرجع 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
حرف *ul_strnconcat(ثابت حرف *s, ثابت حرف *suffix, size_t b)
{
	size_t a;
	حرف *r;

	إذا (!s && !suffix)
		أرجع strdup("");
	إذا (!s)
		أرجع strndup(suffix, b);
	إذا (!suffix)
		أرجع strdup(s);

	أكد(s);
	أكد(suffix);

	a = لڙي_ڊيگهه(s);
	إذا (b > ((size_t) -1) - a)
		أرجع عدم;

	r = يادگيري_ڏيو(a + b + 1);
	إذا (!r)
		أرجع عدم;

	يادگيري_نقل(r, s, a);
	يادگيري_نقل(r + a, suffix, b);
	r[a+b] = 0;

	أرجع r;
}

/* concatenate two strings to a new string */
حرف *ul_strconcat(ثابت حرف *s, ثابت حرف *suffix)
{
	أرجع ul_strnconcat(s, suffix, suffix ? لڙي_ڊيگهه(suffix) : 0);
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

	إذا (sz < 0)
		أرجع عدم;

	res = ul_strnconcat(s, val, sz);
	حرر(val);
	أرجع res;
}

صحيح ul_strappend(حرف **a, ثابت حرف *b)
{
	size_t al, bl;
	حرف *tmp;

	إذا (!a)
		أرجع -EINVAL;
	إذا (!b || !*b)
		أرجع 0;
	إذا (!*a) {
		*a = strdup(b);
		أرجع !*a ? -ENOMEM : 0;
	}

	al = لڙي_ڊيگهه(*a);
	bl = لڙي_ڊيگهه(b);

	tmp = realloc(*a, al + bl + 1);
	إذا (!tmp)
		أرجع -ENOMEM;
	*a = tmp;
	يادگيري_نقل((*a) + al, b, bl + 1);
	أرجع 0;
}

/* the hybrid version of strfconcat and strappend. */
صحيح strfappend(حرف **a, ثابت حرف *format, ...)
{
	va_list ap;
	صحيح res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	أرجع res;
}

خارجي صحيح ul_strvfappend(حرف **a, ثابت حرف *format, va_list ap)
{
	حرف *val;
	صحيح sz;
	صحيح res;

	sz = vasprintf(&val, format, ap);
	إذا (sz < 0)
		أرجع -errno;

	res = ul_strappend(a, val);
	حرر(val);
	أرجع res;
}

ساكن size_t strcspn_escaped(ثابت حرف *s, ثابت حرف *reject)
{
	صحيح escaped = 0;
	صحيح n;

	لكل (n=0; s[n]; n++) {
		إذا (escaped)
			escaped = 0;
		وإلا إذا (s[n] == '\\')
			escaped = 1;
		وإلا إذا (strchr(reject, s[n]))
			اكسر;
	}

	/* if s ends in \, return index of previous char */
	أرجع n - escaped;
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

	لكل (p = (حرف *) s; p && *p; p++) {
		إذا (!esc && *p == '\\') {
			esc = 1;
			استمر;
		}
		إذا (*p == c && (!esc || c == '\\'))
			أرجع p;
		esc = 0;
	}

	أرجع عدم;
}

/* Split a string into words. */
ثابت حرف *ul_ورهايو(ثابت حرف **state, size_t *l, ثابت حرف *separator, صحيح quoted)
{
	ثابت حرف *current;

	current = *state;

	إذا (!*current) {
		أكد(**state == '\0');
		أرجع عدم;
	}

	current += strspn(current, separator);
	إذا (!*current) {
		*state = current;
		أرجع عدم;
	}

	إذا (quoted && strchr("\'\"", *current)) {
		حرف quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		إذا (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			أرجع عدم;
		}
		*state = current++ + *l + 2;
	} وإلا إذا (quoted) {
		*l = strcspn_escaped(current, separator);
		إذا (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			أرجع عدم;
		}
		*state = current + *l;
	} وإلا {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	أرجع current;
}

/* Rewind file pointer forward to new line.  */
صحيح skip_fline(FILE *fp)
{
	صحيح ch;

	افعل {
		إذا ((ch = fgetc(fp)) == EOF)
			أرجع 1;
		إذا (ch == '\n')
			أرجع 0;
	} طالما (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
صحيح ul_stralnumcmp(ثابت حرف *p1, ثابت حرف *p2)
{
	ثابت بنانشان حرف *s1 = (ثابت بنانشان حرف *) p1;
	ثابت بنانشان حرف *s2 = (ثابت بنانشان حرف *) p2;
	بنانشان حرف c1, c2;

	افعل {
		افعل {
			c1 = (بنانشان حرف) *s1++;
		} طالما (c1 != '\0' && !اکر_يا_انگ((بنانشان صحيح) c1));

		افعل {
			c2 = (بنانشان حرف) *s2++;
		} طالما (c2 != '\0' && !اکر_يا_انگ((بنانشان صحيح) c2));

		إذا (c1 != '\0')
			c1 = tolower(c1);
		إذا (c2 != '\0')
			c2 = tolower(c2);
		إذا (c1 == '\0')
			أرجع c1 - c2;
	} طالما (c1 == c2);

	أرجع c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
صحيح ul_optstr_ايندڙ(حرف **optstr, حرف **name, size_t *namesz,
		   حرف **قدر, size_t *valsz)
{
	صحيح open_quote = 0;
	حرف *شروعات = عدم, *stop = عدم, *p, *sep = عدم;
	حرف *optstr0;

	أكد(optstr);
	أكد(*optstr);

	optstr0 = *optstr;

	إذا (name)
		*name = عدم;
	إذا (namesz)
		*namesz = 0;
	إذا (قدر)
		*قدر = عدم;
	إذا (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	طالما (optstr0 && *optstr0 == ',')
		optstr0++;

	لكل (p = optstr0; p && *p; p++) {
		إذا (!شروعات && *p == '=')
			أرجع -EINVAL;
		إذا (!شروعات)
			شروعات = p;		/* beginning of the option item */
		إذا (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		إذا (open_quote)
			استمر;		/* still in quoted block */
		إذا (!sep && p > شروعات && *p == '=')
			sep = p;		/* name and value separator */
		إذا (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		وإلا إذا (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		إذا (!شروعات || !stop)
			استمر;
		إذا (stop <= شروعات)
			أرجع -EINVAL;

		إذا (name)
			*name = شروعات;
		إذا (namesz)
			*namesz = sep ? sep - شروعات : stop - شروعات;
		*optstr = *stop ? stop + 1 : stop;

		إذا (sep) {
			إذا (قدر)
				*قدر = sep + 1;
			إذا (valsz)
				*valsz = stop - sep - 1;
		}
		أرجع 0;
	}

	أرجع 1;				/* end of optstr */
}

صحيح ul_optstr_is_valid(ثابت حرف *optstr)
{
	صحيح rc;
	حرف *p = (حرف *) optstr;

	طالما ((rc = ul_optstr_ايندڙ(&p, عدم, عدم, عدم, عدم)) == 0);
	أرجع rc < 0 ? 0 : 1;
}

حرف *ul_optstr_get_قدر(ثابت حرف *optstr, ثابت حرف *ڪنجي)
{
	size_t sz, namesz = 0, valsz = 0;
	حرف *name = عدم, *قدر = عدم;
	حرف *p = (حرف *) optstr;

	إذا (!optstr || !ڪنجي || !*ڪنجي)
		أرجع عدم;

	sz = لڙي_ڊيگهه(ڪنجي);
	طالما (ul_optstr_ايندڙ(&p, &name, &namesz, &قدر, &valsz) == 0) {
		إذا (namesz != sz || !valsz)
			استمر;
		إذا (لڙي_ڀيٽ(name, ڪنجي, namesz) == 0)
			أرجع strndup(قدر, valsz);
	}
	أرجع عدم;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(حرف *str)
{
	size_t len;
	حرف q;

	إذا (!str || !*str)
		أرجع 0;

	len = لڙي_ڊيگهه(str);
	q = *str;

	إذا ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		أرجع len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	أرجع len - 2;
}

#جيڪڏهن_وصف_ٿيل TEST_پروگرام_STRUTILS

بنية testS {
	حرف *name;
	حرف *قدر;
};

ساكن صحيح test_strdup_منزل_member(صحيح argc, حرف *argv[])
{
	بنية testS *xx;

	إذا (argc < 3)
		أرجع EXIT_FAILURE;

	xx = صفر_يادگيري_ڏيو(1, حجم(*xx));
	إذا (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_منزل_struct_member(xx, name, argv[1]);
	strdup_منزل_struct_member(xx, قدر, argv[2]);

	إذا (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->قدر, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	لکو("1: '%s', 2: '%s'\n", xx->name, xx->قدر);

	حرر(xx->name);
	حرر(xx->قدر);
	حرر(xx);
	أرجع EXIT_SUCCESS;
}

ساكن صحيح test_strutils_sizes(صحيح argc, حرف *argv[])
{
	uintmax_t سائيز = 0;
	حرف *hum1, *hum2, *hum3;

	إذا (argc < 2)
		أرجع EXIT_FAILURE;

	إذا (strtosize(argv[1], &سائيز))
		errx(EXIT_FAILURE, "invalid size '%s' value", argv[1]);

	hum1 = سائيز_منزل_human_string(سائيز_SUFFIX_1_LETTER, سائيز);
	hum2 = سائيز_منزل_human_string(سائيز_SUFFIX_3_LETTER |
				    سائيز_SUFFIX_SPACE, سائيز);
	hum3 = سائيز_منزل_human_string(سائيز_SUFFIX_3_LETTER |
				    سائيز_SUFFIX_SPACE |
				    سائيز_DECIMAL_2_DIGITS, سائيز);

	لکو("%25s : %20ju : %8s : %12s : %13s\n", argv[1], سائيز, hum1, hum2, hum3);
	حرر(hum1);
	حرر(hum2);
	حرر(hum3);

	أرجع EXIT_SUCCESS;
}

ساكن صحيح test_strutils_cmp_paths(صحيح argc, حرف *argv[])
{
	صحيح rc = streq_paths(argv[1], argv[2]);

	إذا (argc < 3)
		أرجع EXIT_FAILURE;

	لکو("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	أرجع EXIT_SUCCESS;
}

ساكن صحيح test_strutils_رستو_is_within(صحيح argc, حرف *argv[])
{
	صحيح rc;

	إذا (argc < 3)
		أرجع EXIT_FAILURE;

	rc = ul_رستو_is_within(argv[1], argv[2]);

	لکو("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	أرجع EXIT_SUCCESS;
}

ساكن صحيح test_strutils_normalize(صحيح argc, حرف *argv[])
{
	بنانشان حرف *src, *dst, *org;
	size_t sz, len;

	إذا (argc < 2)
		أرجع EXIT_FAILURE;

	org = (بنانشان حرف *) strdup(argv[1]);
	src = (بنانشان حرف *) strdup((حرف *) org);
	len = لڙي_ڊيگهه((حرف *) src);
	dst = يادگيري_ڏيو(len + 1);

	إذا (!org || !src || !dst)
		اذهب پورو;

	/* two buffers */
	sz = __normalize_whitespace(src, len, dst, len + 1);
	لکو("1: '%s' --> '%s' [sz=%zu]\n", src, dst, sz);

	/* one buffer */
	sz = normalize_whitespace(src);
	لکو("2: '%s' --> '%s' [sz=%zu]\n", org, src, sz);

پورو:
	حرر(src);
	حرر(dst);
	حرر(org);

	أرجع EXIT_SUCCESS;
}

ساكن صحيح test_strutils_cstrcasecmp(صحيح argc, حرف *argv[])
{
	حرف *a, *b;

	إذا (argc < 3)
		أرجع EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	إذا (!a || !b)
		أرجع EXIT_FAILURE;

	لکو("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	لکو("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	لکو("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, لڙي_ڊيگهه(a)));

	أرجع EXIT_SUCCESS;
}

صحيح مکيه(صحيح argc, حرف *argv[])
{
	إذا (argc == 3 && strcmp(argv[1], "--size") == 0) {
		أرجع test_strutils_sizes(argc - 1, argv + 1);

	} وإلا إذا (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		لکو("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						عدم));
		أرجع EXIT_SUCCESS;
	} وإلا إذا (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		إذا (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		لکو("'%s'-->%s\n", argv[2], val ? "true" : "false");
		أرجع EXIT_SUCCESS;

	} وإلا إذا (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		أرجع test_strutils_cmp_paths(argc - 1, argv + 1);

	} وإلا إذا (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		أرجع test_strutils_رستو_is_within(argc - 1, argv + 1);

	} وإلا إذا (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		أرجع test_strdup_منزل_member(argc - 1, argv + 1);

	} وإلا إذا  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		لکو("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		أرجع EXIT_SUCCESS;

	} وإلا إذا (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		أرجع test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} وإلا إذا (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		أرجع test_strutils_normalize(argc - 1, argv + 1);

	} وإلا إذا (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		لکو("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		أرجع EXIT_SUCCESS;
	} وإلا إذا (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		لکو("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		أرجع EXIT_SUCCESS;
	} وإلا إذا (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		لکو("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		أرجع EXIT_SUCCESS;
	} وإلا إذا (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		لکو("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		أرجع EXIT_SUCCESS;
	} وإلا إذا (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		لکو("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		أرجع EXIT_SUCCESS;
	} وإلا إذا (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		لکو("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		أرجع EXIT_SUCCESS;

	} وإلا إذا (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		لکو("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		أرجع EXIT_SUCCESS;

	} وإلا إذا (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		حرف *buf = "abc\0Y\0\0xyz\0X";
		حرف *پڇاڙي = buf + 12;
		حرف *p = buf;

		افعل {
			لکو("str: '%s'\n", p);
		} طالما ((p = ul_ايندڙ_string(p, پڇاڙي)));

		أرجع EXIT_SUCCESS;

	} وإلا إذا (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		حرف *name = عدم, *val = عدم;
		حرف *p = argv[2];
		صحيح rc;

		إذا (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		طالما ((rc = ul_optstr_ايندڙ(&p, &name, &namesz, &val, &valsz)) == 0) {
			لکو("'%.*s' : '%.*s'\n", (صحيح) namesz, name,
						   (صحيح) valsz, val);
		}
		إذا (rc == 1)
			أرجع EXIT_SUCCESS;
	} وإلا إذا (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		حرف *unquoted = strdup(argv[2]);

		إذا (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		لکو("%s-->%s\n", argv[2], unquoted);
		حرر(unquoted);
		أرجع EXIT_SUCCESS;

	} وإلا {
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
		ٻاهر_نڪرو(EXIT_FAILURE);
	}

	أرجع EXIT_FAILURE;
}
#جيڪڏهن_پورو /* TEST_PROGRAM_STRUTILS */
