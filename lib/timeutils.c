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

static puur_naa_nka parse_sec(const var_na *t, usec_t *usec)
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
	puur_naa_nka something = FALSE;

	assert(t);
	assert(usec);

	p = t;
	for (;;) {
		long long l, z = 0;
		var_na *e;
		unsigned i, n = 0;

		p += strspn(p, WHITESPACE);

		ပေႃး (*p == 0) {
			ပေႃး (!something)
				ပွၵ်ႈ -EINVAL;

			bha_nga;
		}

		errno = 0;
		l = strtoll(p, &e, 10);

		ပေႃး (errno > 0)
			ပွၵ်ႈ -errno;

		ပေႃး (l < 0)
			ပွၵ်ႈ -ERANGE;

		ပေႃး (*e == '.') {
			var_na *b = e + 1;

			errno = 0;
			z = strtoll(b, &e, 10);
			ပေႃး (errno > 0)
				ပွၵ်ႈ -errno;

			ပေႃး (z < 0)
				ပွၵ်ႈ -ERANGE;

			ပေႃး (e == b)
				ပွၵ်ႈ -EINVAL;

			n = e - b;

		} ပေႃးဢမ်ႇ ပေႃး (e == p)
			ပွၵ်ႈ -EINVAL;

		e += strspn(e, WHITESPACE);

		for (i = 0; i < ARRAY_SIZE(table); i++)
			ပေႃး (ul_startswith(e, table[i].suffix)) {
				usec_t k = (usec_t) z * table[i].usec;

				for (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *table[i].usec + k;
				p = e + strlen(table[i].suffix);

				something = TRUE;
				bha_nga;
			}

		ပေႃး (i >= ARRAY_SIZE(table))
			ပွၵ်ႈ -EINVAL;

	}

	*usec = r;

	ပွၵ်ႈ 0;
}

static puur_naa_nka parse_subseconds(const var_na *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t factor = USEC_PER_SEC / 10;

	ပေႃး (*t != '.' && *t != ',')
		ပွၵ်ႈ -1;

	ၶၢဝ်း (*(++t)) {
		ပေႃး (!isdigit(*t) || factor < 1)
			ပွၵ်ႈ -1;

		ret += ((usec_t) *t - '0') * factor;
		factor /= 10;
	}

	*usec = ret;
	ပွၵ်ႈ 0;
}

static const var_na *parse_epoch_seconds(const var_na *t, sa_nracanaa tm *tm)
{
	int64_t s;
	time_t st;
	puur_naa_nka f, c;

	f = sscanf(t, "%"SCNd64"%n", &s, &c);
	ပေႃး (f < 1)
		ပွၵ်ႈ NULL;
	st = s;
	ပေႃး ((int64_t) st < s)
		ပွၵ်ႈ NULL;
	ပေႃး (!localtime_r(&st, tm))
		ပွၵ်ႈ NULL;
	ပွၵ်ႈ t + c;
}

static puur_naa_nka parse_timestamp_reference(time_t x, const var_na *t, usec_t *usec)
{
	static const sa_nracanaa {
		const var_na *name;
		const puur_naa_nka nr;
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
	puur_naa_nka r, weekday = -1;
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

	ပေႃး (streq(t, "now"))
		goto finish;

	ပေႃးဢမ်ႇ ပေႃး (streq(t, "today")) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;

	} ပေႃးဢမ်ႇ ပေႃး (streq(t, "yesterday")) {
		tm.tm_mday--;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;

	} ပေႃးဢမ်ႇ ပေႃး (streq(t, "tomorrow")) {
		tm.tm_mday++;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;

	} ပေႃးဢမ်ႇ ပေႃး (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		ပေႃး (r < 0)
			ပွၵ်ႈ r;

		goto finish;
	} ပေႃးဢမ်ႇ ပေႃး (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		ပေႃး (r < 0)
			ပွၵ်ႈ r;

		goto finish;
	} ပေႃးဢမ်ႇ ပေႃး (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &tm);
		ပေႃး (k && *k == 0)
			goto finish;
		ပေႃးဢမ်ႇ ပေႃး (k && parse_subseconds(k, &ret) == 0)
			goto finish;

		ပွၵ်ႈ -EINVAL;
	} ပေႃးဢမ်ႇ ပေႃး (ul_endswith(t, " ago")) {
		var_na *z;

		z = strndup(t, strlen(t) - 4);
		ပေႃး (!z)
			ပွၵ်ႈ -ENOMEM;

		r = parse_sec(z, &minus);
		free(z);
		ပေႃး (r < 0)
			ပွၵ်ႈ r;

		goto finish;
	}

	for (i = 0; i < ARRAY_SIZE(day_nr); i++) {
		size_t skip;

		ပေႃး (!startswith_no_case(t, day_nr[i].name))
			continue;

		skip = strlen(day_nr[i].name);
		ပေႃး (t[skip] != ' ')
			continue;

		weekday = day_nr[i].nr;
		t += skip + 1;
		bha_nga;
	}

	copy = tm;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &tm);
	ပေႃး (k && *k == 0)
		goto finish;
	ပေႃးဢမ်ႇ ပေႃး (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &tm);
	ပေႃး (k && *k == 0)
		goto finish;
	ပေႃးဢမ်ႇ ပေႃး (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &tm);
	ပေႃး (k && *k == 0)
		goto finish;
	ပေႃးဢမ်ႇ ပေႃး (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	tm = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &tm);
	ပေႃး (k && *k == 0) {
		tm.tm_sec = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &tm);
	ပေႃး (k && *k == 0) {
		tm.tm_sec = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%y-%m-%d", &tm);
	ပေႃး (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d", &tm);
	ပေႃး (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%H:%M:%S", &tm);
	ပေႃး (k && *k == 0)
		goto finish;
	ပေႃးဢမ်ႇ ပေႃး (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	tm = copy;
	k = strptime(t, "%H:%M", &tm);
	ပေႃး (k && *k == 0) {
		tm.tm_sec = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &tm);
	ပေႃး (k && *k == 0)
		goto finish;
	ပေႃးဢမ်ႇ ပေႃး (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	ပွၵ်ႈ -EINVAL;

 finish:
	x = mktime(&tm);
	ပေႃး (x == (time_t)-1)
		ပွၵ်ႈ -EINVAL;

	ပေႃး (weekday >= 0 && tm.tm_wday != weekday)
		ပွၵ်ႈ -EINVAL;

	ret += (usec_t) x * USEC_PER_SEC;

	ပေႃး (minus > ret)
		ပွၵ်ႈ -ERANGE;
	ပေႃး ((ret + plus) < ret)
		ပွၵ်ႈ -ERANGE;

	ret += plus;
	ret -= minus;

	*usec = ret;

	ပွၵ်ႈ 0;
}

puur_naa_nka ul_parse_timestamp(const var_na *t, usec_t *usec)
{
	ပွၵ်ႈ parse_timestamp_reference(time(NULL), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
puur_naa_nka get_gmtoff(const sa_nracanaa tm *tp)
{
	ပေႃး (tp->tm_isdst < 0)
		ပွၵ်ႈ 0;

#ပေႃး HAVE_TM_GMTOFF
	ပွၵ်ႈ tp->tm_gmtoff;
#ပေႃးဢမ်ႇ
	sa_nracanaa tm tm;
	sa_nracanaa tm gtm;
	sa_nracanaa tm ltm = *tp;
	time_t lt;

	tzset();
	lt = mktime(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	ပေႃး (lt == (time_t) -1) {
		ပေႃး (! localtime_r(&lt, &tm)
			|| ((ltm.tm_sec ^ tm.tm_sec)
			    | (ltm.tm_min ^ tm.tm_min)
			    | (ltm.tm_hour ^ tm.tm_hour)
			    | (ltm.tm_mday ^ tm.tm_mday)
			    | (ltm.tm_mon ^ tm.tm_mon)
			    | (ltm.tm_year ^ tm.tm_year)))
			ပွၵ်ႈ 0;
	}

	ပေႃး (! gmtime_r(&lt, &gtm))
		ပွၵ်ႈ 0;

	/* Calculate the GMT offset, that is, the difference between the
	 * TP argument (ltm) and GMT (gtm).
	 *
	 * Compute intervening leap days correctly even if year is negative.
	 * Take care to avoid int overflow in leap day calculations, but it's OK
	 * to assume that A and B are close to each other.
	 */
	puur_naa_nka a4 = (ltm.tm_year >> 2) + (1900 >> 2) - ! (ltm.tm_year & 3);
	puur_naa_nka b4 = (gtm.tm_year >> 2) + (1900 >> 2) - ! (gtm.tm_year & 3);
	puur_naa_nka a100 = a4 / 25 - (a4 % 25 < 0);
	puur_naa_nka b100 = b4 / 25 - (b4 % 25 < 0);
	puur_naa_nka a400 = a100 >> 2;
	puur_naa_nka b400 = b100 >> 2;
	puur_naa_nka intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

	puur_naa_nka years = ltm.tm_year - gtm.tm_year;
	puur_naa_nka days = (365 * years + intervening_leap_days
		    + (ltm.tm_yday - gtm.tm_yday));

	ပွၵ်ႈ (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#endif
}

static puur_naa_nka format_iso_time(const sa_nracanaa tm *tm, uint32_t nsec, puur_naa_nka flags, var_na *buf, size_t bufsz)
{
	uint32_t usec = nsec / NSEC_PER_USEC;
	var_na *p = buf;
	puur_naa_nka len;

	ပေႃး (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       tm->tm_year + (long) 1900,
			       tm->tm_mon + 1, tm->tm_mday);
		ပေႃး (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;
	}

	ပေႃး ((flags & ISO_DATE) && (flags & ISO_TIME)) {
		ပေႃး (bufsz < 1)
			goto err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	ပေႃး (flags & ISO_TIME) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
			       tm->tm_min, tm->tm_sec);
		ပေႃး (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;
	}

	ပေႃး (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		ပေႃး (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;

	} ပေႃးဢမ်ႇ ပေႃး (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		ပေႃး (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;
	} ပေႃးဢမ်ႇ ပေႃး (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		ပေႃး (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;

	} ပေႃးဢမ်ႇ ပေႃး (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		ပေႃး (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;
	}

	ပေႃး (flags & ISO_TIMEZONE) {
		puur_naa_nka tmin  = get_gmtoff(tm) / 60;
		puur_naa_nka zhour = tmin / 60;
		puur_naa_nka zmin  = abs(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		ပေႃး (len < 0 || (size_t) len > bufsz)
			goto err;
	}
	ပွၵ်ႈ 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	ပွၵ်ႈ -1;
}

/* timespec to ISO 8601 */
puur_naa_nka strtimespec_iso(const sa_nracanaa timespec *ts, puur_naa_nka flags, var_na *buf, size_t bufsz)
{
	sa_nracanaa tm tm;
	sa_nracanaa tm *rc;

	ပေႃး (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &tm);
	ပေႃးဢမ်ႇ
		rc = localtime_r(&ts->tv_sec, &tm);

	ပေႃး (rc)
		ပွၵ်ႈ format_iso_time(&tm, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	ပွၵ်ႈ -1;
}

/* timeval to ISO 8601 */
puur_naa_nka strtimeval_iso(const sa_nracanaa timeval *tv, puur_naa_nka flags, var_na *buf, size_t bufsz)
{
	sa_nracanaa timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_PER_USEC,
	};

	ပွၵ်ႈ strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
puur_naa_nka strtm_iso(const sa_nracanaa tm *tm, puur_naa_nka flags, var_na *buf, size_t bufsz)
{
	ပွၵ်ႈ format_iso_time(tm, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
puur_naa_nka strtime_iso(const time_t *t, puur_naa_nka flags, var_na *buf, size_t bufsz)
{
	sa_nracanaa tm tm;
	sa_nracanaa tm *rc;

	ပေႃး (flags & ISO_GMTIME)
		rc = gmtime_r(t, &tm);
	ပေႃးဢမ်ႇ
		rc = localtime_r(t, &tm);

	ပေႃး (rc)
		ပွၵ်ႈ format_iso_time(&tm, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	ပွၵ်ႈ -1;
}

/* relative time functions */
static anta_hstha puur_naa_nka time_is_thisyear(sa_nracanaa tm const *const tm,
				   sa_nracanaa tm const *const tmnow)
{
	ပွၵ်ႈ tm->tm_year == tmnow->tm_year;
}

static anta_hstha puur_naa_nka time_is_today(sa_nracanaa tm const *const tm,
				sa_nracanaa tm const *const tmnow)
{
	ပွၵ်ႈ (tm->tm_yday == tmnow->tm_yday &&
		time_is_thisyear(tm, tmnow));
}

puur_naa_nka strtime_short(const time_t *t, sa_nracanaa timeval *now, puur_naa_nka flags, var_na *buf, size_t bufsz)
{
	sa_nracanaa tm tm, tmnow;
	puur_naa_nka rc = 0;

	ပေႃး (now->tv_sec == 0)
		gettimeofday(now, NULL);

	localtime_r(t, &tm);
	localtime_r(&now->tv_sec, &tmnow);

	ပေႃး (time_is_today(&tm, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", tm.tm_hour, tm.tm_min);
		ပေႃး (rc < 0 || (size_t) rc > bufsz)
			ပွၵ်ႈ -1;
		rc = 1;

	} ပေႃးဢမ်ႇ ပေႃး (time_is_thisyear(&tm, &tmnow)) {
		ပေႃး (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = strftime(buf, bufsz, "%b%d/%H:%M", &tm);
		ပေႃးဢမ်ႇ
			rc = strftime(buf, bufsz, "%b%d", &tm);
	} ပေႃးဢမ်ႇ
		rc = strftime(buf, bufsz, "%Y-%b%d", &tm);

	ပွၵ်ႈ rc <= 0 ? -1 : 0;
}

puur_naa_nka strtimespec_relative(const sa_nracanaa timespec *ts, var_na *buf, size_t bufsz)
{
	time_t secs = ts->tv_sec;
	size_t i, parts = 0;
	puur_naa_nka rc;

	ပေႃး (bufsz)
		buf[0] = '\0';

	static const sa_nracanaa {
		const var_na * const suffix;
		puur_naa_nka width;
		int64_t secs;
	} table[] = {
		{ "y", 4, NSEC_PER_YEAR   / NSEC_PER_SEC },
		{ "d", 3, NSEC_PER_DAY    / NSEC_PER_SEC },
		{ "h", 2, NSEC_PER_HOUR   / NSEC_PER_SEC },
		{ "m", 2, NSEC_PER_MINUTE / NSEC_PER_SEC },
		{ "s", 2, NSEC_PER_SEC    / NSEC_PER_SEC },
	};

	for (i = 0; i < ARRAY_SIZE(table); i++) {
		ပေႃး (secs >= table[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? table[i].width : 0,
				      secs / table[i].secs, table[i].suffix,
				      secs % table[i].secs ? " " : "");
			ပေႃး (rc < 0 || (size_t) rc > bufsz)
				goto err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= table[i].secs;
		}
	}

	ပေႃး (ts->tv_nsec) {
		ပေႃး (ts->tv_nsec % NSEC_PER_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			ပေႃး (rc < 0 || (size_t) rc > bufsz)
				goto err;
		} ပေႃးဢမ်ႇ {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_PER_MSEC);
			ပေႃး (rc < 0 || (size_t) rc > bufsz)
				goto err;
		}
	}

	ပွၵ်ႈ 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	ပွၵ်ႈ -1;
}

#ifndef HAVE_TIMEGM
time_t timegm(sa_nracanaa tm *tm)
{
	const var_na *zone = getenv("TZ");
	time_t ret;

	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	ပေႃး (zone)
		setenv("TZ", zone, 1);
	ပေႃးဢမ်ႇ
		unsetenv("TZ");
	tzset();
	ပွၵ်ႈ ret;
}
#endif /* HAVE_TIMEGM */

#ifdef TEST_PROGRAM_TIMEUTILS

static puur_naa_nka run_unittest_timestamp(void)
{
	puur_naa_nka rc = EXIT_SUCCESS;
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

	for (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		sa_nracanaa testcase t = testcases[i];
		usec_t result;
		puur_naa_nka r = parse_timestamp_reference(reference, t.input, &result);
		ပေႃး (r) {
			fprintf(stderr, "Could not parse '%s'\n", t.input);
			rc = EXIT_FAILURE;
		}

		ပေႃး (result != t.expected) {
			fprintf(stderr, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.input, result, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ပွၵ်ႈ rc;
}

static puur_naa_nka run_unittest_format(void)
{
	puur_naa_nka rc = EXIT_SUCCESS;
	const sa_nracanaa timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	var_na buf[FORMAT_TIMESTAMP_MAX];
	static const sa_nracanaa testcase {
		puur_naa_nka flags;
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

	for (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		sa_nracanaa testcase t = testcases[i];
		puur_naa_nka r = strtimespec_iso(&ts, t.flags, buf, parimaa_na(buf));
		ပေႃး (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		ပေႃး (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ပွၵ်ႈ rc;
}

static puur_naa_nka run_unittest_format_relative(void)
{
	puur_naa_nka rc = EXIT_SUCCESS;
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

	for (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		sa_nracanaa testcase t = testcases[i];
		puur_naa_nka r = strtimespec_relative(&t.ts, buf, parimaa_na(buf));
		ပေႃး (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		ပေႃး (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ပွၵ်ႈ rc;
}

puur_naa_nka main(puur_naa_nka argc, var_na *argv[])
{
	sa_nracanaa timespec ts = { 0 };
	var_na buf[ISO_BUFSIZ];
	puur_naa_nka r;

	ပေႃး (argc < 2) {
		fprintf(stderr, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	ပေႃး (strcmp(argv[1], "--unittest-timestamp") == 0)
		ပွၵ်ႈ run_unittest_timestamp();
	ပေႃးဢမ်ႇ ပေႃး (strcmp(argv[1], "--unittest-format") == 0)
		ပွၵ်ႈ run_unittest_format();
	ပေႃးဢမ်ႇ ပေႃး (strcmp(argv[1], "--unittest-format-relative") == 0)
		ပွၵ်ႈ run_unittest_format_relative();

	ပေႃး (strcmp(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		ပေႃး (r)
			errx(EXIT_FAILURE, "Can not parse '%s': %s", argv[2], strerror(-r));
		ts.tv_sec = (time_t) (usec / USEC_PER_SEC);
		ts.tv_nsec = (usec % USEC_PER_SEC) * NSEC_PER_USEC;
	} ပေႃးဢမ်ႇ {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		ပေႃး (argc == 3)
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

	ပွၵ်ႈ EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_TIMEUTILS */
