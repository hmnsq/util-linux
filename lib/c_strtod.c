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

#جے_تعریف_شدہ __APPLE__
# include <xlocale.h>
#جے_انت

#إذا defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#جے_انت

#إذا defined(USE_CLOCALE)
ساكن متطاير locale_t c_locale;

ساكن locale_t get_c_locale(فراغ)
{
	إذا (!c_locale)
		c_locale = newlocale(LC_ALL_نقاب, "C", (locale_t) 0);
	أرجع c_locale;
}
#جے_انت


مزدوج c_strtod(حرف ثابت *str, حرف **سرا)
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
		أرجع strtod_l(str, سرا, cl);
#وإلاإذا defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	إذا (cl) {
		locale_t org_cl = uselocale(cl);
		إذا (!org_cl)
			أرجع 0;

		res = strtod(str, سرا);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		أرجع res;
	}
#جے_انت /* HAVE_USELOCALE */
#جے_انت /* USE_CLOCALE */
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
	res = strtod(str, سرا);
	errsv = errno;

	إذا (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		حرر(org_locale);
	}
	errno = errsv;
	أرجع res;
}

#جے_تعریف_شدہ TEST_پروگرام
صحيح مکھ(صحيح argc, حرف *argv[])
{
	مزدوج res;
	حرف *سرا;

	إذا (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				پروگرام_invocation_short_name);
		أرجع EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &سرا);
	لکھو("Result: %g, errno: %d, endptr: '%s'\n", res, errno, سرا);

	أرجع errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#جے_انت
