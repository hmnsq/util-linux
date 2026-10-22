/*
 * Locale-independent strtod().
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2021 Karel Zak <kzak@redhat.com>
 */
#ضمن "c.h"

#ضمن <locale.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن "c_strtod.h"

#ifdef __APPLE__
# include <xlocale.h>
#endif

#إذا defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#إذا defined(USE_CLOCALE)
ساكن متطاير locale_t c_locale;

ساكن locale_t get_c_locale(فراغ)
{
	إذا (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	إرجاع c_locale;
}
#endif


مزدوج c_strtod(حرف ثابت *str, حرف **end)
{
	مزدوج res;
	صحيح errsv;

#إذا defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#إذا defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	إذا (cl)
		إرجاع strtod_l(str, end, cl);
#وإلاإذا defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	إذا (cl) {
		locale_t org_cl = uselocale(cl);
		إذا (!org_cl)
			إرجاع 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		إرجاع res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	حرف *org_locale = setlocale(LC_NUMERIC, عدم);

	إذا (org_locale) {
		org_locale = strdup(org_locale);
		إذا (!org_locale)
			إرجاع 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	إذا (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		حرر(org_locale);
	}
	errno = errsv;
	إرجاع res;
}

#ifdef TEST_PROGRAM
صحيح الرئيسية(صحيح argc, حرف *argv[])
{
	مزدوج res;
	حرف *end;

	إذا (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		إرجاع EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	إرجاع errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
