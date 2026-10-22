/*
 * Locale-independent strtod().
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2021 Karel Zak <kzak@redhat.com>
 */
#অন্তর্ভাৱ "c.h"

#অন্তর্ভাৱ <locale.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>

#অন্তর্ভাৱ "c_strtod.h"

#ifdef __APPLE__
# include <xlocale.h>
#endif

#যদি defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#যদি defined(USE_CLOCALE)
স্থির চঞ্চল locale_t c_locale;

স্থির locale_t get_c_locale(শূন্য)
{
	যদি (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	ঘূৰাই c_locale;
}
#endif


দ্ৱিগুণ c_strtod(ৱর্ণ ধ্রুৱ *str, ৱর্ণ **end)
{
	দ্ৱিগুণ res;
	পূর্ণাঙ্ক errsv;

#যদি defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#যদি defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	যদি (cl)
		ঘূৰাই strtod_l(str, end, cl);
#নোচেদ্যদি defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	যদি (cl) {
		locale_t org_cl = uselocale(cl);
		যদি (!org_cl)
			ঘূৰাই 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		ঘূৰাই res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	ৱর্ণ *org_locale = setlocale(LC_NUMERIC, রিক্ত);

	যদি (org_locale) {
		org_locale = strdup(org_locale);
		যদি (!org_locale)
			ঘূৰাই 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	যদি (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		মুক্ত(org_locale);
	}
	errno = errsv;
	ঘূৰাই res;
}

#ifdef TEST_PROGRAM
পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ *argv[])
{
	দ্ৱিগুণ res;
	ৱর্ণ *end;

	যদি (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		ঘূৰাই EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	ঘূৰাই errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
