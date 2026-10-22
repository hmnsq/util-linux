/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Karel Zak <kzak@redhat.com> [2010]
 *          Davidlohr Bueso <dave@gnu.org> [2010]
 */
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <inttypes.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>
#अन्तर्भाव <assert.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "cctype.h"
#अन्तर्भाव "nls.h"
#अन्तर्भाव "strutils.h"
#अन्तर्भाव "timeutils.h"
#अन्तर्भाव "bitops.h"
#अन्तर्भाव "pathnames.h"

स्थिर पूर्णाङ्क STRTOXX_EXIT_संहिता = EXIT_FAILURE;

शून्य strutils_समुच्चयः_exitcode(पूर्णाङ्क ex)
{
	STRTOXX_EXIT_संहिता = ex;
}

स्थिर पूर्णाङ्क do_scale_by_power (uintmax_t *x, पूर्णाङ्क आधारः, पूर्णाङ्क power)
{
	यावत् (power--) {
		यदि (UINTMAX_MAX / आधारः < *x)
			प्रत्यागम -ERANGE;
		*x *= आधारः;
	}
	प्रत्यागम 0;
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
पूर्णाङ्क ul_parse_आकार(ध्रुव वर्ण *str, uintmax_t *res, पूर्णाङ्क *power)
{
	ध्रुव वर्ण *p;
	वर्ण *अन्तः;
	uintmax_t x, frac = 0;
	पूर्णाङ्क आधारः = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	स्थिर ध्रुव वर्ण *suf  = "KMGTPEZY";
	स्थिर ध्रुव वर्ण *suf2 = "kmgtpezy";
	ध्रुव वर्ण *sp;

	*res = 0;

	यदि (!str || !*str) {
		rc = -EINVAL;
		गच्छ err;
	}

	/* Only positive numbers are acceptable
	 *
	 * Note that this check is not perfect, it would be better to
	 * use lconv->negative_sign. But coreutils use the same solution,
	 * so it's probably good enough...
	 */
	p = str;
	यावत् (रिक्तम्_वा((अचिह्नित वर्ण) *p))
		p++;
	यदि (*p == '-') {
		rc = -EINVAL;
		गच्छ err;
	}

	errno = 0, अन्तः = रिक्त;
	x = strtoumax(str, &अन्तः, 0);

	यदि (अन्तः == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		गच्छ err;
	}
	यदि (!अन्तः || !*अन्तः)
		गच्छ समाप्तम्;			/* without suffix */
	p = अन्तः;

	/*
	 * Check size suffixes
	 */
check_suffix:
	यदि (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		आधारः = 1024;			/* XiB, 2^N */
	अन्यथा यदि ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		आधारः = 1000;			/* XB, 10^N */
	अन्यथा यदि (*(p + 1)) {
		संरचना lconv ध्रुव *l = localeconv();
		ध्रुव वर्ण *dp = l ? l->decimal_point : रिक्त;
		size_t dpsz = dp ? वाक्यदैर्घ्यम्(dp) : 0;

		यदि (frac == 0 && *p && dp && वाक्ये_तुलय(dp, p, dpsz) == 0) {
			ध्रुव वर्ण *fstr = p + dpsz;

			प्रति (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			यदि (अङ्कः_वा(*fstr)) {
				errno = 0, अन्तः = रिक्त;
				frac = strtoumax(fstr, &अन्तः, 0);
				यदि (अन्तः == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					गच्छ err;
				}
			} अन्यथा
				अन्तः = (वर्ण *) p;

			यदि (!अन्तः || !*अन्तः) {
				rc = -EINVAL;
				गच्छ err;	/* without suffix, but with fractions */
			}
			p = अन्तः;
			गच्छ check_suffix;
		}
		rc = -EINVAL;
		गच्छ err;			/* unexpected suffix */
	}

	sp = strchr(suf, *p);
	यदि (sp)
		pwr = (sp - suf) + 1;
	अन्यथा {
		sp = strchr(suf2, *p);
		यदि (sp)
			pwr = (sp - suf2) + 1;
		अन्यथा {
			rc = -EINVAL;
			गच्छ err;
		}
	}

	यदि (power)
		*power = pwr;

	rc = do_scale_by_power(&x, आधारः, pwr);
	यदि (rc)
		गच्छ err;

	यदि (frac && pwr) {
		पूर्णाङ्क i;
		uintmax_t frac_आधारः = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_आधारः, आधारः, pwr);
		यदि (rc)
			गच्छ err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		कुरु {
			frac_val = ((frac % 10) * frac_आधारः + frac_val) / 10;
			frac /= 10;
		} यावत् (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		प्रति (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		यदि (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			गच्छ err;
		}
		x += frac_val;
	}
समाप्तम्:
	*res = x;
err:
	यदि (rc < 0)
		errno = -rc;
	प्रत्यागम rc;
}

पूर्णाङ्क strtosize(ध्रुव वर्ण *str, uintmax_t *res)
{
	प्रत्यागम ul_parse_आकार(str, res, रिक्त);
}

पूर्णाङ्क isdigit_strend(ध्रुव वर्ण *str, ध्रुव वर्ण **अन्तः)
{
	ध्रुव वर्ण *p;

	प्रति (p = str; p && *p && अङ्कः_वा((अचिह्नित वर्ण) *p); p++);

	यदि (अन्तः)
		*अन्तः = p;
	प्रत्यागम p && p > str && !*p;
}

पूर्णाङ्क isxdigit_strend(ध्रुव वर्ण *str, ध्रुव वर्ण **अन्तः)
{
	ध्रुव वर्ण *p;

	प्रति (p = str; p && *p && isxdigit((अचिह्नित वर्ण) *p); p++);

	यदि (अन्तः)
		*अन्तः = p;

	प्रत्यागम p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
पूर्णाङ्क ul_strtobool(ध्रुव वर्ण *str, bool *परिणाम)
{
	स्थिर ध्रुव वर्ण *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	स्थिर ध्रुव वर्ण *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	यदि (!str || !परिणाम)
		प्रत्यागम -(errno = EINVAL);

	errno = 0;

	प्रति (i = 0; i < ARRAY_आकार(bool_true); i++) {
		यदि (c_strcasecmp(str, bool_true[i]) == 0) {
			*परिणाम = true;
			प्रत्यागम 0;
		}
	}

	प्रति (i = 0; i < ARRAY_आकार(bool_false); i++) {
		यदि (c_strcasecmp(str, bool_false[i]) == 0) {
			*परिणाम = false;
			प्रत्यागम 0;
		}
	}

	प्रत्यागम -(errno = EINVAL);
}

पूर्णाङ्क ul_parse_switch(ध्रुव वर्ण *arg, ...)
{
	ध्रुव वर्ण *a, *b;
	va_list ap;

	va_start(ap, arg);
	कुरु {
		a = va_arg(ap, वर्ण *);
		यदि (!a)
			भङ्ग;
		b = va_arg(ap, वर्ण *);
		यदि (!b)
			भङ्ग;

		यदि (वाक्यं_तुलय(arg, a) == 0) {
			va_end(ap);
			प्रत्यागम 1;
		}

		यदि (वाक्यं_तुलय(arg, b) == 0) {
			va_end(ap);
			प्रत्यागम 0;
		}
	} यावत् (1);
	va_end(ap);

	errx(STRTOXX_EXIT_संहिता, _("unsupported argument: %s"), arg);
}

#यदि_अपरिभाषितम् HAVE_MEMPCPY
शून्य *mempcpy(शून्य *सीमित dest, ध्रुव शून्य *सीमित src, size_t n)
{
	प्रत्यागम ((वर्ण *)स्मृतिं_प्रतिलिख(dest, src, n)) + n;
}
#यदि_अन्तः

#यदि_अपरिभाषितम् HAVE_STRNLEN
size_t strnlen(ध्रुव वर्ण *s, size_t maxlen)
{
	size_t i;

	प्रति (i = 0; i < maxlen; i++) {
		यदि (s[i] == '\0')
			प्रत्यागम i;
	}
	प्रत्यागम maxlen;
}
#यदि_अन्तः

#यदि_अपरिभाषितम् HAVE_STRNCHR
वर्ण *strnchr(ध्रुव वर्ण *s, size_t maxlen, पूर्णाङ्क c)
{
	प्रति (; maxlen-- && *s != '\0'; ++s)
		यदि (*s == (वर्ण)c)
			प्रत्यागम (वर्ण *)s;
	प्रत्यागम रिक्त;
}
#यदि_अन्तः

#यदि_अपरिभाषितम् HAVE_STRNDUP
वर्ण *strndup(ध्रुव वर्ण *s, size_t n)
{
	size_t len = strnlen(s, n);
	वर्ण *new = स्मृतिं_देहि((len + 1) * परिमाण(वर्ण));
	यदि (!new)
		प्रत्यागम रिक्त;
	new[len] = '\0';
	प्रत्यागम (वर्ण *) स्मृतिं_प्रतिलिख(new, s, len);
}
#यदि_अन्तः

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
पूर्णाङ्क ul_strtos64(ध्रुव वर्ण *str, int64_t *अङ्कः, पूर्णाङ्क आधारः)
{
	वर्ण *अन्तः = रिक्त;

	यदि (str == रिक्त || *str == '\0')
		प्रत्यागम -(errno = EINVAL);

	errno = 0;
	*अङ्कः = (int64_t) strtoimax(str, &अन्तः, आधारः);

	यदि (errno != 0)
		प्रत्यागम -errno;
	यदि (str == अन्तः || (अन्तः && *अन्तः))
		प्रत्यागम -(errno = EINVAL);
	प्रत्यागम 0;
}

पूर्णाङ्क ul_strtou64(ध्रुव वर्ण *str, uint64_t *अङ्कः, पूर्णाङ्क आधारः)
{
	वर्ण *अन्तः = रिक्त;
	int64_t tmp;

	यदि (str == रिक्त || *str == '\0')
		प्रत्यागम -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &अन्तः, आधारः);
	यदि (tmp < 0)
		errno = ERANGE;
	अन्यथा {
		errno = 0;
		*अङ्कः = strtoumax(str, &अन्तः, आधारः);
	}

	यदि (errno != 0)
		प्रत्यागम -errno;
	यदि (str == अन्तः || (अन्तः && *अन्तः))
		प्रत्यागम -(errno = EINVAL);
	प्रत्यागम 0;
}

पूर्णाङ्क ul_strtos32(ध्रुव वर्ण *str, int32_t *अङ्कः, पूर्णाङ्क आधारः)
{
	int64_t tmp;
	पूर्णाङ्क rc;

	rc = ul_strtos64(str, &tmp, आधारः);
	यदि (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	यदि (rc == 0)
		*अङ्कः = (int32_t) tmp;
	प्रत्यागम rc;
}

पूर्णाङ्क ul_strtou32(ध्रुव वर्ण *str, uint32_t *अङ्कः, पूर्णाङ्क आधारः)
{
	uint64_t tmp;
	पूर्णाङ्क rc;

	rc = ul_strtou64(str, &tmp, आधारः);
	यदि (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	यदि (rc == 0)
		*अङ्कः = (uint32_t) tmp;
	प्रत्यागम rc;
}

पूर्णाङ्क ul_strtou16(ध्रुव वर्ण *str, uint16_t *अङ्कः, पूर्णाङ्क आधारः)
{
	uint64_t tmp;
	पूर्णाङ्क rc;

	rc = ul_strtou64(str, &tmp, आधारः);
	यदि (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	यदि (rc == 0)
		*अङ्कः = (uint16_t) tmp;
	प्रत्यागम rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(ध्रुव वर्ण *str, पूर्णाङ्क आधारः, ध्रुव वर्ण *errmesg,
			     int64_t नीचः, int64_t ऊर्ध्वम्)
{
	int64_t अङ्कः = 0;
	पूर्णाङ्क rc;

	rc = ul_strtos64(str, &अङ्कः, आधारः);
	यदि (rc == 0 && ((नीचः && अङ्कः < नीचः) || (ऊर्ध्वम् && अङ्कः > ऊर्ध्वम्)))
		rc = -(errno = ERANGE);

	यदि (rc) {
		यदि (errno == ERANGE)
			err(STRTOXX_EXIT_संहिता, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_संहिता, "%s: '%s'", errmesg, str);
	}
	प्रत्यागम अङ्कः;
}

uint64_t str2unum_or_err(ध्रुव वर्ण *str, पूर्णाङ्क आधारः, ध्रुव वर्ण *errmesg, uint64_t ऊर्ध्वम्)
{
	uint64_t अङ्कः = 0;
	पूर्णाङ्क rc;

	rc = ul_strtou64(str, &अङ्कः, आधारः);
	यदि (rc == 0 && (ऊर्ध्वम् && अङ्कः > ऊर्ध्वम्))
		rc = -(errno = ERANGE);

	यदि (rc) {
		यदि (errno == ERANGE)
			err(STRTOXX_EXIT_संहिता, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_संहिता, "%s: '%s'", errmesg, str);
	}
	प्रत्यागम अङ्कः;
}

द्विगुण strtod_or_err(ध्रुव वर्ण *str, ध्रुव वर्ण *errmesg)
{
	द्विगुण अङ्कः;
	वर्ण *अन्तः = रिक्त;

	errno = 0;
	यदि (str == रिक्त || *str == '\0')
		गच्छ err;
	अङ्कः = strtod(str, &अन्तः);

	यदि (errno || str == अन्तः || (अन्तः && *अन्तः))
		गच्छ err;

	प्रत्यागम अङ्कः;
err:
	यदि (errno == ERANGE)
		err(STRTOXX_EXIT_संहिता, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_संहिता, "%s: '%s'", errmesg, str);
}

पूर्णाङ्क ul_strtold(ध्रुव वर्ण *str, दीर्घ द्विगुण *अङ्कः)
{
	वर्ण *अन्तः = रिक्त;

	errno = 0;
	यदि (str == रिक्त || *str == '\0')
		प्रत्यागम -(errno = EINVAL);
	*अङ्कः = strtold(str, &अन्तः);

	यदि (errno != 0)
		प्रत्यागम -errno;
	यदि (str == अन्तः || (अन्तः && *अन्तः))
		प्रत्यागम -(errno = EINVAL);
	प्रत्यागम 0;
}

दीर्घ द्विगुण strtold_or_err(ध्रुव वर्ण *str, ध्रुव वर्ण *errmesg)
{
	दीर्घ द्विगुण अङ्कः = 0;

	यदि (ul_strtold(str, &अङ्कः) == 0)
		प्रत्यागम अङ्कः;
	यदि (errno == ERANGE)
		err(STRTOXX_EXIT_संहिता, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_संहिता, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(ध्रुव वर्ण *str, ध्रुव वर्ण *errmesg)
{
	uintmax_t अङ्कः;

	यदि (strtosize(str, &अङ्कः) == 0)
		प्रत्यागम अङ्कः;

	यदि (errno)
		err(STRTOXX_EXIT_संहिता, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_संहिता, "%s: '%s'", errmesg, str);
}


शून्य strtotimeval_or_err(ध्रुव वर्ण *str, संरचना timeval *tv, ध्रुव वर्ण *errmesg)
{
	दीर्घ द्विगुण user_निवेशः;

	user_निवेशः = strtold_or_err(str, errmesg);
	tv->tv_sec = (time_t) user_निवेशः;
	tv->tv_usec = (suseconds_t)((user_निवेशः - tv->tv_sec) * USEC_प्रत्येकम्_SEC);
}

शून्य strtotimespec_or_err(ध्रुव वर्ण *str, संरचना timespec *ts, ध्रुव वर्ण *errmesg)
{
	दीर्घ द्विगुण user_निवेशः;

	user_निवेशः = strtold_or_err(str, errmesg);
	ts->tv_sec = (time_t) user_निवेशः;
	ts->tv_nsec = (दीर्घ)((user_निवेशः - ts->tv_sec) * NSEC_प्रत्येकम्_SEC);
}

time_t strtotime_or_err(ध्रुव वर्ण *str, ध्रुव वर्ण *errmesg)
{
	int64_t user_निवेशः;

	user_निवेशः = strtos64_or_err(str, errmesg);
	प्रत्यागम (time_t) user_निवेशः;
}

bool hyperlinkwanted(ध्रुव वर्ण *बहुलकम्)
{
	यदि (बहुलकम् && वाक्यं_तुलय(बहुलकम्, "never") == 0)
		प्रत्यागम false;

	यदि (बहुलकम् && वाक्यं_तुलय(बहुलकम्, "always") == 0)
		प्रत्यागम true;

	यदि (!बहुलकम् || वाक्यं_तुलय(बहुलकम्, "auto") == 0)
		प्रत्यागम isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), बहुलकम्);
}

bool annotationwanted(ध्रुव वर्ण *बहुलकम्)
{
	यदि (बहुलकम् && वाक्यं_तुलय(बहुलकम्, "never") == 0)
		प्रत्यागम false;

	यदि (बहुलकम् && वाक्यं_तुलय(बहुलकम्, "always") == 0)
		प्रत्यागम true;

	यदि (!बहुलकम् || वाक्यं_तुलय(बहुलकम्, "auto") == 0)
		प्रत्यागम isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), बहुलकम्);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
वर्ण *xstrmode(mode_t बहुलकम्, वर्ण *str)
{
	अचिह्नित ह्रस्व i = 0;

	यदि (S_ISDIR(बहुलकम्))
		str[i++] = 'd';
	अन्यथा यदि (S_ISLNK(बहुलकम्))
		str[i++] = 'l';
	अन्यथा यदि (S_ISCHR(बहुलकम्))
		str[i++] = 'c';
	अन्यथा यदि (S_ISBLK(बहुलकम्))
		str[i++] = 'b';
	अन्यथा यदि (S_ISSOCK(बहुलकम्))
		str[i++] = 's';
	अन्यथा यदि (S_ISFIFO(बहुलकम्))
		str[i++] = 'p';
	अन्यथा यदि (S_ISREG(बहुलकम्))
		str[i++] = '-';

	str[i++] = बहुलकम् & S_IRUSR ? 'r' : '-';
	str[i++] = बहुलकम् & S_IWUSR ? 'w' : '-';
	str[i++] = (बहुलकम् & S_ISUID
		? (बहुलकम् & S_IXUSR ? 's' : 'S')
		: (बहुलकम् & S_IXUSR ? 'x' : '-'));
	str[i++] = बहुलकम् & S_IRGRP ? 'r' : '-';
	str[i++] = बहुलकम् & S_IWGRP ? 'w' : '-';
	str[i++] = (बहुलकम् & S_ISGID
		? (बहुलकम् & S_IXGRP ? 's' : 'S')
		: (बहुलकम् & S_IXGRP ? 'x' : '-'));
	str[i++] = बहुलकम् & S_IROTH ? 'r' : '-';
	str[i++] = बहुलकम् & S_IWOTH ? 'w' : '-';
	str[i++] = (बहुलकम् & S_ISVTX
		? (बहुलकम् & S_IXOTH ? 't' : 'T')
		: (बहुलकम् & S_IXOTH ? 'x' : '-'));
	str[i] = '\0';

	प्रत्यागम str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
स्थिर पूर्णाङ्क get_exp(uint64_t n)
{
	पूर्णाङ्क shft;

	प्रति (shft = 10; shft <= 60; shft += 10) {
		यदि (n < (1ULL << shft))
			भङ्ग;
	}
	प्रत्यागम shft - 10;
}

वर्ण *आकार_गन्तव्यम्_human_string(पूर्णाङ्क options, uint64_t अष्टकानि)
{
	वर्ण buf[32];
	पूर्णाङ्क dec, घातांकः;
	uint64_t frac;
	ध्रुव वर्ण *letters = "BKMGTPE";
	वर्ण suffix[परिमाण(" KiB")], *psuf = suffix;
	वर्ण c;

	यदि (options & आकार_SUFFIX_SPACE)
		*psuf++ = ' ';


	घातांकः  = get_exp(अष्टकानि);
	c    = *(letters + (घातांकः ? घातांकः / 10 : 0));
	dec  = घातांकः ? अष्टकानि / (1ULL << घातांकः) : अष्टकानि;
	frac = घातांकः ? अष्टकानि % (1ULL << घातांकः) : 0;

	*psuf++ = c;

	यदि ((options & आकार_SUFFIX_3_LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/*आवर्तनम्*/
	यदि (frac) {
		/* get 3 digits after decimal point */
		यदि (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (घातांकः - 10)) ;
		अन्यथा
			frac = (frac * 1000) / (1ULL << (घातांकः)) ;

		यदि (options & आकार_DECIMAL_2_DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} अन्यथा {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		यदि (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	यदि (frac) {
		संरचना lconv ध्रुव *l = localeconv();
		वर्ण *dp = l ? l->decimal_point : रिक्त;
		पूर्णाङ्क len;

		यदि (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, परिमाण(buf), "%d%s%02" PRIu64, dec, dp, frac);
		यदि (len > 0 && (size_t) len < परिमाण(buf)) {
			/* remove potential extraneous zero */
			यदि (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, परिमाण(buf) - len);
		} अन्यथा
			*buf = '\0';	/* snprintf error */
	} अन्यथा
		snprintf(buf, परिमाण(buf), "%d%s", dec, suffix);

	प्रत्यागम strdup(buf);
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
पूर्णाङ्क string_गन्तव्यम्_idarray(ध्रुव वर्ण *list, पूर्णाङ्क ary[], size_t arysz,
			पूर्णाङ्क (name2id)(ध्रुव वर्ण *, size_t))
{
	ध्रुव वर्ण *begin = रिक्त, *p;
	size_t n = 0;

	यदि (!list || !*list || !ary || !arysz || !name2id)
		प्रत्यागम -1;

	प्रति (p = list; p && *p; p++) {
		ध्रुव वर्ण *अन्तः = रिक्त;
		पूर्णाङ्क id;

		यदि (n >= arysz)
			प्रत्यागम -2;
		यदि (!begin)
			begin = p;		/* begin of the column name */
		यदि (*p == ',')
			अन्तः = p;		/* terminate the name */
		यदि (*(p + 1) == '\0')
			अन्तः = p + 1;		/* end of string */
		यदि (!begin || !अन्तः)
			अनुवर्त;
		यदि (अन्तः <= begin)
			प्रत्यागम -1;

		id = name2id(begin, अन्तः - begin);
		यदि (id == -1)
			प्रत्यागम -1;
		ary[ n++ ] = id;
		begin = रिक्त;
		यदि (अन्तः && !*अन्तः)
			भङ्ग;
	}
	प्रत्यागम n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
पूर्णाङ्क string_सङ्कलय_गन्तव्यम्_idarray(ध्रुव वर्ण *list, पूर्णाङ्क ary[], size_t arysz,
			size_t *ary_pos, पूर्णाङ्क (name2id)(ध्रुव वर्ण *, size_t))
{
	ध्रुव वर्ण *list_सङ्कलय;
	पूर्णाङ्क r;

	यदि (!list || !*list || !ary_pos || *ary_pos > arysz)
		प्रत्यागम -1;

	यदि (list[0] == '+')
		list_सङ्कलय = &list[1];
	अन्यथा {
		list_सङ्कलय = list;
		*ary_pos = 0;
	}

	r = string_गन्तव्यम्_idarray(list_सङ्कलय, &ary[*ary_pos], arysz - *ary_pos, name2id);
	यदि (r > 0)
		*ary_pos += r;
	प्रत्यागम r;
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
पूर्णाङ्क string_गन्तव्यम्_bitarray(ध्रुव वर्ण *list,
		     वर्ण *ary,
		     पूर्णाङ्क (*name2bit)(ध्रुव वर्ण *, size_t),
		     size_t allow_range)
{
	ध्रुव वर्ण *begin = रिक्त, *p;

	यदि (!list || !name2bit || !ary)
		प्रत्यागम -EINVAL;

	प्रति (p = list; p && *p; p++) {
		ध्रुव वर्ण *अन्तः = रिक्त;
		पूर्णाङ्क bit, समुच्चयः_अधस्तनम् = 0, समुच्चयः_higher = 0;

		यदि (!begin)
			begin = p;		/* begin of the level name */
		यदि (*p == ',')
			अन्तः = p;		/* terminate the name */
		यदि (*(p + 1) == '\0')
			अन्तः = p + 1;		/* end of string */
		यदि (!begin || !अन्तः)
			अनुवर्त;
		यदि (अन्तः <= begin)
			प्रत्यागम -1;
		यदि (allow_range) {
			यदि (*(अन्तः - 1) == '+') {
				अन्तः--;
				समुच्चयः_अधस्तनम् = 1;
			} अन्यथा यदि (*begin == '+') {
				begin++;
				समुच्चयः_higher = 1;
			}
		}

		bit = name2bit(begin, अन्तः - begin);
		यदि (bit < 0)
			प्रत्यागम bit;
		setbit(ary, bit);
		यदि (समुच्चयः_अधस्तनम्)
			यावत् (--bit >= 0)
				setbit(ary, bit);
		अन्यथा यदि (समुच्चयः_higher)
			यावत् (++bit < (पूर्णाङ्क) allow_range)
				setbit(ary, bit);
		begin = रिक्त;
		यदि (अन्तः && !*अन्तः)
			भङ्ग;
	}
	प्रत्यागम 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
पूर्णाङ्क string_गन्तव्यम्_bitmask(ध्रुव वर्ण *list,
		     अचिह्नित दीर्घ *आवरणम्,
		     दीर्घ (*name2flag)(ध्रुव वर्ण *, size_t))
{
	ध्रुव वर्ण *begin = रिक्त, *p;

	यदि (!list || !name2flag || !आवरणम्)
		प्रत्यागम -EINVAL;

	प्रति (p = list; p && *p; p++) {
		ध्रुव वर्ण *अन्तः = रिक्त;
		दीर्घ flag;

		यदि (!begin)
			begin = p;		/* begin of the level name */
		यदि (*p == ',')
			अन्तः = p;		/* terminate the name */
		यदि (*(p + 1) == '\0')
			अन्तः = p + 1;		/* end of string */
		यदि (!begin || !अन्तः)
			अनुवर्त;
		यदि (अन्तः <= begin)
			प्रत्यागम -1;

		flag = name2flag(begin, अन्तः - begin);
		यदि (flag < 0)
			प्रत्यागम flag;	/* error */
		*आवरणम् |= flag;
		begin = रिक्त;
		यदि (अन्तः && !*अन्तः)
			भङ्ग;
	}
	प्रत्यागम 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
पूर्णाङ्क ul_parse_range(ध्रुव वर्ण *str, पूर्णाङ्क *अधस्तनम्, पूर्णाङ्क *ऊर्ध्वस्थम्, पूर्णाङ्क def)
{
	वर्ण *अन्तः = रिक्त;

	यदि (!str)
		प्रत्यागम 0;

	*ऊर्ध्वस्थम् = *अधस्तनम् = def;
	errno = 0;

	यदि (*str == ':') {				/* <:N> */
		str++;
		*ऊर्ध्वस्थम् = strtol(str, &अन्तः, 10);
		यदि (errno || !अन्तः || *अन्तः || अन्तः == str)
			प्रत्यागम -1;
	} अन्यथा {
		*ऊर्ध्वस्थम् = *अधस्तनम् = strtol(str, &अन्तः, 10);
		यदि (errno || !अन्तः || अन्तः == str)
			प्रत्यागम -1;

		यदि (*अन्तः == ':' && !*(अन्तः + 1))		/* <M:> */
			*ऊर्ध्वस्थम् = def;
		अन्यथा यदि (*अन्तः == '-' || *अन्तः == ':') {	/* <M:N> <M-N> */
			str = अन्तः + 1;
			अन्तः = रिक्त;
			errno = 0;
			*ऊर्ध्वस्थम् = strtol(str, &अन्तः, 10);

			यदि (errno || !अन्तः || *अन्तः || अन्तः == str)
				प्रत्यागम -1;
		}
	}
	प्रत्यागम 0;
}

ध्रुव वर्ण *ul_मार्गः_अग्र_segment(ध्रुव वर्ण *str, size_t *sz)
{
	ध्रुव वर्ण *start, *p;

	start = str;
	*sz = 0;
	यावत् (start && *start == '/' && *(start + 1) == '/')
		start++;

	यदि (!start || !*start)
		प्रत्यागम रिक्त;

	प्रति (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	प्रत्यागम start;
}

पूर्णाङ्क streq_paths(ध्रुव वर्ण *a, ध्रुव वर्ण *b)
{
	यावत् (a && b) {
		size_t a_sz, b_sz;
		ध्रुव वर्ण *a_seg = ul_मार्गः_अग्र_segment(a, &a_sz);
		ध्रुव वर्ण *b_seg = ul_मार्गः_अग्र_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		यदि (a_sz + b_sz == 0)
			प्रत्यागम 1;

		/* ignore trailing slash */
		यदि (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			प्रत्यागम 1;

		यदि (!a_seg || !b_seg)
			भङ्ग;
		यदि (a_sz != b_sz || वाक्ये_तुलय(a_seg, b_seg, a_sz) != 0)
			भङ्ग;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	प्रत्यागम 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
पूर्णाङ्क ul_मार्गः_is_within(ध्रुव वर्ण *मार्गः, ध्रुव वर्ण *dir)
{
	यदि (!मार्गः || !dir || !*मार्गः || !*dir)
		प्रत्यागम 0;

	यावत् (मार्गः && dir) {
		size_t p_sz, d_sz;
		ध्रुव वर्ण *p_seg = ul_मार्गः_अग्र_segment(मार्गः, &p_sz);
		ध्रुव वर्ण *d_seg = ul_मार्गः_अग्र_segment(dir, &d_sz);

		/* all @dir segments matched */
		यदि (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			प्रत्यागम 1;

		यदि (!p_seg || !d_seg)
			भङ्ग;
		यदि (p_sz != d_sz || वाक्ये_तुलय(p_seg, d_seg, p_sz) != 0)
			भङ्ग;

		मार्गः = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	प्रत्यागम 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
वर्ण *ul_strnconcat(ध्रुव वर्ण *s, ध्रुव वर्ण *suffix, size_t b)
{
	size_t a;
	वर्ण *r;

	यदि (!s && !suffix)
		प्रत्यागम strdup("");
	यदि (!s)
		प्रत्यागम strndup(suffix, b);
	यदि (!suffix)
		प्रत्यागम strdup(s);

	प्रतिज्ञा(s);
	प्रतिज्ञा(suffix);

	a = वाक्यदैर्घ्यम्(s);
	यदि (b > ((size_t) -1) - a)
		प्रत्यागम रिक्त;

	r = स्मृतिं_देहि(a + b + 1);
	यदि (!r)
		प्रत्यागम रिक्त;

	स्मृतिं_प्रतिलिख(r, s, a);
	स्मृतिं_प्रतिलिख(r + a, suffix, b);
	r[a+b] = 0;

	प्रत्यागम r;
}

/* concatenate two strings to a new string */
वर्ण *ul_strconcat(ध्रुव वर्ण *s, ध्रुव वर्ण *suffix)
{
	प्रत्यागम ul_strnconcat(s, suffix, suffix ? वाक्यदैर्घ्यम्(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
वर्ण *ul_strfconcat(ध्रुव वर्ण *s, ध्रुव वर्ण *format, ...)
{
	va_list ap;
	वर्ण *val, *res;
	पूर्णाङ्क sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	यदि (sz < 0)
		प्रत्यागम रिक्त;

	res = ul_strnconcat(s, val, sz);
	मुक्त(val);
	प्रत्यागम res;
}

पूर्णाङ्क ul_strappend(वर्ण **a, ध्रुव वर्ण *b)
{
	size_t al, bl;
	वर्ण *tmp;

	यदि (!a)
		प्रत्यागम -EINVAL;
	यदि (!b || !*b)
		प्रत्यागम 0;
	यदि (!*a) {
		*a = strdup(b);
		प्रत्यागम !*a ? -ENOMEM : 0;
	}

	al = वाक्यदैर्घ्यम्(*a);
	bl = वाक्यदैर्घ्यम्(b);

	tmp = realloc(*a, al + bl + 1);
	यदि (!tmp)
		प्रत्यागम -ENOMEM;
	*a = tmp;
	स्मृतिं_प्रतिलिख((*a) + al, b, bl + 1);
	प्रत्यागम 0;
}

/* the hybrid version of strfconcat and strappend. */
पूर्णाङ्क strfappend(वर्ण **a, ध्रुव वर्ण *format, ...)
{
	va_list ap;
	पूर्णाङ्क res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	प्रत्यागम res;
}

बाह्य पूर्णाङ्क ul_strvfappend(वर्ण **a, ध्रुव वर्ण *format, va_list ap)
{
	वर्ण *val;
	पूर्णाङ्क sz;
	पूर्णाङ्क res;

	sz = vasprintf(&val, format, ap);
	यदि (sz < 0)
		प्रत्यागम -errno;

	res = ul_strappend(a, val);
	मुक्त(val);
	प्रत्यागम res;
}

स्थिर size_t strcspn_escaped(ध्रुव वर्ण *s, ध्रुव वर्ण *reject)
{
	पूर्णाङ्क escaped = 0;
	पूर्णाङ्क n;

	प्रति (n=0; s[n]; n++) {
		यदि (escaped)
			escaped = 0;
		अन्यथा यदि (s[n] == '\\')
			escaped = 1;
		अन्यथा यदि (strchr(reject, s[n]))
			भङ्ग;
	}

	/* if s ends in \, return index of previous char */
	प्रत्यागम n - escaped;
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
वर्ण *ul_strchr_escaped(ध्रुव वर्ण *s, पूर्णाङ्क c)
{
	वर्ण *p;
	पूर्णाङ्क esc = 0;

	प्रति (p = (वर्ण *) s; p && *p; p++) {
		यदि (!esc && *p == '\\') {
			esc = 1;
			अनुवर्त;
		}
		यदि (*p == c && (!esc || c == '\\'))
			प्रत्यागम p;
		esc = 0;
	}

	प्रत्यागम रिक्त;
}

/* Split a string into words. */
ध्रुव वर्ण *ul_विभज(ध्रुव वर्ण **state, size_t *l, ध्रुव वर्ण *separator, पूर्णाङ्क quoted)
{
	ध्रुव वर्ण *current;

	current = *state;

	यदि (!*current) {
		प्रतिज्ञा(**state == '\0');
		प्रत्यागम रिक्त;
	}

	current += strspn(current, separator);
	यदि (!*current) {
		*state = current;
		प्रत्यागम रिक्त;
	}

	यदि (quoted && strchr("\'\"", *current)) {
		वर्ण quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		यदि (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			प्रत्यागम रिक्त;
		}
		*state = current++ + *l + 2;
	} अन्यथा यदि (quoted) {
		*l = strcspn_escaped(current, separator);
		यदि (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			प्रत्यागम रिक्त;
		}
		*state = current + *l;
	} अन्यथा {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	प्रत्यागम current;
}

/* Rewind file pointer forward to new line.  */
पूर्णाङ्क skip_fline(FILE *fp)
{
	पूर्णाङ्क ch;

	कुरु {
		यदि ((ch = fgetc(fp)) == EOF)
			प्रत्यागम 1;
		यदि (ch == '\n')
			प्रत्यागम 0;
	} यावत् (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
पूर्णाङ्क ul_stralnumcmp(ध्रुव वर्ण *p1, ध्रुव वर्ण *p2)
{
	ध्रुव अचिह्नित वर्ण *s1 = (ध्रुव अचिह्नित वर्ण *) p1;
	ध्रुव अचिह्नित वर्ण *s2 = (ध्रुव अचिह्नित वर्ण *) p2;
	अचिह्नित वर्ण c1, c2;

	कुरु {
		कुरु {
			c1 = (अचिह्नित वर्ण) *s1++;
		} यावत् (c1 != '\0' && !अक्षरम्_अङ्कः_वा((अचिह्नित पूर्णाङ्क) c1));

		कुरु {
			c2 = (अचिह्नित वर्ण) *s2++;
		} यावत् (c2 != '\0' && !अक्षरम्_अङ्कः_वा((अचिह्नित पूर्णाङ्क) c2));

		यदि (c1 != '\0')
			c1 = tolower(c1);
		यदि (c2 != '\0')
			c2 = tolower(c2);
		यदि (c1 == '\0')
			प्रत्यागम c1 - c2;
	} यावत् (c1 == c2);

	प्रत्यागम c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
पूर्णाङ्क ul_optstr_अग्र(वर्ण **optstr, वर्ण **name, size_t *namesz,
		   वर्ण **मानम्, size_t *valsz)
{
	पूर्णाङ्क open_quote = 0;
	वर्ण *start = रिक्त, *stop = रिक्त, *p, *sep = रिक्त;
	वर्ण *optstr0;

	प्रतिज्ञा(optstr);
	प्रतिज्ञा(*optstr);

	optstr0 = *optstr;

	यदि (name)
		*name = रिक्त;
	यदि (namesz)
		*namesz = 0;
	यदि (मानम्)
		*मानम् = रिक्त;
	यदि (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	यावत् (optstr0 && *optstr0 == ',')
		optstr0++;

	प्रति (p = optstr0; p && *p; p++) {
		यदि (!start && *p == '=')
			प्रत्यागम -EINVAL;
		यदि (!start)
			start = p;		/* beginning of the option item */
		यदि (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		यदि (open_quote)
			अनुवर्त;		/* still in quoted block */
		यदि (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		यदि (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		अन्यथा यदि (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		यदि (!start || !stop)
			अनुवर्त;
		यदि (stop <= start)
			प्रत्यागम -EINVAL;

		यदि (name)
			*name = start;
		यदि (namesz)
			*namesz = sep ? sep - start : stop - start;
		*optstr = *stop ? stop + 1 : stop;

		यदि (sep) {
			यदि (मानम्)
				*मानम् = sep + 1;
			यदि (valsz)
				*valsz = stop - sep - 1;
		}
		प्रत्यागम 0;
	}

	प्रत्यागम 1;				/* end of optstr */
}

पूर्णाङ्क ul_optstr_is_valid(ध्रुव वर्ण *optstr)
{
	पूर्णाङ्क rc;
	वर्ण *p = (वर्ण *) optstr;

	यावत् ((rc = ul_optstr_अग्र(&p, रिक्त, रिक्त, रिक्त, रिक्त)) == 0);
	प्रत्यागम rc < 0 ? 0 : 1;
}

वर्ण *ul_optstr_get_मानम्(ध्रुव वर्ण *optstr, ध्रुव वर्ण *कुञ्चिका)
{
	size_t sz, namesz = 0, valsz = 0;
	वर्ण *name = रिक्त, *मानम् = रिक्त;
	वर्ण *p = (वर्ण *) optstr;

	यदि (!optstr || !कुञ्चिका || !*कुञ्चिका)
		प्रत्यागम रिक्त;

	sz = वाक्यदैर्घ्यम्(कुञ्चिका);
	यावत् (ul_optstr_अग्र(&p, &name, &namesz, &मानम्, &valsz) == 0) {
		यदि (namesz != sz || !valsz)
			अनुवर्त;
		यदि (वाक्ये_तुलय(name, कुञ्चिका, namesz) == 0)
			प्रत्यागम strndup(मानम्, valsz);
	}
	प्रत्यागम रिक्त;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(वर्ण *str)
{
	size_t len;
	वर्ण q;

	यदि (!str || !*str)
		प्रत्यागम 0;

	len = वाक्यदैर्घ्यम्(str);
	q = *str;

	यदि ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		प्रत्यागम len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	प्रत्यागम len - 2;
}

#यदि_परिभाषितम् TEST_क्रमादेशः_STRUTILS

संरचना testS {
	वर्ण *name;
	वर्ण *मानम्;
};

स्थिर पूर्णाङ्क test_strdup_गन्तव्यम्_member(पूर्णाङ्क argc, वर्ण *argv[])
{
	संरचना testS *xx;

	यदि (argc < 3)
		प्रत्यागम EXIT_FAILURE;

	xx = शून्यस्मृतिं_देहि(1, परिमाण(*xx));
	यदि (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_गन्तव्यम्_struct_member(xx, name, argv[1]);
	strdup_गन्तव्यम्_struct_member(xx, मानम्, argv[2]);

	यदि (वाक्यं_तुलय(xx->name, argv[1]) != 0 &&
	    वाक्यं_तुलय(xx->मानम्, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	प्रारूप_लिख("1: '%s', 2: '%s'\n", xx->name, xx->मानम्);

	मुक्त(xx->name);
	मुक्त(xx->मानम्);
	मुक्त(xx);
	प्रत्यागम EXIT_SUCCESS;
}

स्थिर पूर्णाङ्क test_strutils_sizes(पूर्णाङ्क argc, वर्ण *argv[])
{
	uintmax_t आकार = 0;
	वर्ण *hum1, *hum2, *hum3;

	यदि (argc < 2)
		प्रत्यागम EXIT_FAILURE;

	यदि (strtosize(argv[1], &आकार))
		errx(EXIT_FAILURE, "invalid size '%s' value", argv[1]);

	hum1 = आकार_गन्तव्यम्_human_string(आकार_SUFFIX_1_LETTER, आकार);
	hum2 = आकार_गन्तव्यम्_human_string(आकार_SUFFIX_3_LETTER |
				    आकार_SUFFIX_SPACE, आकार);
	hum3 = आकार_गन्तव्यम्_human_string(आकार_SUFFIX_3_LETTER |
				    आकार_SUFFIX_SPACE |
				    आकार_DECIMAL_2_DIGITS, आकार);

	प्रारूप_लिख("%25s : %20ju : %8s : %12s : %13s\n", argv[1], आकार, hum1, hum2, hum3);
	मुक्त(hum1);
	मुक्त(hum2);
	मुक्त(hum3);

	प्रत्यागम EXIT_SUCCESS;
}

स्थिर पूर्णाङ्क test_strutils_cmp_paths(पूर्णाङ्क argc, वर्ण *argv[])
{
	पूर्णाङ्क rc = streq_paths(argv[1], argv[2]);

	यदि (argc < 3)
		प्रत्यागम EXIT_FAILURE;

	प्रारूप_लिख("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	प्रत्यागम EXIT_SUCCESS;
}

स्थिर पूर्णाङ्क test_strutils_मार्गः_is_within(पूर्णाङ्क argc, वर्ण *argv[])
{
	पूर्णाङ्क rc;

	यदि (argc < 3)
		प्रत्यागम EXIT_FAILURE;

	rc = ul_मार्गः_is_within(argv[1], argv[2]);

	प्रारूप_लिख("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	प्रत्यागम EXIT_SUCCESS;
}

स्थिर पूर्णाङ्क test_strutils_normalize(पूर्णाङ्क argc, वर्ण *argv[])
{
	अचिह्नित वर्ण *src, *dst, *org;
	size_t sz, len;

	यदि (argc < 2)
		प्रत्यागम EXIT_FAILURE;

	org = (अचिह्नित वर्ण *) strdup(argv[1]);
	src = (अचिह्नित वर्ण *) strdup((वर्ण *) org);
	len = वाक्यदैर्घ्यम्((वर्ण *) src);
	dst = स्मृतिं_देहि(len + 1);

	यदि (!org || !src || !dst)
		गच्छ समाप्तम्;

	/* two buffers */
	sz = __normalize_whitespace(src, len, dst, len + 1);
	प्रारूप_लिख("1: '%s' --> '%s' [sz=%zu]\n", src, dst, sz);

	/* one buffer */
	sz = normalize_whitespace(src);
	प्रारूप_लिख("2: '%s' --> '%s' [sz=%zu]\n", org, src, sz);

समाप्तम्:
	मुक्त(src);
	मुक्त(dst);
	मुक्त(org);

	प्रत्यागम EXIT_SUCCESS;
}

स्थिर पूर्णाङ्क test_strutils_cstrcasecmp(पूर्णाङ्क argc, वर्ण *argv[])
{
	वर्ण *a, *b;

	यदि (argc < 3)
		प्रत्यागम EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	यदि (!a || !b)
		प्रत्यागम EXIT_FAILURE;

	प्रारूप_लिख("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	प्रारूप_लिख("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	प्रारूप_लिख("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, वाक्यदैर्घ्यम्(a)));

	प्रत्यागम EXIT_SUCCESS;
}

पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण *argv[])
{
	यदि (argc == 3 && वाक्यं_तुलय(argv[1], "--size") == 0) {
		प्रत्यागम test_strutils_sizes(argc - 1, argv + 1);

	} अन्यथा यदि (argc == 3 && वाक्यं_तुलय(argv[1], "--parse-switch") == 0) {
		प्रारूप_लिख("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						रिक्त));
		प्रत्यागम EXIT_SUCCESS;
	} अन्यथा यदि (argc == 3 && वाक्यं_तुलय(argv[1], "--strtobool") == 0) {
		bool val;
		यदि (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		प्रारूप_लिख("'%s'-->%s\n", argv[2], val ? "true" : "false");
		प्रत्यागम EXIT_SUCCESS;

	} अन्यथा यदि (argc == 4 && वाक्यं_तुलय(argv[1], "--cmp-paths") == 0) {
		प्रत्यागम test_strutils_cmp_paths(argc - 1, argv + 1);

	} अन्यथा यदि (argc == 4 && वाक्यं_तुलय(argv[1], "--path-is-within") == 0) {
		प्रत्यागम test_strutils_मार्गः_is_within(argc - 1, argv + 1);

	} अन्यथा यदि (argc == 4 && वाक्यं_तुलय(argv[1], "--strdup-member") == 0) {
		प्रत्यागम test_strdup_गन्तव्यम्_member(argc - 1, argv + 1);

	} अन्यथा यदि  (argc == 4 && वाक्यं_तुलय(argv[1], "--stralnumcmp") == 0) {
		प्रारूप_लिख("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		प्रत्यागम EXIT_SUCCESS;

	} अन्यथा यदि (argc == 4 && वाक्यं_तुलय(argv[1], "--cstrcasecmp") == 0) {
		प्रत्यागम test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} अन्यथा यदि (argc == 3 && वाक्यं_तुलय(argv[1], "--normalize") == 0) {
		प्रत्यागम test_strutils_normalize(argc - 1, argv + 1);

	} अन्यथा यदि (argc == 3 && वाक्यं_तुलय(argv[1], "--strtos64") == 0) {
		प्रारूप_लिख("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		प्रत्यागम EXIT_SUCCESS;
	} अन्यथा यदि (argc == 3 && वाक्यं_तुलय(argv[1], "--strtou64") == 0) {
		प्रारूप_लिख("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		प्रत्यागम EXIT_SUCCESS;
	} अन्यथा यदि (argc == 3 && वाक्यं_तुलय(argv[1], "--strtos32") == 0) {
		प्रारूप_लिख("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		प्रत्यागम EXIT_SUCCESS;
	} अन्यथा यदि (argc == 3 && वाक्यं_तुलय(argv[1], "--strtou32") == 0) {
		प्रारूप_लिख("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		प्रत्यागम EXIT_SUCCESS;
	} अन्यथा यदि (argc == 3 && वाक्यं_तुलय(argv[1], "--strtos16") == 0) {
		प्रारूप_लिख("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		प्रत्यागम EXIT_SUCCESS;
	} अन्यथा यदि (argc == 3 && वाक्यं_तुलय(argv[1], "--strtou16") == 0) {
		प्रारूप_लिख("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		प्रत्यागम EXIT_SUCCESS;

	} अन्यथा यदि (argc == 4 && वाक्यं_तुलय(argv[1], "--strchr-escaped") == 0) {
		प्रारूप_लिख("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		प्रत्यागम EXIT_SUCCESS;

	} अन्यथा यदि (argc == 2 && वाक्यं_तुलय(argv[1], "--next-string") == 0) {
		वर्ण *buf = "abc\0Y\0\0xyz\0X";
		वर्ण *अन्तः = buf + 12;
		वर्ण *p = buf;

		कुरु {
			प्रारूप_लिख("str: '%s'\n", p);
		} यावत् ((p = ul_अग्र_string(p, अन्तः)));

		प्रत्यागम EXIT_SUCCESS;

	} अन्यथा यदि (argc == 3 && वाक्यं_तुलय(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		वर्ण *name = रिक्त, *val = रिक्त;
		वर्ण *p = argv[2];
		पूर्णाङ्क rc;

		यदि (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		यावत् ((rc = ul_optstr_अग्र(&p, &name, &namesz, &val, &valsz)) == 0) {
			प्रारूप_लिख("'%.*s' : '%.*s'\n", (पूर्णाङ्क) namesz, name,
						   (पूर्णाङ्क) valsz, val);
		}
		यदि (rc == 1)
			प्रत्यागम EXIT_SUCCESS;
	} अन्यथा यदि (argc == 3 && वाक्यं_तुलय(argv[1], "--unquote") == 0) {

		वर्ण *unquoted = strdup(argv[2]);

		यदि (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		प्रारूप_लिख("%s-->%s\n", argv[2], unquoted);
		मुक्त(unquoted);
		प्रत्यागम EXIT_SUCCESS;

	} अन्यथा {
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
		निर्गच्छ(EXIT_FAILURE);
	}

	प्रत्यागम EXIT_FAILURE;
}
#यदि_अन्तः /* TEST_PROGRAM_STRUTILS */
