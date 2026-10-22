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

#නම් defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#නම් defined(USE_CLOCALE)
static ca_ncala locale_t c_locale;

static locale_t get_c_locale(void)
{
	නම් (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	ආපසු c_locale;
}
#endif


dviguu_na c_strtod(var_na const *str, var_na **end)
{
	dviguu_na res;
	පූර්ණ_සංඛ්‍යා errsv;

#නම් defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#නම් defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	නම් (cl)
		ආපසු strtod_l(str, end, cl);
#elif defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	නම් (cl) {
		locale_t org_cl = uselocale(cl);
		නම් (!org_cl)
			ආපසු 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		ආපසු res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	var_na *org_locale = setlocale(LC_NUMERIC, NULL);

	නම් (org_locale) {
		org_locale = strdup(org_locale);
		නම් (!org_locale)
			ආපසු 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	නම් (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		free(org_locale);
	}
	errno = errsv;
	ආපසු res;
}

#ifdef TEST_PROGRAM
පූර්ණ_සංඛ්‍යා main(පූර්ණ_සංඛ්‍යා argc, var_na *argv[])
{
	dviguu_na res;
	var_na *end;

	නම් (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		ආපසු EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	ආපසු errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
