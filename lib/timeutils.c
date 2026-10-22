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
#అన్తర్భావ <assert.h>
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <sys/time.h>
#అన్తర్భావ <inttypes.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "nls.h"
#అన్తర్భావ "strutils.h"
#అన్తర్భావ "timeutils.h"

#నిర్వచించు WHITESPACE " \t\n\r"

#నిర్వచించు streq(a,b) (strcmp((a),(b)) == 0)

స్థిర పూర్ణాంకం parse_sec(స్థిరరాశి అక్షరం *t, usec_t *usec)
{
	స్థిర స్థిరరాశి నిర్మాణం {
		స్థిరరాశి అక్షరం *suffix;
		usec_t usec;
	} పట్టిక[] = {
		{ "seconds",	USEC_చొప్పున_SEC },
		{ "second",	USEC_చొప్పున_SEC },
		{ "sec",	USEC_చొప్పున_SEC },
		{ "s",		USEC_చొప్పున_SEC },
		{ "minutes",	USEC_చొప్పున_MINUTE },
		{ "minute",	USEC_చొప్పున_MINUTE },
		{ "min",	USEC_చొప్పున_MINUTE },
		{ "months",	USEC_చొప్పున_MONTH },
		{ "month",	USEC_చొప్పున_MONTH },
		{ "msec",	USEC_చొప్పున_MSEC },
		{ "ms",		USEC_చొప్పున_MSEC },
		{ "m",		USEC_చొప్పున_MINUTE },
		{ "hours",	USEC_చొప్పున_HOUR },
		{ "hour",	USEC_చొప్పున_HOUR },
		{ "hr",		USEC_చొప్పున_HOUR },
		{ "h",		USEC_చొప్పున_HOUR },
		{ "days",	USEC_చొప్పున_DAY },
		{ "day",	USEC_చొప్పున_DAY },
		{ "d",		USEC_చొప్పున_DAY },
		{ "weeks",	USEC_చొప్పున_WEEK },
		{ "week",	USEC_చొప్పున_WEEK },
		{ "w",		USEC_చొప్పున_WEEK },
		{ "years",	USEC_చొప్పున_YEAR },
		{ "year",	USEC_చొప్పున_YEAR },
		{ "y",		USEC_చొప్పున_YEAR },
		{ "usec",	1ULL },
		{ "us",		1ULL },
		{ "",		USEC_చొప్పున_SEC },	/* default is sec */
	};

	స్థిరరాశి అక్షరం *p;
	usec_t r = 0;
	పూర్ణాంకం something = FALSE;

	ప్రతిజ్ఞా(t);
	ప్రతిజ్ఞా(usec);

	p = t;
	కొరకు (;;) {
		దీర్ఘ దీర్ఘ l, z = 0;
		అక్షరం *e;
		గుర్తులేని i, n = 0;

		p += strspn(p, WHITESPACE);

		ఒకవేళ (*p == 0) {
			ఒకవేళ (!something)
				తిరిగి -EINVAL;

			ఆపు;
		}

		errno = 0;
		l = strtoll(p, &e, 10);

		ఒకవేళ (errno > 0)
			తిరిగి -errno;

		ఒకవేళ (l < 0)
			తిరిగి -ERANGE;

		ఒకవేళ (*e == '.') {
			అక్షరం *b = e + 1;

			errno = 0;
			z = strtoll(b, &e, 10);
			ఒకవేళ (errno > 0)
				తిరిగి -errno;

			ఒకవేళ (z < 0)
				తిరిగి -ERANGE;

			ఒకవేళ (e == b)
				తిరిగి -EINVAL;

			n = e - b;

		} లేకపోతే ఒకవేళ (e == p)
			తిరిగి -EINVAL;

		e += strspn(e, WHITESPACE);

		కొరకు (i = 0; i < ARRAY_కొలత(పట్టిక); i++)
			ఒకవేళ (ul_startswith(e, పట్టిక[i].suffix)) {
				usec_t k = (usec_t) z * పట్టిక[i].usec;

				కొరకు (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *పట్టిక[i].usec + k;
				p = e + అక్షరశ్రేణి_పొడవు(పట్టిక[i].suffix);

				something = TRUE;
				ఆపు;
			}

		ఒకవేళ (i >= ARRAY_కొలత(పట్టిక))
			తిరిగి -EINVAL;

	}

	*usec = r;

	తిరిగి 0;
}

స్థిర పూర్ణాంకం parse_subseconds(స్థిరరాశి అక్షరం *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t గుణకం = USEC_చొప్పున_SEC / 10;

	ఒకవేళ (*t != '.' && *t != ',')
		తిరిగి -1;

	అయితే (*(++t)) {
		ఒకవేళ (!అంకెనా(*t) || గుణకం < 1)
			తిరిగి -1;

		ret += ((usec_t) *t - '0') * గుణకం;
		గుణకం /= 10;
	}

	*usec = ret;
	తిరిగి 0;
}

స్థిర స్థిరరాశి అక్షరం *parse_epoch_seconds(స్థిరరాశి అక్షరం *t, నిర్మాణం tm *tm)
{
	int64_t s;
	time_t st;
	పూర్ణాంకం f, c;

	f = అక్షరశ్రేణి_చదువు(t, "%"SCNd64"%n", &s, &c);
	ఒకవేళ (f < 1)
		తిరిగి ఖాళీ;
	st = s;
	ఒకవేళ ((int64_t) st < s)
		తిరిగి ఖాళీ;
	ఒకవేళ (!localtime_r(&st, tm))
		తిరిగి ఖాళీ;
	తిరిగి t + c;
}

స్థిర పూర్ణాంకం parse_timestamp_reference(time_t x, స్థిరరాశి అక్షరం *t, usec_t *usec)
{
	స్థిర స్థిరరాశి నిర్మాణం {
		స్థిరరాశి అక్షరం *name;
		స్థిరరాశి పూర్ణాంకం nr;
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

	స్థిరరాశి అక్షరం *k;
	నిర్మాణం tm tm, copy;
	usec_t plus = 0, minus = 0, ret = 0;
	పూర్ణాంకం r, weekday = -1;
	గుర్తులేని i;

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

	ప్రతిజ్ఞా(t);
	ప్రతిజ్ఞా(usec);

	localtime_r(&x, &tm);
	tm.tm_isdst = -1;

	ఒకవేళ (streq(t, "now"))
		వెళ్ళు ముగింపు;

	లేకపోతే ఒకవేళ (streq(t, "today")) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		వెళ్ళు ముగింపు;

	} లేకపోతే ఒకవేళ (streq(t, "yesterday")) {
		tm.tm_mday--;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		వెళ్ళు ముగింపు;

	} లేకపోతే ఒకవేళ (streq(t, "tomorrow")) {
		tm.tm_mday++;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		వెళ్ళు ముగింపు;

	} లేకపోతే ఒకవేళ (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		ఒకవేళ (r < 0)
			తిరిగి r;

		వెళ్ళు ముగింపు;
	} లేకపోతే ఒకవేళ (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		ఒకవేళ (r < 0)
			తిరిగి r;

		వెళ్ళు ముగింపు;
	} లేకపోతే ఒకవేళ (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &tm);
		ఒకవేళ (k && *k == 0)
			వెళ్ళు ముగింపు;
		లేకపోతే ఒకవేళ (k && parse_subseconds(k, &ret) == 0)
			వెళ్ళు ముగింపు;

		తిరిగి -EINVAL;
	} లేకపోతే ఒకవేళ (ul_endswith(t, " ago")) {
		అక్షరం *z;

		z = strndup(t, అక్షరశ్రేణి_పొడవు(t) - 4);
		ఒకవేళ (!z)
			తిరిగి -ENOMEM;

		r = parse_sec(z, &minus);
		విడుదల(z);
		ఒకవేళ (r < 0)
			తిరిగి r;

		వెళ్ళు ముగింపు;
	}

	కొరకు (i = 0; i < ARRAY_కొలత(day_nr); i++) {
		size_t skip;

		ఒకవేళ (!startswith_no_case(t, day_nr[i].name))
			కొనసాగు;

		skip = అక్షరశ్రేణి_పొడవు(day_nr[i].name);
		ఒకవేళ (t[skip] != ' ')
			కొనసాగు;

		weekday = day_nr[i].nr;
		t += skip + 1;
		ఆపు;
	}

	copy = tm;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &tm);
	ఒకవేళ (k && *k == 0)
		వెళ్ళు ముగింపు;
	లేకపోతే ఒకవేళ (k && parse_subseconds(k, &ret) == 0)
		వెళ్ళు ముగింపు;

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &tm);
	ఒకవేళ (k && *k == 0)
		వెళ్ళు ముగింపు;
	లేకపోతే ఒకవేళ (k && parse_subseconds(k, &ret) == 0)
		వెళ్ళు ముగింపు;

	tm = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &tm);
	ఒకవేళ (k && *k == 0)
		వెళ్ళు ముగింపు;
	లేకపోతే ఒకవేళ (k && parse_subseconds(k, &ret) == 0)
		వెళ్ళు ముగింపు;

	tm = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &tm);
	ఒకవేళ (k && *k == 0) {
		tm.tm_sec = 0;
		వెళ్ళు ముగింపు;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &tm);
	ఒకవేళ (k && *k == 0) {
		tm.tm_sec = 0;
		వెళ్ళు ముగింపు;
	}

	tm = copy;
	k = strptime(t, "%y-%m-%d", &tm);
	ఒకవేళ (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		వెళ్ళు ముగింపు;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d", &tm);
	ఒకవేళ (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		వెళ్ళు ముగింపు;
	}

	tm = copy;
	k = strptime(t, "%H:%M:%S", &tm);
	ఒకవేళ (k && *k == 0)
		వెళ్ళు ముగింపు;
	లేకపోతే ఒకవేళ (k && parse_subseconds(k, &ret) == 0)
		వెళ్ళు ముగింపు;

	tm = copy;
	k = strptime(t, "%H:%M", &tm);
	ఒకవేళ (k && *k == 0) {
		tm.tm_sec = 0;
		వెళ్ళు ముగింపు;
	}

	tm = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &tm);
	ఒకవేళ (k && *k == 0)
		వెళ్ళు ముగింపు;
	లేకపోతే ఒకవేళ (k && parse_subseconds(k, &ret) == 0)
		వెళ్ళు ముగింపు;

	తిరిగి -EINVAL;

 ముగింపు:
	x = mktime(&tm);
	ఒకవేళ (x == (time_t)-1)
		తిరిగి -EINVAL;

	ఒకవేళ (weekday >= 0 && tm.tm_wday != weekday)
		తిరిగి -EINVAL;

	ret += (usec_t) x * USEC_చొప్పున_SEC;

	ఒకవేళ (minus > ret)
		తిరిగి -ERANGE;
	ఒకవేళ ((ret + plus) < ret)
		తిరిగి -ERANGE;

	ret += plus;
	ret -= minus;

	*usec = ret;

	తిరిగి 0;
}

పూర్ణాంకం ul_parse_timestamp(స్థిరరాశి అక్షరం *t, usec_t *usec)
{
	తిరిగి parse_timestamp_reference(time(ఖాళీ), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
పూర్ణాంకం get_gmtoff(స్థిరరాశి నిర్మాణం tm *tp)
{
	ఒకవేళ (tp->tm_isdst < 0)
		తిరిగి 0;

#ఒకవేళ HAVE_TM_GMTOFF
	తిరిగి tp->tm_gmtoff;
#లేకపోతే
	నిర్మాణం tm tm;
	నిర్మాణం tm gtm;
	నిర్మాణం tm ltm = *tp;
	time_t lt;

	tzset();
	lt = mktime(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	ఒకవేళ (lt == (time_t) -1) {
		ఒకవేళ (! localtime_r(&lt, &tm)
			|| ((ltm.tm_sec ^ tm.tm_sec)
			    | (ltm.tm_min ^ tm.tm_min)
			    | (ltm.tm_hour ^ tm.tm_hour)
			    | (ltm.tm_mday ^ tm.tm_mday)
			    | (ltm.tm_mon ^ tm.tm_mon)
			    | (ltm.tm_year ^ tm.tm_year)))
			తిరిగి 0;
	}

	ఒకవేళ (! gmtime_r(&lt, &gtm))
		తిరిగి 0;

	/* Calculate the GMT offset, that is, the difference between the
	 * TP argument (ltm) and GMT (gtm).
	 *
	 * Compute intervening leap days correctly even if year is negative.
	 * Take care to avoid int overflow in leap day calculations, but it's OK
	 * to assume that A and B are close to each other.
	 */
	పూర్ణాంకం a4 = (ltm.tm_year >> 2) + (1900 >> 2) - ! (ltm.tm_year & 3);
	పూర్ణాంకం b4 = (gtm.tm_year >> 2) + (1900 >> 2) - ! (gtm.tm_year & 3);
	పూర్ణాంకం a100 = a4 / 25 - (a4 % 25 < 0);
	పూర్ణాంకం b100 = b4 / 25 - (b4 % 25 < 0);
	పూర్ణాంకం a400 = a100 >> 2;
	పూర్ణాంకం b400 = b100 >> 2;
	పూర్ణాంకం intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

	పూర్ణాంకం years = ltm.tm_year - gtm.tm_year;
	పూర్ణాంకం days = (365 * years + intervening_leap_days
		    + (ltm.tm_yday - gtm.tm_yday));

	తిరిగి (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#అయితే_ముగింపు
}

స్థిర పూర్ణాంకం format_iso_సమయం(స్థిరరాశి నిర్మాణం tm *tm, uint32_t nsec, పూర్ణాంకం flags, అక్షరం *buf, size_t bufsz)
{
	uint32_t usec = nsec / NSEC_చొప్పున_USEC;
	అక్షరం *p = buf;
	పూర్ణాంకం len;

	ఒకవేళ (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       tm->tm_year + (దీర్ఘ) 1900,
			       tm->tm_mon + 1, tm->tm_mday);
		ఒకవేళ (len < 0 || (size_t) len > bufsz)
			వెళ్ళు err;
		bufsz -= len;
		p += len;
	}

	ఒకవేళ ((flags & ISO_DATE) && (flags & ISO_సమయం)) {
		ఒకవేళ (bufsz < 1)
			వెళ్ళు err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	ఒకవేళ (flags & ISO_సమయం) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
			       tm->tm_min, tm->tm_sec);
		ఒకవేళ (len < 0 || (size_t) len > bufsz)
			వెళ్ళు err;
		bufsz -= len;
		p += len;
	}

	ఒకవేళ (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		ఒకవేళ (len < 0 || (size_t) len > bufsz)
			వెళ్ళు err;
		bufsz -= len;
		p += len;

	} లేకపోతే ఒకవేళ (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		ఒకవేళ (len < 0 || (size_t) len > bufsz)
			వెళ్ళు err;
		bufsz -= len;
		p += len;
	} లేకపోతే ఒకవేళ (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		ఒకవేళ (len < 0 || (size_t) len > bufsz)
			వెళ్ళు err;
		bufsz -= len;
		p += len;

	} లేకపోతే ఒకవేళ (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		ఒకవేళ (len < 0 || (size_t) len > bufsz)
			వెళ్ళు err;
		bufsz -= len;
		p += len;
	}

	ఒకవేళ (flags & ISO_TIMEZONE) {
		పూర్ణాంకం tmin  = get_gmtoff(tm) / 60;
		పూర్ణాంకం zhour = tmin / 60;
		పూర్ణాంకం zmin  = నిరపేక్ష(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		ఒకవేళ (len < 0 || (size_t) len > bufsz)
			వెళ్ళు err;
	}
	తిరిగి 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	తిరిగి -1;
}

/* timespec to ISO 8601 */
పూర్ణాంకం strtimespec_iso(స్థిరరాశి నిర్మాణం timespec *ts, పూర్ణాంకం flags, అక్షరం *buf, size_t bufsz)
{
	నిర్మాణం tm tm;
	నిర్మాణం tm *rc;

	ఒకవేళ (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &tm);
	లేకపోతే
		rc = localtime_r(&ts->tv_sec, &tm);

	ఒకవేళ (rc)
		తిరిగి format_iso_సమయం(&tm, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	తిరిగి -1;
}

/* timeval to ISO 8601 */
పూర్ణాంకం strtimeval_iso(స్థిరరాశి నిర్మాణం timeval *tv, పూర్ణాంకం flags, అక్షరం *buf, size_t bufsz)
{
	నిర్మాణం timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_చొప్పున_USEC,
	};

	తిరిగి strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
పూర్ణాంకం strtm_iso(స్థిరరాశి నిర్మాణం tm *tm, పూర్ణాంకం flags, అక్షరం *buf, size_t bufsz)
{
	తిరిగి format_iso_సమయం(tm, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
పూర్ణాంకం strtime_iso(స్థిరరాశి time_t *t, పూర్ణాంకం flags, అక్షరం *buf, size_t bufsz)
{
	నిర్మాణం tm tm;
	నిర్మాణం tm *rc;

	ఒకవేళ (flags & ISO_GMTIME)
		rc = gmtime_r(t, &tm);
	లేకపోతే
		rc = localtime_r(t, &tm);

	ఒకవేళ (rc)
		తిరిగి format_iso_సమయం(&tm, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	తిరిగి -1;
}

/* relative time functions */
స్థిర అంతఃవరుస పూర్ణాంకం సమయం_is_thisyear(నిర్మాణం tm స్థిరరాశి *స్థిరరాశి tm,
				   నిర్మాణం tm స్థిరరాశి *స్థిరరాశి tmnow)
{
	తిరిగి tm->tm_year == tmnow->tm_year;
}

స్థిర అంతఃవరుస పూర్ణాంకం సమయం_is_today(నిర్మాణం tm స్థిరరాశి *స్థిరరాశి tm,
				నిర్మాణం tm స్థిరరాశి *స్థిరరాశి tmnow)
{
	తిరిగి (tm->tm_yday == tmnow->tm_yday &&
		సమయం_is_thisyear(tm, tmnow));
}

పూర్ణాంకం strtime_short(స్థిరరాశి time_t *t, నిర్మాణం timeval *now, పూర్ణాంకం flags, అక్షరం *buf, size_t bufsz)
{
	నిర్మాణం tm tm, tmnow;
	పూర్ణాంకం rc = 0;

	ఒకవేళ (now->tv_sec == 0)
		gettimeofday(now, ఖాళీ);

	localtime_r(t, &tm);
	localtime_r(&now->tv_sec, &tmnow);

	ఒకవేళ (సమయం_is_today(&tm, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", tm.tm_hour, tm.tm_min);
		ఒకవేళ (rc < 0 || (size_t) rc > bufsz)
			తిరిగి -1;
		rc = 1;

	} లేకపోతే ఒకవేళ (సమయం_is_thisyear(&tm, &tmnow)) {
		ఒకవేళ (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = strftime(buf, bufsz, "%b%d/%H:%M", &tm);
		లేకపోతే
			rc = strftime(buf, bufsz, "%b%d", &tm);
	} లేకపోతే
		rc = strftime(buf, bufsz, "%Y-%b%d", &tm);

	తిరిగి rc <= 0 ? -1 : 0;
}

పూర్ణాంకం strtimespec_relative(స్థిరరాశి నిర్మాణం timespec *ts, అక్షరం *buf, size_t bufsz)
{
	time_t secs = ts->tv_sec;
	size_t i, parts = 0;
	పూర్ణాంకం rc;

	ఒకవేళ (bufsz)
		buf[0] = '\0';

	స్థిర స్థిరరాశి నిర్మాణం {
		స్థిరరాశి అక్షరం * స్థిరరాశి suffix;
		పూర్ణాంకం వెడల్పు;
		int64_t secs;
	} పట్టిక[] = {
		{ "y", 4, NSEC_చొప్పున_YEAR   / NSEC_చొప్పున_SEC },
		{ "d", 3, NSEC_చొప్పున_DAY    / NSEC_చొప్పున_SEC },
		{ "h", 2, NSEC_చొప్పున_HOUR   / NSEC_చొప్పున_SEC },
		{ "m", 2, NSEC_చొప్పున_MINUTE / NSEC_చొప్పున_SEC },
		{ "s", 2, NSEC_చొప్పున_SEC    / NSEC_చొప్పున_SEC },
	};

	కొరకు (i = 0; i < ARRAY_కొలత(పట్టిక); i++) {
		ఒకవేళ (secs >= పట్టిక[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? పట్టిక[i].వెడల్పు : 0,
				      secs / పట్టిక[i].secs, పట్టిక[i].suffix,
				      secs % పట్టిక[i].secs ? " " : "");
			ఒకవేళ (rc < 0 || (size_t) rc > bufsz)
				వెళ్ళు err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= పట్టిక[i].secs;
		}
	}

	ఒకవేళ (ts->tv_nsec) {
		ఒకవేళ (ts->tv_nsec % NSEC_చొప్పున_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			ఒకవేళ (rc < 0 || (size_t) rc > bufsz)
				వెళ్ళు err;
		} లేకపోతే {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_చొప్పున_MSEC);
			ఒకవేళ (rc < 0 || (size_t) rc > bufsz)
				వెళ్ళు err;
		}
	}

	తిరిగి 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	తిరిగి -1;
}

#నిర్వచితం_కాకపోతే HAVE_TIMEGM
time_t timegm(నిర్మాణం tm *tm)
{
	స్థిరరాశి అక్షరం *zone = getenv("TZ");
	time_t ret;

	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	ఒకవేళ (zone)
		setenv("TZ", zone, 1);
	లేకపోతే
		unsetenv("TZ");
	tzset();
	తిరిగి ret;
}
#అయితే_ముగింపు /* HAVE_TIMEGM */

#నిర్వచితమైతే TEST_కార్యక్రమం_TIMEUTILS

స్థిర పూర్ణాంకం నిరంతరం_unittest_timestamp(శూన్య)
{
	పూర్ణాంకం rc = EXIT_SUCCESS;
	time_t reference = 1674180427;
	స్థిర స్థిరరాశి నిర్మాణం testcase {
		స్థిరరాశి అక్షరం * స్థిరరాశి ఆదానం;
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

	కొరకు (size_t i = 0; i < ARRAY_కొలత(testcases); i++) {
		నిర్మాణం testcase t = testcases[i];
		usec_t ఫలితం;
		పూర్ణాంకం r = parse_timestamp_reference(reference, t.ఆదానం, &ఫలితం);
		ఒకవేళ (r) {
			fprintf(stderr, "Could not parse '%s'\n", t.ఆదానం);
			rc = EXIT_FAILURE;
		}

		ఒకవేళ (ఫలితం != t.expected) {
			fprintf(stderr, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.ఆదానం, ఫలితం, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	తిరిగి rc;
}

స్థిర పూర్ణాంకం నిరంతరం_unittest_format(శూన్య)
{
	పూర్ణాంకం rc = EXIT_SUCCESS;
	స్థిరరాశి నిర్మాణం timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	అక్షరం buf[FORMAT_TIMESTAMP_MAX];
	స్థిర స్థిరరాశి నిర్మాణం testcase {
		పూర్ణాంకం flags;
		స్థిరరాశి అక్షరం * స్థిరరాశి expected;
	} testcases[] = {
		{ ISO_DATE,               "2023-01-20"                       },
		{ ISO_సమయం,               "02:07:07"                         },
		{ ISO_TIMEZONE,           "+00:00"                           },
		{ ISO_TIMESTAMP_T,        "2023-01-20T02:07:07+00:00"        },
		{ ISO_TIMESTAMP_COMMA_G,  "2023-01-20 02:07:07,000012+00:00" },
		{ ISO_సమయం | ISO_DOTNSEC, "02:07:07.000012345" },
	};

	setenv("TZ", "GMT", 1);
	tzset();

	కొరకు (size_t i = 0; i < ARRAY_కొలత(testcases); i++) {
		నిర్మాణం testcase t = testcases[i];
		పూర్ణాంకం r = strtimespec_iso(&ts, t.flags, buf, పరిమాణం(buf));
		ఒకవేళ (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		ఒకవేళ (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	తిరిగి rc;
}

స్థిర పూర్ణాంకం నిరంతరం_unittest_format_relative(శూన్య)
{
	పూర్ణాంకం rc = EXIT_SUCCESS;
	అక్షరం buf[FORMAT_TIMESTAMP_MAX];
	స్థిర స్థిరరాశి నిర్మాణం testcase {
		నిర్మాణం timespec ts;
		స్థిరరాశి అక్షరం * స్థిరరాశి expected;
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

	కొరకు (size_t i = 0; i < ARRAY_కొలత(testcases); i++) {
		నిర్మాణం testcase t = testcases[i];
		పూర్ణాంకం r = strtimespec_relative(&t.ts, buf, పరిమాణం(buf));
		ఒకవేళ (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		ఒకవేళ (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	తిరిగి rc;
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[])
{
	నిర్మాణం timespec ts = { 0 };
	అక్షరం buf[ISO_BUFSIZ];
	పూర్ణాంకం r;

	ఒకవేళ (argc < 2) {
		fprintf(stderr, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		నిష్క్రమించు(EXIT_FAILURE);
	}

	ఒకవేళ (strcmp(argv[1], "--unittest-timestamp") == 0)
		తిరిగి నిరంతరం_unittest_timestamp();
	లేకపోతే ఒకవేళ (strcmp(argv[1], "--unittest-format") == 0)
		తిరిగి నిరంతరం_unittest_format();
	లేకపోతే ఒకవేళ (strcmp(argv[1], "--unittest-format-relative") == 0)
		తిరిగి నిరంతరం_unittest_format_relative();

	ఒకవేళ (strcmp(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		ఒకవేళ (r)
			errx(EXIT_FAILURE, "Can not parse '%s': %s", argv[2], strerror(-r));
		ts.tv_sec = (time_t) (usec / USEC_చొప్పున_SEC);
		ts.tv_nsec = (usec % USEC_చొప్పున_SEC) * NSEC_చొప్పున_USEC;
	} లేకపోతే {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		ఒకవేళ (argc == 3)
			ts.tv_nsec = strtos64_or_err(argv[2], "failed to parse <usec>")
				     * NSEC_చొప్పున_USEC;
	}

	strtimespec_iso(&ts, ISO_DATE, buf, పరిమాణం(buf));
	రాయి("Date: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_సమయం, buf, పరిమాణం(buf));
	రాయి("Time: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_DATE | ISO_సమయం | ISO_COMMAUSEC | ISO_T,
		       buf, పరిమాణం(buf));
	రాయి("Full: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIMESTAMP_DOT, buf, పరిమాణం(buf));
	రాయి("Zone: '%s'\n", buf);

	strtimespec_relative(&ts, buf, పరిమాణం(buf));
	రాయి("Rel:  '%s'\n", buf);

	తిరిగి EXIT_SUCCESS;
}

#అయితే_ముగింపు /* TEST_PROGRAM_TIMEUTILS */
