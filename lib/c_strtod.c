/*
 * Locale-independent strtod().
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2021 Karel Zak <kzak@redhat.com>
 */
#அந்தர்பாவ "c.h"

#அந்தர்பாவ <locale.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "c_strtod.h"

#ifdef __APPLE__
# include <xlocale.h>
#endif

#எனில் defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#எனில் defined(USE_CLOCALE)
நிலையான நிலையற்ற locale_t c_locale;

நிலையான locale_t get_c_locale(வெறுமை)
{
	எனில் (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	திரும்பு c_locale;
}
#endif


இரட்டை c_strtod(எழுத்து மாறிலி *str, எழுத்து **end)
{
	இரட்டை res;
	முழு errsv;

#எனில் defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#எனில் defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	எனில் (cl)
		திரும்பு strtod_l(str, end, cl);
#அல்லதெனில் defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	எனில் (cl) {
		locale_t org_cl = uselocale(cl);
		எனில் (!org_cl)
			திரும்பு 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		திரும்பு res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	எழுத்து *org_locale = setlocale(LC_NUMERIC, வெற்று);

	எனில் (org_locale) {
		org_locale = strdup(org_locale);
		எனில் (!org_locale)
			திரும்பு 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	எனில் (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		விடுவி(org_locale);
	}
	errno = errsv;
	திரும்பு res;
}

#ifdef TEST_PROGRAM
முழு முதன்மை(முழு argc, எழுத்து *argv[])
{
	இரட்டை res;
	எழுத்து *end;

	எனில் (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		திரும்பு EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	திரும்பு errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
