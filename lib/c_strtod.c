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

#جيڪڏهن_وصف_ٿيل __APPLE__
# include <xlocale.h>
#جيڪڏهن_پورو

#إذا defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#جيڪڏهن_پورو

#إذا defined(USE_CLOCALE)
ساكن متطاير locale_t c_locale;

ساكن locale_t get_c_locale(فراغ)
{
	إذا (!c_locale)
		c_locale = newlocale(LC_ALL_نقاب, "C", (locale_t) 0);
	أرجع c_locale;
}
#جيڪڏهن_پورو


مزدوج c_strtod(حرف ثابت *str, حرف **پڇاڙي)
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
		أرجع strtod_l(str, پڇاڙي, cl);
#وإلاإذا defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	إذا (cl) {
		locale_t org_cl = uselocale(cl);
		إذا (!org_cl)
			أرجع 0;

		res = strtod(str, پڇاڙي);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		أرجع res;
	}
#جيڪڏهن_پورو /* HAVE_USELOCALE */
#جيڪڏهن_پورو /* USE_CLOCALE */
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
	res = strtod(str, پڇاڙي);
	errsv = errno;

	إذا (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		حرر(org_locale);
	}
	errno = errsv;
	أرجع res;
}

#جيڪڏهن_وصف_ٿيل TEST_پروگرام
صحيح مکيه(صحيح argc, حرف *argv[])
{
	مزدوج res;
	حرف *پڇاڙي;

	إذا (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				پروگرام_invocation_short_name);
		أرجع EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &پڇاڙي);
	لکو("Result: %g, errno: %d, endptr: '%s'\n", res, errno, پڇاڙي);

	أرجع errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#جيڪڏهن_پورو
