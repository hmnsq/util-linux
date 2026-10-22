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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __APPLE__
# include <xlocale.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(USE_CLOCALE)
ಸ್ಥಿರ ಅಸ್ಥಿರ locale_t c_locale;

ಸ್ಥಿರ locale_t get_c_locale(ಶೂನ್ಯ)
{
	ಆದರೆ (!c_locale)
		c_locale = newlocale(LC_ALL_ಮುಖವಾಡ, "C", (locale_t) 0);
	ಹಿಂತಿರುಗಿ c_locale;
}
#ಆದರೆ_ಅಂತ್ಯ


ದ್ವಿಗುಣ c_strtod(ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *str, ಅಕ್ಷರ **ಕೊನೆ)
{
	ದ್ವಿಗುಣ res;
	ಪೂರ್ಣಾಂಕ errsv;

#ಆದರೆ defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#ಆದರೆ defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	ಆದರೆ (cl)
		ಹಿಂತಿರುಗಿ strtod_l(str, ಕೊನೆ, cl);
#ಅಥವಾಆದರೆ defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	ಆದರೆ (cl) {
		locale_t org_cl = uselocale(cl);
		ಆದರೆ (!org_cl)
			ಹಿಂತಿರುಗಿ 0;

		res = strtod(str, ಕೊನೆ);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		ಹಿಂತಿರುಗಿ res;
	}
#ಆದರೆ_ಅಂತ್ಯ /* HAVE_USELOCALE */
#ಆದರೆ_ಅಂತ್ಯ /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	ಅಕ್ಷರ *org_locale = setlocale(LC_NUMERIC, ಖಾಲಿ);

	ಆದರೆ (org_locale) {
		org_locale = strdup(org_locale);
		ಆದರೆ (!org_locale)
			ಹಿಂತಿರುಗಿ 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, ಕೊನೆ);
	errsv = errno;

	ಆದರೆ (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		ಬಿಡುಗಡೆ(org_locale);
	}
	errno = errsv;
	ಹಿಂತಿರುಗಿ res;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ದ್ವಿಗುಣ res;
	ಅಕ್ಷರ *ಕೊನೆ;

	ಆದರೆ (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				ಕಾರ್ಯಕ್ರಮ_invocation_short_name);
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &ಕೊನೆ);
	ಬರೆ("Result: %g, errno: %d, endptr: '%s'\n", res, errno, ಕೊನೆ);

	ಹಿಂತಿರುಗಿ errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#ಆದರೆ_ಅಂತ್ಯ
