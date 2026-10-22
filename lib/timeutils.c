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

static số_nguyên parse_sec(const char *t, usec_t *usec)
{
	static const struct {
		const char *suffix;
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

	const char *p;
	usec_t r = 0;
	số_nguyên something = FALSE;

	assert(t);
	assert(usec);

	p = t;
	cho (;;) {
		long long l, z = 0;
		char *e;
		unsigned i, n = 0;

		p += strspn(p, WHITESPACE);

		nếu (*p == 0) {
			nếu (!something)
				trả_về -EINVAL;

			break;
		}

		errno = 0;
		l = strtoll(p, &e, 10);

		nếu (errno > 0)
			trả_về -errno;

		nếu (l < 0)
			trả_về -ERANGE;

		nếu (*e == '.') {
			char *b = e + 1;

			errno = 0;
			z = strtoll(b, &e, 10);
			nếu (errno > 0)
				trả_về -errno;

			nếu (z < 0)
				trả_về -ERANGE;

			nếu (e == b)
				trả_về -EINVAL;

			n = e - b;

		} không_thì nếu (e == p)
			trả_về -EINVAL;

		e += strspn(e, WHITESPACE);

		cho (i = 0; i < ARRAY_SIZE(table); i++)
			nếu (ul_startswith(e, table[i].suffix)) {
				usec_t k = (usec_t) z * table[i].usec;

				cho (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *table[i].usec + k;
				p = e + strlen(table[i].suffix);

				something = TRUE;
				break;
			}

		nếu (i >= ARRAY_SIZE(table))
			trả_về -EINVAL;

	}

	*usec = r;

	trả_về 0;
}

static số_nguyên parse_subseconds(const char *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t factor = USEC_PER_SEC / 10;

	nếu (*t != '.' && *t != ',')
		trả_về -1;

	trong_khi (*(++t)) {
		nếu (!isdigit(*t) || factor < 1)
			trả_về -1;

		ret += ((usec_t) *t - '0') * factor;
		factor /= 10;
	}

	*usec = ret;
	trả_về 0;
}

static const char *parse_epoch_seconds(const char *t, struct tm *tm)
{
	int64_t s;
	time_t st;
	số_nguyên f, c;

	f = sscanf(t, "%"SCNd64"%n", &s, &c);
	nếu (f < 1)
		trả_về NULL;
	st = s;
	nếu ((int64_t) st < s)
		trả_về NULL;
	nếu (!localtime_r(&st, tm))
		trả_về NULL;
	trả_về t + c;
}

static số_nguyên parse_timestamp_reference(time_t x, const char *t, usec_t *usec)
{
	static const struct {
		const char *name;
		const số_nguyên nr;
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

	const char *k;
	struct tm tm, copy;
	usec_t plus = 0, minus = 0, ret = 0;
	số_nguyên r, weekday = -1;
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

	nếu (streq(t, "now"))
		goto finish;

	không_thì nếu (streq(t, "today")) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;

	} không_thì nếu (streq(t, "yesterday")) {
		tm.tm_mday--;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;

	} không_thì nếu (streq(t, "tomorrow")) {
		tm.tm_mday++;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;

	} không_thì nếu (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		nếu (r < 0)
			trả_về r;

		goto finish;
	} không_thì nếu (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		nếu (r < 0)
			trả_về r;

		goto finish;
	} không_thì nếu (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &tm);
		nếu (k && *k == 0)
			goto finish;
		không_thì nếu (k && parse_subseconds(k, &ret) == 0)
			goto finish;

		trả_về -EINVAL;
	} không_thì nếu (ul_endswith(t, " ago")) {
		char *z;

		z = strndup(t, strlen(t) - 4);
		nếu (!z)
			trả_về -ENOMEM;

		r = parse_sec(z, &minus);
		free(z);
		nếu (r < 0)
			trả_về r;

		goto finish;
	}

	cho (i = 0; i < ARRAY_SIZE(day_nr); i++) {
		size_t skip;

		nếu (!startswith_no_case(t, day_nr[i].name))
			continue;

		skip = strlen(day_nr[i].name);
		nếu (t[skip] != ' ')
			continue;

		weekday = day_nr[i].nr;
		t += skip + 1;
		break;
	}

	copy = tm;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &tm);
	nếu (k && *k == 0)
		goto finish;
	không_thì nếu (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &tm);
	nếu (k && *k == 0)
		goto finish;
	không_thì nếu (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &tm);
	nếu (k && *k == 0)
		goto finish;
	không_thì nếu (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	tm = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &tm);
	nếu (k && *k == 0) {
		tm.tm_sec = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &tm);
	nếu (k && *k == 0) {
		tm.tm_sec = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%y-%m-%d", &tm);
	nếu (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d", &tm);
	nếu (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%H:%M:%S", &tm);
	nếu (k && *k == 0)
		goto finish;
	không_thì nếu (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	tm = copy;
	k = strptime(t, "%H:%M", &tm);
	nếu (k && *k == 0) {
		tm.tm_sec = 0;
		goto finish;
	}

	tm = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &tm);
	nếu (k && *k == 0)
		goto finish;
	không_thì nếu (k && parse_subseconds(k, &ret) == 0)
		goto finish;

	trả_về -EINVAL;

 finish:
	x = mktime(&tm);
	nếu (x == (time_t)-1)
		trả_về -EINVAL;

	nếu (weekday >= 0 && tm.tm_wday != weekday)
		trả_về -EINVAL;

	ret += (usec_t) x * USEC_PER_SEC;

	nếu (minus > ret)
		trả_về -ERANGE;
	nếu ((ret + plus) < ret)
		trả_về -ERANGE;

	ret += plus;
	ret -= minus;

	*usec = ret;

	trả_về 0;
}

số_nguyên ul_parse_timestamp(const char *t, usec_t *usec)
{
	trả_về parse_timestamp_reference(time(NULL), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
số_nguyên get_gmtoff(const struct tm *tp)
{
	nếu (tp->tm_isdst < 0)
		trả_về 0;

#nếu HAVE_TM_GMTOFF
	trả_về tp->tm_gmtoff;
#không_thì
	struct tm tm;
	struct tm gtm;
	struct tm ltm = *tp;
	time_t lt;

	tzset();
	lt = mktime(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	nếu (lt == (time_t) -1) {
		nếu (! localtime_r(&lt, &tm)
			|| ((ltm.tm_sec ^ tm.tm_sec)
			    | (ltm.tm_min ^ tm.tm_min)
			    | (ltm.tm_hour ^ tm.tm_hour)
			    | (ltm.tm_mday ^ tm.tm_mday)
			    | (ltm.tm_mon ^ tm.tm_mon)
			    | (ltm.tm_year ^ tm.tm_year)))
			trả_về 0;
	}

	nếu (! gmtime_r(&lt, &gtm))
		trả_về 0;

	/* Calculate the GMT offset, that is, the difference between the
	 * TP argument (ltm) and GMT (gtm).
	 *
	 * Compute intervening leap days correctly even if year is negative.
	 * Take care to avoid int overflow in leap day calculations, but it's OK
	 * to assume that A and B are close to each other.
	 */
	số_nguyên a4 = (ltm.tm_year >> 2) + (1900 >> 2) - ! (ltm.tm_year & 3);
	số_nguyên b4 = (gtm.tm_year >> 2) + (1900 >> 2) - ! (gtm.tm_year & 3);
	số_nguyên a100 = a4 / 25 - (a4 % 25 < 0);
	số_nguyên b100 = b4 / 25 - (b4 % 25 < 0);
	số_nguyên a400 = a100 >> 2;
	số_nguyên b400 = b100 >> 2;
	số_nguyên intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

	số_nguyên years = ltm.tm_year - gtm.tm_year;
	số_nguyên days = (365 * years + intervening_leap_days
		    + (ltm.tm_yday - gtm.tm_yday));

	trả_về (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#endif
}

static số_nguyên format_iso_time(const struct tm *tm, uint32_t nsec, số_nguyên flags, char *buf, size_t bufsz)
{
	uint32_t usec = nsec / NSEC_PER_USEC;
	char *p = buf;
	số_nguyên len;

	nếu (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       tm->tm_year + (long) 1900,
			       tm->tm_mon + 1, tm->tm_mday);
		nếu (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;
	}

	nếu ((flags & ISO_DATE) && (flags & ISO_TIME)) {
		nếu (bufsz < 1)
			goto err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	nếu (flags & ISO_TIME) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
			       tm->tm_min, tm->tm_sec);
		nếu (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;
	}

	nếu (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		nếu (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;

	} không_thì nếu (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		nếu (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;
	} không_thì nếu (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		nếu (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;

	} không_thì nếu (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		nếu (len < 0 || (size_t) len > bufsz)
			goto err;
		bufsz -= len;
		p += len;
	}

	nếu (flags & ISO_TIMEZONE) {
		số_nguyên tmin  = get_gmtoff(tm) / 60;
		số_nguyên zhour = tmin / 60;
		số_nguyên zmin  = abs(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		nếu (len < 0 || (size_t) len > bufsz)
			goto err;
	}
	trả_về 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	trả_về -1;
}

/* timespec to ISO 8601 */
số_nguyên strtimespec_iso(const struct timespec *ts, số_nguyên flags, char *buf, size_t bufsz)
{
	struct tm tm;
	struct tm *rc;

	nếu (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &tm);
	không_thì
		rc = localtime_r(&ts->tv_sec, &tm);

	nếu (rc)
		trả_về format_iso_time(&tm, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	trả_về -1;
}

/* timeval to ISO 8601 */
số_nguyên strtimeval_iso(const struct timeval *tv, số_nguyên flags, char *buf, size_t bufsz)
{
	struct timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_PER_USEC,
	};

	trả_về strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
số_nguyên strtm_iso(const struct tm *tm, số_nguyên flags, char *buf, size_t bufsz)
{
	trả_về format_iso_time(tm, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
số_nguyên strtime_iso(const time_t *t, số_nguyên flags, char *buf, size_t bufsz)
{
	struct tm tm;
	struct tm *rc;

	nếu (flags & ISO_GMTIME)
		rc = gmtime_r(t, &tm);
	không_thì
		rc = localtime_r(t, &tm);

	nếu (rc)
		trả_về format_iso_time(&tm, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	trả_về -1;
}

/* relative time functions */
static inline số_nguyên time_is_thisyear(struct tm const *const tm,
				   struct tm const *const tmnow)
{
	trả_về tm->tm_year == tmnow->tm_year;
}

static inline số_nguyên time_is_today(struct tm const *const tm,
				struct tm const *const tmnow)
{
	trả_về (tm->tm_yday == tmnow->tm_yday &&
		time_is_thisyear(tm, tmnow));
}

số_nguyên strtime_short(const time_t *t, struct timeval *now, số_nguyên flags, char *buf, size_t bufsz)
{
	struct tm tm, tmnow;
	số_nguyên rc = 0;

	nếu (now->tv_sec == 0)
		gettimeofday(now, NULL);

	localtime_r(t, &tm);
	localtime_r(&now->tv_sec, &tmnow);

	nếu (time_is_today(&tm, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", tm.tm_hour, tm.tm_min);
		nếu (rc < 0 || (size_t) rc > bufsz)
			trả_về -1;
		rc = 1;

	} không_thì nếu (time_is_thisyear(&tm, &tmnow)) {
		nếu (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = strftime(buf, bufsz, "%b%d/%H:%M", &tm);
		không_thì
			rc = strftime(buf, bufsz, "%b%d", &tm);
	} không_thì
		rc = strftime(buf, bufsz, "%Y-%b%d", &tm);

	trả_về rc <= 0 ? -1 : 0;
}

số_nguyên strtimespec_relative(const struct timespec *ts, char *buf, size_t bufsz)
{
	time_t secs = ts->tv_sec;
	size_t i, parts = 0;
	số_nguyên rc;

	nếu (bufsz)
		buf[0] = '\0';

	static const struct {
		const char * const suffix;
		số_nguyên width;
		int64_t secs;
	} table[] = {
		{ "y", 4, NSEC_PER_YEAR   / NSEC_PER_SEC },
		{ "d", 3, NSEC_PER_DAY    / NSEC_PER_SEC },
		{ "h", 2, NSEC_PER_HOUR   / NSEC_PER_SEC },
		{ "m", 2, NSEC_PER_MINUTE / NSEC_PER_SEC },
		{ "s", 2, NSEC_PER_SEC    / NSEC_PER_SEC },
	};

	cho (i = 0; i < ARRAY_SIZE(table); i++) {
		nếu (secs >= table[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? table[i].width : 0,
				      secs / table[i].secs, table[i].suffix,
				      secs % table[i].secs ? " " : "");
			nếu (rc < 0 || (size_t) rc > bufsz)
				goto err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= table[i].secs;
		}
	}

	nếu (ts->tv_nsec) {
		nếu (ts->tv_nsec % NSEC_PER_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			nếu (rc < 0 || (size_t) rc > bufsz)
				goto err;
		} không_thì {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_PER_MSEC);
			nếu (rc < 0 || (size_t) rc > bufsz)
				goto err;
		}
	}

	trả_về 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	trả_về -1;
}

#ifndef HAVE_TIMEGM
time_t timegm(struct tm *tm)
{
	const char *zone = getenv("TZ");
	time_t ret;

	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	nếu (zone)
		setenv("TZ", zone, 1);
	không_thì
		unsetenv("TZ");
	tzset();
	trả_về ret;
}
#endif /* HAVE_TIMEGM */

#ifdef TEST_PROGRAM_TIMEUTILS

static số_nguyên run_unittest_timestamp(void)
{
	số_nguyên rc = EXIT_SUCCESS;
	time_t reference = 1674180427;
	static const struct testcase {
		const char * const input;
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

	cho (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		struct testcase t = testcases[i];
		usec_t result;
		số_nguyên r = parse_timestamp_reference(reference, t.input, &result);
		nếu (r) {
			fprintf(stderr, "Could not parse '%s'\n", t.input);
			rc = EXIT_FAILURE;
		}

		nếu (result != t.expected) {
			fprintf(stderr, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.input, result, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	trả_về rc;
}

static số_nguyên run_unittest_format(void)
{
	số_nguyên rc = EXIT_SUCCESS;
	const struct timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	char buf[FORMAT_TIMESTAMP_MAX];
	static const struct testcase {
		số_nguyên flags;
		const char * const expected;
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

	cho (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		struct testcase t = testcases[i];
		số_nguyên r = strtimespec_iso(&ts, t.flags, buf, sizeof(buf));
		nếu (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		nếu (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	trả_về rc;
}

static số_nguyên run_unittest_format_relative(void)
{
	số_nguyên rc = EXIT_SUCCESS;
	char buf[FORMAT_TIMESTAMP_MAX];
	static const struct testcase {
		struct timespec ts;
		const char * const expected;
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

	cho (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		struct testcase t = testcases[i];
		số_nguyên r = strtimespec_relative(&t.ts, buf, sizeof(buf));
		nếu (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		nếu (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	trả_về rc;
}

số_nguyên main(số_nguyên argc, char *argv[])
{
	struct timespec ts = { 0 };
	char buf[ISO_BUFSIZ];
	số_nguyên r;

	nếu (argc < 2) {
		fprintf(stderr, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	nếu (strcmp(argv[1], "--unittest-timestamp") == 0)
		trả_về run_unittest_timestamp();
	không_thì nếu (strcmp(argv[1], "--unittest-format") == 0)
		trả_về run_unittest_format();
	không_thì nếu (strcmp(argv[1], "--unittest-format-relative") == 0)
		trả_về run_unittest_format_relative();

	nếu (strcmp(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		nếu (r)
			errx(EXIT_FAILURE, "Can not parse '%s': %s", argv[2], strerror(-r));
		ts.tv_sec = (time_t) (usec / USEC_PER_SEC);
		ts.tv_nsec = (usec % USEC_PER_SEC) * NSEC_PER_USEC;
	} không_thì {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		nếu (argc == 3)
			ts.tv_nsec = strtos64_or_err(argv[2], "failed to parse <usec>")
				     * NSEC_PER_USEC;
	}

	strtimespec_iso(&ts, ISO_DATE, buf, sizeof(buf));
	printf("Date: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIME, buf, sizeof(buf));
	printf("Time: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_DATE | ISO_TIME | ISO_COMMAUSEC | ISO_T,
		       buf, sizeof(buf));
	printf("Full: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIMESTAMP_DOT, buf, sizeof(buf));
	printf("Zone: '%s'\n", buf);

	strtimespec_relative(&ts, buf, sizeof(buf));
	printf("Rel:  '%s'\n", buf);

	trả_về EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_TIMEUTILS */
