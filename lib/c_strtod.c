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

#यदि_परिभाषितम् __APPLE__
# include <xlocale.h>
#यदि_अन्तः

#यदि defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#यदि_अन्तः

#यदि defined(USE_CLOCALE)
स्थिर चञ्चल locale_t c_locale;

स्थिर locale_t get_c_locale(शून्य)
{
	यदि (!c_locale)
		c_locale = newlocale(LC_ALL_आवरणम्, "C", (locale_t) 0);
	प्रत्यागम c_locale;
}
#यदि_अन्तः


द्विगुण c_strtod(वर्ण ध्रुव *str, वर्ण **अन्तः)
{
	द्विगुण res;
	पूर्णाङ्क errsv;

#यदि defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#यदि defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	यदि (cl)
		प्रत्यागम strtod_l(str, अन्तः, cl);
#नोचेद्यदि defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	यदि (cl) {
		locale_t org_cl = uselocale(cl);
		यदि (!org_cl)
			प्रत्यागम 0;

		res = strtod(str, अन्तः);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		प्रत्यागम res;
	}
#यदि_अन्तः /* HAVE_USELOCALE */
#यदि_अन्तः /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	वर्ण *org_locale = setlocale(LC_NUMERIC, रिक्त);

	यदि (org_locale) {
		org_locale = strdup(org_locale);
		यदि (!org_locale)
			प्रत्यागम 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, अन्तः);
	errsv = errno;

	यदि (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		मुक्त(org_locale);
	}
	errno = errsv;
	प्रत्यागम res;
}

#यदि_परिभाषितम् TEST_क्रमादेशः
पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण *argv[])
{
	द्विगुण res;
	वर्ण *अन्तः;

	यदि (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				क्रमादेशः_invocation_short_name);
		प्रत्यागम EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &अन्तः);
	प्रारूप_लिख("Result: %g, errno: %d, endptr: '%s'\n", res, errno, अन्तः);

	प्रत्यागम errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#यदि_अन्तः
