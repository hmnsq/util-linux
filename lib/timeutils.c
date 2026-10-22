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
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <inttypes.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "nls.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "strutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "timeutils.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHITESPACE " \t\n\r"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ streq(a,b) (strcmp((a),(b)) == 0)

ਸਥਿਰ ਪੂਰਨ parse_sec(ਅਚਲ ਅੱਖਰ *t, usec_t *usec)
{
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ {
		ਅਚਲ ਅੱਖਰ *suffix;
		usec_t usec;
	} ਸਾਰਣੀ[] = {
		{ "seconds",	USEC_ਪ੍ਰਤੀ_SEC },
		{ "second",	USEC_ਪ੍ਰਤੀ_SEC },
		{ "sec",	USEC_ਪ੍ਰਤੀ_SEC },
		{ "s",		USEC_ਪ੍ਰਤੀ_SEC },
		{ "minutes",	USEC_ਪ੍ਰਤੀ_MINUTE },
		{ "minute",	USEC_ਪ੍ਰਤੀ_MINUTE },
		{ "min",	USEC_ਪ੍ਰਤੀ_MINUTE },
		{ "months",	USEC_ਪ੍ਰਤੀ_MONTH },
		{ "month",	USEC_ਪ੍ਰਤੀ_MONTH },
		{ "msec",	USEC_ਪ੍ਰਤੀ_MSEC },
		{ "ms",		USEC_ਪ੍ਰਤੀ_MSEC },
		{ "m",		USEC_ਪ੍ਰਤੀ_MINUTE },
		{ "hours",	USEC_ਪ੍ਰਤੀ_HOUR },
		{ "hour",	USEC_ਪ੍ਰਤੀ_HOUR },
		{ "hr",		USEC_ਪ੍ਰਤੀ_HOUR },
		{ "h",		USEC_ਪ੍ਰਤੀ_HOUR },
		{ "days",	USEC_ਪ੍ਰਤੀ_DAY },
		{ "day",	USEC_ਪ੍ਰਤੀ_DAY },
		{ "d",		USEC_ਪ੍ਰਤੀ_DAY },
		{ "weeks",	USEC_ਪ੍ਰਤੀ_WEEK },
		{ "week",	USEC_ਪ੍ਰਤੀ_WEEK },
		{ "w",		USEC_ਪ੍ਰਤੀ_WEEK },
		{ "years",	USEC_ਪ੍ਰਤੀ_YEAR },
		{ "year",	USEC_ਪ੍ਰਤੀ_YEAR },
		{ "y",		USEC_ਪ੍ਰਤੀ_YEAR },
		{ "usec",	1ULL },
		{ "us",		1ULL },
		{ "",		USEC_ਪ੍ਰਤੀ_SEC },	/* default is sec */
	};

	ਅਚਲ ਅੱਖਰ *p;
	usec_t r = 0;
	ਪੂਰਨ something = FALSE;

	ਪ੍ਰਤਿਜ੍ਞਾ(t);
	ਪ੍ਰਤਿਜ੍ਞਾ(usec);

	p = t;
	ਲਈ (;;) {
		ਲੰਮਾ ਲੰਮਾ l, z = 0;
		ਅੱਖਰ *e;
		ਬਿਨਾਨਿਸ਼ਾਨੀ i, n = 0;

		p += strspn(p, WHITESPACE);

		ਜੇ (*p == 0) {
			ਜੇ (!something)
				ਵਾਪਸ -EINVAL;

			ਰੋਕੋ;
		}

		errno = 0;
		l = strtoll(p, &e, 10);

		ਜੇ (errno > 0)
			ਵਾਪਸ -errno;

		ਜੇ (l < 0)
			ਵਾਪਸ -ERANGE;

		ਜੇ (*e == '.') {
			ਅੱਖਰ *b = e + 1;

			errno = 0;
			z = strtoll(b, &e, 10);
			ਜੇ (errno > 0)
				ਵਾਪਸ -errno;

			ਜੇ (z < 0)
				ਵਾਪਸ -ERANGE;

			ਜੇ (e == b)
				ਵਾਪਸ -EINVAL;

			n = e - b;

		} ਨਹੀਂਤਾਂ ਜੇ (e == p)
			ਵਾਪਸ -EINVAL;

		e += strspn(e, WHITESPACE);

		ਲਈ (i = 0; i < ARRAY_ਮਾਪ(ਸਾਰਣੀ); i++)
			ਜੇ (ul_startswith(e, ਸਾਰਣੀ[i].suffix)) {
				usec_t k = (usec_t) z * ਸਾਰਣੀ[i].usec;

				ਲਈ (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *ਸਾਰਣੀ[i].usec + k;
				p = e + ਲੜੀ_ਲੰਬਾਈ(ਸਾਰਣੀ[i].suffix);

				something = TRUE;
				ਰੋਕੋ;
			}

		ਜੇ (i >= ARRAY_ਮਾਪ(ਸਾਰਣੀ))
			ਵਾਪਸ -EINVAL;

	}

	*usec = r;

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ parse_subseconds(ਅਚਲ ਅੱਖਰ *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t ਗੁਣਕ = USEC_ਪ੍ਰਤੀ_SEC / 10;

	ਜੇ (*t != '.' && *t != ',')
		ਵਾਪਸ -1;

	ਜਦੋਂ (*(++t)) {
		ਜੇ (!ਅੰਕ_ਹੈ(*t) || ਗੁਣਕ < 1)
			ਵਾਪਸ -1;

		ret += ((usec_t) *t - '0') * ਗੁਣਕ;
		ਗੁਣਕ /= 10;
	}

	*usec = ret;
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *parse_epoch_seconds(ਅਚਲ ਅੱਖਰ *t, ਬਣਤਰ tm *tm)
{
	int64_t s;
	time_t st;
	ਪੂਰਨ f, c;

	f = ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(t, "%"SCNd64"%n", &s, &c);
	ਜੇ (f < 1)
		ਵਾਪਸ ਨਿਰਾ;
	st = s;
	ਜੇ ((int64_t) st < s)
		ਵਾਪਸ ਨਿਰਾ;
	ਜੇ (!localtime_r(&st, tm))
		ਵਾਪਸ ਨਿਰਾ;
	ਵਾਪਸ t + c;
}

ਸਥਿਰ ਪੂਰਨ parse_timestamp_reference(time_t x, ਅਚਲ ਅੱਖਰ *t, usec_t *usec)
{
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ {
		ਅਚਲ ਅੱਖਰ *name;
		ਅਚਲ ਪੂਰਨ nr;
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

	ਅਚਲ ਅੱਖਰ *k;
	ਬਣਤਰ tm tm, copy;
	usec_t plus = 0, minus = 0, ret = 0;
	ਪੂਰਨ r, weekday = -1;
	ਬਿਨਾਨਿਸ਼ਾਨੀ i;

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

	ਪ੍ਰਤਿਜ੍ਞਾ(t);
	ਪ੍ਰਤਿਜ੍ਞਾ(usec);

	localtime_r(&x, &tm);
	tm.tm_isdst = -1;

	ਜੇ (streq(t, "now"))
		ਜਾਓ ਅੰਤ;

	ਨਹੀਂਤਾਂ ਜੇ (streq(t, "today")) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		ਜਾਓ ਅੰਤ;

	} ਨਹੀਂਤਾਂ ਜੇ (streq(t, "yesterday")) {
		tm.tm_mday--;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		ਜਾਓ ਅੰਤ;

	} ਨਹੀਂਤਾਂ ਜੇ (streq(t, "tomorrow")) {
		tm.tm_mday++;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		ਜਾਓ ਅੰਤ;

	} ਨਹੀਂਤਾਂ ਜੇ (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		ਜੇ (r < 0)
			ਵਾਪਸ r;

		ਜਾਓ ਅੰਤ;
	} ਨਹੀਂਤਾਂ ਜੇ (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		ਜੇ (r < 0)
			ਵਾਪਸ r;

		ਜਾਓ ਅੰਤ;
	} ਨਹੀਂਤਾਂ ਜੇ (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &tm);
		ਜੇ (k && *k == 0)
			ਜਾਓ ਅੰਤ;
		ਨਹੀਂਤਾਂ ਜੇ (k && parse_subseconds(k, &ret) == 0)
			ਜਾਓ ਅੰਤ;

		ਵਾਪਸ -EINVAL;
	} ਨਹੀਂਤਾਂ ਜੇ (ul_endswith(t, " ago")) {
		ਅੱਖਰ *z;

		z = strndup(t, ਲੜੀ_ਲੰਬਾਈ(t) - 4);
		ਜੇ (!z)
			ਵਾਪਸ -ENOMEM;

		r = parse_sec(z, &minus);
		ਮੁਕਤ(z);
		ਜੇ (r < 0)
			ਵਾਪਸ r;

		ਜਾਓ ਅੰਤ;
	}

	ਲਈ (i = 0; i < ARRAY_ਮਾਪ(day_nr); i++) {
		size_t skip;

		ਜੇ (!startswith_no_case(t, day_nr[i].name))
			ਜਾਰੀ;

		skip = ਲੜੀ_ਲੰਬਾਈ(day_nr[i].name);
		ਜੇ (t[skip] != ' ')
			ਜਾਰੀ;

		weekday = day_nr[i].nr;
		t += skip + 1;
		ਰੋਕੋ;
	}

	copy = tm;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &tm);
	ਜੇ (k && *k == 0)
		ਜਾਓ ਅੰਤ;
	ਨਹੀਂਤਾਂ ਜੇ (k && parse_subseconds(k, &ret) == 0)
		ਜਾਓ ਅੰਤ;

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &tm);
	ਜੇ (k && *k == 0)
		ਜਾਓ ਅੰਤ;
	ਨਹੀਂਤਾਂ ਜੇ (k && parse_subseconds(k, &ret) == 0)
		ਜਾਓ ਅੰਤ;

	tm = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &tm);
	ਜੇ (k && *k == 0)
		ਜਾਓ ਅੰਤ;
	ਨਹੀਂਤਾਂ ਜੇ (k && parse_subseconds(k, &ret) == 0)
		ਜਾਓ ਅੰਤ;

	tm = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &tm);
	ਜੇ (k && *k == 0) {
		tm.tm_sec = 0;
		ਜਾਓ ਅੰਤ;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &tm);
	ਜੇ (k && *k == 0) {
		tm.tm_sec = 0;
		ਜਾਓ ਅੰਤ;
	}

	tm = copy;
	k = strptime(t, "%y-%m-%d", &tm);
	ਜੇ (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		ਜਾਓ ਅੰਤ;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d", &tm);
	ਜੇ (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		ਜਾਓ ਅੰਤ;
	}

	tm = copy;
	k = strptime(t, "%H:%M:%S", &tm);
	ਜੇ (k && *k == 0)
		ਜਾਓ ਅੰਤ;
	ਨਹੀਂਤਾਂ ਜੇ (k && parse_subseconds(k, &ret) == 0)
		ਜਾਓ ਅੰਤ;

	tm = copy;
	k = strptime(t, "%H:%M", &tm);
	ਜੇ (k && *k == 0) {
		tm.tm_sec = 0;
		ਜਾਓ ਅੰਤ;
	}

	tm = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &tm);
	ਜੇ (k && *k == 0)
		ਜਾਓ ਅੰਤ;
	ਨਹੀਂਤਾਂ ਜੇ (k && parse_subseconds(k, &ret) == 0)
		ਜਾਓ ਅੰਤ;

	ਵਾਪਸ -EINVAL;

 ਅੰਤ:
	x = mktime(&tm);
	ਜੇ (x == (time_t)-1)
		ਵਾਪਸ -EINVAL;

	ਜੇ (weekday >= 0 && tm.tm_wday != weekday)
		ਵਾਪਸ -EINVAL;

	ret += (usec_t) x * USEC_ਪ੍ਰਤੀ_SEC;

	ਜੇ (minus > ret)
		ਵਾਪਸ -ERANGE;
	ਜੇ ((ret + plus) < ret)
		ਵਾਪਸ -ERANGE;

	ret += plus;
	ret -= minus;

	*usec = ret;

	ਵਾਪਸ 0;
}

ਪੂਰਨ ul_parse_timestamp(ਅਚਲ ਅੱਖਰ *t, usec_t *usec)
{
	ਵਾਪਸ parse_timestamp_reference(time(ਨਿਰਾ), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
ਪੂਰਨ get_gmtoff(ਅਚਲ ਬਣਤਰ tm *tp)
{
	ਜੇ (tp->tm_isdst < 0)
		ਵਾਪਸ 0;

#ਜੇ HAVE_TM_GMTOFF
	ਵਾਪਸ tp->tm_gmtoff;
#ਨਹੀਂਤਾਂ
	ਬਣਤਰ tm tm;
	ਬਣਤਰ tm gtm;
	ਬਣਤਰ tm ltm = *tp;
	time_t lt;

	tzset();
	lt = mktime(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	ਜੇ (lt == (time_t) -1) {
		ਜੇ (! localtime_r(&lt, &tm)
			|| ((ltm.tm_sec ^ tm.tm_sec)
			    | (ltm.tm_min ^ tm.tm_min)
			    | (ltm.tm_hour ^ tm.tm_hour)
			    | (ltm.tm_mday ^ tm.tm_mday)
			    | (ltm.tm_mon ^ tm.tm_mon)
			    | (ltm.tm_year ^ tm.tm_year)))
			ਵਾਪਸ 0;
	}

	ਜੇ (! gmtime_r(&lt, &gtm))
		ਵਾਪਸ 0;

	/* Calculate the GMT offset, that is, the difference between the
	 * TP argument (ltm) and GMT (gtm).
	 *
	 * Compute intervening leap days correctly even if year is negative.
	 * Take care to avoid int overflow in leap day calculations, but it's OK
	 * to assume that A and B are close to each other.
	 */
	ਪੂਰਨ a4 = (ltm.tm_year >> 2) + (1900 >> 2) - ! (ltm.tm_year & 3);
	ਪੂਰਨ b4 = (gtm.tm_year >> 2) + (1900 >> 2) - ! (gtm.tm_year & 3);
	ਪੂਰਨ a100 = a4 / 25 - (a4 % 25 < 0);
	ਪੂਰਨ b100 = b4 / 25 - (b4 % 25 < 0);
	ਪੂਰਨ a400 = a100 >> 2;
	ਪੂਰਨ b400 = b100 >> 2;
	ਪੂਰਨ intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

	ਪੂਰਨ years = ltm.tm_year - gtm.tm_year;
	ਪੂਰਨ days = (365 * years + intervening_leap_days
		    + (ltm.tm_yday - gtm.tm_yday));

	ਵਾਪਸ (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ਪੂਰਨ format_iso_ਸਮਾਂ(ਅਚਲ ਬਣਤਰ tm *tm, uint32_t nsec, ਪੂਰਨ flags, ਅੱਖਰ *buf, size_t bufsz)
{
	uint32_t usec = nsec / NSEC_ਪ੍ਰਤੀ_USEC;
	ਅੱਖਰ *p = buf;
	ਪੂਰਨ len;

	ਜੇ (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       tm->tm_year + (ਲੰਮਾ) 1900,
			       tm->tm_mon + 1, tm->tm_mday);
		ਜੇ (len < 0 || (size_t) len > bufsz)
			ਜਾਓ err;
		bufsz -= len;
		p += len;
	}

	ਜੇ ((flags & ISO_DATE) && (flags & ISO_ਸਮਾਂ)) {
		ਜੇ (bufsz < 1)
			ਜਾਓ err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	ਜੇ (flags & ISO_ਸਮਾਂ) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
			       tm->tm_min, tm->tm_sec);
		ਜੇ (len < 0 || (size_t) len > bufsz)
			ਜਾਓ err;
		bufsz -= len;
		p += len;
	}

	ਜੇ (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		ਜੇ (len < 0 || (size_t) len > bufsz)
			ਜਾਓ err;
		bufsz -= len;
		p += len;

	} ਨਹੀਂਤਾਂ ਜੇ (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		ਜੇ (len < 0 || (size_t) len > bufsz)
			ਜਾਓ err;
		bufsz -= len;
		p += len;
	} ਨਹੀਂਤਾਂ ਜੇ (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		ਜੇ (len < 0 || (size_t) len > bufsz)
			ਜਾਓ err;
		bufsz -= len;
		p += len;

	} ਨਹੀਂਤਾਂ ਜੇ (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		ਜੇ (len < 0 || (size_t) len > bufsz)
			ਜਾਓ err;
		bufsz -= len;
		p += len;
	}

	ਜੇ (flags & ISO_TIMEZONE) {
		ਪੂਰਨ tmin  = get_gmtoff(tm) / 60;
		ਪੂਰਨ zhour = tmin / 60;
		ਪੂਰਨ zmin  = ਨਿਰਪੇਖ(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		ਜੇ (len < 0 || (size_t) len > bufsz)
			ਜਾਓ err;
	}
	ਵਾਪਸ 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	ਵਾਪਸ -1;
}

/* timespec to ISO 8601 */
ਪੂਰਨ strtimespec_iso(ਅਚਲ ਬਣਤਰ timespec *ts, ਪੂਰਨ flags, ਅੱਖਰ *buf, size_t bufsz)
{
	ਬਣਤਰ tm tm;
	ਬਣਤਰ tm *rc;

	ਜੇ (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &tm);
	ਨਹੀਂਤਾਂ
		rc = localtime_r(&ts->tv_sec, &tm);

	ਜੇ (rc)
		ਵਾਪਸ format_iso_ਸਮਾਂ(&tm, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	ਵਾਪਸ -1;
}

/* timeval to ISO 8601 */
ਪੂਰਨ strtimeval_iso(ਅਚਲ ਬਣਤਰ timeval *tv, ਪੂਰਨ flags, ਅੱਖਰ *buf, size_t bufsz)
{
	ਬਣਤਰ timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_ਪ੍ਰਤੀ_USEC,
	};

	ਵਾਪਸ strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
ਪੂਰਨ strtm_iso(ਅਚਲ ਬਣਤਰ tm *tm, ਪੂਰਨ flags, ਅੱਖਰ *buf, size_t bufsz)
{
	ਵਾਪਸ format_iso_ਸਮਾਂ(tm, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
ਪੂਰਨ strtime_iso(ਅਚਲ time_t *t, ਪੂਰਨ flags, ਅੱਖਰ *buf, size_t bufsz)
{
	ਬਣਤਰ tm tm;
	ਬਣਤਰ tm *rc;

	ਜੇ (flags & ISO_GMTIME)
		rc = gmtime_r(t, &tm);
	ਨਹੀਂਤਾਂ
		rc = localtime_r(t, &tm);

	ਜੇ (rc)
		ਵਾਪਸ format_iso_ਸਮਾਂ(&tm, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	ਵਾਪਸ -1;
}

/* relative time functions */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ ਸਮਾਂ_is_thisyear(ਬਣਤਰ tm ਅਚਲ *ਅਚਲ tm,
				   ਬਣਤਰ tm ਅਚਲ *ਅਚਲ tmnow)
{
	ਵਾਪਸ tm->tm_year == tmnow->tm_year;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ ਸਮਾਂ_is_today(ਬਣਤਰ tm ਅਚਲ *ਅਚਲ tm,
				ਬਣਤਰ tm ਅਚਲ *ਅਚਲ tmnow)
{
	ਵਾਪਸ (tm->tm_yday == tmnow->tm_yday &&
		ਸਮਾਂ_is_thisyear(tm, tmnow));
}

ਪੂਰਨ strtime_short(ਅਚਲ time_t *t, ਬਣਤਰ timeval *now, ਪੂਰਨ flags, ਅੱਖਰ *buf, size_t bufsz)
{
	ਬਣਤਰ tm tm, tmnow;
	ਪੂਰਨ rc = 0;

	ਜੇ (now->tv_sec == 0)
		gettimeofday(now, ਨਿਰਾ);

	localtime_r(t, &tm);
	localtime_r(&now->tv_sec, &tmnow);

	ਜੇ (ਸਮਾਂ_is_today(&tm, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", tm.tm_hour, tm.tm_min);
		ਜੇ (rc < 0 || (size_t) rc > bufsz)
			ਵਾਪਸ -1;
		rc = 1;

	} ਨਹੀਂਤਾਂ ਜੇ (ਸਮਾਂ_is_thisyear(&tm, &tmnow)) {
		ਜੇ (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = strftime(buf, bufsz, "%b%d/%H:%M", &tm);
		ਨਹੀਂਤਾਂ
			rc = strftime(buf, bufsz, "%b%d", &tm);
	} ਨਹੀਂਤਾਂ
		rc = strftime(buf, bufsz, "%Y-%b%d", &tm);

	ਵਾਪਸ rc <= 0 ? -1 : 0;
}

ਪੂਰਨ strtimespec_relative(ਅਚਲ ਬਣਤਰ timespec *ts, ਅੱਖਰ *buf, size_t bufsz)
{
	time_t secs = ts->tv_sec;
	size_t i, parts = 0;
	ਪੂਰਨ rc;

	ਜੇ (bufsz)
		buf[0] = '\0';

	ਸਥਿਰ ਅਚਲ ਬਣਤਰ {
		ਅਚਲ ਅੱਖਰ * ਅਚਲ suffix;
		ਪੂਰਨ ਚੌੜਾਈ;
		int64_t secs;
	} ਸਾਰਣੀ[] = {
		{ "y", 4, NSEC_ਪ੍ਰਤੀ_YEAR   / NSEC_ਪ੍ਰਤੀ_SEC },
		{ "d", 3, NSEC_ਪ੍ਰਤੀ_DAY    / NSEC_ਪ੍ਰਤੀ_SEC },
		{ "h", 2, NSEC_ਪ੍ਰਤੀ_HOUR   / NSEC_ਪ੍ਰਤੀ_SEC },
		{ "m", 2, NSEC_ਪ੍ਰਤੀ_MINUTE / NSEC_ਪ੍ਰਤੀ_SEC },
		{ "s", 2, NSEC_ਪ੍ਰਤੀ_SEC    / NSEC_ਪ੍ਰਤੀ_SEC },
	};

	ਲਈ (i = 0; i < ARRAY_ਮਾਪ(ਸਾਰਣੀ); i++) {
		ਜੇ (secs >= ਸਾਰਣੀ[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? ਸਾਰਣੀ[i].ਚੌੜਾਈ : 0,
				      secs / ਸਾਰਣੀ[i].secs, ਸਾਰਣੀ[i].suffix,
				      secs % ਸਾਰਣੀ[i].secs ? " " : "");
			ਜੇ (rc < 0 || (size_t) rc > bufsz)
				ਜਾਓ err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= ਸਾਰਣੀ[i].secs;
		}
	}

	ਜੇ (ts->tv_nsec) {
		ਜੇ (ts->tv_nsec % NSEC_ਪ੍ਰਤੀ_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			ਜੇ (rc < 0 || (size_t) rc > bufsz)
				ਜਾਓ err;
		} ਨਹੀਂਤਾਂ {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_ਪ੍ਰਤੀ_MSEC);
			ਜੇ (rc < 0 || (size_t) rc > bufsz)
				ਜਾਓ err;
		}
	}

	ਵਾਪਸ 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	ਵਾਪਸ -1;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ HAVE_TIMEGM
time_t timegm(ਬਣਤਰ tm *tm)
{
	ਅਚਲ ਅੱਖਰ *zone = getenv("TZ");
	time_t ret;

	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	ਜੇ (zone)
		setenv("TZ", zone, 1);
	ਨਹੀਂਤਾਂ
		unsetenv("TZ");
	tzset();
	ਵਾਪਸ ret;
}
#ਜੇ_ਅੰਤ /* HAVE_TIMEGM */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_TIMEUTILS

ਸਥਿਰ ਪੂਰਨ ਲੜੀਵਾਰ_unittest_timestamp(ਖਾਲੀ)
{
	ਪੂਰਨ rc = EXIT_SUCCESS;
	time_t reference = 1674180427;
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ testcase {
		ਅਚਲ ਅੱਖਰ * ਅਚਲ ਨਿਵੇਸ਼;
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

	ਲਈ (size_t i = 0; i < ARRAY_ਮਾਪ(testcases); i++) {
		ਬਣਤਰ testcase t = testcases[i];
		usec_t ਨਤੀਜਾ;
		ਪੂਰਨ r = parse_timestamp_reference(reference, t.ਨਿਵੇਸ਼, &ਨਤੀਜਾ);
		ਜੇ (r) {
			fprintf(stderr, "Could not parse '%s'\n", t.ਨਿਵੇਸ਼);
			rc = EXIT_FAILURE;
		}

		ਜੇ (ਨਤੀਜਾ != t.expected) {
			fprintf(stderr, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.ਨਿਵੇਸ਼, ਨਤੀਜਾ, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ ਲੜੀਵਾਰ_unittest_format(ਖਾਲੀ)
{
	ਪੂਰਨ rc = EXIT_SUCCESS;
	ਅਚਲ ਬਣਤਰ timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	ਅੱਖਰ buf[FORMAT_TIMESTAMP_MAX];
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ testcase {
		ਪੂਰਨ flags;
		ਅਚਲ ਅੱਖਰ * ਅਚਲ expected;
	} testcases[] = {
		{ ISO_DATE,               "2023-01-20"                       },
		{ ISO_ਸਮਾਂ,               "02:07:07"                         },
		{ ISO_TIMEZONE,           "+00:00"                           },
		{ ISO_TIMESTAMP_T,        "2023-01-20T02:07:07+00:00"        },
		{ ISO_TIMESTAMP_COMMA_G,  "2023-01-20 02:07:07,000012+00:00" },
		{ ISO_ਸਮਾਂ | ISO_DOTNSEC, "02:07:07.000012345" },
	};

	setenv("TZ", "GMT", 1);
	tzset();

	ਲਈ (size_t i = 0; i < ARRAY_ਮਾਪ(testcases); i++) {
		ਬਣਤਰ testcase t = testcases[i];
		ਪੂਰਨ r = strtimespec_iso(&ts, t.flags, buf, ਆਕਾਰ(buf));
		ਜੇ (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		ਜੇ (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ ਲੜੀਵਾਰ_unittest_format_relative(ਖਾਲੀ)
{
	ਪੂਰਨ rc = EXIT_SUCCESS;
	ਅੱਖਰ buf[FORMAT_TIMESTAMP_MAX];
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ testcase {
		ਬਣਤਰ timespec ts;
		ਅਚਲ ਅੱਖਰ * ਅਚਲ expected;
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

	ਲਈ (size_t i = 0; i < ARRAY_ਮਾਪ(testcases); i++) {
		ਬਣਤਰ testcase t = testcases[i];
		ਪੂਰਨ r = strtimespec_relative(&t.ts, buf, ਆਕਾਰ(buf));
		ਜੇ (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		ਜੇ (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ਵਾਪਸ rc;
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਬਣਤਰ timespec ts = { 0 };
	ਅੱਖਰ buf[ISO_BUFSIZ];
	ਪੂਰਨ r;

	ਜੇ (argc < 2) {
		fprintf(stderr, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
	}

	ਜੇ (strcmp(argv[1], "--unittest-timestamp") == 0)
		ਵਾਪਸ ਲੜੀਵਾਰ_unittest_timestamp();
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(argv[1], "--unittest-format") == 0)
		ਵਾਪਸ ਲੜੀਵਾਰ_unittest_format();
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(argv[1], "--unittest-format-relative") == 0)
		ਵਾਪਸ ਲੜੀਵਾਰ_unittest_format_relative();

	ਜੇ (strcmp(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		ਜੇ (r)
			errx(EXIT_FAILURE, "Can not parse '%s': %s", argv[2], strerror(-r));
		ts.tv_sec = (time_t) (usec / USEC_ਪ੍ਰਤੀ_SEC);
		ts.tv_nsec = (usec % USEC_ਪ੍ਰਤੀ_SEC) * NSEC_ਪ੍ਰਤੀ_USEC;
	} ਨਹੀਂਤਾਂ {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		ਜੇ (argc == 3)
			ts.tv_nsec = strtos64_or_err(argv[2], "failed to parse <usec>")
				     * NSEC_ਪ੍ਰਤੀ_USEC;
	}

	strtimespec_iso(&ts, ISO_DATE, buf, ਆਕਾਰ(buf));
	ਲਿਖੋ("Date: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_ਸਮਾਂ, buf, ਆਕਾਰ(buf));
	ਲਿਖੋ("Time: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_DATE | ISO_ਸਮਾਂ | ISO_COMMAUSEC | ISO_T,
		       buf, ਆਕਾਰ(buf));
	ਲਿਖੋ("Full: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIMESTAMP_DOT, buf, ਆਕਾਰ(buf));
	ਲਿਖੋ("Zone: '%s'\n", buf);

	strtimespec_relative(&ts, buf, ਆਕਾਰ(buf));
	ਲਿਖੋ("Rel:  '%s'\n", buf);

	ਵਾਪਸ EXIT_SUCCESS;
}

#ਜੇ_ਅੰਤ /* TEST_PROGRAM_TIMEUTILS */
