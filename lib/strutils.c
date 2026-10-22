/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Karel Zak <kzak@redhat.com> [2010]
 *          Davidlohr Bueso <dave@gnu.org> [2010]
 */
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <inttypes.h>
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <strings.h>
#അന്തര്ഭാവ <assert.h>

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "cctype.h"
#അന്തര്ഭാവ "nls.h"
#അന്തര്ഭാവ "strutils.h"
#അന്തര്ഭാവ "timeutils.h"
#അന്തര്ഭാവ "bitops.h"
#അന്തര്ഭാവ "pathnames.h"

സ്ഥിതം പൂർണ്ണസംഖ്യ STRTOXX_EXIT_CODE = EXIT_FAILURE;

ശൂന്യം strutils_set_exitcode(പൂർണ്ണസംഖ്യ ex)
{
	STRTOXX_EXIT_CODE = ex;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ do_scale_by_power (uintmax_t *x, പൂർണ്ണസംഖ്യ base, പൂർണ്ണസംഖ്യ power)
{
	ആകുമ്പോൾ (power--) {
		എങ്കിൽ (UINTMAX_MAX / base < *x)
			തിരികെ -ERANGE;
		*x *= base;
	}
	തിരികെ 0;
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
പൂർണ്ണസംഖ്യ ul_parse_size(സ്ഥിരം അക്ഷരം *str, uintmax_t *res, പൂർണ്ണസംഖ്യ *power)
{
	സ്ഥിരം അക്ഷരം *p;
	അക്ഷരം *end;
	uintmax_t x, frac = 0;
	പൂർണ്ണസംഖ്യ base = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	സ്ഥിതം സ്ഥിരം അക്ഷരം *suf  = "KMGTPEZY";
	സ്ഥിതം സ്ഥിരം അക്ഷരം *suf2 = "kmgtpezy";
	സ്ഥിരം അക്ഷരം *sp;

	*res = 0;

	എങ്കിൽ (!str || !*str) {
		rc = -EINVAL;
		പോകുക err;
	}

	/* Only positive numbers are acceptable
	 *
	 * Note that this check is not perfect, it would be better to
	 * use lconv->negative_sign. But coreutils use the same solution,
	 * so it's probably good enough...
	 */
	p = str;
	ആകുമ്പോൾ (isspace((unsigned അക്ഷരം) *p))
		p++;
	എങ്കിൽ (*p == '-') {
		rc = -EINVAL;
		പോകുക err;
	}

	errno = 0, end = ശൂന്യ;
	x = strtoumax(str, &end, 0);

	എങ്കിൽ (end == str ||
	    (errno != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = errno ? -errno : -EINVAL;
		പോകുക err;
	}
	എങ്കിൽ (!end || !*end)
		പോകുക done;			/* without suffix */
	p = end;

	/*
	 * Check size suffixes
	 */
check_suffix:
	എങ്കിൽ (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		base = 1024;			/* XiB, 2^N */
	അല്ലെങ്കിൽ എങ്കിൽ ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		base = 1000;			/* XB, 10^N */
	അല്ലെങ്കിൽ എങ്കിൽ (*(p + 1)) {
		ഘടന lconv സ്ഥിരം *l = localeconv();
		സ്ഥിരം അക്ഷരം *dp = l ? l->decimal_point : ശൂന്യ;
		size_t dpsz = dp ? strlen(dp) : 0;

		എങ്കിൽ (frac == 0 && *p && dp && strncmp(dp, p, dpsz) == 0) {
			സ്ഥിരം അക്ഷരം *fstr = p + dpsz;

			വേണ്ടി (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			എങ്കിൽ (isdigit(*fstr)) {
				errno = 0, end = ശൂന്യ;
				frac = strtoumax(fstr, &end, 0);
				എങ്കിൽ (end == fstr ||
				    (errno != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = errno ? -errno : -EINVAL;
					പോകുക err;
				}
			} അല്ലെങ്കിൽ
				end = (അക്ഷരം *) p;

			എങ്കിൽ (!end || !*end) {
				rc = -EINVAL;
				പോകുക err;	/* without suffix, but with fractions */
			}
			p = end;
			പോകുക check_suffix;
		}
		rc = -EINVAL;
		പോകുക err;			/* unexpected suffix */
	}

	sp = strchr(suf, *p);
	എങ്കിൽ (sp)
		pwr = (sp - suf) + 1;
	അല്ലെങ്കിൽ {
		sp = strchr(suf2, *p);
		എങ്കിൽ (sp)
			pwr = (sp - suf2) + 1;
		അല്ലെങ്കിൽ {
			rc = -EINVAL;
			പോകുക err;
		}
	}

	എങ്കിൽ (power)
		*power = pwr;

	rc = do_scale_by_power(&x, base, pwr);
	എങ്കിൽ (rc)
		പോകുക err;

	എങ്കിൽ (frac && pwr) {
		പൂർണ്ണസംഖ്യ i;
		uintmax_t frac_base = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_base, base, pwr);
		എങ്കിൽ (rc)
			പോകുക err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		ചെയ്യുക {
			frac_val = ((frac % 10) * frac_base + frac_val) / 10;
			frac /= 10;
		} ആകുമ്പോൾ (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		വേണ്ടി (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		എങ്കിൽ (UINTMAX_MAX - x < frac_val) {
			rc = -ERANGE;
			പോകുക err;
		}
		x += frac_val;
	}
done:
	*res = x;
err:
	എങ്കിൽ (rc < 0)
		errno = -rc;
	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ strtosize(സ്ഥിരം അക്ഷരം *str, uintmax_t *res)
{
	തിരികെ ul_parse_size(str, res, ശൂന്യ);
}

പൂർണ്ണസംഖ്യ isdigit_strend(സ്ഥിരം അക്ഷരം *str, സ്ഥിരം അക്ഷരം **end)
{
	സ്ഥിരം അക്ഷരം *p;

	വേണ്ടി (p = str; p && *p && isdigit((unsigned അക്ഷരം) *p); p++);

	എങ്കിൽ (end)
		*end = p;
	തിരികെ p && p > str && !*p;
}

പൂർണ്ണസംഖ്യ isxdigit_strend(സ്ഥിരം അക്ഷരം *str, സ്ഥിരം അക്ഷരം **end)
{
	സ്ഥിരം അക്ഷരം *p;

	വേണ്ടി (p = str; p && *p && isxdigit((unsigned അക്ഷരം) *p); p++);

	എങ്കിൽ (end)
		*end = p;

	തിരികെ p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
പൂർണ്ണസംഖ്യ ul_strtobool(സ്ഥിരം അക്ഷരം *str, bool *result)
{
	സ്ഥിതം സ്ഥിരം അക്ഷരം *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	സ്ഥിതം സ്ഥിരം അക്ഷരം *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	size_t i;

	എങ്കിൽ (!str || !result)
		തിരികെ -(errno = EINVAL);

	errno = 0;

	വേണ്ടി (i = 0; i < ARRAY_SIZE(bool_true); i++) {
		എങ്കിൽ (c_strcasecmp(str, bool_true[i]) == 0) {
			*result = true;
			തിരികെ 0;
		}
	}

	വേണ്ടി (i = 0; i < ARRAY_SIZE(bool_false); i++) {
		എങ്കിൽ (c_strcasecmp(str, bool_false[i]) == 0) {
			*result = false;
			തിരികെ 0;
		}
	}

	തിരികെ -(errno = EINVAL);
}

പൂർണ്ണസംഖ്യ ul_parse_switch(സ്ഥിരം അക്ഷരം *arg, ...)
{
	സ്ഥിരം അക്ഷരം *a, *b;
	va_list ap;

	va_start(ap, arg);
	ചെയ്യുക {
		a = va_arg(ap, അക്ഷരം *);
		എങ്കിൽ (!a)
			നിർത്തുക;
		b = va_arg(ap, അക്ഷരം *);
		എങ്കിൽ (!b)
			നിർത്തുക;

		എങ്കിൽ (strcmp(arg, a) == 0) {
			va_end(ap);
			തിരികെ 1;
		}

		എങ്കിൽ (strcmp(arg, b) == 0) {
			va_end(ap);
			തിരികെ 0;
		}
	} ആകുമ്പോൾ (1);
	va_end(ap);

	errx(STRTOXX_EXIT_CODE, _("unsupported argument: %s"), arg);
}

#ifndef HAVE_MEMPCPY
ശൂന്യം *mempcpy(ശൂന്യം *restrict dest, സ്ഥിരം ശൂന്യം *restrict src, size_t n)
{
	തിരികെ ((അക്ഷരം *)memcpy(dest, src, n)) + n;
}
#endif

#ifndef HAVE_STRNLEN
size_t strnlen(സ്ഥിരം അക്ഷരം *s, size_t maxlen)
{
	size_t i;

	വേണ്ടി (i = 0; i < maxlen; i++) {
		എങ്കിൽ (s[i] == '\0')
			തിരികെ i;
	}
	തിരികെ maxlen;
}
#endif

#ifndef HAVE_STRNCHR
അക്ഷരം *strnchr(സ്ഥിരം അക്ഷരം *s, size_t maxlen, പൂർണ്ണസംഖ്യ c)
{
	വേണ്ടി (; maxlen-- && *s != '\0'; ++s)
		എങ്കിൽ (*s == (അക്ഷരം)c)
			തിരികെ (അക്ഷരം *)s;
	തിരികെ ശൂന്യ;
}
#endif

#ifndef HAVE_STRNDUP
അക്ഷരം *strndup(സ്ഥിരം അക്ഷരം *s, size_t n)
{
	size_t len = strnlen(s, n);
	അക്ഷരം *new = malloc((len + 1) * വലുപ്പം(അക്ഷരം));
	എങ്കിൽ (!new)
		തിരികെ ശൂന്യ;
	new[len] = '\0';
	തിരികെ (അക്ഷരം *) memcpy(new, s, len);
}
#endif

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
പൂർണ്ണസംഖ്യ ul_strtos64(സ്ഥിരം അക്ഷരം *str, int64_t *അക്കം, പൂർണ്ണസംഖ്യ base)
{
	അക്ഷരം *end = ശൂന്യ;

	എങ്കിൽ (str == ശൂന്യ || *str == '\0')
		തിരികെ -(errno = EINVAL);

	errno = 0;
	*അക്കം = (int64_t) strtoimax(str, &end, base);

	എങ്കിൽ (errno != 0)
		തിരികെ -errno;
	എങ്കിൽ (str == end || (end && *end))
		തിരികെ -(errno = EINVAL);
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ ul_strtou64(സ്ഥിരം അക്ഷരം *str, uint64_t *അക്കം, പൂർണ്ണസംഖ്യ base)
{
	അക്ഷരം *end = ശൂന്യ;
	int64_t tmp;

	എങ്കിൽ (str == ശൂന്യ || *str == '\0')
		തിരികെ -(errno = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	errno = 0;
	tmp = (int64_t) strtoimax(str, &end, base);
	എങ്കിൽ (tmp < 0)
		errno = ERANGE;
	അല്ലെങ്കിൽ {
		errno = 0;
		*അക്കം = strtoumax(str, &end, base);
	}

	എങ്കിൽ (errno != 0)
		തിരികെ -errno;
	എങ്കിൽ (str == end || (end && *end))
		തിരികെ -(errno = EINVAL);
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ ul_strtos32(സ്ഥിരം അക്ഷരം *str, int32_t *അക്കം, പൂർണ്ണസംഖ്യ base)
{
	int64_t tmp;
	പൂർണ്ണസംഖ്യ rc;

	rc = ul_strtos64(str, &tmp, base);
	എങ്കിൽ (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(errno = ERANGE);
	എങ്കിൽ (rc == 0)
		*അക്കം = (int32_t) tmp;
	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ ul_strtou32(സ്ഥിരം അക്ഷരം *str, uint32_t *അക്കം, പൂർണ്ണസംഖ്യ base)
{
	uint64_t tmp;
	പൂർണ്ണസംഖ്യ rc;

	rc = ul_strtou64(str, &tmp, base);
	എങ്കിൽ (rc == 0 && tmp > UINT32_MAX)
		rc = -(errno = ERANGE);
	എങ്കിൽ (rc == 0)
		*അക്കം = (uint32_t) tmp;
	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ ul_strtou16(സ്ഥിരം അക്ഷരം *str, uint16_t *അക്കം, പൂർണ്ണസംഖ്യ base)
{
	uint64_t tmp;
	പൂർണ്ണസംഖ്യ rc;

	rc = ul_strtou64(str, &tmp, base);
	എങ്കിൽ (rc == 0 && tmp > UINT16_MAX)
		rc = -(errno = ERANGE);
	എങ്കിൽ (rc == 0)
		*അക്കം = (uint16_t) tmp;
	തിരികെ rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(സ്ഥിരം അക്ഷരം *str, പൂർണ്ണസംഖ്യ base, സ്ഥിരം അക്ഷരം *errmesg,
			     int64_t താഴെയുള്ള, int64_t up)
{
	int64_t അക്കം = 0;
	പൂർണ്ണസംഖ്യ rc;

	rc = ul_strtos64(str, &അക്കം, base);
	എങ്കിൽ (rc == 0 && ((താഴെയുള്ള && അക്കം < താഴെയുള്ള) || (up && അക്കം > up)))
		rc = -(errno = ERANGE);

	എങ്കിൽ (rc) {
		എങ്കിൽ (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	തിരികെ അക്കം;
}

uint64_t str2unum_or_err(സ്ഥിരം അക്ഷരം *str, പൂർണ്ണസംഖ്യ base, സ്ഥിരം അക്ഷരം *errmesg, uint64_t up)
{
	uint64_t അക്കം = 0;
	പൂർണ്ണസംഖ്യ rc;

	rc = ul_strtou64(str, &അക്കം, base);
	എങ്കിൽ (rc == 0 && (up && അക്കം > up))
		rc = -(errno = ERANGE);

	എങ്കിൽ (rc) {
		എങ്കിൽ (errno == ERANGE)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	തിരികെ അക്കം;
}

ഇരട്ട strtod_or_err(സ്ഥിരം അക്ഷരം *str, സ്ഥിരം അക്ഷരം *errmesg)
{
	ഇരട്ട അക്കം;
	അക്ഷരം *end = ശൂന്യ;

	errno = 0;
	എങ്കിൽ (str == ശൂന്യ || *str == '\0')
		പോകുക err;
	അക്കം = strtod(str, &end);

	എങ്കിൽ (errno || str == end || (end && *end))
		പോകുക err;

	തിരികെ അക്കം;
err:
	എങ്കിൽ (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

പൂർണ്ണസംഖ്യ ul_strtold(സ്ഥിരം അക്ഷരം *str, long ഇരട്ട *അക്കം)
{
	അക്ഷരം *end = ശൂന്യ;

	errno = 0;
	എങ്കിൽ (str == ശൂന്യ || *str == '\0')
		തിരികെ -(errno = EINVAL);
	*അക്കം = strtold(str, &end);

	എങ്കിൽ (errno != 0)
		തിരികെ -errno;
	എങ്കിൽ (str == end || (end && *end))
		തിരികെ -(errno = EINVAL);
	തിരികെ 0;
}

long ഇരട്ട strtold_or_err(സ്ഥിരം അക്ഷരം *str, സ്ഥിരം അക്ഷരം *errmesg)
{
	long ഇരട്ട അക്കം = 0;

	എങ്കിൽ (ul_strtold(str, &അക്കം) == 0)
		തിരികെ അക്കം;
	എങ്കിൽ (errno == ERANGE)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(സ്ഥിരം അക്ഷരം *str, സ്ഥിരം അക്ഷരം *errmesg)
{
	uintmax_t അക്കം;

	എങ്കിൽ (strtosize(str, &അക്കം) == 0)
		തിരികെ അക്കം;

	എങ്കിൽ (errno)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}


ശൂന്യം strtotimeval_or_err(സ്ഥിരം അക്ഷരം *str, ഘടന timeval *tv, സ്ഥിരം അക്ഷരം *errmesg)
{
	long ഇരട്ട user_നിവേശം;

	user_നിവേശം = strtold_or_err(str, errmesg);
	tv->tv_sec = (time_t) user_നിവേശം;
	tv->tv_usec = (suseconds_t)((user_നിവേശം - tv->tv_sec) * USEC_PER_SEC);
}

ശൂന്യം strtotimespec_or_err(സ്ഥിരം അക്ഷരം *str, ഘടന timespec *ts, സ്ഥിരം അക്ഷരം *errmesg)
{
	long ഇരട്ട user_നിവേശം;

	user_നിവേശം = strtold_or_err(str, errmesg);
	ts->tv_sec = (time_t) user_നിവേശം;
	ts->tv_nsec = (long)((user_നിവേശം - ts->tv_sec) * NSEC_PER_SEC);
}

time_t strtotime_or_err(സ്ഥിരം അക്ഷരം *str, സ്ഥിരം അക്ഷരം *errmesg)
{
	int64_t user_നിവേശം;

	user_നിവേശം = strtos64_or_err(str, errmesg);
	തിരികെ (time_t) user_നിവേശം;
}

bool hyperlinkwanted(സ്ഥിരം അക്ഷരം *mode)
{
	എങ്കിൽ (mode && strcmp(mode, "never") == 0)
		തിരികെ false;

	എങ്കിൽ (mode && strcmp(mode, "always") == 0)
		തിരികെ true;

	എങ്കിൽ (!mode || strcmp(mode, "auto") == 0)
		തിരികെ isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --hyperlink: %s"), mode);
}

bool annotationwanted(സ്ഥിരം അക്ഷരം *mode)
{
	എങ്കിൽ (mode && strcmp(mode, "never") == 0)
		തിരികെ false;

	എങ്കിൽ (mode && strcmp(mode, "always") == 0)
		തിരികെ true;

	എങ്കിൽ (!mode || strcmp(mode, "auto") == 0)
		തിരികെ isatty(STDOUT_FILENO) ? true : false;

	errx(EXIT_FAILURE, _("invalid argument of --annotate: %s"), mode);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
അക്ഷരം *xstrmode(mode_t mode, അക്ഷരം *str)
{
	unsigned short i = 0;

	എങ്കിൽ (S_ISDIR(mode))
		str[i++] = 'd';
	അല്ലെങ്കിൽ എങ്കിൽ (S_ISLNK(mode))
		str[i++] = 'l';
	അല്ലെങ്കിൽ എങ്കിൽ (S_ISCHR(mode))
		str[i++] = 'c';
	അല്ലെങ്കിൽ എങ്കിൽ (S_ISBLK(mode))
		str[i++] = 'b';
	അല്ലെങ്കിൽ എങ്കിൽ (S_ISSOCK(mode))
		str[i++] = 's';
	അല്ലെങ്കിൽ എങ്കിൽ (S_ISFIFO(mode))
		str[i++] = 'p';
	അല്ലെങ്കിൽ എങ്കിൽ (S_ISREG(mode))
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

	തിരികെ str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ get_exp(uint64_t n)
{
	പൂർണ്ണസംഖ്യ shft;

	വേണ്ടി (shft = 10; shft <= 60; shft += 10) {
		എങ്കിൽ (n < (1ULL << shft))
			നിർത്തുക;
	}
	തിരികെ shft - 10;
}

അക്ഷരം *size_to_human_string(പൂർണ്ണസംഖ്യ options, uint64_t bytes)
{
	അക്ഷരം buf[32];
	പൂർണ്ണസംഖ്യ dec, exp;
	uint64_t frac;
	സ്ഥിരം അക്ഷരം *letters = "BKMGTPE";
	അക്ഷരം suffix[വലുപ്പം(" KiB")], *psuf = suffix;
	അക്ഷരം c;

	എങ്കിൽ (options & SIZE_SUFFIX_SPACE)
		*psuf++ = ' ';


	exp  = get_exp(bytes);
	c    = *(letters + (exp ? exp / 10 : 0));
	dec  = exp ? bytes / (1ULL << exp) : bytes;
	frac = exp ? bytes % (1ULL << exp) : 0;

	*psuf++ = c;

	എങ്കിൽ ((options & SIZE_SUFFIX_3LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/* round */
	എങ്കിൽ (frac) {
		/* get 3 digits after decimal point */
		എങ്കിൽ (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (exp - 10)) ;
		അല്ലെങ്കിൽ
			frac = (frac * 1000) / (1ULL << (exp)) ;

		എങ്കിൽ (options & SIZE_DECIMAL_2DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} അല്ലെങ്കിൽ {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		എങ്കിൽ (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	എങ്കിൽ (frac) {
		ഘടന lconv സ്ഥിരം *l = localeconv();
		അക്ഷരം *dp = l ? l->decimal_point : ശൂന്യ;
		പൂർണ്ണസംഖ്യ len;

		എങ്കിൽ (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, വലുപ്പം(buf), "%d%s%02" PRIu64, dec, dp, frac);
		എങ്കിൽ (len > 0 && (size_t) len < വലുപ്പം(buf)) {
			/* remove potential extraneous zero */
			എങ്കിൽ (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, വലുപ്പം(buf) - len);
		} അല്ലെങ്കിൽ
			*buf = '\0';	/* snprintf error */
	} അല്ലെങ്കിൽ
		snprintf(buf, വലുപ്പം(buf), "%d%s", dec, suffix);

	തിരികെ strdup(buf);
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
പൂർണ്ണസംഖ്യ string_to_idarray(സ്ഥിരം അക്ഷരം *list, പൂർണ്ണസംഖ്യ ary[], size_t arysz,
			പൂർണ്ണസംഖ്യ (name2id)(സ്ഥിരം അക്ഷരം *, size_t))
{
	സ്ഥിരം അക്ഷരം *begin = ശൂന്യ, *p;
	size_t n = 0;

	എങ്കിൽ (!list || !*list || !ary || !arysz || !name2id)
		തിരികെ -1;

	വേണ്ടി (p = list; p && *p; p++) {
		സ്ഥിരം അക്ഷരം *end = ശൂന്യ;
		പൂർണ്ണസംഖ്യ id;

		എങ്കിൽ (n >= arysz)
			തിരികെ -2;
		എങ്കിൽ (!begin)
			begin = p;		/* begin of the column name */
		എങ്കിൽ (*p == ',')
			end = p;		/* terminate the name */
		എങ്കിൽ (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		എങ്കിൽ (!begin || !end)
			തുടരുക;
		എങ്കിൽ (end <= begin)
			തിരികെ -1;

		id = name2id(begin, end - begin);
		എങ്കിൽ (id == -1)
			തിരികെ -1;
		ary[ n++ ] = id;
		begin = ശൂന്യ;
		എങ്കിൽ (end && !*end)
			നിർത്തുക;
	}
	തിരികെ n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
പൂർണ്ണസംഖ്യ string_add_to_idarray(സ്ഥിരം അക്ഷരം *list, പൂർണ്ണസംഖ്യ ary[], size_t arysz,
			size_t *ary_pos, പൂർണ്ണസംഖ്യ (name2id)(സ്ഥിരം അക്ഷരം *, size_t))
{
	സ്ഥിരം അക്ഷരം *list_add;
	പൂർണ്ണസംഖ്യ r;

	എങ്കിൽ (!list || !*list || !ary_pos || *ary_pos > arysz)
		തിരികെ -1;

	എങ്കിൽ (list[0] == '+')
		list_add = &list[1];
	അല്ലെങ്കിൽ {
		list_add = list;
		*ary_pos = 0;
	}

	r = string_to_idarray(list_add, &ary[*ary_pos], arysz - *ary_pos, name2id);
	എങ്കിൽ (r > 0)
		*ary_pos += r;
	തിരികെ r;
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
പൂർണ്ണസംഖ്യ string_to_bitarray(സ്ഥിരം അക്ഷരം *list,
		     അക്ഷരം *ary,
		     പൂർണ്ണസംഖ്യ (*name2bit)(സ്ഥിരം അക്ഷരം *, size_t),
		     size_t allow_range)
{
	സ്ഥിരം അക്ഷരം *begin = ശൂന്യ, *p;

	എങ്കിൽ (!list || !name2bit || !ary)
		തിരികെ -EINVAL;

	വേണ്ടി (p = list; p && *p; p++) {
		സ്ഥിരം അക്ഷരം *end = ശൂന്യ;
		പൂർണ്ണസംഖ്യ bit, set_lower = 0, set_higher = 0;

		എങ്കിൽ (!begin)
			begin = p;		/* begin of the level name */
		എങ്കിൽ (*p == ',')
			end = p;		/* terminate the name */
		എങ്കിൽ (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		എങ്കിൽ (!begin || !end)
			തുടരുക;
		എങ്കിൽ (end <= begin)
			തിരികെ -1;
		എങ്കിൽ (allow_range) {
			എങ്കിൽ (*(end - 1) == '+') {
				end--;
				set_lower = 1;
			} അല്ലെങ്കിൽ എങ്കിൽ (*begin == '+') {
				begin++;
				set_higher = 1;
			}
		}

		bit = name2bit(begin, end - begin);
		എങ്കിൽ (bit < 0)
			തിരികെ bit;
		setbit(ary, bit);
		എങ്കിൽ (set_lower)
			ആകുമ്പോൾ (--bit >= 0)
				setbit(ary, bit);
		അല്ലെങ്കിൽ എങ്കിൽ (set_higher)
			ആകുമ്പോൾ (++bit < (പൂർണ്ണസംഖ്യ) allow_range)
				setbit(ary, bit);
		begin = ശൂന്യ;
		എങ്കിൽ (end && !*end)
			നിർത്തുക;
	}
	തിരികെ 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
പൂർണ്ണസംഖ്യ string_to_bitmask(സ്ഥിരം അക്ഷരം *list,
		     unsigned long *mask,
		     long (*name2flag)(സ്ഥിരം അക്ഷരം *, size_t))
{
	സ്ഥിരം അക്ഷരം *begin = ശൂന്യ, *p;

	എങ്കിൽ (!list || !name2flag || !mask)
		തിരികെ -EINVAL;

	വേണ്ടി (p = list; p && *p; p++) {
		സ്ഥിരം അക്ഷരം *end = ശൂന്യ;
		long flag;

		എങ്കിൽ (!begin)
			begin = p;		/* begin of the level name */
		എങ്കിൽ (*p == ',')
			end = p;		/* terminate the name */
		എങ്കിൽ (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		എങ്കിൽ (!begin || !end)
			തുടരുക;
		എങ്കിൽ (end <= begin)
			തിരികെ -1;

		flag = name2flag(begin, end - begin);
		എങ്കിൽ (flag < 0)
			തിരികെ flag;	/* error */
		*mask |= flag;
		begin = ശൂന്യ;
		എങ്കിൽ (end && !*end)
			നിർത്തുക;
	}
	തിരികെ 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
പൂർണ്ണസംഖ്യ ul_parse_range(സ്ഥിരം അക്ഷരം *str, പൂർണ്ണസംഖ്യ *lower, പൂർണ്ണസംഖ്യ *upper, പൂർണ്ണസംഖ്യ def)
{
	അക്ഷരം *end = ശൂന്യ;

	എങ്കിൽ (!str)
		തിരികെ 0;

	*upper = *lower = def;
	errno = 0;

	എങ്കിൽ (*str == ':') {				/* <:N> */
		str++;
		*upper = strtol(str, &end, 10);
		എങ്കിൽ (errno || !end || *end || end == str)
			തിരികെ -1;
	} അല്ലെങ്കിൽ {
		*upper = *lower = strtol(str, &end, 10);
		എങ്കിൽ (errno || !end || end == str)
			തിരികെ -1;

		എങ്കിൽ (*end == ':' && !*(end + 1))		/* <M:> */
			*upper = def;
		അല്ലെങ്കിൽ എങ്കിൽ (*end == '-' || *end == ':') {	/* <M:N> <M-N> */
			str = end + 1;
			end = ശൂന്യ;
			errno = 0;
			*upper = strtol(str, &end, 10);

			എങ്കിൽ (errno || !end || *end || end == str)
				തിരികെ -1;
		}
	}
	തിരികെ 0;
}

സ്ഥിരം അക്ഷരം *ul_path_next_segment(സ്ഥിരം അക്ഷരം *str, size_t *sz)
{
	സ്ഥിരം അക്ഷരം *start, *p;

	start = str;
	*sz = 0;
	ആകുമ്പോൾ (start && *start == '/' && *(start + 1) == '/')
		start++;

	എങ്കിൽ (!start || !*start)
		തിരികെ ശൂന്യ;

	വേണ്ടി (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	തിരികെ start;
}

പൂർണ്ണസംഖ്യ streq_paths(സ്ഥിരം അക്ഷരം *a, സ്ഥിരം അക്ഷരം *b)
{
	ആകുമ്പോൾ (a && b) {
		size_t a_sz, b_sz;
		സ്ഥിരം അക്ഷരം *a_seg = ul_path_next_segment(a, &a_sz);
		സ്ഥിരം അക്ഷരം *b_seg = ul_path_next_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		എങ്കിൽ (a_sz + b_sz == 0)
			തിരികെ 1;

		/* ignore trailing slash */
		എങ്കിൽ (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			തിരികെ 1;

		എങ്കിൽ (!a_seg || !b_seg)
			നിർത്തുക;
		എങ്കിൽ (a_sz != b_sz || strncmp(a_seg, b_seg, a_sz) != 0)
			നിർത്തുക;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	തിരികെ 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
പൂർണ്ണസംഖ്യ ul_path_is_within(സ്ഥിരം അക്ഷരം *path, സ്ഥിരം അക്ഷരം *dir)
{
	എങ്കിൽ (!path || !dir || !*path || !*dir)
		തിരികെ 0;

	ആകുമ്പോൾ (path && dir) {
		size_t p_sz, d_sz;
		സ്ഥിരം അക്ഷരം *p_seg = ul_path_next_segment(path, &p_sz);
		സ്ഥിരം അക്ഷരം *d_seg = ul_path_next_segment(dir, &d_sz);

		/* all @dir segments matched */
		എങ്കിൽ (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			തിരികെ 1;

		എങ്കിൽ (!p_seg || !d_seg)
			നിർത്തുക;
		എങ്കിൽ (p_sz != d_sz || strncmp(p_seg, d_seg, p_sz) != 0)
			നിർത്തുക;

		path = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	തിരികെ 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
അക്ഷരം *ul_strnconcat(സ്ഥിരം അക്ഷരം *s, സ്ഥിരം അക്ഷരം *suffix, size_t b)
{
	size_t a;
	അക്ഷരം *r;

	എങ്കിൽ (!s && !suffix)
		തിരികെ strdup("");
	എങ്കിൽ (!s)
		തിരികെ strndup(suffix, b);
	എങ്കിൽ (!suffix)
		തിരികെ strdup(s);

	പ്രതിജ്ഞാ(s);
	പ്രതിജ്ഞാ(suffix);

	a = strlen(s);
	എങ്കിൽ (b > ((size_t) -1) - a)
		തിരികെ ശൂന്യ;

	r = malloc(a + b + 1);
	എങ്കിൽ (!r)
		തിരികെ ശൂന്യ;

	memcpy(r, s, a);
	memcpy(r + a, suffix, b);
	r[a+b] = 0;

	തിരികെ r;
}

/* concatenate two strings to a new string */
അക്ഷരം *ul_strconcat(സ്ഥിരം അക്ഷരം *s, സ്ഥിരം അക്ഷരം *suffix)
{
	തിരികെ ul_strnconcat(s, suffix, suffix ? strlen(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
അക്ഷരം *ul_strfconcat(സ്ഥിരം അക്ഷരം *s, സ്ഥിരം അക്ഷരം *format, ...)
{
	va_list ap;
	അക്ഷരം *val, *res;
	പൂർണ്ണസംഖ്യ sz;

	va_start(ap, format);
	sz = vasprintf(&val, format, ap);
	va_end(ap);

	എങ്കിൽ (sz < 0)
		തിരികെ ശൂന്യ;

	res = ul_strnconcat(s, val, sz);
	സ്വതന്ത്രം(val);
	തിരികെ res;
}

പൂർണ്ണസംഖ്യ ul_strappend(അക്ഷരം **a, സ്ഥിരം അക്ഷരം *b)
{
	size_t al, bl;
	അക്ഷരം *tmp;

	എങ്കിൽ (!a)
		തിരികെ -EINVAL;
	എങ്കിൽ (!b || !*b)
		തിരികെ 0;
	എങ്കിൽ (!*a) {
		*a = strdup(b);
		തിരികെ !*a ? -ENOMEM : 0;
	}

	al = strlen(*a);
	bl = strlen(b);

	tmp = realloc(*a, al + bl + 1);
	എങ്കിൽ (!tmp)
		തിരികെ -ENOMEM;
	*a = tmp;
	memcpy((*a) + al, b, bl + 1);
	തിരികെ 0;
}

/* the hybrid version of strfconcat and strappend. */
പൂർണ്ണസംഖ്യ strfappend(അക്ഷരം **a, സ്ഥിരം അക്ഷരം *format, ...)
{
	va_list ap;
	പൂർണ്ണസംഖ്യ res;

	va_start(ap, format);
	res = ul_strvfappend(a, format, ap);
	va_end(ap);

	തിരികെ res;
}

ബാഹ്യ പൂർണ്ണസംഖ്യ ul_strvfappend(അക്ഷരം **a, സ്ഥിരം അക്ഷരം *format, va_list ap)
{
	അക്ഷരം *val;
	പൂർണ്ണസംഖ്യ sz;
	പൂർണ്ണസംഖ്യ res;

	sz = vasprintf(&val, format, ap);
	എങ്കിൽ (sz < 0)
		തിരികെ -errno;

	res = ul_strappend(a, val);
	സ്വതന്ത്രം(val);
	തിരികെ res;
}

സ്ഥിതം size_t strcspn_escaped(സ്ഥിരം അക്ഷരം *s, സ്ഥിരം അക്ഷരം *reject)
{
	പൂർണ്ണസംഖ്യ escaped = 0;
	പൂർണ്ണസംഖ്യ n;

	വേണ്ടി (n=0; s[n]; n++) {
		എങ്കിൽ (escaped)
			escaped = 0;
		അല്ലെങ്കിൽ എങ്കിൽ (s[n] == '\\')
			escaped = 1;
		അല്ലെങ്കിൽ എങ്കിൽ (strchr(reject, s[n]))
			നിർത്തുക;
	}

	/* if s ends in \, return index of previous char */
	തിരികെ n - escaped;
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
അക്ഷരം *ul_strchr_escaped(സ്ഥിരം അക്ഷരം *s, പൂർണ്ണസംഖ്യ c)
{
	അക്ഷരം *p;
	പൂർണ്ണസംഖ്യ esc = 0;

	വേണ്ടി (p = (അക്ഷരം *) s; p && *p; p++) {
		എങ്കിൽ (!esc && *p == '\\') {
			esc = 1;
			തുടരുക;
		}
		എങ്കിൽ (*p == c && (!esc || c == '\\'))
			തിരികെ p;
		esc = 0;
	}

	തിരികെ ശൂന്യ;
}

/* Split a string into words. */
സ്ഥിരം അക്ഷരം *ul_split(സ്ഥിരം അക്ഷരം **state, size_t *l, സ്ഥിരം അക്ഷരം *separator, പൂർണ്ണസംഖ്യ quoted)
{
	സ്ഥിരം അക്ഷരം *current;

	current = *state;

	എങ്കിൽ (!*current) {
		പ്രതിജ്ഞാ(**state == '\0');
		തിരികെ ശൂന്യ;
	}

	current += strspn(current, separator);
	എങ്കിൽ (!*current) {
		*state = current;
		തിരികെ ശൂന്യ;
	}

	എങ്കിൽ (quoted && strchr("\'\"", *current)) {
		അക്ഷരം quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		എങ്കിൽ (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !strchr(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			തിരികെ ശൂന്യ;
		}
		*state = current++ + *l + 2;
	} അല്ലെങ്കിൽ എങ്കിൽ (quoted) {
		*l = strcspn_escaped(current, separator);
		എങ്കിൽ (current[*l] && !strchr(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			തിരികെ ശൂന്യ;
		}
		*state = current + *l;
	} അല്ലെങ്കിൽ {
		*l = strcspn(current, separator);
		*state = current + *l;
	}

	തിരികെ current;
}

/* Rewind file pointer forward to new line.  */
പൂർണ്ണസംഖ്യ skip_fline(FILE *fp)
{
	പൂർണ്ണസംഖ്യ ch;

	ചെയ്യുക {
		എങ്കിൽ ((ch = fgetc(fp)) == EOF)
			തിരികെ 1;
		എങ്കിൽ (ch == '\n')
			തിരികെ 0;
	} ആകുമ്പോൾ (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
പൂർണ്ണസംഖ്യ ul_stralnumcmp(സ്ഥിരം അക്ഷരം *p1, സ്ഥിരം അക്ഷരം *p2)
{
	സ്ഥിരം unsigned അക്ഷരം *s1 = (സ്ഥിരം unsigned അക്ഷരം *) p1;
	സ്ഥിരം unsigned അക്ഷരം *s2 = (സ്ഥിരം unsigned അക്ഷരം *) p2;
	unsigned അക്ഷരം c1, c2;

	ചെയ്യുക {
		ചെയ്യുക {
			c1 = (unsigned അക്ഷരം) *s1++;
		} ആകുമ്പോൾ (c1 != '\0' && !isalnum((unsigned പൂർണ്ണസംഖ്യ) c1));

		ചെയ്യുക {
			c2 = (unsigned അക്ഷരം) *s2++;
		} ആകുമ്പോൾ (c2 != '\0' && !isalnum((unsigned പൂർണ്ണസംഖ്യ) c2));

		എങ്കിൽ (c1 != '\0')
			c1 = tolower(c1);
		എങ്കിൽ (c2 != '\0')
			c2 = tolower(c2);
		എങ്കിൽ (c1 == '\0')
			തിരികെ c1 - c2;
	} ആകുമ്പോൾ (c1 == c2);

	തിരികെ c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
പൂർണ്ണസംഖ്യ ul_optstr_next(അക്ഷരം **optstr, അക്ഷരം **name, size_t *namesz,
		   അക്ഷരം **മൂല്യം, size_t *valsz)
{
	പൂർണ്ണസംഖ്യ open_quote = 0;
	അക്ഷരം *start = ശൂന്യ, *stop = ശൂന്യ, *p, *sep = ശൂന്യ;
	അക്ഷരം *optstr0;

	പ്രതിജ്ഞാ(optstr);
	പ്രതിജ്ഞാ(*optstr);

	optstr0 = *optstr;

	എങ്കിൽ (name)
		*name = ശൂന്യ;
	എങ്കിൽ (namesz)
		*namesz = 0;
	എങ്കിൽ (മൂല്യം)
		*മൂല്യം = ശൂന്യ;
	എങ്കിൽ (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	ആകുമ്പോൾ (optstr0 && *optstr0 == ',')
		optstr0++;

	വേണ്ടി (p = optstr0; p && *p; p++) {
		എങ്കിൽ (!start && *p == '=')
			തിരികെ -EINVAL;
		എങ്കിൽ (!start)
			start = p;		/* beginning of the option item */
		എങ്കിൽ (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		എങ്കിൽ (open_quote)
			തുടരുക;		/* still in quoted block */
		എങ്കിൽ (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		എങ്കിൽ (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		അല്ലെങ്കിൽ എങ്കിൽ (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		എങ്കിൽ (!start || !stop)
			തുടരുക;
		എങ്കിൽ (stop <= start)
			തിരികെ -EINVAL;

		എങ്കിൽ (name)
			*name = start;
		എങ്കിൽ (namesz)
			*namesz = sep ? sep - start : stop - start;
		*optstr = *stop ? stop + 1 : stop;

		എങ്കിൽ (sep) {
			എങ്കിൽ (മൂല്യം)
				*മൂല്യം = sep + 1;
			എങ്കിൽ (valsz)
				*valsz = stop - sep - 1;
		}
		തിരികെ 0;
	}

	തിരികെ 1;				/* end of optstr */
}

പൂർണ്ണസംഖ്യ ul_optstr_is_valid(സ്ഥിരം അക്ഷരം *optstr)
{
	പൂർണ്ണസംഖ്യ rc;
	അക്ഷരം *p = (അക്ഷരം *) optstr;

	ആകുമ്പോൾ ((rc = ul_optstr_next(&p, ശൂന്യ, ശൂന്യ, ശൂന്യ, ശൂന്യ)) == 0);
	തിരികെ rc < 0 ? 0 : 1;
}

അക്ഷരം *ul_optstr_get_മൂല്യം(സ്ഥിരം അക്ഷരം *optstr, സ്ഥിരം അക്ഷരം *താക്കോൽ)
{
	size_t sz, namesz = 0, valsz = 0;
	അക്ഷരം *name = ശൂന്യ, *മൂല്യം = ശൂന്യ;
	അക്ഷരം *p = (അക്ഷരം *) optstr;

	എങ്കിൽ (!optstr || !താക്കോൽ || !*താക്കോൽ)
		തിരികെ ശൂന്യ;

	sz = strlen(താക്കോൽ);
	ആകുമ്പോൾ (ul_optstr_next(&p, &name, &namesz, &മൂല്യം, &valsz) == 0) {
		എങ്കിൽ (namesz != sz || !valsz)
			തുടരുക;
		എങ്കിൽ (strncmp(name, താക്കോൽ, namesz) == 0)
			തിരികെ strndup(മൂല്യം, valsz);
	}
	തിരികെ ശൂന്യ;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
size_t ul_unquote(അക്ഷരം *str)
{
	size_t len;
	അക്ഷരം q;

	എങ്കിൽ (!str || !*str)
		തിരികെ 0;

	len = strlen(str);
	q = *str;

	എങ്കിൽ ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		തിരികെ len;

	memmove(str, str + 1, len - 2);
	str[len - 2] = '\0';
	തിരികെ len - 2;
}

#ifdef TEST_PROGRAM_STRUTILS

ഘടന testS {
	അക്ഷരം *name;
	അക്ഷരം *മൂല്യം;
};

സ്ഥിതം പൂർണ്ണസംഖ്യ test_strdup_to_member(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	ഘടന testS *xx;

	എങ്കിൽ (argc < 3)
		തിരികെ EXIT_FAILURE;

	xx = calloc(1, വലുപ്പം(*xx));
	എങ്കിൽ (!xx)
		err(EXIT_FAILURE, "calloc() failed");

	strdup_to_struct_member(xx, name, argv[1]);
	strdup_to_struct_member(xx, മൂല്യം, argv[2]);

	എങ്കിൽ (strcmp(xx->name, argv[1]) != 0 &&
	    strcmp(xx->മൂല്യം, argv[2]) != 0)
		errx(EXIT_FAILURE, "strdup_to_struct_member() failed");

	printf("1: '%s', 2: '%s'\n", xx->name, xx->മൂല്യം);

	സ്വതന്ത്രം(xx->name);
	സ്വതന്ത്രം(xx->മൂല്യം);
	സ്വതന്ത്രം(xx);
	തിരികെ EXIT_SUCCESS;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ test_strutils_sizes(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	uintmax_t size = 0;
	അക്ഷരം *hum1, *hum2, *hum3;

	എങ്കിൽ (argc < 2)
		തിരികെ EXIT_FAILURE;

	എങ്കിൽ (strtosize(argv[1], &size))
		errx(EXIT_FAILURE, "invalid size '%s' value", argv[1]);

	hum1 = size_to_human_string(SIZE_SUFFIX_1LETTER, size);
	hum2 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE, size);
	hum3 = size_to_human_string(SIZE_SUFFIX_3LETTER |
				    SIZE_SUFFIX_SPACE |
				    SIZE_DECIMAL_2DIGITS, size);

	printf("%25s : %20ju : %8s : %12s : %13s\n", argv[1], size, hum1, hum2, hum3);
	സ്വതന്ത്രം(hum1);
	സ്വതന്ത്രം(hum2);
	സ്വതന്ത്രം(hum3);

	തിരികെ EXIT_SUCCESS;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ test_strutils_cmp_paths(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	പൂർണ്ണസംഖ്യ rc = streq_paths(argv[1], argv[2]);

	എങ്കിൽ (argc < 3)
		തിരികെ EXIT_FAILURE;

	printf("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	തിരികെ EXIT_SUCCESS;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ test_strutils_path_is_within(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	പൂർണ്ണസംഖ്യ rc;

	എങ്കിൽ (argc < 3)
		തിരികെ EXIT_FAILURE;

	rc = ul_path_is_within(argv[1], argv[2]);

	printf("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	തിരികെ EXIT_SUCCESS;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ test_strutils_normalize(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	unsigned അക്ഷരം *src, *dst, *org;
	size_t sz, len;

	എങ്കിൽ (argc < 2)
		തിരികെ EXIT_FAILURE;

	org = (unsigned അക്ഷരം *) strdup(argv[1]);
	src = (unsigned അക്ഷരം *) strdup((അക്ഷരം *) org);
	len = strlen((അക്ഷരം *) src);
	dst = malloc(len + 1);

	എങ്കിൽ (!org || !src || !dst)
		പോകുക done;

	/* two buffers */
	sz = __normalize_whitespace(src, len, dst, len + 1);
	printf("1: '%s' --> '%s' [sz=%zu]\n", src, dst, sz);

	/* one buffer */
	sz = normalize_whitespace(src);
	printf("2: '%s' --> '%s' [sz=%zu]\n", org, src, sz);

done:
	സ്വതന്ത്രം(src);
	സ്വതന്ത്രം(dst);
	സ്വതന്ത്രം(org);

	തിരികെ EXIT_SUCCESS;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ test_strutils_cstrcasecmp(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	അക്ഷരം *a, *b;

	എങ്കിൽ (argc < 3)
		തിരികെ EXIT_FAILURE;

	a = argv[1];
	b = argv[2];

	എങ്കിൽ (!a || !b)
		തിരികെ EXIT_FAILURE;

	printf("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	printf("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	printf("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, strlen(a)));

	തിരികെ EXIT_SUCCESS;
}

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	എങ്കിൽ (argc == 3 && strcmp(argv[1], "--size") == 0) {
		തിരികെ test_strutils_sizes(argc - 1, argv + 1);

	} അല്ലെങ്കിൽ എങ്കിൽ (argc == 3 && strcmp(argv[1], "--parse-switch") == 0) {
		printf("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						ശൂന്യ));
		തിരികെ EXIT_SUCCESS;
	} അല്ലെങ്കിൽ എങ്കിൽ (argc == 3 && strcmp(argv[1], "--strtobool") == 0) {
		bool val;
		എങ്കിൽ (ul_strtobool(argv[2], &val))
			err(EXIT_FAILURE, "could not parse '%s'", argv[2]);
		printf("'%s'-->%s\n", argv[2], val ? "true" : "false");
		തിരികെ EXIT_SUCCESS;

	} അല്ലെങ്കിൽ എങ്കിൽ (argc == 4 && strcmp(argv[1], "--cmp-paths") == 0) {
		തിരികെ test_strutils_cmp_paths(argc - 1, argv + 1);

	} അല്ലെങ്കിൽ എങ്കിൽ (argc == 4 && strcmp(argv[1], "--path-is-within") == 0) {
		തിരികെ test_strutils_path_is_within(argc - 1, argv + 1);

	} അല്ലെങ്കിൽ എങ്കിൽ (argc == 4 && strcmp(argv[1], "--strdup-member") == 0) {
		തിരികെ test_strdup_to_member(argc - 1, argv + 1);

	} അല്ലെങ്കിൽ എങ്കിൽ  (argc == 4 && strcmp(argv[1], "--stralnumcmp") == 0) {
		printf("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		തിരികെ EXIT_SUCCESS;

	} അല്ലെങ്കിൽ എങ്കിൽ (argc == 4 && strcmp(argv[1], "--cstrcasecmp") == 0) {
		തിരികെ test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} അല്ലെങ്കിൽ എങ്കിൽ (argc == 3 && strcmp(argv[1], "--normalize") == 0) {
		തിരികെ test_strutils_normalize(argc - 1, argv + 1);

	} അല്ലെങ്കിൽ എങ്കിൽ (argc == 3 && strcmp(argv[1], "--strtos64") == 0) {
		printf("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		തിരികെ EXIT_SUCCESS;
	} അല്ലെങ്കിൽ എങ്കിൽ (argc == 3 && strcmp(argv[1], "--strtou64") == 0) {
		printf("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		തിരികെ EXIT_SUCCESS;
	} അല്ലെങ്കിൽ എങ്കിൽ (argc == 3 && strcmp(argv[1], "--strtos32") == 0) {
		printf("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		തിരികെ EXIT_SUCCESS;
	} അല്ലെങ്കിൽ എങ്കിൽ (argc == 3 && strcmp(argv[1], "--strtou32") == 0) {
		printf("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		തിരികെ EXIT_SUCCESS;
	} അല്ലെങ്കിൽ എങ്കിൽ (argc == 3 && strcmp(argv[1], "--strtos16") == 0) {
		printf("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		തിരികെ EXIT_SUCCESS;
	} അല്ലെങ്കിൽ എങ്കിൽ (argc == 3 && strcmp(argv[1], "--strtou16") == 0) {
		printf("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		തിരികെ EXIT_SUCCESS;

	} അല്ലെങ്കിൽ എങ്കിൽ (argc == 4 && strcmp(argv[1], "--strchr-escaped") == 0) {
		printf("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		തിരികെ EXIT_SUCCESS;

	} അല്ലെങ്കിൽ എങ്കിൽ (argc == 2 && strcmp(argv[1], "--next-string") == 0) {
		അക്ഷരം *buf = "abc\0Y\0\0xyz\0X";
		അക്ഷരം *end = buf + 12;
		അക്ഷരം *p = buf;

		ചെയ്യുക {
			printf("str: '%s'\n", p);
		} ആകുമ്പോൾ ((p = ul_next_string(p, end)));

		തിരികെ EXIT_SUCCESS;

	} അല്ലെങ്കിൽ എങ്കിൽ (argc == 3 && strcmp(argv[1], "--optstr") == 0) {

		size_t namesz, valsz;
		അക്ഷരം *name = ശൂന്യ, *val = ശൂന്യ;
		അക്ഷരം *p = argv[2];
		പൂർണ്ണസംഖ്യ rc;

		എങ്കിൽ (!ul_optstr_is_valid(p))
			errx(EXIT_FAILURE, _("unsupported option format: %s"), p);

		ആകുമ്പോൾ ((rc = ul_optstr_next(&p, &name, &namesz, &val, &valsz)) == 0) {
			printf("'%.*s' : '%.*s'\n", (പൂർണ്ണസംഖ്യ) namesz, name,
						   (പൂർണ്ണസംഖ്യ) valsz, val);
		}
		എങ്കിൽ (rc == 1)
			തിരികെ EXIT_SUCCESS;
	} അല്ലെങ്കിൽ എങ്കിൽ (argc == 3 && strcmp(argv[1], "--unquote") == 0) {

		അക്ഷരം *unquoted = strdup(argv[2]);

		എങ്കിൽ (!unquoted)
			err(EXIT_FAILURE, "strdup() failed");

		ul_unquote(unquoted);
		printf("%s-->%s\n", argv[2], unquoted);
		സ്വതന്ത്രം(unquoted);
		തിരികെ EXIT_SUCCESS;

	} അല്ലെങ്കിൽ {
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

	തിരികെ EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_STRUTILS */
