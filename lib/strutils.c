/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Karel Zak <kzak@redhat.com> [2010]
 *          Davidlohr Bueso <dave@gnu.org> [2010]
 */
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <inttypes.h>
#समावेश <ctype.h>
#समावेश <errno.h>
#समावेश <sys/stat.h>
#समावेश <string.h>
#समावेश <strings.h>
#समावेश <assert.h>

#समावेश "c.h"
#समावेश "cctype.h"
#समावेश "nls.h"
#समावेश "strutils.h"
#समावेश "timeutils.h"
#समावेश "bitops.h"
#समावेश "pathnames.h"

अटल पूर्णांक STRTOXX_EXIT_CODE = निकास_त्रुटि;

व्योम strutils_set_exitcode(पूर्णांक ex)
{
	STRTOXX_EXIT_CODE = ex;
}

अटल पूर्णांक do_scale_by_power (uintmax_t *x, पूर्णांक base, पूर्णांक power)
{
	जबतक (power--) {
		अगर (UINTMAX_MAX / base < *x)
			वापस -दुस्फल;
		*x *= base;
	}
	वापस 0;
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
पूर्णांक ul_parse_आकार(स्थिर अक्षर *str, uintmax_t *res, पूर्णांक *power)
{
	स्थिर अक्षर *p;
	अक्षर *end;
	uintmax_t x, frac = 0;
	पूर्णांक base = 1024, rc = 0, pwr = 0, frac_zeros = 0;

	अटल स्थिर अक्षर *suf  = "KMGTPEZY";
	अटल स्थिर अक्षर *suf2 = "kmgtpezy";
	स्थिर अक्षर *sp;

	*res = 0;

	अगर (!str || !*str) {
		rc = -EINVAL;
		जाओ err;
	}

	/* Only positive numbers are acceptable
	 *
	 * Note that this check is not perfect, it would be better to
	 * use lconv->negative_sign. But coreutils use the same solution,
	 * so it's probably good enough...
	 */
	p = str;
	जबतक (है_खाली((अचिन्हित अक्षर) *p))
		p++;
	अगर (*p == '-') {
		rc = -EINVAL;
		जाओ err;
	}

	त्रुटि_सं = 0, end = शून्य;
	x = strtoumax(str, &end, 0);

	अगर (end == str ||
	    (त्रुटि_सं != 0 && (x == UINTMAX_MAX || x == 0))) {
		rc = त्रुटि_सं ? -त्रुटि_सं : -EINVAL;
		जाओ err;
	}
	अगर (!end || !*end)
		जाओ done;			/* without suffix */
	p = end;

	/*
	 * Check size suffixes
	 */
check_suffix:
	अगर (*(p + 1) == 'i' && (*(p + 2) == 'B' || *(p + 2) == 'b') && !*(p + 3))
		base = 1024;			/* XiB, 2^N */
	अन्यथा अगर ((*(p + 1) == 'B' || *(p + 1) == 'b') && !*(p + 2))
		base = 1000;			/* XB, 10^N */
	अन्यथा अगर (*(p + 1)) {
		काष्ठा स्था_बदल स्थिर *l = क्षेत्र_बदलो();
		स्थिर अक्षर *dp = l ? l->decimal_point : शून्य;
		माप_प्रकार dpsz = dp ? म_माप(dp) : 0;

		अगर (frac == 0 && *p && dp && म_भेदन(dp, p, dpsz) == 0) {
			स्थिर अक्षर *fstr = p + dpsz;

			क्रम (p = fstr; *p == '0'; p++)
				frac_zeros++;
			fstr = p;
			अगर (है_अंक(*fstr)) {
				त्रुटि_सं = 0, end = शून्य;
				frac = strtoumax(fstr, &end, 0);
				अगर (end == fstr ||
				    (त्रुटि_सं != 0 && (frac == UINTMAX_MAX || frac == 0))) {
					rc = त्रुटि_सं ? -त्रुटि_सं : -EINVAL;
					जाओ err;
				}
			} अन्यथा
				end = (अक्षर *) p;

			अगर (!end || !*end) {
				rc = -EINVAL;
				जाओ err;	/* without suffix, but with fractions */
			}
			p = end;
			जाओ check_suffix;
		}
		rc = -EINVAL;
		जाओ err;			/* unexpected suffix */
	}

	sp = म_अक्षर(suf, *p);
	अगर (sp)
		pwr = (sp - suf) + 1;
	अन्यथा {
		sp = म_अक्षर(suf2, *p);
		अगर (sp)
			pwr = (sp - suf2) + 1;
		अन्यथा {
			rc = -EINVAL;
			जाओ err;
		}
	}

	अगर (power)
		*power = pwr;

	rc = do_scale_by_power(&x, base, pwr);
	अगर (rc)
		जाओ err;

	अगर (frac && pwr) {
		पूर्णांक i;
		uintmax_t frac_base = 1, frac_val = 0;

		/* mega, giga, ... */
		rc = do_scale_by_power(&frac_base, base, pwr);
		अगर (rc)
			जाओ err;

		/*
		 * Go backwardly from last digit and compute frac_base * 0.<frac>
		 * rounded down. For example 0.25G is (5G / 10 + 2G) / 10.
		 * The intermediate value is always smaller than 10 * frac_base.
		 */
		करो {
			frac_val = ((frac % 10) * frac_base + frac_val) / 10;
			frac /= 10;
		} जबतक (frac);

		/* 'frac' is without zeros (5 means 0.5 as well as 0.05) */
		क्रम (i = 0; i < frac_zeros && frac_val; i++)
			frac_val /= 10;

		अगर (UINTMAX_MAX - x < frac_val) {
			rc = -दुस्फल;
			जाओ err;
		}
		x += frac_val;
	}
done:
	*res = x;
err:
	अगर (rc < 0)
		त्रुटि_सं = -rc;
	वापस rc;
}

पूर्णांक strtosize(स्थिर अक्षर *str, uintmax_t *res)
{
	वापस ul_parse_आकार(str, res, शून्य);
}

पूर्णांक isdigit_strend(स्थिर अक्षर *str, स्थिर अक्षर **end)
{
	स्थिर अक्षर *p;

	क्रम (p = str; p && *p && है_अंक((अचिन्हित अक्षर) *p); p++);

	अगर (end)
		*end = p;
	वापस p && p > str && !*p;
}

पूर्णांक isxdigit_strend(स्थिर अक्षर *str, स्थिर अक्षर **end)
{
	स्थिर अक्षर *p;

	क्रम (p = str; p && *p && है_षष्ठादशक((अचिन्हित अक्षर) *p); p++);

	अगर (end)
		*end = p;

	वापस p && p > str && !*p;
}

/*
 *  For example: ul_parse_switch(argv[i], "on", "off",  "yes", "no",  NULL);
 */
पूर्णांक ul_strtobool(स्थिर अक्षर *str, bool *परिणाम)
{
	अटल स्थिर अक्षर *bool_true[]  = { "1", "y", "t", "yes", "true", "on", "enable" };
	अटल स्थिर अक्षर *bool_false[] = { "0", "n", "f", "no", "false", "off", "disable", "not", };
	माप_प्रकार i;

	अगर (!str || !परिणाम)
		वापस -(त्रुटि_सं = EINVAL);

	त्रुटि_सं = 0;

	क्रम (i = 0; i < ARRAY_आकार(bool_true); i++) {
		अगर (c_strcasecmp(str, bool_true[i]) == 0) {
			*परिणाम = true;
			वापस 0;
		}
	}

	क्रम (i = 0; i < ARRAY_आकार(bool_false); i++) {
		अगर (c_strcasecmp(str, bool_false[i]) == 0) {
			*परिणाम = false;
			वापस 0;
		}
	}

	वापस -(त्रुटि_सं = EINVAL);
}

पूर्णांक ul_parse_switch(स्थिर अक्षर *arg, ...)
{
	स्थिर अक्षर *a, *b;
	बहु_सूची ap;

	बहु_शुरू(ap, arg);
	करो {
		a = बहु_तर्क(ap, अक्षर *);
		अगर (!a)
			अवरोध;
		b = बहु_तर्क(ap, अक्षर *);
		अगर (!b)
			अवरोध;

		अगर (म_भेद(arg, a) == 0) {
			बहु_पूर्ण(ap);
			वापस 1;
		}

		अगर (म_भेद(arg, b) == 0) {
			बहु_पूर्ण(ap);
			वापस 0;
		}
	} जबतक (1);
	बहु_पूर्ण(ap);

	errx(STRTOXX_EXIT_CODE, _("unsupported argument: %s"), arg);
}

#अगर_अघोषित HAVE_MEMPCPY
व्योम *mempcpy(व्योम *restrict dest, स्थिर व्योम *restrict src, माप_प्रकार n)
{
	वापस ((अक्षर *)स_नकल(dest, src, n)) + n;
}
#पूर्ण_अगर

#अगर_अघोषित HAVE_STRNLEN
माप_प्रकार strnlen(स्थिर अक्षर *s, माप_प्रकार maxlen)
{
	माप_प्रकार i;

	क्रम (i = 0; i < maxlen; i++) {
		अगर (s[i] == '\0')
			वापस i;
	}
	वापस maxlen;
}
#पूर्ण_अगर

#अगर_अघोषित HAVE_STRNCHR
अक्षर *strnchr(स्थिर अक्षर *s, माप_प्रकार maxlen, पूर्णांक c)
{
	क्रम (; maxlen-- && *s != '\0'; ++s)
		अगर (*s == (अक्षर)c)
			वापस (अक्षर *)s;
	वापस शून्य;
}
#पूर्ण_अगर

#अगर_अघोषित HAVE_STRNDUP
अक्षर *strndup(स्थिर अक्षर *s, माप_प्रकार n)
{
	माप_प्रकार len = strnlen(s, n);
	अक्षर *new = दो_स्मृति((len + 1) * माप(अक्षर));
	अगर (!new)
		वापस शून्य;
	new[len] = '\0';
	वापस (अक्षर *) स_नकल(new, s, len);
}
#पूर्ण_अगर

/*
 * convert strings to numbers; returns <0 on error, and 0 on success
 */
पूर्णांक ul_strtos64(स्थिर अक्षर *str, int64_t *अंक, पूर्णांक base)
{
	अक्षर *end = शून्य;

	अगर (str == शून्य || *str == '\0')
		वापस -(त्रुटि_सं = EINVAL);

	त्रुटि_सं = 0;
	*अंक = (int64_t) strtoimax(str, &end, base);

	अगर (त्रुटि_सं != 0)
		वापस -त्रुटि_सं;
	अगर (str == end || (end && *end))
		वापस -(त्रुटि_सं = EINVAL);
	वापस 0;
}

पूर्णांक ul_strtou64(स्थिर अक्षर *str, uint64_t *अंक, पूर्णांक base)
{
	अक्षर *end = शून्य;
	int64_t tmp;

	अगर (str == शून्य || *str == '\0')
		वापस -(त्रुटि_सं = EINVAL);

	/* we need to ignore negative numbers, note that for invalid negative
	 * number strtoimax() returns negative number too, so we do not
	 * need to check errno here */
	त्रुटि_सं = 0;
	tmp = (int64_t) strtoimax(str, &end, base);
	अगर (tmp < 0)
		त्रुटि_सं = दुस्फल;
	अन्यथा {
		त्रुटि_सं = 0;
		*अंक = strtoumax(str, &end, base);
	}

	अगर (त्रुटि_सं != 0)
		वापस -त्रुटि_सं;
	अगर (str == end || (end && *end))
		वापस -(त्रुटि_सं = EINVAL);
	वापस 0;
}

पूर्णांक ul_strtos32(स्थिर अक्षर *str, int32_t *अंक, पूर्णांक base)
{
	int64_t tmp;
	पूर्णांक rc;

	rc = ul_strtos64(str, &tmp, base);
	अगर (rc == 0 && (tmp < INT32_MIN || tmp > INT32_MAX))
		rc = -(त्रुटि_सं = दुस्फल);
	अगर (rc == 0)
		*अंक = (int32_t) tmp;
	वापस rc;
}

पूर्णांक ul_strtou32(स्थिर अक्षर *str, uint32_t *अंक, पूर्णांक base)
{
	uint64_t tmp;
	पूर्णांक rc;

	rc = ul_strtou64(str, &tmp, base);
	अगर (rc == 0 && tmp > UINT32_MAX)
		rc = -(त्रुटि_सं = दुस्फल);
	अगर (rc == 0)
		*अंक = (uint32_t) tmp;
	वापस rc;
}

पूर्णांक ul_strtou16(स्थिर अक्षर *str, uint16_t *अंक, पूर्णांक base)
{
	uint64_t tmp;
	पूर्णांक rc;

	rc = ul_strtou64(str, &tmp, base);
	अगर (rc == 0 && tmp > UINT16_MAX)
		rc = -(त्रुटि_सं = दुस्फल);
	अगर (rc == 0)
		*अंक = (uint16_t) tmp;
	वापस rc;
}

/*
 * Convert strings to numbers in defined range and print message on error.
 *
 * These functions are used when we read input from users (getopt() etc.). It's
 * better to consolidate the code and keep it all based on 64-bit numbers than
 * implement it for 32 and 16-bit numbers too.
 */
int64_t str2num_or_err(स्थिर अक्षर *str, पूर्णांक base, स्थिर अक्षर *errmesg,
			     int64_t निचला, int64_t up)
{
	int64_t अंक = 0;
	पूर्णांक rc;

	rc = ul_strtos64(str, &अंक, base);
	अगर (rc == 0 && ((निचला && अंक < निचला) || (up && अंक > up)))
		rc = -(त्रुटि_सं = दुस्फल);

	अगर (rc) {
		अगर (त्रुटि_सं == दुस्फल)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	वापस अंक;
}

uint64_t str2unum_or_err(स्थिर अक्षर *str, पूर्णांक base, स्थिर अक्षर *errmesg, uint64_t up)
{
	uint64_t अंक = 0;
	पूर्णांक rc;

	rc = ul_strtou64(str, &अंक, base);
	अगर (rc == 0 && (up && अंक > up))
		rc = -(त्रुटि_सं = दुस्फल);

	अगर (rc) {
		अगर (त्रुटि_सं == दुस्फल)
			err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
		errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
	}
	वापस अंक;
}

द्विगुन strtod_or_err(स्थिर अक्षर *str, स्थिर अक्षर *errmesg)
{
	द्विगुन अंक;
	अक्षर *end = शून्य;

	त्रुटि_सं = 0;
	अगर (str == शून्य || *str == '\0')
		जाओ err;
	अंक = म_से_भग्न(str, &end);

	अगर (त्रुटि_सं || str == end || (end && *end))
		जाओ err;

	वापस अंक;
err:
	अगर (त्रुटि_सं == दुस्फल)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

पूर्णांक ul_strtold(स्थिर अक्षर *str, दीर्घ द्विगुन *अंक)
{
	अक्षर *end = शून्य;

	त्रुटि_सं = 0;
	अगर (str == शून्य || *str == '\0')
		वापस -(त्रुटि_सं = EINVAL);
	*अंक = strtold(str, &end);

	अगर (त्रुटि_सं != 0)
		वापस -त्रुटि_सं;
	अगर (str == end || (end && *end))
		वापस -(त्रुटि_सं = EINVAL);
	वापस 0;
}

दीर्घ द्विगुन strtold_or_err(स्थिर अक्षर *str, स्थिर अक्षर *errmesg)
{
	दीर्घ द्विगुन अंक = 0;

	अगर (ul_strtold(str, &अंक) == 0)
		वापस अंक;
	अगर (त्रुटि_सं == दुस्फल)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}

uintmax_t strtosize_or_err(स्थिर अक्षर *str, स्थिर अक्षर *errmesg)
{
	uintmax_t अंक;

	अगर (strtosize(str, &अंक) == 0)
		वापस अंक;

	अगर (त्रुटि_सं)
		err(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);

	errx(STRTOXX_EXIT_CODE, "%s: '%s'", errmesg, str);
}


व्योम strtotimeval_or_err(स्थिर अक्षर *str, काष्ठा timeval *tv, स्थिर अक्षर *errmesg)
{
	दीर्घ द्विगुन user_निवेश;

	user_निवेश = strtold_or_err(str, errmesg);
	tv->tv_sec = (समय_प्रकार) user_निवेश;
	tv->tv_usec = (suseconds_t)((user_निवेश - tv->tv_sec) * USEC_PER_SEC);
}

व्योम strtotimespec_or_err(स्थिर अक्षर *str, काष्ठा timespec *ts, स्थिर अक्षर *errmesg)
{
	दीर्घ द्विगुन user_निवेश;

	user_निवेश = strtold_or_err(str, errmesg);
	ts->tv_sec = (समय_प्रकार) user_निवेश;
	ts->tv_nsec = (दीर्घ)((user_निवेश - ts->tv_sec) * NSEC_PER_SEC);
}

समय_प्रकार strtotime_or_err(स्थिर अक्षर *str, स्थिर अक्षर *errmesg)
{
	int64_t user_निवेश;

	user_निवेश = strtos64_or_err(str, errmesg);
	वापस (समय_प्रकार) user_निवेश;
}

bool hyperlinkwanted(स्थिर अक्षर *mode)
{
	अगर (mode && म_भेद(mode, "never") == 0)
		वापस false;

	अगर (mode && म_भेद(mode, "always") == 0)
		वापस true;

	अगर (!mode || म_भेद(mode, "auto") == 0)
		वापस isatty(STDOUT_FILENO) ? true : false;

	errx(निकास_त्रुटि, _("invalid argument of --hyperlink: %s"), mode);
}

bool annotationwanted(स्थिर अक्षर *mode)
{
	अगर (mode && म_भेद(mode, "never") == 0)
		वापस false;

	अगर (mode && म_भेद(mode, "always") == 0)
		वापस true;

	अगर (!mode || म_भेद(mode, "auto") == 0)
		वापस isatty(STDOUT_FILENO) ? true : false;

	errx(निकास_त्रुटि, _("invalid argument of --annotate: %s"), mode);
}

/*
 * Converts stat->st_mode to ls(1)-like mode string. The size of "str" must
 * be 11 bytes.
 */
अक्षर *xstrmode(mode_t mode, अक्षर *str)
{
	अचिन्हित लघु i = 0;

	अगर (S_ISDIR(mode))
		str[i++] = 'd';
	अन्यथा अगर (S_ISLNK(mode))
		str[i++] = 'l';
	अन्यथा अगर (S_ISCHR(mode))
		str[i++] = 'c';
	अन्यथा अगर (S_ISBLK(mode))
		str[i++] = 'b';
	अन्यथा अगर (S_ISSOCK(mode))
		str[i++] = 's';
	अन्यथा अगर (S_ISFIFO(mode))
		str[i++] = 'p';
	अन्यथा अगर (S_ISREG(mode))
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

	वापस str;
}

/*
 * returns exponent (2^x=n) in range KiB..EiB (2^10..2^60)
 */
अटल पूर्णांक get_exp(uint64_t n)
{
	पूर्णांक shft;

	क्रम (shft = 10; shft <= 60; shft += 10) {
		अगर (n < (1ULL << shft))
			अवरोध;
	}
	वापस shft - 10;
}

अक्षर *आकार_to_human_string(पूर्णांक options, uint64_t bytes)
{
	अक्षर buf[32];
	पूर्णांक dec, exp;
	uint64_t frac;
	स्थिर अक्षर *letters = "BKMGTPE";
	अक्षर suffix[माप(" KiB")], *psuf = suffix;
	अक्षर c;

	अगर (options & आकार_SUFFIX_SPACE)
		*psuf++ = ' ';


	exp  = get_exp(bytes);
	c    = *(letters + (exp ? exp / 10 : 0));
	dec  = exp ? bytes / (1ULL << exp) : bytes;
	frac = exp ? bytes % (1ULL << exp) : 0;

	*psuf++ = c;

	अगर ((options & आकार_SUFFIX_3_LETTER) && (c != 'B')) {
		*psuf++ = 'i';
		*psuf++ = 'B';
	}

	*psuf = '\0';

	/* fprintf(stderr, "exp: %d, unit: %c, dec: %d, frac: %jd\n",
	 *                 exp, suffix[0], dec, frac);
	 */

	/* round */
	अगर (frac) {
		/* get 3 digits after decimal point */
		अगर (frac >= UINT64_MAX / 1000)
			frac = ((frac / 1024) * 1000) / (1ULL << (exp - 10)) ;
		अन्यथा
			frac = (frac * 1000) / (1ULL << (exp)) ;

		अगर (options & आकार_DECIMAL_2_DIGITS) {
			/* round 4/5 and keep 2 digits after decimal point */
			frac = (frac + 5) / 10 ;
		} अन्यथा {
			/* round 4/5 and keep 1 digit after decimal point */
			frac = ((frac + 50) / 100) * 10 ;
		}

		/* rounding could have overflowed */
		अगर (frac == 100) {
			dec++;
			frac = 0;
		}
	}

	अगर (frac) {
		काष्ठा स्था_बदल स्थिर *l = क्षेत्र_बदलो();
		अक्षर *dp = l ? l->decimal_point : शून्य;
		पूर्णांक len;

		अगर (!dp || !*dp)
			dp = ".";

		len = snprintf(buf, माप(buf), "%d%s%02" PRIu64, dec, dp, frac);
		अगर (len > 0 && (माप_प्रकार) len < माप(buf)) {
			/* remove potential extraneous zero */
			अगर (buf[len - 1] == '0')
				buf[len--] = '\0';
			/* append suffix */
			xstrncpy(buf+len, suffix, माप(buf) - len);
		} अन्यथा
			*buf = '\0';	/* snprintf error */
	} अन्यथा
		snprintf(buf, माप(buf), "%d%s", dec, suffix);

	वापस strdup(buf);
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
पूर्णांक string_to_idarray(स्थिर अक्षर *list, पूर्णांक ary[], माप_प्रकार arysz,
			पूर्णांक (name2id)(स्थिर अक्षर *, माप_प्रकार))
{
	स्थिर अक्षर *begin = शून्य, *p;
	माप_प्रकार n = 0;

	अगर (!list || !*list || !ary || !arysz || !name2id)
		वापस -1;

	क्रम (p = list; p && *p; p++) {
		स्थिर अक्षर *end = शून्य;
		पूर्णांक id;

		अगर (n >= arysz)
			वापस -2;
		अगर (!begin)
			begin = p;		/* begin of the column name */
		अगर (*p == ',')
			end = p;		/* terminate the name */
		अगर (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		अगर (!begin || !end)
			जारी;
		अगर (end <= begin)
			वापस -1;

		id = name2id(begin, end - begin);
		अगर (id == -1)
			वापस -1;
		ary[ n++ ] = id;
		begin = शून्य;
		अगर (end && !*end)
			अवरोध;
	}
	वापस n;
}

/*
 * Parses the array like string_to_idarray but if format is "+aaa,bbb"
 * it adds fields to array instead of replacing them.
 */
पूर्णांक string_add_to_idarray(स्थिर अक्षर *list, पूर्णांक ary[], माप_प्रकार arysz,
			माप_प्रकार *ary_pos, पूर्णांक (name2id)(स्थिर अक्षर *, माप_प्रकार))
{
	स्थिर अक्षर *list_add;
	पूर्णांक r;

	अगर (!list || !*list || !ary_pos || *ary_pos > arysz)
		वापस -1;

	अगर (list[0] == '+')
		list_add = &list[1];
	अन्यथा {
		list_add = list;
		*ary_pos = 0;
	}

	r = string_to_idarray(list_add, &ary[*ary_pos], arysz - *ary_pos, name2id);
	अगर (r > 0)
		*ary_pos += r;
	वापस r;
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
पूर्णांक string_to_bitarray(स्थिर अक्षर *list,
		     अक्षर *ary,
		     पूर्णांक (*name2bit)(स्थिर अक्षर *, माप_प्रकार),
		     माप_प्रकार allow_range)
{
	स्थिर अक्षर *begin = शून्य, *p;

	अगर (!list || !name2bit || !ary)
		वापस -EINVAL;

	क्रम (p = list; p && *p; p++) {
		स्थिर अक्षर *end = शून्य;
		पूर्णांक bit, set_lower = 0, set_higher = 0;

		अगर (!begin)
			begin = p;		/* begin of the level name */
		अगर (*p == ',')
			end = p;		/* terminate the name */
		अगर (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		अगर (!begin || !end)
			जारी;
		अगर (end <= begin)
			वापस -1;
		अगर (allow_range) {
			अगर (*(end - 1) == '+') {
				end--;
				set_lower = 1;
			} अन्यथा अगर (*begin == '+') {
				begin++;
				set_higher = 1;
			}
		}

		bit = name2bit(begin, end - begin);
		अगर (bit < 0)
			वापस bit;
		setbit(ary, bit);
		अगर (set_lower)
			जबतक (--bit >= 0)
				setbit(ary, bit);
		अन्यथा अगर (set_higher)
			जबतक (++bit < (पूर्णांक) allow_range)
				setbit(ary, bit);
		begin = शून्य;
		अगर (end && !*end)
			अवरोध;
	}
	वापस 0;
}

/*
 * LIST ::= <item> [, <item>]
 *
 * The <item> is translated to 'id' by name2flag() function and the flags is
 * set to the 'mask'
*
 * Returns: 0 on success, <0 on error.
 */
पूर्णांक string_to_bitmask(स्थिर अक्षर *list,
		     अचिन्हित दीर्घ *mask,
		     दीर्घ (*name2flag)(स्थिर अक्षर *, माप_प्रकार))
{
	स्थिर अक्षर *begin = शून्य, *p;

	अगर (!list || !name2flag || !mask)
		वापस -EINVAL;

	क्रम (p = list; p && *p; p++) {
		स्थिर अक्षर *end = शून्य;
		दीर्घ flag;

		अगर (!begin)
			begin = p;		/* begin of the level name */
		अगर (*p == ',')
			end = p;		/* terminate the name */
		अगर (*(p + 1) == '\0')
			end = p + 1;		/* end of string */
		अगर (!begin || !end)
			जारी;
		अगर (end <= begin)
			वापस -1;

		flag = name2flag(begin, end - begin);
		अगर (flag < 0)
			वापस flag;	/* error */
		*mask |= flag;
		begin = शून्य;
		अगर (end && !*end)
			अवरोध;
	}
	वापस 0;
}

/*
 * Parse the lower and higher values in a string containing
 * "lower:higher" or "lower-higher" format. Note that either
 * the lower or the higher values may be missing, and the def
 * value will be assigned to it by default.
 *
 * Returns: 0 on success, <0 on error.
 */
पूर्णांक ul_parse_range(स्थिर अक्षर *str, पूर्णांक *lower, पूर्णांक *upper, पूर्णांक def)
{
	अक्षर *end = शून्य;

	अगर (!str)
		वापस 0;

	*upper = *lower = def;
	त्रुटि_सं = 0;

	अगर (*str == ':') {				/* <:N> */
		str++;
		*upper = म_से_दीर्घ(str, &end, 10);
		अगर (त्रुटि_सं || !end || *end || end == str)
			वापस -1;
	} अन्यथा {
		*upper = *lower = म_से_दीर्घ(str, &end, 10);
		अगर (त्रुटि_सं || !end || end == str)
			वापस -1;

		अगर (*end == ':' && !*(end + 1))		/* <M:> */
			*upper = def;
		अन्यथा अगर (*end == '-' || *end == ':') {	/* <M:N> <M-N> */
			str = end + 1;
			end = शून्य;
			त्रुटि_सं = 0;
			*upper = म_से_दीर्घ(str, &end, 10);

			अगर (त्रुटि_सं || !end || *end || end == str)
				वापस -1;
		}
	}
	वापस 0;
}

स्थिर अक्षर *ul_path_अग्र_segment(स्थिर अक्षर *str, माप_प्रकार *sz)
{
	स्थिर अक्षर *start, *p;

	start = str;
	*sz = 0;
	जबतक (start && *start == '/' && *(start + 1) == '/')
		start++;

	अगर (!start || !*start)
		वापस शून्य;

	क्रम (*sz = 1, p = start + 1; *p && *p != '/'; p++) {
		(*sz)++;
	}

	वापस start;
}

पूर्णांक streq_paths(स्थिर अक्षर *a, स्थिर अक्षर *b)
{
	जबतक (a && b) {
		माप_प्रकार a_sz, b_sz;
		स्थिर अक्षर *a_seg = ul_path_अग्र_segment(a, &a_sz);
		स्थिर अक्षर *b_seg = ul_path_अग्र_segment(b, &b_sz);

		/*
		fprintf(stderr, "A>>>(%zu) '%s'\n", a_sz, a_seg);
		fprintf(stderr, "B>>>(%zu) '%s'\n", b_sz, b_seg);
		*/

		/* end of the path */
		अगर (a_sz + b_sz == 0)
			वापस 1;

		/* ignore trailing slash */
		अगर (a_sz + b_sz == 1 &&
		    ((a_seg && *a_seg == '/') || (b_seg && *b_seg == '/')))
			वापस 1;

		अगर (!a_seg || !b_seg)
			अवरोध;
		अगर (a_sz != b_sz || म_भेदन(a_seg, b_seg, a_sz) != 0)
			अवरोध;

		a = a_seg + a_sz;
		b = b_seg + b_sz;
	};

	वापस 0;
}

/* Returns 1 if @path is @dir or a path below @dir, compared like streq_paths() */
पूर्णांक ul_path_is_within(स्थिर अक्षर *path, स्थिर अक्षर *dir)
{
	अगर (!path || !dir || !*path || !*dir)
		वापस 0;

	जबतक (path && dir) {
		माप_प्रकार p_sz, d_sz;
		स्थिर अक्षर *p_seg = ul_path_अग्र_segment(path, &p_sz);
		स्थिर अक्षर *d_seg = ul_path_अग्र_segment(dir, &d_sz);

		/* all @dir segments matched */
		अगर (d_sz == 0 || (d_sz == 1 && d_seg && *d_seg == '/'))
			वापस 1;

		अगर (!p_seg || !d_seg)
			अवरोध;
		अगर (p_sz != d_sz || म_भेदन(p_seg, d_seg, p_sz) != 0)
			अवरोध;

		path = p_seg + p_sz;
		dir = d_seg + d_sz;
	}

	वापस 0;
}

/* concatenate two strings to a new string, the size of the second string is limited by @b */
अक्षर *ul_strnconcat(स्थिर अक्षर *s, स्थिर अक्षर *suffix, माप_प्रकार b)
{
	माप_प्रकार a;
	अक्षर *r;

	अगर (!s && !suffix)
		वापस strdup("");
	अगर (!s)
		वापस strndup(suffix, b);
	अगर (!suffix)
		वापस strdup(s);

	निश्चित(s);
	निश्चित(suffix);

	a = म_माप(s);
	अगर (b > ((माप_प्रकार) -1) - a)
		वापस शून्य;

	r = दो_स्मृति(a + b + 1);
	अगर (!r)
		वापस शून्य;

	स_नकल(r, s, a);
	स_नकल(r + a, suffix, b);
	r[a+b] = 0;

	वापस r;
}

/* concatenate two strings to a new string */
अक्षर *ul_strconcat(स्थिर अक्षर *s, स्थिर अक्षर *suffix)
{
	वापस ul_strnconcat(s, suffix, suffix ? म_माप(suffix) : 0);
}

/* concatenate @s and string defined by @format to a new string */
अक्षर *ul_strfconcat(स्थिर अक्षर *s, स्थिर अक्षर *format, ...)
{
	बहु_सूची ap;
	अक्षर *val, *res;
	पूर्णांक sz;

	बहु_शुरू(ap, format);
	sz = vasprintf(&val, format, ap);
	बहु_पूर्ण(ap);

	अगर (sz < 0)
		वापस शून्य;

	res = ul_strnconcat(s, val, sz);
	मुक्त(val);
	वापस res;
}

पूर्णांक ul_strappend(अक्षर **a, स्थिर अक्षर *b)
{
	माप_प्रकार al, bl;
	अक्षर *tmp;

	अगर (!a)
		वापस -EINVAL;
	अगर (!b || !*b)
		वापस 0;
	अगर (!*a) {
		*a = strdup(b);
		वापस !*a ? -ENOMEM : 0;
	}

	al = म_माप(*a);
	bl = म_माप(b);

	tmp = पुनः_स्मृति(*a, al + bl + 1);
	अगर (!tmp)
		वापस -ENOMEM;
	*a = tmp;
	स_नकल((*a) + al, b, bl + 1);
	वापस 0;
}

/* the hybrid version of strfconcat and strappend. */
पूर्णांक strfappend(अक्षर **a, स्थिर अक्षर *format, ...)
{
	बहु_सूची ap;
	पूर्णांक res;

	बहु_शुरू(ap, format);
	res = ul_strvfappend(a, format, ap);
	बहु_पूर्ण(ap);

	वापस res;
}

बाह्य पूर्णांक ul_strvfappend(अक्षर **a, स्थिर अक्षर *format, बहु_सूची ap)
{
	अक्षर *val;
	पूर्णांक sz;
	पूर्णांक res;

	sz = vasprintf(&val, format, ap);
	अगर (sz < 0)
		वापस -त्रुटि_सं;

	res = ul_strappend(a, val);
	मुक्त(val);
	वापस res;
}

अटल माप_प्रकार strcspn_escaped(स्थिर अक्षर *s, स्थिर अक्षर *reject)
{
	पूर्णांक escaped = 0;
	पूर्णांक n;

	क्रम (n=0; s[n]; n++) {
		अगर (escaped)
			escaped = 0;
		अन्यथा अगर (s[n] == '\\')
			escaped = 1;
		अन्यथा अगर (म_अक्षर(reject, s[n]))
			अवरोध;
	}

	/* if s ends in \, return index of previous char */
	वापस n - escaped;
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
अक्षर *ul_strchr_escaped(स्थिर अक्षर *s, पूर्णांक c)
{
	अक्षर *p;
	पूर्णांक esc = 0;

	क्रम (p = (अक्षर *) s; p && *p; p++) {
		अगर (!esc && *p == '\\') {
			esc = 1;
			जारी;
		}
		अगर (*p == c && (!esc || c == '\\'))
			वापस p;
		esc = 0;
	}

	वापस शून्य;
}

/* Split a string into words. */
स्थिर अक्षर *ul_split(स्थिर अक्षर **state, माप_प्रकार *l, स्थिर अक्षर *separator, पूर्णांक quoted)
{
	स्थिर अक्षर *current;

	current = *state;

	अगर (!*current) {
		निश्चित(**state == '\0');
		वापस शून्य;
	}

	current += म_अखोज(current, separator);
	अगर (!*current) {
		*state = current;
		वापस शून्य;
	}

	अगर (quoted && म_अक्षर("\'\"", *current)) {
		अक्षर quotechars[2] = {*current, '\0'};

		*l = strcspn_escaped(current + 1, quotechars);
		अगर (current[*l + 1] == '\0' || current[*l + 1] != quotechars[0] ||
		    (current[*l + 2] && !म_अक्षर(separator, current[*l + 2]))) {
			/* right quote missing or garbage at the end */
			*state = current;
			वापस शून्य;
		}
		*state = current++ + *l + 2;
	} अन्यथा अगर (quoted) {
		*l = strcspn_escaped(current, separator);
		अगर (current[*l] && !म_अक्षर(separator, current[*l])) {
			/* unfinished escape */
			*state = current;
			वापस शून्य;
		}
		*state = current + *l;
	} अन्यथा {
		*l = म_खोज(current, separator);
		*state = current + *l;
	}

	वापस current;
}

/* Rewind file pointer forward to new line.  */
पूर्णांक skip_fline(खाता *fp)
{
	पूर्णांक ch;

	करो {
		अगर ((ch = ख_अक्षर_लो(fp)) == खातापूर्ण)
			वापस 1;
		अगर (ch == '\n')
			वापस 0;
	} जबतक (1);
}


/* compare two strings, but ignoring non-alnum and case of the characters, for example
 * "Hello (123)!" is the same as "hello123".
 */
पूर्णांक ul_stralnumcmp(स्थिर अक्षर *p1, स्थिर अक्षर *p2)
{
	स्थिर अचिन्हित अक्षर *s1 = (स्थिर अचिन्हित अक्षर *) p1;
	स्थिर अचिन्हित अक्षर *s2 = (स्थिर अचिन्हित अक्षर *) p2;
	अचिन्हित अक्षर c1, c2;

	करो {
		करो {
			c1 = (अचिन्हित अक्षर) *s1++;
		} जबतक (c1 != '\0' && !है_अक्षर_अंक((अचिन्हित पूर्णांक) c1));

		करो {
			c2 = (अचिन्हित अक्षर) *s2++;
		} जबतक (c2 != '\0' && !है_अक्षर_अंक((अचिन्हित पूर्णांक) c2));

		अगर (c1 != '\0')
			c1 = छोटे(c1);
		अगर (c2 != '\0')
			c2 = छोटे(c2);
		अगर (c1 == '\0')
			वापस c1 - c2;
	} जबतक (c1 == c2);

	वापस c1 - c2;
}

/*
 * Parses the first option from @optstr. The @optstr pointer is set to the beginning
 * of the next option. The options string looks like 'aaa,bbb=data,foo,bar="xxx"'.
 *
 * Note this function is used by libmount to parse mount options. Be careful when modify.
 *
 * Returns -EINVAL on parse error, 1 at the end of optstr and 0 on success.
 */
पूर्णांक ul_optstr_अग्र(अक्षर **optstr, अक्षर **name, माप_प्रकार *namesz,
		   अक्षर **मूल्य, माप_प्रकार *valsz)
{
	पूर्णांक open_quote = 0;
	अक्षर *start = शून्य, *stop = शून्य, *p, *sep = शून्य;
	अक्षर *optstr0;

	निश्चित(optstr);
	निश्चित(*optstr);

	optstr0 = *optstr;

	अगर (name)
		*name = शून्य;
	अगर (namesz)
		*namesz = 0;
	अगर (मूल्य)
		*मूल्य = शून्य;
	अगर (valsz)
		*valsz = 0;

	/* trim leading commas as to not invalidate option
	 * strings with multiple consecutive commas */
	जबतक (optstr0 && *optstr0 == ',')
		optstr0++;

	क्रम (p = optstr0; p && *p; p++) {
		अगर (!start && *p == '=')
			वापस -EINVAL;
		अगर (!start)
			start = p;		/* beginning of the option item */
		अगर (*p == '"' && (p == optstr0 || *(p - 1) != '\\'))
			open_quote ^= 1;	/* reverse the status */
		अगर (open_quote)
			जारी;		/* still in quoted block */
		अगर (!sep && p > start && *p == '=')
			sep = p;		/* name and value separator */
		अगर (*p == ',' && (p == optstr0 || *(p - 1) != '\\'))
			stop = p;		/* terminate the option item */
		अन्यथा अगर (*(p + 1) == '\0')
			stop = p + 1;		/* end of optstr */
		अगर (!start || !stop)
			जारी;
		अगर (stop <= start)
			वापस -EINVAL;

		अगर (name)
			*name = start;
		अगर (namesz)
			*namesz = sep ? sep - start : stop - start;
		*optstr = *stop ? stop + 1 : stop;

		अगर (sep) {
			अगर (मूल्य)
				*मूल्य = sep + 1;
			अगर (valsz)
				*valsz = stop - sep - 1;
		}
		वापस 0;
	}

	वापस 1;				/* end of optstr */
}

पूर्णांक ul_optstr_is_valid(स्थिर अक्षर *optstr)
{
	पूर्णांक rc;
	अक्षर *p = (अक्षर *) optstr;

	जबतक ((rc = ul_optstr_अग्र(&p, शून्य, शून्य, शून्य, शून्य)) == 0);
	वापस rc < 0 ? 0 : 1;
}

अक्षर *ul_optstr_get_मूल्य(स्थिर अक्षर *optstr, स्थिर अक्षर *कुंजी)
{
	माप_प्रकार sz, namesz = 0, valsz = 0;
	अक्षर *name = शून्य, *मूल्य = शून्य;
	अक्षर *p = (अक्षर *) optstr;

	अगर (!optstr || !कुंजी || !*कुंजी)
		वापस शून्य;

	sz = म_माप(कुंजी);
	जबतक (ul_optstr_अग्र(&p, &name, &namesz, &मूल्य, &valsz) == 0) {
		अगर (namesz != sz || !valsz)
			जारी;
		अगर (म_भेदन(name, कुंजी, namesz) == 0)
			वापस strndup(मूल्य, valsz);
	}
	वापस शून्य;
}

/* Removes one pair of matching surrounding quotes ('' or "") from @str.
 *
 * Returns size of the new string (without \0).
 */
माप_प्रकार ul_unquote(अक्षर *str)
{
	माप_प्रकार len;
	अक्षर q;

	अगर (!str || !*str)
		वापस 0;

	len = म_माप(str);
	q = *str;

	अगर ((q != '"' && q != '\'') || len < 2 || str[len - 1] != q)
		वापस len;

	स_हटाओ(str, str + 1, len - 2);
	str[len - 2] = '\0';
	वापस len - 2;
}

#अगर_घोषित TEST_PROGRAM_STRUTILS

काष्ठा testS {
	अक्षर *name;
	अक्षर *मूल्य;
};

अटल पूर्णांक test_strdup_to_member(पूर्णांक argc, अक्षर *argv[])
{
	काष्ठा testS *xx;

	अगर (argc < 3)
		वापस निकास_त्रुटि;

	xx = सुस्मृति(1, माप(*xx));
	अगर (!xx)
		err(निकास_त्रुटि, "calloc() failed");

	strdup_to_struct_member(xx, name, argv[1]);
	strdup_to_struct_member(xx, मूल्य, argv[2]);

	अगर (म_भेद(xx->name, argv[1]) != 0 &&
	    म_भेद(xx->मूल्य, argv[2]) != 0)
		errx(निकास_त्रुटि, "strdup_to_struct_member() failed");

	म_लिखो("1: '%s', 2: '%s'\n", xx->name, xx->मूल्य);

	मुक्त(xx->name);
	मुक्त(xx->मूल्य);
	मुक्त(xx);
	वापस निकास_सफल;
}

अटल पूर्णांक test_strutils_sizes(पूर्णांक argc, अक्षर *argv[])
{
	uintmax_t आकार = 0;
	अक्षर *hum1, *hum2, *hum3;

	अगर (argc < 2)
		वापस निकास_त्रुटि;

	अगर (strtosize(argv[1], &आकार))
		errx(निकास_त्रुटि, "invalid size '%s' value", argv[1]);

	hum1 = आकार_to_human_string(आकार_SUFFIX_1_LETTER, आकार);
	hum2 = आकार_to_human_string(आकार_SUFFIX_3_LETTER |
				    आकार_SUFFIX_SPACE, आकार);
	hum3 = आकार_to_human_string(आकार_SUFFIX_3_LETTER |
				    आकार_SUFFIX_SPACE |
				    आकार_DECIMAL_2_DIGITS, आकार);

	म_लिखो("%25s : %20ju : %8s : %12s : %13s\n", argv[1], आकार, hum1, hum2, hum3);
	मुक्त(hum1);
	मुक्त(hum2);
	मुक्त(hum3);

	वापस निकास_सफल;
}

अटल पूर्णांक test_strutils_cmp_paths(पूर्णांक argc, अक्षर *argv[])
{
	पूर्णांक rc = streq_paths(argv[1], argv[2]);

	अगर (argc < 3)
		वापस निकास_त्रुटि;

	म_लिखो("%s: '%s' '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	वापस निकास_सफल;
}

अटल पूर्णांक test_strutils_path_is_within(पूर्णांक argc, अक्षर *argv[])
{
	पूर्णांक rc;

	अगर (argc < 3)
		वापस निकास_त्रुटि;

	rc = ul_path_is_within(argv[1], argv[2]);

	म_लिखो("%s: '%s' within '%s'\n", rc == 1 ? "YES" : "NOT", argv[1], argv[2]);
	वापस निकास_सफल;
}

अटल पूर्णांक test_strutils_normalize(पूर्णांक argc, अक्षर *argv[])
{
	अचिन्हित अक्षर *src, *dst, *org;
	माप_प्रकार sz, len;

	अगर (argc < 2)
		वापस निकास_त्रुटि;

	org = (अचिन्हित अक्षर *) strdup(argv[1]);
	src = (अचिन्हित अक्षर *) strdup((अक्षर *) org);
	len = म_माप((अक्षर *) src);
	dst = दो_स्मृति(len + 1);

	अगर (!org || !src || !dst)
		जाओ done;

	/* two buffers */
	sz = __normalize_whitespace(src, len, dst, len + 1);
	म_लिखो("1: '%s' --> '%s' [sz=%zu]\n", src, dst, sz);

	/* one buffer */
	sz = normalize_whitespace(src);
	म_लिखो("2: '%s' --> '%s' [sz=%zu]\n", org, src, sz);

done:
	मुक्त(src);
	मुक्त(dst);
	मुक्त(org);

	वापस निकास_सफल;
}

अटल पूर्णांक test_strutils_cstrcasecmp(पूर्णांक argc, अक्षर *argv[])
{
	अक्षर *a, *b;

	अगर (argc < 3)
		वापस निकास_त्रुटि;

	a = argv[1];
	b = argv[2];

	अगर (!a || !b)
		वापस निकास_त्रुटि;

	म_लिखो("cmp    '%s' '%s' = %d\n", a, b, strcasecmp(a, b));
	म_लिखो("c_cmp  '%s' '%s' = %d\n", a, b, c_strcasecmp(a, b));
	म_लिखो("c_ncmp '%s' '%s' = %d\n", a, b, c_strncasecmp(a, b, म_माप(a)));

	वापस निकास_सफल;
}

पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[])
{
	अगर (argc == 3 && म_भेद(argv[1], "--size") == 0) {
		वापस test_strutils_sizes(argc - 1, argv + 1);

	} अन्यथा अगर (argc == 3 && म_भेद(argv[1], "--parse-switch") == 0) {
		म_लिखो("'%s'-->%d\n", argv[2], ul_parse_switch(argv[2],
						"on", "off",
						"enable", "disable",
						"yes", "no",
						"1", "0",
						शून्य));
		वापस निकास_सफल;
	} अन्यथा अगर (argc == 3 && म_भेद(argv[1], "--strtobool") == 0) {
		bool val;
		अगर (ul_strtobool(argv[2], &val))
			err(निकास_त्रुटि, "could not parse '%s'", argv[2]);
		म_लिखो("'%s'-->%s\n", argv[2], val ? "true" : "false");
		वापस निकास_सफल;

	} अन्यथा अगर (argc == 4 && म_भेद(argv[1], "--cmp-paths") == 0) {
		वापस test_strutils_cmp_paths(argc - 1, argv + 1);

	} अन्यथा अगर (argc == 4 && म_भेद(argv[1], "--path-is-within") == 0) {
		वापस test_strutils_path_is_within(argc - 1, argv + 1);

	} अन्यथा अगर (argc == 4 && म_भेद(argv[1], "--strdup-member") == 0) {
		वापस test_strdup_to_member(argc - 1, argv + 1);

	} अन्यथा अगर  (argc == 4 && म_भेद(argv[1], "--stralnumcmp") == 0) {
		म_लिखो("%s\n", ul_stralnumcmp(argv[2], argv[3]) == 0 ?
				"match" : "mismatch");
		वापस निकास_सफल;

	} अन्यथा अगर (argc == 4 && म_भेद(argv[1], "--cstrcasecmp") == 0) {
		वापस test_strutils_cstrcasecmp(argc - 1, argv + 1);

	} अन्यथा अगर (argc == 3 && म_भेद(argv[1], "--normalize") == 0) {
		वापस test_strutils_normalize(argc - 1, argv + 1);

	} अन्यथा अगर (argc == 3 && म_भेद(argv[1], "--strtos64") == 0) {
		म_लिखो("'%s'-->%"PRId64"\n", argv[2], strtos64_or_err(argv[2], "strtos64 failed"));
		वापस निकास_सफल;
	} अन्यथा अगर (argc == 3 && म_भेद(argv[1], "--strtou64") == 0) {
		म_लिखो("'%s'-->%"PRIu64"\n", argv[2], strtou64_or_err(argv[2], "strtou64 failed"));
		वापस निकास_सफल;
	} अन्यथा अगर (argc == 3 && म_भेद(argv[1], "--strtos32") == 0) {
		म_लिखो("'%s'-->%d\n", argv[2], strtos32_or_err(argv[2], "strtos32 failed"));
		वापस निकास_सफल;
	} अन्यथा अगर (argc == 3 && म_भेद(argv[1], "--strtou32") == 0) {
		म_लिखो("'%s'-->%u\n", argv[2], strtou32_or_err(argv[2], "strtou32 failed"));
		वापस निकास_सफल;
	} अन्यथा अगर (argc == 3 && म_भेद(argv[1], "--strtos16") == 0) {
		म_लिखो("'%s'-->%hd\n", argv[2], strtos16_or_err(argv[2], "strtos16 failed"));
		वापस निकास_सफल;
	} अन्यथा अगर (argc == 3 && म_भेद(argv[1], "--strtou16") == 0) {
		म_लिखो("'%s'-->%hu\n", argv[2], strtou16_or_err(argv[2], "strtou16 failed"));
		वापस निकास_सफल;

	} अन्यथा अगर (argc == 4 && म_भेद(argv[1], "--strchr-escaped") == 0) {
		म_लिखो("\"%s\" --> \"%s\"\n", argv[2], ul_strchr_escaped(argv[2], *argv[3]));
		वापस निकास_सफल;

	} अन्यथा अगर (argc == 2 && म_भेद(argv[1], "--next-string") == 0) {
		अक्षर *buf = "abc\0Y\0\0xyz\0X";
		अक्षर *end = buf + 12;
		अक्षर *p = buf;

		करो {
			म_लिखो("str: '%s'\n", p);
		} जबतक ((p = ul_अग्र_string(p, end)));

		वापस निकास_सफल;

	} अन्यथा अगर (argc == 3 && म_भेद(argv[1], "--optstr") == 0) {

		माप_प्रकार namesz, valsz;
		अक्षर *name = शून्य, *val = शून्य;
		अक्षर *p = argv[2];
		पूर्णांक rc;

		अगर (!ul_optstr_is_valid(p))
			errx(निकास_त्रुटि, _("unsupported option format: %s"), p);

		जबतक ((rc = ul_optstr_अग्र(&p, &name, &namesz, &val, &valsz)) == 0) {
			म_लिखो("'%.*s' : '%.*s'\n", (पूर्णांक) namesz, name,
						   (पूर्णांक) valsz, val);
		}
		अगर (rc == 1)
			वापस निकास_सफल;
	} अन्यथा अगर (argc == 3 && म_भेद(argv[1], "--unquote") == 0) {

		अक्षर *unquoted = strdup(argv[2]);

		अगर (!unquoted)
			err(निकास_त्रुटि, "strdup() failed");

		ul_unquote(unquoted);
		म_लिखो("%s-->%s\n", argv[2], unquoted);
		मुक्त(unquoted);
		वापस निकास_सफल;

	} अन्यथा {
		ख_लिखो(मानक_त्रुटि, "usage: %1$s --size <number>[suffix]\n"
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
		निकास(निकास_त्रुटि);
	}

	वापस निकास_त्रुटि;
}
#पूर्ण_अगर /* TEST_PROGRAM_STRUTILS */
