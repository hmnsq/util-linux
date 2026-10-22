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

#якщо defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#якщо defined(USE_CLOCALE)
static volatile locale_t c_locale;

static locale_t get_c_locale(void)
{
	якщо (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	повернути c_locale;
}
#endif


double c_strtod(char const *str, char **end)
{
	double res;
	ціле errsv;

#якщо defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#якщо defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	якщо (cl)
		повернути strtod_l(str, end, cl);
#elif defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	якщо (cl) {
		locale_t org_cl = uselocale(cl);
		якщо (!org_cl)
			повернути 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		повернути res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	char *org_locale = setlocale(LC_NUMERIC, NULL);

	якщо (org_locale) {
		org_locale = strdup(org_locale);
		якщо (!org_locale)
			повернути 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	якщо (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		free(org_locale);
	}
	errno = errsv;
	повернути res;
}

#ifdef TEST_PROGRAM
ціле main(ціле argc, char *argv[])
{
	double res;
	char *end;

	якщо (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		повернути EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	повернути errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
