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

#ຖ້າ defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#ຖ້າ defined(USE_CLOCALE)
static ca_ncala locale_t c_locale;

static locale_t get_c_locale(void)
{
	ຖ້າ (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	ສົ່ງຄືນ c_locale;
}
#endif


dviguu_na c_strtod(var_na const *str, var_na **end)
{
	dviguu_na res;
	ຈຳນວນເຕັມ errsv;

#ຖ້າ defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#ຖ້າ defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	ຖ້າ (cl)
		ສົ່ງຄືນ strtod_l(str, end, cl);
#elif defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	ຖ້າ (cl) {
		locale_t org_cl = uselocale(cl);
		ຖ້າ (!org_cl)
			ສົ່ງຄືນ 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		ສົ່ງຄືນ res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	var_na *org_locale = setlocale(LC_NUMERIC, NULL);

	ຖ້າ (org_locale) {
		org_locale = strdup(org_locale);
		ຖ້າ (!org_locale)
			ສົ່ງຄືນ 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	ຖ້າ (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		free(org_locale);
	}
	errno = errsv;
	ສົ່ງຄືນ res;
}

#ifdef TEST_PROGRAM
ຈຳນວນເຕັມ main(ຈຳນວນເຕັມ argc, var_na *argv[])
{
	dviguu_na res;
	var_na *end;

	ຖ້າ (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		ສົ່ງຄືນ EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	ສົ່ງຄືນ errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
