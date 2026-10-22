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

#もし defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#もし defined(USE_CLOCALE)
static volatile locale_t c_locale;

static locale_t get_c_locale(void)
{
	もし (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	戻る c_locale;
}
#endif


double c_strtod(char const *str, char **end)
{
	double res;
	整数 errsv;

#もし defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#もし defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	もし (cl)
		戻る strtod_l(str, end, cl);
#elif defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	もし (cl) {
		locale_t org_cl = uselocale(cl);
		もし (!org_cl)
			戻る 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		戻る res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	char *org_locale = setlocale(LC_NUMERIC, NULL);

	もし (org_locale) {
		org_locale = strdup(org_locale);
		もし (!org_locale)
			戻る 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	もし (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		free(org_locale);
	}
	errno = errsv;
	戻る res;
}

#ifdef TEST_PROGRAM
整数 メインページ(整数 argc, char *argv[])
{
	double res;
	char *end;

	もし (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		戻る EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	戻る errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
