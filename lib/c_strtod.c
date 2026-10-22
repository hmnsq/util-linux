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

#यदि defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#यदि defined(USE_CLOCALE)
स्थिर चञ्चल locale_t c_locale;

स्थिर locale_t get_c_locale(शून्य)
{
	यदि (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	फर्काऊ c_locale;
}
#endif


द्विगुण c_strtod(वर्ण अचर *str, वर्ण **end)
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
		फर्काऊ strtod_l(str, end, cl);
#अथवायदि defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	यदि (cl) {
		locale_t org_cl = uselocale(cl);
		यदि (!org_cl)
			फर्काऊ 0;

		res = म_से_भग्न(str, end);
		errsv = त्रुटि_सं;

		uselocale(org_cl);
		त्रुटि_सं = errsv;
		फर्काऊ res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	वर्ण *org_locale = रखो_क्षेत्र(LC_NUMERIC, रिक्त);

	यदि (org_locale) {
		org_locale = strdup(org_locale);
		यदि (!org_locale)
			फर्काऊ 0;

		रखो_क्षेत्र(LC_NUMERIC, "C");
	}
	res = म_से_भग्न(str, end);
	errsv = त्रुटि_सं;

	यदि (org_locale) {
		रखो_क्षेत्र(LC_NUMERIC, org_locale);
		मुक्त(org_locale);
	}
	त्रुटि_सं = errsv;
	फर्काऊ res;
}

#ifdef TEST_PROGRAM
पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	द्विगुण res;
	वर्ण *end;

	यदि (argc < 2) {
		ख_लिखो(मानक_त्रुटि, "usage: %s decimal.number\n",
				program_invocation_short_name);
		फर्काऊ निकास_त्रुटि;
	}

	res = c_strtod(argv[1], &end);
	म_लिखो("Result: %g, errno: %d, endptr: '%s'\n", res, त्रुटि_सं, end);

	फर्काऊ त्रुटि_सं ? निकास_त्रुटि : निकास_सफल;
}
#endif
