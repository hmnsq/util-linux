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
#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <sys/time.h>
#அந்தர்பாவ <inttypes.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "nls.h"
#அந்தர்பாவ "strutils.h"
#அந்தர்பாவ "timeutils.h"

#define WHITESPACE " \t\n\r"

#define streq(a,b) (strcmp((a),(b)) == 0)

நிலையான முழு parse_sec(மாறிலி எழுத்து *t, usec_t *usec)
{
	நிலையான மாறிலி கட்டமைப்பு {
		மாறிலி எழுத்து *suffix;
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

	மாறிலி எழுத்து *p;
	usec_t r = 0;
	முழு something = FALSE;

	ப்ரதிஜ்ஞா(t);
	ப்ரதிஜ்ஞா(usec);

	p = t;
	க்கு (;;) {
		long long l, z = 0;
		எழுத்து *e;
		unsigned i, n = 0;

		p += strspn(p, WHITESPACE);

		எனில் (*p == 0) {
			எனில் (!something)
				திரும்பு -EINVAL;

			நிறுத்து;
		}

		errno = 0;
		l = strtoll(p, &e, 10);

		எனில் (errno > 0)
			திரும்பு -errno;

		எனில் (l < 0)
			திரும்பு -ERANGE;

		எனில் (*e == '.') {
			எழுத்து *b = e + 1;

			errno = 0;
			z = strtoll(b, &e, 10);
			எனில் (errno > 0)
				திரும்பு -errno;

			எனில் (z < 0)
				திரும்பு -ERANGE;

			எனில் (e == b)
				திரும்பு -EINVAL;

			n = e - b;

		} இல்லையெனில் எனில் (e == p)
			திரும்பு -EINVAL;

		e += strspn(e, WHITESPACE);

		க்கு (i = 0; i < ARRAY_SIZE(table); i++)
			எனில் (ul_startswith(e, table[i].suffix)) {
				usec_t k = (usec_t) z * table[i].usec;

				க்கு (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *table[i].usec + k;
				p = e + strlen(table[i].suffix);

				something = TRUE;
				நிறுத்து;
			}

		எனில் (i >= ARRAY_SIZE(table))
			திரும்பு -EINVAL;

	}

	*usec = r;

	திரும்பு 0;
}

நிலையான முழு parse_subseconds(மாறிலி எழுத்து *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t factor = USEC_PER_SEC / 10;

	எனில் (*t != '.' && *t != ',')
		திரும்பு -1;

	போது (*(++t)) {
		எனில் (!isdigit(*t) || factor < 1)
			திரும்பு -1;

		ret += ((usec_t) *t - '0') * factor;
		factor /= 10;
	}

	*usec = ret;
	திரும்பு 0;
}

நிலையான மாறிலி எழுத்து *parse_epoch_seconds(மாறிலி எழுத்து *t, கட்டமைப்பு tm *tm)
{
	int64_t s;
	time_t st;
	முழு f, c;

	f = sscanf(t, "%"SCNd64"%n", &s, &c);
	எனில் (f < 1)
		திரும்பு வெற்று;
	st = s;
	எனில் ((int64_t) st < s)
		திரும்பு வெற்று;
	எனில் (!localtime_r(&st, tm))
		திரும்பு வெற்று;
	திரும்பு t + c;
}

நிலையான முழு parse_timestamp_reference(time_t x, மாறிலி எழுத்து *t, usec_t *usec)
{
	நிலையான மாறிலி கட்டமைப்பு {
		மாறிலி எழுத்து *name;
		மாறிலி முழு nr;
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

	மாறிலி எழுத்து *k;
	கட்டமைப்பு tm tm, copy;
	usec_t plus = 0, minus = 0, ret = 0;
	முழு r, weekday = -1;
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

	ப்ரதிஜ்ஞா(t);
	ப்ரதிஜ்ஞா(usec);

	localtime_r(&x, &tm);
	tm.tm_isdst = -1;

	எனில் (streq(t, "now"))
		செல் finish;

	இல்லையெனில் எனில் (streq(t, "today")) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		செல் finish;

	} இல்லையெனில் எனில் (streq(t, "yesterday")) {
		tm.tm_mday--;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		செல் finish;

	} இல்லையெனில் எனில் (streq(t, "tomorrow")) {
		tm.tm_mday++;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		செல் finish;

	} இல்லையெனில் எனில் (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		எனில் (r < 0)
			திரும்பு r;

		செல் finish;
	} இல்லையெனில் எனில் (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		எனில் (r < 0)
			திரும்பு r;

		செல் finish;
	} இல்லையெனில் எனில் (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &tm);
		எனில் (k && *k == 0)
			செல் finish;
		இல்லையெனில் எனில் (k && parse_subseconds(k, &ret) == 0)
			செல் finish;

		திரும்பு -EINVAL;
	} இல்லையெனில் எனில் (ul_endswith(t, " ago")) {
		எழுத்து *z;

		z = strndup(t, strlen(t) - 4);
		எனில் (!z)
			திரும்பு -ENOMEM;

		r = parse_sec(z, &minus);
		விடுவி(z);
		எனில் (r < 0)
			திரும்பு r;

		செல் finish;
	}

	க்கு (i = 0; i < ARRAY_SIZE(day_nr); i++) {
		size_t skip;

		எனில் (!startswith_no_case(t, day_nr[i].name))
			தொடர்;

		skip = strlen(day_nr[i].name);
		எனில் (t[skip] != ' ')
			தொடர்;

		weekday = day_nr[i].nr;
		t += skip + 1;
		நிறுத்து;
	}

	copy = tm;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &tm);
	எனில் (k && *k == 0)
		செல் finish;
	இல்லையெனில் எனில் (k && parse_subseconds(k, &ret) == 0)
		செல் finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &tm);
	எனில் (k && *k == 0)
		செல் finish;
	இல்லையெனில் எனில் (k && parse_subseconds(k, &ret) == 0)
		செல் finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &tm);
	எனில் (k && *k == 0)
		செல் finish;
	இல்லையெனில் எனில் (k && parse_subseconds(k, &ret) == 0)
		செல் finish;

	tm = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &tm);
	எனில் (k && *k == 0) {
		tm.tm_sec = 0;
		செல் finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &tm);
	எனில் (k && *k == 0) {
		tm.tm_sec = 0;
		செல் finish;
	}

	tm = copy;
	k = strptime(t, "%y-%m-%d", &tm);
	எனில் (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		செல் finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d", &tm);
	எனில் (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		செல் finish;
	}

	tm = copy;
	k = strptime(t, "%H:%M:%S", &tm);
	எனில் (k && *k == 0)
		செல் finish;
	இல்லையெனில் எனில் (k && parse_subseconds(k, &ret) == 0)
		செல் finish;

	tm = copy;
	k = strptime(t, "%H:%M", &tm);
	எனில் (k && *k == 0) {
		tm.tm_sec = 0;
		செல் finish;
	}

	tm = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &tm);
	எனில் (k && *k == 0)
		செல் finish;
	இல்லையெனில் எனில் (k && parse_subseconds(k, &ret) == 0)
		செல் finish;

	திரும்பு -EINVAL;

 finish:
	x = mktime(&tm);
	எனில் (x == (time_t)-1)
		திரும்பு -EINVAL;

	எனில் (weekday >= 0 && tm.tm_wday != weekday)
		திரும்பு -EINVAL;

	ret += (usec_t) x * USEC_PER_SEC;

	எனில் (minus > ret)
		திரும்பு -ERANGE;
	எனில் ((ret + plus) < ret)
		திரும்பு -ERANGE;

	ret += plus;
	ret -= minus;

	*usec = ret;

	திரும்பு 0;
}

முழு ul_parse_timestamp(மாறிலி எழுத்து *t, usec_t *usec)
{
	திரும்பு parse_timestamp_reference(time(வெற்று), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
முழு get_gmtoff(மாறிலி கட்டமைப்பு tm *tp)
{
	எனில் (tp->tm_isdst < 0)
		திரும்பு 0;

#எனில் HAVE_TM_GMTOFF
	திரும்பு tp->tm_gmtoff;
#இல்லையெனில்
	கட்டமைப்பு tm tm;
	கட்டமைப்பு tm gtm;
	கட்டமைப்பு tm ltm = *tp;
	time_t lt;

	tzset();
	lt = mktime(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	எனில் (lt == (time_t) -1) {
		எனில் (! localtime_r(&lt, &tm)
			|| ((ltm.tm_sec ^ tm.tm_sec)
			    | (ltm.tm_min ^ tm.tm_min)
			    | (ltm.tm_hour ^ tm.tm_hour)
			    | (ltm.tm_mday ^ tm.tm_mday)
			    | (ltm.tm_mon ^ tm.tm_mon)
			    | (ltm.tm_year ^ tm.tm_year)))
			திரும்பு 0;
	}

	எனில் (! gmtime_r(&lt, &gtm))
		திரும்பு 0;

	/* Calculate the GMT offset, that is, the difference between the
	 * TP argument (ltm) and GMT (gtm).
	 *
	 * Compute intervening leap days correctly even if year is negative.
	 * Take care to avoid int overflow in leap day calculations, but it's OK
	 * to assume that A and B are close to each other.
	 */
	முழு a4 = (ltm.tm_year >> 2) + (1900 >> 2) - ! (ltm.tm_year & 3);
	முழு b4 = (gtm.tm_year >> 2) + (1900 >> 2) - ! (gtm.tm_year & 3);
	முழு a100 = a4 / 25 - (a4 % 25 < 0);
	முழு b100 = b4 / 25 - (b4 % 25 < 0);
	முழு a400 = a100 >> 2;
	முழு b400 = b100 >> 2;
	முழு intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

	முழு years = ltm.tm_year - gtm.tm_year;
	முழு days = (365 * years + intervening_leap_days
		    + (ltm.tm_yday - gtm.tm_yday));

	திரும்பு (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#endif
}

நிலையான முழு format_iso_time(மாறிலி கட்டமைப்பு tm *tm, uint32_t nsec, முழு flags, எழுத்து *buf, size_t bufsz)
{
	uint32_t usec = nsec / NSEC_PER_USEC;
	எழுத்து *p = buf;
	முழு len;

	எனில் (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       tm->tm_year + (long) 1900,
			       tm->tm_mon + 1, tm->tm_mday);
		எனில் (len < 0 || (size_t) len > bufsz)
			செல் err;
		bufsz -= len;
		p += len;
	}

	எனில் ((flags & ISO_DATE) && (flags & ISO_TIME)) {
		எனில் (bufsz < 1)
			செல் err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	எனில் (flags & ISO_TIME) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
			       tm->tm_min, tm->tm_sec);
		எனில் (len < 0 || (size_t) len > bufsz)
			செல் err;
		bufsz -= len;
		p += len;
	}

	எனில் (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		எனில் (len < 0 || (size_t) len > bufsz)
			செல் err;
		bufsz -= len;
		p += len;

	} இல்லையெனில் எனில் (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		எனில் (len < 0 || (size_t) len > bufsz)
			செல் err;
		bufsz -= len;
		p += len;
	} இல்லையெனில் எனில் (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		எனில் (len < 0 || (size_t) len > bufsz)
			செல் err;
		bufsz -= len;
		p += len;

	} இல்லையெனில் எனில் (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		எனில் (len < 0 || (size_t) len > bufsz)
			செல் err;
		bufsz -= len;
		p += len;
	}

	எனில் (flags & ISO_TIMEZONE) {
		முழு tmin  = get_gmtoff(tm) / 60;
		முழு zhour = tmin / 60;
		முழு zmin  = abs(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		எனில் (len < 0 || (size_t) len > bufsz)
			செல் err;
	}
	திரும்பு 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	திரும்பு -1;
}

/* timespec to ISO 8601 */
முழு strtimespec_iso(மாறிலி கட்டமைப்பு timespec *ts, முழு flags, எழுத்து *buf, size_t bufsz)
{
	கட்டமைப்பு tm tm;
	கட்டமைப்பு tm *rc;

	எனில் (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &tm);
	இல்லையெனில்
		rc = localtime_r(&ts->tv_sec, &tm);

	எனில் (rc)
		திரும்பு format_iso_time(&tm, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	திரும்பு -1;
}

/* timeval to ISO 8601 */
முழு strtimeval_iso(மாறிலி கட்டமைப்பு timeval *tv, முழு flags, எழுத்து *buf, size_t bufsz)
{
	கட்டமைப்பு timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_PER_USEC,
	};

	திரும்பு strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
முழு strtm_iso(மாறிலி கட்டமைப்பு tm *tm, முழு flags, எழுத்து *buf, size_t bufsz)
{
	திரும்பு format_iso_time(tm, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
முழு strtime_iso(மாறிலி time_t *t, முழு flags, எழுத்து *buf, size_t bufsz)
{
	கட்டமைப்பு tm tm;
	கட்டமைப்பு tm *rc;

	எனில் (flags & ISO_GMTIME)
		rc = gmtime_r(t, &tm);
	இல்லையெனில்
		rc = localtime_r(t, &tm);

	எனில் (rc)
		திரும்பு format_iso_time(&tm, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	திரும்பு -1;
}

/* relative time functions */
நிலையான உள்வரி முழு time_is_thisyear(கட்டமைப்பு tm மாறிலி *மாறிலி tm,
				   கட்டமைப்பு tm மாறிலி *மாறிலி tmnow)
{
	திரும்பு tm->tm_year == tmnow->tm_year;
}

நிலையான உள்வரி முழு time_is_today(கட்டமைப்பு tm மாறிலி *மாறிலி tm,
				கட்டமைப்பு tm மாறிலி *மாறிலி tmnow)
{
	திரும்பு (tm->tm_yday == tmnow->tm_yday &&
		time_is_thisyear(tm, tmnow));
}

முழு strtime_short(மாறிலி time_t *t, கட்டமைப்பு timeval *now, முழு flags, எழுத்து *buf, size_t bufsz)
{
	கட்டமைப்பு tm tm, tmnow;
	முழு rc = 0;

	எனில் (now->tv_sec == 0)
		gettimeofday(now, வெற்று);

	localtime_r(t, &tm);
	localtime_r(&now->tv_sec, &tmnow);

	எனில் (time_is_today(&tm, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", tm.tm_hour, tm.tm_min);
		எனில் (rc < 0 || (size_t) rc > bufsz)
			திரும்பு -1;
		rc = 1;

	} இல்லையெனில் எனில் (time_is_thisyear(&tm, &tmnow)) {
		எனில் (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = strftime(buf, bufsz, "%b%d/%H:%M", &tm);
		இல்லையெனில்
			rc = strftime(buf, bufsz, "%b%d", &tm);
	} இல்லையெனில்
		rc = strftime(buf, bufsz, "%Y-%b%d", &tm);

	திரும்பு rc <= 0 ? -1 : 0;
}

முழு strtimespec_relative(மாறிலி கட்டமைப்பு timespec *ts, எழுத்து *buf, size_t bufsz)
{
	time_t secs = ts->tv_sec;
	size_t i, parts = 0;
	முழு rc;

	எனில் (bufsz)
		buf[0] = '\0';

	நிலையான மாறிலி கட்டமைப்பு {
		மாறிலி எழுத்து * மாறிலி suffix;
		முழு width;
		int64_t secs;
	} table[] = {
		{ "y", 4, NSEC_PER_YEAR   / NSEC_PER_SEC },
		{ "d", 3, NSEC_PER_DAY    / NSEC_PER_SEC },
		{ "h", 2, NSEC_PER_HOUR   / NSEC_PER_SEC },
		{ "m", 2, NSEC_PER_MINUTE / NSEC_PER_SEC },
		{ "s", 2, NSEC_PER_SEC    / NSEC_PER_SEC },
	};

	க்கு (i = 0; i < ARRAY_SIZE(table); i++) {
		எனில் (secs >= table[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? table[i].width : 0,
				      secs / table[i].secs, table[i].suffix,
				      secs % table[i].secs ? " " : "");
			எனில் (rc < 0 || (size_t) rc > bufsz)
				செல் err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= table[i].secs;
		}
	}

	எனில் (ts->tv_nsec) {
		எனில் (ts->tv_nsec % NSEC_PER_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			எனில் (rc < 0 || (size_t) rc > bufsz)
				செல் err;
		} இல்லையெனில் {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_PER_MSEC);
			எனில் (rc < 0 || (size_t) rc > bufsz)
				செல் err;
		}
	}

	திரும்பு 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	திரும்பு -1;
}

#ifndef HAVE_TIMEGM
time_t timegm(கட்டமைப்பு tm *tm)
{
	மாறிலி எழுத்து *zone = getenv("TZ");
	time_t ret;

	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	எனில் (zone)
		setenv("TZ", zone, 1);
	இல்லையெனில்
		unsetenv("TZ");
	tzset();
	திரும்பு ret;
}
#endif /* HAVE_TIMEGM */

#ifdef TEST_PROGRAM_TIMEUTILS

நிலையான முழு run_unittest_timestamp(வெறுமை)
{
	முழு rc = EXIT_SUCCESS;
	time_t reference = 1674180427;
	நிலையான மாறிலி கட்டமைப்பு testcase {
		மாறிலி எழுத்து * மாறிலி உள்ளீடு;
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

	க்கு (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		கட்டமைப்பு testcase t = testcases[i];
		usec_t result;
		முழு r = parse_timestamp_reference(reference, t.உள்ளீடு, &result);
		எனில் (r) {
			fprintf(stderr, "Could not parse '%s'\n", t.உள்ளீடு);
			rc = EXIT_FAILURE;
		}

		எனில் (result != t.expected) {
			fprintf(stderr, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.உள்ளீடு, result, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	திரும்பு rc;
}

நிலையான முழு run_unittest_format(வெறுமை)
{
	முழு rc = EXIT_SUCCESS;
	மாறிலி கட்டமைப்பு timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	எழுத்து buf[FORMAT_TIMESTAMP_MAX];
	நிலையான மாறிலி கட்டமைப்பு testcase {
		முழு flags;
		மாறிலி எழுத்து * மாறிலி expected;
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

	க்கு (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		கட்டமைப்பு testcase t = testcases[i];
		முழு r = strtimespec_iso(&ts, t.flags, buf, அளவு(buf));
		எனில் (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		எனில் (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	திரும்பு rc;
}

நிலையான முழு run_unittest_format_relative(வெறுமை)
{
	முழு rc = EXIT_SUCCESS;
	எழுத்து buf[FORMAT_TIMESTAMP_MAX];
	நிலையான மாறிலி கட்டமைப்பு testcase {
		கட்டமைப்பு timespec ts;
		மாறிலி எழுத்து * மாறிலி expected;
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

	க்கு (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		கட்டமைப்பு testcase t = testcases[i];
		முழு r = strtimespec_relative(&t.ts, buf, அளவு(buf));
		எனில் (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		எனில் (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	திரும்பு rc;
}

முழு முதன்மை(முழு argc, எழுத்து *argv[])
{
	கட்டமைப்பு timespec ts = { 0 };
	எழுத்து buf[ISO_BUFSIZ];
	முழு r;

	எனில் (argc < 2) {
		fprintf(stderr, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	எனில் (strcmp(argv[1], "--unittest-timestamp") == 0)
		திரும்பு run_unittest_timestamp();
	இல்லையெனில் எனில் (strcmp(argv[1], "--unittest-format") == 0)
		திரும்பு run_unittest_format();
	இல்லையெனில் எனில் (strcmp(argv[1], "--unittest-format-relative") == 0)
		திரும்பு run_unittest_format_relative();

	எனில் (strcmp(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		எனில் (r)
			errx(EXIT_FAILURE, "Can not parse '%s': %s", argv[2], strerror(-r));
		ts.tv_sec = (time_t) (usec / USEC_PER_SEC);
		ts.tv_nsec = (usec % USEC_PER_SEC) * NSEC_PER_USEC;
	} இல்லையெனில் {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		எனில் (argc == 3)
			ts.tv_nsec = strtos64_or_err(argv[2], "failed to parse <usec>")
				     * NSEC_PER_USEC;
	}

	strtimespec_iso(&ts, ISO_DATE, buf, அளவு(buf));
	printf("Date: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIME, buf, அளவு(buf));
	printf("Time: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_DATE | ISO_TIME | ISO_COMMAUSEC | ISO_T,
		       buf, அளவு(buf));
	printf("Full: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIMESTAMP_DOT, buf, அளவு(buf));
	printf("Zone: '%s'\n", buf);

	strtimespec_relative(&ts, buf, அளவு(buf));
	printf("Rel:  '%s'\n", buf);

	திரும்பு EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_TIMEUTILS */
