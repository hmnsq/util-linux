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

#𐡄𐡍 defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#𐡄𐡍 defined(USE_CLOCALE)
ساكن متطاير locale_t c_locale;

ساكن locale_t get_c_locale(فراغ)
{
	𐡄𐡍 (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	أرجع c_locale;
}
#endif


مزدوج c_strtod(حرف ثابت *str, حرف **end)
{
	مزدوج res;
	صحيح errsv;

#𐡄𐡍 defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#𐡄𐡍 defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	𐡄𐡍 (cl)
		أرجع strtod_l(str, end, cl);
#وإلاإذا defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	𐡄𐡍 (cl) {
		locale_t org_cl = uselocale(cl);
		𐡄𐡍 (!org_cl)
			أرجع 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		أرجع res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	حرف *org_locale = setlocale(LC_NUMERIC, عدم);

	𐡄𐡍 (org_locale) {
		org_locale = strdup(org_locale);
		𐡄𐡍 (!org_locale)
			أرجع 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	𐡄𐡍 (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		حرر(org_locale);
	}
	errno = errsv;
	أرجع res;
}

#ifdef TEST_PROGRAM
صحيح main(صحيح argc, حرف *argv[])
{
	مزدوج res;
	حرف *end;

	𐡄𐡍 (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		أرجع EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	أرجع errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
