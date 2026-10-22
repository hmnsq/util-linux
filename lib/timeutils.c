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
#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <sys/time.h>
#ଅନ୍ତର୍ଭାଵ <inttypes.h>

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "nls.h"
#ଅନ୍ତର୍ଭାଵ "strutils.h"
#ଅନ୍ତର୍ଭାଵ "timeutils.h"

#define WHITESPACE " \t\n\r"

#define streq(a,b) (strcmp((a),(b)) == 0)

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_sec(ଅଚଳ ଅକ୍ଷର *t, usec_t *usec)
{
	ସ୍ଥିର ଅଚଳ ସଂରଚନା {
		ଅଚଳ ଅକ୍ଷର *suffix;
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

	ଅଚଳ ଅକ୍ଷର *p;
	usec_t r = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା something = FALSE;

	ପ୍ରତିଜ୍ଞା(t);
	ପ୍ରତିଜ୍ଞା(usec);

	p = t;
	ପାଇଁ (;;) {
		long long l, z = 0;
		ଅକ୍ଷର *e;
		unsigned i, n = 0;

		p += strspn(p, WHITESPACE);

		ଯଦି (*p == 0) {
			ଯଦି (!something)
				ଫେରନ୍ତୁ -EINVAL;

			ଭାଙ୍ଗ;
		}

		errno = 0;
		l = strtoll(p, &e, 10);

		ଯଦି (errno > 0)
			ଫେରନ୍ତୁ -errno;

		ଯଦି (l < 0)
			ଫେରନ୍ତୁ -ERANGE;

		ଯଦି (*e == '.') {
			ଅକ୍ଷର *b = e + 1;

			errno = 0;
			z = strtoll(b, &e, 10);
			ଯଦି (errno > 0)
				ଫେରନ୍ତୁ -errno;

			ଯଦି (z < 0)
				ଫେରନ୍ତୁ -ERANGE;

			ଯଦି (e == b)
				ଫେରନ୍ତୁ -EINVAL;

			n = e - b;

		} ଅନ୍ୟଥା ଯଦି (e == p)
			ଫେରନ୍ତୁ -EINVAL;

		e += strspn(e, WHITESPACE);

		ପାଇଁ (i = 0; i < ARRAY_SIZE(table); i++)
			ଯଦି (ul_startswith(e, table[i].suffix)) {
				usec_t k = (usec_t) z * table[i].usec;

				ପାଇଁ (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *table[i].usec + k;
				p = e + strlen(table[i].suffix);

				something = TRUE;
				ଭାଙ୍ଗ;
			}

		ଯଦି (i >= ARRAY_SIZE(table))
			ଫେରନ୍ତୁ -EINVAL;

	}

	*usec = r;

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_subseconds(ଅଚଳ ଅକ୍ଷର *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t factor = USEC_PER_SEC / 10;

	ଯଦି (*t != '.' && *t != ',')
		ଫେରନ୍ତୁ -1;

	ଯେତେବେଳେ (*(++t)) {
		ଯଦି (!isdigit(*t) || factor < 1)
			ଫେରନ୍ତୁ -1;

		ret += ((usec_t) *t - '0') * factor;
		factor /= 10;
	}

	*usec = ret;
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *parse_epoch_seconds(ଅଚଳ ଅକ୍ଷର *t, ସଂରଚନା tm *tm)
{
	int64_t s;
	time_t st;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା f, c;

	f = sscanf(t, "%"SCNd64"%n", &s, &c);
	ଯଦି (f < 1)
		ଫେରନ୍ତୁ ଖାଲି;
	st = s;
	ଯଦି ((int64_t) st < s)
		ଫେରନ୍ତୁ ଖାଲି;
	ଯଦି (!localtime_r(&st, tm))
		ଫେରନ୍ତୁ ଖାଲି;
	ଫେରନ୍ତୁ t + c;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_timestamp_reference(time_t x, ଅଚଳ ଅକ୍ଷର *t, usec_t *usec)
{
	ସ୍ଥିର ଅଚଳ ସଂରଚନା {
		ଅଚଳ ଅକ୍ଷର *name;
		ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା nr;
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

	ଅଚଳ ଅକ୍ଷର *k;
	ସଂରଚନା tm tm, copy;
	usec_t plus = 0, minus = 0, ret = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା r, weekday = -1;
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

	ପ୍ରତିଜ୍ଞା(t);
	ପ୍ରତିଜ୍ଞା(usec);

	localtime_r(&x, &tm);
	tm.tm_isdst = -1;

	ଯଦି (streq(t, "now"))
		ଯାଅ finish;

	ଅନ୍ୟଥା ଯଦି (streq(t, "today")) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		ଯାଅ finish;

	} ଅନ୍ୟଥା ଯଦି (streq(t, "yesterday")) {
		tm.tm_mday--;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		ଯାଅ finish;

	} ଅନ୍ୟଥା ଯଦି (streq(t, "tomorrow")) {
		tm.tm_mday++;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		ଯାଅ finish;

	} ଅନ୍ୟଥା ଯଦି (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		ଯଦି (r < 0)
			ଫେରନ୍ତୁ r;

		ଯାଅ finish;
	} ଅନ୍ୟଥା ଯଦି (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		ଯଦି (r < 0)
			ଫେରନ୍ତୁ r;

		ଯାଅ finish;
	} ଅନ୍ୟଥା ଯଦି (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &tm);
		ଯଦି (k && *k == 0)
			ଯାଅ finish;
		ଅନ୍ୟଥା ଯଦି (k && parse_subseconds(k, &ret) == 0)
			ଯାଅ finish;

		ଫେରନ୍ତୁ -EINVAL;
	} ଅନ୍ୟଥା ଯଦି (ul_endswith(t, " ago")) {
		ଅକ୍ଷର *z;

		z = strndup(t, strlen(t) - 4);
		ଯଦି (!z)
			ଫେରନ୍ତୁ -ENOMEM;

		r = parse_sec(z, &minus);
		ମୁକ୍ତ(z);
		ଯଦି (r < 0)
			ଫେରନ୍ତୁ r;

		ଯାଅ finish;
	}

	ପାଇଁ (i = 0; i < ARRAY_SIZE(day_nr); i++) {
		size_t skip;

		ଯଦି (!startswith_no_case(t, day_nr[i].name))
			ଜାରି;

		skip = strlen(day_nr[i].name);
		ଯଦି (t[skip] != ' ')
			ଜାରି;

		weekday = day_nr[i].nr;
		t += skip + 1;
		ଭାଙ୍ଗ;
	}

	copy = tm;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &tm);
	ଯଦି (k && *k == 0)
		ଯାଅ finish;
	ଅନ୍ୟଥା ଯଦି (k && parse_subseconds(k, &ret) == 0)
		ଯାଅ finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &tm);
	ଯଦି (k && *k == 0)
		ଯାଅ finish;
	ଅନ୍ୟଥା ଯଦି (k && parse_subseconds(k, &ret) == 0)
		ଯାଅ finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &tm);
	ଯଦି (k && *k == 0)
		ଯାଅ finish;
	ଅନ୍ୟଥା ଯଦି (k && parse_subseconds(k, &ret) == 0)
		ଯାଅ finish;

	tm = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &tm);
	ଯଦି (k && *k == 0) {
		tm.tm_sec = 0;
		ଯାଅ finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &tm);
	ଯଦି (k && *k == 0) {
		tm.tm_sec = 0;
		ଯାଅ finish;
	}

	tm = copy;
	k = strptime(t, "%y-%m-%d", &tm);
	ଯଦି (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		ଯାଅ finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d", &tm);
	ଯଦି (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		ଯାଅ finish;
	}

	tm = copy;
	k = strptime(t, "%H:%M:%S", &tm);
	ଯଦି (k && *k == 0)
		ଯାଅ finish;
	ଅନ୍ୟଥା ଯଦି (k && parse_subseconds(k, &ret) == 0)
		ଯାଅ finish;

	tm = copy;
	k = strptime(t, "%H:%M", &tm);
	ଯଦି (k && *k == 0) {
		tm.tm_sec = 0;
		ଯାଅ finish;
	}

	tm = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &tm);
	ଯଦି (k && *k == 0)
		ଯାଅ finish;
	ଅନ୍ୟଥା ଯଦି (k && parse_subseconds(k, &ret) == 0)
		ଯାଅ finish;

	ଫେରନ୍ତୁ -EINVAL;

 finish:
	x = mktime(&tm);
	ଯଦି (x == (time_t)-1)
		ଫେରନ୍ତୁ -EINVAL;

	ଯଦି (weekday >= 0 && tm.tm_wday != weekday)
		ଫେରନ୍ତୁ -EINVAL;

	ret += (usec_t) x * USEC_PER_SEC;

	ଯଦି (minus > ret)
		ଫେରନ୍ତୁ -ERANGE;
	ଯଦି ((ret + plus) < ret)
		ଫେରନ୍ତୁ -ERANGE;

	ret += plus;
	ret -= minus;

	*usec = ret;

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_parse_timestamp(ଅଚଳ ଅକ୍ଷର *t, usec_t *usec)
{
	ଫେରନ୍ତୁ parse_timestamp_reference(time(ଖାଲି), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_gmtoff(ଅଚଳ ସଂରଚନା tm *tp)
{
	ଯଦି (tp->tm_isdst < 0)
		ଫେରନ୍ତୁ 0;

#ଯଦି HAVE_TM_GMTOFF
	ଫେରନ୍ତୁ tp->tm_gmtoff;
#ଅନ୍ୟଥା
	ସଂରଚନା tm tm;
	ସଂରଚନା tm gtm;
	ସଂରଚନା tm ltm = *tp;
	time_t lt;

	tzset();
	lt = mktime(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	ଯଦି (lt == (time_t) -1) {
		ଯଦି (! localtime_r(&lt, &tm)
			|| ((ltm.tm_sec ^ tm.tm_sec)
			    | (ltm.tm_min ^ tm.tm_min)
			    | (ltm.tm_hour ^ tm.tm_hour)
			    | (ltm.tm_mday ^ tm.tm_mday)
			    | (ltm.tm_mon ^ tm.tm_mon)
			    | (ltm.tm_year ^ tm.tm_year)))
			ଫେରନ୍ତୁ 0;
	}

	ଯଦି (! gmtime_r(&lt, &gtm))
		ଫେରନ୍ତୁ 0;

	/* Calculate the GMT offset, that is, the difference between the
	 * TP argument (ltm) and GMT (gtm).
	 *
	 * Compute intervening leap days correctly even if year is negative.
	 * Take care to avoid int overflow in leap day calculations, but it's OK
	 * to assume that A and B are close to each other.
	 */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା a4 = (ltm.tm_year >> 2) + (1900 >> 2) - ! (ltm.tm_year & 3);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା b4 = (gtm.tm_year >> 2) + (1900 >> 2) - ! (gtm.tm_year & 3);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା a100 = a4 / 25 - (a4 % 25 < 0);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା b100 = b4 / 25 - (b4 % 25 < 0);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା a400 = a100 >> 2;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା b400 = b100 >> 2;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା years = ltm.tm_year - gtm.tm_year;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା days = (365 * years + intervening_leap_days
		    + (ltm.tm_yday - gtm.tm_yday));

	ଫେରନ୍ତୁ (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#endif
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା format_iso_time(ଅଚଳ ସଂରଚନା tm *tm, uint32_t nsec, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ଅକ୍ଷର *buf, size_t bufsz)
{
	uint32_t usec = nsec / NSEC_PER_USEC;
	ଅକ୍ଷର *p = buf;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;

	ଯଦି (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       tm->tm_year + (long) 1900,
			       tm->tm_mon + 1, tm->tm_mday);
		ଯଦି (len < 0 || (size_t) len > bufsz)
			ଯାଅ err;
		bufsz -= len;
		p += len;
	}

	ଯଦି ((flags & ISO_DATE) && (flags & ISO_TIME)) {
		ଯଦି (bufsz < 1)
			ଯାଅ err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	ଯଦି (flags & ISO_TIME) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
			       tm->tm_min, tm->tm_sec);
		ଯଦି (len < 0 || (size_t) len > bufsz)
			ଯାଅ err;
		bufsz -= len;
		p += len;
	}

	ଯଦି (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		ଯଦି (len < 0 || (size_t) len > bufsz)
			ଯାଅ err;
		bufsz -= len;
		p += len;

	} ଅନ୍ୟଥା ଯଦି (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		ଯଦି (len < 0 || (size_t) len > bufsz)
			ଯାଅ err;
		bufsz -= len;
		p += len;
	} ଅନ୍ୟଥା ଯଦି (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		ଯଦି (len < 0 || (size_t) len > bufsz)
			ଯାଅ err;
		bufsz -= len;
		p += len;

	} ଅନ୍ୟଥା ଯଦି (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		ଯଦି (len < 0 || (size_t) len > bufsz)
			ଯାଅ err;
		bufsz -= len;
		p += len;
	}

	ଯଦି (flags & ISO_TIMEZONE) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmin  = get_gmtoff(tm) / 60;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା zhour = tmin / 60;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା zmin  = abs(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		ଯଦି (len < 0 || (size_t) len > bufsz)
			ଯାଅ err;
	}
	ଫେରନ୍ତୁ 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	ଫେରନ୍ତୁ -1;
}

/* timespec to ISO 8601 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା strtimespec_iso(ଅଚଳ ସଂରଚନା timespec *ts, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ଅକ୍ଷର *buf, size_t bufsz)
{
	ସଂରଚନା tm tm;
	ସଂରଚନା tm *rc;

	ଯଦି (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &tm);
	ଅନ୍ୟଥା
		rc = localtime_r(&ts->tv_sec, &tm);

	ଯଦି (rc)
		ଫେରନ୍ତୁ format_iso_time(&tm, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	ଫେରନ୍ତୁ -1;
}

/* timeval to ISO 8601 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା strtimeval_iso(ଅଚଳ ସଂରଚନା timeval *tv, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ଅକ୍ଷର *buf, size_t bufsz)
{
	ସଂରଚନା timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_PER_USEC,
	};

	ଫେରନ୍ତୁ strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା strtm_iso(ଅଚଳ ସଂରଚନା tm *tm, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ଅକ୍ଷର *buf, size_t bufsz)
{
	ଫେରନ୍ତୁ format_iso_time(tm, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା strtime_iso(ଅଚଳ time_t *t, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ଅକ୍ଷର *buf, size_t bufsz)
{
	ସଂରଚନା tm tm;
	ସଂରଚନା tm *rc;

	ଯଦି (flags & ISO_GMTIME)
		rc = gmtime_r(t, &tm);
	ଅନ୍ୟଥା
		rc = localtime_r(t, &tm);

	ଯଦି (rc)
		ଫେରନ୍ତୁ format_iso_time(&tm, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	ଫେରନ୍ତୁ -1;
}

/* relative time functions */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା time_is_thisyear(ସଂରଚନା tm ଅଚଳ *ଅଚଳ tm,
				   ସଂରଚନା tm ଅଚଳ *ଅଚଳ tmnow)
{
	ଫେରନ୍ତୁ tm->tm_year == tmnow->tm_year;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା time_is_today(ସଂରଚନା tm ଅଚଳ *ଅଚଳ tm,
				ସଂରଚନା tm ଅଚଳ *ଅଚଳ tmnow)
{
	ଫେରନ୍ତୁ (tm->tm_yday == tmnow->tm_yday &&
		time_is_thisyear(tm, tmnow));
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା strtime_short(ଅଚଳ time_t *t, ସଂରଚନା timeval *now, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ଅକ୍ଷର *buf, size_t bufsz)
{
	ସଂରଚନା tm tm, tmnow;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

	ଯଦି (now->tv_sec == 0)
		gettimeofday(now, ଖାଲି);

	localtime_r(t, &tm);
	localtime_r(&now->tv_sec, &tmnow);

	ଯଦି (time_is_today(&tm, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", tm.tm_hour, tm.tm_min);
		ଯଦି (rc < 0 || (size_t) rc > bufsz)
			ଫେରନ୍ତୁ -1;
		rc = 1;

	} ଅନ୍ୟଥା ଯଦି (time_is_thisyear(&tm, &tmnow)) {
		ଯଦି (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = strftime(buf, bufsz, "%b%d/%H:%M", &tm);
		ଅନ୍ୟଥା
			rc = strftime(buf, bufsz, "%b%d", &tm);
	} ଅନ୍ୟଥା
		rc = strftime(buf, bufsz, "%Y-%b%d", &tm);

	ଫେରନ୍ତୁ rc <= 0 ? -1 : 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା strtimespec_relative(ଅଚଳ ସଂରଚନା timespec *ts, ଅକ୍ଷର *buf, size_t bufsz)
{
	time_t secs = ts->tv_sec;
	size_t i, parts = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	ଯଦି (bufsz)
		buf[0] = '\0';

	ସ୍ଥିର ଅଚଳ ସଂରଚନା {
		ଅଚଳ ଅକ୍ଷର * ଅଚଳ suffix;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା width;
		int64_t secs;
	} table[] = {
		{ "y", 4, NSEC_PER_YEAR   / NSEC_PER_SEC },
		{ "d", 3, NSEC_PER_DAY    / NSEC_PER_SEC },
		{ "h", 2, NSEC_PER_HOUR   / NSEC_PER_SEC },
		{ "m", 2, NSEC_PER_MINUTE / NSEC_PER_SEC },
		{ "s", 2, NSEC_PER_SEC    / NSEC_PER_SEC },
	};

	ପାଇଁ (i = 0; i < ARRAY_SIZE(table); i++) {
		ଯଦି (secs >= table[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? table[i].width : 0,
				      secs / table[i].secs, table[i].suffix,
				      secs % table[i].secs ? " " : "");
			ଯଦି (rc < 0 || (size_t) rc > bufsz)
				ଯାଅ err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= table[i].secs;
		}
	}

	ଯଦି (ts->tv_nsec) {
		ଯଦି (ts->tv_nsec % NSEC_PER_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			ଯଦି (rc < 0 || (size_t) rc > bufsz)
				ଯାଅ err;
		} ଅନ୍ୟଥା {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_PER_MSEC);
			ଯଦି (rc < 0 || (size_t) rc > bufsz)
				ଯାଅ err;
		}
	}

	ଫେରନ୍ତୁ 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	ଫେରନ୍ତୁ -1;
}

#ifndef HAVE_TIMEGM
time_t timegm(ସଂରଚନା tm *tm)
{
	ଅଚଳ ଅକ୍ଷର *zone = getenv("TZ");
	time_t ret;

	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	ଯଦି (zone)
		setenv("TZ", zone, 1);
	ଅନ୍ୟଥା
		unsetenv("TZ");
	tzset();
	ଫେରନ୍ତୁ ret;
}
#endif /* HAVE_TIMEGM */

#ifdef TEST_PROGRAM_TIMEUTILS

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_unittest_timestamp(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = EXIT_SUCCESS;
	time_t reference = 1674180427;
	ସ୍ଥିର ଅଚଳ ସଂରଚନା testcase {
		ଅଚଳ ଅକ୍ଷର * ଅଚଳ ନିବେଶ;
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

	ପାଇଁ (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		ସଂରଚନା testcase t = testcases[i];
		usec_t result;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା r = parse_timestamp_reference(reference, t.ନିବେଶ, &result);
		ଯଦି (r) {
			fprintf(stderr, "Could not parse '%s'\n", t.ନିବେଶ);
			rc = EXIT_FAILURE;
		}

		ଯଦି (result != t.expected) {
			fprintf(stderr, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.ନିବେଶ, result, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_unittest_format(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = EXIT_SUCCESS;
	ଅଚଳ ସଂରଚନା timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	ଅକ୍ଷର buf[FORMAT_TIMESTAMP_MAX];
	ସ୍ଥିର ଅଚଳ ସଂରଚନା testcase {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags;
		ଅଚଳ ଅକ୍ଷର * ଅଚଳ expected;
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

	ପାଇଁ (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		ସଂରଚନା testcase t = testcases[i];
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା r = strtimespec_iso(&ts, t.flags, buf, ଆକାର(buf));
		ଯଦି (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		ଯଦି (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_unittest_format_relative(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = EXIT_SUCCESS;
	ଅକ୍ଷର buf[FORMAT_TIMESTAMP_MAX];
	ସ୍ଥିର ଅଚଳ ସଂରଚନା testcase {
		ସଂରଚନା timespec ts;
		ଅଚଳ ଅକ୍ଷର * ଅଚଳ expected;
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

	ପାଇଁ (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		ସଂରଚନା testcase t = testcases[i];
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା r = strtimespec_relative(&t.ts, buf, ଆକାର(buf));
		ଯଦି (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		ଯଦି (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ସଂରଚନା timespec ts = { 0 };
	ଅକ୍ଷର buf[ISO_BUFSIZ];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା r;

	ଯଦି (argc < 2) {
		fprintf(stderr, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	ଯଦି (strcmp(argv[1], "--unittest-timestamp") == 0)
		ଫେରନ୍ତୁ run_unittest_timestamp();
	ଅନ୍ୟଥା ଯଦି (strcmp(argv[1], "--unittest-format") == 0)
		ଫେରନ୍ତୁ run_unittest_format();
	ଅନ୍ୟଥା ଯଦି (strcmp(argv[1], "--unittest-format-relative") == 0)
		ଫେରନ୍ତୁ run_unittest_format_relative();

	ଯଦି (strcmp(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		ଯଦି (r)
			errx(EXIT_FAILURE, "Can not parse '%s': %s", argv[2], strerror(-r));
		ts.tv_sec = (time_t) (usec / USEC_PER_SEC);
		ts.tv_nsec = (usec % USEC_PER_SEC) * NSEC_PER_USEC;
	} ଅନ୍ୟଥା {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		ଯଦି (argc == 3)
			ts.tv_nsec = strtos64_or_err(argv[2], "failed to parse <usec>")
				     * NSEC_PER_USEC;
	}

	strtimespec_iso(&ts, ISO_DATE, buf, ଆକାର(buf));
	printf("Date: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIME, buf, ଆକାର(buf));
	printf("Time: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_DATE | ISO_TIME | ISO_COMMAUSEC | ISO_T,
		       buf, ଆକାର(buf));
	printf("Full: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIMESTAMP_DOT, buf, ଆକାର(buf));
	printf("Zone: '%s'\n", buf);

	strtimespec_relative(&ts, buf, ଆକାର(buf));
	printf("Rel:  '%s'\n", buf);

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_TIMEUTILS */
