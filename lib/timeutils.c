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
#समावेश <assert.h>
#समावेश <ctype.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <time.h>
#समावेश <sys/time.h>
#समावेश <inttypes.h>

#समावेश "c.h"
#समावेश "nls.h"
#समावेश "strutils.h"
#समावेश "timeutils.h"

#घोषणा WHITESPACE " \t\n\r"

#घोषणा streq(a,b) (म_भेद((a),(b)) == 0)

अटल पूर्णांक parse_sec(स्थिर अक्षर *t, usec_t *usec)
{
	अटल स्थिर काष्ठा {
		स्थिर अक्षर *suffix;
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

	स्थिर अक्षर *p;
	usec_t r = 0;
	पूर्णांक something = FALSE;

	निश्चित(t);
	निश्चित(usec);

	p = t;
	क्रम (;;) {
		दीर्घ दीर्घ l, z = 0;
		अक्षर *e;
		अचिन्हित i, n = 0;

		p += म_अखोज(p, WHITESPACE);

		अगर (*p == 0) {
			अगर (!something)
				वापस -EINVAL;

			अवरोध;
		}

		त्रुटि_सं = 0;
		l = strtoll(p, &e, 10);

		अगर (त्रुटि_सं > 0)
			वापस -त्रुटि_सं;

		अगर (l < 0)
			वापस -दुस्फल;

		अगर (*e == '.') {
			अक्षर *b = e + 1;

			त्रुटि_सं = 0;
			z = strtoll(b, &e, 10);
			अगर (त्रुटि_सं > 0)
				वापस -त्रुटि_सं;

			अगर (z < 0)
				वापस -दुस्फल;

			अगर (e == b)
				वापस -EINVAL;

			n = e - b;

		} अन्यथा अगर (e == p)
			वापस -EINVAL;

		e += म_अखोज(e, WHITESPACE);

		क्रम (i = 0; i < ARRAY_आकार(table); i++)
			अगर (ul_startswith(e, table[i].suffix)) {
				usec_t k = (usec_t) z * table[i].usec;

				क्रम (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *table[i].usec + k;
				p = e + म_माप(table[i].suffix);

				something = TRUE;
				अवरोध;
			}

		अगर (i >= ARRAY_आकार(table))
			वापस -EINVAL;

	}

	*usec = r;

	वापस 0;
}

अटल पूर्णांक parse_subseconds(स्थिर अक्षर *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t factor = USEC_PER_SEC / 10;

	अगर (*t != '.' && *t != ',')
		वापस -1;

	जबतक (*(++t)) {
		अगर (!है_अंक(*t) || factor < 1)
			वापस -1;

		ret += ((usec_t) *t - '0') * factor;
		factor /= 10;
	}

	*usec = ret;
	वापस 0;
}

अटल स्थिर अक्षर *parse_epoch_seconds(स्थिर अक्षर *t, काष्ठा पंचांग *पंचांग)
{
	int64_t s;
	समय_प्रकार st;
	पूर्णांक f, c;

	f = माला_पूछो(t, "%"SCNd64"%n", &s, &c);
	अगर (f < 1)
		वापस शून्य;
	st = s;
	अगर ((int64_t) st < s)
		वापस शून्य;
	अगर (!localtime_r(&st, पंचांग))
		वापस शून्य;
	वापस t + c;
}

अटल पूर्णांक parse_timestamp_reference(समय_प्रकार x, स्थिर अक्षर *t, usec_t *usec)
{
	अटल स्थिर काष्ठा {
		स्थिर अक्षर *name;
		स्थिर पूर्णांक nr;
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

	स्थिर अक्षर *k;
	काष्ठा पंचांग पंचांग, copy;
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

	निश्चित(t);
	निश्चित(usec);

	localtime_r(&x, &पंचांग);
	पंचांग.tm_isdst = -1;

	अगर (streq(t, "now"))
		जाओ finish;

	अन्यथा अगर (streq(t, "today")) {
		पंचांग.tm_sec = पंचांग.tm_min = पंचांग.tm_hour = 0;
		जाओ finish;

	} अन्यथा अगर (streq(t, "yesterday")) {
		पंचांग.tm_mday--;
		पंचांग.tm_sec = पंचांग.tm_min = पंचांग.tm_hour = 0;
		जाओ finish;

	} अन्यथा अगर (streq(t, "tomorrow")) {
		पंचांग.tm_mday++;
		पंचांग.tm_sec = पंचांग.tm_min = पंचांग.tm_hour = 0;
		जाओ finish;

	} अन्यथा अगर (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		अगर (r < 0)
			वापस r;

		जाओ finish;
	} अन्यथा अगर (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		अगर (r < 0)
			वापस r;

		जाओ finish;
	} अन्यथा अगर (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &पंचांग);
		अगर (k && *k == 0)
			जाओ finish;
		अन्यथा अगर (k && parse_subseconds(k, &ret) == 0)
			जाओ finish;

		वापस -EINVAL;
	} अन्यथा अगर (ul_endswith(t, " ago")) {
		अक्षर *z;

		z = strndup(t, म_माप(t) - 4);
		अगर (!z)
			वापस -ENOMEM;

		r = parse_sec(z, &minus);
		मुक्त(z);
		अगर (r < 0)
			वापस r;

		जाओ finish;
	}

	क्रम (i = 0; i < ARRAY_आकार(day_nr); i++) {
		माप_प्रकार skip;

		अगर (!startswith_no_case(t, day_nr[i].name))
			जारी;

		skip = म_माप(day_nr[i].name);
		अगर (t[skip] != ' ')
			जारी;

		weekday = day_nr[i].nr;
		t += skip + 1;
		अवरोध;
	}

	copy = पंचांग;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &पंचांग);
	अगर (k && *k == 0)
		जाओ finish;
	अन्यथा अगर (k && parse_subseconds(k, &ret) == 0)
		जाओ finish;

	पंचांग = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &पंचांग);
	अगर (k && *k == 0)
		जाओ finish;
	अन्यथा अगर (k && parse_subseconds(k, &ret) == 0)
		जाओ finish;

	पंचांग = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &पंचांग);
	अगर (k && *k == 0)
		जाओ finish;
	अन्यथा अगर (k && parse_subseconds(k, &ret) == 0)
		जाओ finish;

	पंचांग = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &पंचांग);
	अगर (k && *k == 0) {
		पंचांग.tm_sec = 0;
		जाओ finish;
	}

	पंचांग = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &पंचांग);
	अगर (k && *k == 0) {
		पंचांग.tm_sec = 0;
		जाओ finish;
	}

	पंचांग = copy;
	k = strptime(t, "%y-%m-%d", &पंचांग);
	अगर (k && *k == 0) {
		पंचांग.tm_sec = पंचांग.tm_min = पंचांग.tm_hour = 0;
		जाओ finish;
	}

	पंचांग = copy;
	k = strptime(t, "%Y-%m-%d", &पंचांग);
	अगर (k && *k == 0) {
		पंचांग.tm_sec = पंचांग.tm_min = पंचांग.tm_hour = 0;
		जाओ finish;
	}

	पंचांग = copy;
	k = strptime(t, "%H:%M:%S", &पंचांग);
	अगर (k && *k == 0)
		जाओ finish;
	अन्यथा अगर (k && parse_subseconds(k, &ret) == 0)
		जाओ finish;

	पंचांग = copy;
	k = strptime(t, "%H:%M", &पंचांग);
	अगर (k && *k == 0) {
		पंचांग.tm_sec = 0;
		जाओ finish;
	}

	पंचांग = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &पंचांग);
	अगर (k && *k == 0)
		जाओ finish;
	अन्यथा अगर (k && parse_subseconds(k, &ret) == 0)
		जाओ finish;

	वापस -EINVAL;

 finish:
	x = स_गढ़ो(&पंचांग);
	अगर (x == (समय_प्रकार)-1)
		वापस -EINVAL;

	अगर (weekday >= 0 && पंचांग.tm_wday != weekday)
		वापस -EINVAL;

	ret += (usec_t) x * USEC_PER_SEC;

	अगर (minus > ret)
		वापस -दुस्फल;
	अगर ((ret + plus) < ret)
		वापस -दुस्फल;

	ret += plus;
	ret -= minus;

	*usec = ret;

	वापस 0;
}

पूर्णांक ul_parse_timestamp(स्थिर अक्षर *t, usec_t *usec)
{
	वापस parse_timestamp_reference(समय(शून्य), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
पूर्णांक get_gmtoff(स्थिर काष्ठा पंचांग *tp)
{
	अगर (tp->tm_isdst < 0)
		वापस 0;

#अगर HAVE_TM_GMTOFF
	वापस tp->tm_gmtoff;
#अन्यथा
	काष्ठा पंचांग पंचांग;
	काष्ठा पंचांग gtm;
	काष्ठा पंचांग ltm = *tp;
	समय_प्रकार lt;

	tzset();
	lt = स_गढ़ो(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	अगर (lt == (समय_प्रकार) -1) {
		अगर (! localtime_r(&lt, &पंचांग)
			|| ((ltm.tm_sec ^ पंचांग.tm_sec)
			    | (ltm.tm_min ^ पंचांग.tm_min)
			    | (ltm.tm_hour ^ पंचांग.tm_hour)
			    | (ltm.tm_mday ^ पंचांग.tm_mday)
			    | (ltm.tm_mon ^ पंचांग.tm_mon)
			    | (ltm.tm_year ^ पंचांग.tm_year)))
			वापस 0;
	}

	अगर (! gmtime_r(&lt, &gtm))
		वापस 0;

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

	वापस (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#पूर्ण_अगर
}

अटल पूर्णांक format_iso_time(स्थिर काष्ठा पंचांग *पंचांग, uint32_t nsec, पूर्णांक flags, अक्षर *buf, माप_प्रकार bufsz)
{
	uint32_t usec = nsec / NSEC_PER_USEC;
	अक्षर *p = buf;
	पूर्णांक len;

	अगर (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       पंचांग->tm_year + (दीर्घ) 1900,
			       पंचांग->tm_mon + 1, पंचांग->tm_mday);
		अगर (len < 0 || (माप_प्रकार) len > bufsz)
			जाओ err;
		bufsz -= len;
		p += len;
	}

	अगर ((flags & ISO_DATE) && (flags & ISO_TIME)) {
		अगर (bufsz < 1)
			जाओ err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	अगर (flags & ISO_TIME) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", पंचांग->tm_hour,
			       पंचांग->tm_min, पंचांग->tm_sec);
		अगर (len < 0 || (माप_प्रकार) len > bufsz)
			जाओ err;
		bufsz -= len;
		p += len;
	}

	अगर (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		अगर (len < 0 || (माप_प्रकार) len > bufsz)
			जाओ err;
		bufsz -= len;
		p += len;

	} अन्यथा अगर (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		अगर (len < 0 || (माप_प्रकार) len > bufsz)
			जाओ err;
		bufsz -= len;
		p += len;
	} अन्यथा अगर (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		अगर (len < 0 || (माप_प्रकार) len > bufsz)
			जाओ err;
		bufsz -= len;
		p += len;

	} अन्यथा अगर (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		अगर (len < 0 || (माप_प्रकार) len > bufsz)
			जाओ err;
		bufsz -= len;
		p += len;
	}

	अगर (flags & ISO_TIMEZONE) {
		पूर्णांक tmin  = get_gmtoff(पंचांग) / 60;
		पूर्णांक zhour = tmin / 60;
		पूर्णांक zmin  = असल(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		अगर (len < 0 || (माप_प्रकार) len > bufsz)
			जाओ err;
	}
	वापस 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	वापस -1;
}

/* timespec to ISO 8601 */
पूर्णांक strtimespec_iso(स्थिर काष्ठा timespec *ts, पूर्णांक flags, अक्षर *buf, माप_प्रकार bufsz)
{
	काष्ठा पंचांग पंचांग;
	काष्ठा पंचांग *rc;

	अगर (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &पंचांग);
	अन्यथा
		rc = localtime_r(&ts->tv_sec, &पंचांग);

	अगर (rc)
		वापस format_iso_time(&पंचांग, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	वापस -1;
}

/* timeval to ISO 8601 */
पूर्णांक strtimeval_iso(स्थिर काष्ठा timeval *tv, पूर्णांक flags, अक्षर *buf, माप_प्रकार bufsz)
{
	काष्ठा timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_PER_USEC,
	};

	वापस strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
पूर्णांक strtm_iso(स्थिर काष्ठा पंचांग *पंचांग, पूर्णांक flags, अक्षर *buf, माप_प्रकार bufsz)
{
	वापस format_iso_time(पंचांग, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
पूर्णांक strtime_iso(स्थिर समय_प्रकार *t, पूर्णांक flags, अक्षर *buf, माप_प्रकार bufsz)
{
	काष्ठा पंचांग पंचांग;
	काष्ठा पंचांग *rc;

	अगर (flags & ISO_GMTIME)
		rc = gmtime_r(t, &पंचांग);
	अन्यथा
		rc = localtime_r(t, &पंचांग);

	अगर (rc)
		वापस format_iso_time(&पंचांग, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	वापस -1;
}

/* relative time functions */
अटल अंतरभूत पूर्णांक time_is_thisyear(काष्ठा पंचांग स्थिर *स्थिर पंचांग,
				   काष्ठा पंचांग स्थिर *स्थिर tmnow)
{
	वापस पंचांग->tm_year == tmnow->tm_year;
}

अटल अंतरभूत पूर्णांक time_is_today(काष्ठा पंचांग स्थिर *स्थिर पंचांग,
				काष्ठा पंचांग स्थिर *स्थिर tmnow)
{
	वापस (पंचांग->tm_yday == tmnow->tm_yday &&
		time_is_thisyear(पंचांग, tmnow));
}

पूर्णांक strtime_short(स्थिर समय_प्रकार *t, काष्ठा timeval *now, पूर्णांक flags, अक्षर *buf, माप_प्रकार bufsz)
{
	काष्ठा पंचांग पंचांग, tmnow;
	पूर्णांक rc = 0;

	अगर (now->tv_sec == 0)
		gettimeofday(now, शून्य);

	localtime_r(t, &पंचांग);
	localtime_r(&now->tv_sec, &tmnow);

	अगर (time_is_today(&पंचांग, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", पंचांग.tm_hour, पंचांग.tm_min);
		अगर (rc < 0 || (माप_प्रकार) rc > bufsz)
			वापस -1;
		rc = 1;

	} अन्यथा अगर (time_is_thisyear(&पंचांग, &tmnow)) {
		अगर (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = स_माला(buf, bufsz, "%b%d/%H:%M", &पंचांग);
		अन्यथा
			rc = स_माला(buf, bufsz, "%b%d", &पंचांग);
	} अन्यथा
		rc = स_माला(buf, bufsz, "%Y-%b%d", &पंचांग);

	वापस rc <= 0 ? -1 : 0;
}

पूर्णांक strtimespec_relative(स्थिर काष्ठा timespec *ts, अक्षर *buf, माप_प्रकार bufsz)
{
	समय_प्रकार secs = ts->tv_sec;
	माप_प्रकार i, parts = 0;
	पूर्णांक rc;

	अगर (bufsz)
		buf[0] = '\0';

	अटल स्थिर काष्ठा {
		स्थिर अक्षर * स्थिर suffix;
		पूर्णांक width;
		int64_t secs;
	} table[] = {
		{ "y", 4, NSEC_PER_YEAR   / NSEC_PER_SEC },
		{ "d", 3, NSEC_PER_DAY    / NSEC_PER_SEC },
		{ "h", 2, NSEC_PER_HOUR   / NSEC_PER_SEC },
		{ "m", 2, NSEC_PER_MINUTE / NSEC_PER_SEC },
		{ "s", 2, NSEC_PER_SEC    / NSEC_PER_SEC },
	};

	क्रम (i = 0; i < ARRAY_आकार(table); i++) {
		अगर (secs >= table[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? table[i].width : 0,
				      secs / table[i].secs, table[i].suffix,
				      secs % table[i].secs ? " " : "");
			अगर (rc < 0 || (माप_प्रकार) rc > bufsz)
				जाओ err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= table[i].secs;
		}
	}

	अगर (ts->tv_nsec) {
		अगर (ts->tv_nsec % NSEC_PER_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			अगर (rc < 0 || (माप_प्रकार) rc > bufsz)
				जाओ err;
		} अन्यथा {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_PER_MSEC);
			अगर (rc < 0 || (माप_प्रकार) rc > bufsz)
				जाओ err;
		}
	}

	वापस 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	वापस -1;
}

#अगर_अघोषित HAVE_TIMEGM
समय_प्रकार timegm(काष्ठा पंचांग *पंचांग)
{
	स्थिर अक्षर *zone = दो_पर्या("TZ");
	समय_प्रकार ret;

	setenv("TZ", "", 1);
	tzset();
	ret = स_गढ़ो(पंचांग);
	अगर (zone)
		setenv("TZ", zone, 1);
	अन्यथा
		unsetenv("TZ");
	tzset();
	वापस ret;
}
#पूर्ण_अगर /* HAVE_TIMEGM */

#अगर_घोषित TEST_PROGRAM_TIMEUTILS

अटल पूर्णांक run_unittest_timestamp(व्योम)
{
	पूर्णांक rc = निकास_सफल;
	समय_प्रकार reference = 1674180427;
	अटल स्थिर काष्ठा testcase {
		स्थिर अक्षर * स्थिर निवेश;
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

	क्रम (माप_प्रकार i = 0; i < ARRAY_आकार(testcases); i++) {
		काष्ठा testcase t = testcases[i];
		usec_t परिणाम;
		पूर्णांक r = parse_timestamp_reference(reference, t.निवेश, &परिणाम);
		अगर (r) {
			ख_लिखो(मानक_त्रुटि, "Could not parse '%s'\n", t.निवेश);
			rc = निकास_त्रुटि;
		}

		अगर (परिणाम != t.expected) {
			ख_लिखो(मानक_त्रुटि, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.निवेश, परिणाम, t.expected);
			rc = निकास_त्रुटि;
		}
	}

	वापस rc;
}

अटल पूर्णांक run_unittest_format(व्योम)
{
	पूर्णांक rc = निकास_सफल;
	स्थिर काष्ठा timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	अक्षर buf[FORMAT_TIMESTAMP_MAX];
	अटल स्थिर काष्ठा testcase {
		पूर्णांक flags;
		स्थिर अक्षर * स्थिर expected;
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

	क्रम (माप_प्रकार i = 0; i < ARRAY_आकार(testcases); i++) {
		काष्ठा testcase t = testcases[i];
		पूर्णांक r = strtimespec_iso(&ts, t.flags, buf, माप(buf));
		अगर (r) {
			ख_लिखो(मानक_त्रुटि, "Could not format '%s'\n", t.expected);
			rc = निकास_त्रुटि;
		}

		अगर (म_भेद(buf, t.expected)) {
			ख_लिखो(मानक_त्रुटि, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = निकास_त्रुटि;
		}
	}

	वापस rc;
}

अटल पूर्णांक run_unittest_format_relative(व्योम)
{
	पूर्णांक rc = निकास_सफल;
	अक्षर buf[FORMAT_TIMESTAMP_MAX];
	अटल स्थिर काष्ठा testcase {
		काष्ठा timespec ts;
		स्थिर अक्षर * स्थिर expected;
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

	क्रम (माप_प्रकार i = 0; i < ARRAY_आकार(testcases); i++) {
		काष्ठा testcase t = testcases[i];
		पूर्णांक r = strtimespec_relative(&t.ts, buf, माप(buf));
		अगर (r) {
			ख_लिखो(मानक_त्रुटि, "Could not format '%s'\n", t.expected);
			rc = निकास_त्रुटि;
		}

		अगर (म_भेद(buf, t.expected)) {
			ख_लिखो(मानक_त्रुटि, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = निकास_त्रुटि;
		}
	}

	वापस rc;
}

पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[])
{
	काष्ठा timespec ts = { 0 };
	अक्षर buf[ISO_BUFSIZ];
	पूर्णांक r;

	अगर (argc < 2) {
		ख_लिखो(मानक_त्रुटि, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		निकास(निकास_त्रुटि);
	}

	अगर (म_भेद(argv[1], "--unittest-timestamp") == 0)
		वापस run_unittest_timestamp();
	अन्यथा अगर (म_भेद(argv[1], "--unittest-format") == 0)
		वापस run_unittest_format();
	अन्यथा अगर (म_भेद(argv[1], "--unittest-format-relative") == 0)
		वापस run_unittest_format_relative();

	अगर (म_भेद(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		अगर (r)
			errx(निकास_त्रुटि, "Can not parse '%s': %s", argv[2], म_त्रुटि(-r));
		ts.tv_sec = (समय_प्रकार) (usec / USEC_PER_SEC);
		ts.tv_nsec = (usec % USEC_PER_SEC) * NSEC_PER_USEC;
	} अन्यथा {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		अगर (argc == 3)
			ts.tv_nsec = strtos64_or_err(argv[2], "failed to parse <usec>")
				     * NSEC_PER_USEC;
	}

	strtimespec_iso(&ts, ISO_DATE, buf, माप(buf));
	म_लिखो("Date: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIME, buf, माप(buf));
	म_लिखो("Time: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_DATE | ISO_TIME | ISO_COMMAUSEC | ISO_T,
		       buf, माप(buf));
	म_लिखो("Full: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIMESTAMP_DOT, buf, माप(buf));
	म_लिखो("Zone: '%s'\n", buf);

	strtimespec_relative(&ts, buf, माप(buf));
	म_लिखो("Rel:  '%s'\n", buf);

	वापस निकास_सफल;
}

#पूर्ण_अगर /* TEST_PROGRAM_TIMEUTILS */
