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

static ចំនួនគត់ parse_sec(const var_na *t, usec_t *usec)
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
	ចំនួនគត់ something = FALSE;

	assert(t);
	assert(usec);

	p = t;
	សម្រាប់ (;;) {
		long long l, z = 0;
		var_na *e;
		unsigned i, n = 0;

		p += strspn(p, WHITESPACE);

		ប្រសិនបើ (*p == 0) {
			ប្រសិនបើ (!something)
				ត្រឡប់ -EINVAL;

			bha_nga;
		}

		errno = 0;
		l = strtoll(p, &e, 10);

		ប្រសិនបើ (errno > 0)
			ត្រឡប់ -errno;

		ប្រសិនបើ (l < 0)
			ត្រឡប់ -ERANGE;

		ប្រសិនបើ (*e == '.') {
			var_na *b = e + 1;

			errno = 0;
			z = strtoll(b, &e, 10);
			ប្រសិនបើ (errno > 0)
				ត្រឡប់ -errno;

			ប្រសិនបើ (z < 0)
				ត្រឡប់ -ERANGE;

			ប្រសិនបើ (e == b)
				ត្រឡប់ -EINVAL;

			n = e - b;

		} បើមិនដូច្នេះ ប្រសិនបើ (e == p)
			ត្រឡប់ -EINVAL;

		e += strspn(e, WHITESPACE);

		សម្រាប់ (i = 0; i < ARRAY_SIZE(table); i++)
			ប្រសិនបើ (ul_startswith(e, table[i].suffix)) {
				usec_t k = (usec_t) z * table[i].usec;

				សម្រាប់ (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *table[i].usec + k;
				p = e + strlen(table[i].suffix);

				something = TRUE;
				bha_nga;
			}

		ប្រសិនបើ (i >= ARRAY_SIZE(table))
			ត្រឡប់ -EINVAL;

	}

	*usec = r;

	ត្រឡប់ 0;
}

static ចំនួនគត់ parse_subseconds(const var_na *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t factor = USEC_PER_SEC / 10;

	ប្រសិនបើ (*t != '.' && *t != ',')
		ត្រឡប់ -1;

	ខណៈពេល (*(++t)) {
		ប្រសិនបើ (!isdigit(*t) || factor < 1)
			ត្រឡប់ -1;

		ret += ((usec_t) *t - '0') * factor;
		factor /= 10;
	}

	*usec = ret;
	ត្រឡប់ 0;
}

static const var_na *parse_epoch_seconds(const var_na *t, sa_nracanaa tm *tm)
{
	int64_t s;
	time_t st;
	ចំនួនគត់ f, c;

	f = sscanf(t, "%"SCNd64"%n", &s, &c);
	ប្រសិនបើ (f < 1)
		ត្រឡប់ NULL;
	st = s;
	ប្រសិនបើ ((int64_t) st < s)
		ត្រឡប់ NULL;
	ប្រសិនបើ (!localtime_r(&st, tm))
		ត្រឡប់ NULL;
	ត្រឡប់ t + c;
}

static ចំនួនគត់ parse_timestamp_reference(time_t x, const var_na *t, usec_t *usec)
{
	static const sa_nracanaa {
		const var_na *name;
		const ចំនួនគត់ nr;
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
	ចំនួនគត់ r, weekday = -1;
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

	ប្រសិនបើ (streq(t, "now"))
		goto finish;

	បើមិនដូច្នេះ ប្រសិនបើ (streq(t, "today")) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;

	} បើមិនដូច្នេះ ប្រសិនបើ (streq(t, "yesterday")) {
		tm.tm_mday--;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;

	} បើមិនដូច្នេះ ប្រសិនបើ (streq(t, "tomorrow")) {
		tm.tm_mday++;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;

	} បើមិនដូច្នេះ ប្រសិនបើ (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		ប្រសិនបើ (r < 0)
			ត្រឡប់ r;

		goto finish;
	} បើមិនដូច្នេះ ប្រសិនបើ (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		ប្រសិនបើ (r < 0)
			ត្រឡប់ r;

		goto finish;
	} បើមិនដូច្នេះ ប្រសិនបើ (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &tm);
		ប្រសិនបើ (k && *k == 0)
			goto finish;
		បើមិនដូច្នេះ ប្រសិនបើ (k && parse_subseconds(k, &ret) == 0)
			goto finish;

		ត្រឡប់ -EINVAL;
	} បើមិនដូច្នេះ ប្រសិនបើ (ul_endswith(t, " ago")) {
		var_na *z;

		z = strndup(t, strlen(t) - 4);
		ប្រសិនបើ (!z)
			ត្រឡប់ -ENOMEM;

		r = parse_sec(z, &minus);
		free(z);
		ប្រសិនបើ (r < 0)
			ត្រឡប់ r;

		goto finish;
	}

	សម្រាប់ (i = 0; i < ARRAY_SIZE(day_nr); i++) {
		size_t skip;

		ប្រសិនបើ (!startswith_no_case(t, day_nr[i].name))
			continue;

		skip = strlen(day_nr[i].name);
		ប្រសិនបើ (t[skip] != ' ')
			continue;

		weekday = day_nr[i].nr;
		t += skip + 1;
		bha_nga;
	}

	copy = tm;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &tm);
	ប្រសិនបើ (k && *k == 0)
		goto finish;
	បើមិនដូច្នេះ ប្រសិនបើ (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &tm);
	ប្រសិនបើ (k && *k == 0)
		goto finish;
	បើមិនដូច្នេះ ប្រសិនបើ (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &tm);
	ប្រសិនបើ (k && *k == 0)
		goto finish;
	បើមិនដូច្នេះ ប្រសិនបើ (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	tm = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &tm);
	ប្រសិនបើ (k && *k == 0) {
		tm.tm_sec = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &tm);
	ប្រសិនបើ (k && *k == 0) {
		tm.tm_sec = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%y-%m-%d", &tm);
	ប្រសិនបើ (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d", &tm);
	ប្រសិនបើ (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%H:%M:%S", &tm);
	ប្រសិនបើ (k && *k == 0)
		goto finish;
	បើមិនដូច្នេះ ប្រសិនបើ (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	tm = copy;
	k = strptime(t, "%H:%M", &tm);
	ប្រសិនបើ (k && *k == 0) {
		tm.tm_sec = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &tm);
	ប្រសិនបើ (k && *k == 0)
		goto finish;
	បើមិនដូច្នេះ ប្រសិនបើ (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	ត្រឡប់ -EINVAL;

 finish:
	x = mktime(&tm);
	ប្រសិនបើ (x == (time_t)-1)
		ត្រឡប់ -EINVAL;

	ប្រសិនបើ (weekday >= 0 && tm.tm_wday != weekday)
		ត្រឡប់ -EINVAL;

	ret += (usec_t) x * USEC_PER_SEC;

	ប្រសិនបើ (minus > ret)
		ត្រឡប់ -ERANGE;
	ប្រសិនបើ ((ret + plus) < ret)
		ត្រឡប់ -ERANGE;

	ret += plus;
	ret -= minus;

	*usec = ret;

	ត្រឡប់ 0;
}

ចំនួនគត់ ul_parse_timestamp(const var_na *t, usec_t *usec)
{
	ត្រឡប់ parse_timestamp_reference(time(NULL), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
ចំនួនគត់ get_gmtoff(const sa_nracanaa tm *tp)
{
	ប្រសិនបើ (tp->tm_isdst < 0)
		ត្រឡប់ 0;

#ប្រសិនបើ HAVE_TM_GMTOFF
	ត្រឡប់ tp->tm_gmtoff;
#បើមិនដូច្នេះ
	sa_nracanaa tm tm;
	sa_nracanaa tm gtm;
	sa_nracanaa tm ltm = *tp;
	time_t lt;

	tzset();
	lt = mktime(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	ប្រសិនបើ (lt == (time_t) -1) {
		ប្រសិនបើ (! localtime_r(&lt, &tm)
			|| ((ltm.tm_sec ^ tm.tm_sec)
			    | (ltm.tm_min ^ tm.tm_min)
			    | (ltm.tm_hour ^ tm.tm_hour)
			    | (ltm.tm_mday ^ tm.tm_mday)
			    | (ltm.tm_mon ^ tm.tm_mon)
			    | (ltm.tm_year ^ tm.tm_year)))
			ត្រឡប់ 0;
	}

	ប្រសិនបើ (! gmtime_r(&lt, &gtm))
		ត្រឡប់ 0;

	/* Calculate the GMT offset, that is, the difference between the
	 * TP argument (ltm) and GMT (gtm).
	 *
	 * Compute intervening leap days correctly even if year is negative.
	 * Take care to avoid int overflow in leap day calculations, but it's OK
	 * to assume that A and B are close to each other.
	 */
	ចំនួនគត់ a4 = (ltm.tm_year >> 2) + (1900 >> 2) - ! (ltm.tm_year & 3);
	ចំនួនគត់ b4 = (gtm.tm_year >> 2) + (1900 >> 2) - ! (gtm.tm_year & 3);
	ចំនួនគត់ a100 = a4 / 25 - (a4 % 25 < 0);
	ចំនួនគត់ b100 = b4 / 25 - (b4 % 25 < 0);
	ចំនួនគត់ a400 = a100 >> 2;
	ចំនួនគត់ b400 = b100 >> 2;
	ចំនួនគត់ intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

	ចំនួនគត់ years = ltm.tm_year - gtm.tm_year;
	ចំនួនគត់ days = (365 * years + intervening_leap_days
		    + (ltm.tm_yday - gtm.tm_yday));

	ត្រឡប់ (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#endif
}

static ចំនួនគត់ format_iso_time(const sa_nracanaa tm *tm, uint32_t nsec, ចំនួនគត់ flags, var_na *buf, size_t bufsz)
{
	uint32_t usec = nsec / NSEC_PER_USEC;
	var_na *p = buf;
	ចំនួនគត់ len;

	ប្រសិនបើ (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       tm->tm_year + (long) 1900,
			       tm->tm_mon + 1, tm->tm_mday);
		ប្រសិនបើ (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;
	}

	ប្រសិនបើ ((flags & ISO_DATE) && (flags & ISO_TIME)) {
		ប្រសិនបើ (bufsz < 1)
			goto err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	ប្រសិនបើ (flags & ISO_TIME) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
			       tm->tm_min, tm->tm_sec);
		ប្រសិនបើ (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;
	}

	ប្រសិនបើ (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		ប្រសិនបើ (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;

	} បើមិនដូច្នេះ ប្រសិនបើ (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		ប្រសិនបើ (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;
	} បើមិនដូច្នេះ ប្រសិនបើ (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		ប្រសិនបើ (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;

	} បើមិនដូច្នេះ ប្រសិនបើ (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		ប្រសិនបើ (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;
	}

	ប្រសិនបើ (flags & ISO_TIMEZONE) {
		ចំនួនគត់ tmin  = get_gmtoff(tm) / 60;
		ចំនួនគត់ zhour = tmin / 60;
		ចំនួនគត់ zmin  = abs(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		ប្រសិនបើ (len < 0 || (size_t) len > bufsz)
			goto err;
	}
	ត្រឡប់ 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	ត្រឡប់ -1;
}

/* timespec to ISO 8601 */
ចំនួនគត់ strtimespec_iso(const sa_nracanaa timespec *ts, ចំនួនគត់ flags, var_na *buf, size_t bufsz)
{
	sa_nracanaa tm tm;
	sa_nracanaa tm *rc;

	ប្រសិនបើ (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &tm);
	បើមិនដូច្នេះ
		rc = localtime_r(&ts->tv_sec, &tm);

	ប្រសិនបើ (rc)
		ត្រឡប់ format_iso_time(&tm, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	ត្រឡប់ -1;
}

/* timeval to ISO 8601 */
ចំនួនគត់ strtimeval_iso(const sa_nracanaa timeval *tv, ចំនួនគត់ flags, var_na *buf, size_t bufsz)
{
	sa_nracanaa timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_PER_USEC,
	};

	ត្រឡប់ strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
ចំនួនគត់ strtm_iso(const sa_nracanaa tm *tm, ចំនួនគត់ flags, var_na *buf, size_t bufsz)
{
	ត្រឡប់ format_iso_time(tm, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
ចំនួនគត់ strtime_iso(const time_t *t, ចំនួនគត់ flags, var_na *buf, size_t bufsz)
{
	sa_nracanaa tm tm;
	sa_nracanaa tm *rc;

	ប្រសិនបើ (flags & ISO_GMTIME)
		rc = gmtime_r(t, &tm);
	បើមិនដូច្នេះ
		rc = localtime_r(t, &tm);

	ប្រសិនបើ (rc)
		ត្រឡប់ format_iso_time(&tm, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	ត្រឡប់ -1;
}

/* relative time functions */
static anta_hstha ចំនួនគត់ time_is_thisyear(sa_nracanaa tm const *const tm,
				   sa_nracanaa tm const *const tmnow)
{
	ត្រឡប់ tm->tm_year == tmnow->tm_year;
}

static anta_hstha ចំនួនគត់ time_is_today(sa_nracanaa tm const *const tm,
				sa_nracanaa tm const *const tmnow)
{
	ត្រឡប់ (tm->tm_yday == tmnow->tm_yday &&
		time_is_thisyear(tm, tmnow));
}

ចំនួនគត់ strtime_short(const time_t *t, sa_nracanaa timeval *now, ចំនួនគត់ flags, var_na *buf, size_t bufsz)
{
	sa_nracanaa tm tm, tmnow;
	ចំនួនគត់ rc = 0;

	ប្រសិនបើ (now->tv_sec == 0)
		gettimeofday(now, NULL);

	localtime_r(t, &tm);
	localtime_r(&now->tv_sec, &tmnow);

	ប្រសិនបើ (time_is_today(&tm, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", tm.tm_hour, tm.tm_min);
		ប្រសិនបើ (rc < 0 || (size_t) rc > bufsz)
			ត្រឡប់ -1;
		rc = 1;

	} បើមិនដូច្នេះ ប្រសិនបើ (time_is_thisyear(&tm, &tmnow)) {
		ប្រសិនបើ (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = strftime(buf, bufsz, "%b%d/%H:%M", &tm);
		បើមិនដូច្នេះ
			rc = strftime(buf, bufsz, "%b%d", &tm);
	} បើមិនដូច្នេះ
		rc = strftime(buf, bufsz, "%Y-%b%d", &tm);

	ត្រឡប់ rc <= 0 ? -1 : 0;
}

ចំនួនគត់ strtimespec_relative(const sa_nracanaa timespec *ts, var_na *buf, size_t bufsz)
{
	time_t secs = ts->tv_sec;
	size_t i, parts = 0;
	ចំនួនគត់ rc;

	ប្រសិនបើ (bufsz)
		buf[0] = '\0';

	static const sa_nracanaa {
		const var_na * const suffix;
		ចំនួនគត់ width;
		int64_t secs;
	} table[] = {
		{ "y", 4, NSEC_PER_YEAR   / NSEC_PER_SEC },
		{ "d", 3, NSEC_PER_DAY    / NSEC_PER_SEC },
		{ "h", 2, NSEC_PER_HOUR   / NSEC_PER_SEC },
		{ "m", 2, NSEC_PER_MINUTE / NSEC_PER_SEC },
		{ "s", 2, NSEC_PER_SEC    / NSEC_PER_SEC },
	};

	សម្រាប់ (i = 0; i < ARRAY_SIZE(table); i++) {
		ប្រសិនបើ (secs >= table[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? table[i].width : 0,
				      secs / table[i].secs, table[i].suffix,
				      secs % table[i].secs ? " " : "");
			ប្រសិនបើ (rc < 0 || (size_t) rc > bufsz)
				goto err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= table[i].secs;
		}
	}

	ប្រសិនបើ (ts->tv_nsec) {
		ប្រសិនបើ (ts->tv_nsec % NSEC_PER_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			ប្រសិនបើ (rc < 0 || (size_t) rc > bufsz)
				goto err;
		} បើមិនដូច្នេះ {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_PER_MSEC);
			ប្រសិនបើ (rc < 0 || (size_t) rc > bufsz)
				goto err;
		}
	}

	ត្រឡប់ 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	ត្រឡប់ -1;
}

#ifndef HAVE_TIMEGM
time_t timegm(sa_nracanaa tm *tm)
{
	const var_na *zone = getenv("TZ");
	time_t ret;

	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	ប្រសិនបើ (zone)
		setenv("TZ", zone, 1);
	បើមិនដូច្នេះ
		unsetenv("TZ");
	tzset();
	ត្រឡប់ ret;
}
#endif /* HAVE_TIMEGM */

#ifdef TEST_PROGRAM_TIMEUTILS

static ចំនួនគត់ run_unittest_timestamp(void)
{
	ចំនួនគត់ rc = EXIT_SUCCESS;
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

	សម្រាប់ (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		sa_nracanaa testcase t = testcases[i];
		usec_t result;
		ចំនួនគត់ r = parse_timestamp_reference(reference, t.input, &result);
		ប្រសិនបើ (r) {
			fprintf(stderr, "Could not parse '%s'\n", t.input);
			rc = EXIT_FAILURE;
		}

		ប្រសិនបើ (result != t.expected) {
			fprintf(stderr, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.input, result, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ត្រឡប់ rc;
}

static ចំនួនគត់ run_unittest_format(void)
{
	ចំនួនគត់ rc = EXIT_SUCCESS;
	const sa_nracanaa timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	var_na buf[FORMAT_TIMESTAMP_MAX];
	static const sa_nracanaa testcase {
		ចំនួនគត់ flags;
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

	សម្រាប់ (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		sa_nracanaa testcase t = testcases[i];
		ចំនួនគត់ r = strtimespec_iso(&ts, t.flags, buf, parimaa_na(buf));
		ប្រសិនបើ (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		ប្រសិនបើ (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ត្រឡប់ rc;
}

static ចំនួនគត់ run_unittest_format_relative(void)
{
	ចំនួនគត់ rc = EXIT_SUCCESS;
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

	សម្រាប់ (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		sa_nracanaa testcase t = testcases[i];
		ចំនួនគត់ r = strtimespec_relative(&t.ts, buf, parimaa_na(buf));
		ប្រសិនបើ (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		ប្រសិនបើ (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ត្រឡប់ rc;
}

ចំនួនគត់ main(ចំនួនគត់ argc, var_na *argv[])
{
	sa_nracanaa timespec ts = { 0 };
	var_na buf[ISO_BUFSIZ];
	ចំនួនគត់ r;

	ប្រសិនបើ (argc < 2) {
		fprintf(stderr, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	ប្រសិនបើ (strcmp(argv[1], "--unittest-timestamp") == 0)
		ត្រឡប់ run_unittest_timestamp();
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(argv[1], "--unittest-format") == 0)
		ត្រឡប់ run_unittest_format();
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(argv[1], "--unittest-format-relative") == 0)
		ត្រឡប់ run_unittest_format_relative();

	ប្រសិនបើ (strcmp(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		ប្រសិនបើ (r)
			errx(EXIT_FAILURE, "Can not parse '%s': %s", argv[2], strerror(-r));
		ts.tv_sec = (time_t) (usec / USEC_PER_SEC);
		ts.tv_nsec = (usec % USEC_PER_SEC) * NSEC_PER_USEC;
	} បើមិនដូច្នេះ {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		ប្រសិនបើ (argc == 3)
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

	ត្រឡប់ EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_TIMEUTILS */
