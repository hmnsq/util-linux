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

#define streq(a,b) (म_भेद((a),(b)) == 0)

स्थिर पूर्णांक parse_sec(स्थिरांक अक्षर *t, usec_t *usec)
{
	स्थिर स्थिरांक संरचना {
		स्थिरांक अक्षर *suffix;
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

	स्थिरांक अक्षर *p;
	usec_t r = 0;
	पूर्णांक something = FALSE;

	प्रतिज्ञा(t);
	प्रतिज्ञा(usec);

	p = t;
	साठी (;;) {
		दीर्घ दीर्घ l, z = 0;
		अक्षर *e;
		अचिन्हित i, n = 0;

		p += म_अखोज(p, WHITESPACE);

		जर (*p == 0) {
			जर (!something)
				परत -EINVAL;

			थांबा;
		}

		त्रुटि_सं = 0;
		l = strtoll(p, &e, 10);

		जर (त्रुटि_सं > 0)
			परत -त्रुटि_सं;

		जर (l < 0)
			परत -दुस्फल;

		जर (*e == '.') {
			अक्षर *b = e + 1;

			त्रुटि_सं = 0;
			z = strtoll(b, &e, 10);
			जर (त्रुटि_सं > 0)
				परत -त्रुटि_सं;

			जर (z < 0)
				परत -दुस्फल;

			जर (e == b)
				परत -EINVAL;

			n = e - b;

		} अन्यथा जर (e == p)
			परत -EINVAL;

		e += म_अखोज(e, WHITESPACE);

		साठी (i = 0; i < ARRAY_SIZE(table); i++)
			जर (ul_startswith(e, table[i].suffix)) {
				usec_t k = (usec_t) z * table[i].usec;

				साठी (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *table[i].usec + k;
				p = e + म_माप(table[i].suffix);

				something = TRUE;
				थांबा;
			}

		जर (i >= ARRAY_SIZE(table))
			परत -EINVAL;

	}

	*usec = r;

	परत 0;
}

स्थिर पूर्णांक parse_subseconds(स्थिरांक अक्षर *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t factor = USEC_PER_SEC / 10;

	जर (*t != '.' && *t != ',')
		परत -1;

	पर्यंत (*(++t)) {
		जर (!है_अंक(*t) || factor < 1)
			परत -1;

		ret += ((usec_t) *t - '0') * factor;
		factor /= 10;
	}

	*usec = ret;
	परत 0;
}

स्थिर स्थिरांक अक्षर *parse_epoch_seconds(स्थिरांक अक्षर *t, संरचना पंचांग *पंचांग)
{
	int64_t s;
	समय_प्रकार st;
	पूर्णांक f, c;

	f = माला_पूछो(t, "%"SCNd64"%n", &s, &c);
	जर (f < 1)
		परत रिक्त;
	st = s;
	जर ((int64_t) st < s)
		परत रिक्त;
	जर (!localtime_r(&st, पंचांग))
		परत रिक्त;
	परत t + c;
}

स्थिर पूर्णांक parse_timestamp_reference(समय_प्रकार x, स्थिरांक अक्षर *t, usec_t *usec)
{
	स्थिर स्थिरांक संरचना {
		स्थिरांक अक्षर *name;
		स्थिरांक पूर्णांक nr;
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

	स्थिरांक अक्षर *k;
	संरचना पंचांग पंचांग, copy;
	usec_t plus = 0, minus = 0, ret = 0;
	पूर्णांक r, weekday = -1;
	अचिन्हित i;

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

	localtime_r(&x, &पंचांग);
	पंचांग.tm_isdst = -1;

	जर (streq(t, "now"))
		जा finish;

	अन्यथा जर (streq(t, "today")) {
		पंचांग.tm_sec = पंचांग.tm_min = पंचांग.tm_hour = 0;
		जा finish;

	} अन्यथा जर (streq(t, "yesterday")) {
		पंचांग.tm_mday--;
		पंचांग.tm_sec = पंचांग.tm_min = पंचांग.tm_hour = 0;
		जा finish;

	} अन्यथा जर (streq(t, "tomorrow")) {
		पंचांग.tm_mday++;
		पंचांग.tm_sec = पंचांग.tm_min = पंचांग.tm_hour = 0;
		जा finish;

	} अन्यथा जर (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		जर (r < 0)
			परत r;

		जा finish;
	} अन्यथा जर (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		जर (r < 0)
			परत r;

		जा finish;
	} अन्यथा जर (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &पंचांग);
		जर (k && *k == 0)
			जा finish;
		अन्यथा जर (k && parse_subseconds(k, &ret) == 0)
			जा finish;

		परत -EINVAL;
	} अन्यथा जर (ul_endswith(t, " ago")) {
		अक्षर *z;

		z = strndup(t, म_माप(t) - 4);
		जर (!z)
			परत -ENOMEM;

		r = parse_sec(z, &minus);
		मुक्त(z);
		जर (r < 0)
			परत r;

		जा finish;
	}

	साठी (i = 0; i < ARRAY_SIZE(day_nr); i++) {
		माप_प्रकार skip;

		जर (!startswith_no_case(t, day_nr[i].name))
			सुरूठेवा;

		skip = म_माप(day_nr[i].name);
		जर (t[skip] != ' ')
			सुरूठेवा;

		weekday = day_nr[i].nr;
		t += skip + 1;
		थांबा;
	}

	copy = पंचांग;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &पंचांग);
	जर (k && *k == 0)
		जा finish;
	अन्यथा जर (k && parse_subseconds(k, &ret) == 0)
		जा finish;

	पंचांग = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &पंचांग);
	जर (k && *k == 0)
		जा finish;
	अन्यथा जर (k && parse_subseconds(k, &ret) == 0)
		जा finish;

	पंचांग = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &पंचांग);
	जर (k && *k == 0)
		जा finish;
	अन्यथा जर (k && parse_subseconds(k, &ret) == 0)
		जा finish;

	पंचांग = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &पंचांग);
	जर (k && *k == 0) {
		पंचांग.tm_sec = 0;
		जा finish;
	}

	पंचांग = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &पंचांग);
	जर (k && *k == 0) {
		पंचांग.tm_sec = 0;
		जा finish;
	}

	पंचांग = copy;
	k = strptime(t, "%y-%m-%d", &पंचांग);
	जर (k && *k == 0) {
		पंचांग.tm_sec = पंचांग.tm_min = पंचांग.tm_hour = 0;
		जा finish;
	}

	पंचांग = copy;
	k = strptime(t, "%Y-%m-%d", &पंचांग);
	जर (k && *k == 0) {
		पंचांग.tm_sec = पंचांग.tm_min = पंचांग.tm_hour = 0;
		जा finish;
	}

	पंचांग = copy;
	k = strptime(t, "%H:%M:%S", &पंचांग);
	जर (k && *k == 0)
		जा finish;
	अन्यथा जर (k && parse_subseconds(k, &ret) == 0)
		जा finish;

	पंचांग = copy;
	k = strptime(t, "%H:%M", &पंचांग);
	जर (k && *k == 0) {
		पंचांग.tm_sec = 0;
		जा finish;
	}

	पंचांग = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &पंचांग);
	जर (k && *k == 0)
		जा finish;
	अन्यथा जर (k && parse_subseconds(k, &ret) == 0)
		जा finish;

	परत -EINVAL;

 finish:
	x = स_गढ़ो(&पंचांग);
	जर (x == (समय_प्रकार)-1)
		परत -EINVAL;

	जर (weekday >= 0 && पंचांग.tm_wday != weekday)
		परत -EINVAL;

	ret += (usec_t) x * USEC_PER_SEC;

	जर (minus > ret)
		परत -दुस्फल;
	जर ((ret + plus) < ret)
		परत -दुस्फल;

	ret += plus;
	ret -= minus;

	*usec = ret;

	परत 0;
}

पूर्णांक ul_parse_timestamp(स्थिरांक अक्षर *t, usec_t *usec)
{
	परत parse_timestamp_reference(समय(रिक्त), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
पूर्णांक get_gmtoff(स्थिरांक संरचना पंचांग *tp)
{
	जर (tp->tm_isdst < 0)
		परत 0;

#जर HAVE_TM_GMTOFF
	परत tp->tm_gmtoff;
#अन्यथा
	संरचना पंचांग पंचांग;
	संरचना पंचांग gtm;
	संरचना पंचांग ltm = *tp;
	समय_प्रकार lt;

	tzset();
	lt = स_गढ़ो(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	जर (lt == (समय_प्रकार) -1) {
		जर (! localtime_r(&lt, &पंचांग)
			|| ((ltm.tm_sec ^ पंचांग.tm_sec)
			    | (ltm.tm_min ^ पंचांग.tm_min)
			    | (ltm.tm_hour ^ पंचांग.tm_hour)
			    | (ltm.tm_mday ^ पंचांग.tm_mday)
			    | (ltm.tm_mon ^ पंचांग.tm_mon)
			    | (ltm.tm_year ^ पंचांग.tm_year)))
			परत 0;
	}

	जर (! gmtime_r(&lt, &gtm))
		परत 0;

	/* Calculate the GMT offset, that is, the difference between the
	 * TP argument (ltm) and GMT (gtm).
	 *
	 * Compute intervening leap days correctly even if year is negative.
	 * Take care to avoid int overflow in leap day calculations, but it's OK
	 * to assume that A and B are close to each other.
	 */
	पूर्णांक a4 = (ltm.tm_year >> 2) + (1900 >> 2) - ! (ltm.tm_year & 3);
	पूर्णांक b4 = (gtm.tm_year >> 2) + (1900 >> 2) - ! (gtm.tm_year & 3);
	पूर्णांक a100 = a4 / 25 - (a4 % 25 < 0);
	पूर्णांक b100 = b4 / 25 - (b4 % 25 < 0);
	पूर्णांक a400 = a100 >> 2;
	पूर्णांक b400 = b100 >> 2;
	पूर्णांक intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

	पूर्णांक years = ltm.tm_year - gtm.tm_year;
	पूर्णांक days = (365 * years + intervening_leap_days
		    + (ltm.tm_yday - gtm.tm_yday));

	परत (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#endif
}

स्थिर पूर्णांक format_iso_time(स्थिरांक संरचना पंचांग *पंचांग, uint32_t nsec, पूर्णांक flags, अक्षर *buf, माप_प्रकार bufsz)
{
	uint32_t usec = nsec / NSEC_PER_USEC;
	अक्षर *p = buf;
	पूर्णांक len;

	जर (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       पंचांग->tm_year + (दीर्घ) 1900,
			       पंचांग->tm_mon + 1, पंचांग->tm_mday);
		जर (len < 0 || (माप_प्रकार) len > bufsz)
			जा err;
		bufsz -= len;
		p += len;
	}

	जर ((flags & ISO_DATE) && (flags & ISO_TIME)) {
		जर (bufsz < 1)
			जा err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	जर (flags & ISO_TIME) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", पंचांग->tm_hour,
			       पंचांग->tm_min, पंचांग->tm_sec);
		जर (len < 0 || (माप_प्रकार) len > bufsz)
			जा err;
		bufsz -= len;
		p += len;
	}

	जर (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		जर (len < 0 || (माप_प्रकार) len > bufsz)
			जा err;
		bufsz -= len;
		p += len;

	} अन्यथा जर (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		जर (len < 0 || (माप_प्रकार) len > bufsz)
			जा err;
		bufsz -= len;
		p += len;
	} अन्यथा जर (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		जर (len < 0 || (माप_प्रकार) len > bufsz)
			जा err;
		bufsz -= len;
		p += len;

	} अन्यथा जर (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		जर (len < 0 || (माप_प्रकार) len > bufsz)
			जा err;
		bufsz -= len;
		p += len;
	}

	जर (flags & ISO_TIMEZONE) {
		पूर्णांक tmin  = get_gmtoff(पंचांग) / 60;
		पूर्णांक zhour = tmin / 60;
		पूर्णांक zmin  = असल(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		जर (len < 0 || (माप_प्रकार) len > bufsz)
			जा err;
	}
	परत 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	परत -1;
}

/* timespec to ISO 8601 */
पूर्णांक strtimespec_iso(स्थिरांक संरचना timespec *ts, पूर्णांक flags, अक्षर *buf, माप_प्रकार bufsz)
{
	संरचना पंचांग पंचांग;
	संरचना पंचांग *rc;

	जर (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &पंचांग);
	अन्यथा
		rc = localtime_r(&ts->tv_sec, &पंचांग);

	जर (rc)
		परत format_iso_time(&पंचांग, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	परत -1;
}

/* timeval to ISO 8601 */
पूर्णांक strtimeval_iso(स्थिरांक संरचना timeval *tv, पूर्णांक flags, अक्षर *buf, माप_प्रकार bufsz)
{
	संरचना timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_PER_USEC,
	};

	परत strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
पूर्णांक strtm_iso(स्थिरांक संरचना पंचांग *पंचांग, पूर्णांक flags, अक्षर *buf, माप_प्रकार bufsz)
{
	परत format_iso_time(पंचांग, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
पूर्णांक strtime_iso(स्थिरांक समय_प्रकार *t, पूर्णांक flags, अक्षर *buf, माप_प्रकार bufsz)
{
	संरचना पंचांग पंचांग;
	संरचना पंचांग *rc;

	जर (flags & ISO_GMTIME)
		rc = gmtime_r(t, &पंचांग);
	अन्यथा
		rc = localtime_r(t, &पंचांग);

	जर (rc)
		परत format_iso_time(&पंचांग, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	परत -1;
}

/* relative time functions */
स्थिर ओळीत पूर्णांक time_is_thisyear(संरचना पंचांग स्थिरांक *स्थिरांक पंचांग,
				   संरचना पंचांग स्थिरांक *स्थिरांक tmnow)
{
	परत पंचांग->tm_year == tmnow->tm_year;
}

स्थिर ओळीत पूर्णांक time_is_today(संरचना पंचांग स्थिरांक *स्थिरांक पंचांग,
				संरचना पंचांग स्थिरांक *स्थिरांक tmnow)
{
	परत (पंचांग->tm_yday == tmnow->tm_yday &&
		time_is_thisyear(पंचांग, tmnow));
}

पूर्णांक strtime_short(स्थिरांक समय_प्रकार *t, संरचना timeval *now, पूर्णांक flags, अक्षर *buf, माप_प्रकार bufsz)
{
	संरचना पंचांग पंचांग, tmnow;
	पूर्णांक rc = 0;

	जर (now->tv_sec == 0)
		gettimeofday(now, रिक्त);

	localtime_r(t, &पंचांग);
	localtime_r(&now->tv_sec, &tmnow);

	जर (time_is_today(&पंचांग, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", पंचांग.tm_hour, पंचांग.tm_min);
		जर (rc < 0 || (माप_प्रकार) rc > bufsz)
			परत -1;
		rc = 1;

	} अन्यथा जर (time_is_thisyear(&पंचांग, &tmnow)) {
		जर (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = स_माला(buf, bufsz, "%b%d/%H:%M", &पंचांग);
		अन्यथा
			rc = स_माला(buf, bufsz, "%b%d", &पंचांग);
	} अन्यथा
		rc = स_माला(buf, bufsz, "%Y-%b%d", &पंचांग);

	परत rc <= 0 ? -1 : 0;
}

पूर्णांक strtimespec_relative(स्थिरांक संरचना timespec *ts, अक्षर *buf, माप_प्रकार bufsz)
{
	समय_प्रकार secs = ts->tv_sec;
	माप_प्रकार i, parts = 0;
	पूर्णांक rc;

	जर (bufsz)
		buf[0] = '\0';

	स्थिर स्थिरांक संरचना {
		स्थिरांक अक्षर * स्थिरांक suffix;
		पूर्णांक width;
		int64_t secs;
	} table[] = {
		{ "y", 4, NSEC_PER_YEAR   / NSEC_PER_SEC },
		{ "d", 3, NSEC_PER_DAY    / NSEC_PER_SEC },
		{ "h", 2, NSEC_PER_HOUR   / NSEC_PER_SEC },
		{ "m", 2, NSEC_PER_MINUTE / NSEC_PER_SEC },
		{ "s", 2, NSEC_PER_SEC    / NSEC_PER_SEC },
	};

	साठी (i = 0; i < ARRAY_SIZE(table); i++) {
		जर (secs >= table[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? table[i].width : 0,
				      secs / table[i].secs, table[i].suffix,
				      secs % table[i].secs ? " " : "");
			जर (rc < 0 || (माप_प्रकार) rc > bufsz)
				जा err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= table[i].secs;
		}
	}

	जर (ts->tv_nsec) {
		जर (ts->tv_nsec % NSEC_PER_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			जर (rc < 0 || (माप_प्रकार) rc > bufsz)
				जा err;
		} अन्यथा {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_PER_MSEC);
			जर (rc < 0 || (माप_प्रकार) rc > bufsz)
				जा err;
		}
	}

	परत 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	परत -1;
}

#ifndef HAVE_TIMEGM
समय_प्रकार timegm(संरचना पंचांग *पंचांग)
{
	स्थिरांक अक्षर *zone = दो_पर्या("TZ");
	समय_प्रकार ret;

	setenv("TZ", "", 1);
	tzset();
	ret = स_गढ़ो(पंचांग);
	जर (zone)
		setenv("TZ", zone, 1);
	अन्यथा
		unsetenv("TZ");
	tzset();
	परत ret;
}
#endif /* HAVE_TIMEGM */

#ifdef TEST_PROGRAM_TIMEUTILS

स्थिर पूर्णांक run_unittest_timestamp(शून्य)
{
	पूर्णांक rc = निकास_सफल;
	समय_प्रकार reference = 1674180427;
	स्थिर स्थिरांक संरचना testcase {
		स्थिरांक अक्षर * स्थिरांक आदान;
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

	साठी (माप_प्रकार i = 0; i < ARRAY_SIZE(testcases); i++) {
		संरचना testcase t = testcases[i];
		usec_t result;
		पूर्णांक r = parse_timestamp_reference(reference, t.आदान, &result);
		जर (r) {
			ख_लिखो(मानक_त्रुटि, "Could not parse '%s'\n", t.आदान);
			rc = निकास_त्रुटि;
		}

		जर (result != t.expected) {
			ख_लिखो(मानक_त्रुटि, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.आदान, result, t.expected);
			rc = निकास_त्रुटि;
		}
	}

	परत rc;
}

स्थिर पूर्णांक run_unittest_format(शून्य)
{
	पूर्णांक rc = निकास_सफल;
	स्थिरांक संरचना timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	अक्षर buf[FORMAT_TIMESTAMP_MAX];
	स्थिर स्थिरांक संरचना testcase {
		पूर्णांक flags;
		स्थिरांक अक्षर * स्थिरांक expected;
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

	साठी (माप_प्रकार i = 0; i < ARRAY_SIZE(testcases); i++) {
		संरचना testcase t = testcases[i];
		पूर्णांक r = strtimespec_iso(&ts, t.flags, buf, आकार(buf));
		जर (r) {
			ख_लिखो(मानक_त्रुटि, "Could not format '%s'\n", t.expected);
			rc = निकास_त्रुटि;
		}

		जर (म_भेद(buf, t.expected)) {
			ख_लिखो(मानक_त्रुटि, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = निकास_त्रुटि;
		}
	}

	परत rc;
}

स्थिर पूर्णांक run_unittest_format_relative(शून्य)
{
	पूर्णांक rc = निकास_सफल;
	अक्षर buf[FORMAT_TIMESTAMP_MAX];
	स्थिर स्थिरांक संरचना testcase {
		संरचना timespec ts;
		स्थिरांक अक्षर * स्थिरांक expected;
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

	साठी (माप_प्रकार i = 0; i < ARRAY_SIZE(testcases); i++) {
		संरचना testcase t = testcases[i];
		पूर्णांक r = strtimespec_relative(&t.ts, buf, आकार(buf));
		जर (r) {
			ख_लिखो(मानक_त्रुटि, "Could not format '%s'\n", t.expected);
			rc = निकास_त्रुटि;
		}

		जर (म_भेद(buf, t.expected)) {
			ख_लिखो(मानक_त्रुटि, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = निकास_त्रुटि;
		}
	}

	परत rc;
}

पूर्णांक main(पूर्णांक argc, अक्षर *argv[])
{
	संरचना timespec ts = { 0 };
	अक्षर buf[ISO_BUFSIZ];
	पूर्णांक r;

	जर (argc < 2) {
		ख_लिखो(मानक_त्रुटि, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		निकास(निकास_त्रुटि);
	}

	जर (म_भेद(argv[1], "--unittest-timestamp") == 0)
		परत run_unittest_timestamp();
	अन्यथा जर (म_भेद(argv[1], "--unittest-format") == 0)
		परत run_unittest_format();
	अन्यथा जर (म_भेद(argv[1], "--unittest-format-relative") == 0)
		परत run_unittest_format_relative();

	जर (म_भेद(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		जर (r)
			errx(निकास_त्रुटि, "Can not parse '%s': %s", argv[2], म_त्रुटि(-r));
		ts.tv_sec = (समय_प्रकार) (usec / USEC_PER_SEC);
		ts.tv_nsec = (usec % USEC_PER_SEC) * NSEC_PER_USEC;
	} अन्यथा {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		जर (argc == 3)
			ts.tv_nsec = strtos64_or_err(argv[2], "failed to parse <usec>")
				     * NSEC_PER_USEC;
	}

	strtimespec_iso(&ts, ISO_DATE, buf, आकार(buf));
	म_लिखो("Date: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIME, buf, आकार(buf));
	म_लिखो("Time: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_DATE | ISO_TIME | ISO_COMMAUSEC | ISO_T,
		       buf, आकार(buf));
	म_लिखो("Full: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIMESTAMP_DOT, buf, आकार(buf));
	म_लिखो("Zone: '%s'\n", buf);

	strtimespec_relative(&ts, buf, आकार(buf));
	म_लिखो("Rel:  '%s'\n", buf);

	परत निकास_सफल;
}

#endif /* TEST_PROGRAM_TIMEUTILS */
