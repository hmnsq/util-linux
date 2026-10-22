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
#شامل <assert.h>
#شامل <ctype.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <time.h>
#شامل <sys/time.h>
#شامل <inttypes.h>

#شامل "c.h"
#شامل "nls.h"
#شامل "strutils.h"
#شامل "timeutils.h"

#تعریف_کرو WHITESPACE " \t\n\r"

#تعریف_کرو streq(a,b) (strcmp((a),(b)) == 0)

ساکن صحیح parse_sec(ثابت حرف *t, usec_t *usec)
{
	ساکن ثابت ساخت {
		ثابت حرف *suffix;
		usec_t usec;
	} جدول[] = {
		{ "seconds",	USEC_فی_SEC },
		{ "second",	USEC_فی_SEC },
		{ "sec",	USEC_فی_SEC },
		{ "s",		USEC_فی_SEC },
		{ "minutes",	USEC_فی_MINUTE },
		{ "minute",	USEC_فی_MINUTE },
		{ "min",	USEC_فی_MINUTE },
		{ "months",	USEC_فی_MONTH },
		{ "month",	USEC_فی_MONTH },
		{ "msec",	USEC_فی_MSEC },
		{ "ms",		USEC_فی_MSEC },
		{ "m",		USEC_فی_MINUTE },
		{ "hours",	USEC_فی_HOUR },
		{ "hour",	USEC_فی_HOUR },
		{ "hr",		USEC_فی_HOUR },
		{ "h",		USEC_فی_HOUR },
		{ "days",	USEC_فی_DAY },
		{ "day",	USEC_فی_DAY },
		{ "d",		USEC_فی_DAY },
		{ "weeks",	USEC_فی_WEEK },
		{ "week",	USEC_فی_WEEK },
		{ "w",		USEC_فی_WEEK },
		{ "years",	USEC_فی_YEAR },
		{ "year",	USEC_فی_YEAR },
		{ "y",		USEC_فی_YEAR },
		{ "usec",	1ULL },
		{ "us",		1ULL },
		{ "",		USEC_فی_SEC },	/* default is sec */
	};

	ثابت حرف *p;
	usec_t r = 0;
	صحیح something = FALSE;

	أكد(t);
	أكد(usec);

	p = t;
	برائے (;;) {
		طویل طویل l, z = 0;
		حرف *e;
		غیرعلامتی i, n = 0;

		p += strspn(p, WHITESPACE);

		اگر (*p == 0) {
			اگر (!something)
				واپس -EINVAL;

			توڑو;
		}

		errno = 0;
		l = strtoll(p, &e, 10);

		اگر (errno > 0)
			واپس -errno;

		اگر (l < 0)
			واپس -ERANGE;

		اگر (*e == '.') {
			حرف *b = e + 1;

			errno = 0;
			z = strtoll(b, &e, 10);
			اگر (errno > 0)
				واپس -errno;

			اگر (z < 0)
				واپس -ERANGE;

			اگر (e == b)
				واپس -EINVAL;

			n = e - b;

		} ورنہ اگر (e == p)
			واپس -EINVAL;

		e += strspn(e, WHITESPACE);

		برائے (i = 0; i < ARRAY_سائز(جدول); i++)
			اگر (ul_startswith(e, جدول[i].suffix)) {
				usec_t k = (usec_t) z * جدول[i].usec;

				برائے (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *جدول[i].usec + k;
				p = e + لڑی_لمبائی(جدول[i].suffix);

				something = TRUE;
				توڑو;
			}

		اگر (i >= ARRAY_سائز(جدول))
			واپس -EINVAL;

	}

	*usec = r;

	واپس 0;
}

ساکن صحیح parse_subseconds(ثابت حرف *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t ضربی_عدد = USEC_فی_SEC / 10;

	اگر (*t != '.' && *t != ',')
		واپس -1;

	جب (*(++t)) {
		اگر (!ہندسہ_ہے(*t) || ضربی_عدد < 1)
			واپس -1;

		ret += ((usec_t) *t - '0') * ضربی_عدد;
		ضربی_عدد /= 10;
	}

	*usec = ret;
	واپس 0;
}

ساکن ثابت حرف *parse_epoch_seconds(ثابت حرف *t, ساخت tm *tm)
{
	int64_t s;
	time_t st;
	صحیح f, c;

	f = لڑی_سے_پڑھو(t, "%"SCNd64"%n", &s, &c);
	اگر (f < 1)
		واپس تہی;
	st = s;
	اگر ((int64_t) st < s)
		واپس تہی;
	اگر (!localtime_r(&st, tm))
		واپس تہی;
	واپس t + c;
}

ساکن صحیح parse_timestamp_reference(time_t x, ثابت حرف *t, usec_t *usec)
{
	ساکن ثابت ساخت {
		ثابت حرف *name;
		ثابت صحیح nr;
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

	ثابت حرف *k;
	ساخت tm tm, copy;
	usec_t plus = 0, minus = 0, ret = 0;
	صحیح r, weekday = -1;
	غیرعلامتی i;

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

	أكد(t);
	أكد(usec);

	localtime_r(&x, &tm);
	tm.tm_isdst = -1;

	اگر (streq(t, "now"))
		جاؤ اختتام;

	ورنہ اگر (streq(t, "today")) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		جاؤ اختتام;

	} ورنہ اگر (streq(t, "yesterday")) {
		tm.tm_mday--;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		جاؤ اختتام;

	} ورنہ اگر (streq(t, "tomorrow")) {
		tm.tm_mday++;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		جاؤ اختتام;

	} ورنہ اگر (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		اگر (r < 0)
			واپس r;

		جاؤ اختتام;
	} ورنہ اگر (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		اگر (r < 0)
			واپس r;

		جاؤ اختتام;
	} ورنہ اگر (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &tm);
		اگر (k && *k == 0)
			جاؤ اختتام;
		ورنہ اگر (k && parse_subseconds(k, &ret) == 0)
			جاؤ اختتام;

		واپس -EINVAL;
	} ورنہ اگر (ul_endswith(t, " ago")) {
		حرف *z;

		z = strndup(t, لڑی_لمبائی(t) - 4);
		اگر (!z)
			واپس -ENOMEM;

		r = parse_sec(z, &minus);
		آزاد(z);
		اگر (r < 0)
			واپس r;

		جاؤ اختتام;
	}

	برائے (i = 0; i < ARRAY_سائز(day_nr); i++) {
		size_t skip;

		اگر (!startswith_no_case(t, day_nr[i].name))
			جاری;

		skip = لڑی_لمبائی(day_nr[i].name);
		اگر (t[skip] != ' ')
			جاری;

		weekday = day_nr[i].nr;
		t += skip + 1;
		توڑو;
	}

	copy = tm;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &tm);
	اگر (k && *k == 0)
		جاؤ اختتام;
	ورنہ اگر (k && parse_subseconds(k, &ret) == 0)
		جاؤ اختتام;

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &tm);
	اگر (k && *k == 0)
		جاؤ اختتام;
	ورنہ اگر (k && parse_subseconds(k, &ret) == 0)
		جاؤ اختتام;

	tm = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &tm);
	اگر (k && *k == 0)
		جاؤ اختتام;
	ورنہ اگر (k && parse_subseconds(k, &ret) == 0)
		جاؤ اختتام;

	tm = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &tm);
	اگر (k && *k == 0) {
		tm.tm_sec = 0;
		جاؤ اختتام;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &tm);
	اگر (k && *k == 0) {
		tm.tm_sec = 0;
		جاؤ اختتام;
	}

	tm = copy;
	k = strptime(t, "%y-%m-%d", &tm);
	اگر (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		جاؤ اختتام;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d", &tm);
	اگر (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		جاؤ اختتام;
	}

	tm = copy;
	k = strptime(t, "%H:%M:%S", &tm);
	اگر (k && *k == 0)
		جاؤ اختتام;
	ورنہ اگر (k && parse_subseconds(k, &ret) == 0)
		جاؤ اختتام;

	tm = copy;
	k = strptime(t, "%H:%M", &tm);
	اگر (k && *k == 0) {
		tm.tm_sec = 0;
		جاؤ اختتام;
	}

	tm = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &tm);
	اگر (k && *k == 0)
		جاؤ اختتام;
	ورنہ اگر (k && parse_subseconds(k, &ret) == 0)
		جاؤ اختتام;

	واپس -EINVAL;

 اختتام:
	x = mktime(&tm);
	اگر (x == (time_t)-1)
		واپس -EINVAL;

	اگر (weekday >= 0 && tm.tm_wday != weekday)
		واپس -EINVAL;

	ret += (usec_t) x * USEC_فی_SEC;

	اگر (minus > ret)
		واپس -ERANGE;
	اگر ((ret + plus) < ret)
		واپس -ERANGE;

	ret += plus;
	ret -= minus;

	*usec = ret;

	واپس 0;
}

صحیح ul_parse_timestamp(ثابت حرف *t, usec_t *usec)
{
	واپس parse_timestamp_reference(time(تہی), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
صحیح get_gmtoff(ثابت ساخت tm *tp)
{
	اگر (tp->tm_isdst < 0)
		واپس 0;

#اگر HAVE_TM_GMTOFF
	واپس tp->tm_gmtoff;
#ورنہ
	ساخت tm tm;
	ساخت tm gtm;
	ساخت tm ltm = *tp;
	time_t lt;

	tzset();
	lt = mktime(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	اگر (lt == (time_t) -1) {
		اگر (! localtime_r(&lt, &tm)
			|| ((ltm.tm_sec ^ tm.tm_sec)
			    | (ltm.tm_min ^ tm.tm_min)
			    | (ltm.tm_hour ^ tm.tm_hour)
			    | (ltm.tm_mday ^ tm.tm_mday)
			    | (ltm.tm_mon ^ tm.tm_mon)
			    | (ltm.tm_year ^ tm.tm_year)))
			واپس 0;
	}

	اگر (! gmtime_r(&lt, &gtm))
		واپس 0;

	/* Calculate the GMT offset, that is, the difference between the
	 * TP argument (ltm) and GMT (gtm).
	 *
	 * Compute intervening leap days correctly even if year is negative.
	 * Take care to avoid int overflow in leap day calculations, but it's OK
	 * to assume that A and B are close to each other.
	 */
	صحیح a4 = (ltm.tm_year >> 2) + (1900 >> 2) - ! (ltm.tm_year & 3);
	صحیح b4 = (gtm.tm_year >> 2) + (1900 >> 2) - ! (gtm.tm_year & 3);
	صحیح a100 = a4 / 25 - (a4 % 25 < 0);
	صحیح b100 = b4 / 25 - (b4 % 25 < 0);
	صحیح a400 = a100 >> 2;
	صحیح b400 = b100 >> 2;
	صحیح intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

	صحیح years = ltm.tm_year - gtm.tm_year;
	صحیح days = (365 * years + intervening_leap_days
		    + (ltm.tm_yday - gtm.tm_yday));

	واپس (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#اگر_ختم
}

ساکن صحیح format_iso_وقت(ثابت ساخت tm *tm, uint32_t nsec, صحیح flags, حرف *buf, size_t bufsz)
{
	uint32_t usec = nsec / NSEC_فی_USEC;
	حرف *p = buf;
	صحیح len;

	اگر (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       tm->tm_year + (طویل) 1900,
			       tm->tm_mon + 1, tm->tm_mday);
		اگر (len < 0 || (size_t) len > bufsz)
			جاؤ err;
		bufsz -= len;
		p += len;
	}

	اگر ((flags & ISO_DATE) && (flags & ISO_وقت)) {
		اگر (bufsz < 1)
			جاؤ err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	اگر (flags & ISO_وقت) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
			       tm->tm_min, tm->tm_sec);
		اگر (len < 0 || (size_t) len > bufsz)
			جاؤ err;
		bufsz -= len;
		p += len;
	}

	اگر (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		اگر (len < 0 || (size_t) len > bufsz)
			جاؤ err;
		bufsz -= len;
		p += len;

	} ورنہ اگر (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		اگر (len < 0 || (size_t) len > bufsz)
			جاؤ err;
		bufsz -= len;
		p += len;
	} ورنہ اگر (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		اگر (len < 0 || (size_t) len > bufsz)
			جاؤ err;
		bufsz -= len;
		p += len;

	} ورنہ اگر (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		اگر (len < 0 || (size_t) len > bufsz)
			جاؤ err;
		bufsz -= len;
		p += len;
	}

	اگر (flags & ISO_TIMEZONE) {
		صحیح tmin  = get_gmtoff(tm) / 60;
		صحیح zhour = tmin / 60;
		صحیح zmin  = مطلق_قدر(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		اگر (len < 0 || (size_t) len > bufsz)
			جاؤ err;
	}
	واپس 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	واپس -1;
}

/* timespec to ISO 8601 */
صحیح strtimespec_iso(ثابت ساخت timespec *ts, صحیح flags, حرف *buf, size_t bufsz)
{
	ساخت tm tm;
	ساخت tm *rc;

	اگر (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &tm);
	ورنہ
		rc = localtime_r(&ts->tv_sec, &tm);

	اگر (rc)
		واپس format_iso_وقت(&tm, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	واپس -1;
}

/* timeval to ISO 8601 */
صحیح strtimeval_iso(ثابت ساخت timeval *tv, صحیح flags, حرف *buf, size_t bufsz)
{
	ساخت timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_فی_USEC,
	};

	واپس strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
صحیح strtm_iso(ثابت ساخت tm *tm, صحیح flags, حرف *buf, size_t bufsz)
{
	واپس format_iso_وقت(tm, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
صحیح strtime_iso(ثابت time_t *t, صحیح flags, حرف *buf, size_t bufsz)
{
	ساخت tm tm;
	ساخت tm *rc;

	اگر (flags & ISO_GMTIME)
		rc = gmtime_r(t, &tm);
	ورنہ
		rc = localtime_r(t, &tm);

	اگر (rc)
		واپس format_iso_وقت(&tm, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	واپس -1;
}

/* relative time functions */
ساکن اندرونی صحیح وقت_is_thisyear(ساخت tm ثابت *ثابت tm,
				   ساخت tm ثابت *ثابت tmnow)
{
	واپس tm->tm_year == tmnow->tm_year;
}

ساکن اندرونی صحیح وقت_is_today(ساخت tm ثابت *ثابت tm,
				ساخت tm ثابت *ثابت tmnow)
{
	واپس (tm->tm_yday == tmnow->tm_yday &&
		وقت_is_thisyear(tm, tmnow));
}

صحیح strtime_short(ثابت time_t *t, ساخت timeval *now, صحیح flags, حرف *buf, size_t bufsz)
{
	ساخت tm tm, tmnow;
	صحیح rc = 0;

	اگر (now->tv_sec == 0)
		gettimeofday(now, تہی);

	localtime_r(t, &tm);
	localtime_r(&now->tv_sec, &tmnow);

	اگر (وقت_is_today(&tm, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", tm.tm_hour, tm.tm_min);
		اگر (rc < 0 || (size_t) rc > bufsz)
			واپس -1;
		rc = 1;

	} ورنہ اگر (وقت_is_thisyear(&tm, &tmnow)) {
		اگر (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = strftime(buf, bufsz, "%b%d/%H:%M", &tm);
		ورنہ
			rc = strftime(buf, bufsz, "%b%d", &tm);
	} ورنہ
		rc = strftime(buf, bufsz, "%Y-%b%d", &tm);

	واپس rc <= 0 ? -1 : 0;
}

صحیح strtimespec_relative(ثابت ساخت timespec *ts, حرف *buf, size_t bufsz)
{
	time_t secs = ts->tv_sec;
	size_t i, parts = 0;
	صحیح rc;

	اگر (bufsz)
		buf[0] = '\0';

	ساکن ثابت ساخت {
		ثابت حرف * ثابت suffix;
		صحیح چوڑائی;
		int64_t secs;
	} جدول[] = {
		{ "y", 4, NSEC_فی_YEAR   / NSEC_فی_SEC },
		{ "d", 3, NSEC_فی_DAY    / NSEC_فی_SEC },
		{ "h", 2, NSEC_فی_HOUR   / NSEC_فی_SEC },
		{ "m", 2, NSEC_فی_MINUTE / NSEC_فی_SEC },
		{ "s", 2, NSEC_فی_SEC    / NSEC_فی_SEC },
	};

	برائے (i = 0; i < ARRAY_سائز(جدول); i++) {
		اگر (secs >= جدول[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? جدول[i].چوڑائی : 0,
				      secs / جدول[i].secs, جدول[i].suffix,
				      secs % جدول[i].secs ? " " : "");
			اگر (rc < 0 || (size_t) rc > bufsz)
				جاؤ err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= جدول[i].secs;
		}
	}

	اگر (ts->tv_nsec) {
		اگر (ts->tv_nsec % NSEC_فی_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			اگر (rc < 0 || (size_t) rc > bufsz)
				جاؤ err;
		} ورنہ {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_فی_MSEC);
			اگر (rc < 0 || (size_t) rc > bufsz)
				جاؤ err;
		}
	}

	واپس 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	واپس -1;
}

#اگر_غیر_تعریف_شدہ HAVE_TIMEGM
time_t timegm(ساخت tm *tm)
{
	ثابت حرف *zone = getenv("TZ");
	time_t ret;

	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	اگر (zone)
		setenv("TZ", zone, 1);
	ورنہ
		unsetenv("TZ");
	tzset();
	واپس ret;
}
#اگر_ختم /* HAVE_TIMEGM */

#اگر_تعریف_شدہ TEST_پروگرام_TIMEUTILS

ساکن صحیح تسلسل_unittest_timestamp(خالی)
{
	صحیح rc = EXIT_SUCCESS;
	time_t reference = 1674180427;
	ساکن ثابت ساخت testcase {
		ثابت حرف * ثابت داخلہ;
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

	برائے (size_t i = 0; i < ARRAY_سائز(testcases); i++) {
		ساخت testcase t = testcases[i];
		usec_t نتیجہ;
		صحیح r = parse_timestamp_reference(reference, t.داخلہ, &نتیجہ);
		اگر (r) {
			fprintf(stderr, "Could not parse '%s'\n", t.داخلہ);
			rc = EXIT_FAILURE;
		}

		اگر (نتیجہ != t.expected) {
			fprintf(stderr, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.داخلہ, نتیجہ, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	واپس rc;
}

ساکن صحیح تسلسل_unittest_format(خالی)
{
	صحیح rc = EXIT_SUCCESS;
	ثابت ساخت timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	حرف buf[FORMAT_TIMESTAMP_MAX];
	ساکن ثابت ساخت testcase {
		صحیح flags;
		ثابت حرف * ثابت expected;
	} testcases[] = {
		{ ISO_DATE,               "2023-01-20"                       },
		{ ISO_وقت,               "02:07:07"                         },
		{ ISO_TIMEZONE,           "+00:00"                           },
		{ ISO_TIMESTAMP_T,        "2023-01-20T02:07:07+00:00"        },
		{ ISO_TIMESTAMP_COMMA_G,  "2023-01-20 02:07:07,000012+00:00" },
		{ ISO_وقت | ISO_DOTNSEC, "02:07:07.000012345" },
	};

	setenv("TZ", "GMT", 1);
	tzset();

	برائے (size_t i = 0; i < ARRAY_سائز(testcases); i++) {
		ساخت testcase t = testcases[i];
		صحیح r = strtimespec_iso(&ts, t.flags, buf, حجم(buf));
		اگر (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		اگر (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	واپس rc;
}

ساکن صحیح تسلسل_unittest_format_relative(خالی)
{
	صحیح rc = EXIT_SUCCESS;
	حرف buf[FORMAT_TIMESTAMP_MAX];
	ساکن ثابت ساخت testcase {
		ساخت timespec ts;
		ثابت حرف * ثابت expected;
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

	برائے (size_t i = 0; i < ARRAY_سائز(testcases); i++) {
		ساخت testcase t = testcases[i];
		صحیح r = strtimespec_relative(&t.ts, buf, حجم(buf));
		اگر (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		اگر (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	واپس rc;
}

صحیح مین(صحیح argc, حرف *argv[])
{
	ساخت timespec ts = { 0 };
	حرف buf[ISO_BUFSIZ];
	صحیح r;

	اگر (argc < 2) {
		fprintf(stderr, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		باہر_نکلو(EXIT_FAILURE);
	}

	اگر (strcmp(argv[1], "--unittest-timestamp") == 0)
		واپس تسلسل_unittest_timestamp();
	ورنہ اگر (strcmp(argv[1], "--unittest-format") == 0)
		واپس تسلسل_unittest_format();
	ورنہ اگر (strcmp(argv[1], "--unittest-format-relative") == 0)
		واپس تسلسل_unittest_format_relative();

	اگر (strcmp(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		اگر (r)
			errx(EXIT_FAILURE, "Can not parse '%s': %s", argv[2], strerror(-r));
		ts.tv_sec = (time_t) (usec / USEC_فی_SEC);
		ts.tv_nsec = (usec % USEC_فی_SEC) * NSEC_فی_USEC;
	} ورنہ {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		اگر (argc == 3)
			ts.tv_nsec = strtos64_or_err(argv[2], "failed to parse <usec>")
				     * NSEC_فی_USEC;
	}

	strtimespec_iso(&ts, ISO_DATE, buf, حجم(buf));
	لکھو("Date: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_وقت, buf, حجم(buf));
	لکھو("Time: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_DATE | ISO_وقت | ISO_COMMAUSEC | ISO_T,
		       buf, حجم(buf));
	لکھو("Full: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIMESTAMP_DOT, buf, حجم(buf));
	لکھو("Zone: '%s'\n", buf);

	strtimespec_relative(&ts, buf, حجم(buf));
	لکھو("Rel:  '%s'\n", buf);

	واپس EXIT_SUCCESS;
}

#اگر_ختم /* TEST_PROGRAM_TIMEUTILS */
