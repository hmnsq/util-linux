/*
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * First set of functions in this file are part of systemd, and were
 * copied to util-linux at August 2013.
 *
 * Copyright 2010 Lennart Poettering
 * Copyright (C) 2014 Karel Zak <kzak@redhat.com>
 *
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 */
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <sys/time.h>
#ಅನ್ತರ್ಭಾವ <inttypes.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "nls.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"
#ಅನ್ತರ್ಭಾವ "timeutils.h"

#ವ್ಯಾಖ್ಯಾನಿಸು WHITESPACE " \t\n\r"

#ವ್ಯಾಖ್ಯಾನಿಸು streq(a,b) (strcmp((a),(b)) == 0)

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_sec(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *t, usec_t *usec)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *suffix;
		usec_t usec;
	} ಕೋಷ್ಟಕ[] = {
		{ "seconds",	USEC_ಪ್ರತಿ_SEC },
		{ "second",	USEC_ಪ್ರತಿ_SEC },
		{ "sec",	USEC_ಪ್ರತಿ_SEC },
		{ "s",		USEC_ಪ್ರತಿ_SEC },
		{ "minutes",	USEC_ಪ್ರತಿ_MINUTE },
		{ "minute",	USEC_ಪ್ರತಿ_MINUTE },
		{ "min",	USEC_ಪ್ರತಿ_MINUTE },
		{ "months",	USEC_ಪ್ರತಿ_MONTH },
		{ "month",	USEC_ಪ್ರತಿ_MONTH },
		{ "msec",	USEC_ಪ್ರತಿ_MSEC },
		{ "ms",		USEC_ಪ್ರತಿ_MSEC },
		{ "m",		USEC_ಪ್ರತಿ_MINUTE },
		{ "hours",	USEC_ಪ್ರತಿ_HOUR },
		{ "hour",	USEC_ಪ್ರತಿ_HOUR },
		{ "hr",		USEC_ಪ್ರತಿ_HOUR },
		{ "h",		USEC_ಪ್ರತಿ_HOUR },
		{ "days",	USEC_ಪ್ರತಿ_DAY },
		{ "day",	USEC_ಪ್ರತಿ_DAY },
		{ "d",		USEC_ಪ್ರತಿ_DAY },
		{ "weeks",	USEC_ಪ್ರತಿ_WEEK },
		{ "week",	USEC_ಪ್ರತಿ_WEEK },
		{ "w",		USEC_ಪ್ರತಿ_WEEK },
		{ "years",	USEC_ಪ್ರತಿ_YEAR },
		{ "year",	USEC_ಪ್ರತಿ_YEAR },
		{ "y",		USEC_ಪ್ರತಿ_YEAR },
		{ "usec",	1ULL },
		{ "us",		1ULL },
		{ "",		USEC_ಪ್ರತಿ_SEC },	/* default is sec */
	};

	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
	usec_t r = 0;
	ಪೂರ್ಣಾಂಕ something = FALSE;

	ಪ್ರತಿಜ್ಞಾ(t);
	ಪ್ರತಿಜ್ಞಾ(usec);

	p = t;
	ಗಾಗಿ (;;) {
		ದೀರ್ಘ ದೀರ್ಘ l, z = 0;
		ಅಕ್ಷರ *e;
		ಚಿಹ್ನೆರಹಿತ i, n = 0;

		p += strspn(p, WHITESPACE);

		ಆದರೆ (*p == 0) {
			ಆದರೆ (!something)
				ಹಿಂತಿರುಗಿ -EINVAL;

			ನಿಲ್ಲಿಸು;
		}

		errno = 0;
		l = strtoll(p, &e, 10);

		ಆದರೆ (errno > 0)
			ಹಿಂತಿರುಗಿ -errno;

		ಆದರೆ (l < 0)
			ಹಿಂತಿರುಗಿ -ERANGE;

		ಆದರೆ (*e == '.') {
			ಅಕ್ಷರ *b = e + 1;

			errno = 0;
			z = strtoll(b, &e, 10);
			ಆದರೆ (errno > 0)
				ಹಿಂತಿರುಗಿ -errno;

			ಆದರೆ (z < 0)
				ಹಿಂತಿರುಗಿ -ERANGE;

			ಆದರೆ (e == b)
				ಹಿಂತಿರುಗಿ -EINVAL;

			n = e - b;

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (e == p)
			ಹಿಂತಿರುಗಿ -EINVAL;

		e += strspn(e, WHITESPACE);

		ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(ಕೋಷ್ಟಕ); i++)
			ಆದರೆ (ul_startswith(e, ಕೋಷ್ಟಕ[i].suffix)) {
				usec_t k = (usec_t) z * ಕೋಷ್ಟಕ[i].usec;

				ಗಾಗಿ (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *ಕೋಷ್ಟಕ[i].usec + k;
				p = e + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಕೋಷ್ಟಕ[i].suffix);

				something = TRUE;
				ನಿಲ್ಲಿಸು;
			}

		ಆದರೆ (i >= ARRAY_ಗಾತ್ರ(ಕೋಷ್ಟಕ))
			ಹಿಂತಿರುಗಿ -EINVAL;

	}

	*usec = r;

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_subseconds(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t ಗುಣಕ = USEC_ಪ್ರತಿ_SEC / 10;

	ಆದರೆ (*t != '.' && *t != ',')
		ಹಿಂತಿರುಗಿ -1;

	ಆಗ (*(++t)) {
		ಆದರೆ (!ಅಂಕಿಯೇ(*t) || ಗುಣಕ < 1)
			ಹಿಂತಿರುಗಿ -1;

		ret += ((usec_t) *t - '0') * ಗುಣಕ;
		ಗುಣಕ /= 10;
	}

	*usec = ret;
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *parse_epoch_seconds(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *t, ರಚನೆ tm *tm)
{
	int64_t s;
	time_t st;
	ಪೂರ್ಣಾಂಕ f, c;

	f = ಅಕ್ಷರಸರಣಿ_ಓದು(t, "%"SCNd64"%n", &s, &c);
	ಆದರೆ (f < 1)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	st = s;
	ಆದರೆ ((int64_t) st < s)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	ಆದರೆ (!localtime_r(&st, tm))
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	ಹಿಂತಿರುಗಿ t + c;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_timestamp_reference(time_t x, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *t, usec_t *usec)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
		ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ nr;
	} day_nr[] = {
		{ "Sunday",	0 },
		{ "Sun",	0 },
		{ "Monday",	1 },
		{ "Mon",	1 },
		{ "Tuesday",	2 },
		{ "Tue",	2 },
		{ "Wednesday",	3 },
		{ "Wed",	3 },
		{ "Thursday",	4 },
		{ "Thu",	4 },
		{ "Friday",	5 },
		{ "Fri",	5 },
		{ "Saturday",	6 },
		{ "Sat",	6 },
	};

	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *k;
	ರಚನೆ tm tm, copy;
	usec_t plus = 0, minus = 0, ret = 0;
	ಪೂರ್ಣಾಂಕ r, weekday = -1;
	ಚಿಹ್ನೆರಹಿತ i;

	/*
	 * Allowed syntaxes:
	 *
	 *   2012-09-22 16:34:22 !
	 *   2012-09-22T16:34:22 !
	 *   20120922163422      !
	 *   @1348331662	 ! (seconds since the Epoch (1970-01-01 00:00 UTC))
	 *   2012-09-22 16:34	   (seconds will be set to 0)
	 *   2012-09-22		   (time will be set to 00:00:00)
	 *   16:34:22		 ! (date will be set to today)
	 *   16:34		   (date will be set to today, seconds to 0)
	 *   now
	 *   yesterday		   (time is set to 00:00:00)
	 *   today		   (time is set to 00:00:00)
	 *   tomorrow		   (time is set to 00:00:00)
	 *   +5min
	 *   -5days
	 *
	 *   Syntaxes marked with '!' also optionally allow up to six digits of
	 *   subsecond granularity, separated by '.' or ',':
	 *
	 *   2012-09-22 16:34:22.12
	 *   2012-09-22 16:34:22.123456
	 *
	 *
	 */

	ಪ್ರತಿಜ್ಞಾ(t);
	ಪ್ರತಿಜ್ಞಾ(usec);

	localtime_r(&x, &tm);
	tm.tm_isdst = -1;

	ಆದರೆ (streq(t, "now"))
		ಹೋಗು ಮುಕ್ತಾಯ;

	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (streq(t, "today")) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		ಹೋಗು ಮುಕ್ತಾಯ;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (streq(t, "yesterday")) {
		tm.tm_mday--;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		ಹೋಗು ಮುಕ್ತಾಯ;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (streq(t, "tomorrow")) {
		tm.tm_mday++;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		ಹೋಗು ಮುಕ್ತಾಯ;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		ಆದರೆ (r < 0)
			ಹಿಂತಿರುಗಿ r;

		ಹೋಗು ಮುಕ್ತಾಯ;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		ಆದರೆ (r < 0)
			ಹಿಂತಿರುಗಿ r;

		ಹೋಗು ಮುಕ್ತಾಯ;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &tm);
		ಆದರೆ (k && *k == 0)
			ಹೋಗು ಮುಕ್ತಾಯ;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (k && parse_subseconds(k, &ret) == 0)
			ಹೋಗು ಮುಕ್ತಾಯ;

		ಹಿಂತಿರುಗಿ -EINVAL;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ul_endswith(t, " ago")) {
		ಅಕ್ಷರ *z;

		z = strndup(t, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(t) - 4);
		ಆದರೆ (!z)
			ಹಿಂತಿರುಗಿ -ENOMEM;

		r = parse_sec(z, &minus);
		ಬಿಡುಗಡೆ(z);
		ಆದರೆ (r < 0)
			ಹಿಂತಿರುಗಿ r;

		ಹೋಗು ಮುಕ್ತಾಯ;
	}

	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(day_nr); i++) {
		size_t skip;

		ಆದರೆ (!startswith_no_case(t, day_nr[i].name))
			ಮುಂದುವರಿ;

		skip = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(day_nr[i].name);
		ಆದರೆ (t[skip] != ' ')
			ಮುಂದುವರಿ;

		weekday = day_nr[i].nr;
		t += skip + 1;
		ನಿಲ್ಲಿಸು;
	}

	copy = tm;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &tm);
	ಆದರೆ (k && *k == 0)
		ಹೋಗು ಮುಕ್ತಾಯ;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (k && parse_subseconds(k, &ret) == 0)
		ಹೋಗು ಮುಕ್ತಾಯ;

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &tm);
	ಆದರೆ (k && *k == 0)
		ಹೋಗು ಮುಕ್ತಾಯ;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (k && parse_subseconds(k, &ret) == 0)
		ಹೋಗು ಮುಕ್ತಾಯ;

	tm = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &tm);
	ಆದರೆ (k && *k == 0)
		ಹೋಗು ಮುಕ್ತಾಯ;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (k && parse_subseconds(k, &ret) == 0)
		ಹೋಗು ಮುಕ್ತಾಯ;

	tm = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &tm);
	ಆದರೆ (k && *k == 0) {
		tm.tm_sec = 0;
		ಹೋಗು ಮುಕ್ತಾಯ;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &tm);
	ಆದರೆ (k && *k == 0) {
		tm.tm_sec = 0;
		ಹೋಗು ಮುಕ್ತಾಯ;
	}

	tm = copy;
	k = strptime(t, "%y-%m-%d", &tm);
	ಆದರೆ (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		ಹೋಗು ಮುಕ್ತಾಯ;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d", &tm);
	ಆದರೆ (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		ಹೋಗು ಮುಕ್ತಾಯ;
	}

	tm = copy;
	k = strptime(t, "%H:%M:%S", &tm);
	ಆದರೆ (k && *k == 0)
		ಹೋಗು ಮುಕ್ತಾಯ;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (k && parse_subseconds(k, &ret) == 0)
		ಹೋಗು ಮುಕ್ತಾಯ;

	tm = copy;
	k = strptime(t, "%H:%M", &tm);
	ಆದರೆ (k && *k == 0) {
		tm.tm_sec = 0;
		ಹೋಗು ಮುಕ್ತಾಯ;
	}

	tm = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &tm);
	ಆದರೆ (k && *k == 0)
		ಹೋಗು ಮುಕ್ತಾಯ;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (k && parse_subseconds(k, &ret) == 0)
		ಹೋಗು ಮುಕ್ತಾಯ;

	ಹಿಂತಿರುಗಿ -EINVAL;

 ಮುಕ್ತಾಯ:
	x = mktime(&tm);
	ಆದರೆ (x == (time_t)-1)
		ಹಿಂತಿರುಗಿ -EINVAL;

	ಆದರೆ (weekday >= 0 && tm.tm_wday != weekday)
		ಹಿಂತಿರುಗಿ -EINVAL;

	ret += (usec_t) x * USEC_ಪ್ರತಿ_SEC;

	ಆದರೆ (minus > ret)
		ಹಿಂತಿರುಗಿ -ERANGE;
	ಆದರೆ ((ret + plus) < ret)
		ಹಿಂತಿರುಗಿ -ERANGE;

	ret += plus;
	ret -= minus;

	*usec = ret;

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ ul_parse_timestamp(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *t, usec_t *usec)
{
	ಹಿಂತಿರುಗಿ parse_timestamp_reference(time(ಖಾಲಿ), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
ಪೂರ್ಣಾಂಕ get_gmtoff(ಸ್ಥಿರಾಂಕ ರಚನೆ tm *tp)
{
	ಆದರೆ (tp->tm_isdst < 0)
		ಹಿಂತಿರುಗಿ 0;

#ಆದರೆ HAVE_TM_GMTOFF
	ಹಿಂತಿರುಗಿ tp->tm_gmtoff;
#ಇಲ್ಲದಿದ್ದರೆ
	ರಚನೆ tm tm;
	ರಚನೆ tm gtm;
	ರಚನೆ tm ltm = *tp;
	time_t lt;

	tzset();
	lt = mktime(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	ಆದರೆ (lt == (time_t) -1) {
		ಆದರೆ (! localtime_r(&lt, &tm)
			|| ((ltm.tm_sec ^ tm.tm_sec)
			    | (ltm.tm_min ^ tm.tm_min)
			    | (ltm.tm_hour ^ tm.tm_hour)
			    | (ltm.tm_mday ^ tm.tm_mday)
			    | (ltm.tm_mon ^ tm.tm_mon)
			    | (ltm.tm_year ^ tm.tm_year)))
			ಹಿಂತಿರುಗಿ 0;
	}

	ಆದರೆ (! gmtime_r(&lt, &gtm))
		ಹಿಂತಿರುಗಿ 0;

	/* Calculate the GMT offset, that is, the difference between the
	 * TP argument (ltm) and GMT (gtm).
	 *
	 * Compute intervening leap days correctly even if year is negative.
	 * Take care to avoid int overflow in leap day calculations, but it's OK
	 * to assume that A and B are close to each other.
	 */
	ಪೂರ್ಣಾಂಕ a4 = (ltm.tm_year >> 2) + (1900 >> 2) - ! (ltm.tm_year & 3);
	ಪೂರ್ಣಾಂಕ b4 = (gtm.tm_year >> 2) + (1900 >> 2) - ! (gtm.tm_year & 3);
	ಪೂರ್ಣಾಂಕ a100 = a4 / 25 - (a4 % 25 < 0);
	ಪೂರ್ಣಾಂಕ b100 = b4 / 25 - (b4 % 25 < 0);
	ಪೂರ್ಣಾಂಕ a400 = a100 >> 2;
	ಪೂರ್ಣಾಂಕ b400 = b100 >> 2;
	ಪೂರ್ಣಾಂಕ intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

	ಪೂರ್ಣಾಂಕ years = ltm.tm_year - gtm.tm_year;
	ಪೂರ್ಣಾಂಕ days = (365 * years + intervening_leap_days
		    + (ltm.tm_yday - gtm.tm_yday));

	ಹಿಂತಿರುಗಿ (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ format_iso_ಸಮಯ(ಸ್ಥಿರಾಂಕ ರಚನೆ tm *tm, uint32_t nsec, ಪೂರ್ಣಾಂಕ flags, ಅಕ್ಷರ *buf, size_t bufsz)
{
	uint32_t usec = nsec / NSEC_ಪ್ರತಿ_USEC;
	ಅಕ್ಷರ *p = buf;
	ಪೂರ್ಣಾಂಕ len;

	ಆದರೆ (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       tm->tm_year + (ದೀರ್ಘ) 1900,
			       tm->tm_mon + 1, tm->tm_mday);
		ಆದರೆ (len < 0 || (size_t) len > bufsz)
			ಹೋಗು err;
		bufsz -= len;
		p += len;
	}

	ಆದರೆ ((flags & ISO_DATE) && (flags & ISO_ಸಮಯ)) {
		ಆದರೆ (bufsz < 1)
			ಹೋಗು err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	ಆದರೆ (flags & ISO_ಸಮಯ) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
			       tm->tm_min, tm->tm_sec);
		ಆದರೆ (len < 0 || (size_t) len > bufsz)
			ಹೋಗು err;
		bufsz -= len;
		p += len;
	}

	ಆದರೆ (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		ಆದರೆ (len < 0 || (size_t) len > bufsz)
			ಹೋಗು err;
		bufsz -= len;
		p += len;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		ಆದರೆ (len < 0 || (size_t) len > bufsz)
			ಹೋಗು err;
		bufsz -= len;
		p += len;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		ಆದರೆ (len < 0 || (size_t) len > bufsz)
			ಹೋಗು err;
		bufsz -= len;
		p += len;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		ಆದರೆ (len < 0 || (size_t) len > bufsz)
			ಹೋಗು err;
		bufsz -= len;
		p += len;
	}

	ಆದರೆ (flags & ISO_TIMEZONE) {
		ಪೂರ್ಣಾಂಕ tmin  = get_gmtoff(tm) / 60;
		ಪೂರ್ಣಾಂಕ zhour = tmin / 60;
		ಪೂರ್ಣಾಂಕ zmin  = ನಿರಪೇಕ್ಷ(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		ಆದರೆ (len < 0 || (size_t) len > bufsz)
			ಹೋಗು err;
	}
	ಹಿಂತಿರುಗಿ 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	ಹಿಂತಿರುಗಿ -1;
}

/* timespec to ISO 8601 */
ಪೂರ್ಣಾಂಕ strtimespec_iso(ಸ್ಥಿರಾಂಕ ರಚನೆ timespec *ts, ಪೂರ್ಣಾಂಕ flags, ಅಕ್ಷರ *buf, size_t bufsz)
{
	ರಚನೆ tm tm;
	ರಚನೆ tm *rc;

	ಆದರೆ (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &tm);
	ಇಲ್ಲದಿದ್ದರೆ
		rc = localtime_r(&ts->tv_sec, &tm);

	ಆದರೆ (rc)
		ಹಿಂತಿರುಗಿ format_iso_ಸಮಯ(&tm, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	ಹಿಂತಿರುಗಿ -1;
}

/* timeval to ISO 8601 */
ಪೂರ್ಣಾಂಕ strtimeval_iso(ಸ್ಥಿರಾಂಕ ರಚನೆ timeval *tv, ಪೂರ್ಣಾಂಕ flags, ಅಕ್ಷರ *buf, size_t bufsz)
{
	ರಚನೆ timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_ಪ್ರತಿ_USEC,
	};

	ಹಿಂತಿರುಗಿ strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
ಪೂರ್ಣಾಂಕ strtm_iso(ಸ್ಥಿರಾಂಕ ರಚನೆ tm *tm, ಪೂರ್ಣಾಂಕ flags, ಅಕ್ಷರ *buf, size_t bufsz)
{
	ಹಿಂತಿರುಗಿ format_iso_ಸಮಯ(tm, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
ಪೂರ್ಣಾಂಕ strtime_iso(ಸ್ಥಿರಾಂಕ time_t *t, ಪೂರ್ಣಾಂಕ flags, ಅಕ್ಷರ *buf, size_t bufsz)
{
	ರಚನೆ tm tm;
	ರಚನೆ tm *rc;

	ಆದರೆ (flags & ISO_GMTIME)
		rc = gmtime_r(t, &tm);
	ಇಲ್ಲದಿದ್ದರೆ
		rc = localtime_r(t, &tm);

	ಆದರೆ (rc)
		ಹಿಂತಿರುಗಿ format_iso_ಸಮಯ(&tm, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	ಹಿಂತಿರುಗಿ -1;
}

/* relative time functions */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ ಸಮಯ_is_thisyear(ರಚನೆ tm ಸ್ಥಿರಾಂಕ *ಸ್ಥಿರಾಂಕ tm,
				   ರಚನೆ tm ಸ್ಥಿರಾಂಕ *ಸ್ಥಿರಾಂಕ tmnow)
{
	ಹಿಂತಿರುಗಿ tm->tm_year == tmnow->tm_year;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ ಸಮಯ_is_today(ರಚನೆ tm ಸ್ಥಿರಾಂಕ *ಸ್ಥಿರಾಂಕ tm,
				ರಚನೆ tm ಸ್ಥಿರಾಂಕ *ಸ್ಥಿರಾಂಕ tmnow)
{
	ಹಿಂತಿರುಗಿ (tm->tm_yday == tmnow->tm_yday &&
		ಸಮಯ_is_thisyear(tm, tmnow));
}

ಪೂರ್ಣಾಂಕ strtime_short(ಸ್ಥಿರಾಂಕ time_t *t, ರಚನೆ timeval *now, ಪೂರ್ಣಾಂಕ flags, ಅಕ್ಷರ *buf, size_t bufsz)
{
	ರಚನೆ tm tm, tmnow;
	ಪೂರ್ಣಾಂಕ rc = 0;

	ಆದರೆ (now->tv_sec == 0)
		gettimeofday(now, ಖಾಲಿ);

	localtime_r(t, &tm);
	localtime_r(&now->tv_sec, &tmnow);

	ಆದರೆ (ಸಮಯ_is_today(&tm, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", tm.tm_hour, tm.tm_min);
		ಆದರೆ (rc < 0 || (size_t) rc > bufsz)
			ಹಿಂತಿರುಗಿ -1;
		rc = 1;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಸಮಯ_is_thisyear(&tm, &tmnow)) {
		ಆದರೆ (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = strftime(buf, bufsz, "%b%d/%H:%M", &tm);
		ಇಲ್ಲದಿದ್ದರೆ
			rc = strftime(buf, bufsz, "%b%d", &tm);
	} ಇಲ್ಲದಿದ್ದರೆ
		rc = strftime(buf, bufsz, "%Y-%b%d", &tm);

	ಹಿಂತಿರುಗಿ rc <= 0 ? -1 : 0;
}

ಪೂರ್ಣಾಂಕ strtimespec_relative(ಸ್ಥಿರಾಂಕ ರಚನೆ timespec *ts, ಅಕ್ಷರ *buf, size_t bufsz)
{
	time_t secs = ts->tv_sec;
	size_t i, parts = 0;
	ಪೂರ್ಣಾಂಕ rc;

	ಆದರೆ (bufsz)
		buf[0] = '\0';

	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ suffix;
		ಪೂರ್ಣಾಂಕ ಅಗಲ;
		int64_t secs;
	} ಕೋಷ್ಟಕ[] = {
		{ "y", 4, NSEC_ಪ್ರತಿ_YEAR   / NSEC_ಪ್ರತಿ_SEC },
		{ "d", 3, NSEC_ಪ್ರತಿ_DAY    / NSEC_ಪ್ರತಿ_SEC },
		{ "h", 2, NSEC_ಪ್ರತಿ_HOUR   / NSEC_ಪ್ರತಿ_SEC },
		{ "m", 2, NSEC_ಪ್ರತಿ_MINUTE / NSEC_ಪ್ರತಿ_SEC },
		{ "s", 2, NSEC_ಪ್ರತಿ_SEC    / NSEC_ಪ್ರತಿ_SEC },
	};

	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(ಕೋಷ್ಟಕ); i++) {
		ಆದರೆ (secs >= ಕೋಷ್ಟಕ[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? ಕೋಷ್ಟಕ[i].ಅಗಲ : 0,
				      secs / ಕೋಷ್ಟಕ[i].secs, ಕೋಷ್ಟಕ[i].suffix,
				      secs % ಕೋಷ್ಟಕ[i].secs ? " " : "");
			ಆದರೆ (rc < 0 || (size_t) rc > bufsz)
				ಹೋಗು err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= ಕೋಷ್ಟಕ[i].secs;
		}
	}

	ಆದರೆ (ts->tv_nsec) {
		ಆದರೆ (ts->tv_nsec % NSEC_ಪ್ರತಿ_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			ಆದರೆ (rc < 0 || (size_t) rc > bufsz)
				ಹೋಗು err;
		} ಇಲ್ಲದಿದ್ದರೆ {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_ಪ್ರತಿ_MSEC);
			ಆದರೆ (rc < 0 || (size_t) rc > bufsz)
				ಹೋಗು err;
		}
	}

	ಹಿಂತಿರುಗಿ 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	ಹಿಂತಿರುಗಿ -1;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ HAVE_TIMEGM
time_t timegm(ರಚನೆ tm *tm)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zone = getenv("TZ");
	time_t ret;

	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	ಆದರೆ (zone)
		setenv("TZ", zone, 1);
	ಇಲ್ಲದಿದ್ದರೆ
		unsetenv("TZ");
	tzset();
	ಹಿಂತಿರುಗಿ ret;
}
#ಆದರೆ_ಅಂತ್ಯ /* HAVE_TIMEGM */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_TIMEUTILS

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸತತ_unittest_timestamp(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ rc = EXIT_SUCCESS;
	time_t reference = 1674180427;
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ testcase {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ ಒಳಹರಿವು;
		usec_t expected;
	} testcases[] = {
		{ "2012-09-22 16:34:22"    , 1348331662000000 },
		{ "2012-09-22 16:34:22,012", 1348331662012000 },
		{ "2012-09-22 16:34:22.012", 1348331662012000 },
		{ "@1348331662"            , 1348331662000000 },
		{ "@1348331662.234567"     , 1348331662234567 },
		{ "@0"                     ,                0 },
		{ "2012-09-22 16:34"       , 1348331640000000 },
		{ "2012-09-22"             , 1348272000000000 },
		{ "16:34:22"               , 1674232462000000 },
		{ "16:34:22,123456"        , 1674232462123456 },
		{ "16:34:22.123456"        , 1674232462123456 },
		{ "16:34"                  , 1674232440000000 },
		{ "now"                    , 1674180427000000 },
		{ "yesterday"              , 1674086400000000 },
		{ "today"                  , 1674172800000000 },
		{ "tomorrow"               , 1674259200000000 },
		{ "+5min"                  , 1674180727000000 },
		{ "-5days"                 , 1673748427000000 },
		{ "20120922163422"         , 1348331662000000 },
	};

	setenv("TZ", "GMT", 1);
	tzset();

	ಗಾಗಿ (size_t i = 0; i < ARRAY_ಗಾತ್ರ(testcases); i++) {
		ರಚನೆ testcase t = testcases[i];
		usec_t ಫಲಿತಾಂಶ;
		ಪೂರ್ಣಾಂಕ r = parse_timestamp_reference(reference, t.ಒಳಹರಿವು, &ಫಲಿತಾಂಶ);
		ಆದರೆ (r) {
			fprintf(stderr, "Could not parse '%s'\n", t.ಒಳಹರಿವು);
			rc = EXIT_FAILURE;
		}

		ಆದರೆ (ಫಲಿತಾಂಶ != t.expected) {
			fprintf(stderr, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.ಒಳಹರಿವು, ಫಲಿತಾಂಶ, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸತತ_unittest_format(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ rc = EXIT_SUCCESS;
	ಸ್ಥಿರಾಂಕ ರಚನೆ timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	ಅಕ್ಷರ buf[FORMAT_TIMESTAMP_MAX];
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ testcase {
		ಪೂರ್ಣಾಂಕ flags;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ expected;
	} testcases[] = {
		{ ISO_DATE,               "2023-01-20"                       },
		{ ISO_ಸಮಯ,               "02:07:07"                         },
		{ ISO_TIMEZONE,           "+00:00"                           },
		{ ISO_TIMESTAMP_T,        "2023-01-20T02:07:07+00:00"        },
		{ ISO_TIMESTAMP_COMMA_G,  "2023-01-20 02:07:07,000012+00:00" },
		{ ISO_ಸಮಯ | ISO_DOTNSEC, "02:07:07.000012345" },
	};

	setenv("TZ", "GMT", 1);
	tzset();

	ಗಾಗಿ (size_t i = 0; i < ARRAY_ಗಾತ್ರ(testcases); i++) {
		ರಚನೆ testcase t = testcases[i];
		ಪೂರ್ಣಾಂಕ r = strtimespec_iso(&ts, t.flags, buf, ಗಾತ್ರ(buf));
		ಆದರೆ (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		ಆದರೆ (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸತತ_unittest_format_relative(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ rc = EXIT_SUCCESS;
	ಅಕ್ಷರ buf[FORMAT_TIMESTAMP_MAX];
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ testcase {
		ರಚನೆ timespec ts;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ expected;
	} testcases[] = {
		{{}, "" },
		{{         1 },                  "1s" },
		{{        10 },                 "10s" },
		{{       100 },              "1m 40s" },
		{{      1000 },             "16m 40s" },
		{{     10000 },          "2h 46m 40s" },
		{{    100000 },      "1d  3h 46m 40s" },
		{{   1000000 },     "11d 13h 46m 40s" },
		{{  10000000 },    "115d 17h 46m 40s" },
		{{ 100000000 }, "3y  61d 15h 46m 40s" },
		{{        60 },                  "1m" },
		{{      3600 },                  "1h" },

		{{ 1,       1 }, "1s         1ns" },
		{{ 0,       1 },            "1ns" },
		{{ 0, 1000000 },            "1ms" },
		{{ 0, 1000001 },      "1000001ns" },
	};

	setenv("TZ", "GMT", 1);
	tzset();

	ಗಾಗಿ (size_t i = 0; i < ARRAY_ಗಾತ್ರ(testcases); i++) {
		ರಚನೆ testcase t = testcases[i];
		ಪೂರ್ಣಾಂಕ r = strtimespec_relative(&t.ts, buf, ಗಾತ್ರ(buf));
		ಆದರೆ (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		ಆದರೆ (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ರಚನೆ timespec ts = { 0 };
	ಅಕ್ಷರ buf[ISO_BUFSIZ];
	ಪೂರ್ಣಾಂಕ r;

	ಆದರೆ (argc < 2) {
		fprintf(stderr, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
	}

	ಆದರೆ (strcmp(argv[1], "--unittest-timestamp") == 0)
		ಹಿಂತಿರುಗಿ ಸತತ_unittest_timestamp();
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(argv[1], "--unittest-format") == 0)
		ಹಿಂತಿರುಗಿ ಸತತ_unittest_format();
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(argv[1], "--unittest-format-relative") == 0)
		ಹಿಂತಿರುಗಿ ಸತತ_unittest_format_relative();

	ಆದರೆ (strcmp(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		ಆದರೆ (r)
			errx(EXIT_FAILURE, "Can not parse '%s': %s", argv[2], strerror(-r));
		ts.tv_sec = (time_t) (usec / USEC_ಪ್ರತಿ_SEC);
		ts.tv_nsec = (usec % USEC_ಪ್ರತಿ_SEC) * NSEC_ಪ್ರತಿ_USEC;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		ಆದರೆ (argc == 3)
			ts.tv_nsec = strtos64_or_err(argv[2], "failed to parse <usec>")
				     * NSEC_ಪ್ರತಿ_USEC;
	}

	strtimespec_iso(&ts, ISO_DATE, buf, ಗಾತ್ರ(buf));
	ಬರೆ("Date: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_ಸಮಯ, buf, ಗಾತ್ರ(buf));
	ಬರೆ("Time: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_DATE | ISO_ಸಮಯ | ISO_COMMAUSEC | ISO_T,
		       buf, ಗಾತ್ರ(buf));
	ಬರೆ("Full: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIMESTAMP_DOT, buf, ಗಾತ್ರ(buf));
	ಬರೆ("Zone: '%s'\n", buf);

	strtimespec_relative(&ts, buf, ಗಾತ್ರ(buf));
	ಬರೆ("Rel:  '%s'\n", buf);

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

#ಆದರೆ_ಅಂತ್ಯ /* TEST_PROGRAM_TIMEUTILS */
