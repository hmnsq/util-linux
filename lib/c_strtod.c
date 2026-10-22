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

#если defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#если defined(USE_CLOCALE)
static volatile locale_t c_locale;

static locale_t get_c_locale(void)
{
	если (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	вернуть c_locale;
}
#endif


double c_strtod(char const *str, char **end)
{
	double res;
	целое errsv;

#если defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#если defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	если (cl)
		вернуть strtod_l(str, end, cl);
#elif defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	если (cl) {
		locale_t org_cl = uselocale(cl);
		если (!org_cl)
			вернуть 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		вернуть res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	char *org_locale = setlocale(LC_NUMERIC, NULL);

	если (org_locale) {
		org_locale = strdup(org_locale);
		если (!org_locale)
			вернуть 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	если (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		free(org_locale);
	}
	errno = errsv;
	вернуть res;
}

#ifdef TEST_PROGRAM
целое основной(целое argc, char *argv[])
{
	double res;
	char *end;

	если (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		вернуть EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	вернуть errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
