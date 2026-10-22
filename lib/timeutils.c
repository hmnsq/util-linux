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
#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <sys/time.h>
#অন্তর্ভাব <inttypes.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "nls.h"
#অন্তর্ভাব "strutils.h"
#অন্তর্ভাব "timeutils.h"

#define WHITESPACE " \t\n\r"

#define streq(a,b) (strcmp((a),(b)) == 0)

স্থির পূর্ণসংখ্যা parse_sec(ধ্রুবক অক্ষর *t, usec_t *usec)
{
	স্থির ধ্রুবক কাঠামো {
		ধ্রুবক অক্ষর *suffix;
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

	ধ্রুবক অক্ষর *p;
	usec_t r = 0;
	পূর্ণসংখ্যা something = FALSE;

	প্রতিজ্ঞা(t);
	প্রতিজ্ঞা(usec);

	p = t;
	জন্য (;;) {
		long long l, z = 0;
		অক্ষর *e;
		unsigned i, n = 0;

		p += strspn(p, WHITESPACE);

		যদি (*p == 0) {
			যদি (!something)
				ফেরত -EINVAL;

			বিরতি;
		}

		errno = 0;
		l = strtoll(p, &e, 10);

		যদি (errno > 0)
			ফেরত -errno;

		যদি (l < 0)
			ফেরত -ERANGE;

		যদি (*e == '.') {
			অক্ষর *b = e + 1;

			errno = 0;
			z = strtoll(b, &e, 10);
			যদি (errno > 0)
				ফেরত -errno;

			যদি (z < 0)
				ফেরত -ERANGE;

			যদি (e == b)
				ফেরত -EINVAL;

			n = e - b;

		} অন্যথা যদি (e == p)
			ফেরত -EINVAL;

		e += strspn(e, WHITESPACE);

		জন্য (i = 0; i < ARRAY_আকার(table); i++)
			যদি (ul_startswith(e, table[i].suffix)) {
				usec_t k = (usec_t) z * table[i].usec;

				জন্য (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *table[i].usec + k;
				p = e + strlen(table[i].suffix);

				something = TRUE;
				বিরতি;
			}

		যদি (i >= ARRAY_আকার(table))
			ফেরত -EINVAL;

	}

	*usec = r;

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা parse_subseconds(ধ্রুবক অক্ষর *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t factor = USEC_PER_SEC / 10;

	যদি (*t != '.' && *t != ',')
		ফেরত -1;

	যখন (*(++t)) {
		যদি (!isdigit(*t) || factor < 1)
			ফেরত -1;

		ret += ((usec_t) *t - '0') * factor;
		factor /= 10;
	}

	*usec = ret;
	ফেরত 0;
}

স্থির ধ্রুবক অক্ষর *parse_epoch_seconds(ধ্রুবক অক্ষর *t, কাঠামো tm *tm)
{
	int64_t s;
	time_t st;
	পূর্ণসংখ্যা f, c;

	f = sscanf(t, "%"SCNd64"%n", &s, &c);
	যদি (f < 1)
		ফেরত ফাঁকা;
	st = s;
	যদি ((int64_t) st < s)
		ফেরত ফাঁকা;
	যদি (!localtime_r(&st, tm))
		ফেরত ফাঁকা;
	ফেরত t + c;
}

স্থির পূর্ণসংখ্যা parse_timestamp_reference(time_t x, ধ্রুবক অক্ষর *t, usec_t *usec)
{
	স্থির ধ্রুবক কাঠামো {
		ধ্রুবক অক্ষর *name;
		ধ্রুবক পূর্ণসংখ্যা nr;
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

	ধ্রুবক অক্ষর *k;
	কাঠামো tm tm, copy;
	usec_t plus = 0, minus = 0, ret = 0;
	পূর্ণসংখ্যা r, weekday = -1;
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
		যাও finish;

	অন্যথা যদি (streq(t, "today")) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		যাও finish;

	} অন্যথা যদি (streq(t, "yesterday")) {
		tm.tm_mday--;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		যাও finish;

	} অন্যথা যদি (streq(t, "tomorrow")) {
		tm.tm_mday++;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		যাও finish;

	} অন্যথা যদি (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		যদি (r < 0)
			ফেরত r;

		যাও finish;
	} অন্যথা যদি (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		যদি (r < 0)
			ফেরত r;

		যাও finish;
	} অন্যথা যদি (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &tm);
		যদি (k && *k == 0)
			যাও finish;
		অন্যথা যদি (k && parse_subseconds(k, &ret) == 0)
			যাও finish;

		ফেরত -EINVAL;
	} অন্যথা যদি (ul_endswith(t, " ago")) {
		অক্ষর *z;

		z = strndup(t, strlen(t) - 4);
		যদি (!z)
			ফেরত -ENOMEM;

		r = parse_sec(z, &minus);
		মুক্ত(z);
		যদি (r < 0)
			ফেরত r;

		যাও finish;
	}

	জন্য (i = 0; i < ARRAY_আকার(day_nr); i++) {
		size_t skip;

		যদি (!startswith_no_case(t, day_nr[i].name))
			চালু;

		skip = strlen(day_nr[i].name);
		যদি (t[skip] != ' ')
			চালু;

		weekday = day_nr[i].nr;
		t += skip + 1;
		বিরতি;
	}

	copy = tm;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &tm);
	যদি (k && *k == 0)
		যাও finish;
	অন্যথা যদি (k && parse_subseconds(k, &ret) == 0)
		যাও finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &tm);
	যদি (k && *k == 0)
		যাও finish;
	অন্যথা যদি (k && parse_subseconds(k, &ret) == 0)
		যাও finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &tm);
	যদি (k && *k == 0)
		যাও finish;
	অন্যথা যদি (k && parse_subseconds(k, &ret) == 0)
		যাও finish;

	tm = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &tm);
	যদি (k && *k == 0) {
		tm.tm_sec = 0;
		যাও finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &tm);
	যদি (k && *k == 0) {
		tm.tm_sec = 0;
		যাও finish;
	}

	tm = copy;
	k = strptime(t, "%y-%m-%d", &tm);
	যদি (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		যাও finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d", &tm);
	যদি (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		যাও finish;
	}

	tm = copy;
	k = strptime(t, "%H:%M:%S", &tm);
	যদি (k && *k == 0)
		যাও finish;
	অন্যথা যদি (k && parse_subseconds(k, &ret) == 0)
		যাও finish;

	tm = copy;
	k = strptime(t, "%H:%M", &tm);
	যদি (k && *k == 0) {
		tm.tm_sec = 0;
		যাও finish;
	}

	tm = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &tm);
	যদি (k && *k == 0)
		যাও finish;
	অন্যথা যদি (k && parse_subseconds(k, &ret) == 0)
		যাও finish;

	ফেরত -EINVAL;

 finish:
	x = mktime(&tm);
	যদি (x == (time_t)-1)
		ফেরত -EINVAL;

	যদি (weekday >= 0 && tm.tm_wday != weekday)
		ফেরত -EINVAL;

	ret += (usec_t) x * USEC_PER_SEC;

	যদি (minus > ret)
		ফেরত -ERANGE;
	যদি ((ret + plus) < ret)
		ফেরত -ERANGE;

	ret += plus;
	ret -= minus;

	*usec = ret;

	ফেরত 0;
}

পূর্ণসংখ্যা ul_parse_timestamp(ধ্রুবক অক্ষর *t, usec_t *usec)
{
	ফেরত parse_timestamp_reference(time(ফাঁকা), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
পূর্ণসংখ্যা get_gmtoff(ধ্রুবক কাঠামো tm *tp)
{
	যদি (tp->tm_isdst < 0)
		ফেরত 0;

#যদি HAVE_TM_GMTOFF
	ফেরত tp->tm_gmtoff;
#অন্যথা
	কাঠামো tm tm;
	কাঠামো tm gtm;
	কাঠামো tm ltm = *tp;
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
			ফেরত 0;
	}

	যদি (! gmtime_r(&lt, &gtm))
		ফেরত 0;

	/* Calculate the GMT offset, that is, the difference between the
	 * TP argument (ltm) and GMT (gtm).
	 *
	 * Compute intervening leap days correctly even if year is negative.
	 * Take care to avoid int overflow in leap day calculations, but it's OK
	 * to assume that A and B are close to each other.
	 */
	পূর্ণসংখ্যা a4 = (ltm.tm_year >> 2) + (1900 >> 2) - ! (ltm.tm_year & 3);
	পূর্ণসংখ্যা b4 = (gtm.tm_year >> 2) + (1900 >> 2) - ! (gtm.tm_year & 3);
	পূর্ণসংখ্যা a100 = a4 / 25 - (a4 % 25 < 0);
	পূর্ণসংখ্যা b100 = b4 / 25 - (b4 % 25 < 0);
	পূর্ণসংখ্যা a400 = a100 >> 2;
	পূর্ণসংখ্যা b400 = b100 >> 2;
	পূর্ণসংখ্যা intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

	পূর্ণসংখ্যা years = ltm.tm_year - gtm.tm_year;
	পূর্ণসংখ্যা days = (365 * years + intervening_leap_days
		    + (ltm.tm_yday - gtm.tm_yday));

	ফেরত (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#endif
}

স্থির পূর্ণসংখ্যা format_iso_time(ধ্রুবক কাঠামো tm *tm, uint32_t nsec, পূর্ণসংখ্যা flags, অক্ষর *buf, size_t bufsz)
{
	uint32_t usec = nsec / NSEC_PER_USEC;
	অক্ষর *p = buf;
	পূর্ণসংখ্যা len;

	যদি (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       tm->tm_year + (long) 1900,
			       tm->tm_mon + 1, tm->tm_mday);
		যদি (len < 0 || (size_t) len > bufsz)
			যাও err;
		bufsz -= len;
		p += len;
	}

	যদি ((flags & ISO_DATE) && (flags & ISO_TIME)) {
		যদি (bufsz < 1)
			যাও err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	যদি (flags & ISO_TIME) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
			       tm->tm_min, tm->tm_sec);
		যদি (len < 0 || (size_t) len > bufsz)
			যাও err;
		bufsz -= len;
		p += len;
	}

	যদি (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		যদি (len < 0 || (size_t) len > bufsz)
			যাও err;
		bufsz -= len;
		p += len;

	} অন্যথা যদি (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		যদি (len < 0 || (size_t) len > bufsz)
			যাও err;
		bufsz -= len;
		p += len;
	} অন্যথা যদি (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		যদি (len < 0 || (size_t) len > bufsz)
			যাও err;
		bufsz -= len;
		p += len;

	} অন্যথা যদি (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		যদি (len < 0 || (size_t) len > bufsz)
			যাও err;
		bufsz -= len;
		p += len;
	}

	যদি (flags & ISO_TIMEZONE) {
		পূর্ণসংখ্যা tmin  = get_gmtoff(tm) / 60;
		পূর্ণসংখ্যা zhour = tmin / 60;
		পূর্ণসংখ্যা zmin  = abs(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		যদি (len < 0 || (size_t) len > bufsz)
			যাও err;
	}
	ফেরত 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	ফেরত -1;
}

/* timespec to ISO 8601 */
পূর্ণসংখ্যা strtimespec_iso(ধ্রুবক কাঠামো timespec *ts, পূর্ণসংখ্যা flags, অক্ষর *buf, size_t bufsz)
{
	কাঠামো tm tm;
	কাঠামো tm *rc;

	যদি (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &tm);
	অন্যথা
		rc = localtime_r(&ts->tv_sec, &tm);

	যদি (rc)
		ফেরত format_iso_time(&tm, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	ফেরত -1;
}

/* timeval to ISO 8601 */
পূর্ণসংখ্যা strtimeval_iso(ধ্রুবক কাঠামো timeval *tv, পূর্ণসংখ্যা flags, অক্ষর *buf, size_t bufsz)
{
	কাঠামো timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_PER_USEC,
	};

	ফেরত strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
পূর্ণসংখ্যা strtm_iso(ধ্রুবক কাঠামো tm *tm, পূর্ণসংখ্যা flags, অক্ষর *buf, size_t bufsz)
{
	ফেরত format_iso_time(tm, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
পূর্ণসংখ্যা strtime_iso(ধ্রুবক time_t *t, পূর্ণসংখ্যা flags, অক্ষর *buf, size_t bufsz)
{
	কাঠামো tm tm;
	কাঠামো tm *rc;

	যদি (flags & ISO_GMTIME)
		rc = gmtime_r(t, &tm);
	অন্যথা
		rc = localtime_r(t, &tm);

	যদি (rc)
		ফেরত format_iso_time(&tm, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	ফেরত -1;
}

/* relative time functions */
স্থির অন্তঃরেখ পূর্ণসংখ্যা time_is_thisyear(কাঠামো tm ধ্রুবক *ধ্রুবক tm,
				   কাঠামো tm ধ্রুবক *ধ্রুবক tmnow)
{
	ফেরত tm->tm_year == tmnow->tm_year;
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা time_is_today(কাঠামো tm ধ্রুবক *ধ্রুবক tm,
				কাঠামো tm ধ্রুবক *ধ্রুবক tmnow)
{
	ফেরত (tm->tm_yday == tmnow->tm_yday &&
		time_is_thisyear(tm, tmnow));
}

পূর্ণসংখ্যা strtime_short(ধ্রুবক time_t *t, কাঠামো timeval *now, পূর্ণসংখ্যা flags, অক্ষর *buf, size_t bufsz)
{
	কাঠামো tm tm, tmnow;
	পূর্ণসংখ্যা rc = 0;

	যদি (now->tv_sec == 0)
		gettimeofday(now, ফাঁকা);

	localtime_r(t, &tm);
	localtime_r(&now->tv_sec, &tmnow);

	যদি (time_is_today(&tm, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", tm.tm_hour, tm.tm_min);
		যদি (rc < 0 || (size_t) rc > bufsz)
			ফেরত -1;
		rc = 1;

	} অন্যথা যদি (time_is_thisyear(&tm, &tmnow)) {
		যদি (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = strftime(buf, bufsz, "%b%d/%H:%M", &tm);
		অন্যথা
			rc = strftime(buf, bufsz, "%b%d", &tm);
	} অন্যথা
		rc = strftime(buf, bufsz, "%Y-%b%d", &tm);

	ফেরত rc <= 0 ? -1 : 0;
}

পূর্ণসংখ্যা strtimespec_relative(ধ্রুবক কাঠামো timespec *ts, অক্ষর *buf, size_t bufsz)
{
	time_t secs = ts->tv_sec;
	size_t i, parts = 0;
	পূর্ণসংখ্যা rc;

	যদি (bufsz)
		buf[0] = '\0';

	স্থির ধ্রুবক কাঠামো {
		ধ্রুবক অক্ষর * ধ্রুবক suffix;
		পূর্ণসংখ্যা width;
		int64_t secs;
	} table[] = {
		{ "y", 4, NSEC_PER_YEAR   / NSEC_PER_SEC },
		{ "d", 3, NSEC_PER_DAY    / NSEC_PER_SEC },
		{ "h", 2, NSEC_PER_HOUR   / NSEC_PER_SEC },
		{ "m", 2, NSEC_PER_MINUTE / NSEC_PER_SEC },
		{ "s", 2, NSEC_PER_SEC    / NSEC_PER_SEC },
	};

	জন্য (i = 0; i < ARRAY_আকার(table); i++) {
		যদি (secs >= table[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? table[i].width : 0,
				      secs / table[i].secs, table[i].suffix,
				      secs % table[i].secs ? " " : "");
			যদি (rc < 0 || (size_t) rc > bufsz)
				যাও err;
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
				যাও err;
		} অন্যথা {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_PER_MSEC);
			যদি (rc < 0 || (size_t) rc > bufsz)
				যাও err;
		}
	}

	ফেরত 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	ফেরত -1;
}

#ifndef HAVE_TIMEGM
time_t timegm(কাঠামো tm *tm)
{
	ধ্রুবক অক্ষর *zone = getenv("TZ");
	time_t ret;

	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	যদি (zone)
		setenv("TZ", zone, 1);
	অন্যথা
		unsetenv("TZ");
	tzset();
	ফেরত ret;
}
#endif /* HAVE_TIMEGM */

#ifdef TEST_PROGRAM_TIMEUTILS

স্থির পূর্ণসংখ্যা run_unittest_timestamp(শূন্য)
{
	পূর্ণসংখ্যা rc = EXIT_SUCCESS;
	time_t reference = 1674180427;
	স্থির ধ্রুবক কাঠামো testcase {
		ধ্রুবক অক্ষর * ধ্রুবক প্রবেশ;
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

	জন্য (size_t i = 0; i < ARRAY_আকার(testcases); i++) {
		কাঠামো testcase t = testcases[i];
		usec_t ফলাফল;
		পূর্ণসংখ্যা r = parse_timestamp_reference(reference, t.প্রবেশ, &ফলাফল);
		যদি (r) {
			fprintf(stderr, "Could not parse '%s'\n", t.প্রবেশ);
			rc = EXIT_FAILURE;
		}

		যদি (ফলাফল != t.expected) {
			fprintf(stderr, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.প্রবেশ, ফলাফল, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ফেরত rc;
}

স্থির পূর্ণসংখ্যা run_unittest_format(শূন্য)
{
	পূর্ণসংখ্যা rc = EXIT_SUCCESS;
	ধ্রুবক কাঠামো timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	অক্ষর buf[FORMAT_TIMESTAMP_MAX];
	স্থির ধ্রুবক কাঠামো testcase {
		পূর্ণসংখ্যা flags;
		ধ্রুবক অক্ষর * ধ্রুবক expected;
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

	জন্য (size_t i = 0; i < ARRAY_আকার(testcases); i++) {
		কাঠামো testcase t = testcases[i];
		পূর্ণসংখ্যা r = strtimespec_iso(&ts, t.flags, buf, আকার(buf));
		যদি (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		যদি (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ফেরত rc;
}

স্থির পূর্ণসংখ্যা run_unittest_format_relative(শূন্য)
{
	পূর্ণসংখ্যা rc = EXIT_SUCCESS;
	অক্ষর buf[FORMAT_TIMESTAMP_MAX];
	স্থির ধ্রুবক কাঠামো testcase {
		কাঠামো timespec ts;
		ধ্রুবক অক্ষর * ধ্রুবক expected;
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

	জন্য (size_t i = 0; i < ARRAY_আকার(testcases); i++) {
		কাঠামো testcase t = testcases[i];
		পূর্ণসংখ্যা r = strtimespec_relative(&t.ts, buf, আকার(buf));
		যদি (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		যদি (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ফেরত rc;
}

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	কাঠামো timespec ts = { 0 };
	অক্ষর buf[ISO_BUFSIZ];
	পূর্ণসংখ্যা r;

	যদি (argc < 2) {
		fprintf(stderr, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	যদি (strcmp(argv[1], "--unittest-timestamp") == 0)
		ফেরত run_unittest_timestamp();
	অন্যথা যদি (strcmp(argv[1], "--unittest-format") == 0)
		ফেরত run_unittest_format();
	অন্যথা যদি (strcmp(argv[1], "--unittest-format-relative") == 0)
		ফেরত run_unittest_format_relative();

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

	strtimespec_iso(&ts, ISO_DATE, buf, আকার(buf));
	printf("Date: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIME, buf, আকার(buf));
	printf("Time: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_DATE | ISO_TIME | ISO_COMMAUSEC | ISO_T,
		       buf, আকার(buf));
	printf("Full: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIMESTAMP_DOT, buf, আকার(buf));
	printf("Zone: '%s'\n", buf);

	strtimespec_relative(&ts, buf, আকার(buf));
	printf("Rel:  '%s'\n", buf);

	ফেরত EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_TIMEUTILS */
