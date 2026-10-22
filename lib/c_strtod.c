/*
 * Locale-independent strtod().
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2021 Karel Zak <kzak@redhat.com>
 */
#అన్తర్భావ "c.h"

#అన్తర్భావ <locale.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "c_strtod.h"

#నిర్వచితమైతే __APPLE__
# include <xlocale.h>
#అయితే_ముగింపు

#ఒకవేళ defined(HAVE_NEWLOCALE) && (defined(HAVE_STRTOD_L) || defined(HAVE_USELOCALE))
# define USE_CLOCALE
#అయితే_ముగింపు

#ఒకవేళ defined(USE_CLOCALE)
స్థిర అస్థిర locale_t c_locale;

స్థిర locale_t get_c_locale(శూన్య)
{
	ఒకవేళ (!c_locale)
		c_locale = newlocale(LC_ALL_ముసుగు, "C", (locale_t) 0);
	తిరిగి c_locale;
}
#అయితే_ముగింపు


ద్విగుణ c_strtod(అక్షరం స్థిరరాశి *str, అక్షరం **చివర)
{
	ద్విగుణ res;
	పూర్ణాంకం errsv;

#ఒకవేళ defined(USE_CLOCALE)
	locale_t cl = get_c_locale();

#ఒకవేళ defined(HAVE_STRTOD_L)
	/*
	 * A) try strtod_l() for "C" locale
	 */
	ఒకవేళ (cl)
		తిరిగి strtod_l(str, చివర, cl);
#లేదావేళ defined(HAVE_USELOCALE)
	/*
	 * B) classic strtod(), but switch to "C" locale by uselocale()
	 */
	ఒకవేళ (cl) {
		locale_t org_cl = uselocale(cl);
		ఒకవేళ (!org_cl)
			తిరిగి 0;

		res = strtod(str, చివర);
		errsv = errno;

		uselocale(org_cl);
		errno = errsv;
		తిరిగి res;
	}
#అయితే_ముగింపు /* HAVE_USELOCALE */
#అయితే_ముగింపు /* USE_CLOCALE */
	/*
	 * C) classic strtod(), but switch to "C" locale by setlocale()
	 */
	అక్షరం *org_locale = setlocale(LC_NUMERIC, ఖాళీ);

	ఒకవేళ (org_locale) {
		org_locale = strdup(org_locale);
		ఒకవేళ (!org_locale)
			తిరిగి 0;

		setlocale(LC_NUMERIC, "C");
	}
	res = strtod(str, చివర);
	errsv = errno;

	ఒకవేళ (org_locale) {
		setlocale(LC_NUMERIC, org_locale);
		విడుదల(org_locale);
	}
	errno = errsv;
	తిరిగి res;
}

#నిర్వచితమైతే TEST_కార్యక్రమం
పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[])
{
	ద్విగుణ res;
	అక్షరం *చివర;

	ఒకవేళ (argc < 2) {
		fprintf(stderr, "usage: %s decimal.number\n",
				కార్యక్రమం_invocation_short_name);
		తిరిగి EXIT_FAILURE;
	}

	res = c_strtod(argv[1], &చివర);
	రాయి("Result: %g, errno: %d, endptr: '%s'\n", res, errno, చివర);

	తిరిగి errno ? EXIT_FAILURE : EXIT_SUCCESS;
}
#అయితే_ముగింపు
