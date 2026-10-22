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
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>

#include "c.h"
#include "nls.h"
#include "strutils.h"
#include "timeutils.h"

#define WHITESPACE " \t\n\r"

#define streq(a,b) (strcmp((a),(b)) == 0)

static ကိန်းပြည့် parse_sec(const var_na *t, usec_t *usec)
{
	static const sa_nracanaa {
		const var_na *suffix;
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

	const var_na *p;
	usec_t r = 0;
	ကိန်းပြည့် something = FALSE;

	assert(t);
	assert(usec);

	p = t;
	အတွက် (;;) {
		long long l, z = 0;
		var_na *e;
		unsigned i, n = 0;

		p += strspn(p, WHITESPACE);

		အကယ်၍ (*p == 0) {
			အကယ်၍ (!something)
				ပြန်ပေး -EINVAL;

			bha_nga;
		}

		errno = 0;
		l = strtoll(p, &e, 10);

		အကယ်၍ (errno > 0)
			ပြန်ပေး -errno;

		အကယ်၍ (l < 0)
			ပြန်ပေး -ERANGE;

		အကယ်၍ (*e == '.') {
			var_na *b = e + 1;

			errno = 0;
			z = strtoll(b, &e, 10);
			အကယ်၍ (errno > 0)
				ပြန်ပေး -errno;

			အကယ်၍ (z < 0)
				ပြန်ပေး -ERANGE;

			အကယ်၍ (e == b)
				ပြန်ပေး -EINVAL;

			n = e - b;

		} မဟုတ်ပါက အကယ်၍ (e == p)
			ပြန်ပေး -EINVAL;

		e += strspn(e, WHITESPACE);

		အတွက် (i = 0; i < ARRAY_SIZE(table); i++)
			အကယ်၍ (ul_startswith(e, table[i].suffix)) {
				usec_t k = (usec_t) z * table[i].usec;

				အတွက် (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *table[i].usec + k;
				p = e + strlen(table[i].suffix);

				something = TRUE;
				bha_nga;
			}

		အကယ်၍ (i >= ARRAY_SIZE(table))
			ပြန်ပေး -EINVAL;

	}

	*usec = r;

	ပြန်ပေး 0;
}

static ကိန်းပြည့် parse_subseconds(const var_na *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t factor = USEC_PER_SEC / 10;

	အကယ်၍ (*t != '.' && *t != ',')
		ပြန်ပေး -1;

	နေစဉ် (*(++t)) {
		အကယ်၍ (!isdigit(*t) || factor < 1)
			ပြန်ပေး -1;

		ret += ((usec_t) *t - '0') * factor;
		factor /= 10;
	}

	*usec = ret;
	ပြန်ပေး 0;
}

static const var_na *parse_epoch_seconds(const var_na *t, sa_nracanaa tm *tm)
{
	int64_t s;
	time_t st;
	ကိန်းပြည့် f, c;

	f = sscanf(t, "%"SCNd64"%n", &s, &c);
	အကယ်၍ (f < 1)
		ပြန်ပေး NULL;
	st = s;
	အကယ်၍ ((int64_t) st < s)
		ပြန်ပေး NULL;
	အကယ်၍ (!localtime_r(&st, tm))
		ပြန်ပေး NULL;
	ပြန်ပေး t + c;
}

static ကိန်းပြည့် parse_timestamp_reference(time_t x, const var_na *t, usec_t *usec)
{
	static const sa_nracanaa {
		const var_na *name;
		const ကိန်းပြည့် nr;
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

	const var_na *k;
	sa_nracanaa tm tm, copy;
	usec_t plus = 0, minus = 0, ret = 0;
	ကိန်းပြည့် r, weekday = -1;
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

	assert(t);
	assert(usec);

	localtime_r(&x, &tm);
	tm.tm_isdst = -1;

	အကယ်၍ (streq(t, "now"))
		goto finish;

	မဟုတ်ပါက အကယ်၍ (streq(t, "today")) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;

	} မဟုတ်ပါက အကယ်၍ (streq(t, "yesterday")) {
		tm.tm_mday--;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;

	} မဟုတ်ပါက အကယ်၍ (streq(t, "tomorrow")) {
		tm.tm_mday++;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;

	} မဟုတ်ပါက အကယ်၍ (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		အကယ်၍ (r < 0)
			ပြန်ပေး r;

		goto finish;
	} မဟုတ်ပါက အကယ်၍ (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		အကယ်၍ (r < 0)
			ပြန်ပေး r;

		goto finish;
	} မဟုတ်ပါက အကယ်၍ (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &tm);
		အကယ်၍ (k && *k == 0)
			goto finish;
		မဟုတ်ပါက အကယ်၍ (k && parse_subseconds(k, &ret) == 0)
			goto finish;

		ပြန်ပေး -EINVAL;
	} မဟုတ်ပါက အကယ်၍ (ul_endswith(t, " ago")) {
		var_na *z;

		z = strndup(t, strlen(t) - 4);
		အကယ်၍ (!z)
			ပြန်ပေး -ENOMEM;

		r = parse_sec(z, &minus);
		free(z);
		အကယ်၍ (r < 0)
			ပြန်ပေး r;

		goto finish;
	}

	အတွက် (i = 0; i < ARRAY_SIZE(day_nr); i++) {
		size_t skip;

		အကယ်၍ (!startswith_no_case(t, day_nr[i].name))
			continue;

		skip = strlen(day_nr[i].name);
		အကယ်၍ (t[skip] != ' ')
			continue;

		weekday = day_nr[i].nr;
		t += skip + 1;
		bha_nga;
	}

	copy = tm;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &tm);
	အကယ်၍ (k && *k == 0)
		goto finish;
	မဟုတ်ပါက အကယ်၍ (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &tm);
	အကယ်၍ (k && *k == 0)
		goto finish;
	မဟုတ်ပါက အကယ်၍ (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &tm);
	အကယ်၍ (k && *k == 0)
		goto finish;
	မဟုတ်ပါက အကယ်၍ (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	tm = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &tm);
	အကယ်၍ (k && *k == 0) {
		tm.tm_sec = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &tm);
	အကယ်၍ (k && *k == 0) {
		tm.tm_sec = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%y-%m-%d", &tm);
	အကယ်၍ (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d", &tm);
	အကယ်၍ (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%H:%M:%S", &tm);
	အကယ်၍ (k && *k == 0)
		goto finish;
	မဟုတ်ပါက အကယ်၍ (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	tm = copy;
	k = strptime(t, "%H:%M", &tm);
	အကယ်၍ (k && *k == 0) {
		tm.tm_sec = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &tm);
	အကယ်၍ (k && *k == 0)
		goto finish;
	မဟုတ်ပါက အကယ်၍ (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	ပြန်ပေး -EINVAL;

 finish:
	x = mktime(&tm);
	အကယ်၍ (x == (time_t)-1)
		ပြန်ပေး -EINVAL;

	အကယ်၍ (weekday >= 0 && tm.tm_wday != weekday)
		ပြန်ပေး -EINVAL;

	ret += (usec_t) x * USEC_PER_SEC;

	အကယ်၍ (minus > ret)
		ပြန်ပေး -ERANGE;
	အကယ်၍ ((ret + plus) < ret)
		ပြန်ပေး -ERANGE;

	ret += plus;
	ret -= minus;

	*usec = ret;

	ပြန်ပေး 0;
}

ကိန်းပြည့် ul_parse_timestamp(const var_na *t, usec_t *usec)
{
	ပြန်ပေး parse_timestamp_reference(time(NULL), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
ကိန်းပြည့် get_gmtoff(const sa_nracanaa tm *tp)
{
	အကယ်၍ (tp->tm_isdst < 0)
		ပြန်ပေး 0;

#အကယ်၍ HAVE_TM_GMTOFF
	ပြန်ပေး tp->tm_gmtoff;
#မဟုတ်ပါက
	sa_nracanaa tm tm;
	sa_nracanaa tm gtm;
	sa_nracanaa tm ltm = *tp;
	time_t lt;

	tzset();
	lt = mktime(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	အကယ်၍ (lt == (time_t) -1) {
		အကယ်၍ (! localtime_r(&lt, &tm)
			|| ((ltm.tm_sec ^ tm.tm_sec)
			    | (ltm.tm_min ^ tm.tm_min)
			    | (ltm.tm_hour ^ tm.tm_hour)
			    | (ltm.tm_mday ^ tm.tm_mday)
			    | (ltm.tm_mon ^ tm.tm_mon)
			    | (ltm.tm_year ^ tm.tm_year)))
			ပြန်ပေး 0;
	}

	အကယ်၍ (! gmtime_r(&lt, &gtm))
		ပြန်ပေး 0;

	/* Calculate the GMT offset, that is, the difference between the
	 * TP argument (ltm) and GMT (gtm).
	 *
	 * Compute intervening leap days correctly even if year is negative.
	 * Take care to avoid int overflow in leap day calculations, but it's OK
	 * to assume that A and B are close to each other.
	 */
	ကိန်းပြည့် a4 = (ltm.tm_year >> 2) + (1900 >> 2) - ! (ltm.tm_year & 3);
	ကိန်းပြည့် b4 = (gtm.tm_year >> 2) + (1900 >> 2) - ! (gtm.tm_year & 3);
	ကိန်းပြည့် a100 = a4 / 25 - (a4 % 25 < 0);
	ကိန်းပြည့် b100 = b4 / 25 - (b4 % 25 < 0);
	ကိန်းပြည့် a400 = a100 >> 2;
	ကိန်းပြည့် b400 = b100 >> 2;
	ကိန်းပြည့် intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

	ကိန်းပြည့် years = ltm.tm_year - gtm.tm_year;
	ကိန်းပြည့် days = (365 * years + intervening_leap_days
		    + (ltm.tm_yday - gtm.tm_yday));

	ပြန်ပေး (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#endif
}

static ကိန်းပြည့် format_iso_time(const sa_nracanaa tm *tm, uint32_t nsec, ကိန်းပြည့် flags, var_na *buf, size_t bufsz)
{
	uint32_t usec = nsec / NSEC_PER_USEC;
	var_na *p = buf;
	ကိန်းပြည့် len;

	အကယ်၍ (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       tm->tm_year + (long) 1900,
			       tm->tm_mon + 1, tm->tm_mday);
		အကယ်၍ (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;
	}

	အကယ်၍ ((flags & ISO_DATE) && (flags & ISO_TIME)) {
		အကယ်၍ (bufsz < 1)
			goto err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	အကယ်၍ (flags & ISO_TIME) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
			       tm->tm_min, tm->tm_sec);
		အကယ်၍ (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;
	}

	အကယ်၍ (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		အကယ်၍ (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;

	} မဟုတ်ပါက အကယ်၍ (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		အကယ်၍ (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;
	} မဟုတ်ပါက အကယ်၍ (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		အကယ်၍ (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;

	} မဟုတ်ပါက အကယ်၍ (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		အကယ်၍ (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;
	}

	အကယ်၍ (flags & ISO_TIMEZONE) {
		ကိန်းပြည့် tmin  = get_gmtoff(tm) / 60;
		ကိန်းပြည့် zhour = tmin / 60;
		ကိန်းပြည့် zmin  = abs(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		အကယ်၍ (len < 0 || (size_t) len > bufsz)
			goto err;
	}
	ပြန်ပေး 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	ပြန်ပေး -1;
}

/* timespec to ISO 8601 */
ကိန်းပြည့် strtimespec_iso(const sa_nracanaa timespec *ts, ကိန်းပြည့် flags, var_na *buf, size_t bufsz)
{
	sa_nracanaa tm tm;
	sa_nracanaa tm *rc;

	အကယ်၍ (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &tm);
	မဟုတ်ပါက
		rc = localtime_r(&ts->tv_sec, &tm);

	အကယ်၍ (rc)
		ပြန်ပေး format_iso_time(&tm, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	ပြန်ပေး -1;
}

/* timeval to ISO 8601 */
ကိန်းပြည့် strtimeval_iso(const sa_nracanaa timeval *tv, ကိန်းပြည့် flags, var_na *buf, size_t bufsz)
{
	sa_nracanaa timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_PER_USEC,
	};

	ပြန်ပေး strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
ကိန်းပြည့် strtm_iso(const sa_nracanaa tm *tm, ကိန်းပြည့် flags, var_na *buf, size_t bufsz)
{
	ပြန်ပေး format_iso_time(tm, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
ကိန်းပြည့် strtime_iso(const time_t *t, ကိန်းပြည့် flags, var_na *buf, size_t bufsz)
{
	sa_nracanaa tm tm;
	sa_nracanaa tm *rc;

	အကယ်၍ (flags & ISO_GMTIME)
		rc = gmtime_r(t, &tm);
	မဟုတ်ပါက
		rc = localtime_r(t, &tm);

	အကယ်၍ (rc)
		ပြန်ပေး format_iso_time(&tm, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	ပြန်ပေး -1;
}

/* relative time functions */
static anta_hstha ကိန်းပြည့် time_is_thisyear(sa_nracanaa tm const *const tm,
				   sa_nracanaa tm const *const tmnow)
{
	ပြန်ပေး tm->tm_year == tmnow->tm_year;
}

static anta_hstha ကိန်းပြည့် time_is_today(sa_nracanaa tm const *const tm,
				sa_nracanaa tm const *const tmnow)
{
	ပြန်ပေး (tm->tm_yday == tmnow->tm_yday &&
		time_is_thisyear(tm, tmnow));
}

ကိန်းပြည့် strtime_short(const time_t *t, sa_nracanaa timeval *now, ကိန်းပြည့် flags, var_na *buf, size_t bufsz)
{
	sa_nracanaa tm tm, tmnow;
	ကိန်းပြည့် rc = 0;

	အကယ်၍ (now->tv_sec == 0)
		gettimeofday(now, NULL);

	localtime_r(t, &tm);
	localtime_r(&now->tv_sec, &tmnow);

	အကယ်၍ (time_is_today(&tm, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", tm.tm_hour, tm.tm_min);
		အကယ်၍ (rc < 0 || (size_t) rc > bufsz)
			ပြန်ပေး -1;
		rc = 1;

	} မဟုတ်ပါက အကယ်၍ (time_is_thisyear(&tm, &tmnow)) {
		အကယ်၍ (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = strftime(buf, bufsz, "%b%d/%H:%M", &tm);
		မဟုတ်ပါက
			rc = strftime(buf, bufsz, "%b%d", &tm);
	} မဟုတ်ပါက
		rc = strftime(buf, bufsz, "%Y-%b%d", &tm);

	ပြန်ပေး rc <= 0 ? -1 : 0;
}

ကိန်းပြည့် strtimespec_relative(const sa_nracanaa timespec *ts, var_na *buf, size_t bufsz)
{
	time_t secs = ts->tv_sec;
	size_t i, parts = 0;
	ကိန်းပြည့် rc;

	အကယ်၍ (bufsz)
		buf[0] = '\0';

	static const sa_nracanaa {
		const var_na * const suffix;
		ကိန်းပြည့် width;
		int64_t secs;
	} table[] = {
		{ "y", 4, NSEC_PER_YEAR   / NSEC_PER_SEC },
		{ "d", 3, NSEC_PER_DAY    / NSEC_PER_SEC },
		{ "h", 2, NSEC_PER_HOUR   / NSEC_PER_SEC },
		{ "m", 2, NSEC_PER_MINUTE / NSEC_PER_SEC },
		{ "s", 2, NSEC_PER_SEC    / NSEC_PER_SEC },
	};

	အတွက် (i = 0; i < ARRAY_SIZE(table); i++) {
		အကယ်၍ (secs >= table[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? table[i].width : 0,
				      secs / table[i].secs, table[i].suffix,
				      secs % table[i].secs ? " " : "");
			အကယ်၍ (rc < 0 || (size_t) rc > bufsz)
				goto err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= table[i].secs;
		}
	}

	အကယ်၍ (ts->tv_nsec) {
		အကယ်၍ (ts->tv_nsec % NSEC_PER_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			အကယ်၍ (rc < 0 || (size_t) rc > bufsz)
				goto err;
		} မဟုတ်ပါက {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_PER_MSEC);
			အကယ်၍ (rc < 0 || (size_t) rc > bufsz)
				goto err;
		}
	}

	ပြန်ပေး 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	ပြန်ပေး -1;
}

#ifndef HAVE_TIMEGM
time_t timegm(sa_nracanaa tm *tm)
{
	const var_na *zone = getenv("TZ");
	time_t ret;

	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	အကယ်၍ (zone)
		setenv("TZ", zone, 1);
	မဟုတ်ပါက
		unsetenv("TZ");
	tzset();
	ပြန်ပေး ret;
}
#endif /* HAVE_TIMEGM */

#ifdef TEST_PROGRAM_TIMEUTILS

static ကိန်းပြည့် run_unittest_timestamp(void)
{
	ကိန်းပြည့် rc = EXIT_SUCCESS;
	time_t reference = 1674180427;
	static const sa_nracanaa testcase {
		const var_na * const input;
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

	အတွက် (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		sa_nracanaa testcase t = testcases[i];
		usec_t result;
		ကိန်းပြည့် r = parse_timestamp_reference(reference, t.input, &result);
		အကယ်၍ (r) {
			fprintf(stderr, "Could not parse '%s'\n", t.input);
			rc = EXIT_FAILURE;
		}

		အကယ်၍ (result != t.expected) {
			fprintf(stderr, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.input, result, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ပြန်ပေး rc;
}

static ကိန်းပြည့် run_unittest_format(void)
{
	ကိန်းပြည့် rc = EXIT_SUCCESS;
	const sa_nracanaa timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	var_na buf[FORMAT_TIMESTAMP_MAX];
	static const sa_nracanaa testcase {
		ကိန်းပြည့် flags;
		const var_na * const expected;
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

	အတွက် (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		sa_nracanaa testcase t = testcases[i];
		ကိန်းပြည့် r = strtimespec_iso(&ts, t.flags, buf, parimaa_na(buf));
		အကယ်၍ (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		အကယ်၍ (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ပြန်ပေး rc;
}

static ကိန်းပြည့် run_unittest_format_relative(void)
{
	ကိန်းပြည့် rc = EXIT_SUCCESS;
	var_na buf[FORMAT_TIMESTAMP_MAX];
	static const sa_nracanaa testcase {
		sa_nracanaa timespec ts;
		const var_na * const expected;
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

	အတွက် (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		sa_nracanaa testcase t = testcases[i];
		ကိန်းပြည့် r = strtimespec_relative(&t.ts, buf, parimaa_na(buf));
		အကယ်၍ (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		အကယ်၍ (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ပြန်ပေး rc;
}

ကိန်းပြည့် main(ကိန်းပြည့် argc, var_na *argv[])
{
	sa_nracanaa timespec ts = { 0 };
	var_na buf[ISO_BUFSIZ];
	ကိန်းပြည့် r;

	အကယ်၍ (argc < 2) {
		fprintf(stderr, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	အကယ်၍ (strcmp(argv[1], "--unittest-timestamp") == 0)
		ပြန်ပေး run_unittest_timestamp();
	မဟုတ်ပါက အကယ်၍ (strcmp(argv[1], "--unittest-format") == 0)
		ပြန်ပေး run_unittest_format();
	မဟုတ်ပါက အကယ်၍ (strcmp(argv[1], "--unittest-format-relative") == 0)
		ပြန်ပေး run_unittest_format_relative();

	အကယ်၍ (strcmp(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		အကယ်၍ (r)
			errx(EXIT_FAILURE, "Can not parse '%s': %s", argv[2], strerror(-r));
		ts.tv_sec = (time_t) (usec / USEC_PER_SEC);
		ts.tv_nsec = (usec % USEC_PER_SEC) * NSEC_PER_USEC;
	} မဟုတ်ပါက {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		အကယ်၍ (argc == 3)
			ts.tv_nsec = strtos64_or_err(argv[2], "failed to parse <usec>")
				     * NSEC_PER_USEC;
	}

	strtimespec_iso(&ts, ISO_DATE, buf, parimaa_na(buf));
	printf("Date: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIME, buf, parimaa_na(buf));
	printf("Time: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_DATE | ISO_TIME | ISO_COMMAUSEC | ISO_T,
		       buf, parimaa_na(buf));
	printf("Full: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIMESTAMP_DOT, buf, parimaa_na(buf));
	printf("Zone: '%s'\n", buf);

	strtimespec_relative(&ts, buf, parimaa_na(buf));
	printf("Rel:  '%s'\n", buf);

	ပြန်ပေး EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_TIMEUTILS */
