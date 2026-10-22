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

#जर defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#जर defined(USE_CLOCALE)
स्थिर अस्थिर locale_t c_locale;

स्थिर locale_t get_c_locale(शून्य)
{
	जर (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	परत c_locale;
}
#endif


द्विगुण c_strtod(अक्षर स्थिरांक *str, अक्षर **end)
{
	द्विगुण res;
	पूर्णांक errsv;

#जर defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#जर defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	जर (cl)
		परत strtod_l(str, end, cl);
#किंवाजर defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	जर (cl) {
		locale_t org_cl = uselocale(cl);
		जर (!org_cl)
			परत 0;

		res = म_से_भग्न(str, end);
		errsv = त्रुटि_सं;

		uselocale(org_cl);
		त्रुटि_सं = errsv;
		परत res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	अक्षर *org_locale = रखो_क्षेत्र(LC_NUMERIC, रिक्त);

	जर (org_locale) {
		org_locale = strdup(org_locale);
		जर (!org_locale)
			परत 0;

		रखो_क्षेत्र(LC_NUMERIC, "C");
	}
	res = म_से_भग्न(str, end);
	errsv = त्रुटि_सं;

	जर (org_locale) {
		रखो_क्षेत्र(LC_NUMERIC, org_locale);
		मुक्त(org_locale);
	}
	त्रुटि_सं = errsv;
	परत res;
}

#ifdef TEST_PROGRAM
पूर्णांक main(पूर्णांक argc, अक्षर *argv[])
{
	द्विगुण res;
	अक्षर *end;

	जर (argc < 2) {
		ख_लिखो(मानक_त्रुटि, "usage: %s decimal.number\n",
				program_invocation_short_name);
		परत निकास_त्रुटि;
	}

	res = c_strtod(argv[1], &end);
	म_लिखो("Result: %g, errno: %d, endptr: '%s'\n", res, त्रुटि_सं, end);

	परत त्रुटि_सं ? निकास_त्रुटि : निकास_सफल;
}
#endif
