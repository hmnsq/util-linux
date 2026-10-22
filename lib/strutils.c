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

ساکن صحیح STRTOXX_EXIT_رمز = EXIT_FAILURE;

خالی strutils_سیٹ_exitcode(صحیح ex)
{
	STRTOXX_EXIT_رمز = ex;
}

ساکن صحیح do_scale_by_power (uintmax_t *x, صحیح بنیاد, صحیح power)
{
	جب (power--) {
		اگر (UINTMAX_MAX / بنیاد < *x)
			واپس -ERANGE;
		*x *= بنیاد;
	}
	واپس 0;
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
صحیح ul_parse_سائز(ثابت حرف *str, uintmax_t *res, صحیح *power)
{
	ثابت حرف *p;
	حرف *انتہا;
	uintmax_t x, frac = 0;
	صحیح بنیاد = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	ساکن ثابت حرف *suf  = "KMGTPEZY";
	ساکن ثابت حرف *suf2 = "kmgtpezy";
	ثابت حرف *sp;

	*res = 0;

	اگر (!str || !*str) {
		rc = -EINVAL;
		جاؤ err;
	}

	/* Only positive numbers are acceptable
	 *
	 * Note that this check is not perfect, it would be better to
	 * use lconv->negative_sign. But coreutils use the same solution,
	 * so it's probably good enough...
	 */
	p = str;
	جب (خلا_ہے((غیرعلامتی حرف) *p))
		p++;
	اگر (*p == '-') {
		rc = -EINVAL;
		جاؤ err;
	}

	errno = 0, انتہا = تہی;
	x = strtoumax(str, &انتہا, 0);

	اگر (انتہا == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		جاؤ err;
	}
	اگر (!انتہا || !*انتہا)
		جاؤ مکمل;			/* without suffix */
	p = انتہا;

	/*
	 * Check size suffixes
	 */
check_suffix:
	اگر (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		بنیاد = 1024;			/* XiB, 2^N */
	ورنہ اگر ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		بنیاد = 1000;			/* XB, 10^N */
	ورنہ اگر (*(p + 1)) {
		ساخت lconv ثابت *l = localeconv();
		ثابت حرف *dp = l ? l->decimal_نقطہ : تہی;
		size_t dpsz = dp ? لڑی_لمبائی(dp) : 0;

		اگر (frac == 0 && *p && dp && لڑی_موازنہ(dp, p, dpsz) == 0) {
			ثابت حرف *fstr = p + dpsz;

			برائے (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			اگر (ہندسہ_ہے(*fstr)) {
				errno = 0, انتہا = تہی;
				frac = strtoumax(fstr, &انتہا, 0);
				اگر (انتہا == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					جاؤ err;
				}
			} ورنہ
				انتہا = (حرف *) p;

			اگر (!انتہا || !*انتہا) {
				rc = -EINVAL;
				جاؤ err;	/* without suffix, but with fractions */
			}
			p = انتہا;
			جاؤ check_suffix;
		}
		rc = -EINVAL;
		جاؤ err;			/* unexpected suffix */
	}

	sp = strchr(suf, *p);
	اگر (sp)
		pwr = (sp - suf) + 1;
	ورنہ {
		sp = strchr(suf2, *p);
		اگر (sp)
			pwr = (sp - suf2) + 1;
		ورنہ {
			rc = -EINVAL;
			جاؤ err;
		}
	}

	اگر (power)
		*power = pwr;

	rc = do_scale_by_power(&x, بنیاد, pwr);
	اگر (rc)
		جاؤ err;

	اگر (frac && pwr) {
		صحیح i;
		uintmax_t frac_بنیاد = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_بنیاد, بنیاد, pwr);
		اگر (rc)
			جاؤ err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		کرو {
			frac_val = ((frac % 10) * frac_بنیاد + frac_val) / 10;
			frac /= 10;
		} جب (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		برائے (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		اگر (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			جاؤ err;
		}
		x += frac_val;
	}
مکمل:
	*res = x;
err:
	اگر (rc < 0)
		errno = -rc;
	واپس rc;
}

صحیح strtosize(ثابت حرف *str, uintmax_t *res)
{
	واپس ul_parse_سائز(str, res, تہی);
}

صحیح isdigit_strend(ثابت حرف *str, ثابت حرف **انتہا)
{
	ثابت حرف *p;

	برائے (p = str; p && *p && ہندسہ_ہے((غیرعلامتی حرف) *p); p++);

	اگر (انتہا)
		*انتہا = p;
	واپس p && p > str && !*p;
}

صحیح isxdigit_strend(ثابت حرف *str, ثابت حرف **انتہا)
{
	ثابت حرف *p;

	برائے (p = str; p && *p && isxdigit((غیرعلامتی حرف) *p); p++);

	اگر (انتہا)
		*انتہا = p;

	واپس p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
صحیح ul_strtobool(ثابت حرف *str, bool *نتیجہ)
{
	ساکن ثابت حرف *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	ساکن ثابت حرف *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	اگر (!str || !نتیجہ)
		واپس -(errno = EINVAL);

	errno = 0;

	برائے (i = 0; i < ARRAY_سائز(bool_true); i++) {
		اگر (c_strcasecmp(str, bool_true[i]) == 0) {
			*نتیجہ = true;
			واپس 0;
		}
	}

	برائے (i = 0; i < ARRAY_سائز(bool_false); i++) {
		اگر (c_strcasecmp(str, bool_false[i]) == 0) {
			*نتیجہ = false;
			واپس 0;
		}
	}

	واپس -(errno = EINVAL);
}

صحیح ul_parse_switch(ثابت حرف *arg, ...)
{
	ثابت حرف *a, *b;
	va_list ap;

	va_start(ap, arg);
	کرو {
		a = va_arg(ap, حرف *);
		اگر (!a)
			توڑو;
		b = va_arg(ap, حرف *);
		اگر (!b)
			توڑو;

		اگر (strcmp(arg, a) == 0) {
			va_end(ap);
			واپس 1;
		}

		اگر (strcmp(arg, b) == 0) {
			va_end(ap);
			واپس 0;
		}
	} جب (1);
	va_end(ap);

	errx(STRTOXX_EXIT_رمز, _("unsupported argument: %s"), arg);
}

#اگر_غیر_تعریف_شدہ HAVE_MEMPCPY
خالی *mempcpy(خالی *محدود dest, ثابت خالی *محدود src, size_t n)
{
	واپس ((حرف *)حافظہ_نقل(dest, src, n)) + n;
}
#اگر_ختم

#اگر_غیر_تعریف_شدہ HAVE_STRNLEN
size_t strnlen(ثابت حرف *s, size_t maxlen)
{
	size_t i;

	برائے (i = 0; i < maxlen; i++) {
		اگر (s[i] == '\0')
			واپس i;
	}
	واپس maxlen;
}
#اگر_ختم

#اگر_غیر_تعریف_شدہ HAVE_STRNCHR
حرف *strnchr(ثابت حرف *s, size_t maxlen, صحیح c)
{
	برائے (; maxlen-- && *s != '\0'; ++s)
		اگر (*s == (حرف)c)
			واپس (حرف *)s;
	واپس تہی;
}
#اگر_ختم

#اگر_غیر_تعریف_شدہ HAVE_STRNDUP
حرف *strndup(ثابت حرف *s, size_t n)
{
	size_t len = strnlen(s, n);
	حرف *new = حافظہ_دو((len + 1) * حجم(حرف));
	اگر (!new)
		واپس تہی;
	new[len] = '\0';
	واپس (حرف *) حافظہ_نقل(new, s, len);
}
#اگر_ختم

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
صحیح ul_strtos64(ثابت حرف *str, int64_t *عدد, صحیح بنیاد)
{
	حرف *انتہا = تہی;

	اگر (str == تہی || *str == '\0')
		واپس -(errno = EINVAL);

	errno = 0;
	*عدد = (int64_t) strtoimax(str, &انتہا, بنیاد);

	اگر (errno != 0)
		واپس -errno;
	اگر (str == انتہا || (انتہا && *انتہا))
		واپس -(errno = EINVAL);
	واپس 0;
}

صحیح ul_strtou64(ثابت حرف *str, uint64_t *عدد, صحیح بنیاد)
{
	حرف *انتہا = تہی;
	int64_t tmp;

	اگر (str == تہی || *str == '\0')
		واپس -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &انتہا, بنیاد);
	اگر (tmp < 0)
		errno = ERANGE;
	ورنہ {
		errno = 0;
		*عدد = strtoumax(str, &انتہا, بنیاد);
	}

	اگر (errno != 0)
		واپس -errno;
	اگر (str == انتہا || (انتہا && *انتہا))
		واپس -(errno = EINVAL);
	واپس 0;
}

صحیح ul_strtos32(ثابت حرف *str, int32_t *عدد, صحیح بنیاد)
{
	int64_t tmp;
	صحیح rc;

	rc = ul_strtos64(str, &tmp, بنیاد);
	اگر (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	اگر (rc == 0)
		*عدد = (int32_t) tmp;
	واپس rc;
}

صحیح ul_strtou32(ثابت حرف *str, uint32_t *عدد, صحیح بنیاد)
{
	uint64_t tmp;
	صحیح rc;

	rc = ul_strtou64(str, &tmp, بنیاد);
	اگر (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	اگر (rc == 0)
		*عدد = (uint32_t) tmp;
	واپس rc;
}

صحیح ul_strtou16(ثابت حرف *str, uint16_t *عدد, صحیح بنیاد)
{
	uint64_t tmp;
	صحیح rc;

	rc = ul_strtou64(str, &tmp, بنیاد);
	اگر (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	اگر (rc == 0)
		*عدد = (uint16_t) tmp;
	واپس rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(ثابت حرف *str, صحیح بنیاد, ثابت حرف *errmesg,
			     int64_t کم, int64_t اوپر)
{
	int64_t عدد = 0;
	صحیح rc;

	rc = ul_strtos64(str, &عدد, بنیاد);
	اگر (rc == 0 && ((کم && عدد < کم) || (اوپر && عدد > اوپر)))
		rc = -(errno = ERANGE);

	اگر (rc) {
		اگر (errno == ERANGE)
			err(STRTOXX_EXIT_رمز, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_رمز, "%s: '%s'", errmesg, str);
	}
	واپس عدد;
}

uint64_t str2unum_or_err(ثابت حرف *str, صحیح بنیاد, ثابت حرف *errmesg, uint64_t اوپر)
{
	uint64_t عدد = 0;
	صحیح rc;

	rc = ul_strtou64(str, &عدد, بنیاد);
	اگر (rc == 0 && (اوپر && عدد > اوپر))
		rc = -(errno = ERANGE);

	اگر (rc) {
		اگر (errno == ERANGE)
			err(STRTOXX_EXIT_رمز, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_رمز, "%s: '%s'", errmesg, str);
	}
	واپس عدد;
}

دوہرا strtod_or_err(ثابت حرف *str, ثابت حرف *errmesg)
{
	دوہرا عدد;
	حرف *انتہا = تہی;

	errno = 0;
	اگر (str == تہی || *str == '\0')
		جاؤ err;
	عدد = strtod(str, &انتہا);

	اگر (errno || str == انتہا || (انتہا && *انتہا))
		جاؤ err;

	واپس عدد;
err:
	اگر (errno == ERANGE)
		err(STRTOXX_EXIT_رمز, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_رمز, "%s: '%s'", errmesg, str);
}

صحیح ul_strtold(ثابت حرف *str, طویل دوہرا *عدد)
{
	حرف *انتہا = تہی;

	errno = 0;
	اگر (str == تہی || *str == '\0')
		واپس -(errno = EINVAL);
	*عدد = strtold(str, &انتہا);

	اگر (errno != 0)
		واپس -errno;
	اگر (str == انتہا || (انتہا && *انتہا))
		واپس -(errno = EINVAL);
	واپس 0;
}

طویل دوہرا strtold_or_err(ثابت حرف *str, ثابت حرف *errmesg)
{
	طویل دوہرا عدد = 0;

	اگر (ul_strtold(str, &عدد) == 0)
		واپس عدد;
	اگر (errno == ERANGE)
		err(STRTOXX_EXIT_رمز, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_رمز, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(ثابت حرف *str, ثابت حرف *errmesg)
{
	uintmax_t عدد;

	اگر (strtosize(str, &عدد) == 0)
		واپس عدد;

	اگر (errno)
		err(STRTOXX_EXIT_رمز, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_رمز, "%s: '%s'", errmesg, str);
}


خالی strtotimeval_or_err(ثابت حرف *str, ساخت timeval *tv, ثابت حرف *errmesg)
{
	طویل دوہرا user_داخلہ;

	user_داخلہ = strtold_or_err(str, errmesg);
	tv->tv_sec = (time_t) user_داخلہ;
	tv->tv_usec = (suseconds_t)((user_داخلہ - tv->tv_sec) * USEC_فی_SEC);
}

خالی strtotimespec_or_err(ثابت حرف *str, ساخت timespec *ts, ثابت حرف *errmesg)
{
	طویل دوہرا user_داخلہ;

	user_داخلہ = strtold_or_err(str, errmesg);
	ts->tv_sec = (time_t) user_داخلہ;
	ts->tv_nsec = (طویل)((user_داخلہ - ts->tv_sec) * NSEC_فی_SEC);
}

time_t strtotime_or_err(ثابت حرف *str, ثابت حرف *errmesg)
{
	int64_t user_داخلہ;

	user_داخلہ = strtos64_or_err(str, errmesg);
	واپس (time_t) user_داخلہ;
}

bool hyperlinkwanted(ثابت حرف *منوال)
{
	اگر (منوال && strcmp(منوال, "never") == 0)
		واپس false;

	اگر (منوال && strcmp(منوال, "always") == 0)
		واپس true;

	اگر (!منوال || strcmp(منوال, "auto") == 0)
		واپس isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), منوال);
}

bool annotationwanted(ثابت حرف *منوال)
{
	اگر (منوال && strcmp(منوال, "never") == 0)
		واپس false;

	اگر (منوال && strcmp(منوال, "always") == 0)
		واپس true;

	اگر (!منوال || strcmp(منوال, "auto") == 0)
		واپس isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), منوال);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
حرف *xstrmode(mode_t منوال, حرف *str)
{
	غیرعلامتی مختصر i = 0;

	اگر (S_ISDIR(منوال))
		str[i++] = 'd';
	ورنہ اگر (S_ISLNK(منوال))
		str[i++] = 'l';
	ورنہ اگر (S_ISCHR(منوال))
		str[i++] = 'c';
	ورنہ اگر (S_ISBLK(منوال))
		str[i++] = 'b';
	ورنہ اگر (S_ISSOCK(منوال))
		str[i++] = 's';
	ورنہ اگر (S_ISFIFO(منوال))
		str[i++] = 'p';
	ورنہ اگر (S_ISREG(منوال))
		str[i++] = '-';

	str[i++] = منوال & S_IRUSR ? 'r' : '-';
	str[i++] = منوال & S_IWUSR ? 'w' : '-';
	str[i++] = (منوال & S_ISUID
		? (منوال & S_IXUSR ? 's' : 'S')
		: (منوال & S_IXUSR ? 'x' : '-'));
	str[i++] = منوال & S_IRGRP ? 'r' : '-';
	str[i++] = منوال & S_IWGRP ? 'w' : '-';
	str[i++] = (منوال & S_ISGID
		? (منوال & S_IXGRP ? 's' : 'S')
		: (منوال & S_IXGRP ? 'x' : '-'));
	str[i++] = منوال & S_IROTH ? 'r' : '-';
	str[i++] = منوال & S_IWOTH ? 'w' : '-';
	str[i++] = (منوال & S_ISVTX
		? (منوال & S_IXOTH ? 't' : 'T')
		: (منوال & S_IXOTH ? 'x' : '-'));
	str[i] = '\0';

	واپس str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
ساکن صحیح get_exp(uint64_t n)
{
	صحیح shft;

	برائے (shft = 10; shft <= 60; shft += 10) {
		اگر (n < (1ULL << shft))
			توڑو;
	}
	واپس shft - 10;
}

حرف *سائز_مقصد_human_string(صحیح options, uint64_t بائٹیں)
{
	حرف buf[32];
	صحیح dec, قوت_نما;
	uint64_t frac;
	ثابت حرف *letters = "BKMGTPE";
	حرف suffix[حجم(" KiB")], *psuf = suffix;
	حرف c;

	اگر (options & سائز_SUFFIX_SPACE)
		*psuf++ = ' ';


	قوت_نما  = get_exp(بائٹیں);
	c    = *(letters + (قوت_نما ? قوت_نما / 10 : 0));
	dec  = قوت_نما ? بائٹیں / (1ULL << قوت_نما) : بائٹیں;
	frac = قوت_نما ? بائٹیں % (1ULL << قوت_نما) : 0;

	*psuf++ = c;

	اگر ((options & سائز_SUFFIX_3_LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/*باری*/
	اگر (frac) {
		/* get 3 digits after decimal point */
		اگر (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (قوت_نما - 10)) ;
		ورنہ
			frac = (frac * 1000) / (1ULL << (قوت_نما)) ;

		اگر (options & سائز_DECIMAL_2_DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} ورنہ {
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
		ساخت lconv ثابت *l = localeconv();
		حرف *dp = l ? l->decimal_نقطہ : تہی;
		صحیح len;

		اگر (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, حجم(buf), "%d%s%02" PRIu64, dec, dp, frac);
		اگر (len > 0 && (size_t) len < حجم(buf)) {
			/* remove potential extraneous zero */
			اگر (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, حجم(buf) - len);
		} ورنہ
			*buf = '\0';	/* snprintf error */
	} ورنہ
		snprintf(buf, حجم(buf), "%d%s", dec, suffix);

	واپس strdup(buf);
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
صحیح string_مقصد_idarray(ثابت حرف *list, صحیح ary[], size_t arysz,
			صحیح (name2id)(ثابت حرف *, size_t))
{
	ثابت حرف *begin = تہی, *p;
	size_t n = 0;

	اگر (!list || !*list || !ary || !arysz || !name2id)
		واپس -1;

	برائے (p = list; p && *p; p++) {
		ثابت حرف *انتہا = تہی;
		صحیح id;

		اگر (n >= arysz)
			واپس -2;
		اگر (!begin)
			begin = p;		/* begin of the column name */
		اگر (*p == ',')
			انتہا = p;		/* terminate the name */
		اگر (*(p + 1) == '\0')
			انتہا = p + 1;		/* end of string */
		اگر (!begin || !انتہا)
			جاری;
		اگر (انتہا <= begin)
			واپس -1;

		id = name2id(begin, انتہا - begin);
		اگر (id == -1)
			واپس -1;
		ary[ n++ ] = id;
		begin = تہی;
		اگر (انتہا && !*انتہا)
			توڑو;
	}
	واپس n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
صحیح string_جوڑو_مقصد_idarray(ثابت حرف *list, صحیح ary[], size_t arysz,
			size_t *ary_pos, صحیح (name2id)(ثابت حرف *, size_t))
{
	ثابت حرف *list_جوڑو;
	صحیح r;

	اگر (!list || !*list || !ary_pos || *ary_pos > arysz)
		واپس -1;

	اگر (list[0] == '+')
		list_جوڑو = &list[1];
	ورنہ {
		list_جوڑو = list;
		*ary_pos = 0;
	}

	r = string_مقصد_idarray(list_جوڑو, &ary[*ary_pos], arysz - *ary_pos, name2id);
	اگر (r > 0)
		*ary_pos += r;
	واپس r;
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
صحیح string_مقصد_bitarray(ثابت حرف *list,
		     حرف *ary,
		     صحیح (*name2bit)(ثابت حرف *, size_t),
		     size_t allow_range)
{
	ثابت حرف *begin = تہی, *p;

	اگر (!list || !name2bit || !ary)
		واپس -EINVAL;

	برائے (p = list; p && *p; p++) {
		ثابت حرف *انتہا = تہی;
		صحیح ثنائی_ہندسہ, سیٹ_زیریں = 0, سیٹ_higher = 0;

		اگر (!begin)
			begin = p;		/* begin of the level name */
		اگر (*p == ',')
			انتہا = p;		/* terminate the name */
		اگر (*(p + 1) == '\0')
			انتہا = p + 1;		/* end of string */
		اگر (!begin || !انتہا)
			جاری;
		اگر (انتہا <= begin)
			واپس -1;
		اگر (allow_range) {
			اگر (*(انتہا - 1) == '+') {
				انتہا--;
				سیٹ_زیریں = 1;
			} ورنہ اگر (*begin == '+') {
				begin++;
				سیٹ_higher = 1;
			}
		}

		ثنائی_ہندسہ = name2bit(begin, انتہا - begin);
		اگر (ثنائی_ہندسہ < 0)
			واپس ثنائی_ہندسہ;
		setbit(ary, ثنائی_ہندسہ);
		اگر (سیٹ_زیریں)
			جب (--ثنائی_ہندسہ >= 0)
				setbit(ary, ثنائی_ہندسہ);
		ورنہ اگر (سیٹ_higher)
			جب (++ثنائی_ہندسہ < (صحیح) allow_range)
				setbit(ary, ثنائی_ہندسہ);
		begin = تہی;
		اگر (انتہا && !*انتہا)
			توڑو;
	}
	واپس 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
صحیح string_مقصد_bitmask(ثابت حرف *list,
		     غیرعلامتی طویل *نقاب,
		     طویل (*name2flag)(ثابت حرف *, size_t))
{
	ثابت حرف *begin = تہی, *p;

	اگر (!list || !name2flag || !نقاب)
		واپس -EINVAL;

	برائے (p = list; p && *p; p++) {
		ثابت حرف *انتہا = تہی;
		طویل flag;

		اگر (!begin)
			begin = p;		/* begin of the level name */
		اگر (*p == ',')
			انتہا = p;		/* terminate the name */
		اگر (*(p + 1) == '\0')
			انتہا = p + 1;		/* end of string */
		اگر (!begin || !انتہا)
			جاری;
		اگر (انتہا <= begin)
			واپس -1;

		flag = name2flag(begin, انتہا - begin);
		اگر (flag < 0)
			واپس flag;	/* error */
		*نقاب |= flag;
		begin = تہی;
		اگر (انتہا && !*انتہا)
			توڑو;
	}
	واپس 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
صحیح ul_parse_range(ثابت حرف *str, صحیح *زیریں, صحیح *بالائی, صحیح def)
{
	حرف *انتہا = تہی;

	اگر (!str)
		واپس 0;

	*بالائی = *زیریں = def;
	errno = 0;

	اگر (*str == ':') {				/* <:N> */
		str++;
		*بالائی = strtol(str, &انتہا, 10);
		اگر (errno || !انتہا || *انتہا || انتہا == str)
			واپس -1;
	} ورنہ {
		*بالائی = *زیریں = strtol(str, &انتہا, 10);
		اگر (errno || !انتہا || انتہا == str)
			واپس -1;

		اگر (*انتہا == ':' && !*(انتہا + 1))		/* <M:> */
			*بالائی = def;
		ورنہ اگر (*انتہا == '-' || *انتہا == ':') {	/* <M:N> <M-N> */
			str = انتہا + 1;
			انتہا = تہی;
			errno = 0;
			*بالائی = strtol(str, &انتہا, 10);

			اگر (errno || !انتہا || *انتہا || انتہا == str)
				واپس -1;
		}
	}
	واپس 0;
}

ثابت حرف *ul_راستہ_اگلا_segment(ثابت حرف *str, size_t *sz)
{
	ثابت حرف *آغاز, *p;

	آغاز = str;
	*sz = 0;
	جب (آغاز && *آغاز == '/' && *(آغاز + 1) == '/')
		آغاز++;

	اگر (!آغاز || !*آغاز)
		واپس تہی;

	برائے (*sz = 1, p = آغاز + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	واپس آغاز;
}

صحیح streq_paths(ثابت حرف *a, ثابت حرف *b)
{
	جب (a && b) {
		size_t a_sz, b_sz;
		ثابت حرف *a_seg = ul_راستہ_اگلا_segment(a, &a_sz);
		ثابت حرف *b_seg = ul_راستہ_اگلا_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		اگر (a_sz + b_sz == 0)
			واپس 1;

		/* ignore trailing slash */
		اگر (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			واپس 1;

		اگر (!a_seg || !b_seg)
			توڑو;
		اگر (a_sz != b_sz || لڑی_موازنہ(a_seg, b_seg, a_sz) != 0)
			توڑو;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	واپس 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
صحیح ul_راستہ_is_within(ثابت حرف *راستہ, ثابت حرف *dir)
{
	اگر (!راستہ || !dir || !*راستہ || !*dir)
		واپس 0;

	جب (راستہ && dir) {
		size_t p_sz, d_sz;
		ثابت حرف *p_seg = ul_راستہ_اگلا_segment(راستہ, &p_sz);
		ثابت حرف *d_seg = ul_راستہ_اگلا_segment(dir, &d_sz);

		/* all @dir segments matched */
		اگر (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			واپس 1;

		اگر (!p_seg || !d_seg)
			توڑو;
		اگر (p_sz != d_sz || لڑی_موازنہ(p_seg, d_seg, p_sz) != 0)
			توڑو;

		راستہ = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	واپس 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
حرف *ul_strnconcat(ثابت حرف *s, ثابت حرف *suffix, size_t b)
{
	size_t a;
	حرف *r;

	اگر (!s && !suffix)
		واپس strdup("");
	اگر (!s)
		واپس strndup(suffix, b);
	اگر (!suffix)
		واپس strdup(s);

	أكد(s);
	أكد(suffix);

	a = لڑی_لمبائی(s);
	اگر (b > ((size_t) -1) - a)
		واپس تہی;

	r = حافظہ_دو(a + b + 1);
	اگر (!r)
		واپس تہی;

	حافظہ_نقل(r, s, a);
	حافظہ_نقل(r + a, suffix, b);
	r[a+b] = 0;

	واپس r;
}

/* concatenate two strings to a new string */
حرف *ul_strconcat(ثابت حرف *s, ثابت حرف *suffix)
{
	واپس ul_strnconcat(s, suffix, suffix ? لڑی_لمبائی(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
حرف *ul_strfconcat(ثابت حرف *s, ثابت حرف *format, ...)
{
	va_list ap;
	حرف *val, *res;
	صحیح sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	اگر (sz < 0)
		واپس تہی;

	res = ul_strnconcat(s, val, sz);
	آزاد(val);
	واپس res;
}

صحیح ul_strappend(حرف **a, ثابت حرف *b)
{
	size_t al, bl;
	حرف *tmp;

	اگر (!a)
		واپس -EINVAL;
	اگر (!b || !*b)
		واپس 0;
	اگر (!*a) {
		*a = strdup(b);
		واپس !*a ? -ENOMEM : 0;
	}

	al = لڑی_لمبائی(*a);
	bl = لڑی_لمبائی(b);

	tmp = realloc(*a, al + bl + 1);
	اگر (!tmp)
		واپس -ENOMEM;
	*a = tmp;
	حافظہ_نقل((*a) + al, b, bl + 1);
	واپس 0;
}

/* the hybrid version of strfconcat and strappend. */
صحیح strfappend(حرف **a, ثابت حرف *format, ...)
{
	va_list ap;
	صحیح res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	واپس res;
}

خارجی صحیح ul_strvfappend(حرف **a, ثابت حرف *format, va_list ap)
{
	حرف *val;
	صحیح sz;
	صحیح res;

	sz = vasprintf(&val, format, ap);
	اگر (sz < 0)
		واپس -errno;

	res = ul_strappend(a, val);
	آزاد(val);
	واپس res;
}

ساکن size_t strcspn_escaped(ثابت حرف *s, ثابت حرف *reject)
{
	صحیح escaped = 0;
	صحیح n;

	برائے (n=0; s[n]; n++) {
		اگر (escaped)
			escaped = 0;
		ورنہ اگر (s[n] == '\\')
			escaped = 1;
		ورنہ اگر (strchr(reject, s[n]))
			توڑو;
	}

	/* if s ends in \, return index of previous char */
	واپس n - escaped;
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
حرف *ul_strchr_escaped(ثابت حرف *s, صحیح c)
{
	حرف *p;
	صحیح esc = 0;

	برائے (p = (حرف *) s; p && *p; p++) {
		اگر (!esc && *p == '\\') {
			esc = 1;
			جاری;
		}
		اگر (*p == c && (!esc || c == '\\'))
			واپس p;
		esc = 0;
	}

	واپس تہی;
}

/* Split a string into words. */
ثابت حرف *ul_بانٹو(ثابت حرف **state, size_t *l, ثابت حرف *separator, صحیح quoted)
{
	ثابت حرف *current;

	current = *state;

	اگر (!*current) {
		أكد(**state == '\0');
		واپس تہی;
	}

	current += strspn(current, separator);
	اگر (!*current) {
		*state = current;
		واپس تہی;
	}

	اگر (quoted && strchr("\'\"", *current)) {
		حرف quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		اگر (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			واپس تہی;
		}
		*state = current++ + *l + 2;
	} ورنہ اگر (quoted) {
		*l = strcspn_escaped(current, separator);
		اگر (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			واپس تہی;
		}
		*state = current + *l;
	} ورنہ {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	واپس current;
}

/* Rewind file pointer forward to new line.  */
صحیح skip_fline(FILE *fp)
{
	صحیح ch;

	کرو {
		اگر ((ch = fgetc(fp)) == EOF)
			واپس 1;
		اگر (ch == '\n')
			واپس 0;
	} جب (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
صحیح ul_stralnumcmp(ثابت حرف *p1, ثابت حرف *p2)
{
	ثابت غیرعلامتی حرف *s1 = (ثابت غیرعلامتی حرف *) p1;
	ثابت غیرعلامتی حرف *s2 = (ثابت غیرعلامتی حرف *) p2;
	غیرعلامتی حرف c1, c2;

	کرو {
		کرو {
			c1 = (غیرعلامتی حرف) *s1++;
		} جب (c1 != '\0' && !حرف_یا_ہندسہ((غیرعلامتی صحیح) c1));

		کرو {
			c2 = (غیرعلامتی حرف) *s2++;
		} جب (c2 != '\0' && !حرف_یا_ہندسہ((غیرعلامتی صحیح) c2));

		اگر (c1 != '\0')
			c1 = tolower(c1);
		اگر (c2 != '\0')
			c2 = tolower(c2);
		اگر (c1 == '\0')
			واپس c1 - c2;
	} جب (c1 == c2);

	واپس c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
صحیح ul_optstr_اگلا(حرف **optstr, حرف **name, size_t *namesz,
		   حرف **قدر, size_t *valsz)
{
	صحیح open_quote = 0;
	حرف *آغاز = تہی, *stop = تہی, *p, *sep = تہی;
	حرف *optstr0;

	أكد(optstr);
	أكد(*optstr);

	optstr0 = *optstr;

	اگر (name)
		*name = تہی;
	اگر (namesz)
		*namesz = 0;
	اگر (قدر)
		*قدر = تہی;
	اگر (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	جب (optstr0 && *optstr0 == ',')
		optstr0++;

	برائے (p = optstr0; p && *p; p++) {
		اگر (!آغاز && *p == '=')
			واپس -EINVAL;
		اگر (!آغاز)
			آغاز = p;		/* beginning of the option item */
		اگر (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		اگر (open_quote)
			جاری;		/* still in quoted block */
		اگر (!sep && p > آغاز && *p == '=')
			sep = p;		/* name and value separator */
		اگر (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		ورنہ اگر (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		اگر (!آغاز || !stop)
			جاری;
		اگر (stop <= آغاز)
			واپس -EINVAL;

		اگر (name)
			*name = آغاز;
		اگر (namesz)
			*namesz = sep ? sep - آغاز : stop - آغاز;
		*optstr = *stop ? stop + 1 : stop;

		اگر (sep) {
			اگر (قدر)
				*قدر = sep + 1;
			اگر (valsz)
				*valsz = stop - sep - 1;
		}
		واپس 0;
	}

	واپس 1;				/* end of optstr */
}

صحیح ul_optstr_is_valid(ثابت حرف *optstr)
{
	صحیح rc;
	حرف *p = (حرف *) optstr;

	جب ((rc = ul_optstr_اگلا(&p, تہی, تہی, تہی, تہی)) == 0);
	واپس rc < 0 ? 0 : 1;
}

حرف *ul_optstr_get_قدر(ثابت حرف *optstr, ثابت حرف *کلید)
{
	size_t sz, namesz = 0, valsz = 0;
	حرف *name = تہی, *قدر = تہی;
	حرف *p = (حرف *) optstr;

	اگر (!optstr || !کلید || !*کلید)
		واپس تہی;

	sz = لڑی_لمبائی(کلید);
	جب (ul_optstr_اگلا(&p, &name, &namesz, &قدر, &valsz) == 0) {
		اگر (namesz != sz || !valsz)
			جاری;
		اگر (لڑی_موازنہ(name, کلید, namesz) == 0)
			واپس strndup(قدر, valsz);
	}
	واپس تہی;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(حرف *str)
{
	size_t len;
	حرف q;

	اگر (!str || !*str)
		واپس 0;

	len = لڑی_لمبائی(str);
	q = *str;

	اگر ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		واپس len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	واپس len - 2;
}

#اگر_تعریف_شدہ TEST_پروگرام_STRUTILS

ساخت testS {
	حرف *name;
	حرف *قدر;
};

ساکن صحیح test_strdup_مقصد_member(صحیح argc, حرف *argv[])
{
	ساخت testS *xx;

	اگر (argc < 3)
		واپس EXIT_FAILURE;

	xx = صفر_حافظہ_دو(1, حجم(*xx));
	اگر (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_مقصد_struct_member(xx, name, argv[1]);
	strdup_مقصد_struct_member(xx, قدر, argv[2]);

	اگر (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->قدر, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	لکھو("1: '%s', 2: '%s'\n", xx->name, xx->قدر);

	آزاد(xx->name);
	آزاد(xx->قدر);
	آزاد(xx);
	واپس EXIT_SUCCESS;
}

ساکن صحیح test_strutils_sizes(صحیح argc, حرف *argv[])
{
	uintmax_t سائز = 0;
	حرف *hum1, *hum2, *hum3;

	اگر (argc < 2)
		واپس EXIT_FAILURE;

	اگر (strtosize(argv[1], &سائز))
		errx(EXIT_FAILURE, "invalid size '%s' value", argv[1]);

	hum1 = سائز_مقصد_human_string(سائز_SUFFIX_1_LETTER, سائز);
	hum2 = سائز_مقصد_human_string(سائز_SUFFIX_3_LETTER |
				    سائز_SUFFIX_SPACE, سائز);
	hum3 = سائز_مقصد_human_string(سائز_SUFFIX_3_LETTER |
				    سائز_SUFFIX_SPACE |
				    سائز_DECIMAL_2_DIGITS, سائز);

	لکھو("%25s : %20ju : %8s : %12s : %13s\n", argv[1], سائز, hum1, hum2, hum3);
	آزاد(hum1);
	آزاد(hum2);
	آزاد(hum3);

	واپس EXIT_SUCCESS;
}

ساکن صحیح test_strutils_cmp_paths(صحیح argc, حرف *argv[])
{
	صحیح rc = streq_paths(argv[1], argv[2]);

	اگر (argc < 3)
		واپس EXIT_FAILURE;

	لکھو("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	واپس EXIT_SUCCESS;
}

ساکن صحیح test_strutils_راستہ_is_within(صحیح argc, حرف *argv[])
{
	صحیح rc;

	اگر (argc < 3)
		واپس EXIT_FAILURE;

	rc = ul_راستہ_is_within(argv[1], argv[2]);

	لکھو("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	واپس EXIT_SUCCESS;
}

ساکن صحیح test_strutils_normalize(صحیح argc, حرف *argv[])
{
	غیرعلامتی حرف *src, *dst, *org;
	size_t sz, len;

	اگر (argc < 2)
		واپس EXIT_FAILURE;

	org = (غیرعلامتی حرف *) strdup(argv[1]);
	src = (غیرعلامتی حرف *) strdup((حرف *) org);
	len = لڑی_لمبائی((حرف *) src);
	dst = حافظہ_دو(len + 1);

	اگر (!org || !src || !dst)
		جاؤ مکمل;

	/* two buffers */
	sz = __normalize_whitespace(src, len, dst, len + 1);
	لکھو("1: '%s' --> '%s' [sz=%zu]\n", src, dst, sz);

	/* one buffer */
	sz = normalize_whitespace(src);
	لکھو("2: '%s' --> '%s' [sz=%zu]\n", org, src, sz);

مکمل:
	آزاد(src);
	آزاد(dst);
	آزاد(org);

	واپس EXIT_SUCCESS;
}

ساکن صحیح test_strutils_cstrcasecmp(صحیح argc, حرف *argv[])
{
	حرف *a, *b;

	اگر (argc < 3)
		واپس EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	اگر (!a || !b)
		واپس EXIT_FAILURE;

	لکھو("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	لکھو("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	لکھو("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, لڑی_لمبائی(a)));

	واپس EXIT_SUCCESS;
}

صحیح مین(صحیح argc, حرف *argv[])
{
	اگر (argc == 3 && strcmp(argv[1], "--size") == 0) {
		واپس test_strutils_sizes(argc - 1, argv + 1);

	} ورنہ اگر (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		لکھو("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						تہی));
		واپس EXIT_SUCCESS;
	} ورنہ اگر (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		اگر (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		لکھو("'%s'-->%s\n", argv[2], val ? "true" : "false");
		واپس EXIT_SUCCESS;

	} ورنہ اگر (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		واپس test_strutils_cmp_paths(argc - 1, argv + 1);

	} ورنہ اگر (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		واپس test_strutils_راستہ_is_within(argc - 1, argv + 1);

	} ورنہ اگر (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		واپس test_strdup_مقصد_member(argc - 1, argv + 1);

	} ورنہ اگر  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		لکھو("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		واپس EXIT_SUCCESS;

	} ورنہ اگر (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		واپس test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} ورنہ اگر (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		واپس test_strutils_normalize(argc - 1, argv + 1);

	} ورنہ اگر (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		لکھو("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		واپس EXIT_SUCCESS;
	} ورنہ اگر (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		لکھو("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		واپس EXIT_SUCCESS;
	} ورنہ اگر (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		لکھو("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		واپس EXIT_SUCCESS;
	} ورنہ اگر (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		لکھو("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		واپس EXIT_SUCCESS;
	} ورنہ اگر (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		لکھو("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		واپس EXIT_SUCCESS;
	} ورنہ اگر (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		لکھو("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		واپس EXIT_SUCCESS;

	} ورنہ اگر (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		لکھو("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		واپس EXIT_SUCCESS;

	} ورنہ اگر (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		حرف *buf = "abc\0Y\0\0xyz\0X";
		حرف *انتہا = buf + 12;
		حرف *p = buf;

		کرو {
			لکھو("str: '%s'\n", p);
		} جب ((p = ul_اگلا_string(p, انتہا)));

		واپس EXIT_SUCCESS;

	} ورنہ اگر (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		حرف *name = تہی, *val = تہی;
		حرف *p = argv[2];
		صحیح rc;

		اگر (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		جب ((rc = ul_optstr_اگلا(&p, &name, &namesz, &val, &valsz)) == 0) {
			لکھو("'%.*s' : '%.*s'\n", (صحیح) namesz, name,
						   (صحیح) valsz, val);
		}
		اگر (rc == 1)
			واپس EXIT_SUCCESS;
	} ورنہ اگر (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		حرف *unquoted = strdup(argv[2]);

		اگر (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		لکھو("%s-->%s\n", argv[2], unquoted);
		آزاد(unquoted);
		واپس EXIT_SUCCESS;

	} ورنہ {
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
		باہر_نکلو(EXIT_FAILURE);
	}

	واپس EXIT_FAILURE;
}
#اگر_ختم /* TEST_PROGRAM_STRUTILS */
