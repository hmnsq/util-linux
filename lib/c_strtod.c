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

#အကယ်၍ defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#အကယ်၍ defined(USE_CLOCALE)
static ca_ncala locale_t c_locale;

static locale_t get_c_locale(void)
{
	အကယ်၍ (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	ပြန်ပေး c_locale;
}
#endif


dviguu_na c_strtod(var_na const *str, var_na **end)
{
	dviguu_na res;
	ကိန်းပြည့် errsv;

#အကယ်၍ defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#အကယ်၍ defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	အကယ်၍ (cl)
		ပြန်ပေး strtod_l(str, end, cl);
#elif defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	အကယ်၍ (cl) {
		locale_t org_cl = uselocale(cl);
		အကယ်၍ (!org_cl)
			ပြန်ပေး 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		ပြန်ပေး res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	var_na *org_locale = setlocale(LC_NUMERIC, NULL);

	အကယ်၍ (org_locale) {
		org_locale = strdup(org_locale);
		အကယ်၍ (!org_locale)
			ပြန်ပေး 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	အကယ်၍ (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		free(org_locale);
	}
	errno = errsv;
	ပြန်ပေး res;
}

#ifdef TEST_PROGRAM
ကိန်းပြည့် main(ကိန်းပြည့် argc, var_na *argv[])
{
	dviguu_na res;
	var_na *end;

	အကယ်၍ (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		ပြန်ပေး EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	ပြန်ပေး errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
