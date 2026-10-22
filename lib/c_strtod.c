/*
 * Locale-independent strtod().
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2021 Karel Zak <kzak@redhat.com>
 */
#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <locale.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c_strtod.h"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __APPLE__
# include <xlocale.h>
#ਜੇ_ਅੰਤ

#ਜੇ defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#ਜੇ_ਅੰਤ

#ਜੇ defined(USE_CLOCALE)
ਸਥਿਰ ਅਸਥਿਰ locale_t c_locale;

ਸਥਿਰ locale_t get_c_locale(ਖਾਲੀ)
{
	ਜੇ (!c_locale)
		c_locale = newlocale(LC_ALL_ਨਕਾਬ, "C", (locale_t) 0);
	ਵਾਪਸ c_locale;
}
#ਜੇ_ਅੰਤ


ਦੁੱਗਣਾ c_strtod(ਅੱਖਰ ਅਚਲ *str, ਅੱਖਰ **ਸਿਰਾ)
{
	ਦੁੱਗਣਾ res;
	ਪੂਰਨ errsv;

#ਜੇ defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#ਜੇ defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	ਜੇ (cl)
		ਵਾਪਸ strtod_l(str, ਸਿਰਾ, cl);
#ਜਾਂਜੇ defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	ਜੇ (cl) {
		locale_t org_cl = uselocale(cl);
		ਜੇ (!org_cl)
			ਵਾਪਸ 0;

		res = strtod(str, ਸਿਰਾ);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		ਵਾਪਸ res;
	}
#ਜੇ_ਅੰਤ /* HAVE_USELOCALE */
#ਜੇ_ਅੰਤ /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	ਅੱਖਰ *org_locale = setlocale(LC_NUMERIC, ਨਿਰਾ);

	ਜੇ (org_locale) {
		org_locale = strdup(org_locale);
		ਜੇ (!org_locale)
			ਵਾਪਸ 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, ਸਿਰਾ);
	errsv = errno;

	ਜੇ (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		ਮੁਕਤ(org_locale);
	}
	errno = errsv;
	ਵਾਪਸ res;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ
ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਦੁੱਗਣਾ res;
	ਅੱਖਰ *ਸਿਰਾ;

	ਜੇ (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				ਪ੍ਰੋਗਰਾਮ_invocation_short_name);
		ਵਾਪਸ EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &ਸਿਰਾ);
	ਲਿਖੋ("Result: %g, errno: %d, endptr: '%s'\n", res, errno, ਸਿਰਾ);

	ਵਾਪਸ errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#ਜੇ_ਅੰਤ
