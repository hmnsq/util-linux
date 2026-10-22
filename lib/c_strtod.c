/*
 * Locale-independent strtod().
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2021 Karel Zak <kzak@redhat.com>
 */
#અન્તર્ભાવ "c.h"

#અન્તર્ભાવ <locale.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "c_strtod.h"

#ifdef __APPLE__
# include <xlocale.h>
#endif

#જો defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#જો defined(USE_CLOCALE)
સ્થિર અસ્થિર locale_t c_locale;

સ્થિર locale_t get_c_locale(શૂન્ય)
{
	જો (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	પાછા c_locale;
}
#endif


દ્વિગુણ c_strtod(અક્ષર અચલ *str, અક્ષર **end)
{
	દ્વિગુણ res;
	પૂર્ણાંક errsv;

#જો defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#જો defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	જો (cl)
		પાછા strtod_l(str, end, cl);
#અથવાજો defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	જો (cl) {
		locale_t org_cl = uselocale(cl);
		જો (!org_cl)
			પાછા 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		પાછા res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	અક્ષર *org_locale = setlocale(LC_NUMERIC, ખાલી);

	જો (org_locale) {
		org_locale = strdup(org_locale);
		જો (!org_locale)
			પાછા 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	જો (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		મુક્ત(org_locale);
	}
	errno = errsv;
	પાછા res;
}

#ifdef TEST_PROGRAM
પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[])
{
	દ્વિગુણ res;
	અક્ષર *end;

	જો (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		પાછા EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	પાછા errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
