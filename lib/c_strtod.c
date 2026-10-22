/*
 * Locale-independent strtod().
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2021 Karel Zak <kzak@redhat.com>
 */
#ಅನ್ತರ್ಭಾವ "c.h"

#ಅನ್ತರ್ಭಾವ <locale.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "c_strtod.h"

#ifdef __APPLE__
# include <xlocale.h>
#endif

#ಯದಿ defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#ಯದಿ defined(USE_CLOCALE)
ಸ್ಥಿರ ಚಞ್ಚಲ locale_t c_locale;

ಸ್ಥಿರ locale_t get_c_locale(ಶೂನ್ಯ)
{
	ಯದಿ (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	ಪ್ರತ್ಯಾಗಮ c_locale;
}
#endif


ದ್ವಿಗುಣ c_strtod(ವರ್ಣ ಧ್ರುವ *str, ವರ್ಣ **end)
{
	ದ್ವಿಗುಣ res;
	ಪೂರ್ಣಾಙ್ಕ errsv;

#ಯದಿ defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#ಯದಿ defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	ಯದಿ (cl)
		ಪ್ರತ್ಯಾಗಮ strtod_l(str, end, cl);
#ನೋಚೇದ್ಯದಿ defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	ಯದಿ (cl) {
		locale_t org_cl = uselocale(cl);
		ಯದಿ (!org_cl)
			ಪ್ರತ್ಯಾಗಮ 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		ಪ್ರತ್ಯಾಗಮ res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	ವರ್ಣ *org_locale = setlocale(LC_NUMERIC, ರಿಕ್ತ);

	ಯದಿ (org_locale) {
		org_locale = strdup(org_locale);
		ಯದಿ (!org_locale)
			ಪ್ರತ್ಯಾಗಮ 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	ಯದಿ (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		ಮುಕ್ತ(org_locale);
	}
	errno = errsv;
	ಪ್ರತ್ಯಾಗಮ res;
}

#ifdef TEST_PROGRAM
ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ದ್ವಿಗುಣ res;
	ವರ್ಣ *end;

	ಯದಿ (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	ಪ್ರತ್ಯಾಗಮ errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
