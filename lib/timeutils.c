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

#define WHITESPACE " \t\n\r"

#define streq(a,b) (strcmp((a),(b)) == 0)

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_sec(ಧ್ರುವ ವರ್ಣ *t, usec_t *usec)
{
	ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ {
		ಧ್ರುವ ವರ್ಣ *suffix;
		usec_t usec;
	} table[] = {
		{ "seconds",	USEC_PER_SEC },
		{ "second",	USEC_PER_SEC },
		{ "sec",	USEC_PER_SEC },
		{ "s",		USEC_PER_SEC },
		{ "minutes",	USEC_PER_MINUTE },
		{ "minute",	USEC_PER_MINUTE },
		{ "min",	USEC_PER_MINUTE },
		{ "months",	USEC_PER_MONTH },
		{ "month",	USEC_PER_MONTH },
		{ "msec",	USEC_PER_MSEC },
		{ "ms",		USEC_PER_MSEC },
		{ "m",		USEC_PER_MINUTE },
		{ "hours",	USEC_PER_HOUR },
		{ "hour",	USEC_PER_HOUR },
		{ "hr",		USEC_PER_HOUR },
		{ "h",		USEC_PER_HOUR },
		{ "days",	USEC_PER_DAY },
		{ "day",	USEC_PER_DAY },
		{ "d",		USEC_PER_DAY },
		{ "weeks",	USEC_PER_WEEK },
		{ "week",	USEC_PER_WEEK },
		{ "w",		USEC_PER_WEEK },
		{ "years",	USEC_PER_YEAR },
		{ "year",	USEC_PER_YEAR },
		{ "y",		USEC_PER_YEAR },
		{ "usec",	1ULL },
		{ "us",		1ULL },
		{ "",		USEC_PER_SEC },	/* default is sec */
	};

	ಧ್ರುವ ವರ್ಣ *p;
	usec_t r = 0;
	ಪೂರ್ಣಾಙ್ಕ something = FALSE;

	ಪ್ರತಿಜ್ಞಾ(t);
	ಪ್ರತಿಜ್ಞಾ(usec);

	p = t;
	ಪ್ರತಿ (;;) {
		long long l, z = 0;
		ವರ್ಣ *e;
		unsigned i, n = 0;

		p += strspn(p, WHITESPACE);

		ಯದಿ (*p == 0) {
			ಯದಿ (!something)
				ಪ್ರತ್ಯಾಗಮ -EINVAL;

			ಭಙ್ಗ;
		}

		errno = 0;
		l = strtoll(p, &e, 10);

		ಯದಿ (errno > 0)
			ಪ್ರತ್ಯಾಗಮ -errno;

		ಯದಿ (l < 0)
			ಪ್ರತ್ಯಾಗಮ -ERANGE;

		ಯದಿ (*e == '.') {
			ವರ್ಣ *b = e + 1;

			errno = 0;
			z = strtoll(b, &e, 10);
			ಯದಿ (errno > 0)
				ಪ್ರತ್ಯಾಗಮ -errno;

			ಯದಿ (z < 0)
				ಪ್ರತ್ಯಾಗಮ -ERANGE;

			ಯದಿ (e == b)
				ಪ್ರತ್ಯಾಗಮ -EINVAL;

			n = e - b;

		} ಅನ್ಯಥಾ ಯದಿ (e == p)
			ಪ್ರತ್ಯಾಗಮ -EINVAL;

		e += strspn(e, WHITESPACE);

		ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(table); i++)
			ಯದಿ (ul_startswith(e, table[i].suffix)) {
				usec_t k = (usec_t) z * table[i].usec;

				ಪ್ರತಿ (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *table[i].usec + k;
				p = e + strlen(table[i].suffix);

				something = TRUE;
				ಭಙ್ಗ;
			}

		ಯದಿ (i >= ARRAY_SIZE(table))
			ಪ್ರತ್ಯಾಗಮ -EINVAL;

	}

	*usec = r;

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_subseconds(ಧ್ರುವ ವರ್ಣ *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t factor = USEC_PER_SEC / 10;

	ಯದಿ (*t != '.' && *t != ',')
		ಪ್ರತ್ಯಾಗಮ -1;

	ಯಾವತ್ (*(++t)) {
		ಯದಿ (!isdigit(*t) || factor < 1)
			ಪ್ರತ್ಯಾಗಮ -1;

		ret += ((usec_t) *t - '0') * factor;
		factor /= 10;
	}

	*usec = ret;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *parse_epoch_seconds(ಧ್ರುವ ವರ್ಣ *t, ಸಂರಚನಾ tm *tm)
{
	int64_t s;
	time_t st;
	ಪೂರ್ಣಾಙ್ಕ f, c;

	f = sscanf(t, "%"SCNd64"%n", &s, &c);
	ಯದಿ (f < 1)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	st = s;
	ಯದಿ ((int64_t) st < s)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	ಯದಿ (!localtime_r(&st, tm))
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	ಪ್ರತ್ಯಾಗಮ t + c;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_timestamp_reference(time_t x, ಧ್ರುವ ವರ್ಣ *t, usec_t *usec)
{
	ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ {
		ಧ್ರುವ ವರ್ಣ *name;
		ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ nr;
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

	ಧ್ರುವ ವರ್ಣ *k;
	ಸಂರಚನಾ tm tm, copy;
	usec_t plus = 0, minus = 0, ret = 0;
	ಪೂರ್ಣಾಙ್ಕ r, weekday = -1;
	unsigned i;

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

	ಯದಿ (streq(t, "now"))
		ಗಚ್ಛ finish;

	ಅನ್ಯಥಾ ಯದಿ (streq(t, "today")) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		ಗಚ್ಛ finish;

	} ಅನ್ಯಥಾ ಯದಿ (streq(t, "yesterday")) {
		tm.tm_mday--;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		ಗಚ್ಛ finish;

	} ಅನ್ಯಥಾ ಯದಿ (streq(t, "tomorrow")) {
		tm.tm_mday++;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		ಗಚ್ಛ finish;

	} ಅನ್ಯಥಾ ಯದಿ (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		ಯದಿ (r < 0)
			ಪ್ರತ್ಯಾಗಮ r;

		ಗಚ್ಛ finish;
	} ಅನ್ಯಥಾ ಯದಿ (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		ಯದಿ (r < 0)
			ಪ್ರತ್ಯಾಗಮ r;

		ಗಚ್ಛ finish;
	} ಅನ್ಯಥಾ ಯದಿ (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &tm);
		ಯದಿ (k && *k == 0)
			ಗಚ್ಛ finish;
		ಅನ್ಯಥಾ ಯದಿ (k && parse_subseconds(k, &ret) == 0)
			ಗಚ್ಛ finish;

		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	} ಅನ್ಯಥಾ ಯದಿ (ul_endswith(t, " ago")) {
		ವರ್ಣ *z;

		z = strndup(t, strlen(t) - 4);
		ಯದಿ (!z)
			ಪ್ರತ್ಯಾಗಮ -ENOMEM;

		r = parse_sec(z, &minus);
		ಮುಕ್ತ(z);
		ಯದಿ (r < 0)
			ಪ್ರತ್ಯಾಗಮ r;

		ಗಚ್ಛ finish;
	}

	ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(day_nr); i++) {
		size_t skip;

		ಯದಿ (!startswith_no_case(t, day_nr[i].name))
			ಅನುವರ್ತ;

		skip = strlen(day_nr[i].name);
		ಯದಿ (t[skip] != ' ')
			ಅನುವರ್ತ;

		weekday = day_nr[i].nr;
		t += skip + 1;
		ಭಙ್ಗ;
	}

	copy = tm;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &tm);
	ಯದಿ (k && *k == 0)
		ಗಚ್ಛ finish;
	ಅನ್ಯಥಾ ಯದಿ (k && parse_subseconds(k, &ret) == 0)
		ಗಚ್ಛ finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &tm);
	ಯದಿ (k && *k == 0)
		ಗಚ್ಛ finish;
	ಅನ್ಯಥಾ ಯದಿ (k && parse_subseconds(k, &ret) == 0)
		ಗಚ್ಛ finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &tm);
	ಯದಿ (k && *k == 0)
		ಗಚ್ಛ finish;
	ಅನ್ಯಥಾ ಯದಿ (k && parse_subseconds(k, &ret) == 0)
		ಗಚ್ಛ finish;

	tm = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &tm);
	ಯದಿ (k && *k == 0) {
		tm.tm_sec = 0;
		ಗಚ್ಛ finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &tm);
	ಯದಿ (k && *k == 0) {
		tm.tm_sec = 0;
		ಗಚ್ಛ finish;
	}

	tm = copy;
	k = strptime(t, "%y-%m-%d", &tm);
	ಯದಿ (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		ಗಚ್ಛ finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d", &tm);
	ಯದಿ (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		ಗಚ್ಛ finish;
	}

	tm = copy;
	k = strptime(t, "%H:%M:%S", &tm);
	ಯದಿ (k && *k == 0)
		ಗಚ್ಛ finish;
	ಅನ್ಯಥಾ ಯದಿ (k && parse_subseconds(k, &ret) == 0)
		ಗಚ್ಛ finish;

	tm = copy;
	k = strptime(t, "%H:%M", &tm);
	ಯದಿ (k && *k == 0) {
		tm.tm_sec = 0;
		ಗಚ್ಛ finish;
	}

	tm = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &tm);
	ಯದಿ (k && *k == 0)
		ಗಚ್ಛ finish;
	ಅನ್ಯಥಾ ಯದಿ (k && parse_subseconds(k, &ret) == 0)
		ಗಚ್ಛ finish;

	ಪ್ರತ್ಯಾಗಮ -EINVAL;

 finish:
	x = mktime(&tm);
	ಯದಿ (x == (time_t)-1)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	ಯದಿ (weekday >= 0 && tm.tm_wday != weekday)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	ret += (usec_t) x * USEC_PER_SEC;

	ಯದಿ (minus > ret)
		ಪ್ರತ್ಯಾಗಮ -ERANGE;
	ಯದಿ ((ret + plus) < ret)
		ಪ್ರತ್ಯಾಗಮ -ERANGE;

	ret += plus;
	ret -= minus;

	*usec = ret;

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ ul_parse_timestamp(ಧ್ರುವ ವರ್ಣ *t, usec_t *usec)
{
	ಪ್ರತ್ಯಾಗಮ parse_timestamp_reference(time(ರಿಕ್ತ), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
ಪೂರ್ಣಾಙ್ಕ get_gmtoff(ಧ್ರುವ ಸಂರಚನಾ tm *tp)
{
	ಯದಿ (tp->tm_isdst < 0)
		ಪ್ರತ್ಯಾಗಮ 0;

#ಯದಿ HAVE_TM_GMTOFF
	ಪ್ರತ್ಯಾಗಮ tp->tm_gmtoff;
#ಅನ್ಯಥಾ
	ಸಂರಚನಾ tm tm;
	ಸಂರಚನಾ tm gtm;
	ಸಂರಚನಾ tm ltm = *tp;
	time_t lt;

	tzset();
	lt = mktime(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	ಯದಿ (lt == (time_t) -1) {
		ಯದಿ (! localtime_r(&lt, &tm)
			|| ((ltm.tm_sec ^ tm.tm_sec)
			    | (ltm.tm_min ^ tm.tm_min)
			    | (ltm.tm_hour ^ tm.tm_hour)
			    | (ltm.tm_mday ^ tm.tm_mday)
			    | (ltm.tm_mon ^ tm.tm_mon)
			    | (ltm.tm_year ^ tm.tm_year)))
			ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಯದಿ (! gmtime_r(&lt, &gtm))
		ಪ್ರತ್ಯಾಗಮ 0;

	/* Calculate the GMT offset, that is, the difference between the
	 * TP argument (ltm) and GMT (gtm).
	 *
	 * Compute intervening leap days correctly even if year is negative.
	 * Take care to avoid int overflow in leap day calculations, but it's OK
	 * to assume that A and B are close to each other.
	 */
	ಪೂರ್ಣಾಙ್ಕ a4 = (ltm.tm_year >> 2) + (1900 >> 2) - ! (ltm.tm_year & 3);
	ಪೂರ್ಣಾಙ್ಕ b4 = (gtm.tm_year >> 2) + (1900 >> 2) - ! (gtm.tm_year & 3);
	ಪೂರ್ಣಾಙ್ಕ a100 = a4 / 25 - (a4 % 25 < 0);
	ಪೂರ್ಣಾಙ್ಕ b100 = b4 / 25 - (b4 % 25 < 0);
	ಪೂರ್ಣಾಙ್ಕ a400 = a100 >> 2;
	ಪೂರ್ಣಾಙ್ಕ b400 = b100 >> 2;
	ಪೂರ್ಣಾಙ್ಕ intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

	ಪೂರ್ಣಾಙ್ಕ years = ltm.tm_year - gtm.tm_year;
	ಪೂರ್ಣಾಙ್ಕ days = (365 * years + intervening_leap_days
		    + (ltm.tm_yday - gtm.tm_yday));

	ಪ್ರತ್ಯಾಗಮ (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#endif
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ format_iso_time(ಧ್ರುವ ಸಂರಚನಾ tm *tm, uint32_t nsec, ಪೂರ್ಣಾಙ್ಕ flags, ವರ್ಣ *buf, size_t bufsz)
{
	uint32_t usec = nsec / NSEC_PER_USEC;
	ವರ್ಣ *p = buf;
	ಪೂರ್ಣಾಙ್ಕ len;

	ಯದಿ (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       tm->tm_year + (long) 1900,
			       tm->tm_mon + 1, tm->tm_mday);
		ಯದಿ (len < 0 || (size_t) len > bufsz)
			ಗಚ್ಛ err;
		bufsz -= len;
		p += len;
	}

	ಯದಿ ((flags & ISO_DATE) && (flags & ISO_TIME)) {
		ಯದಿ (bufsz < 1)
			ಗಚ್ಛ err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	ಯದಿ (flags & ISO_TIME) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
			       tm->tm_min, tm->tm_sec);
		ಯದಿ (len < 0 || (size_t) len > bufsz)
			ಗಚ್ಛ err;
		bufsz -= len;
		p += len;
	}

	ಯದಿ (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		ಯದಿ (len < 0 || (size_t) len > bufsz)
			ಗಚ್ಛ err;
		bufsz -= len;
		p += len;

	} ಅನ್ಯಥಾ ಯದಿ (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		ಯದಿ (len < 0 || (size_t) len > bufsz)
			ಗಚ್ಛ err;
		bufsz -= len;
		p += len;
	} ಅನ್ಯಥಾ ಯದಿ (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		ಯದಿ (len < 0 || (size_t) len > bufsz)
			ಗಚ್ಛ err;
		bufsz -= len;
		p += len;

	} ಅನ್ಯಥಾ ಯದಿ (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		ಯದಿ (len < 0 || (size_t) len > bufsz)
			ಗಚ್ಛ err;
		bufsz -= len;
		p += len;
	}

	ಯದಿ (flags & ISO_TIMEZONE) {
		ಪೂರ್ಣಾಙ್ಕ tmin  = get_gmtoff(tm) / 60;
		ಪೂರ್ಣಾಙ್ಕ zhour = tmin / 60;
		ಪೂರ್ಣಾಙ್ಕ zmin  = abs(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		ಯದಿ (len < 0 || (size_t) len > bufsz)
			ಗಚ್ಛ err;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	ಪ್ರತ್ಯಾಗಮ -1;
}

/* timespec to ISO 8601 */
ಪೂರ್ಣಾಙ್ಕ strtimespec_iso(ಧ್ರುವ ಸಂರಚನಾ timespec *ts, ಪೂರ್ಣಾಙ್ಕ flags, ವರ್ಣ *buf, size_t bufsz)
{
	ಸಂರಚನಾ tm tm;
	ಸಂರಚನಾ tm *rc;

	ಯದಿ (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &tm);
	ಅನ್ಯಥಾ
		rc = localtime_r(&ts->tv_sec, &tm);

	ಯದಿ (rc)
		ಪ್ರತ್ಯಾಗಮ format_iso_time(&tm, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	ಪ್ರತ್ಯಾಗಮ -1;
}

/* timeval to ISO 8601 */
ಪೂರ್ಣಾಙ್ಕ strtimeval_iso(ಧ್ರುವ ಸಂರಚನಾ timeval *tv, ಪೂರ್ಣಾಙ್ಕ flags, ವರ್ಣ *buf, size_t bufsz)
{
	ಸಂರಚನಾ timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_PER_USEC,
	};

	ಪ್ರತ್ಯಾಗಮ strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
ಪೂರ್ಣಾಙ್ಕ strtm_iso(ಧ್ರುವ ಸಂರಚನಾ tm *tm, ಪೂರ್ಣಾಙ್ಕ flags, ವರ್ಣ *buf, size_t bufsz)
{
	ಪ್ರತ್ಯಾಗಮ format_iso_time(tm, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
ಪೂರ್ಣಾಙ್ಕ strtime_iso(ಧ್ರುವ time_t *t, ಪೂರ್ಣಾಙ್ಕ flags, ವರ್ಣ *buf, size_t bufsz)
{
	ಸಂರಚನಾ tm tm;
	ಸಂರಚನಾ tm *rc;

	ಯದಿ (flags & ISO_GMTIME)
		rc = gmtime_r(t, &tm);
	ಅನ್ಯಥಾ
		rc = localtime_r(t, &tm);

	ಯದಿ (rc)
		ಪ್ರತ್ಯಾಗಮ format_iso_time(&tm, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	ಪ್ರತ್ಯಾಗಮ -1;
}

/* relative time functions */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ time_is_thisyear(ಸಂರಚನಾ tm ಧ್ರುವ *ಧ್ರುವ tm,
				   ಸಂರಚನಾ tm ಧ್ರುವ *ಧ್ರುವ tmnow)
{
	ಪ್ರತ್ಯಾಗಮ tm->tm_year == tmnow->tm_year;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ time_is_today(ಸಂರಚನಾ tm ಧ್ರುವ *ಧ್ರುವ tm,
				ಸಂರಚನಾ tm ಧ್ರುವ *ಧ್ರುವ tmnow)
{
	ಪ್ರತ್ಯಾಗಮ (tm->tm_yday == tmnow->tm_yday &&
		time_is_thisyear(tm, tmnow));
}

ಪೂರ್ಣಾಙ್ಕ strtime_short(ಧ್ರುವ time_t *t, ಸಂರಚನಾ timeval *now, ಪೂರ್ಣಾಙ್ಕ flags, ವರ್ಣ *buf, size_t bufsz)
{
	ಸಂರಚನಾ tm tm, tmnow;
	ಪೂರ್ಣಾಙ್ಕ rc = 0;

	ಯದಿ (now->tv_sec == 0)
		gettimeofday(now, ರಿಕ್ತ);

	localtime_r(t, &tm);
	localtime_r(&now->tv_sec, &tmnow);

	ಯದಿ (time_is_today(&tm, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", tm.tm_hour, tm.tm_min);
		ಯದಿ (rc < 0 || (size_t) rc > bufsz)
			ಪ್ರತ್ಯಾಗಮ -1;
		rc = 1;

	} ಅನ್ಯಥಾ ಯದಿ (time_is_thisyear(&tm, &tmnow)) {
		ಯದಿ (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = strftime(buf, bufsz, "%b%d/%H:%M", &tm);
		ಅನ್ಯಥಾ
			rc = strftime(buf, bufsz, "%b%d", &tm);
	} ಅನ್ಯಥಾ
		rc = strftime(buf, bufsz, "%Y-%b%d", &tm);

	ಪ್ರತ್ಯಾಗಮ rc <= 0 ? -1 : 0;
}

ಪೂರ್ಣಾಙ್ಕ strtimespec_relative(ಧ್ರುವ ಸಂರಚನಾ timespec *ts, ವರ್ಣ *buf, size_t bufsz)
{
	time_t secs = ts->tv_sec;
	size_t i, parts = 0;
	ಪೂರ್ಣಾಙ್ಕ rc;

	ಯದಿ (bufsz)
		buf[0] = '\0';

	ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ {
		ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ suffix;
		ಪೂರ್ಣಾಙ್ಕ width;
		int64_t secs;
	} table[] = {
		{ "y", 4, NSEC_PER_YEAR   / NSEC_PER_SEC },
		{ "d", 3, NSEC_PER_DAY    / NSEC_PER_SEC },
		{ "h", 2, NSEC_PER_HOUR   / NSEC_PER_SEC },
		{ "m", 2, NSEC_PER_MINUTE / NSEC_PER_SEC },
		{ "s", 2, NSEC_PER_SEC    / NSEC_PER_SEC },
	};

	ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(table); i++) {
		ಯದಿ (secs >= table[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? table[i].width : 0,
				      secs / table[i].secs, table[i].suffix,
				      secs % table[i].secs ? " " : "");
			ಯದಿ (rc < 0 || (size_t) rc > bufsz)
				ಗಚ್ಛ err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= table[i].secs;
		}
	}

	ಯದಿ (ts->tv_nsec) {
		ಯದಿ (ts->tv_nsec % NSEC_PER_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			ಯದಿ (rc < 0 || (size_t) rc > bufsz)
				ಗಚ್ಛ err;
		} ಅನ್ಯಥಾ {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_PER_MSEC);
			ಯದಿ (rc < 0 || (size_t) rc > bufsz)
				ಗಚ್ಛ err;
		}
	}

	ಪ್ರತ್ಯಾಗಮ 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	ಪ್ರತ್ಯಾಗಮ -1;
}

#ifndef HAVE_TIMEGM
time_t timegm(ಸಂರಚನಾ tm *tm)
{
	ಧ್ರುವ ವರ್ಣ *zone = getenv("TZ");
	time_t ret;

	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	ಯದಿ (zone)
		setenv("TZ", zone, 1);
	ಅನ್ಯಥಾ
		unsetenv("TZ");
	tzset();
	ಪ್ರತ್ಯಾಗಮ ret;
}
#endif /* HAVE_TIMEGM */

#ifdef TEST_PROGRAM_TIMEUTILS

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ run_unittest_timestamp(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ rc = EXIT_SUCCESS;
	time_t reference = 1674180427;
	ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ testcase {
		ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ ಒಳಹರಿವು;
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

	ಪ್ರತಿ (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		ಸಂರಚನಾ testcase t = testcases[i];
		usec_t result;
		ಪೂರ್ಣಾಙ್ಕ r = parse_timestamp_reference(reference, t.ಒಳಹರಿವು, &result);
		ಯದಿ (r) {
			fprintf(stderr, "Could not parse '%s'\n", t.ಒಳಹರಿವು);
			rc = EXIT_FAILURE;
		}

		ಯದಿ (result != t.expected) {
			fprintf(stderr, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.ಒಳಹರಿವು, result, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ run_unittest_format(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ rc = EXIT_SUCCESS;
	ಧ್ರುವ ಸಂರಚನಾ timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	ವರ್ಣ buf[FORMAT_TIMESTAMP_MAX];
	ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ testcase {
		ಪೂರ್ಣಾಙ್ಕ flags;
		ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ expected;
	} testcases[] = {
		{ ISO_DATE,               "2023-01-20"                       },
		{ ISO_TIME,               "02:07:07"                         },
		{ ISO_TIMEZONE,           "+00:00"                           },
		{ ISO_TIMESTAMP_T,        "2023-01-20T02:07:07+00:00"        },
		{ ISO_TIMESTAMP_COMMA_G,  "2023-01-20 02:07:07,000012+00:00" },
		{ ISO_TIME | ISO_DOTNSEC, "02:07:07.000012345" },
	};

	setenv("TZ", "GMT", 1);
	tzset();

	ಪ್ರತಿ (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		ಸಂರಚನಾ testcase t = testcases[i];
		ಪೂರ್ಣಾಙ್ಕ r = strtimespec_iso(&ts, t.flags, buf, ಪರಿಮಾಣ(buf));
		ಯದಿ (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		ಯದಿ (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ run_unittest_format_relative(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ rc = EXIT_SUCCESS;
	ವರ್ಣ buf[FORMAT_TIMESTAMP_MAX];
	ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ testcase {
		ಸಂರಚನಾ timespec ts;
		ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ expected;
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

	ಪ್ರತಿ (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		ಸಂರಚನಾ testcase t = testcases[i];
		ಪೂರ್ಣಾಙ್ಕ r = strtimespec_relative(&t.ts, buf, ಪರಿಮಾಣ(buf));
		ಯದಿ (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		ಯದಿ (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ಸಂರಚನಾ timespec ts = { 0 };
	ವರ್ಣ buf[ISO_BUFSIZ];
	ಪೂರ್ಣಾಙ್ಕ r;

	ಯದಿ (argc < 2) {
		fprintf(stderr, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	ಯದಿ (strcmp(argv[1], "--unittest-timestamp") == 0)
		ಪ್ರತ್ಯಾಗಮ run_unittest_timestamp();
	ಅನ್ಯಥಾ ಯದಿ (strcmp(argv[1], "--unittest-format") == 0)
		ಪ್ರತ್ಯಾಗಮ run_unittest_format();
	ಅನ್ಯಥಾ ಯದಿ (strcmp(argv[1], "--unittest-format-relative") == 0)
		ಪ್ರತ್ಯಾಗಮ run_unittest_format_relative();

	ಯದಿ (strcmp(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		ಯದಿ (r)
			errx(EXIT_FAILURE, "Can not parse '%s': %s", argv[2], strerror(-r));
		ts.tv_sec = (time_t) (usec / USEC_PER_SEC);
		ts.tv_nsec = (usec % USEC_PER_SEC) * NSEC_PER_USEC;
	} ಅನ್ಯಥಾ {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		ಯದಿ (argc == 3)
			ts.tv_nsec = strtos64_or_err(argv[2], "failed to parse <usec>")
				     * NSEC_PER_USEC;
	}

	strtimespec_iso(&ts, ISO_DATE, buf, ಪರಿಮಾಣ(buf));
	printf("Date: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIME, buf, ಪರಿಮಾಣ(buf));
	printf("Time: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_DATE | ISO_TIME | ISO_COMMAUSEC | ISO_T,
		       buf, ಪರಿಮಾಣ(buf));
	printf("Full: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIMESTAMP_DOT, buf, ಪರಿಮಾಣ(buf));
	printf("Zone: '%s'\n", buf);

	strtimespec_relative(&ts, buf, ಪರಿಮಾಣ(buf));
	printf("Rel:  '%s'\n", buf);

	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_TIMEUTILS */
