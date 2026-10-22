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
#ضمن <assert.h>
#ضمن <ctype.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <time.h>
#ضمن <sys/time.h>
#ضمن <inttypes.h>

#ضمن "c.h"
#ضمن "nls.h"
#ضمن "strutils.h"
#ضمن "timeutils.h"

#define WHITESPACE " \t\n\r"

#define streq(a,b) (strcmp((a),(b)) == 0)

ساكن صحيح parse_sec(ثابت حرف *t, usec_t *usec)
{
	ساكن ثابت بنية {
		ثابت حرف *suffix;
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

	ثابت حرف *p;
	usec_t r = 0;
	صحيح something = FALSE;

	أكد(t);
	أكد(usec);

	p = t;
	لكل (;;) {
		long long l, z = 0;
		حرف *e;
		unsigned i, n = 0;

		p += strspn(p, WHITESPACE);

		𐤀𐤌 (*p == 0) {
			𐤀𐤌 (!something)
				أرجع -EINVAL;

			اكسر;
		}

		errno = 0;
		l = strtoll(p, &e, 10);

		𐤀𐤌 (errno > 0)
			أرجع -errno;

		𐤀𐤌 (l < 0)
			أرجع -ERANGE;

		𐤀𐤌 (*e == '.') {
			حرف *b = e + 1;

			errno = 0;
			z = strtoll(b, &e, 10);
			𐤀𐤌 (errno > 0)
				أرجع -errno;

			𐤀𐤌 (z < 0)
				أرجع -ERANGE;

			𐤀𐤌 (e == b)
				أرجع -EINVAL;

			n = e - b;

		} وإلا 𐤀𐤌 (e == p)
			أرجع -EINVAL;

		e += strspn(e, WHITESPACE);

		لكل (i = 0; i < ARRAY_SIZE(table); i++)
			𐤀𐤌 (ul_startswith(e, table[i].suffix)) {
				usec_t k = (usec_t) z * table[i].usec;

				لكل (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *table[i].usec + k;
				p = e + strlen(table[i].suffix);

				something = TRUE;
				اكسر;
			}

		𐤀𐤌 (i >= ARRAY_SIZE(table))
			أرجع -EINVAL;

	}

	*usec = r;

	أرجع 0;
}

ساكن صحيح parse_subseconds(ثابت حرف *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t factor = USEC_PER_SEC / 10;

	𐤀𐤌 (*t != '.' && *t != ',')
		أرجع -1;

	طالما (*(++t)) {
		𐤀𐤌 (!isdigit(*t) || factor < 1)
			أرجع -1;

		ret += ((usec_t) *t - '0') * factor;
		factor /= 10;
	}

	*usec = ret;
	أرجع 0;
}

ساكن ثابت حرف *parse_epoch_seconds(ثابت حرف *t, بنية tm *tm)
{
	int64_t s;
	time_t st;
	صحيح f, c;

	f = sscanf(t, "%"SCNd64"%n", &s, &c);
	𐤀𐤌 (f < 1)
		أرجع عدم;
	st = s;
	𐤀𐤌 ((int64_t) st < s)
		أرجع عدم;
	𐤀𐤌 (!localtime_r(&st, tm))
		أرجع عدم;
	أرجع t + c;
}

ساكن صحيح parse_timestamp_reference(time_t x, ثابت حرف *t, usec_t *usec)
{
	ساكن ثابت بنية {
		ثابت حرف *name;
		ثابت صحيح nr;
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
	بنية tm tm, copy;
	usec_t plus = 0, minus = 0, ret = 0;
	صحيح r, weekday = -1;
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

	𐤀𐤌 (streq(t, "now"))
		اذهب finish;

	وإلا 𐤀𐤌 (streq(t, "today")) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		اذهب finish;

	} وإلا 𐤀𐤌 (streq(t, "yesterday")) {
		tm.tm_mday--;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		اذهب finish;

	} وإلا 𐤀𐤌 (streq(t, "tomorrow")) {
		tm.tm_mday++;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		اذهب finish;

	} وإلا 𐤀𐤌 (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		𐤀𐤌 (r < 0)
			أرجع r;

		اذهب finish;
	} وإلا 𐤀𐤌 (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		𐤀𐤌 (r < 0)
			أرجع r;

		اذهب finish;
	} وإلا 𐤀𐤌 (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &tm);
		𐤀𐤌 (k && *k == 0)
			اذهب finish;
		وإلا 𐤀𐤌 (k && parse_subseconds(k, &ret) == 0)
			اذهب finish;

		أرجع -EINVAL;
	} وإلا 𐤀𐤌 (ul_endswith(t, " ago")) {
		حرف *z;

		z = strndup(t, strlen(t) - 4);
		𐤀𐤌 (!z)
			أرجع -ENOMEM;

		r = parse_sec(z, &minus);
		حرر(z);
		𐤀𐤌 (r < 0)
			أرجع r;

		اذهب finish;
	}

	لكل (i = 0; i < ARRAY_SIZE(day_nr); i++) {
		size_t skip;

		𐤀𐤌 (!startswith_no_case(t, day_nr[i].name))
			استمر;

		skip = strlen(day_nr[i].name);
		𐤀𐤌 (t[skip] != ' ')
			استمر;

		weekday = day_nr[i].nr;
		t += skip + 1;
		اكسر;
	}

	copy = tm;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &tm);
	𐤀𐤌 (k && *k == 0)
		اذهب finish;
	وإلا 𐤀𐤌 (k && parse_subseconds(k, &ret) == 0)
		اذهب finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &tm);
	𐤀𐤌 (k && *k == 0)
		اذهب finish;
	وإلا 𐤀𐤌 (k && parse_subseconds(k, &ret) == 0)
		اذهب finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &tm);
	𐤀𐤌 (k && *k == 0)
		اذهب finish;
	وإلا 𐤀𐤌 (k && parse_subseconds(k, &ret) == 0)
		اذهب finish;

	tm = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &tm);
	𐤀𐤌 (k && *k == 0) {
		tm.tm_sec = 0;
		اذهب finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &tm);
	𐤀𐤌 (k && *k == 0) {
		tm.tm_sec = 0;
		اذهب finish;
	}

	tm = copy;
	k = strptime(t, "%y-%m-%d", &tm);
	𐤀𐤌 (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		اذهب finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d", &tm);
	𐤀𐤌 (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		اذهب finish;
	}

	tm = copy;
	k = strptime(t, "%H:%M:%S", &tm);
	𐤀𐤌 (k && *k == 0)
		اذهب finish;
	وإلا 𐤀𐤌 (k && parse_subseconds(k, &ret) == 0)
		اذهب finish;

	tm = copy;
	k = strptime(t, "%H:%M", &tm);
	𐤀𐤌 (k && *k == 0) {
		tm.tm_sec = 0;
		اذهب finish;
	}

	tm = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &tm);
	𐤀𐤌 (k && *k == 0)
		اذهب finish;
	وإلا 𐤀𐤌 (k && parse_subseconds(k, &ret) == 0)
		اذهب finish;

	أرجع -EINVAL;

 finish:
	x = mktime(&tm);
	𐤀𐤌 (x == (time_t)-1)
		أرجع -EINVAL;

	𐤀𐤌 (weekday >= 0 && tm.tm_wday != weekday)
		أرجع -EINVAL;

	ret += (usec_t) x * USEC_PER_SEC;

	𐤀𐤌 (minus > ret)
		أرجع -ERANGE;
	𐤀𐤌 ((ret + plus) < ret)
		أرجع -ERANGE;

	ret += plus;
	ret -= minus;

	*usec = ret;

	أرجع 0;
}

صحيح ul_parse_timestamp(ثابت حرف *t, usec_t *usec)
{
	أرجع parse_timestamp_reference(time(عدم), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
صحيح get_gmtoff(ثابت بنية tm *tp)
{
	𐤀𐤌 (tp->tm_isdst < 0)
		أرجع 0;

#𐤀𐤌 HAVE_TM_GMTOFF
	أرجع tp->tm_gmtoff;
#وإلا
	بنية tm tm;
	بنية tm gtm;
	بنية tm ltm = *tp;
	time_t lt;

	tzset();
	lt = mktime(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	𐤀𐤌 (lt == (time_t) -1) {
		𐤀𐤌 (! localtime_r(&lt, &tm)
			|| ((ltm.tm_sec ^ tm.tm_sec)
			    | (ltm.tm_min ^ tm.tm_min)
			    | (ltm.tm_hour ^ tm.tm_hour)
			    | (ltm.tm_mday ^ tm.tm_mday)
			    | (ltm.tm_mon ^ tm.tm_mon)
			    | (ltm.tm_year ^ tm.tm_year)))
			أرجع 0;
	}

	𐤀𐤌 (! gmtime_r(&lt, &gtm))
		أرجع 0;

	/* Calculate the GMT offset, that is, the difference between the
	 * TP argument (ltm) and GMT (gtm).
	 *
	 * Compute intervening leap days correctly even if year is negative.
	 * Take care to avoid int overflow in leap day calculations, but it's OK
	 * to assume that A and B are close to each other.
	 */
	صحيح a4 = (ltm.tm_year >> 2) + (1900 >> 2) - ! (ltm.tm_year & 3);
	صحيح b4 = (gtm.tm_year >> 2) + (1900 >> 2) - ! (gtm.tm_year & 3);
	صحيح a100 = a4 / 25 - (a4 % 25 < 0);
	صحيح b100 = b4 / 25 - (b4 % 25 < 0);
	صحيح a400 = a100 >> 2;
	صحيح b400 = b100 >> 2;
	صحيح intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

	صحيح years = ltm.tm_year - gtm.tm_year;
	صحيح days = (365 * years + intervening_leap_days
		    + (ltm.tm_yday - gtm.tm_yday));

	أرجع (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#endif
}

ساكن صحيح format_iso_time(ثابت بنية tm *tm, uint32_t nsec, صحيح flags, حرف *buf, size_t bufsz)
{
	uint32_t usec = nsec / NSEC_PER_USEC;
	حرف *p = buf;
	صحيح len;

	𐤀𐤌 (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       tm->tm_year + (long) 1900,
			       tm->tm_mon + 1, tm->tm_mday);
		𐤀𐤌 (len < 0 || (size_t) len > bufsz)
			اذهب err;
		bufsz -= len;
		p += len;
	}

	𐤀𐤌 ((flags & ISO_DATE) && (flags & ISO_TIME)) {
		𐤀𐤌 (bufsz < 1)
			اذهب err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	𐤀𐤌 (flags & ISO_TIME) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
			       tm->tm_min, tm->tm_sec);
		𐤀𐤌 (len < 0 || (size_t) len > bufsz)
			اذهب err;
		bufsz -= len;
		p += len;
	}

	𐤀𐤌 (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		𐤀𐤌 (len < 0 || (size_t) len > bufsz)
			اذهب err;
		bufsz -= len;
		p += len;

	} وإلا 𐤀𐤌 (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		𐤀𐤌 (len < 0 || (size_t) len > bufsz)
			اذهب err;
		bufsz -= len;
		p += len;
	} وإلا 𐤀𐤌 (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		𐤀𐤌 (len < 0 || (size_t) len > bufsz)
			اذهب err;
		bufsz -= len;
		p += len;

	} وإلا 𐤀𐤌 (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		𐤀𐤌 (len < 0 || (size_t) len > bufsz)
			اذهب err;
		bufsz -= len;
		p += len;
	}

	𐤀𐤌 (flags & ISO_TIMEZONE) {
		صحيح tmin  = get_gmtoff(tm) / 60;
		صحيح zhour = tmin / 60;
		صحيح zmin  = abs(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		𐤀𐤌 (len < 0 || (size_t) len > bufsz)
			اذهب err;
	}
	أرجع 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	أرجع -1;
}

/* timespec to ISO 8601 */
صحيح strtimespec_iso(ثابت بنية timespec *ts, صحيح flags, حرف *buf, size_t bufsz)
{
	بنية tm tm;
	بنية tm *rc;

	𐤀𐤌 (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &tm);
	وإلا
		rc = localtime_r(&ts->tv_sec, &tm);

	𐤀𐤌 (rc)
		أرجع format_iso_time(&tm, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	أرجع -1;
}

/* timeval to ISO 8601 */
صحيح strtimeval_iso(ثابت بنية timeval *tv, صحيح flags, حرف *buf, size_t bufsz)
{
	بنية timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_PER_USEC,
	};

	أرجع strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
صحيح strtm_iso(ثابت بنية tm *tm, صحيح flags, حرف *buf, size_t bufsz)
{
	أرجع format_iso_time(tm, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
صحيح strtime_iso(ثابت time_t *t, صحيح flags, حرف *buf, size_t bufsz)
{
	بنية tm tm;
	بنية tm *rc;

	𐤀𐤌 (flags & ISO_GMTIME)
		rc = gmtime_r(t, &tm);
	وإلا
		rc = localtime_r(t, &tm);

	𐤀𐤌 (rc)
		أرجع format_iso_time(&tm, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	أرجع -1;
}

/* relative time functions */
ساكن مضمن صحيح time_is_thisyear(بنية tm ثابت *ثابت tm,
				   بنية tm ثابت *ثابت tmnow)
{
	أرجع tm->tm_year == tmnow->tm_year;
}

ساكن مضمن صحيح time_is_today(بنية tm ثابت *ثابت tm,
				بنية tm ثابت *ثابت tmnow)
{
	أرجع (tm->tm_yday == tmnow->tm_yday &&
		time_is_thisyear(tm, tmnow));
}

صحيح strtime_short(ثابت time_t *t, بنية timeval *now, صحيح flags, حرف *buf, size_t bufsz)
{
	بنية tm tm, tmnow;
	صحيح rc = 0;

	𐤀𐤌 (now->tv_sec == 0)
		gettimeofday(now, عدم);

	localtime_r(t, &tm);
	localtime_r(&now->tv_sec, &tmnow);

	𐤀𐤌 (time_is_today(&tm, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", tm.tm_hour, tm.tm_min);
		𐤀𐤌 (rc < 0 || (size_t) rc > bufsz)
			أرجع -1;
		rc = 1;

	} وإلا 𐤀𐤌 (time_is_thisyear(&tm, &tmnow)) {
		𐤀𐤌 (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = strftime(buf, bufsz, "%b%d/%H:%M", &tm);
		وإلا
			rc = strftime(buf, bufsz, "%b%d", &tm);
	} وإلا
		rc = strftime(buf, bufsz, "%Y-%b%d", &tm);

	أرجع rc <= 0 ? -1 : 0;
}

صحيح strtimespec_relative(ثابت بنية timespec *ts, حرف *buf, size_t bufsz)
{
	time_t secs = ts->tv_sec;
	size_t i, parts = 0;
	صحيح rc;

	𐤀𐤌 (bufsz)
		buf[0] = '\0';

	ساكن ثابت بنية {
		ثابت حرف * ثابت suffix;
		صحيح width;
		int64_t secs;
	} table[] = {
		{ "y", 4, NSEC_PER_YEAR   / NSEC_PER_SEC },
		{ "d", 3, NSEC_PER_DAY    / NSEC_PER_SEC },
		{ "h", 2, NSEC_PER_HOUR   / NSEC_PER_SEC },
		{ "m", 2, NSEC_PER_MINUTE / NSEC_PER_SEC },
		{ "s", 2, NSEC_PER_SEC    / NSEC_PER_SEC },
	};

	لكل (i = 0; i < ARRAY_SIZE(table); i++) {
		𐤀𐤌 (secs >= table[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? table[i].width : 0,
				      secs / table[i].secs, table[i].suffix,
				      secs % table[i].secs ? " " : "");
			𐤀𐤌 (rc < 0 || (size_t) rc > bufsz)
				اذهب err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= table[i].secs;
		}
	}

	𐤀𐤌 (ts->tv_nsec) {
		𐤀𐤌 (ts->tv_nsec % NSEC_PER_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			𐤀𐤌 (rc < 0 || (size_t) rc > bufsz)
				اذهب err;
		} وإلا {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_PER_MSEC);
			𐤀𐤌 (rc < 0 || (size_t) rc > bufsz)
				اذهب err;
		}
	}

	أرجع 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	أرجع -1;
}

#ifndef HAVE_TIMEGM
time_t timegm(بنية tm *tm)
{
	ثابت حرف *zone = getenv("TZ");
	time_t ret;

	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	𐤀𐤌 (zone)
		setenv("TZ", zone, 1);
	وإلا
		unsetenv("TZ");
	tzset();
	أرجع ret;
}
#endif /* HAVE_TIMEGM */

#ifdef TEST_PROGRAM_TIMEUTILS

ساكن صحيح run_unittest_timestamp(فراغ)
{
	صحيح rc = EXIT_SUCCESS;
	time_t reference = 1674180427;
	ساكن ثابت بنية testcase {
		ثابت حرف * ثابت input;
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

	لكل (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		بنية testcase t = testcases[i];
		usec_t result;
		صحيح r = parse_timestamp_reference(reference, t.input, &result);
		𐤀𐤌 (r) {
			fprintf(stderr, "Could not parse '%s'\n", t.input);
			rc = EXIT_FAILURE;
		}

		𐤀𐤌 (result != t.expected) {
			fprintf(stderr, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.input, result, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	أرجع rc;
}

ساكن صحيح run_unittest_format(فراغ)
{
	صحيح rc = EXIT_SUCCESS;
	ثابت بنية timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	حرف buf[FORMAT_TIMESTAMP_MAX];
	ساكن ثابت بنية testcase {
		صحيح flags;
		ثابت حرف * ثابت expected;
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

	لكل (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		بنية testcase t = testcases[i];
		صحيح r = strtimespec_iso(&ts, t.flags, buf, حجم(buf));
		𐤀𐤌 (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		𐤀𐤌 (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	أرجع rc;
}

ساكن صحيح run_unittest_format_relative(فراغ)
{
	صحيح rc = EXIT_SUCCESS;
	حرف buf[FORMAT_TIMESTAMP_MAX];
	ساكن ثابت بنية testcase {
		بنية timespec ts;
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

	لكل (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		بنية testcase t = testcases[i];
		صحيح r = strtimespec_relative(&t.ts, buf, حجم(buf));
		𐤀𐤌 (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		𐤀𐤌 (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	أرجع rc;
}

صحيح main(صحيح argc, حرف *argv[])
{
	بنية timespec ts = { 0 };
	حرف buf[ISO_BUFSIZ];
	صحيح r;

	𐤀𐤌 (argc < 2) {
		fprintf(stderr, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	𐤀𐤌 (strcmp(argv[1], "--unittest-timestamp") == 0)
		أرجع run_unittest_timestamp();
	وإلا 𐤀𐤌 (strcmp(argv[1], "--unittest-format") == 0)
		أرجع run_unittest_format();
	وإلا 𐤀𐤌 (strcmp(argv[1], "--unittest-format-relative") == 0)
		أرجع run_unittest_format_relative();

	𐤀𐤌 (strcmp(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		𐤀𐤌 (r)
			errx(EXIT_FAILURE, "Can not parse '%s': %s", argv[2], strerror(-r));
		ts.tv_sec = (time_t) (usec / USEC_PER_SEC);
		ts.tv_nsec = (usec % USEC_PER_SEC) * NSEC_PER_USEC;
	} وإلا {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		𐤀𐤌 (argc == 3)
			ts.tv_nsec = strtos64_or_err(argv[2], "failed to parse <usec>")
				     * NSEC_PER_USEC;
	}

	strtimespec_iso(&ts, ISO_DATE, buf, حجم(buf));
	printf("Date: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIME, buf, حجم(buf));
	printf("Time: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_DATE | ISO_TIME | ISO_COMMAUSEC | ISO_T,
		       buf, حجم(buf));
	printf("Full: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIMESTAMP_DOT, buf, حجم(buf));
	printf("Zone: '%s'\n", buf);

	strtimespec_relative(&ts, buf, حجم(buf));
	printf("Rel:  '%s'\n", buf);

	أرجع EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_TIMEUTILS */
