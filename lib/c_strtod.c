/*
 * Locale-independent strtod().
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2021 Karel Zak <kzak@redhat.com>
 */
#അന്തര്ഭാവ "c.h"

#അന്തര്ഭാവ <locale.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "c_strtod.h"

#ifdef __APPLE__
# include <xlocale.h>
#endif

#എങ്കിൽ defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#എങ്കിൽ defined(USE_CLOCALE)
സ്ഥിതം അസ്ഥിര locale_t c_locale;

സ്ഥിതം locale_t get_c_locale(ശൂന്യം)
{
	എങ്കിൽ (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	തിരികെ c_locale;
}
#endif


ഇരട്ട c_strtod(അക്ഷരം സ്ഥിരം *str, അക്ഷരം **end)
{
	ഇരട്ട res;
	പൂർണ്ണസംഖ്യ errsv;

#എങ്കിൽ defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#എങ്കിൽ defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	എങ്കിൽ (cl)
		തിരികെ strtod_l(str, end, cl);
#അല്ലെങ്കിലെങ്കിൽ defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	എങ്കിൽ (cl) {
		locale_t org_cl = uselocale(cl);
		എങ്കിൽ (!org_cl)
			തിരികെ 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		തിരികെ res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	അക്ഷരം *org_locale = setlocale(LC_NUMERIC, ശൂന്യ);

	എങ്കിൽ (org_locale) {
		org_locale = strdup(org_locale);
		എങ്കിൽ (!org_locale)
			തിരികെ 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	എങ്കിൽ (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		സ്വതന്ത്രം(org_locale);
	}
	errno = errsv;
	തിരികെ res;
}

#ifdef TEST_PROGRAM
പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	ഇരട്ട res;
	അക്ഷരം *end;

	എങ്കിൽ (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		തിരികെ EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	തിരികെ errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
