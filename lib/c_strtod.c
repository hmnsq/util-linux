/*
 * Locale-independent strtod().
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2021 Karel Zak <kzak@redhat.com>
 */
#अन्तर्भाव "c.h"

#अन्तर्भाव <locale.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "c_strtod.h"

#ifdef __APPLE__
# include <xlocale.h>
#endif

#सचे defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#सचे defined(USE_CLOCALE)
स्थिर चञ्चल locale_t c_locale;

स्थिर locale_t get_c_locale(शून्य)
{
	सचे (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	पटिनिवत्त c_locale;
}
#endif


द्विगुण c_strtod(वर्ण ध्रुव *str, वर्ण **end)
{
	द्विगुण res;
	पूर्णाङ्क errsv;

#सचे defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#सचे defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	सचे (cl)
		पटिनिवत्त strtod_l(str, end, cl);
#नोचेद्यदि defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	सचे (cl) {
		locale_t org_cl = uselocale(cl);
		सचे (!org_cl)
			पटिनिवत्त 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		पटिनिवत्त res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	वर्ण *org_locale = setlocale(LC_NUMERIC, रिक्त);

	सचे (org_locale) {
		org_locale = strdup(org_locale);
		सचे (!org_locale)
			पटिनिवत्त 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	सचे (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		मुक्त(org_locale);
	}
	errno = errsv;
	पटिनिवत्त res;
}

#ifdef TEST_PROGRAM
पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	द्विगुण res;
	वर्ण *end;

	सचे (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		पटिनिवत्त EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	पटिनिवत्त errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
