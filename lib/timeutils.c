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

static ཧྲིལ parse_sec(const var_na *t, usec_t *usec)
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
	ཧྲིལ something = FALSE;

	assert(t);
	assert(usec);

	p = t;
	དོན་ལུ (;;) {
		long long l, z = 0;
		var_na *e;
		unsigned i, n = 0;

		p += strspn(p, WHITESPACE);

		གལ་སྲིད (*p == 0) {
			གལ་སྲིད (!something)
				ལོག -EINVAL;

			bha_nga;
		}

		errno = 0;
		l = strtoll(p, &e, 10);

		གལ་སྲིད (errno > 0)
			ལོག -errno;

		གལ་སྲིད (l < 0)
			ལོག -ERANGE;

		གལ་སྲིད (*e == '.') {
			var_na *b = e + 1;

			errno = 0;
			z = strtoll(b, &e, 10);
			གལ་སྲིད (errno > 0)
				ལོག -errno;

			གལ་སྲིད (z < 0)
				ལོག -ERANGE;

			གལ་སྲིད (e == b)
				ལོག -EINVAL;

			n = e - b;

		} དེ་མིན གལ་སྲིད (e == p)
			ལོག -EINVAL;

		e += strspn(e, WHITESPACE);

		དོན་ལུ (i = 0; i < ARRAY_SIZE(table); i++)
			གལ་སྲིད (ul_startswith(e, table[i].suffix)) {
				usec_t k = (usec_t) z * table[i].usec;

				དོན་ལུ (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *table[i].usec + k;
				p = e + strlen(table[i].suffix);

				something = TRUE;
				bha_nga;
			}

		གལ་སྲིད (i >= ARRAY_SIZE(table))
			ལོག -EINVAL;

	}

	*usec = r;

	ལོག 0;
}

static ཧྲིལ parse_subseconds(const var_na *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t factor = USEC_PER_SEC / 10;

	གལ་སྲིད (*t != '.' && *t != ',')
		ལོག -1;

	སྐབས (*(++t)) {
		གལ་སྲིད (!isdigit(*t) || factor < 1)
			ལོག -1;

		ret += ((usec_t) *t - '0') * factor;
		factor /= 10;
	}

	*usec = ret;
	ལོག 0;
}

static const var_na *parse_epoch_seconds(const var_na *t, sa_nracanaa tm *tm)
{
	int64_t s;
	time_t st;
	ཧྲིལ f, c;

	f = sscanf(t, "%"SCNd64"%n", &s, &c);
	གལ་སྲིད (f < 1)
		ལོག NULL;
	st = s;
	གལ་སྲིད ((int64_t) st < s)
		ལོག NULL;
	གལ་སྲིད (!localtime_r(&st, tm))
		ལོག NULL;
	ལོག t + c;
}

static ཧྲིལ parse_timestamp_reference(time_t x, const var_na *t, usec_t *usec)
{
	static const sa_nracanaa {
		const var_na *name;
		const ཧྲིལ nr;
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
	ཧྲིལ r, weekday = -1;
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

	གལ་སྲིད (streq(t, "now"))
		goto finish;

	དེ་མིན གལ་སྲིད (streq(t, "today")) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;

	} དེ་མིན གལ་སྲིད (streq(t, "yesterday")) {
		tm.tm_mday--;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;

	} དེ་མིན གལ་སྲིད (streq(t, "tomorrow")) {
		tm.tm_mday++;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;

	} དེ་མིན གལ་སྲིད (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		གལ་སྲིད (r < 0)
			ལོག r;

		goto finish;
	} དེ་མིན གལ་སྲིད (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		གལ་སྲིད (r < 0)
			ལོག r;

		goto finish;
	} དེ་མིན གལ་སྲིད (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &tm);
		གལ་སྲིད (k && *k == 0)
			goto finish;
		དེ་མིན གལ་སྲིད (k && parse_subseconds(k, &ret) == 0)
			goto finish;

		ལོག -EINVAL;
	} དེ་མིན གལ་སྲིད (ul_endswith(t, " ago")) {
		var_na *z;

		z = strndup(t, strlen(t) - 4);
		གལ་སྲིད (!z)
			ལོག -ENOMEM;

		r = parse_sec(z, &minus);
		free(z);
		གལ་སྲིད (r < 0)
			ལོག r;

		goto finish;
	}

	དོན་ལུ (i = 0; i < ARRAY_SIZE(day_nr); i++) {
		size_t skip;

		གལ་སྲིད (!startswith_no_case(t, day_nr[i].name))
			continue;

		skip = strlen(day_nr[i].name);
		གལ་སྲིད (t[skip] != ' ')
			continue;

		weekday = day_nr[i].nr;
		t += skip + 1;
		bha_nga;
	}

	copy = tm;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &tm);
	གལ་སྲིད (k && *k == 0)
		goto finish;
	དེ་མིན གལ་སྲིད (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &tm);
	གལ་སྲིད (k && *k == 0)
		goto finish;
	དེ་མིན གལ་སྲིད (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &tm);
	གལ་སྲིད (k && *k == 0)
		goto finish;
	དེ་མིན གལ་སྲིད (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	tm = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &tm);
	གལ་སྲིད (k && *k == 0) {
		tm.tm_sec = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &tm);
	གལ་སྲིད (k && *k == 0) {
		tm.tm_sec = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%y-%m-%d", &tm);
	གལ་སྲིད (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d", &tm);
	གལ་སྲིད (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%H:%M:%S", &tm);
	གལ་སྲིད (k && *k == 0)
		goto finish;
	དེ་མིན གལ་སྲིད (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	tm = copy;
	k = strptime(t, "%H:%M", &tm);
	གལ་སྲིད (k && *k == 0) {
		tm.tm_sec = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &tm);
	གལ་སྲིད (k && *k == 0)
		goto finish;
	དེ་མིན གལ་སྲིད (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	ལོག -EINVAL;

 finish:
	x = mktime(&tm);
	གལ་སྲིད (x == (time_t)-1)
		ལོག -EINVAL;

	གལ་སྲིད (weekday >= 0 && tm.tm_wday != weekday)
		ལོག -EINVAL;

	ret += (usec_t) x * USEC_PER_SEC;

	གལ་སྲིད (minus > ret)
		ལོག -ERANGE;
	གལ་སྲིད ((ret + plus) < ret)
		ལོག -ERANGE;

	ret += plus;
	ret -= minus;

	*usec = ret;

	ལོག 0;
}

ཧྲིལ ul_parse_timestamp(const var_na *t, usec_t *usec)
{
	ལོག parse_timestamp_reference(time(NULL), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
ཧྲིལ get_gmtoff(const sa_nracanaa tm *tp)
{
	གལ་སྲིད (tp->tm_isdst < 0)
		ལོག 0;

#གལ་སྲིད HAVE_TM_GMTOFF
	ལོག tp->tm_gmtoff;
#དེ་མིན
	sa_nracanaa tm tm;
	sa_nracanaa tm gtm;
	sa_nracanaa tm ltm = *tp;
	time_t lt;

	tzset();
	lt = mktime(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	གལ་སྲིད (lt == (time_t) -1) {
		གལ་སྲིད (! localtime_r(&lt, &tm)
			|| ((ltm.tm_sec ^ tm.tm_sec)
			    | (ltm.tm_min ^ tm.tm_min)
			    | (ltm.tm_hour ^ tm.tm_hour)
			    | (ltm.tm_mday ^ tm.tm_mday)
			    | (ltm.tm_mon ^ tm.tm_mon)
			    | (ltm.tm_year ^ tm.tm_year)))
			ལོག 0;
	}

	གལ་སྲིད (! gmtime_r(&lt, &gtm))
		ལོག 0;

	/* Calculate the GMT offset, that is, the difference between the
	 * TP argument (ltm) and GMT (gtm).
	 *
	 * Compute intervening leap days correctly even if year is negative.
	 * Take care to avoid int overflow in leap day calculations, but it's OK
	 * to assume that A and B are close to each other.
	 */
	ཧྲིལ a4 = (ltm.tm_year >> 2) + (1900 >> 2) - ! (ltm.tm_year & 3);
	ཧྲིལ b4 = (gtm.tm_year >> 2) + (1900 >> 2) - ! (gtm.tm_year & 3);
	ཧྲིལ a100 = a4 / 25 - (a4 % 25 < 0);
	ཧྲིལ b100 = b4 / 25 - (b4 % 25 < 0);
	ཧྲིལ a400 = a100 >> 2;
	ཧྲིལ b400 = b100 >> 2;
	ཧྲིལ intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

	ཧྲིལ years = ltm.tm_year - gtm.tm_year;
	ཧྲིལ days = (365 * years + intervening_leap_days
		    + (ltm.tm_yday - gtm.tm_yday));

	ལོག (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#endif
}

static ཧྲིལ format_iso_time(const sa_nracanaa tm *tm, uint32_t nsec, ཧྲིལ flags, var_na *buf, size_t bufsz)
{
	uint32_t usec = nsec / NSEC_PER_USEC;
	var_na *p = buf;
	ཧྲིལ len;

	གལ་སྲིད (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       tm->tm_year + (long) 1900,
			       tm->tm_mon + 1, tm->tm_mday);
		གལ་སྲིད (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;
	}

	གལ་སྲིད ((flags & ISO_DATE) && (flags & ISO_TIME)) {
		གལ་སྲིད (bufsz < 1)
			goto err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	གལ་སྲིད (flags & ISO_TIME) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
			       tm->tm_min, tm->tm_sec);
		གལ་སྲིད (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;
	}

	གལ་སྲིད (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		གལ་སྲིད (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;

	} དེ་མིན གལ་སྲིད (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		གལ་སྲིད (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;
	} དེ་མིན གལ་སྲིད (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		གལ་སྲིད (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;

	} དེ་མིན གལ་སྲིད (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		གལ་སྲིད (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;
	}

	གལ་སྲིད (flags & ISO_TIMEZONE) {
		ཧྲིལ tmin  = get_gmtoff(tm) / 60;
		ཧྲིལ zhour = tmin / 60;
		ཧྲིལ zmin  = abs(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		གལ་སྲིད (len < 0 || (size_t) len > bufsz)
			goto err;
	}
	ལོག 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	ལོག -1;
}

/* timespec to ISO 8601 */
ཧྲིལ strtimespec_iso(const sa_nracanaa timespec *ts, ཧྲིལ flags, var_na *buf, size_t bufsz)
{
	sa_nracanaa tm tm;
	sa_nracanaa tm *rc;

	གལ་སྲིད (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &tm);
	དེ་མིན
		rc = localtime_r(&ts->tv_sec, &tm);

	གལ་སྲིད (rc)
		ལོག format_iso_time(&tm, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	ལོག -1;
}

/* timeval to ISO 8601 */
ཧྲིལ strtimeval_iso(const sa_nracanaa timeval *tv, ཧྲིལ flags, var_na *buf, size_t bufsz)
{
	sa_nracanaa timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_PER_USEC,
	};

	ལོག strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
ཧྲིལ strtm_iso(const sa_nracanaa tm *tm, ཧྲིལ flags, var_na *buf, size_t bufsz)
{
	ལོག format_iso_time(tm, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
ཧྲིལ strtime_iso(const time_t *t, ཧྲིལ flags, var_na *buf, size_t bufsz)
{
	sa_nracanaa tm tm;
	sa_nracanaa tm *rc;

	གལ་སྲིད (flags & ISO_GMTIME)
		rc = gmtime_r(t, &tm);
	དེ་མིན
		rc = localtime_r(t, &tm);

	གལ་སྲིད (rc)
		ལོག format_iso_time(&tm, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	ལོག -1;
}

/* relative time functions */
static anta_hstha ཧྲིལ time_is_thisyear(sa_nracanaa tm const *const tm,
				   sa_nracanaa tm const *const tmnow)
{
	ལོག tm->tm_year == tmnow->tm_year;
}

static anta_hstha ཧྲིལ time_is_today(sa_nracanaa tm const *const tm,
				sa_nracanaa tm const *const tmnow)
{
	ལོག (tm->tm_yday == tmnow->tm_yday &&
		time_is_thisyear(tm, tmnow));
}

ཧྲིལ strtime_short(const time_t *t, sa_nracanaa timeval *now, ཧྲིལ flags, var_na *buf, size_t bufsz)
{
	sa_nracanaa tm tm, tmnow;
	ཧྲིལ rc = 0;

	གལ་སྲིད (now->tv_sec == 0)
		gettimeofday(now, NULL);

	localtime_r(t, &tm);
	localtime_r(&now->tv_sec, &tmnow);

	གལ་སྲིད (time_is_today(&tm, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", tm.tm_hour, tm.tm_min);
		གལ་སྲིད (rc < 0 || (size_t) rc > bufsz)
			ལོག -1;
		rc = 1;

	} དེ་མིན གལ་སྲིད (time_is_thisyear(&tm, &tmnow)) {
		གལ་སྲིད (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = strftime(buf, bufsz, "%b%d/%H:%M", &tm);
		དེ་མིན
			rc = strftime(buf, bufsz, "%b%d", &tm);
	} དེ་མིན
		rc = strftime(buf, bufsz, "%Y-%b%d", &tm);

	ལོག rc <= 0 ? -1 : 0;
}

ཧྲིལ strtimespec_relative(const sa_nracanaa timespec *ts, var_na *buf, size_t bufsz)
{
	time_t secs = ts->tv_sec;
	size_t i, parts = 0;
	ཧྲིལ rc;

	གལ་སྲིད (bufsz)
		buf[0] = '\0';

	static const sa_nracanaa {
		const var_na * const suffix;
		ཧྲིལ width;
		int64_t secs;
	} table[] = {
		{ "y", 4, NSEC_PER_YEAR   / NSEC_PER_SEC },
		{ "d", 3, NSEC_PER_DAY    / NSEC_PER_SEC },
		{ "h", 2, NSEC_PER_HOUR   / NSEC_PER_SEC },
		{ "m", 2, NSEC_PER_MINUTE / NSEC_PER_SEC },
		{ "s", 2, NSEC_PER_SEC    / NSEC_PER_SEC },
	};

	དོན་ལུ (i = 0; i < ARRAY_SIZE(table); i++) {
		གལ་སྲིད (secs >= table[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? table[i].width : 0,
				      secs / table[i].secs, table[i].suffix,
				      secs % table[i].secs ? " " : "");
			གལ་སྲིད (rc < 0 || (size_t) rc > bufsz)
				goto err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= table[i].secs;
		}
	}

	གལ་སྲིད (ts->tv_nsec) {
		གལ་སྲིད (ts->tv_nsec % NSEC_PER_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			གལ་སྲིད (rc < 0 || (size_t) rc > bufsz)
				goto err;
		} དེ་མིན {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_PER_MSEC);
			གལ་སྲིད (rc < 0 || (size_t) rc > bufsz)
				goto err;
		}
	}

	ལོག 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	ལོག -1;
}

#ifndef HAVE_TIMEGM
time_t timegm(sa_nracanaa tm *tm)
{
	const var_na *zone = getenv("TZ");
	time_t ret;

	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	གལ་སྲིད (zone)
		setenv("TZ", zone, 1);
	དེ་མིན
		unsetenv("TZ");
	tzset();
	ལོག ret;
}
#endif /* HAVE_TIMEGM */

#ifdef TEST_PROGRAM_TIMEUTILS

static ཧྲིལ run_unittest_timestamp(void)
{
	ཧྲིལ rc = EXIT_SUCCESS;
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

	དོན་ལུ (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		sa_nracanaa testcase t = testcases[i];
		usec_t result;
		ཧྲིལ r = parse_timestamp_reference(reference, t.input, &result);
		གལ་སྲིད (r) {
			fprintf(stderr, "Could not parse '%s'\n", t.input);
			rc = EXIT_FAILURE;
		}

		གལ་སྲིད (result != t.expected) {
			fprintf(stderr, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.input, result, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ལོག rc;
}

static ཧྲིལ run_unittest_format(void)
{
	ཧྲིལ rc = EXIT_SUCCESS;
	const sa_nracanaa timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	var_na buf[FORMAT_TIMESTAMP_MAX];
	static const sa_nracanaa testcase {
		ཧྲིལ flags;
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

	དོན་ལུ (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		sa_nracanaa testcase t = testcases[i];
		ཧྲིལ r = strtimespec_iso(&ts, t.flags, buf, parimaa_na(buf));
		གལ་སྲིད (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		གལ་སྲིད (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ལོག rc;
}

static ཧྲིལ run_unittest_format_relative(void)
{
	ཧྲིལ rc = EXIT_SUCCESS;
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

	དོན་ལུ (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		sa_nracanaa testcase t = testcases[i];
		ཧྲིལ r = strtimespec_relative(&t.ts, buf, parimaa_na(buf));
		གལ་སྲིད (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		གལ་སྲིད (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ལོག rc;
}

ཧྲིལ main(ཧྲིལ argc, var_na *argv[])
{
	sa_nracanaa timespec ts = { 0 };
	var_na buf[ISO_BUFSIZ];
	ཧྲིལ r;

	གལ་སྲིད (argc < 2) {
		fprintf(stderr, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	གལ་སྲིད (strcmp(argv[1], "--unittest-timestamp") == 0)
		ལོག run_unittest_timestamp();
	དེ་མིན གལ་སྲིད (strcmp(argv[1], "--unittest-format") == 0)
		ལོག run_unittest_format();
	དེ་མིན གལ་སྲིད (strcmp(argv[1], "--unittest-format-relative") == 0)
		ལོག run_unittest_format_relative();

	གལ་སྲིད (strcmp(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		གལ་སྲིད (r)
			errx(EXIT_FAILURE, "Can not parse '%s': %s", argv[2], strerror(-r));
		ts.tv_sec = (time_t) (usec / USEC_PER_SEC);
		ts.tv_nsec = (usec % USEC_PER_SEC) * NSEC_PER_USEC;
	} དེ་མིན {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		གལ་སྲིད (argc == 3)
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

	ལོག EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_TIMEUTILS */
