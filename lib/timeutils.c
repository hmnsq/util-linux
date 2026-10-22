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
#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <sys/time.h>
#અન્તર્ભાવ <inttypes.h>

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "nls.h"
#અન્તર્ભાવ "strutils.h"
#અન્તર્ભાવ "timeutils.h"

#define WHITESPACE " \t\n\r"

#define streq(a,b) (strcmp((a),(b)) == 0)

સ્થિર પૂર્ણાંક parse_sec(અચલ અક્ષર *t, usec_t *usec)
{
	સ્થિર અચલ સંરચના {
		અચલ અક્ષર *suffix;
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

	અચલ અક્ષર *p;
	usec_t r = 0;
	પૂર્ણાંક something = FALSE;

	પ્રતિજ્ઞા(t);
	પ્રતિજ્ઞા(usec);

	p = t;
	માટે (;;) {
		long long l, z = 0;
		અક્ષર *e;
		unsigned i, n = 0;

		p += strspn(p, WHITESPACE);

		જો (*p == 0) {
			જો (!something)
				પાછા -EINVAL;

			તોડો;
		}

		errno = 0;
		l = strtoll(p, &e, 10);

		જો (errno > 0)
			પાછા -errno;

		જો (l < 0)
			પાછા -ERANGE;

		જો (*e == '.') {
			અક્ષર *b = e + 1;

			errno = 0;
			z = strtoll(b, &e, 10);
			જો (errno > 0)
				પાછા -errno;

			જો (z < 0)
				પાછા -ERANGE;

			જો (e == b)
				પાછા -EINVAL;

			n = e - b;

		} નહીંતર જો (e == p)
			પાછા -EINVAL;

		e += strspn(e, WHITESPACE);

		માટે (i = 0; i < ARRAY_SIZE(table); i++)
			જો (ul_startswith(e, table[i].suffix)) {
				usec_t k = (usec_t) z * table[i].usec;

				માટે (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *table[i].usec + k;
				p = e + strlen(table[i].suffix);

				something = TRUE;
				તોડો;
			}

		જો (i >= ARRAY_SIZE(table))
			પાછા -EINVAL;

	}

	*usec = r;

	પાછા 0;
}

સ્થિર પૂર્ણાંક parse_subseconds(અચલ અક્ષર *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t factor = USEC_PER_SEC / 10;

	જો (*t != '.' && *t != ',')
		પાછા -1;

	જ્યારે (*(++t)) {
		જો (!isdigit(*t) || factor < 1)
			પાછા -1;

		ret += ((usec_t) *t - '0') * factor;
		factor /= 10;
	}

	*usec = ret;
	પાછા 0;
}

સ્થિર અચલ અક્ષર *parse_epoch_seconds(અચલ અક્ષર *t, સંરચના tm *tm)
{
	int64_t s;
	time_t st;
	પૂર્ણાંક f, c;

	f = sscanf(t, "%"SCNd64"%n", &s, &c);
	જો (f < 1)
		પાછા ખાલી;
	st = s;
	જો ((int64_t) st < s)
		પાછા ખાલી;
	જો (!localtime_r(&st, tm))
		પાછા ખાલી;
	પાછા t + c;
}

સ્થિર પૂર્ણાંક parse_timestamp_reference(time_t x, અચલ અક્ષર *t, usec_t *usec)
{
	સ્થિર અચલ સંરચના {
		અચલ અક્ષર *name;
		અચલ પૂર્ણાંક nr;
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

	અચલ અક્ષર *k;
	સંરચના tm tm, copy;
	usec_t plus = 0, minus = 0, ret = 0;
	પૂર્ણાંક r, weekday = -1;
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

	પ્રતિજ્ઞા(t);
	પ્રતિજ્ઞા(usec);

	localtime_r(&x, &tm);
	tm.tm_isdst = -1;

	જો (streq(t, "now"))
		જાઓ finish;

	નહીંતર જો (streq(t, "today")) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		જાઓ finish;

	} નહીંતર જો (streq(t, "yesterday")) {
		tm.tm_mday--;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		જાઓ finish;

	} નહીંતર જો (streq(t, "tomorrow")) {
		tm.tm_mday++;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		જાઓ finish;

	} નહીંતર જો (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		જો (r < 0)
			પાછા r;

		જાઓ finish;
	} નહીંતર જો (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		જો (r < 0)
			પાછા r;

		જાઓ finish;
	} નહીંતર જો (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &tm);
		જો (k && *k == 0)
			જાઓ finish;
		નહીંતર જો (k && parse_subseconds(k, &ret) == 0)
			જાઓ finish;

		પાછા -EINVAL;
	} નહીંતર જો (ul_endswith(t, " ago")) {
		અક્ષર *z;

		z = strndup(t, strlen(t) - 4);
		જો (!z)
			પાછા -ENOMEM;

		r = parse_sec(z, &minus);
		મુક્ત(z);
		જો (r < 0)
			પાછા r;

		જાઓ finish;
	}

	માટે (i = 0; i < ARRAY_SIZE(day_nr); i++) {
		size_t skip;

		જો (!startswith_no_case(t, day_nr[i].name))
			ચાલુ;

		skip = strlen(day_nr[i].name);
		જો (t[skip] != ' ')
			ચાલુ;

		weekday = day_nr[i].nr;
		t += skip + 1;
		તોડો;
	}

	copy = tm;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &tm);
	જો (k && *k == 0)
		જાઓ finish;
	નહીંતર જો (k && parse_subseconds(k, &ret) == 0)
		જાઓ finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &tm);
	જો (k && *k == 0)
		જાઓ finish;
	નહીંતર જો (k && parse_subseconds(k, &ret) == 0)
		જાઓ finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &tm);
	જો (k && *k == 0)
		જાઓ finish;
	નહીંતર જો (k && parse_subseconds(k, &ret) == 0)
		જાઓ finish;

	tm = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &tm);
	જો (k && *k == 0) {
		tm.tm_sec = 0;
		જાઓ finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &tm);
	જો (k && *k == 0) {
		tm.tm_sec = 0;
		જાઓ finish;
	}

	tm = copy;
	k = strptime(t, "%y-%m-%d", &tm);
	જો (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		જાઓ finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d", &tm);
	જો (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		જાઓ finish;
	}

	tm = copy;
	k = strptime(t, "%H:%M:%S", &tm);
	જો (k && *k == 0)
		જાઓ finish;
	નહીંતર જો (k && parse_subseconds(k, &ret) == 0)
		જાઓ finish;

	tm = copy;
	k = strptime(t, "%H:%M", &tm);
	જો (k && *k == 0) {
		tm.tm_sec = 0;
		જાઓ finish;
	}

	tm = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &tm);
	જો (k && *k == 0)
		જાઓ finish;
	નહીંતર જો (k && parse_subseconds(k, &ret) == 0)
		જાઓ finish;

	પાછા -EINVAL;

 finish:
	x = mktime(&tm);
	જો (x == (time_t)-1)
		પાછા -EINVAL;

	જો (weekday >= 0 && tm.tm_wday != weekday)
		પાછા -EINVAL;

	ret += (usec_t) x * USEC_PER_SEC;

	જો (minus > ret)
		પાછા -ERANGE;
	જો ((ret + plus) < ret)
		પાછા -ERANGE;

	ret += plus;
	ret -= minus;

	*usec = ret;

	પાછા 0;
}

પૂર્ણાંક ul_parse_timestamp(અચલ અક્ષર *t, usec_t *usec)
{
	પાછા parse_timestamp_reference(time(ખાલી), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
પૂર્ણાંક get_gmtoff(અચલ સંરચના tm *tp)
{
	જો (tp->tm_isdst < 0)
		પાછા 0;

#જો HAVE_TM_GMTOFF
	પાછા tp->tm_gmtoff;
#નહીંતર
	સંરચના tm tm;
	સંરચના tm gtm;
	સંરચના tm ltm = *tp;
	time_t lt;

	tzset();
	lt = mktime(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	જો (lt == (time_t) -1) {
		જો (! localtime_r(&lt, &tm)
			|| ((ltm.tm_sec ^ tm.tm_sec)
			    | (ltm.tm_min ^ tm.tm_min)
			    | (ltm.tm_hour ^ tm.tm_hour)
			    | (ltm.tm_mday ^ tm.tm_mday)
			    | (ltm.tm_mon ^ tm.tm_mon)
			    | (ltm.tm_year ^ tm.tm_year)))
			પાછા 0;
	}

	જો (! gmtime_r(&lt, &gtm))
		પાછા 0;

	/* Calculate the GMT offset, that is, the difference between the
	 * TP argument (ltm) and GMT (gtm).
	 *
	 * Compute intervening leap days correctly even if year is negative.
	 * Take care to avoid int overflow in leap day calculations, but it's OK
	 * to assume that A and B are close to each other.
	 */
	પૂર્ણાંક a4 = (ltm.tm_year >> 2) + (1900 >> 2) - ! (ltm.tm_year & 3);
	પૂર્ણાંક b4 = (gtm.tm_year >> 2) + (1900 >> 2) - ! (gtm.tm_year & 3);
	પૂર્ણાંક a100 = a4 / 25 - (a4 % 25 < 0);
	પૂર્ણાંક b100 = b4 / 25 - (b4 % 25 < 0);
	પૂર્ણાંક a400 = a100 >> 2;
	પૂર્ણાંક b400 = b100 >> 2;
	પૂર્ણાંક intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

	પૂર્ણાંક years = ltm.tm_year - gtm.tm_year;
	પૂર્ણાંક days = (365 * years + intervening_leap_days
		    + (ltm.tm_yday - gtm.tm_yday));

	પાછા (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#endif
}

સ્થિર પૂર્ણાંક format_iso_time(અચલ સંરચના tm *tm, uint32_t nsec, પૂર્ણાંક flags, અક્ષર *buf, size_t bufsz)
{
	uint32_t usec = nsec / NSEC_PER_USEC;
	અક્ષર *p = buf;
	પૂર્ણાંક len;

	જો (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       tm->tm_year + (long) 1900,
			       tm->tm_mon + 1, tm->tm_mday);
		જો (len < 0 || (size_t) len > bufsz)
			જાઓ err;
		bufsz -= len;
		p += len;
	}

	જો ((flags & ISO_DATE) && (flags & ISO_TIME)) {
		જો (bufsz < 1)
			જાઓ err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	જો (flags & ISO_TIME) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
			       tm->tm_min, tm->tm_sec);
		જો (len < 0 || (size_t) len > bufsz)
			જાઓ err;
		bufsz -= len;
		p += len;
	}

	જો (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		જો (len < 0 || (size_t) len > bufsz)
			જાઓ err;
		bufsz -= len;
		p += len;

	} નહીંતર જો (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		જો (len < 0 || (size_t) len > bufsz)
			જાઓ err;
		bufsz -= len;
		p += len;
	} નહીંતર જો (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		જો (len < 0 || (size_t) len > bufsz)
			જાઓ err;
		bufsz -= len;
		p += len;

	} નહીંતર જો (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		જો (len < 0 || (size_t) len > bufsz)
			જાઓ err;
		bufsz -= len;
		p += len;
	}

	જો (flags & ISO_TIMEZONE) {
		પૂર્ણાંક tmin  = get_gmtoff(tm) / 60;
		પૂર્ણાંક zhour = tmin / 60;
		પૂર્ણાંક zmin  = abs(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		જો (len < 0 || (size_t) len > bufsz)
			જાઓ err;
	}
	પાછા 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	પાછા -1;
}

/* timespec to ISO 8601 */
પૂર્ણાંક strtimespec_iso(અચલ સંરચના timespec *ts, પૂર્ણાંક flags, અક્ષર *buf, size_t bufsz)
{
	સંરચના tm tm;
	સંરચના tm *rc;

	જો (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &tm);
	નહીંતર
		rc = localtime_r(&ts->tv_sec, &tm);

	જો (rc)
		પાછા format_iso_time(&tm, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	પાછા -1;
}

/* timeval to ISO 8601 */
પૂર્ણાંક strtimeval_iso(અચલ સંરચના timeval *tv, પૂર્ણાંક flags, અક્ષર *buf, size_t bufsz)
{
	સંરચના timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_PER_USEC,
	};

	પાછા strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
પૂર્ણાંક strtm_iso(અચલ સંરચના tm *tm, પૂર્ણાંક flags, અક્ષર *buf, size_t bufsz)
{
	પાછા format_iso_time(tm, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
પૂર્ણાંક strtime_iso(અચલ time_t *t, પૂર્ણાંક flags, અક્ષર *buf, size_t bufsz)
{
	સંરચના tm tm;
	સંરચના tm *rc;

	જો (flags & ISO_GMTIME)
		rc = gmtime_r(t, &tm);
	નહીંતર
		rc = localtime_r(t, &tm);

	જો (rc)
		પાછા format_iso_time(&tm, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	પાછા -1;
}

/* relative time functions */
સ્થિર અંતઃરેખ પૂર્ણાંક time_is_thisyear(સંરચના tm અચલ *અચલ tm,
				   સંરચના tm અચલ *અચલ tmnow)
{
	પાછા tm->tm_year == tmnow->tm_year;
}

સ્થિર અંતઃરેખ પૂર્ણાંક time_is_today(સંરચના tm અચલ *અચલ tm,
				સંરચના tm અચલ *અચલ tmnow)
{
	પાછા (tm->tm_yday == tmnow->tm_yday &&
		time_is_thisyear(tm, tmnow));
}

પૂર્ણાંક strtime_short(અચલ time_t *t, સંરચના timeval *now, પૂર્ણાંક flags, અક્ષર *buf, size_t bufsz)
{
	સંરચના tm tm, tmnow;
	પૂર્ણાંક rc = 0;

	જો (now->tv_sec == 0)
		gettimeofday(now, ખાલી);

	localtime_r(t, &tm);
	localtime_r(&now->tv_sec, &tmnow);

	જો (time_is_today(&tm, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", tm.tm_hour, tm.tm_min);
		જો (rc < 0 || (size_t) rc > bufsz)
			પાછા -1;
		rc = 1;

	} નહીંતર જો (time_is_thisyear(&tm, &tmnow)) {
		જો (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = strftime(buf, bufsz, "%b%d/%H:%M", &tm);
		નહીંતર
			rc = strftime(buf, bufsz, "%b%d", &tm);
	} નહીંતર
		rc = strftime(buf, bufsz, "%Y-%b%d", &tm);

	પાછા rc <= 0 ? -1 : 0;
}

પૂર્ણાંક strtimespec_relative(અચલ સંરચના timespec *ts, અક્ષર *buf, size_t bufsz)
{
	time_t secs = ts->tv_sec;
	size_t i, parts = 0;
	પૂર્ણાંક rc;

	જો (bufsz)
		buf[0] = '\0';

	સ્થિર અચલ સંરચના {
		અચલ અક્ષર * અચલ suffix;
		પૂર્ણાંક width;
		int64_t secs;
	} table[] = {
		{ "y", 4, NSEC_PER_YEAR   / NSEC_PER_SEC },
		{ "d", 3, NSEC_PER_DAY    / NSEC_PER_SEC },
		{ "h", 2, NSEC_PER_HOUR   / NSEC_PER_SEC },
		{ "m", 2, NSEC_PER_MINUTE / NSEC_PER_SEC },
		{ "s", 2, NSEC_PER_SEC    / NSEC_PER_SEC },
	};

	માટે (i = 0; i < ARRAY_SIZE(table); i++) {
		જો (secs >= table[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? table[i].width : 0,
				      secs / table[i].secs, table[i].suffix,
				      secs % table[i].secs ? " " : "");
			જો (rc < 0 || (size_t) rc > bufsz)
				જાઓ err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= table[i].secs;
		}
	}

	જો (ts->tv_nsec) {
		જો (ts->tv_nsec % NSEC_PER_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			જો (rc < 0 || (size_t) rc > bufsz)
				જાઓ err;
		} નહીંતર {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_PER_MSEC);
			જો (rc < 0 || (size_t) rc > bufsz)
				જાઓ err;
		}
	}

	પાછા 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	પાછા -1;
}

#ifndef HAVE_TIMEGM
time_t timegm(સંરચના tm *tm)
{
	અચલ અક્ષર *zone = getenv("TZ");
	time_t ret;

	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	જો (zone)
		setenv("TZ", zone, 1);
	નહીંતર
		unsetenv("TZ");
	tzset();
	પાછા ret;
}
#endif /* HAVE_TIMEGM */

#ifdef TEST_PROGRAM_TIMEUTILS

સ્થિર પૂર્ણાંક run_unittest_timestamp(શૂન્ય)
{
	પૂર્ણાંક rc = EXIT_SUCCESS;
	time_t reference = 1674180427;
	સ્થિર અચલ સંરચના testcase {
		અચલ અક્ષર * અચલ આગત;
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

	માટે (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		સંરચના testcase t = testcases[i];
		usec_t result;
		પૂર્ણાંક r = parse_timestamp_reference(reference, t.આગત, &result);
		જો (r) {
			fprintf(stderr, "Could not parse '%s'\n", t.આગત);
			rc = EXIT_FAILURE;
		}

		જો (result != t.expected) {
			fprintf(stderr, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.આગત, result, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	પાછા rc;
}

સ્થિર પૂર્ણાંક run_unittest_format(શૂન્ય)
{
	પૂર્ણાંક rc = EXIT_SUCCESS;
	અચલ સંરચના timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	અક્ષર buf[FORMAT_TIMESTAMP_MAX];
	સ્થિર અચલ સંરચના testcase {
		પૂર્ણાંક flags;
		અચલ અક્ષર * અચલ expected;
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

	માટે (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		સંરચના testcase t = testcases[i];
		પૂર્ણાંક r = strtimespec_iso(&ts, t.flags, buf, કદ(buf));
		જો (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		જો (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	પાછા rc;
}

સ્થિર પૂર્ણાંક run_unittest_format_relative(શૂન્ય)
{
	પૂર્ણાંક rc = EXIT_SUCCESS;
	અક્ષર buf[FORMAT_TIMESTAMP_MAX];
	સ્થિર અચલ સંરચના testcase {
		સંરચના timespec ts;
		અચલ અક્ષર * અચલ expected;
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

	માટે (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		સંરચના testcase t = testcases[i];
		પૂર્ણાંક r = strtimespec_relative(&t.ts, buf, કદ(buf));
		જો (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		જો (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	પાછા rc;
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[])
{
	સંરચના timespec ts = { 0 };
	અક્ષર buf[ISO_BUFSIZ];
	પૂર્ણાંક r;

	જો (argc < 2) {
		fprintf(stderr, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	જો (strcmp(argv[1], "--unittest-timestamp") == 0)
		પાછા run_unittest_timestamp();
	નહીંતર જો (strcmp(argv[1], "--unittest-format") == 0)
		પાછા run_unittest_format();
	નહીંતર જો (strcmp(argv[1], "--unittest-format-relative") == 0)
		પાછા run_unittest_format_relative();

	જો (strcmp(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		જો (r)
			errx(EXIT_FAILURE, "Can not parse '%s': %s", argv[2], strerror(-r));
		ts.tv_sec = (time_t) (usec / USEC_PER_SEC);
		ts.tv_nsec = (usec % USEC_PER_SEC) * NSEC_PER_USEC;
	} નહીંતર {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		જો (argc == 3)
			ts.tv_nsec = strtos64_or_err(argv[2], "failed to parse <usec>")
				     * NSEC_PER_USEC;
	}

	strtimespec_iso(&ts, ISO_DATE, buf, કદ(buf));
	printf("Date: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIME, buf, કદ(buf));
	printf("Time: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_DATE | ISO_TIME | ISO_COMMAUSEC | ISO_T,
		       buf, કદ(buf));
	printf("Full: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIMESTAMP_DOT, buf, કદ(buf));
	printf("Zone: '%s'\n", buf);

	strtimespec_relative(&ts, buf, કદ(buf));
	printf("Rel:  '%s'\n", buf);

	પાછા EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_TIMEUTILS */
