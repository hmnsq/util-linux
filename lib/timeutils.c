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

#define WHITESPACE " \t\n\r"

#define streq(a,b) (strcmp((a),(b)) == 0)

ایستا صحیح parse_sec(ثابت نویسه *t, usec_t *usec)
{
	ایستا ثابت ساختار {
		ثابت نویسه *suffix;
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

	ثابت نویسه *p;
	usec_t r = 0;
	صحیح something = FALSE;

	أكد(t);
	أكد(usec);

	p = t;
	برای (;;) {
		long long l, z = 0;
		نویسه *e;
		unsigned i, n = 0;

		p += strspn(p, WHITESPACE);

		اگر (*p == 0) {
			اگر (!something)
				بازگشت -EINVAL;

			بشکن;
		}

		errno = 0;
		l = strtoll(p, &e, 10);

		اگر (errno > 0)
			بازگشت -errno;

		اگر (l < 0)
			بازگشت -ERANGE;

		اگر (*e == '.') {
			نویسه *b = e + 1;

			errno = 0;
			z = strtoll(b, &e, 10);
			اگر (errno > 0)
				بازگشت -errno;

			اگر (z < 0)
				بازگشت -ERANGE;

			اگر (e == b)
				بازگشت -EINVAL;

			n = e - b;

		} وگرنه اگر (e == p)
			بازگشت -EINVAL;

		e += strspn(e, WHITESPACE);

		برای (i = 0; i < ARRAY_SIZE(table); i++)
			اگر (ul_startswith(e, table[i].suffix)) {
				usec_t k = (usec_t) z * table[i].usec;

				برای (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *table[i].usec + k;
				p = e + strlen(table[i].suffix);

				something = TRUE;
				بشکن;
			}

		اگر (i >= ARRAY_SIZE(table))
			بازگشت -EINVAL;

	}

	*usec = r;

	بازگشت 0;
}

ایستا صحیح parse_subseconds(ثابت نویسه *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t factor = USEC_PER_SEC / 10;

	اگر (*t != '.' && *t != ',')
		بازگشت -1;

	وقتی (*(++t)) {
		اگر (!isdigit(*t) || factor < 1)
			بازگشت -1;

		ret += ((usec_t) *t - '0') * factor;
		factor /= 10;
	}

	*usec = ret;
	بازگشت 0;
}

ایستا ثابت نویسه *parse_epoch_seconds(ثابت نویسه *t, ساختار tm *tm)
{
	int64_t s;
	time_t st;
	صحیح f, c;

	f = sscanf(t, "%"SCNd64"%n", &s, &c);
	اگر (f < 1)
		بازگشت پوچ;
	st = s;
	اگر ((int64_t) st < s)
		بازگشت پوچ;
	اگر (!localtime_r(&st, tm))
		بازگشت پوچ;
	بازگشت t + c;
}

ایستا صحیح parse_timestamp_reference(time_t x, ثابت نویسه *t, usec_t *usec)
{
	ایستا ثابت ساختار {
		ثابت نویسه *name;
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

	ثابت نویسه *k;
	ساختار tm tm, copy;
	usec_t plus = 0, minus = 0, ret = 0;
	صحیح r, weekday = -1;
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

	أكد(t);
	أكد(usec);

	localtime_r(&x, &tm);
	tm.tm_isdst = -1;

	اگر (streq(t, "now"))
		برو finish;

	وگرنه اگر (streq(t, "today")) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		برو finish;

	} وگرنه اگر (streq(t, "yesterday")) {
		tm.tm_mday--;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		برو finish;

	} وگرنه اگر (streq(t, "tomorrow")) {
		tm.tm_mday++;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		برو finish;

	} وگرنه اگر (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		اگر (r < 0)
			بازگشت r;

		برو finish;
	} وگرنه اگر (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		اگر (r < 0)
			بازگشت r;

		برو finish;
	} وگرنه اگر (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &tm);
		اگر (k && *k == 0)
			برو finish;
		وگرنه اگر (k && parse_subseconds(k, &ret) == 0)
			برو finish;

		بازگشت -EINVAL;
	} وگرنه اگر (ul_endswith(t, " ago")) {
		نویسه *z;

		z = strndup(t, strlen(t) - 4);
		اگر (!z)
			بازگشت -ENOMEM;

		r = parse_sec(z, &minus);
		آزاد(z);
		اگر (r < 0)
			بازگشت r;

		برو finish;
	}

	برای (i = 0; i < ARRAY_SIZE(day_nr); i++) {
		size_t skip;

		اگر (!startswith_no_case(t, day_nr[i].name))
			ادامه;

		skip = strlen(day_nr[i].name);
		اگر (t[skip] != ' ')
			ادامه;

		weekday = day_nr[i].nr;
		t += skip + 1;
		بشکن;
	}

	copy = tm;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &tm);
	اگر (k && *k == 0)
		برو finish;
	وگرنه اگر (k && parse_subseconds(k, &ret) == 0)
		برو finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &tm);
	اگر (k && *k == 0)
		برو finish;
	وگرنه اگر (k && parse_subseconds(k, &ret) == 0)
		برو finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &tm);
	اگر (k && *k == 0)
		برو finish;
	وگرنه اگر (k && parse_subseconds(k, &ret) == 0)
		برو finish;

	tm = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &tm);
	اگر (k && *k == 0) {
		tm.tm_sec = 0;
		برو finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &tm);
	اگر (k && *k == 0) {
		tm.tm_sec = 0;
		برو finish;
	}

	tm = copy;
	k = strptime(t, "%y-%m-%d", &tm);
	اگر (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		برو finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d", &tm);
	اگر (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		برو finish;
	}

	tm = copy;
	k = strptime(t, "%H:%M:%S", &tm);
	اگر (k && *k == 0)
		برو finish;
	وگرنه اگر (k && parse_subseconds(k, &ret) == 0)
		برو finish;

	tm = copy;
	k = strptime(t, "%H:%M", &tm);
	اگر (k && *k == 0) {
		tm.tm_sec = 0;
		برو finish;
	}

	tm = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &tm);
	اگر (k && *k == 0)
		برو finish;
	وگرنه اگر (k && parse_subseconds(k, &ret) == 0)
		برو finish;

	بازگشت -EINVAL;

 finish:
	x = mktime(&tm);
	اگر (x == (time_t)-1)
		بازگشت -EINVAL;

	اگر (weekday >= 0 && tm.tm_wday != weekday)
		بازگشت -EINVAL;

	ret += (usec_t) x * USEC_PER_SEC;

	اگر (minus > ret)
		بازگشت -ERANGE;
	اگر ((ret + plus) < ret)
		بازگشت -ERANGE;

	ret += plus;
	ret -= minus;

	*usec = ret;

	بازگشت 0;
}

صحیح ul_parse_timestamp(ثابت نویسه *t, usec_t *usec)
{
	بازگشت parse_timestamp_reference(time(پوچ), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
صحیح get_gmtoff(ثابت ساختار tm *tp)
{
	اگر (tp->tm_isdst < 0)
		بازگشت 0;

#اگر HAVE_TM_GMTOFF
	بازگشت tp->tm_gmtoff;
#وگرنه
	ساختار tm tm;
	ساختار tm gtm;
	ساختار tm ltm = *tp;
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
			بازگشت 0;
	}

	اگر (! gmtime_r(&lt, &gtm))
		بازگشت 0;

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

	بازگشت (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#endif
}

ایستا صحیح format_iso_time(ثابت ساختار tm *tm, uint32_t nsec, صحیح flags, نویسه *buf, size_t bufsz)
{
	uint32_t usec = nsec / NSEC_PER_USEC;
	نویسه *p = buf;
	صحیح len;

	اگر (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       tm->tm_year + (long) 1900,
			       tm->tm_mon + 1, tm->tm_mday);
		اگر (len < 0 || (size_t) len > bufsz)
			برو err;
		bufsz -= len;
		p += len;
	}

	اگر ((flags & ISO_DATE) && (flags & ISO_TIME)) {
		اگر (bufsz < 1)
			برو err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	اگر (flags & ISO_TIME) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
			       tm->tm_min, tm->tm_sec);
		اگر (len < 0 || (size_t) len > bufsz)
			برو err;
		bufsz -= len;
		p += len;
	}

	اگر (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		اگر (len < 0 || (size_t) len > bufsz)
			برو err;
		bufsz -= len;
		p += len;

	} وگرنه اگر (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		اگر (len < 0 || (size_t) len > bufsz)
			برو err;
		bufsz -= len;
		p += len;
	} وگرنه اگر (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		اگر (len < 0 || (size_t) len > bufsz)
			برو err;
		bufsz -= len;
		p += len;

	} وگرنه اگر (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		اگر (len < 0 || (size_t) len > bufsz)
			برو err;
		bufsz -= len;
		p += len;
	}

	اگر (flags & ISO_TIMEZONE) {
		صحیح tmin  = get_gmtoff(tm) / 60;
		صحیح zhour = tmin / 60;
		صحیح zmin  = abs(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		اگر (len < 0 || (size_t) len > bufsz)
			برو err;
	}
	بازگشت 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	بازگشت -1;
}

/* timespec to ISO 8601 */
صحیح strtimespec_iso(ثابت ساختار timespec *ts, صحیح flags, نویسه *buf, size_t bufsz)
{
	ساختار tm tm;
	ساختار tm *rc;

	اگر (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &tm);
	وگرنه
		rc = localtime_r(&ts->tv_sec, &tm);

	اگر (rc)
		بازگشت format_iso_time(&tm, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	بازگشت -1;
}

/* timeval to ISO 8601 */
صحیح strtimeval_iso(ثابت ساختار timeval *tv, صحیح flags, نویسه *buf, size_t bufsz)
{
	ساختار timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_PER_USEC,
	};

	بازگشت strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
صحیح strtm_iso(ثابت ساختار tm *tm, صحیح flags, نویسه *buf, size_t bufsz)
{
	بازگشت format_iso_time(tm, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
صحیح strtime_iso(ثابت time_t *t, صحیح flags, نویسه *buf, size_t bufsz)
{
	ساختار tm tm;
	ساختار tm *rc;

	اگر (flags & ISO_GMTIME)
		rc = gmtime_r(t, &tm);
	وگرنه
		rc = localtime_r(t, &tm);

	اگر (rc)
		بازگشت format_iso_time(&tm, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	بازگشت -1;
}

/* relative time functions */
ایستا درونخطی صحیح time_is_thisyear(ساختار tm ثابت *ثابت tm,
				   ساختار tm ثابت *ثابت tmnow)
{
	بازگشت tm->tm_year == tmnow->tm_year;
}

ایستا درونخطی صحیح time_is_today(ساختار tm ثابت *ثابت tm,
				ساختار tm ثابت *ثابت tmnow)
{
	بازگشت (tm->tm_yday == tmnow->tm_yday &&
		time_is_thisyear(tm, tmnow));
}

صحیح strtime_short(ثابت time_t *t, ساختار timeval *now, صحیح flags, نویسه *buf, size_t bufsz)
{
	ساختار tm tm, tmnow;
	صحیح rc = 0;

	اگر (now->tv_sec == 0)
		gettimeofday(now, پوچ);

	localtime_r(t, &tm);
	localtime_r(&now->tv_sec, &tmnow);

	اگر (time_is_today(&tm, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", tm.tm_hour, tm.tm_min);
		اگر (rc < 0 || (size_t) rc > bufsz)
			بازگشت -1;
		rc = 1;

	} وگرنه اگر (time_is_thisyear(&tm, &tmnow)) {
		اگر (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = strftime(buf, bufsz, "%b%d/%H:%M", &tm);
		وگرنه
			rc = strftime(buf, bufsz, "%b%d", &tm);
	} وگرنه
		rc = strftime(buf, bufsz, "%Y-%b%d", &tm);

	بازگشت rc <= 0 ? -1 : 0;
}

صحیح strtimespec_relative(ثابت ساختار timespec *ts, نویسه *buf, size_t bufsz)
{
	time_t secs = ts->tv_sec;
	size_t i, parts = 0;
	صحیح rc;

	اگر (bufsz)
		buf[0] = '\0';

	ایستا ثابت ساختار {
		ثابت نویسه * ثابت suffix;
		صحیح width;
		int64_t secs;
	} table[] = {
		{ "y", 4, NSEC_PER_YEAR   / NSEC_PER_SEC },
		{ "d", 3, NSEC_PER_DAY    / NSEC_PER_SEC },
		{ "h", 2, NSEC_PER_HOUR   / NSEC_PER_SEC },
		{ "m", 2, NSEC_PER_MINUTE / NSEC_PER_SEC },
		{ "s", 2, NSEC_PER_SEC    / NSEC_PER_SEC },
	};

	برای (i = 0; i < ARRAY_SIZE(table); i++) {
		اگر (secs >= table[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? table[i].width : 0,
				      secs / table[i].secs, table[i].suffix,
				      secs % table[i].secs ? " " : "");
			اگر (rc < 0 || (size_t) rc > bufsz)
				برو err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= table[i].secs;
		}
	}

	اگر (ts->tv_nsec) {
		اگر (ts->tv_nsec % NSEC_PER_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			اگر (rc < 0 || (size_t) rc > bufsz)
				برو err;
		} وگرنه {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_PER_MSEC);
			اگر (rc < 0 || (size_t) rc > bufsz)
				برو err;
		}
	}

	بازگشت 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	بازگشت -1;
}

#ifndef HAVE_TIMEGM
time_t timegm(ساختار tm *tm)
{
	ثابت نویسه *zone = getenv("TZ");
	time_t ret;

	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	اگر (zone)
		setenv("TZ", zone, 1);
	وگرنه
		unsetenv("TZ");
	tzset();
	بازگشت ret;
}
#endif /* HAVE_TIMEGM */

#ifdef TEST_PROGRAM_TIMEUTILS

ایستا صحیح run_unittest_timestamp(تهی)
{
	صحیح rc = EXIT_SUCCESS;
	time_t reference = 1674180427;
	ایستا ثابت ساختار testcase {
		ثابت نویسه * ثابت درون_داد;
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

	برای (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		ساختار testcase t = testcases[i];
		usec_t result;
		صحیح r = parse_timestamp_reference(reference, t.درون_داد, &result);
		اگر (r) {
			fprintf(stderr, "Could not parse '%s'\n", t.درون_داد);
			rc = EXIT_FAILURE;
		}

		اگر (result != t.expected) {
			fprintf(stderr, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.درون_داد, result, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	بازگشت rc;
}

ایستا صحیح run_unittest_format(تهی)
{
	صحیح rc = EXIT_SUCCESS;
	ثابت ساختار timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	نویسه buf[FORMAT_TIMESTAMP_MAX];
	ایستا ثابت ساختار testcase {
		صحیح flags;
		ثابت نویسه * ثابت expected;
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

	برای (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		ساختار testcase t = testcases[i];
		صحیح r = strtimespec_iso(&ts, t.flags, buf, اندازه(buf));
		اگر (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		اگر (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	بازگشت rc;
}

ایستا صحیح run_unittest_format_relative(تهی)
{
	صحیح rc = EXIT_SUCCESS;
	نویسه buf[FORMAT_TIMESTAMP_MAX];
	ایستا ثابت ساختار testcase {
		ساختار timespec ts;
		ثابت نویسه * ثابت expected;
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

	برای (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		ساختار testcase t = testcases[i];
		صحیح r = strtimespec_relative(&t.ts, buf, اندازه(buf));
		اگر (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		اگر (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	بازگشت rc;
}

صحیح اصلی(صحیح argc, نویسه *argv[])
{
	ساختار timespec ts = { 0 };
	نویسه buf[ISO_BUFSIZ];
	صحیح r;

	اگر (argc < 2) {
		fprintf(stderr, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	اگر (strcmp(argv[1], "--unittest-timestamp") == 0)
		بازگشت run_unittest_timestamp();
	وگرنه اگر (strcmp(argv[1], "--unittest-format") == 0)
		بازگشت run_unittest_format();
	وگرنه اگر (strcmp(argv[1], "--unittest-format-relative") == 0)
		بازگشت run_unittest_format_relative();

	اگر (strcmp(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		اگر (r)
			errx(EXIT_FAILURE, "Can not parse '%s': %s", argv[2], strerror(-r));
		ts.tv_sec = (time_t) (usec / USEC_PER_SEC);
		ts.tv_nsec = (usec % USEC_PER_SEC) * NSEC_PER_USEC;
	} وگرنه {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		اگر (argc == 3)
			ts.tv_nsec = strtos64_or_err(argv[2], "failed to parse <usec>")
				     * NSEC_PER_USEC;
	}

	strtimespec_iso(&ts, ISO_DATE, buf, اندازه(buf));
	printf("Date: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIME, buf, اندازه(buf));
	printf("Time: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_DATE | ISO_TIME | ISO_COMMAUSEC | ISO_T,
		       buf, اندازه(buf));
	printf("Full: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIMESTAMP_DOT, buf, اندازه(buf));
	printf("Zone: '%s'\n", buf);

	strtimespec_relative(&ts, buf, اندازه(buf));
	printf("Rel:  '%s'\n", buf);

	بازگشت EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_TIMEUTILS */
