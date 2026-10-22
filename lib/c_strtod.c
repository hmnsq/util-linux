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

#ܐܢ defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#ܐܢ defined(USE_CLOCALE)
ساكن متطاير locale_t c_locale;

ساكن locale_t get_c_locale(فراغ)
{
	ܐܢ (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	ܦܢܐ c_locale;
}
#endif


مزدوج c_strtod(حرف ثابت *str, حرف **end)
{
	مزدوج res;
	صحيح errsv;

#ܐܢ defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#ܐܢ defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	ܐܢ (cl)
		ܦܢܐ strtod_l(str, end, cl);
#وإلاإذا defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	ܐܢ (cl) {
		locale_t org_cl = uselocale(cl);
		ܐܢ (!org_cl)
			ܦܢܐ 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		ܦܢܐ res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	حرف *org_locale = setlocale(LC_NUMERIC, عدم);

	ܐܢ (org_locale) {
		org_locale = strdup(org_locale);
		ܐܢ (!org_locale)
			ܦܢܐ 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	ܐܢ (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		حرر(org_locale);
	}
	errno = errsv;
	ܦܢܐ res;
}

#ifdef TEST_PROGRAM
صحيح main(صحيح argc, حرف *argv[])
{
	مزدوج res;
	حرف *end;

	ܐܢ (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		ܦܢܐ EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	ܦܢܐ errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
