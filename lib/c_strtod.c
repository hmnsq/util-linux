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

#אם defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#אם defined(USE_CLOCALE)
ساكن متطاير locale_t c_locale;

ساكن locale_t get_c_locale(فراغ)
{
	אם (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	החזר c_locale;
}
#endif


مزدوج c_strtod(حرف ثابت *str, حرف **end)
{
	مزدوج res;
	صحيح errsv;

#אם defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#אם defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	אם (cl)
		החזר strtod_l(str, end, cl);
#وإلاإذا defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	אם (cl) {
		locale_t org_cl = uselocale(cl);
		אם (!org_cl)
			החזר 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		החזר res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	حرف *org_locale = setlocale(LC_NUMERIC, عدم);

	אם (org_locale) {
		org_locale = strdup(org_locale);
		אם (!org_locale)
			החזר 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	אם (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		حرر(org_locale);
	}
	errno = errsv;
	החזר res;
}

#ifdef TEST_PROGRAM
صحيح main(صحيح argc, حرف *argv[])
{
	مزدوج res;
	حرف *end;

	אם (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		החזר EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	החזר errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
