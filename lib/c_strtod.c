/*
 * Locale-independent strtod().
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2021 Karel Zak <kzak@redhat.com>
 */
#समावेश "c.h"

#समावेश <locale.h>
#समावेश <stdlib.h>
#समावेश <string.h>

#समावेश "c_strtod.h"

#अगर_घोषित __APPLE__
# include <xlocale.h>
#पूर्ण_अगर

#अगर defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#पूर्ण_अगर

#अगर defined(USE_CLOCALE)
अटल अस्थिर locale_t c_locale;

अटल locale_t get_c_locale(व्योम)
{
	अगर (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	वापस c_locale;
}
#पूर्ण_अगर


द्विगुन c_strtod(अक्षर स्थिर *str, अक्षर **end)
{
	द्विगुन res;
	पूर्णांक errsv;

#अगर defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#अगर defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	अगर (cl)
		वापस strtod_l(str, end, cl);
#या_अगर defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	अगर (cl) {
		locale_t org_cl = uselocale(cl);
		अगर (!org_cl)
			वापस 0;

		res = म_से_भग्न(str, end);
		errsv = त्रुटि_सं;

		uselocale(org_cl);
		त्रुटि_सं = errsv;
		वापस res;
	}
#पूर्ण_अगर /* HAVE_USELOCALE */
#पूर्ण_अगर /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	अक्षर *org_locale = रखो_क्षेत्र(LC_NUMERIC, शून्य);

	अगर (org_locale) {
		org_locale = strdup(org_locale);
		अगर (!org_locale)
			वापस 0;

		रखो_क्षेत्र(LC_NUMERIC, "C");
	}
	res = म_से_भग्न(str, end);
	errsv = त्रुटि_सं;

	अगर (org_locale) {
		रखो_क्षेत्र(LC_NUMERIC, org_locale);
		मुक्त(org_locale);
	}
	त्रुटि_सं = errsv;
	वापस res;
}

#अगर_घोषित TEST_PROGRAM
पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[])
{
	द्विगुन res;
	अक्षर *end;

	अगर (argc < 2) {
		ख_लिखो(मानक_त्रुटि, "usage: %s decimal.number\n",
				program_invocation_short_name);
		वापस निकास_त्रुटि;
	}

	res = c_strtod(argv[1], &end);
	म_लिखो("Result: %g, errno: %d, endptr: '%s'\n", res, त्रुटि_सं, end);

	वापस त्रुटि_सं ? निकास_त्रुटि : निकास_सफल;
}
#पूर्ण_अगर
