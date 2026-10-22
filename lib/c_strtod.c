/*
 * Locale-independent strtod().
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2021 Karel Zak <kzak@redhat.com>
 */
#অন্তর্ভাব "c.h"

#অন্তর্ভাব <locale.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "c_strtod.h"

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
	প্রত্যাগম c_locale;
}
#endif


দ্বিগুণ c_strtod(বর্ণ ধ্রুব *str, বর্ণ **end)
{
	দ্বিগুণ res;
	পূর্ণাঙ্ক errsv;

#যদি defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#যদি defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	যদি (cl)
		প্রত্যাগম strtod_l(str, end, cl);
#নোচেদ্যদি defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	যদি (cl) {
		locale_t org_cl = uselocale(cl);
		যদি (!org_cl)
			প্রত্যাগম 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		প্রত্যাগম res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	বর্ণ *org_locale = setlocale(LC_NUMERIC, রিক্ত);

	যদি (org_locale) {
		org_locale = strdup(org_locale);
		যদি (!org_locale)
			প্রত্যাগম 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	যদি (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		মুক্ত(org_locale);
	}
	errno = errsv;
	প্রত্যাগম res;
}

#ifdef TEST_PROGRAM
পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ *argv[])
{
	দ্বিগুণ res;
	বর্ণ *end;

	যদি (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		প্রত্যাগম EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	প্রত্যাগম errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
