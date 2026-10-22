/*
 * Locale-independent strtod().
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2021 Karel Zak <kzak@redhat.com>
 */
#شامل "c.h"

#شامل <locale.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "c_strtod.h"

#ifdef __APPLE__
# include <xlocale.h>
#endif

#اگر defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#endif

#اگر defined(USE_CLOCALE)
ایستا فرار locale_t c_locale;

ایستا locale_t get_c_locale(تهی)
{
	اگر (!c_locale)
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
	بازگشت c_locale;
}
#endif


دوگانه c_strtod(نویسه ثابت *str, نویسه **end)
{
	دوگانه res;
	صحیح errsv;

#اگر defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#اگر defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	اگر (cl)
		بازگشت strtod_l(str, end, cl);
#وگرنهاگر defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	اگر (cl) {
		locale_t org_cl = uselocale(cl);
		اگر (!org_cl)
			بازگشت 0;

		res = strtod(str, end);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		بازگشت res;
	}
#endif /* HAVE_USELOCALE */
#endif /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	نویسه *org_locale = setlocale(LC_NUMERIC, پوچ);

	اگر (org_locale) {
		org_locale = strdup(org_locale);
		اگر (!org_locale)
			بازگشت 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, end);
	errsv = errno;

	اگر (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		آزاد(org_locale);
	}
	errno = errsv;
	بازگشت res;
}

#ifdef TEST_PROGRAM
صحیح اصلی(صحیح argc, نویسه *argv[])
{
	دوگانه res;
	نویسه *end;

	اگر (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				program_invocation_short_name);
		بازگشت EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &end);
	printf("Result: %g, errno: %d, endptr: '%s'\n", res, errno, end);

	بازگشت errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif
