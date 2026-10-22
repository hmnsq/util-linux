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
#अन्तर्भाव <assert.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <sys/time.h>
#अन्तर्भाव <inttypes.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "nls.h"
#अन्तर्भाव "strutils.h"
#अन्तर्भाव "timeutils.h"

#define WHITESPACE " \t\n\r"

#define streq(a,b) (strcmp((a),(b)) == 0)

स्थिर पूर्णाङ्क parse_sec(ध्रुव वर्ण *t, usec_t *usec)
{
	स्थिर ध्रुव संरचना {
		ध्रुव वर्ण *suffix;
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

	ध्रुव वर्ण *p;
	usec_t r = 0;
	पूर्णाङ्क something = FALSE;

	प्रतिज्ञा(t);
	प्रतिज्ञा(usec);

	p = t;
	अत्थाय (;;) {
		long long l, z = 0;
		वर्ण *e;
		unsigned i, n = 0;

		p += strspn(p, WHITESPACE);

		सचे (*p == 0) {
			सचे (!something)
				पटिनिवत्त -EINVAL;

			भङ्ग;
		}

		errno = 0;
		l = strtoll(p, &e, 10);

		सचे (errno > 0)
			पटिनिवत्त -errno;

		सचे (l < 0)
			पटिनिवत्त -ERANGE;

		सचे (*e == '.') {
			वर्ण *b = e + 1;

			errno = 0;
			z = strtoll(b, &e, 10);
			सचे (errno > 0)
				पटिनिवत्त -errno;

			सचे (z < 0)
				पटिनिवत्त -ERANGE;

			सचे (e == b)
				पटिनिवत्त -EINVAL;

			n = e - b;

		} अञ्ञथा सचे (e == p)
			पटिनिवत्त -EINVAL;

		e += strspn(e, WHITESPACE);

		अत्थाय (i = 0; i < ARRAY_SIZE(table); i++)
			सचे (ul_startswith(e, table[i].suffix)) {
				usec_t k = (usec_t) z * table[i].usec;

				अत्थाय (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *table[i].usec + k;
				p = e + strlen(table[i].suffix);

				something = TRUE;
				भङ्ग;
			}

		सचे (i >= ARRAY_SIZE(table))
			पटिनिवत्त -EINVAL;

	}

	*usec = r;

	पटिनिवत्त 0;
}

स्थिर पूर्णाङ्क parse_subseconds(ध्रुव वर्ण *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t factor = USEC_PER_SEC / 10;

	सचे (*t != '.' && *t != ',')
		पटिनिवत्त -1;

	याव (*(++t)) {
		सचे (!isdigit(*t) || factor < 1)
			पटिनिवत्त -1;

		ret += ((usec_t) *t - '0') * factor;
		factor /= 10;
	}

	*usec = ret;
	पटिनिवत्त 0;
}

स्थिर ध्रुव वर्ण *parse_epoch_seconds(ध्रुव वर्ण *t, संरचना tm *tm)
{
	int64_t s;
	time_t st;
	पूर्णाङ्क f, c;

	f = sscanf(t, "%"SCNd64"%n", &s, &c);
	सचे (f < 1)
		पटिनिवत्त रिक्त;
	st = s;
	सचे ((int64_t) st < s)
		पटिनिवत्त रिक्त;
	सचे (!localtime_r(&st, tm))
		पटिनिवत्त रिक्त;
	पटिनिवत्त t + c;
}

स्थिर पूर्णाङ्क parse_timestamp_reference(time_t x, ध्रुव वर्ण *t, usec_t *usec)
{
	स्थिर ध्रुव संरचना {
		ध्रुव वर्ण *name;
		ध्रुव पूर्णाङ्क nr;
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

	ध्रुव वर्ण *k;
	संरचना tm tm, copy;
	usec_t plus = 0, minus = 0, ret = 0;
	पूर्णाङ्क r, weekday = -1;
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

	प्रतिज्ञा(t);
	प्रतिज्ञा(usec);

	localtime_r(&x, &tm);
	tm.tm_isdst = -1;

	सचे (streq(t, "now"))
		गच्छ finish;

	अञ्ञथा सचे (streq(t, "today")) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		गच्छ finish;

	} अञ्ञथा सचे (streq(t, "yesterday")) {
		tm.tm_mday--;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		गच्छ finish;

	} अञ्ञथा सचे (streq(t, "tomorrow")) {
		tm.tm_mday++;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		गच्छ finish;

	} अञ्ञथा सचे (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		सचे (r < 0)
			पटिनिवत्त r;

		गच्छ finish;
	} अञ्ञथा सचे (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		सचे (r < 0)
			पटिनिवत्त r;

		गच्छ finish;
	} अञ्ञथा सचे (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &tm);
		सचे (k && *k == 0)
			गच्छ finish;
		अञ्ञथा सचे (k && parse_subseconds(k, &ret) == 0)
			गच्छ finish;

		पटिनिवत्त -EINVAL;
	} अञ्ञथा सचे (ul_endswith(t, " ago")) {
		वर्ण *z;

		z = strndup(t, strlen(t) - 4);
		सचे (!z)
			पटिनिवत्त -ENOMEM;

		r = parse_sec(z, &minus);
		मुक्त(z);
		सचे (r < 0)
			पटिनिवत्त r;

		गच्छ finish;
	}

	अत्थाय (i = 0; i < ARRAY_SIZE(day_nr); i++) {
		size_t skip;

		सचे (!startswith_no_case(t, day_nr[i].name))
			अनुवर्त;

		skip = strlen(day_nr[i].name);
		सचे (t[skip] != ' ')
			अनुवर्त;

		weekday = day_nr[i].nr;
		t += skip + 1;
		भङ्ग;
	}

	copy = tm;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &tm);
	सचे (k && *k == 0)
		गच्छ finish;
	अञ्ञथा सचे (k && parse_subseconds(k, &ret) == 0)
		गच्छ finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &tm);
	सचे (k && *k == 0)
		गच्छ finish;
	अञ्ञथा सचे (k && parse_subseconds(k, &ret) == 0)
		गच्छ finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &tm);
	सचे (k && *k == 0)
		गच्छ finish;
	अञ्ञथा सचे (k && parse_subseconds(k, &ret) == 0)
		गच्छ finish;

	tm = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &tm);
	सचे (k && *k == 0) {
		tm.tm_sec = 0;
		गच्छ finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &tm);
	सचे (k && *k == 0) {
		tm.tm_sec = 0;
		गच्छ finish;
	}

	tm = copy;
	k = strptime(t, "%y-%m-%d", &tm);
	सचे (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		गच्छ finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d", &tm);
	सचे (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		गच्छ finish;
	}

	tm = copy;
	k = strptime(t, "%H:%M:%S", &tm);
	सचे (k && *k == 0)
		गच्छ finish;
	अञ्ञथा सचे (k && parse_subseconds(k, &ret) == 0)
		गच्छ finish;

	tm = copy;
	k = strptime(t, "%H:%M", &tm);
	सचे (k && *k == 0) {
		tm.tm_sec = 0;
		गच्छ finish;
	}

	tm = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &tm);
	सचे (k && *k == 0)
		गच्छ finish;
	अञ्ञथा सचे (k && parse_subseconds(k, &ret) == 0)
		गच्छ finish;

	पटिनिवत्त -EINVAL;

 finish:
	x = mktime(&tm);
	सचे (x == (time_t)-1)
		पटिनिवत्त -EINVAL;

	सचे (weekday >= 0 && tm.tm_wday != weekday)
		पटिनिवत्त -EINVAL;

	ret += (usec_t) x * USEC_PER_SEC;

	सचे (minus > ret)
		पटिनिवत्त -ERANGE;
	सचे ((ret + plus) < ret)
		पटिनिवत्त -ERANGE;

	ret += plus;
	ret -= minus;

	*usec = ret;

	पटिनिवत्त 0;
}

पूर्णाङ्क ul_parse_timestamp(ध्रुव वर्ण *t, usec_t *usec)
{
	पटिनिवत्त parse_timestamp_reference(time(रिक्त), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
पूर्णाङ्क get_gmtoff(ध्रुव संरचना tm *tp)
{
	सचे (tp->tm_isdst < 0)
		पटिनिवत्त 0;

#सचे HAVE_TM_GMTOFF
	पटिनिवत्त tp->tm_gmtoff;
#अञ्ञथा
	संरचना tm tm;
	संरचना tm gtm;
	संरचना tm ltm = *tp;
	time_t lt;

	tzset();
	lt = mktime(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	सचे (lt == (time_t) -1) {
		सचे (! localtime_r(&lt, &tm)
			|| ((ltm.tm_sec ^ tm.tm_sec)
			    | (ltm.tm_min ^ tm.tm_min)
			    | (ltm.tm_hour ^ tm.tm_hour)
			    | (ltm.tm_mday ^ tm.tm_mday)
			    | (ltm.tm_mon ^ tm.tm_mon)
			    | (ltm.tm_year ^ tm.tm_year)))
			पटिनिवत्त 0;
	}

	सचे (! gmtime_r(&lt, &gtm))
		पटिनिवत्त 0;

	/* Calculate the GMT offset, that is, the difference between the
	 * TP argument (ltm) and GMT (gtm).
	 *
	 * Compute intervening leap days correctly even if year is negative.
	 * Take care to avoid int overflow in leap day calculations, but it's OK
	 * to assume that A and B are close to each other.
	 */
	पूर्णाङ्क a4 = (ltm.tm_year >> 2) + (1900 >> 2) - ! (ltm.tm_year & 3);
	पूर्णाङ्क b4 = (gtm.tm_year >> 2) + (1900 >> 2) - ! (gtm.tm_year & 3);
	पूर्णाङ्क a100 = a4 / 25 - (a4 % 25 < 0);
	पूर्णाङ्क b100 = b4 / 25 - (b4 % 25 < 0);
	पूर्णाङ्क a400 = a100 >> 2;
	पूर्णाङ्क b400 = b100 >> 2;
	पूर्णाङ्क intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

	पूर्णाङ्क years = ltm.tm_year - gtm.tm_year;
	पूर्णाङ्क days = (365 * years + intervening_leap_days
		    + (ltm.tm_yday - gtm.tm_yday));

	पटिनिवत्त (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#endif
}

स्थिर पूर्णाङ्क format_iso_time(ध्रुव संरचना tm *tm, uint32_t nsec, पूर्णाङ्क flags, वर्ण *buf, size_t bufsz)
{
	uint32_t usec = nsec / NSEC_PER_USEC;
	वर्ण *p = buf;
	पूर्णाङ्क len;

	सचे (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       tm->tm_year + (long) 1900,
			       tm->tm_mon + 1, tm->tm_mday);
		सचे (len < 0 || (size_t) len > bufsz)
			गच्छ err;
		bufsz -= len;
		p += len;
	}

	सचे ((flags & ISO_DATE) && (flags & ISO_TIME)) {
		सचे (bufsz < 1)
			गच्छ err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	सचे (flags & ISO_TIME) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
			       tm->tm_min, tm->tm_sec);
		सचे (len < 0 || (size_t) len > bufsz)
			गच्छ err;
		bufsz -= len;
		p += len;
	}

	सचे (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		सचे (len < 0 || (size_t) len > bufsz)
			गच्छ err;
		bufsz -= len;
		p += len;

	} अञ्ञथा सचे (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		सचे (len < 0 || (size_t) len > bufsz)
			गच्छ err;
		bufsz -= len;
		p += len;
	} अञ्ञथा सचे (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		सचे (len < 0 || (size_t) len > bufsz)
			गच्छ err;
		bufsz -= len;
		p += len;

	} अञ्ञथा सचे (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		सचे (len < 0 || (size_t) len > bufsz)
			गच्छ err;
		bufsz -= len;
		p += len;
	}

	सचे (flags & ISO_TIMEZONE) {
		पूर्णाङ्क tmin  = get_gmtoff(tm) / 60;
		पूर्णाङ्क zhour = tmin / 60;
		पूर्णाङ्क zmin  = abs(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		सचे (len < 0 || (size_t) len > bufsz)
			गच्छ err;
	}
	पटिनिवत्त 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	पटिनिवत्त -1;
}

/* timespec to ISO 8601 */
पूर्णाङ्क strtimespec_iso(ध्रुव संरचना timespec *ts, पूर्णाङ्क flags, वर्ण *buf, size_t bufsz)
{
	संरचना tm tm;
	संरचना tm *rc;

	सचे (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &tm);
	अञ्ञथा
		rc = localtime_r(&ts->tv_sec, &tm);

	सचे (rc)
		पटिनिवत्त format_iso_time(&tm, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	पटिनिवत्त -1;
}

/* timeval to ISO 8601 */
पूर्णाङ्क strtimeval_iso(ध्रुव संरचना timeval *tv, पूर्णाङ्क flags, वर्ण *buf, size_t bufsz)
{
	संरचना timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_PER_USEC,
	};

	पटिनिवत्त strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
पूर्णाङ्क strtm_iso(ध्रुव संरचना tm *tm, पूर्णाङ्क flags, वर्ण *buf, size_t bufsz)
{
	पटिनिवत्त format_iso_time(tm, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
पूर्णाङ्क strtime_iso(ध्रुव time_t *t, पूर्णाङ्क flags, वर्ण *buf, size_t bufsz)
{
	संरचना tm tm;
	संरचना tm *rc;

	सचे (flags & ISO_GMTIME)
		rc = gmtime_r(t, &tm);
	अञ्ञथा
		rc = localtime_r(t, &tm);

	सचे (rc)
		पटिनिवत्त format_iso_time(&tm, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	पटिनिवत्त -1;
}

/* relative time functions */
स्थिर अन्तःस्थ पूर्णाङ्क time_is_thisyear(संरचना tm ध्रुव *ध्रुव tm,
				   संरचना tm ध्रुव *ध्रुव tmnow)
{
	पटिनिवत्त tm->tm_year == tmnow->tm_year;
}

स्थिर अन्तःस्थ पूर्णाङ्क time_is_today(संरचना tm ध्रुव *ध्रुव tm,
				संरचना tm ध्रुव *ध्रुव tmnow)
{
	पटिनिवत्त (tm->tm_yday == tmnow->tm_yday &&
		time_is_thisyear(tm, tmnow));
}

पूर्णाङ्क strtime_short(ध्रुव time_t *t, संरचना timeval *now, पूर्णाङ्क flags, वर्ण *buf, size_t bufsz)
{
	संरचना tm tm, tmnow;
	पूर्णाङ्क rc = 0;

	सचे (now->tv_sec == 0)
		gettimeofday(now, रिक्त);

	localtime_r(t, &tm);
	localtime_r(&now->tv_sec, &tmnow);

	सचे (time_is_today(&tm, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", tm.tm_hour, tm.tm_min);
		सचे (rc < 0 || (size_t) rc > bufsz)
			पटिनिवत्त -1;
		rc = 1;

	} अञ्ञथा सचे (time_is_thisyear(&tm, &tmnow)) {
		सचे (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = strftime(buf, bufsz, "%b%d/%H:%M", &tm);
		अञ्ञथा
			rc = strftime(buf, bufsz, "%b%d", &tm);
	} अञ्ञथा
		rc = strftime(buf, bufsz, "%Y-%b%d", &tm);

	पटिनिवत्त rc <= 0 ? -1 : 0;
}

पूर्णाङ्क strtimespec_relative(ध्रुव संरचना timespec *ts, वर्ण *buf, size_t bufsz)
{
	time_t secs = ts->tv_sec;
	size_t i, parts = 0;
	पूर्णाङ्क rc;

	सचे (bufsz)
		buf[0] = '\0';

	स्थिर ध्रुव संरचना {
		ध्रुव वर्ण * ध्रुव suffix;
		पूर्णाङ्क width;
		int64_t secs;
	} table[] = {
		{ "y", 4, NSEC_PER_YEAR   / NSEC_PER_SEC },
		{ "d", 3, NSEC_PER_DAY    / NSEC_PER_SEC },
		{ "h", 2, NSEC_PER_HOUR   / NSEC_PER_SEC },
		{ "m", 2, NSEC_PER_MINUTE / NSEC_PER_SEC },
		{ "s", 2, NSEC_PER_SEC    / NSEC_PER_SEC },
	};

	अत्थाय (i = 0; i < ARRAY_SIZE(table); i++) {
		सचे (secs >= table[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? table[i].width : 0,
				      secs / table[i].secs, table[i].suffix,
				      secs % table[i].secs ? " " : "");
			सचे (rc < 0 || (size_t) rc > bufsz)
				गच्छ err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= table[i].secs;
		}
	}

	सचे (ts->tv_nsec) {
		सचे (ts->tv_nsec % NSEC_PER_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			सचे (rc < 0 || (size_t) rc > bufsz)
				गच्छ err;
		} अञ्ञथा {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_PER_MSEC);
			सचे (rc < 0 || (size_t) rc > bufsz)
				गच्छ err;
		}
	}

	पटिनिवत्त 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	पटिनिवत्त -1;
}

#ifndef HAVE_TIMEGM
time_t timegm(संरचना tm *tm)
{
	ध्रुव वर्ण *zone = getenv("TZ");
	time_t ret;

	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	सचे (zone)
		setenv("TZ", zone, 1);
	अञ्ञथा
		unsetenv("TZ");
	tzset();
	पटिनिवत्त ret;
}
#endif /* HAVE_TIMEGM */

#ifdef TEST_PROGRAM_TIMEUTILS

स्थिर पूर्णाङ्क run_unittest_timestamp(शून्य)
{
	पूर्णाङ्क rc = EXIT_SUCCESS;
	time_t reference = 1674180427;
	स्थिर ध्रुव संरचना testcase {
		ध्रुव वर्ण * ध्रुव निवेशः;
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

	अत्थाय (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		संरचना testcase t = testcases[i];
		usec_t result;
		पूर्णाङ्क r = parse_timestamp_reference(reference, t.निवेशः, &result);
		सचे (r) {
			fprintf(stderr, "Could not parse '%s'\n", t.निवेशः);
			rc = EXIT_FAILURE;
		}

		सचे (result != t.expected) {
			fprintf(stderr, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.निवेशः, result, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	पटिनिवत्त rc;
}

स्थिर पूर्णाङ्क run_unittest_format(शून्य)
{
	पूर्णाङ्क rc = EXIT_SUCCESS;
	ध्रुव संरचना timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	वर्ण buf[FORMAT_TIMESTAMP_MAX];
	स्थिर ध्रुव संरचना testcase {
		पूर्णाङ्क flags;
		ध्रुव वर्ण * ध्रुव expected;
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

	अत्थाय (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		संरचना testcase t = testcases[i];
		पूर्णाङ्क r = strtimespec_iso(&ts, t.flags, buf, परिमाण(buf));
		सचे (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		सचे (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	पटिनिवत्त rc;
}

स्थिर पूर्णाङ्क run_unittest_format_relative(शून्य)
{
	पूर्णाङ्क rc = EXIT_SUCCESS;
	वर्ण buf[FORMAT_TIMESTAMP_MAX];
	स्थिर ध्रुव संरचना testcase {
		संरचना timespec ts;
		ध्रुव वर्ण * ध्रुव expected;
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

	अत्थाय (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		संरचना testcase t = testcases[i];
		पूर्णाङ्क r = strtimespec_relative(&t.ts, buf, परिमाण(buf));
		सचे (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		सचे (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	पटिनिवत्त rc;
}

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	संरचना timespec ts = { 0 };
	वर्ण buf[ISO_BUFSIZ];
	पूर्णाङ्क r;

	सचे (argc < 2) {
		fprintf(stderr, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	सचे (strcmp(argv[1], "--unittest-timestamp") == 0)
		पटिनिवत्त run_unittest_timestamp();
	अञ्ञथा सचे (strcmp(argv[1], "--unittest-format") == 0)
		पटिनिवत्त run_unittest_format();
	अञ्ञथा सचे (strcmp(argv[1], "--unittest-format-relative") == 0)
		पटिनिवत्त run_unittest_format_relative();

	सचे (strcmp(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		सचे (r)
			errx(EXIT_FAILURE, "Can not parse '%s': %s", argv[2], strerror(-r));
		ts.tv_sec = (time_t) (usec / USEC_PER_SEC);
		ts.tv_nsec = (usec % USEC_PER_SEC) * NSEC_PER_USEC;
	} अञ्ञथा {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		सचे (argc == 3)
			ts.tv_nsec = strtos64_or_err(argv[2], "failed to parse <usec>")
				     * NSEC_PER_USEC;
	}

	strtimespec_iso(&ts, ISO_DATE, buf, परिमाण(buf));
	printf("Date: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIME, buf, परिमाण(buf));
	printf("Time: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_DATE | ISO_TIME | ISO_COMMAUSEC | ISO_T,
		       buf, परिमाण(buf));
	printf("Full: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIMESTAMP_DOT, buf, परिमाण(buf));
	printf("Zone: '%s'\n", buf);

	strtimespec_relative(&ts, buf, परिमाण(buf));
	printf("Rel:  '%s'\n", buf);

	पटिनिवत्त EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_TIMEUTILS */
