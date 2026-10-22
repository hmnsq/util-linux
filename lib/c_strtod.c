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

#如果 defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#如果 defined(USE_CLOCALE)
static volatile locale_t c_locale;

static locale_t get_c_locale(void)
{
	如果 (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	返回 c_locale;
}
#endif


double c_strtod(char const *str, char **end)
{
	double res;
	整数 errsv;

#如果 defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#如果 defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	如果 (cl)
		返回 strtod_l(str, end, cl);
#elif defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	如果 (cl) {
		locale_t org_cl = uselocale(cl);
		如果 (!org_cl)
			返回 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		返回 res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	char *org_locale = setlocale(LC_NUMERIC, NULL);

	如果 (org_locale) {
		org_locale = strdup(org_locale);
		如果 (!org_locale)
			返回 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	如果 (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		free(org_locale);
	}
	errno = errsv;
	返回 res;
}

#ifdef TEST_PROGRAM
整数 主要(整数 argc, char *argv[])
{
	double res;
	char *end;

	如果 (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		返回 EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	返回 errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
