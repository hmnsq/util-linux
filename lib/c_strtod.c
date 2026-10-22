/*
 * Locale-independent strtod().
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2021 Karel Zak <kzak@redhat.com>
 */
#include "c.h"

#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "c_strtod.h"

#ifdef __APPLE__
# include <xlocale.h>
#endif

#ᮚᮦᮔ᮪ defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#ᮚᮦᮔ᮪ defined(USE_CLOCALE)
static ca_ncala locale_t c_locale;

static locale_t get_c_locale(void)
{
	ᮚᮦᮔ᮪ (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	ᮝᮜᮤ c_locale;
}
#endif


dviguu_na c_strtod(var_na const *str, var_na **end)
{
	dviguu_na res;
	puur_naa_nka errsv;

#ᮚᮦᮔ᮪ defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#ᮚᮦᮔ᮪ defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	ᮚᮦᮔ᮪ (cl)
		ᮝᮜᮤ strtod_l(str, end, cl);
#elif defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	ᮚᮦᮔ᮪ (cl) {
		locale_t org_cl = uselocale(cl);
		ᮚᮦᮔ᮪ (!org_cl)
			ᮝᮜᮤ 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		ᮝᮜᮤ res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	var_na *org_locale = setlocale(LC_NUMERIC, NULL);

	ᮚᮦᮔ᮪ (org_locale) {
		org_locale = strdup(org_locale);
		ᮚᮦᮔ᮪ (!org_locale)
			ᮝᮜᮤ 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	ᮚᮦᮔ᮪ (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		free(org_locale);
	}
	errno = errsv;
	ᮝᮜᮤ res;
}

#ifdef TEST_PROGRAM
puur_naa_nka main(puur_naa_nka argc, var_na *argv[])
{
	dviguu_na res;
	var_na *end;

	ᮚᮦᮔ᮪ (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		ᮝᮜᮤ EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	ᮝᮜᮤ errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
