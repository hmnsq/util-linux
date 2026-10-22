/*
 * Locale-independent strtod().
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2021 Karel Zak <kzak@redhat.com>
 */
#ଅନ୍ତର୍ଭାଵ "c.h"

#ଅନ୍ତର୍ଭାଵ <locale.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "c_strtod.h"

#ifdef __APPLE__
# include <xlocale.h>
#endif

#ଯଦି defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#ଯଦି defined(USE_CLOCALE)
ସ୍ଥିର ଅସ୍ଥିର locale_t c_locale;

ସ୍ଥିର locale_t get_c_locale(ଶୂନ୍ୟ)
{
	ଯଦି (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	ଫେରନ୍ତୁ c_locale;
}
#endif


ଦ୍ୱିଗୁଣ c_strtod(ଅକ୍ଷର ଅଚଳ *str, ଅକ୍ଷର **end)
{
	ଦ୍ୱିଗୁଣ res;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା errsv;

#ଯଦି defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#ଯଦି defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	ଯଦି (cl)
		ଫେରନ୍ତୁ strtod_l(str, end, cl);
#ନଚେତଯଦି defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	ଯଦି (cl) {
		locale_t org_cl = uselocale(cl);
		ଯଦି (!org_cl)
			ଫେରନ୍ତୁ 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		ଫେରନ୍ତୁ res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	ଅକ୍ଷର *org_locale = setlocale(LC_NUMERIC, ଖାଲି);

	ଯଦି (org_locale) {
		org_locale = strdup(org_locale);
		ଯଦି (!org_locale)
			ଫେରନ୍ତୁ 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	ଯଦି (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		ମୁକ୍ତ(org_locale);
	}
	errno = errsv;
	ଫେରନ୍ତୁ res;
}

#ifdef TEST_PROGRAM
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ଦ୍ୱିଗୁଣ res;
	ଅକ୍ଷର *end;

	ଯଦି (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		ଫେରନ୍ତୁ EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	ଫେରନ୍ତୁ errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
