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
#অন্তর্ভাৱ <assert.h>
#অন্তর্ভাৱ <ctype.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <time.h>
#অন্তর্ভাৱ <sys/time.h>
#অন্তর্ভাৱ <inttypes.h>

#অন্তর্ভাৱ "c.h"
#অন্তর্ভাৱ "nls.h"
#অন্তর্ভাৱ "strutils.h"
#অন্তর্ভাৱ "timeutils.h"

#define WHITESPACE " \t\n\r"

#define streq(a,b) (strcmp((a),(b)) == 0)

স্থির পূর্ণাঙ্ক parse_sec(ধ্রুৱ ৱর্ণ *t, usec_t *usec)
{
	স্থির ধ্রুৱ সংরচনা {
		ধ্রুৱ ৱর্ণ *suffix;
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

	ধ্রুৱ ৱর্ণ *p;
	usec_t r = 0;
	পূর্ণাঙ্ক something = FALSE;

	প্রতিজ্ঞা(t);
	প্রতিজ্ঞা(usec);

	p = t;
	বাবে (;;) {
		long long l, z = 0;
		ৱর্ণ *e;
		unsigned i, n = 0;

		p += strspn(p, WHITESPACE);

		যদি (*p == 0) {
			যদি (!something)
				ঘূৰাই -EINVAL;

			ভঙ্গ;
		}

		errno = 0;
		l = strtoll(p, &e, 10);

		যদি (errno > 0)
			ঘূৰাই -errno;

		যদি (l < 0)
			ঘূৰাই -ERANGE;

		যদি (*e == '.') {
			ৱর্ণ *b = e + 1;

			errno = 0;
			z = strtoll(b, &e, 10);
			যদি (errno > 0)
				ঘূৰাই -errno;

			যদি (z < 0)
				ঘূৰাই -ERANGE;

			যদি (e == b)
				ঘূৰাই -EINVAL;

			n = e - b;

		} অন্যথা যদি (e == p)
			ঘূৰাই -EINVAL;

		e += strspn(e, WHITESPACE);

		বাবে (i = 0; i < ARRAY_SIZE(table); i++)
			যদি (ul_startswith(e, table[i].suffix)) {
				usec_t k = (usec_t) z * table[i].usec;

				বাবে (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *table[i].usec + k;
				p = e + strlen(table[i].suffix);

				something = TRUE;
				ভঙ্গ;
			}

		যদি (i >= ARRAY_SIZE(table))
			ঘূৰাই -EINVAL;

	}

	*usec = r;

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক parse_subseconds(ধ্রুৱ ৱর্ণ *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t factor = USEC_PER_SEC / 10;

	যদি (*t != '.' && *t != ',')
		ঘূৰাই -1;

	যেতিয়ালৈ (*(++t)) {
		যদি (!isdigit(*t) || factor < 1)
			ঘূৰাই -1;

		ret += ((usec_t) *t - '0') * factor;
		factor /= 10;
	}

	*usec = ret;
	ঘূৰাই 0;
}

স্থির ধ্রুৱ ৱর্ণ *parse_epoch_seconds(ধ্রুৱ ৱর্ণ *t, সংরচনা tm *tm)
{
	int64_t s;
	time_t st;
	পূর্ণাঙ্ক f, c;

	f = sscanf(t, "%"SCNd64"%n", &s, &c);
	যদি (f < 1)
		ঘূৰাই রিক্ত;
	st = s;
	যদি ((int64_t) st < s)
		ঘূৰাই রিক্ত;
	যদি (!localtime_r(&st, tm))
		ঘূৰাই রিক্ত;
	ঘূৰাই t + c;
}

স্থির পূর্ণাঙ্ক parse_timestamp_reference(time_t x, ধ্রুৱ ৱর্ণ *t, usec_t *usec)
{
	স্থির ধ্রুৱ সংরচনা {
		ধ্রুৱ ৱর্ণ *name;
		ধ্রুৱ পূর্ণাঙ্ক nr;
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

	ধ্রুৱ ৱর্ণ *k;
	সংরচনা tm tm, copy;
	usec_t plus = 0, minus = 0, ret = 0;
	পূর্ণাঙ্ক r, weekday = -1;
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

	প্রতিজ্ঞা(t);
	প্রতিজ্ঞা(usec);

	localtime_r(&x, &tm);
	tm.tm_isdst = -1;

	যদি (streq(t, "now"))
		গচ্ছ finish;

	অন্যথা যদি (streq(t, "today")) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		গচ্ছ finish;

	} অন্যথা যদি (streq(t, "yesterday")) {
		tm.tm_mday--;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		গচ্ছ finish;

	} অন্যথা যদি (streq(t, "tomorrow")) {
		tm.tm_mday++;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		গচ্ছ finish;

	} অন্যথা যদি (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		যদি (r < 0)
			ঘূৰাই r;

		গচ্ছ finish;
	} অন্যথা যদি (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		যদি (r < 0)
			ঘূৰাই r;

		গচ্ছ finish;
	} অন্যথা যদি (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &tm);
		যদি (k && *k == 0)
			গচ্ছ finish;
		অন্যথা যদি (k && parse_subseconds(k, &ret) == 0)
			গচ্ছ finish;

		ঘূৰাই -EINVAL;
	} অন্যথা যদি (ul_endswith(t, " ago")) {
		ৱর্ণ *z;

		z = strndup(t, strlen(t) - 4);
		যদি (!z)
			ঘূৰাই -ENOMEM;

		r = parse_sec(z, &minus);
		মুক্ত(z);
		যদি (r < 0)
			ঘূৰাই r;

		গচ্ছ finish;
	}

	বাবে (i = 0; i < ARRAY_SIZE(day_nr); i++) {
		size_t skip;

		যদি (!startswith_no_case(t, day_nr[i].name))
			অনুৱর্ত;

		skip = strlen(day_nr[i].name);
		যদি (t[skip] != ' ')
			অনুৱর্ত;

		weekday = day_nr[i].nr;
		t += skip + 1;
		ভঙ্গ;
	}

	copy = tm;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &tm);
	যদি (k && *k == 0)
		গচ্ছ finish;
	অন্যথা যদি (k && parse_subseconds(k, &ret) == 0)
		গচ্ছ finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &tm);
	যদি (k && *k == 0)
		গচ্ছ finish;
	অন্যথা যদি (k && parse_subseconds(k, &ret) == 0)
		গচ্ছ finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &tm);
	যদি (k && *k == 0)
		গচ্ছ finish;
	অন্যথা যদি (k && parse_subseconds(k, &ret) == 0)
		গচ্ছ finish;

	tm = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &tm);
	যদি (k && *k == 0) {
		tm.tm_sec = 0;
		গচ্ছ finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &tm);
	যদি (k && *k == 0) {
		tm.tm_sec = 0;
		গচ্ছ finish;
	}

	tm = copy;
	k = strptime(t, "%y-%m-%d", &tm);
	যদি (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		গচ্ছ finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d", &tm);
	যদি (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		গচ্ছ finish;
	}

	tm = copy;
	k = strptime(t, "%H:%M:%S", &tm);
	যদি (k && *k == 0)
		গচ্ছ finish;
	অন্যথা যদি (k && parse_subseconds(k, &ret) == 0)
		গচ্ছ finish;

	tm = copy;
	k = strptime(t, "%H:%M", &tm);
	যদি (k && *k == 0) {
		tm.tm_sec = 0;
		গচ্ছ finish;
	}

	tm = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &tm);
	যদি (k && *k == 0)
		গচ্ছ finish;
	অন্যথা যদি (k && parse_subseconds(k, &ret) == 0)
		গচ্ছ finish;

	ঘূৰাই -EINVAL;

 finish:
	x = mktime(&tm);
	যদি (x == (time_t)-1)
		ঘূৰাই -EINVAL;

	যদি (weekday >= 0 && tm.tm_wday != weekday)
		ঘূৰাই -EINVAL;

	ret += (usec_t) x * USEC_PER_SEC;

	যদি (minus > ret)
		ঘূৰাই -ERANGE;
	যদি ((ret + plus) < ret)
		ঘূৰাই -ERANGE;

	ret += plus;
	ret -= minus;

	*usec = ret;

	ঘূৰাই 0;
}

পূর্ণাঙ্ক ul_parse_timestamp(ধ্রুৱ ৱর্ণ *t, usec_t *usec)
{
	ঘূৰাই parse_timestamp_reference(time(রিক্ত), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
পূর্ণাঙ্ক get_gmtoff(ধ্রুৱ সংরচনা tm *tp)
{
	যদি (tp->tm_isdst < 0)
		ঘূৰাই 0;

#যদি HAVE_TM_GMTOFF
	ঘূৰাই tp->tm_gmtoff;
#অন্যথা
	সংরচনা tm tm;
	সংরচনা tm gtm;
	সংরচনা tm ltm = *tp;
	time_t lt;

	tzset();
	lt = mktime(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	যদি (lt == (time_t) -1) {
		যদি (! localtime_r(&lt, &tm)
			|| ((ltm.tm_sec ^ tm.tm_sec)
			    | (ltm.tm_min ^ tm.tm_min)
			    | (ltm.tm_hour ^ tm.tm_hour)
			    | (ltm.tm_mday ^ tm.tm_mday)
			    | (ltm.tm_mon ^ tm.tm_mon)
			    | (ltm.tm_year ^ tm.tm_year)))
			ঘূৰাই 0;
	}

	যদি (! gmtime_r(&lt, &gtm))
		ঘূৰাই 0;

	/* Calculate the GMT offset, that is, the difference between the
	 * TP argument (ltm) and GMT (gtm).
	 *
	 * Compute intervening leap days correctly even if year is negative.
	 * Take care to avoid int overflow in leap day calculations, but it's OK
	 * to assume that A and B are close to each other.
	 */
	পূর্ণাঙ্ক a4 = (ltm.tm_year >> 2) + (1900 >> 2) - ! (ltm.tm_year & 3);
	পূর্ণাঙ্ক b4 = (gtm.tm_year >> 2) + (1900 >> 2) - ! (gtm.tm_year & 3);
	পূর্ণাঙ্ক a100 = a4 / 25 - (a4 % 25 < 0);
	পূর্ণাঙ্ক b100 = b4 / 25 - (b4 % 25 < 0);
	পূর্ণাঙ্ক a400 = a100 >> 2;
	পূর্ণাঙ্ক b400 = b100 >> 2;
	পূর্ণাঙ্ক intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

	পূর্ণাঙ্ক years = ltm.tm_year - gtm.tm_year;
	পূর্ণাঙ্ক days = (365 * years + intervening_leap_days
		    + (ltm.tm_yday - gtm.tm_yday));

	ঘূৰাই (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#endif
}

স্থির পূর্ণাঙ্ক format_iso_time(ধ্রুৱ সংরচনা tm *tm, uint32_t nsec, পূর্ণাঙ্ক flags, ৱর্ণ *buf, size_t bufsz)
{
	uint32_t usec = nsec / NSEC_PER_USEC;
	ৱর্ণ *p = buf;
	পূর্ণাঙ্ক len;

	যদি (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       tm->tm_year + (long) 1900,
			       tm->tm_mon + 1, tm->tm_mday);
		যদি (len < 0 || (size_t) len > bufsz)
			গচ্ছ err;
		bufsz -= len;
		p += len;
	}

	যদি ((flags & ISO_DATE) && (flags & ISO_TIME)) {
		যদি (bufsz < 1)
			গচ্ছ err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	যদি (flags & ISO_TIME) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
			       tm->tm_min, tm->tm_sec);
		যদি (len < 0 || (size_t) len > bufsz)
			গচ্ছ err;
		bufsz -= len;
		p += len;
	}

	যদি (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		যদি (len < 0 || (size_t) len > bufsz)
			গচ্ছ err;
		bufsz -= len;
		p += len;

	} অন্যথা যদি (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		যদি (len < 0 || (size_t) len > bufsz)
			গচ্ছ err;
		bufsz -= len;
		p += len;
	} অন্যথা যদি (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		যদি (len < 0 || (size_t) len > bufsz)
			গচ্ছ err;
		bufsz -= len;
		p += len;

	} অন্যথা যদি (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		যদি (len < 0 || (size_t) len > bufsz)
			গচ্ছ err;
		bufsz -= len;
		p += len;
	}

	যদি (flags & ISO_TIMEZONE) {
		পূর্ণাঙ্ক tmin  = get_gmtoff(tm) / 60;
		পূর্ণাঙ্ক zhour = tmin / 60;
		পূর্ণাঙ্ক zmin  = abs(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		যদি (len < 0 || (size_t) len > bufsz)
			গচ্ছ err;
	}
	ঘূৰাই 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	ঘূৰাই -1;
}

/* timespec to ISO 8601 */
পূর্ণাঙ্ক strtimespec_iso(ধ্রুৱ সংরচনা timespec *ts, পূর্ণাঙ্ক flags, ৱর্ণ *buf, size_t bufsz)
{
	সংরচনা tm tm;
	সংরচনা tm *rc;

	যদি (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &tm);
	অন্যথা
		rc = localtime_r(&ts->tv_sec, &tm);

	যদি (rc)
		ঘূৰাই format_iso_time(&tm, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	ঘূৰাই -1;
}

/* timeval to ISO 8601 */
পূর্ণাঙ্ক strtimeval_iso(ধ্রুৱ সংরচনা timeval *tv, পূর্ণাঙ্ক flags, ৱর্ণ *buf, size_t bufsz)
{
	সংরচনা timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_PER_USEC,
	};

	ঘূৰাই strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
পূর্ণাঙ্ক strtm_iso(ধ্রুৱ সংরচনা tm *tm, পূর্ণাঙ্ক flags, ৱর্ণ *buf, size_t bufsz)
{
	ঘূৰাই format_iso_time(tm, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
পূর্ণাঙ্ক strtime_iso(ধ্রুৱ time_t *t, পূর্ণাঙ্ক flags, ৱর্ণ *buf, size_t bufsz)
{
	সংরচনা tm tm;
	সংরচনা tm *rc;

	যদি (flags & ISO_GMTIME)
		rc = gmtime_r(t, &tm);
	অন্যথা
		rc = localtime_r(t, &tm);

	যদি (rc)
		ঘূৰাই format_iso_time(&tm, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	ঘূৰাই -1;
}

/* relative time functions */
স্থির অন্তঃস্থ পূর্ণাঙ্ক time_is_thisyear(সংরচনা tm ধ্রুৱ *ধ্রুৱ tm,
				   সংরচনা tm ধ্রুৱ *ধ্রুৱ tmnow)
{
	ঘূৰাই tm->tm_year == tmnow->tm_year;
}

স্থির অন্তঃস্থ পূর্ণাঙ্ক time_is_today(সংরচনা tm ধ্রুৱ *ধ্রুৱ tm,
				সংরচনা tm ধ্রুৱ *ধ্রুৱ tmnow)
{
	ঘূৰাই (tm->tm_yday == tmnow->tm_yday &&
		time_is_thisyear(tm, tmnow));
}

পূর্ণাঙ্ক strtime_short(ধ্রুৱ time_t *t, সংরচনা timeval *now, পূর্ণাঙ্ক flags, ৱর্ণ *buf, size_t bufsz)
{
	সংরচনা tm tm, tmnow;
	পূর্ণাঙ্ক rc = 0;

	যদি (now->tv_sec == 0)
		gettimeofday(now, রিক্ত);

	localtime_r(t, &tm);
	localtime_r(&now->tv_sec, &tmnow);

	যদি (time_is_today(&tm, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", tm.tm_hour, tm.tm_min);
		যদি (rc < 0 || (size_t) rc > bufsz)
			ঘূৰাই -1;
		rc = 1;

	} অন্যথা যদি (time_is_thisyear(&tm, &tmnow)) {
		যদি (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = strftime(buf, bufsz, "%b%d/%H:%M", &tm);
		অন্যথা
			rc = strftime(buf, bufsz, "%b%d", &tm);
	} অন্যথা
		rc = strftime(buf, bufsz, "%Y-%b%d", &tm);

	ঘূৰাই rc <= 0 ? -1 : 0;
}

পূর্ণাঙ্ক strtimespec_relative(ধ্রুৱ সংরচনা timespec *ts, ৱর্ণ *buf, size_t bufsz)
{
	time_t secs = ts->tv_sec;
	size_t i, parts = 0;
	পূর্ণাঙ্ক rc;

	যদি (bufsz)
		buf[0] = '\0';

	স্থির ধ্রুৱ সংরচনা {
		ধ্রুৱ ৱর্ণ * ধ্রুৱ suffix;
		পূর্ণাঙ্ক width;
		int64_t secs;
	} table[] = {
		{ "y", 4, NSEC_PER_YEAR   / NSEC_PER_SEC },
		{ "d", 3, NSEC_PER_DAY    / NSEC_PER_SEC },
		{ "h", 2, NSEC_PER_HOUR   / NSEC_PER_SEC },
		{ "m", 2, NSEC_PER_MINUTE / NSEC_PER_SEC },
		{ "s", 2, NSEC_PER_SEC    / NSEC_PER_SEC },
	};

	বাবে (i = 0; i < ARRAY_SIZE(table); i++) {
		যদি (secs >= table[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? table[i].width : 0,
				      secs / table[i].secs, table[i].suffix,
				      secs % table[i].secs ? " " : "");
			যদি (rc < 0 || (size_t) rc > bufsz)
				গচ্ছ err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= table[i].secs;
		}
	}

	যদি (ts->tv_nsec) {
		যদি (ts->tv_nsec % NSEC_PER_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			যদি (rc < 0 || (size_t) rc > bufsz)
				গচ্ছ err;
		} অন্যথা {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_PER_MSEC);
			যদি (rc < 0 || (size_t) rc > bufsz)
				গচ্ছ err;
		}
	}

	ঘূৰাই 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	ঘূৰাই -1;
}

#ifndef HAVE_TIMEGM
time_t timegm(সংরচনা tm *tm)
{
	ধ্রুৱ ৱর্ণ *zone = getenv("TZ");
	time_t ret;

	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	যদি (zone)
		setenv("TZ", zone, 1);
	অন্যথা
		unsetenv("TZ");
	tzset();
	ঘূৰাই ret;
}
#endif /* HAVE_TIMEGM */

#ifdef TEST_PROGRAM_TIMEUTILS

স্থির পূর্ণাঙ্ক run_unittest_timestamp(শূন্য)
{
	পূর্ণাঙ্ক rc = EXIT_SUCCESS;
	time_t reference = 1674180427;
	স্থির ধ্রুৱ সংরচনা testcase {
		ধ্রুৱ ৱর্ণ * ধ্রুৱ প্ৰবেশ;
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

	বাবে (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		সংরচনা testcase t = testcases[i];
		usec_t result;
		পূর্ণাঙ্ক r = parse_timestamp_reference(reference, t.প্ৰবেশ, &result);
		যদি (r) {
			fprintf(stderr, "Could not parse '%s'\n", t.প্ৰবেশ);
			rc = EXIT_FAILURE;
		}

		যদি (result != t.expected) {
			fprintf(stderr, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.প্ৰবেশ, result, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ঘূৰাই rc;
}

স্থির পূর্ণাঙ্ক run_unittest_format(শূন্য)
{
	পূর্ণাঙ্ক rc = EXIT_SUCCESS;
	ধ্রুৱ সংরচনা timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	ৱর্ণ buf[FORMAT_TIMESTAMP_MAX];
	স্থির ধ্রুৱ সংরচনা testcase {
		পূর্ণাঙ্ক flags;
		ধ্রুৱ ৱর্ণ * ধ্রুৱ expected;
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

	বাবে (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		সংরচনা testcase t = testcases[i];
		পূর্ণাঙ্ক r = strtimespec_iso(&ts, t.flags, buf, পরিমাণ(buf));
		যদি (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		যদি (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ঘূৰাই rc;
}

স্থির পূর্ণাঙ্ক run_unittest_format_relative(শূন্য)
{
	পূর্ণাঙ্ক rc = EXIT_SUCCESS;
	ৱর্ণ buf[FORMAT_TIMESTAMP_MAX];
	স্থির ধ্রুৱ সংরচনা testcase {
		সংরচনা timespec ts;
		ধ্রুৱ ৱর্ণ * ধ্রুৱ expected;
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

	বাবে (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		সংরচনা testcase t = testcases[i];
		পূর্ণাঙ্ক r = strtimespec_relative(&t.ts, buf, পরিমাণ(buf));
		যদি (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		যদি (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ঘূৰাই rc;
}

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ *argv[])
{
	সংরচনা timespec ts = { 0 };
	ৱর্ণ buf[ISO_BUFSIZ];
	পূর্ণাঙ্ক r;

	যদি (argc < 2) {
		fprintf(stderr, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	যদি (strcmp(argv[1], "--unittest-timestamp") == 0)
		ঘূৰাই run_unittest_timestamp();
	অন্যথা যদি (strcmp(argv[1], "--unittest-format") == 0)
		ঘূৰাই run_unittest_format();
	অন্যথা যদি (strcmp(argv[1], "--unittest-format-relative") == 0)
		ঘূৰাই run_unittest_format_relative();

	যদি (strcmp(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		যদি (r)
			errx(EXIT_FAILURE, "Can not parse '%s': %s", argv[2], strerror(-r));
		ts.tv_sec = (time_t) (usec / USEC_PER_SEC);
		ts.tv_nsec = (usec % USEC_PER_SEC) * NSEC_PER_USEC;
	} অন্যথা {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		যদি (argc == 3)
			ts.tv_nsec = strtos64_or_err(argv[2], "failed to parse <usec>")
				     * NSEC_PER_USEC;
	}

	strtimespec_iso(&ts, ISO_DATE, buf, পরিমাণ(buf));
	printf("Date: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIME, buf, পরিমাণ(buf));
	printf("Time: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_DATE | ISO_TIME | ISO_COMMAUSEC | ISO_T,
		       buf, পরিমাণ(buf));
	printf("Full: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIMESTAMP_DOT, buf, পরিমাণ(buf));
	printf("Zone: '%s'\n", buf);

	strtimespec_relative(&ts, buf, পরিমাণ(buf));
	printf("Rel:  '%s'\n", buf);

	ঘূৰাই EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_TIMEUTILS */
