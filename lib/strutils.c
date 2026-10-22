/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Karel Zak <kzak@redhat.com> [2010]
 *          Davidlohr Bueso <dave@gnu.org> [2010]
 */
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <inttypes.h>
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <strings.h>
#అన్తర్భావ <assert.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "cctype.h"
#అన్తర్భావ "nls.h"
#అన్తర్భావ "strutils.h"
#అన్తర్భావ "timeutils.h"
#అన్తర్భావ "bitops.h"
#అన్తర్భావ "pathnames.h"

స్థిర పూర్ణాంకం STRTOXX_EXIT_సంకేతం = EXIT_FAILURE;

శూన్య strutils_సమితి_exitcode(పూర్ణాంకం ex)
{
	STRTOXX_EXIT_సంకేతం = ex;
}

స్థిర పూర్ణాంకం do_scale_by_power (uintmax_t *x, పూర్ణాంకం పునాది, పూర్ణాంకం power)
{
	అయితే (power--) {
		ఒకవేళ (UINTMAX_MAX / పునాది < *x)
			తిరిగి -ERANGE;
		*x *= పునాది;
	}
	తిరిగి 0;
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
పూర్ణాంకం ul_parse_కొలత(స్థిరరాశి అక్షరం *str, uintmax_t *res, పూర్ణాంకం *power)
{
	స్థిరరాశి అక్షరం *p;
	అక్షరం *చివర;
	uintmax_t x, frac = 0;
	పూర్ణాంకం పునాది = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	స్థిర స్థిరరాశి అక్షరం *suf  = "KMGTPEZY";
	స్థిర స్థిరరాశి అక్షరం *suf2 = "kmgtpezy";
	స్థిరరాశి అక్షరం *sp;

	*res = 0;

	ఒకవేళ (!str || !*str) {
		rc = -EINVAL;
		వెళ్ళు err;
	}

	/* Only positive numbers are acceptable
	 *
	 * Note that this check is not perfect, it would be better to
	 * use lconv->negative_sign. But coreutils use the same solution,
	 * so it's probably good enough...
	 */
	p = str;
	అయితే (ఖాళీనా((గుర్తులేని అక్షరం) *p))
		p++;
	ఒకవేళ (*p == '-') {
		rc = -EINVAL;
		వెళ్ళు err;
	}

	errno = 0, చివర = ఖాళీ;
	x = strtoumax(str, &చివర, 0);

	ఒకవేళ (చివర == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		వెళ్ళు err;
	}
	ఒకవేళ (!చివర || !*చివర)
		వెళ్ళు పూర్తి;			/* without suffix */
	p = చివర;

	/*
	 * Check size suffixes
	 */
check_suffix:
	ఒకవేళ (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		పునాది = 1024;			/* XiB, 2^N */
	లేకపోతే ఒకవేళ ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		పునాది = 1000;			/* XB, 10^N */
	లేకపోతే ఒకవేళ (*(p + 1)) {
		నిర్మాణం lconv స్థిరరాశి *l = localeconv();
		స్థిరరాశి అక్షరం *dp = l ? l->decimal_బిందువు : ఖాళీ;
		size_t dpsz = dp ? అక్షరశ్రేణి_పొడవు(dp) : 0;

		ఒకవేళ (frac == 0 && *p && dp && అక్షరశ్రేణి_పోలిక(dp, p, dpsz) == 0) {
			స్థిరరాశి అక్షరం *fstr = p + dpsz;

			కొరకు (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			ఒకవేళ (అంకెనా(*fstr)) {
				errno = 0, చివర = ఖాళీ;
				frac = strtoumax(fstr, &చివర, 0);
				ఒకవేళ (చివర == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					వెళ్ళు err;
				}
			} లేకపోతే
				చివర = (అక్షరం *) p;

			ఒకవేళ (!చివర || !*చివర) {
				rc = -EINVAL;
				వెళ్ళు err;	/* without suffix, but with fractions */
			}
			p = చివర;
			వెళ్ళు check_suffix;
		}
		rc = -EINVAL;
		వెళ్ళు err;			/* unexpected suffix */
	}

	sp = strchr(suf, *p);
	ఒకవేళ (sp)
		pwr = (sp - suf) + 1;
	లేకపోతే {
		sp = strchr(suf2, *p);
		ఒకవేళ (sp)
			pwr = (sp - suf2) + 1;
		లేకపోతే {
			rc = -EINVAL;
			వెళ్ళు err;
		}
	}

	ఒకవేళ (power)
		*power = pwr;

	rc = do_scale_by_power(&x, పునాది, pwr);
	ఒకవేళ (rc)
		వెళ్ళు err;

	ఒకవేళ (frac && pwr) {
		పూర్ణాంకం i;
		uintmax_t frac_పునాది = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_పునాది, పునాది, pwr);
		ఒకవేళ (rc)
			వెళ్ళు err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		చేయి {
			frac_val = ((frac % 10) * frac_పునాది + frac_val) / 10;
			frac /= 10;
		} అయితే (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		కొరకు (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		ఒకవేళ (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			వెళ్ళు err;
		}
		x += frac_val;
	}
పూర్తి:
	*res = x;
err:
	ఒకవేళ (rc < 0)
		errno = -rc;
	తిరిగి rc;
}

పూర్ణాంకం strtosize(స్థిరరాశి అక్షరం *str, uintmax_t *res)
{
	తిరిగి ul_parse_కొలత(str, res, ఖాళీ);
}

పూర్ణాంకం isdigit_strend(స్థిరరాశి అక్షరం *str, స్థిరరాశి అక్షరం **చివర)
{
	స్థిరరాశి అక్షరం *p;

	కొరకు (p = str; p && *p && అంకెనా((గుర్తులేని అక్షరం) *p); p++);

	ఒకవేళ (చివర)
		*చివర = p;
	తిరిగి p && p > str && !*p;
}

పూర్ణాంకం isxdigit_strend(స్థిరరాశి అక్షరం *str, స్థిరరాశి అక్షరం **చివర)
{
	స్థిరరాశి అక్షరం *p;

	కొరకు (p = str; p && *p && isxdigit((గుర్తులేని అక్షరం) *p); p++);

	ఒకవేళ (చివర)
		*చివర = p;

	తిరిగి p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
పూర్ణాంకం ul_strtobool(స్థిరరాశి అక్షరం *str, bool *ఫలితం)
{
	స్థిర స్థిరరాశి అక్షరం *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	స్థిర స్థిరరాశి అక్షరం *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	ఒకవేళ (!str || !ఫలితం)
		తిరిగి -(errno = EINVAL);

	errno = 0;

	కొరకు (i = 0; i < ARRAY_కొలత(bool_true); i++) {
		ఒకవేళ (c_strcasecmp(str, bool_true[i]) == 0) {
			*ఫలితం = true;
			తిరిగి 0;
		}
	}

	కొరకు (i = 0; i < ARRAY_కొలత(bool_false); i++) {
		ఒకవేళ (c_strcasecmp(str, bool_false[i]) == 0) {
			*ఫలితం = false;
			తిరిగి 0;
		}
	}

	తిరిగి -(errno = EINVAL);
}

పూర్ణాంకం ul_parse_switch(స్థిరరాశి అక్షరం *arg, ...)
{
	స్థిరరాశి అక్షరం *a, *b;
	va_list ap;

	va_start(ap, arg);
	చేయి {
		a = va_arg(ap, అక్షరం *);
		ఒకవేళ (!a)
			ఆపు;
		b = va_arg(ap, అక్షరం *);
		ఒకవేళ (!b)
			ఆపు;

		ఒకవేళ (strcmp(arg, a) == 0) {
			va_end(ap);
			తిరిగి 1;
		}

		ఒకవేళ (strcmp(arg, b) == 0) {
			va_end(ap);
			తిరిగి 0;
		}
	} అయితే (1);
	va_end(ap);

	errx(STRTOXX_EXIT_సంకేతం, _("unsupported argument: %s"), arg);
}

#నిర్వచితం_కాకపోతే HAVE_MEMPCPY
శూన్య *mempcpy(శూన్య *పరిమిత dest, స్థిరరాశి శూన్య *పరిమిత src, size_t n)
{
	తిరిగి ((అక్షరం *)జ్ఞాపకం_నకలు(dest, src, n)) + n;
}
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే HAVE_STRNLEN
size_t strnlen(స్థిరరాశి అక్షరం *s, size_t maxlen)
{
	size_t i;

	కొరకు (i = 0; i < maxlen; i++) {
		ఒకవేళ (s[i] == '\0')
			తిరిగి i;
	}
	తిరిగి maxlen;
}
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే HAVE_STRNCHR
అక్షరం *strnchr(స్థిరరాశి అక్షరం *s, size_t maxlen, పూర్ణాంకం c)
{
	కొరకు (; maxlen-- && *s != '\0'; ++s)
		ఒకవేళ (*s == (అక్షరం)c)
			తిరిగి (అక్షరం *)s;
	తిరిగి ఖాళీ;
}
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే HAVE_STRNDUP
అక్షరం *strndup(స్థిరరాశి అక్షరం *s, size_t n)
{
	size_t len = strnlen(s, n);
	అక్షరం *new = జ్ఞాపకం_ఇవ్వు((len + 1) * పరిమాణం(అక్షరం));
	ఒకవేళ (!new)
		తిరిగి ఖాళీ;
	new[len] = '\0';
	తిరిగి (అక్షరం *) జ్ఞాపకం_నకలు(new, s, len);
}
#అయితే_ముగింపు

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
పూర్ణాంకం ul_strtos64(స్థిరరాశి అక్షరం *str, int64_t *అంకె, పూర్ణాంకం పునాది)
{
	అక్షరం *చివర = ఖాళీ;

	ఒకవేళ (str == ఖాళీ || *str == '\0')
		తిరిగి -(errno = EINVAL);

	errno = 0;
	*అంకె = (int64_t) strtoimax(str, &చివర, పునాది);

	ఒకవేళ (errno != 0)
		తిరిగి -errno;
	ఒకవేళ (str == చివర || (చివర && *చివర))
		తిరిగి -(errno = EINVAL);
	తిరిగి 0;
}

పూర్ణాంకం ul_strtou64(స్థిరరాశి అక్షరం *str, uint64_t *అంకె, పూర్ణాంకం పునాది)
{
	అక్షరం *చివర = ఖాళీ;
	int64_t tmp;

	ఒకవేళ (str == ఖాళీ || *str == '\0')
		తిరిగి -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &చివర, పునాది);
	ఒకవేళ (tmp < 0)
		errno = ERANGE;
	లేకపోతే {
		errno = 0;
		*అంకె = strtoumax(str, &చివర, పునాది);
	}

	ఒకవేళ (errno != 0)
		తిరిగి -errno;
	ఒకవేళ (str == చివర || (చివర && *చివర))
		తిరిగి -(errno = EINVAL);
	తిరిగి 0;
}

పూర్ణాంకం ul_strtos32(స్థిరరాశి అక్షరం *str, int32_t *అంకె, పూర్ణాంకం పునాది)
{
	int64_t tmp;
	పూర్ణాంకం rc;

	rc = ul_strtos64(str, &tmp, పునాది);
	ఒకవేళ (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	ఒకవేళ (rc == 0)
		*అంకె = (int32_t) tmp;
	తిరిగి rc;
}

పూర్ణాంకం ul_strtou32(స్థిరరాశి అక్షరం *str, uint32_t *అంకె, పూర్ణాంకం పునాది)
{
	uint64_t tmp;
	పూర్ణాంకం rc;

	rc = ul_strtou64(str, &tmp, పునాది);
	ఒకవేళ (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	ఒకవేళ (rc == 0)
		*అంకె = (uint32_t) tmp;
	తిరిగి rc;
}

పూర్ణాంకం ul_strtou16(స్థిరరాశి అక్షరం *str, uint16_t *అంకె, పూర్ణాంకం పునాది)
{
	uint64_t tmp;
	పూర్ణాంకం rc;

	rc = ul_strtou64(str, &tmp, పునాది);
	ఒకవేళ (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	ఒకవేళ (rc == 0)
		*అంకె = (uint16_t) tmp;
	తిరిగి rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(స్థిరరాశి అక్షరం *str, పూర్ణాంకం పునాది, స్థిరరాశి అక్షరం *errmesg,
			     int64_t దిగువ, int64_t పైకి)
{
	int64_t అంకె = 0;
	పూర్ణాంకం rc;

	rc = ul_strtos64(str, &అంకె, పునాది);
	ఒకవేళ (rc == 0 && ((దిగువ && అంకె < దిగువ) || (పైకి && అంకె > పైకి)))
		rc = -(errno = ERANGE);

	ఒకవేళ (rc) {
		ఒకవేళ (errno == ERANGE)
			err(STRTOXX_EXIT_సంకేతం, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_సంకేతం, "%s: '%s'", errmesg, str);
	}
	తిరిగి అంకె;
}

uint64_t str2unum_or_err(స్థిరరాశి అక్షరం *str, పూర్ణాంకం పునాది, స్థిరరాశి అక్షరం *errmesg, uint64_t పైకి)
{
	uint64_t అంకె = 0;
	పూర్ణాంకం rc;

	rc = ul_strtou64(str, &అంకె, పునాది);
	ఒకవేళ (rc == 0 && (పైకి && అంకె > పైకి))
		rc = -(errno = ERANGE);

	ఒకవేళ (rc) {
		ఒకవేళ (errno == ERANGE)
			err(STRTOXX_EXIT_సంకేతం, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_సంకేతం, "%s: '%s'", errmesg, str);
	}
	తిరిగి అంకె;
}

ద్విగుణ strtod_or_err(స్థిరరాశి అక్షరం *str, స్థిరరాశి అక్షరం *errmesg)
{
	ద్విగుణ అంకె;
	అక్షరం *చివర = ఖాళీ;

	errno = 0;
	ఒకవేళ (str == ఖాళీ || *str == '\0')
		వెళ్ళు err;
	అంకె = strtod(str, &చివర);

	ఒకవేళ (errno || str == చివర || (చివర && *చివర))
		వెళ్ళు err;

	తిరిగి అంకె;
err:
	ఒకవేళ (errno == ERANGE)
		err(STRTOXX_EXIT_సంకేతం, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_సంకేతం, "%s: '%s'", errmesg, str);
}

పూర్ణాంకం ul_strtold(స్థిరరాశి అక్షరం *str, దీర్ఘ ద్విగుణ *అంకె)
{
	అక్షరం *చివర = ఖాళీ;

	errno = 0;
	ఒకవేళ (str == ఖాళీ || *str == '\0')
		తిరిగి -(errno = EINVAL);
	*అంకె = strtold(str, &చివర);

	ఒకవేళ (errno != 0)
		తిరిగి -errno;
	ఒకవేళ (str == చివర || (చివర && *చివర))
		తిరిగి -(errno = EINVAL);
	తిరిగి 0;
}

దీర్ఘ ద్విగుణ strtold_or_err(స్థిరరాశి అక్షరం *str, స్థిరరాశి అక్షరం *errmesg)
{
	దీర్ఘ ద్విగుణ అంకె = 0;

	ఒకవేళ (ul_strtold(str, &అంకె) == 0)
		తిరిగి అంకె;
	ఒకవేళ (errno == ERANGE)
		err(STRTOXX_EXIT_సంకేతం, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_సంకేతం, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(స్థిరరాశి అక్షరం *str, స్థిరరాశి అక్షరం *errmesg)
{
	uintmax_t అంకె;

	ఒకవేళ (strtosize(str, &అంకె) == 0)
		తిరిగి అంకె;

	ఒకవేళ (errno)
		err(STRTOXX_EXIT_సంకేతం, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_సంకేతం, "%s: '%s'", errmesg, str);
}


శూన్య strtotimeval_or_err(స్థిరరాశి అక్షరం *str, నిర్మాణం timeval *tv, స్థిరరాశి అక్షరం *errmesg)
{
	దీర్ఘ ద్విగుణ user_ఆదానం;

	user_ఆదానం = strtold_or_err(str, errmesg);
	tv->tv_sec = (time_t) user_ఆదానం;
	tv->tv_usec = (suseconds_t)((user_ఆదానం - tv->tv_sec) * USEC_చొప్పున_SEC);
}

శూన్య strtotimespec_or_err(స్థిరరాశి అక్షరం *str, నిర్మాణం timespec *ts, స్థిరరాశి అక్షరం *errmesg)
{
	దీర్ఘ ద్విగుణ user_ఆదానం;

	user_ఆదానం = strtold_or_err(str, errmesg);
	ts->tv_sec = (time_t) user_ఆదానం;
	ts->tv_nsec = (దీర్ఘ)((user_ఆదానం - ts->tv_sec) * NSEC_చొప్పున_SEC);
}

time_t strtotime_or_err(స్థిరరాశి అక్షరం *str, స్థిరరాశి అక్షరం *errmesg)
{
	int64_t user_ఆదానం;

	user_ఆదానం = strtos64_or_err(str, errmesg);
	తిరిగి (time_t) user_ఆదానం;
}

bool hyperlinkwanted(స్థిరరాశి అక్షరం *బహుళకం)
{
	ఒకవేళ (బహుళకం && strcmp(బహుళకం, "never") == 0)
		తిరిగి false;

	ఒకవేళ (బహుళకం && strcmp(బహుళకం, "always") == 0)
		తిరిగి true;

	ఒకవేళ (!బహుళకం || strcmp(బహుళకం, "auto") == 0)
		తిరిగి isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), బహుళకం);
}

bool annotationwanted(స్థిరరాశి అక్షరం *బహుళకం)
{
	ఒకవేళ (బహుళకం && strcmp(బహుళకం, "never") == 0)
		తిరిగి false;

	ఒకవేళ (బహుళకం && strcmp(బహుళకం, "always") == 0)
		తిరిగి true;

	ఒకవేళ (!బహుళకం || strcmp(బహుళకం, "auto") == 0)
		తిరిగి isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), బహుళకం);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
అక్షరం *xstrmode(mode_t బహుళకం, అక్షరం *str)
{
	గుర్తులేని పొట్టి i = 0;

	ఒకవేళ (S_ISDIR(బహుళకం))
		str[i++] = 'd';
	లేకపోతే ఒకవేళ (S_ISLNK(బహుళకం))
		str[i++] = 'l';
	లేకపోతే ఒకవేళ (S_ISCHR(బహుళకం))
		str[i++] = 'c';
	లేకపోతే ఒకవేళ (S_ISBLK(బహుళకం))
		str[i++] = 'b';
	లేకపోతే ఒకవేళ (S_ISSOCK(బహుళకం))
		str[i++] = 's';
	లేకపోతే ఒకవేళ (S_ISFIFO(బహుళకం))
		str[i++] = 'p';
	లేకపోతే ఒకవేళ (S_ISREG(బహుళకం))
		str[i++] = '-';

	str[i++] = బహుళకం & S_IRUSR ? 'r' : '-';
	str[i++] = బహుళకం & S_IWUSR ? 'w' : '-';
	str[i++] = (బహుళకం & S_ISUID
		? (బహుళకం & S_IXUSR ? 's' : 'S')
		: (బహుళకం & S_IXUSR ? 'x' : '-'));
	str[i++] = బహుళకం & S_IRGRP ? 'r' : '-';
	str[i++] = బహుళకం & S_IWGRP ? 'w' : '-';
	str[i++] = (బహుళకం & S_ISGID
		? (బహుళకం & S_IXGRP ? 's' : 'S')
		: (బహుళకం & S_IXGRP ? 'x' : '-'));
	str[i++] = బహుళకం & S_IROTH ? 'r' : '-';
	str[i++] = బహుళకం & S_IWOTH ? 'w' : '-';
	str[i++] = (బహుళకం & S_ISVTX
		? (బహుళకం & S_IXOTH ? 't' : 'T')
		: (బహుళకం & S_IXOTH ? 'x' : '-'));
	str[i] = '\0';

	తిరిగి str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
స్థిర పూర్ణాంకం get_exp(uint64_t n)
{
	పూర్ణాంకం shft;

	కొరకు (shft = 10; shft <= 60; shft += 10) {
		ఒకవేళ (n < (1ULL << shft))
			ఆపు;
	}
	తిరిగి shft - 10;
}

అక్షరం *కొలత_గమ్యం_human_string(పూర్ణాంకం options, uint64_t బైట్లు)
{
	అక్షరం buf[32];
	పూర్ణాంకం dec, ఘాతాంకం;
	uint64_t frac;
	స్థిరరాశి అక్షరం *letters = "BKMGTPE";
	అక్షరం suffix[పరిమాణం(" KiB")], *psuf = suffix;
	అక్షరం c;

	ఒకవేళ (options & కొలత_SUFFIX_SPACE)
		*psuf++ = ' ';


	ఘాతాంకం  = get_exp(బైట్లు);
	c    = *(letters + (ఘాతాంకం ? ఘాతాంకం / 10 : 0));
	dec  = ఘాతాంకం ? బైట్లు / (1ULL << ఘాతాంకం) : బైట్లు;
	frac = ఘాతాంకం ? బైట్లు % (1ULL << ఘాతాంకం) : 0;

	*psuf++ = c;

	ఒకవేళ ((options & కొలత_SUFFIX_3_LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/*విడత*/
	ఒకవేళ (frac) {
		/* get 3 digits after decimal point */
		ఒకవేళ (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (ఘాతాంకం - 10)) ;
		లేకపోతే
			frac = (frac * 1000) / (1ULL << (ఘాతాంకం)) ;

		ఒకవేళ (options & కొలత_DECIMAL_2_DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} లేకపోతే {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		ఒకవేళ (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	ఒకవేళ (frac) {
		నిర్మాణం lconv స్థిరరాశి *l = localeconv();
		అక్షరం *dp = l ? l->decimal_బిందువు : ఖాళీ;
		పూర్ణాంకం len;

		ఒకవేళ (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, పరిమాణం(buf), "%d%s%02" PRIu64, dec, dp, frac);
		ఒకవేళ (len > 0 && (size_t) len < పరిమాణం(buf)) {
			/* remove potential extraneous zero */
			ఒకవేళ (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, పరిమాణం(buf) - len);
		} లేకపోతే
			*buf = '\0';	/* snprintf error */
	} లేకపోతే
		snprintf(buf, పరిమాణం(buf), "%d%s", dec, suffix);

	తిరిగి strdup(buf);
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
పూర్ణాంకం string_గమ్యం_idarray(స్థిరరాశి అక్షరం *list, పూర్ణాంకం ary[], size_t arysz,
			పూర్ణాంకం (name2id)(స్థిరరాశి అక్షరం *, size_t))
{
	స్థిరరాశి అక్షరం *begin = ఖాళీ, *p;
	size_t n = 0;

	ఒకవేళ (!list || !*list || !ary || !arysz || !name2id)
		తిరిగి -1;

	కొరకు (p = list; p && *p; p++) {
		స్థిరరాశి అక్షరం *చివర = ఖాళీ;
		పూర్ణాంకం id;

		ఒకవేళ (n >= arysz)
			తిరిగి -2;
		ఒకవేళ (!begin)
			begin = p;		/* begin of the column name */
		ఒకవేళ (*p == ',')
			చివర = p;		/* terminate the name */
		ఒకవేళ (*(p + 1) == '\0')
			చివర = p + 1;		/* end of string */
		ఒకవేళ (!begin || !చివర)
			కొనసాగు;
		ఒకవేళ (చివర <= begin)
			తిరిగి -1;

		id = name2id(begin, చివర - begin);
		ఒకవేళ (id == -1)
			తిరిగి -1;
		ary[ n++ ] = id;
		begin = ఖాళీ;
		ఒకవేళ (చివర && !*చివర)
			ఆపు;
	}
	తిరిగి n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
పూర్ణాంకం string_కూడు_గమ్యం_idarray(స్థిరరాశి అక్షరం *list, పూర్ణాంకం ary[], size_t arysz,
			size_t *ary_pos, పూర్ణాంకం (name2id)(స్థిరరాశి అక్షరం *, size_t))
{
	స్థిరరాశి అక్షరం *list_కూడు;
	పూర్ణాంకం r;

	ఒకవేళ (!list || !*list || !ary_pos || *ary_pos > arysz)
		తిరిగి -1;

	ఒకవేళ (list[0] == '+')
		list_కూడు = &list[1];
	లేకపోతే {
		list_కూడు = list;
		*ary_pos = 0;
	}

	r = string_గమ్యం_idarray(list_కూడు, &ary[*ary_pos], arysz - *ary_pos, name2id);
	ఒకవేళ (r > 0)
		*ary_pos += r;
	తిరిగి r;
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
పూర్ణాంకం string_గమ్యం_bitarray(స్థిరరాశి అక్షరం *list,
		     అక్షరం *ary,
		     పూర్ణాంకం (*name2bit)(స్థిరరాశి అక్షరం *, size_t),
		     size_t allow_range)
{
	స్థిరరాశి అక్షరం *begin = ఖాళీ, *p;

	ఒకవేళ (!list || !name2bit || !ary)
		తిరిగి -EINVAL;

	కొరకు (p = list; p && *p; p++) {
		స్థిరరాశి అక్షరం *చివర = ఖాళీ;
		పూర్ణాంకం బిట్టు, సమితి_క్రింది = 0, సమితి_higher = 0;

		ఒకవేళ (!begin)
			begin = p;		/* begin of the level name */
		ఒకవేళ (*p == ',')
			చివర = p;		/* terminate the name */
		ఒకవేళ (*(p + 1) == '\0')
			చివర = p + 1;		/* end of string */
		ఒకవేళ (!begin || !చివర)
			కొనసాగు;
		ఒకవేళ (చివర <= begin)
			తిరిగి -1;
		ఒకవేళ (allow_range) {
			ఒకవేళ (*(చివర - 1) == '+') {
				చివర--;
				సమితి_క్రింది = 1;
			} లేకపోతే ఒకవేళ (*begin == '+') {
				begin++;
				సమితి_higher = 1;
			}
		}

		బిట్టు = name2bit(begin, చివర - begin);
		ఒకవేళ (బిట్టు < 0)
			తిరిగి బిట్టు;
		setbit(ary, బిట్టు);
		ఒకవేళ (సమితి_క్రింది)
			అయితే (--బిట్టు >= 0)
				setbit(ary, బిట్టు);
		లేకపోతే ఒకవేళ (సమితి_higher)
			అయితే (++బిట్టు < (పూర్ణాంకం) allow_range)
				setbit(ary, బిట్టు);
		begin = ఖాళీ;
		ఒకవేళ (చివర && !*చివర)
			ఆపు;
	}
	తిరిగి 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
పూర్ణాంకం string_గమ్యం_bitmask(స్థిరరాశి అక్షరం *list,
		     గుర్తులేని దీర్ఘ *ముసుగు,
		     దీర్ఘ (*name2flag)(స్థిరరాశి అక్షరం *, size_t))
{
	స్థిరరాశి అక్షరం *begin = ఖాళీ, *p;

	ఒకవేళ (!list || !name2flag || !ముసుగు)
		తిరిగి -EINVAL;

	కొరకు (p = list; p && *p; p++) {
		స్థిరరాశి అక్షరం *చివర = ఖాళీ;
		దీర్ఘ flag;

		ఒకవేళ (!begin)
			begin = p;		/* begin of the level name */
		ఒకవేళ (*p == ',')
			చివర = p;		/* terminate the name */
		ఒకవేళ (*(p + 1) == '\0')
			చివర = p + 1;		/* end of string */
		ఒకవేళ (!begin || !చివర)
			కొనసాగు;
		ఒకవేళ (చివర <= begin)
			తిరిగి -1;

		flag = name2flag(begin, చివర - begin);
		ఒకవేళ (flag < 0)
			తిరిగి flag;	/* error */
		*ముసుగు |= flag;
		begin = ఖాళీ;
		ఒకవేళ (చివర && !*చివర)
			ఆపు;
	}
	తిరిగి 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
పూర్ణాంకం ul_parse_range(స్థిరరాశి అక్షరం *str, పూర్ణాంకం *క్రింది, పూర్ణాంకం *పై, పూర్ణాంకం def)
{
	అక్షరం *చివర = ఖాళీ;

	ఒకవేళ (!str)
		తిరిగి 0;

	*పై = *క్రింది = def;
	errno = 0;

	ఒకవేళ (*str == ':') {				/* <:N> */
		str++;
		*పై = strtol(str, &చివర, 10);
		ఒకవేళ (errno || !చివర || *చివర || చివర == str)
			తిరిగి -1;
	} లేకపోతే {
		*పై = *క్రింది = strtol(str, &చివర, 10);
		ఒకవేళ (errno || !చివర || చివర == str)
			తిరిగి -1;

		ఒకవేళ (*చివర == ':' && !*(చివర + 1))		/* <M:> */
			*పై = def;
		లేకపోతే ఒకవేళ (*చివర == '-' || *చివర == ':') {	/* <M:N> <M-N> */
			str = చివర + 1;
			చివర = ఖాళీ;
			errno = 0;
			*పై = strtol(str, &చివర, 10);

			ఒకవేళ (errno || !చివర || *చివర || చివర == str)
				తిరిగి -1;
		}
	}
	తిరిగి 0;
}

స్థిరరాశి అక్షరం *ul_మార్గం_తదుపరి_segment(స్థిరరాశి అక్షరం *str, size_t *sz)
{
	స్థిరరాశి అక్షరం *ప్రారంభం, *p;

	ప్రారంభం = str;
	*sz = 0;
	అయితే (ప్రారంభం && *ప్రారంభం == '/' && *(ప్రారంభం + 1) == '/')
		ప్రారంభం++;

	ఒకవేళ (!ప్రారంభం || !*ప్రారంభం)
		తిరిగి ఖాళీ;

	కొరకు (*sz = 1, p = ప్రారంభం + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	తిరిగి ప్రారంభం;
}

పూర్ణాంకం streq_paths(స్థిరరాశి అక్షరం *a, స్థిరరాశి అక్షరం *b)
{
	అయితే (a && b) {
		size_t a_sz, b_sz;
		స్థిరరాశి అక్షరం *a_seg = ul_మార్గం_తదుపరి_segment(a, &a_sz);
		స్థిరరాశి అక్షరం *b_seg = ul_మార్గం_తదుపరి_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		ఒకవేళ (a_sz + b_sz == 0)
			తిరిగి 1;

		/* ignore trailing slash */
		ఒకవేళ (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			తిరిగి 1;

		ఒకవేళ (!a_seg || !b_seg)
			ఆపు;
		ఒకవేళ (a_sz != b_sz || అక్షరశ్రేణి_పోలిక(a_seg, b_seg, a_sz) != 0)
			ఆపు;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	తిరిగి 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
పూర్ణాంకం ul_మార్గం_is_within(స్థిరరాశి అక్షరం *మార్గం, స్థిరరాశి అక్షరం *dir)
{
	ఒకవేళ (!మార్గం || !dir || !*మార్గం || !*dir)
		తిరిగి 0;

	అయితే (మార్గం && dir) {
		size_t p_sz, d_sz;
		స్థిరరాశి అక్షరం *p_seg = ul_మార్గం_తదుపరి_segment(మార్గం, &p_sz);
		స్థిరరాశి అక్షరం *d_seg = ul_మార్గం_తదుపరి_segment(dir, &d_sz);

		/* all @dir segments matched */
		ఒకవేళ (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			తిరిగి 1;

		ఒకవేళ (!p_seg || !d_seg)
			ఆపు;
		ఒకవేళ (p_sz != d_sz || అక్షరశ్రేణి_పోలిక(p_seg, d_seg, p_sz) != 0)
			ఆపు;

		మార్గం = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	తిరిగి 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
అక్షరం *ul_strnconcat(స్థిరరాశి అక్షరం *s, స్థిరరాశి అక్షరం *suffix, size_t b)
{
	size_t a;
	అక్షరం *r;

	ఒకవేళ (!s && !suffix)
		తిరిగి strdup("");
	ఒకవేళ (!s)
		తిరిగి strndup(suffix, b);
	ఒకవేళ (!suffix)
		తిరిగి strdup(s);

	ప్రతిజ్ఞా(s);
	ప్రతిజ్ఞా(suffix);

	a = అక్షరశ్రేణి_పొడవు(s);
	ఒకవేళ (b > ((size_t) -1) - a)
		తిరిగి ఖాళీ;

	r = జ్ఞాపకం_ఇవ్వు(a + b + 1);
	ఒకవేళ (!r)
		తిరిగి ఖాళీ;

	జ్ఞాపకం_నకలు(r, s, a);
	జ్ఞాపకం_నకలు(r + a, suffix, b);
	r[a+b] = 0;

	తిరిగి r;
}

/* concatenate two strings to a new string */
అక్షరం *ul_strconcat(స్థిరరాశి అక్షరం *s, స్థిరరాశి అక్షరం *suffix)
{
	తిరిగి ul_strnconcat(s, suffix, suffix ? అక్షరశ్రేణి_పొడవు(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
అక్షరం *ul_strfconcat(స్థిరరాశి అక్షరం *s, స్థిరరాశి అక్షరం *format, ...)
{
	va_list ap;
	అక్షరం *val, *res;
	పూర్ణాంకం sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	ఒకవేళ (sz < 0)
		తిరిగి ఖాళీ;

	res = ul_strnconcat(s, val, sz);
	విడుదల(val);
	తిరిగి res;
}

పూర్ణాంకం ul_strappend(అక్షరం **a, స్థిరరాశి అక్షరం *b)
{
	size_t al, bl;
	అక్షరం *tmp;

	ఒకవేళ (!a)
		తిరిగి -EINVAL;
	ఒకవేళ (!b || !*b)
		తిరిగి 0;
	ఒకవేళ (!*a) {
		*a = strdup(b);
		తిరిగి !*a ? -ENOMEM : 0;
	}

	al = అక్షరశ్రేణి_పొడవు(*a);
	bl = అక్షరశ్రేణి_పొడవు(b);

	tmp = realloc(*a, al + bl + 1);
	ఒకవేళ (!tmp)
		తిరిగి -ENOMEM;
	*a = tmp;
	జ్ఞాపకం_నకలు((*a) + al, b, bl + 1);
	తిరిగి 0;
}

/* the hybrid version of strfconcat and strappend. */
పూర్ణాంకం strfappend(అక్షరం **a, స్థిరరాశి అక్షరం *format, ...)
{
	va_list ap;
	పూర్ణాంకం res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	తిరిగి res;
}

బాహ్య పూర్ణాంకం ul_strvfappend(అక్షరం **a, స్థిరరాశి అక్షరం *format, va_list ap)
{
	అక్షరం *val;
	పూర్ణాంకం sz;
	పూర్ణాంకం res;

	sz = vasprintf(&val, format, ap);
	ఒకవేళ (sz < 0)
		తిరిగి -errno;

	res = ul_strappend(a, val);
	విడుదల(val);
	తిరిగి res;
}

స్థిర size_t strcspn_escaped(స్థిరరాశి అక్షరం *s, స్థిరరాశి అక్షరం *reject)
{
	పూర్ణాంకం escaped = 0;
	పూర్ణాంకం n;

	కొరకు (n=0; s[n]; n++) {
		ఒకవేళ (escaped)
			escaped = 0;
		లేకపోతే ఒకవేళ (s[n] == '\\')
			escaped = 1;
		లేకపోతే ఒకవేళ (strchr(reject, s[n]))
			ఆపు;
	}

	/* if s ends in \, return index of previous char */
	తిరిగి n - escaped;
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
అక్షరం *ul_strchr_escaped(స్థిరరాశి అక్షరం *s, పూర్ణాంకం c)
{
	అక్షరం *p;
	పూర్ణాంకం esc = 0;

	కొరకు (p = (అక్షరం *) s; p && *p; p++) {
		ఒకవేళ (!esc && *p == '\\') {
			esc = 1;
			కొనసాగు;
		}
		ఒకవేళ (*p == c && (!esc || c == '\\'))
			తిరిగి p;
		esc = 0;
	}

	తిరిగి ఖాళీ;
}

/* Split a string into words. */
స్థిరరాశి అక్షరం *ul_విభజించు(స్థిరరాశి అక్షరం **state, size_t *l, స్థిరరాశి అక్షరం *separator, పూర్ణాంకం quoted)
{
	స్థిరరాశి అక్షరం *current;

	current = *state;

	ఒకవేళ (!*current) {
		ప్రతిజ్ఞా(**state == '\0');
		తిరిగి ఖాళీ;
	}

	current += strspn(current, separator);
	ఒకవేళ (!*current) {
		*state = current;
		తిరిగి ఖాళీ;
	}

	ఒకవేళ (quoted && strchr("\'\"", *current)) {
		అక్షరం quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		ఒకవేళ (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			తిరిగి ఖాళీ;
		}
		*state = current++ + *l + 2;
	} లేకపోతే ఒకవేళ (quoted) {
		*l = strcspn_escaped(current, separator);
		ఒకవేళ (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			తిరిగి ఖాళీ;
		}
		*state = current + *l;
	} లేకపోతే {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	తిరిగి current;
}

/* Rewind file pointer forward to new line.  */
పూర్ణాంకం skip_fline(FILE *fp)
{
	పూర్ణాంకం ch;

	చేయి {
		ఒకవేళ ((ch = fgetc(fp)) == EOF)
			తిరిగి 1;
		ఒకవేళ (ch == '\n')
			తిరిగి 0;
	} అయితే (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
పూర్ణాంకం ul_stralnumcmp(స్థిరరాశి అక్షరం *p1, స్థిరరాశి అక్షరం *p2)
{
	స్థిరరాశి గుర్తులేని అక్షరం *s1 = (స్థిరరాశి గుర్తులేని అక్షరం *) p1;
	స్థిరరాశి గుర్తులేని అక్షరం *s2 = (స్థిరరాశి గుర్తులేని అక్షరం *) p2;
	గుర్తులేని అక్షరం c1, c2;

	చేయి {
		చేయి {
			c1 = (గుర్తులేని అక్షరం) *s1++;
		} అయితే (c1 != '\0' && !అక్షరమా_అంకెనా((గుర్తులేని పూర్ణాంకం) c1));

		చేయి {
			c2 = (గుర్తులేని అక్షరం) *s2++;
		} అయితే (c2 != '\0' && !అక్షరమా_అంకెనా((గుర్తులేని పూర్ణాంకం) c2));

		ఒకవేళ (c1 != '\0')
			c1 = tolower(c1);
		ఒకవేళ (c2 != '\0')
			c2 = tolower(c2);
		ఒకవేళ (c1 == '\0')
			తిరిగి c1 - c2;
	} అయితే (c1 == c2);

	తిరిగి c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
పూర్ణాంకం ul_optstr_తదుపరి(అక్షరం **optstr, అక్షరం **name, size_t *namesz,
		   అక్షరం **విలువ, size_t *valsz)
{
	పూర్ణాంకం open_quote = 0;
	అక్షరం *ప్రారంభం = ఖాళీ, *stop = ఖాళీ, *p, *sep = ఖాళీ;
	అక్షరం *optstr0;

	ప్రతిజ్ఞా(optstr);
	ప్రతిజ్ఞా(*optstr);

	optstr0 = *optstr;

	ఒకవేళ (name)
		*name = ఖాళీ;
	ఒకవేళ (namesz)
		*namesz = 0;
	ఒకవేళ (విలువ)
		*విలువ = ఖాళీ;
	ఒకవేళ (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	అయితే (optstr0 && *optstr0 == ',')
		optstr0++;

	కొరకు (p = optstr0; p && *p; p++) {
		ఒకవేళ (!ప్రారంభం && *p == '=')
			తిరిగి -EINVAL;
		ఒకవేళ (!ప్రారంభం)
			ప్రారంభం = p;		/* beginning of the option item */
		ఒకవేళ (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		ఒకవేళ (open_quote)
			కొనసాగు;		/* still in quoted block */
		ఒకవేళ (!sep && p > ప్రారంభం && *p == '=')
			sep = p;		/* name and value separator */
		ఒకవేళ (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		లేకపోతే ఒకవేళ (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		ఒకవేళ (!ప్రారంభం || !stop)
			కొనసాగు;
		ఒకవేళ (stop <= ప్రారంభం)
			తిరిగి -EINVAL;

		ఒకవేళ (name)
			*name = ప్రారంభం;
		ఒకవేళ (namesz)
			*namesz = sep ? sep - ప్రారంభం : stop - ప్రారంభం;
		*optstr = *stop ? stop + 1 : stop;

		ఒకవేళ (sep) {
			ఒకవేళ (విలువ)
				*విలువ = sep + 1;
			ఒకవేళ (valsz)
				*valsz = stop - sep - 1;
		}
		తిరిగి 0;
	}

	తిరిగి 1;				/* end of optstr */
}

పూర్ణాంకం ul_optstr_is_valid(స్థిరరాశి అక్షరం *optstr)
{
	పూర్ణాంకం rc;
	అక్షరం *p = (అక్షరం *) optstr;

	అయితే ((rc = ul_optstr_తదుపరి(&p, ఖాళీ, ఖాళీ, ఖాళీ, ఖాళీ)) == 0);
	తిరిగి rc < 0 ? 0 : 1;
}

అక్షరం *ul_optstr_get_విలువ(స్థిరరాశి అక్షరం *optstr, స్థిరరాశి అక్షరం *కీలకం)
{
	size_t sz, namesz = 0, valsz = 0;
	అక్షరం *name = ఖాళీ, *విలువ = ఖాళీ;
	అక్షరం *p = (అక్షరం *) optstr;

	ఒకవేళ (!optstr || !కీలకం || !*కీలకం)
		తిరిగి ఖాళీ;

	sz = అక్షరశ్రేణి_పొడవు(కీలకం);
	అయితే (ul_optstr_తదుపరి(&p, &name, &namesz, &విలువ, &valsz) == 0) {
		ఒకవేళ (namesz != sz || !valsz)
			కొనసాగు;
		ఒకవేళ (అక్షరశ్రేణి_పోలిక(name, కీలకం, namesz) == 0)
			తిరిగి strndup(విలువ, valsz);
	}
	తిరిగి ఖాళీ;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(అక్షరం *str)
{
	size_t len;
	అక్షరం q;

	ఒకవేళ (!str || !*str)
		తిరిగి 0;

	len = అక్షరశ్రేణి_పొడవు(str);
	q = *str;

	ఒకవేళ ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		తిరిగి len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	తిరిగి len - 2;
}

#నిర్వచితమైతే TEST_కార్యక్రమం_STRUTILS

నిర్మాణం testS {
	అక్షరం *name;
	అక్షరం *విలువ;
};

స్థిర పూర్ణాంకం test_strdup_గమ్యం_member(పూర్ణాంకం argc, అక్షరం *argv[])
{
	నిర్మాణం testS *xx;

	ఒకవేళ (argc < 3)
		తిరిగి EXIT_FAILURE;

	xx = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*xx));
	ఒకవేళ (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_గమ్యం_struct_member(xx, name, argv[1]);
	strdup_గమ్యం_struct_member(xx, విలువ, argv[2]);

	ఒకవేళ (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->విలువ, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	రాయి("1: '%s', 2: '%s'\n", xx->name, xx->విలువ);

	విడుదల(xx->name);
	విడుదల(xx->విలువ);
	విడుదల(xx);
	తిరిగి EXIT_SUCCESS;
}

స్థిర పూర్ణాంకం test_strutils_sizes(పూర్ణాంకం argc, అక్షరం *argv[])
{
	uintmax_t కొలత = 0;
	అక్షరం *hum1, *hum2, *hum3;

	ఒకవేళ (argc < 2)
		తిరిగి EXIT_FAILURE;

	ఒకవేళ (strtosize(argv[1], &కొలత))
		errx(EXIT_FAILURE, "invalid size '%s' value", argv[1]);

	hum1 = కొలత_గమ్యం_human_string(కొలత_SUFFIX_1_LETTER, కొలత);
	hum2 = కొలత_గమ్యం_human_string(కొలత_SUFFIX_3_LETTER |
				    కొలత_SUFFIX_SPACE, కొలత);
	hum3 = కొలత_గమ్యం_human_string(కొలత_SUFFIX_3_LETTER |
				    కొలత_SUFFIX_SPACE |
				    కొలత_DECIMAL_2_DIGITS, కొలత);

	రాయి("%25s : %20ju : %8s : %12s : %13s\n", argv[1], కొలత, hum1, hum2, hum3);
	విడుదల(hum1);
	విడుదల(hum2);
	విడుదల(hum3);

	తిరిగి EXIT_SUCCESS;
}

స్థిర పూర్ణాంకం test_strutils_cmp_paths(పూర్ణాంకం argc, అక్షరం *argv[])
{
	పూర్ణాంకం rc = streq_paths(argv[1], argv[2]);

	ఒకవేళ (argc < 3)
		తిరిగి EXIT_FAILURE;

	రాయి("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	తిరిగి EXIT_SUCCESS;
}

స్థిర పూర్ణాంకం test_strutils_మార్గం_is_within(పూర్ణాంకం argc, అక్షరం *argv[])
{
	పూర్ణాంకం rc;

	ఒకవేళ (argc < 3)
		తిరిగి EXIT_FAILURE;

	rc = ul_మార్గం_is_within(argv[1], argv[2]);

	రాయి("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	తిరిగి EXIT_SUCCESS;
}

స్థిర పూర్ణాంకం test_strutils_normalize(పూర్ణాంకం argc, అక్షరం *argv[])
{
	గుర్తులేని అక్షరం *src, *dst, *org;
	size_t sz, len;

	ఒకవేళ (argc < 2)
		తిరిగి EXIT_FAILURE;

	org = (గుర్తులేని అక్షరం *) strdup(argv[1]);
	src = (గుర్తులేని అక్షరం *) strdup((అక్షరం *) org);
	len = అక్షరశ్రేణి_పొడవు((అక్షరం *) src);
	dst = జ్ఞాపకం_ఇవ్వు(len + 1);

	ఒకవేళ (!org || !src || !dst)
		వెళ్ళు పూర్తి;

	/* two buffers */
	sz = __normalize_whitespace(src, len, dst, len + 1);
	రాయి("1: '%s' --> '%s' [sz=%zu]\n", src, dst, sz);

	/* one buffer */
	sz = normalize_whitespace(src);
	రాయి("2: '%s' --> '%s' [sz=%zu]\n", org, src, sz);

పూర్తి:
	విడుదల(src);
	విడుదల(dst);
	విడుదల(org);

	తిరిగి EXIT_SUCCESS;
}

స్థిర పూర్ణాంకం test_strutils_cstrcasecmp(పూర్ణాంకం argc, అక్షరం *argv[])
{
	అక్షరం *a, *b;

	ఒకవేళ (argc < 3)
		తిరిగి EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	ఒకవేళ (!a || !b)
		తిరిగి EXIT_FAILURE;

	రాయి("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	రాయి("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	రాయి("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, అక్షరశ్రేణి_పొడవు(a)));

	తిరిగి EXIT_SUCCESS;
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[])
{
	ఒకవేళ (argc == 3 && strcmp(argv[1], "--size") == 0) {
		తిరిగి test_strutils_sizes(argc - 1, argv + 1);

	} లేకపోతే ఒకవేళ (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		రాయి("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						ఖాళీ));
		తిరిగి EXIT_SUCCESS;
	} లేకపోతే ఒకవేళ (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		ఒకవేళ (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		రాయి("'%s'-->%s\n", argv[2], val ? "true" : "false");
		తిరిగి EXIT_SUCCESS;

	} లేకపోతే ఒకవేళ (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		తిరిగి test_strutils_cmp_paths(argc - 1, argv + 1);

	} లేకపోతే ఒకవేళ (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		తిరిగి test_strutils_మార్గం_is_within(argc - 1, argv + 1);

	} లేకపోతే ఒకవేళ (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		తిరిగి test_strdup_గమ్యం_member(argc - 1, argv + 1);

	} లేకపోతే ఒకవేళ  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		రాయి("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		తిరిగి EXIT_SUCCESS;

	} లేకపోతే ఒకవేళ (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		తిరిగి test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} లేకపోతే ఒకవేళ (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		తిరిగి test_strutils_normalize(argc - 1, argv + 1);

	} లేకపోతే ఒకవేళ (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		రాయి("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		తిరిగి EXIT_SUCCESS;
	} లేకపోతే ఒకవేళ (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		రాయి("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		తిరిగి EXIT_SUCCESS;
	} లేకపోతే ఒకవేళ (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		రాయి("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		తిరిగి EXIT_SUCCESS;
	} లేకపోతే ఒకవేళ (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		రాయి("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		తిరిగి EXIT_SUCCESS;
	} లేకపోతే ఒకవేళ (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		రాయి("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		తిరిగి EXIT_SUCCESS;
	} లేకపోతే ఒకవేళ (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		రాయి("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		తిరిగి EXIT_SUCCESS;

	} లేకపోతే ఒకవేళ (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		రాయి("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		తిరిగి EXIT_SUCCESS;

	} లేకపోతే ఒకవేళ (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		అక్షరం *buf = "abc\0Y\0\0xyz\0X";
		అక్షరం *చివర = buf + 12;
		అక్షరం *p = buf;

		చేయి {
			రాయి("str: '%s'\n", p);
		} అయితే ((p = ul_తదుపరి_string(p, చివర)));

		తిరిగి EXIT_SUCCESS;

	} లేకపోతే ఒకవేళ (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		అక్షరం *name = ఖాళీ, *val = ఖాళీ;
		అక్షరం *p = argv[2];
		పూర్ణాంకం rc;

		ఒకవేళ (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		అయితే ((rc = ul_optstr_తదుపరి(&p, &name, &namesz, &val, &valsz)) == 0) {
			రాయి("'%.*s' : '%.*s'\n", (పూర్ణాంకం) namesz, name,
						   (పూర్ణాంకం) valsz, val);
		}
		ఒకవేళ (rc == 1)
			తిరిగి EXIT_SUCCESS;
	} లేకపోతే ఒకవేళ (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		అక్షరం *unquoted = strdup(argv[2]);

		ఒకవేళ (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		రాయి("%s-->%s\n", argv[2], unquoted);
		విడుదల(unquoted);
		తిరిగి EXIT_SUCCESS;

	} లేకపోతే {
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
		నిష్క్రమించు(EXIT_FAILURE);
	}

	తిరిగి EXIT_FAILURE;
}
#అయితే_ముగింపు /* TEST_PROGRAM_STRUTILS */
