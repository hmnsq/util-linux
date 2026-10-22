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

#اگر_تعریف_شده __APPLE__
# include <xlocale.h>
#پایان_اگر

#إذا defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#پایان_اگر

#إذا defined(USE_CLOCALE)
ساكن متطاير locale_t c_locale;

ساكن locale_t get_c_locale(فراغ)
{
	إذا (!c_locale)
		c_locale = newlocale(LC_ALL_ماسک, "C", (locale_t) 0);
	أرجع c_locale;
}
#پایان_اگر


مزدوج c_strtod(حرف ثابت *str, حرف **انتها)
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
		أرجع strtod_l(str, انتها, cl);
#وإلاإذا defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	إذا (cl) {
		locale_t org_cl = uselocale(cl);
		إذا (!org_cl)
			أرجع 0;

		res = strtod(str, انتها);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		أرجع res;
	}
#پایان_اگر /* HAVE_USELOCALE */
#پایان_اگر /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	حرف *org_locale = setlocale(LC_NUMERIC, عدم);

	إذا (org_locale) {
		org_locale = strdup(org_locale);
		إذا (!org_locale)
			أرجع 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, انتها);
	errsv = errno;

	إذا (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		حرر(org_locale);
	}
	errno = errsv;
	أرجع res;
}

#اگر_تعریف_شده TEST_برنامه
صحيح اصلی(صحيح argc, حرف *argv[])
{
	مزدوج res;
	حرف *انتها;

	إذا (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				برنامه_invocation_short_name);
		أرجع EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &انتها);
	چاپ_کن("Result: %g, errno: %d, endptr: '%s'\n", res, errno, انتها);

	أرجع errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#پایان_اگر
