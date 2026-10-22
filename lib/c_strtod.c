/*
 * Locale-independent strtod().
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2021 Karel Zak <kzak@redhat.com>
 */
#شامل "c.h"

#شامل <locale.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "c_strtod.h"

#اگر_تعریف_شدہ __APPLE__
# include <xlocale.h>
#اگر_ختم

#اگر defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#اگر_ختم

#اگر defined(USE_CLOCALE)
ساکن متلون locale_t c_locale;

ساکن locale_t get_c_locale(خالی)
{
	اگر (!c_locale)
		c_locale = newlocale(LC_ALL_نقاب, "C", (locale_t) 0);
	واپس c_locale;
}
#اگر_ختم


دوہرا c_strtod(حرف ثابت *str, حرف **انتہا)
{
	دوہرا res;
	صحیح errsv;

#اگر defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#اگر defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	اگر (cl)
		واپس strtod_l(str, انتہا, cl);
#وگرنہ_اگر defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	اگر (cl) {
		locale_t org_cl = uselocale(cl);
		اگر (!org_cl)
			واپس 0;

		res = strtod(str, انتہا);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		واپس res;
	}
#اگر_ختم /* HAVE_USELOCALE */
#اگر_ختم /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	حرف *org_locale = setlocale(LC_NUMERIC, تہی);

	اگر (org_locale) {
		org_locale = strdup(org_locale);
		اگر (!org_locale)
			واپس 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, انتہا);
	errsv = errno;

	اگر (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		آزاد(org_locale);
	}
	errno = errsv;
	واپس res;
}

#اگر_تعریف_شدہ TEST_پروگرام
صحیح مین(صحیح argc, حرف *argv[])
{
	دوہرا res;
	حرف *انتہا;

	اگر (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				پروگرام_invocation_short_name);
		واپس EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &انتہا);
	لکھو("Result: %g, errno: %d, endptr: '%s'\n", res, errno, انتہا);

	واپس errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#اگر_ختم
