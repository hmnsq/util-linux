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

#만약 defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#만약 defined(USE_CLOCALE)
static volatile locale_t c_locale;

static locale_t get_c_locale(void)
{
	만약 (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	반환 c_locale;
}
#endif


double c_strtod(char const *str, char **end)
{
	double res;
	정수 errsv;

#만약 defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#만약 defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	만약 (cl)
		반환 strtod_l(str, end, cl);
#elif defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	만약 (cl) {
		locale_t org_cl = uselocale(cl);
		만약 (!org_cl)
			반환 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		반환 res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	char *org_locale = setlocale(LC_NUMERIC, NULL);

	만약 (org_locale) {
		org_locale = strdup(org_locale);
		만약 (!org_locale)
			반환 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	만약 (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		free(org_locale);
	}
	errno = errsv;
	반환 res;
}

#ifdef TEST_PROGRAM
정수 main(정수 argc, char *argv[])
{
	double res;
	char *end;

	만약 (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		반환 EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	반환 errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
