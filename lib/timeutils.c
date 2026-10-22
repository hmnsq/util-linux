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

#परिभाषय WHITESPACE " \t\n\r"

#परिभाषय streq(a,b) (वाक्यं_तुलय((a),(b)) == 0)

स्थिर पूर्णाङ्क parse_sec(ध्रुव वर्ण *t, usec_t *usec)
{
	स्थिर ध्रुव संरचना {
		ध्रुव वर्ण *suffix;
		usec_t usec;
	} सारणी[] = {
		{ "seconds",	USEC_प्रत्येकम्_SEC },
		{ "second",	USEC_प्रत्येकम्_SEC },
		{ "sec",	USEC_प्रत्येकम्_SEC },
		{ "s",		USEC_प्रत्येकम्_SEC },
		{ "minutes",	USEC_प्रत्येकम्_MINUTE },
		{ "minute",	USEC_प्रत्येकम्_MINUTE },
		{ "min",	USEC_प्रत्येकम्_MINUTE },
		{ "months",	USEC_प्रत्येकम्_MONTH },
		{ "month",	USEC_प्रत्येकम्_MONTH },
		{ "msec",	USEC_प्रत्येकम्_MSEC },
		{ "ms",		USEC_प्रत्येकम्_MSEC },
		{ "m",		USEC_प्रत्येकम्_MINUTE },
		{ "hours",	USEC_प्रत्येकम्_HOUR },
		{ "hour",	USEC_प्रत्येकम्_HOUR },
		{ "hr",		USEC_प्रत्येकम्_HOUR },
		{ "h",		USEC_प्रत्येकम्_HOUR },
		{ "days",	USEC_प्रत्येकम्_DAY },
		{ "day",	USEC_प्रत्येकम्_DAY },
		{ "d",		USEC_प्रत्येकम्_DAY },
		{ "weeks",	USEC_प्रत्येकम्_WEEK },
		{ "week",	USEC_प्रत्येकम्_WEEK },
		{ "w",		USEC_प्रत्येकम्_WEEK },
		{ "years",	USEC_प्रत्येकम्_YEAR },
		{ "year",	USEC_प्रत्येकम्_YEAR },
		{ "y",		USEC_प्रत्येकम्_YEAR },
		{ "usec",	1ULL },
		{ "us",		1ULL },
		{ "",		USEC_प्रत्येकम्_SEC },	/* default is sec */
	};

	ध्रुव वर्ण *p;
	usec_t r = 0;
	पूर्णाङ्क something = FALSE;

	प्रतिज्ञा(t);
	प्रतिज्ञा(usec);

	p = t;
	प्रति (;;) {
		दीर्घ दीर्घ l, z = 0;
		वर्ण *e;
		अचिह्नित i, n = 0;

		p += strspn(p, WHITESPACE);

		यदि (*p == 0) {
			यदि (!something)
				प्रत्यागम -EINVAL;

			भङ्ग;
		}

		errno = 0;
		l = strtoll(p, &e, 10);

		यदि (errno > 0)
			प्रत्यागम -errno;

		यदि (l < 0)
			प्रत्यागम -ERANGE;

		यदि (*e == '.') {
			वर्ण *b = e + 1;

			errno = 0;
			z = strtoll(b, &e, 10);
			यदि (errno > 0)
				प्रत्यागम -errno;

			यदि (z < 0)
				प्रत्यागम -ERANGE;

			यदि (e == b)
				प्रत्यागम -EINVAL;

			n = e - b;

		} अन्यथा यदि (e == p)
			प्रत्यागम -EINVAL;

		e += strspn(e, WHITESPACE);

		प्रति (i = 0; i < ARRAY_आकार(सारणी); i++)
			यदि (ul_startswith(e, सारणी[i].suffix)) {
				usec_t k = (usec_t) z * सारणी[i].usec;

				प्रति (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *सारणी[i].usec + k;
				p = e + वाक्यदैर्घ्यम्(सारणी[i].suffix);

				something = TRUE;
				भङ्ग;
			}

		यदि (i >= ARRAY_आकार(सारणी))
			प्रत्यागम -EINVAL;

	}

	*usec = r;

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क parse_subseconds(ध्रुव वर्ण *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t गुणकः = USEC_प्रत्येकम्_SEC / 10;

	यदि (*t != '.' && *t != ',')
		प्रत्यागम -1;

	यावत् (*(++t)) {
		यदि (!अङ्कः_वा(*t) || गुणकः < 1)
			प्रत्यागम -1;

		ret += ((usec_t) *t - '0') * गुणकः;
		गुणकः /= 10;
	}

	*usec = ret;
	प्रत्यागम 0;
}

स्थिर ध्रुव वर्ण *parse_epoch_seconds(ध्रुव वर्ण *t, संरचना tm *tm)
{
	int64_t s;
	time_t st;
	पूर्णाङ्क f, c;

	f = वाक्यात्_पठ(t, "%"SCNd64"%n", &s, &c);
	यदि (f < 1)
		प्रत्यागम रिक्त;
	st = s;
	यदि ((int64_t) st < s)
		प्रत्यागम रिक्त;
	यदि (!localtime_r(&st, tm))
		प्रत्यागम रिक्त;
	प्रत्यागम t + c;
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
	अचिह्नित i;

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

	यदि (streq(t, "now"))
		गच्छ finish;

	अन्यथा यदि (streq(t, "today")) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		गच्छ finish;

	} अन्यथा यदि (streq(t, "yesterday")) {
		tm.tm_mday--;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		गच्छ finish;

	} अन्यथा यदि (streq(t, "tomorrow")) {
		tm.tm_mday++;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		गच्छ finish;

	} अन्यथा यदि (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		यदि (r < 0)
			प्रत्यागम r;

		गच्छ finish;
	} अन्यथा यदि (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		यदि (r < 0)
			प्रत्यागम r;

		गच्छ finish;
	} अन्यथा यदि (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &tm);
		यदि (k && *k == 0)
			गच्छ finish;
		अन्यथा यदि (k && parse_subseconds(k, &ret) == 0)
			गच्छ finish;

		प्रत्यागम -EINVAL;
	} अन्यथा यदि (ul_endswith(t, " ago")) {
		वर्ण *z;

		z = strndup(t, वाक्यदैर्घ्यम्(t) - 4);
		यदि (!z)
			प्रत्यागम -ENOMEM;

		r = parse_sec(z, &minus);
		मुक्त(z);
		यदि (r < 0)
			प्रत्यागम r;

		गच्छ finish;
	}

	प्रति (i = 0; i < ARRAY_आकार(day_nr); i++) {
		size_t skip;

		यदि (!startswith_no_case(t, day_nr[i].name))
			अनुवर्त;

		skip = वाक्यदैर्घ्यम्(day_nr[i].name);
		यदि (t[skip] != ' ')
			अनुवर्त;

		weekday = day_nr[i].nr;
		t += skip + 1;
		भङ्ग;
	}

	copy = tm;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &tm);
	यदि (k && *k == 0)
		गच्छ finish;
	अन्यथा यदि (k && parse_subseconds(k, &ret) == 0)
		गच्छ finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &tm);
	यदि (k && *k == 0)
		गच्छ finish;
	अन्यथा यदि (k && parse_subseconds(k, &ret) == 0)
		गच्छ finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &tm);
	यदि (k && *k == 0)
		गच्छ finish;
	अन्यथा यदि (k && parse_subseconds(k, &ret) == 0)
		गच्छ finish;

	tm = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &tm);
	यदि (k && *k == 0) {
		tm.tm_sec = 0;
		गच्छ finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &tm);
	यदि (k && *k == 0) {
		tm.tm_sec = 0;
		गच्छ finish;
	}

	tm = copy;
	k = strptime(t, "%y-%m-%d", &tm);
	यदि (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		गच्छ finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d", &tm);
	यदि (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		गच्छ finish;
	}

	tm = copy;
	k = strptime(t, "%H:%M:%S", &tm);
	यदि (k && *k == 0)
		गच्छ finish;
	अन्यथा यदि (k && parse_subseconds(k, &ret) == 0)
		गच्छ finish;

	tm = copy;
	k = strptime(t, "%H:%M", &tm);
	यदि (k && *k == 0) {
		tm.tm_sec = 0;
		गच्छ finish;
	}

	tm = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &tm);
	यदि (k && *k == 0)
		गच्छ finish;
	अन्यथा यदि (k && parse_subseconds(k, &ret) == 0)
		गच्छ finish;

	प्रत्यागम -EINVAL;

 finish:
	x = mktime(&tm);
	यदि (x == (time_t)-1)
		प्रत्यागम -EINVAL;

	यदि (weekday >= 0 && tm.tm_wday != weekday)
		प्रत्यागम -EINVAL;

	ret += (usec_t) x * USEC_प्रत्येकम्_SEC;

	यदि (minus > ret)
		प्रत्यागम -ERANGE;
	यदि ((ret + plus) < ret)
		प्रत्यागम -ERANGE;

	ret += plus;
	ret -= minus;

	*usec = ret;

	प्रत्यागम 0;
}

पूर्णाङ्क ul_parse_timestamp(ध्रुव वर्ण *t, usec_t *usec)
{
	प्रत्यागम parse_timestamp_reference(time(रिक्त), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
पूर्णाङ्क get_gmtoff(ध्रुव संरचना tm *tp)
{
	यदि (tp->tm_isdst < 0)
		प्रत्यागम 0;

#यदि HAVE_TM_GMTOFF
	प्रत्यागम tp->tm_gmtoff;
#अन्यथा
	संरचना tm tm;
	संरचना tm gtm;
	संरचना tm ltm = *tp;
	time_t lt;

	tzset();
	lt = mktime(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	यदि (lt == (time_t) -1) {
		यदि (! localtime_r(&lt, &tm)
			|| ((ltm.tm_sec ^ tm.tm_sec)
			    | (ltm.tm_min ^ tm.tm_min)
			    | (ltm.tm_hour ^ tm.tm_hour)
			    | (ltm.tm_mday ^ tm.tm_mday)
			    | (ltm.tm_mon ^ tm.tm_mon)
			    | (ltm.tm_year ^ tm.tm_year)))
			प्रत्यागम 0;
	}

	यदि (! gmtime_r(&lt, &gtm))
		प्रत्यागम 0;

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

	प्रत्यागम (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#यदि_अन्तः
}

स्थिर पूर्णाङ्क format_iso_time(ध्रुव संरचना tm *tm, uint32_t nsec, पूर्णाङ्क flags, वर्ण *buf, size_t bufsz)
{
	uint32_t usec = nsec / NSEC_प्रत्येकम्_USEC;
	वर्ण *p = buf;
	पूर्णाङ्क len;

	यदि (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       tm->tm_year + (दीर्घ) 1900,
			       tm->tm_mon + 1, tm->tm_mday);
		यदि (len < 0 || (size_t) len > bufsz)
			गच्छ err;
		bufsz -= len;
		p += len;
	}

	यदि ((flags & ISO_DATE) && (flags & ISO_TIME)) {
		यदि (bufsz < 1)
			गच्छ err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	यदि (flags & ISO_TIME) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
			       tm->tm_min, tm->tm_sec);
		यदि (len < 0 || (size_t) len > bufsz)
			गच्छ err;
		bufsz -= len;
		p += len;
	}

	यदि (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		यदि (len < 0 || (size_t) len > bufsz)
			गच्छ err;
		bufsz -= len;
		p += len;

	} अन्यथा यदि (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		यदि (len < 0 || (size_t) len > bufsz)
			गच्छ err;
		bufsz -= len;
		p += len;
	} अन्यथा यदि (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		यदि (len < 0 || (size_t) len > bufsz)
			गच्छ err;
		bufsz -= len;
		p += len;

	} अन्यथा यदि (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		यदि (len < 0 || (size_t) len > bufsz)
			गच्छ err;
		bufsz -= len;
		p += len;
	}

	यदि (flags & ISO_TIMEZONE) {
		पूर्णाङ्क tmin  = get_gmtoff(tm) / 60;
		पूर्णाङ्क zhour = tmin / 60;
		पूर्णाङ्क zmin  = निरपेक्षम्(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		यदि (len < 0 || (size_t) len > bufsz)
			गच्छ err;
	}
	प्रत्यागम 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	प्रत्यागम -1;
}

/* timespec to ISO 8601 */
पूर्णाङ्क strtimespec_iso(ध्रुव संरचना timespec *ts, पूर्णाङ्क flags, वर्ण *buf, size_t bufsz)
{
	संरचना tm tm;
	संरचना tm *rc;

	यदि (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &tm);
	अन्यथा
		rc = localtime_r(&ts->tv_sec, &tm);

	यदि (rc)
		प्रत्यागम format_iso_time(&tm, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	प्रत्यागम -1;
}

/* timeval to ISO 8601 */
पूर्णाङ्क strtimeval_iso(ध्रुव संरचना timeval *tv, पूर्णाङ्क flags, वर्ण *buf, size_t bufsz)
{
	संरचना timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_प्रत्येकम्_USEC,
	};

	प्रत्यागम strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
पूर्णाङ्क strtm_iso(ध्रुव संरचना tm *tm, पूर्णाङ्क flags, वर्ण *buf, size_t bufsz)
{
	प्रत्यागम format_iso_time(tm, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
पूर्णाङ्क strtime_iso(ध्रुव time_t *t, पूर्णाङ्क flags, वर्ण *buf, size_t bufsz)
{
	संरचना tm tm;
	संरचना tm *rc;

	यदि (flags & ISO_GMTIME)
		rc = gmtime_r(t, &tm);
	अन्यथा
		rc = localtime_r(t, &tm);

	यदि (rc)
		प्रत्यागम format_iso_time(&tm, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	प्रत्यागम -1;
}

/* relative time functions */
स्थिर अन्तःस्थ पूर्णाङ्क time_is_thisyear(संरचना tm ध्रुव *ध्रुव tm,
				   संरचना tm ध्रुव *ध्रुव tmnow)
{
	प्रत्यागम tm->tm_year == tmnow->tm_year;
}

स्थिर अन्तःस्थ पूर्णाङ्क time_is_today(संरचना tm ध्रुव *ध्रुव tm,
				संरचना tm ध्रुव *ध्रुव tmnow)
{
	प्रत्यागम (tm->tm_yday == tmnow->tm_yday &&
		time_is_thisyear(tm, tmnow));
}

पूर्णाङ्क strtime_short(ध्रुव time_t *t, संरचना timeval *now, पूर्णाङ्क flags, वर्ण *buf, size_t bufsz)
{
	संरचना tm tm, tmnow;
	पूर्णाङ्क rc = 0;

	यदि (now->tv_sec == 0)
		gettimeofday(now, रिक्त);

	localtime_r(t, &tm);
	localtime_r(&now->tv_sec, &tmnow);

	यदि (time_is_today(&tm, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", tm.tm_hour, tm.tm_min);
		यदि (rc < 0 || (size_t) rc > bufsz)
			प्रत्यागम -1;
		rc = 1;

	} अन्यथा यदि (time_is_thisyear(&tm, &tmnow)) {
		यदि (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = strftime(buf, bufsz, "%b%d/%H:%M", &tm);
		अन्यथा
			rc = strftime(buf, bufsz, "%b%d", &tm);
	} अन्यथा
		rc = strftime(buf, bufsz, "%Y-%b%d", &tm);

	प्रत्यागम rc <= 0 ? -1 : 0;
}

पूर्णाङ्क strtimespec_relative(ध्रुव संरचना timespec *ts, वर्ण *buf, size_t bufsz)
{
	time_t secs = ts->tv_sec;
	size_t i, parts = 0;
	पूर्णाङ्क rc;

	यदि (bufsz)
		buf[0] = '\0';

	स्थिर ध्रुव संरचना {
		ध्रुव वर्ण * ध्रुव suffix;
		पूर्णाङ्क विस्तारः;
		int64_t secs;
	} सारणी[] = {
		{ "y", 4, NSEC_प्रत्येकम्_YEAR   / NSEC_प्रत्येकम्_SEC },
		{ "d", 3, NSEC_प्रत्येकम्_DAY    / NSEC_प्रत्येकम्_SEC },
		{ "h", 2, NSEC_प्रत्येकम्_HOUR   / NSEC_प्रत्येकम्_SEC },
		{ "m", 2, NSEC_प्रत्येकम्_MINUTE / NSEC_प्रत्येकम्_SEC },
		{ "s", 2, NSEC_प्रत्येकम्_SEC    / NSEC_प्रत्येकम्_SEC },
	};

	प्रति (i = 0; i < ARRAY_आकार(सारणी); i++) {
		यदि (secs >= सारणी[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? सारणी[i].विस्तारः : 0,
				      secs / सारणी[i].secs, सारणी[i].suffix,
				      secs % सारणी[i].secs ? " " : "");
			यदि (rc < 0 || (size_t) rc > bufsz)
				गच्छ err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= सारणी[i].secs;
		}
	}

	यदि (ts->tv_nsec) {
		यदि (ts->tv_nsec % NSEC_प्रत्येकम्_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			यदि (rc < 0 || (size_t) rc > bufsz)
				गच्छ err;
		} अन्यथा {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_प्रत्येकम्_MSEC);
			यदि (rc < 0 || (size_t) rc > bufsz)
				गच्छ err;
		}
	}

	प्रत्यागम 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	प्रत्यागम -1;
}

#यदि_अपरिभाषितम् HAVE_TIMEGM
time_t timegm(संरचना tm *tm)
{
	ध्रुव वर्ण *zone = getenv("TZ");
	time_t ret;

	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	यदि (zone)
		setenv("TZ", zone, 1);
	अन्यथा
		unsetenv("TZ");
	tzset();
	प्रत्यागम ret;
}
#यदि_अन्तः /* HAVE_TIMEGM */

#यदि_परिभाषितम् TEST_क्रमादेशः_TIMEUTILS

स्थिर पूर्णाङ्क सततम्_unittest_timestamp(शून्य)
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

	प्रति (size_t i = 0; i < ARRAY_आकार(testcases); i++) {
		संरचना testcase t = testcases[i];
		usec_t परिणाम;
		पूर्णाङ्क r = parse_timestamp_reference(reference, t.निवेशः, &परिणाम);
		यदि (r) {
			fprintf(stderr, "Could not parse '%s'\n", t.निवेशः);
			rc = EXIT_FAILURE;
		}

		यदि (परिणाम != t.expected) {
			fprintf(stderr, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.निवेशः, परिणाम, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	प्रत्यागम rc;
}

स्थिर पूर्णाङ्क सततम्_unittest_format(शून्य)
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

	प्रति (size_t i = 0; i < ARRAY_आकार(testcases); i++) {
		संरचना testcase t = testcases[i];
		पूर्णाङ्क r = strtimespec_iso(&ts, t.flags, buf, परिमाण(buf));
		यदि (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		यदि (वाक्यं_तुलय(buf, t.expected)) {
			fprintf(stderr, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	प्रत्यागम rc;
}

स्थिर पूर्णाङ्क सततम्_unittest_format_relative(शून्य)
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

	प्रति (size_t i = 0; i < ARRAY_आकार(testcases); i++) {
		संरचना testcase t = testcases[i];
		पूर्णाङ्क r = strtimespec_relative(&t.ts, buf, परिमाण(buf));
		यदि (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		यदि (वाक्यं_तुलय(buf, t.expected)) {
			fprintf(stderr, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	प्रत्यागम rc;
}

पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण *argv[])
{
	संरचना timespec ts = { 0 };
	वर्ण buf[ISO_BUFSIZ];
	पूर्णाङ्क r;

	यदि (argc < 2) {
		fprintf(stderr, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		निर्गच्छ(EXIT_FAILURE);
	}

	यदि (वाक्यं_तुलय(argv[1], "--unittest-timestamp") == 0)
		प्रत्यागम सततम्_unittest_timestamp();
	अन्यथा यदि (वाक्यं_तुलय(argv[1], "--unittest-format") == 0)
		प्रत्यागम सततम्_unittest_format();
	अन्यथा यदि (वाक्यं_तुलय(argv[1], "--unittest-format-relative") == 0)
		प्रत्यागम सततम्_unittest_format_relative();

	यदि (वाक्यं_तुलय(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		यदि (r)
			errx(EXIT_FAILURE, "Can not parse '%s': %s", argv[2], strerror(-r));
		ts.tv_sec = (time_t) (usec / USEC_प्रत्येकम्_SEC);
		ts.tv_nsec = (usec % USEC_प्रत्येकम्_SEC) * NSEC_प्रत्येकम्_USEC;
	} अन्यथा {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		यदि (argc == 3)
			ts.tv_nsec = strtos64_or_err(argv[2], "failed to parse <usec>")
				     * NSEC_प्रत्येकम्_USEC;
	}

	strtimespec_iso(&ts, ISO_DATE, buf, परिमाण(buf));
	प्रारूप_लिख("Date: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIME, buf, परिमाण(buf));
	प्रारूप_लिख("Time: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_DATE | ISO_TIME | ISO_COMMAUSEC | ISO_T,
		       buf, परिमाण(buf));
	प्रारूप_लिख("Full: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIMESTAMP_DOT, buf, परिमाण(buf));
	प्रारूप_लिख("Zone: '%s'\n", buf);

	strtimespec_relative(&ts, buf, परिमाण(buf));
	प्रारूप_लिख("Rel:  '%s'\n", buf);

	प्रत्यागम EXIT_SUCCESS;
}

#यदि_अन्तः /* TEST_PROGRAM_TIMEUTILS */
