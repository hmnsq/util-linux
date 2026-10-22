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
স্থির অস্থির locale_t c_locale;

স্থির locale_t get_c_locale(শূন্য)
{
	যদি (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	ফেরত c_locale;
}
#endif


দ্বিগুণ c_strtod(অক্ষর ধ্রুবক *str, অক্ষর **end)
{
	দ্বিগুণ res;
	পূর্ণসংখ্যা errsv;

#যদি defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#যদি defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	যদি (cl)
		ফেরত strtod_l(str, end, cl);
#নতুবা_যদি defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	যদি (cl) {
		locale_t org_cl = uselocale(cl);
		যদি (!org_cl)
			ফেরত 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		ফেরত res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	অক্ষর *org_locale = setlocale(LC_NUMERIC, ফাঁকা);

	যদি (org_locale) {
		org_locale = strdup(org_locale);
		যদি (!org_locale)
			ফেরত 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	যদি (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		মুক্ত(org_locale);
	}
	errno = errsv;
	ফেরত res;
}

#ifdef TEST_PROGRAM
পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	দ্বিগুণ res;
	অক্ষর *end;

	যদি (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		ফেরত EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	ফেরত errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
