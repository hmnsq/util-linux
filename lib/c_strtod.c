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

#nếu defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#nếu defined(USE_CLOCALE)
static volatile locale_t c_locale;

static locale_t get_c_locale(void)
{
	nếu (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	trả_về c_locale;
}
#endif


double c_strtod(char const *str, char **end)
{
	double res;
	số_nguyên errsv;

#nếu defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#nếu defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	nếu (cl)
		trả_về strtod_l(str, end, cl);
#elif defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	nếu (cl) {
		locale_t org_cl = uselocale(cl);
		nếu (!org_cl)
			trả_về 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		trả_về res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	char *org_locale = setlocale(LC_NUMERIC, NULL);

	nếu (org_locale) {
		org_locale = strdup(org_locale);
		nếu (!org_locale)
			trả_về 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	nếu (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		free(org_locale);
	}
	errno = errsv;
	trả_về res;
}

#ifdef TEST_PROGRAM
số_nguyên main(số_nguyên argc, char *argv[])
{
	double res;
	char *end;

	nếu (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		trả_về EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	trả_về errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
