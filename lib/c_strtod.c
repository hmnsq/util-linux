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

#ᱡᱩᱫᱤ defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#ᱡᱩᱫᱤ defined(USE_CLOCALE)
static ca_ncala locale_t c_locale;

static locale_t get_c_locale(void)
{
	ᱡᱩᱫᱤ (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	ᱨᱩᱣᱟᱹᱲ c_locale;
}
#endif


dviguu_na c_strtod(var_na const *str, var_na **end)
{
	dviguu_na res;
	puur_naa_nka errsv;

#ᱡᱩᱫᱤ defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#ᱡᱩᱫᱤ defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	ᱡᱩᱫᱤ (cl)
		ᱨᱩᱣᱟᱹᱲ strtod_l(str, end, cl);
#elif defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	ᱡᱩᱫᱤ (cl) {
		locale_t org_cl = uselocale(cl);
		ᱡᱩᱫᱤ (!org_cl)
			ᱨᱩᱣᱟᱹᱲ 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		ᱨᱩᱣᱟᱹᱲ res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	var_na *org_locale = setlocale(LC_NUMERIC, NULL);

	ᱡᱩᱫᱤ (org_locale) {
		org_locale = strdup(org_locale);
		ᱡᱩᱫᱤ (!org_locale)
			ᱨᱩᱣᱟᱹᱲ 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	ᱡᱩᱫᱤ (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		free(org_locale);
	}
	errno = errsv;
	ᱨᱩᱣᱟᱹᱲ res;
}

#ifdef TEST_PROGRAM
puur_naa_nka main(puur_naa_nka argc, var_na *argv[])
{
	dviguu_na res;
	var_na *end;

	ᱡᱩᱫᱤ (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		ᱨᱩᱣᱟᱹᱲ EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	ᱨᱩᱣᱟᱹᱲ errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
