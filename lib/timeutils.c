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

स्थिर पूर्णाङ्क parse_sec(अचर वर्ण *t, usec_t *usec)
{
	स्थिर अचर संरचना {
		अचर वर्ण *suffix;
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

	अचर वर्ण *p;
	usec_t r = 0;
	पूर्णाङ्क something = FALSE;

	प्रतिज्ञा(t);
	प्रतिज्ञा(usec);

	p = t;
	लागि (;;) {
		दीर्घ दीर्घ l, z = 0;
		वर्ण *e;
		अचिन्हित i, n = 0;

		p += म_अखोज(p, WHITESPACE);

		यदि (*p == 0) {
			यदि (!something)
				फर्काऊ -EINVAL;

			रोक;
		}

		त्रुटि_सं = 0;
		l = strtoll(p, &e, 10);

		यदि (त्रुटि_सं > 0)
			फर्काऊ -त्रुटि_सं;

		यदि (l < 0)
			फर्काऊ -दुस्फल;

		यदि (*e == '.') {
			वर्ण *b = e + 1;

			त्रुटि_सं = 0;
			z = strtoll(b, &e, 10);
			यदि (त्रुटि_सं > 0)
				फर्काऊ -त्रुटि_सं;

			यदि (z < 0)
				फर्काऊ -दुस्फल;

			यदि (e == b)
				फर्काऊ -EINVAL;

			n = e - b;

		} अन्यथा यदि (e == p)
			फर्काऊ -EINVAL;

		e += म_अखोज(e, WHITESPACE);

		लागि (i = 0; i < ARRAY_SIZE(table); i++)
			यदि (ul_startswith(e, table[i].suffix)) {
				usec_t k = (usec_t) z * table[i].usec;

				लागि (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *table[i].usec + k;
				p = e + म_माप(table[i].suffix);

				something = TRUE;
				रोक;
			}

		यदि (i >= ARRAY_SIZE(table))
			फर्काऊ -EINVAL;

	}

	*usec = r;

	फर्काऊ 0;
}

स्थिर पूर्णाङ्क parse_subseconds(अचर वर्ण *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t factor = USEC_PER_SEC / 10;

	यदि (*t != '.' && *t != ',')
		फर्काऊ -1;

	जबसम्म (*(++t)) {
		यदि (!है_अंक(*t) || factor < 1)
			फर्काऊ -1;

		ret += ((usec_t) *t - '0') * factor;
		factor /= 10;
	}

	*usec = ret;
	फर्काऊ 0;
}

स्थिर अचर वर्ण *parse_epoch_seconds(अचर वर्ण *t, संरचना पंचांग *पंचांग)
{
	int64_t s;
	समय_प्रकार st;
	पूर्णाङ्क f, c;

	f = माला_पूछो(t, "%"SCNd64"%n", &s, &c);
	यदि (f < 1)
		फर्काऊ रिक्त;
	st = s;
	यदि ((int64_t) st < s)
		फर्काऊ रिक्त;
	यदि (!localtime_r(&st, पंचांग))
		फर्काऊ रिक्त;
	फर्काऊ t + c;
}

स्थिर पूर्णाङ्क parse_timestamp_reference(समय_प्रकार x, अचर वर्ण *t, usec_t *usec)
{
	स्थिर अचर संरचना {
		अचर वर्ण *name;
		अचर पूर्णाङ्क nr;
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

	अचर वर्ण *k;
	संरचना पंचांग पंचांग, copy;
	usec_t plus = 0, minus = 0, ret = 0;
	पूर्णाङ्क r, weekday = -1;
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

	यदि (streq(t, "now"))
		गच्छ finish;

	अन्यथा यदि (streq(t, "today")) {
		पंचांग.tm_sec = पंचांग.tm_min = पंचांग.tm_hour = 0;
		गच्छ finish;

	} अन्यथा यदि (streq(t, "yesterday")) {
		पंचांग.tm_mday--;
		पंचांग.tm_sec = पंचांग.tm_min = पंचांग.tm_hour = 0;
		गच्छ finish;

	} अन्यथा यदि (streq(t, "tomorrow")) {
		पंचांग.tm_mday++;
		पंचांग.tm_sec = पंचांग.tm_min = पंचांग.tm_hour = 0;
		गच्छ finish;

	} अन्यथा यदि (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		यदि (r < 0)
			फर्काऊ r;

		गच्छ finish;
	} अन्यथा यदि (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		यदि (r < 0)
			फर्काऊ r;

		गच्छ finish;
	} अन्यथा यदि (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &पंचांग);
		यदि (k && *k == 0)
			गच्छ finish;
		अन्यथा यदि (k && parse_subseconds(k, &ret) == 0)
			गच्छ finish;

		फर्काऊ -EINVAL;
	} अन्यथा यदि (ul_endswith(t, " ago")) {
		वर्ण *z;

		z = strndup(t, म_माप(t) - 4);
		यदि (!z)
			फर्काऊ -ENOMEM;

		r = parse_sec(z, &minus);
		मुक्त(z);
		यदि (r < 0)
			फर्काऊ r;

		गच्छ finish;
	}

	लागि (i = 0; i < ARRAY_SIZE(day_nr); i++) {
		माप_प्रकार skip;

		यदि (!startswith_no_case(t, day_nr[i].name))
			जारी;

		skip = म_माप(day_nr[i].name);
		यदि (t[skip] != ' ')
			जारी;

		weekday = day_nr[i].nr;
		t += skip + 1;
		रोक;
	}

	copy = पंचांग;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &पंचांग);
	यदि (k && *k == 0)
		गच्छ finish;
	अन्यथा यदि (k && parse_subseconds(k, &ret) == 0)
		गच्छ finish;

	पंचांग = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &पंचांग);
	यदि (k && *k == 0)
		गच्छ finish;
	अन्यथा यदि (k && parse_subseconds(k, &ret) == 0)
		गच्छ finish;

	पंचांग = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &पंचांग);
	यदि (k && *k == 0)
		गच्छ finish;
	अन्यथा यदि (k && parse_subseconds(k, &ret) == 0)
		गच्छ finish;

	पंचांग = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &पंचांग);
	यदि (k && *k == 0) {
		पंचांग.tm_sec = 0;
		गच्छ finish;
	}

	पंचांग = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &पंचांग);
	यदि (k && *k == 0) {
		पंचांग.tm_sec = 0;
		गच्छ finish;
	}

	पंचांग = copy;
	k = strptime(t, "%y-%m-%d", &पंचांग);
	यदि (k && *k == 0) {
		पंचांग.tm_sec = पंचांग.tm_min = पंचांग.tm_hour = 0;
		गच्छ finish;
	}

	पंचांग = copy;
	k = strptime(t, "%Y-%m-%d", &पंचांग);
	यदि (k && *k == 0) {
		पंचांग.tm_sec = पंचांग.tm_min = पंचांग.tm_hour = 0;
		गच्छ finish;
	}

	पंचांग = copy;
	k = strptime(t, "%H:%M:%S", &पंचांग);
	यदि (k && *k == 0)
		गच्छ finish;
	अन्यथा यदि (k && parse_subseconds(k, &ret) == 0)
		गच्छ finish;

	पंचांग = copy;
	k = strptime(t, "%H:%M", &पंचांग);
	यदि (k && *k == 0) {
		पंचांग.tm_sec = 0;
		गच्छ finish;
	}

	पंचांग = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &पंचांग);
	यदि (k && *k == 0)
		गच्छ finish;
	अन्यथा यदि (k && parse_subseconds(k, &ret) == 0)
		गच्छ finish;

	फर्काऊ -EINVAL;

 finish:
	x = स_गढ़ो(&पंचांग);
	यदि (x == (समय_प्रकार)-1)
		फर्काऊ -EINVAL;

	यदि (weekday >= 0 && पंचांग.tm_wday != weekday)
		फर्काऊ -EINVAL;

	ret += (usec_t) x * USEC_PER_SEC;

	यदि (minus > ret)
		फर्काऊ -दुस्फल;
	यदि ((ret + plus) < ret)
		फर्काऊ -दुस्फल;

	ret += plus;
	ret -= minus;

	*usec = ret;

	फर्काऊ 0;
}

पूर्णाङ्क ul_parse_timestamp(अचर वर्ण *t, usec_t *usec)
{
	फर्काऊ parse_timestamp_reference(समय(रिक्त), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
पूर्णाङ्क get_gmtoff(अचर संरचना पंचांग *tp)
{
	यदि (tp->tm_isdst < 0)
		फर्काऊ 0;

#यदि HAVE_TM_GMTOFF
	फर्काऊ tp->tm_gmtoff;
#अन्यथा
	संरचना पंचांग पंचांग;
	संरचना पंचांग gtm;
	संरचना पंचांग ltm = *tp;
	समय_प्रकार lt;

	tzset();
	lt = स_गढ़ो(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	यदि (lt == (समय_प्रकार) -1) {
		यदि (! localtime_r(&lt, &पंचांग)
			|| ((ltm.tm_sec ^ पंचांग.tm_sec)
			    | (ltm.tm_min ^ पंचांग.tm_min)
			    | (ltm.tm_hour ^ पंचांग.tm_hour)
			    | (ltm.tm_mday ^ पंचांग.tm_mday)
			    | (ltm.tm_mon ^ पंचांग.tm_mon)
			    | (ltm.tm_year ^ पंचांग.tm_year)))
			फर्काऊ 0;
	}

	यदि (! gmtime_r(&lt, &gtm))
		फर्काऊ 0;

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

	फर्काऊ (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#endif
}

स्थिर पूर्णाङ्क format_iso_time(अचर संरचना पंचांग *पंचांग, uint32_t nsec, पूर्णाङ्क flags, वर्ण *buf, माप_प्रकार bufsz)
{
	uint32_t usec = nsec / NSEC_PER_USEC;
	वर्ण *p = buf;
	पूर्णाङ्क len;

	यदि (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       पंचांग->tm_year + (दीर्घ) 1900,
			       पंचांग->tm_mon + 1, पंचांग->tm_mday);
		यदि (len < 0 || (माप_प्रकार) len > bufsz)
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
		len = snprintf(p, bufsz, "%02d:%02d:%02d", पंचांग->tm_hour,
			       पंचांग->tm_min, पंचांग->tm_sec);
		यदि (len < 0 || (माप_प्रकार) len > bufsz)
			गच्छ err;
		bufsz -= len;
		p += len;
	}

	यदि (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		यदि (len < 0 || (माप_प्रकार) len > bufsz)
			गच्छ err;
		bufsz -= len;
		p += len;

	} अन्यथा यदि (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		यदि (len < 0 || (माप_प्रकार) len > bufsz)
			गच्छ err;
		bufsz -= len;
		p += len;
	} अन्यथा यदि (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		यदि (len < 0 || (माप_प्रकार) len > bufsz)
			गच्छ err;
		bufsz -= len;
		p += len;

	} अन्यथा यदि (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		यदि (len < 0 || (माप_प्रकार) len > bufsz)
			गच्छ err;
		bufsz -= len;
		p += len;
	}

	यदि (flags & ISO_TIMEZONE) {
		पूर्णाङ्क tmin  = get_gmtoff(पंचांग) / 60;
		पूर्णाङ्क zhour = tmin / 60;
		पूर्णाङ्क zmin  = असल(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		यदि (len < 0 || (माप_प्रकार) len > bufsz)
			गच्छ err;
	}
	फर्काऊ 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	फर्काऊ -1;
}

/* timespec to ISO 8601 */
पूर्णाङ्क strtimespec_iso(अचर संरचना timespec *ts, पूर्णाङ्क flags, वर्ण *buf, माप_प्रकार bufsz)
{
	संरचना पंचांग पंचांग;
	संरचना पंचांग *rc;

	यदि (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &पंचांग);
	अन्यथा
		rc = localtime_r(&ts->tv_sec, &पंचांग);

	यदि (rc)
		फर्काऊ format_iso_time(&पंचांग, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	फर्काऊ -1;
}

/* timeval to ISO 8601 */
पूर्णाङ्क strtimeval_iso(अचर संरचना timeval *tv, पूर्णाङ्क flags, वर्ण *buf, माप_प्रकार bufsz)
{
	संरचना timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_PER_USEC,
	};

	फर्काऊ strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
पूर्णाङ्क strtm_iso(अचर संरचना पंचांग *पंचांग, पूर्णाङ्क flags, वर्ण *buf, माप_प्रकार bufsz)
{
	फर्काऊ format_iso_time(पंचांग, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
पूर्णाङ्क strtime_iso(अचर समय_प्रकार *t, पूर्णाङ्क flags, वर्ण *buf, माप_प्रकार bufsz)
{
	संरचना पंचांग पंचांग;
	संरचना पंचांग *rc;

	यदि (flags & ISO_GMTIME)
		rc = gmtime_r(t, &पंचांग);
	अन्यथा
		rc = localtime_r(t, &पंचांग);

	यदि (rc)
		फर्काऊ format_iso_time(&पंचांग, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	फर्काऊ -1;
}

/* relative time functions */
स्थिर अन्तःस्थ पूर्णाङ्क time_is_thisyear(संरचना पंचांग अचर *अचर पंचांग,
				   संरचना पंचांग अचर *अचर tmnow)
{
	फर्काऊ पंचांग->tm_year == tmnow->tm_year;
}

स्थिर अन्तःस्थ पूर्णाङ्क time_is_today(संरचना पंचांग अचर *अचर पंचांग,
				संरचना पंचांग अचर *अचर tmnow)
{
	फर्काऊ (पंचांग->tm_yday == tmnow->tm_yday &&
		time_is_thisyear(पंचांग, tmnow));
}

पूर्णाङ्क strtime_short(अचर समय_प्रकार *t, संरचना timeval *now, पूर्णाङ्क flags, वर्ण *buf, माप_प्रकार bufsz)
{
	संरचना पंचांग पंचांग, tmnow;
	पूर्णाङ्क rc = 0;

	यदि (now->tv_sec == 0)
		gettimeofday(now, रिक्त);

	localtime_r(t, &पंचांग);
	localtime_r(&now->tv_sec, &tmnow);

	यदि (time_is_today(&पंचांग, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", पंचांग.tm_hour, पंचांग.tm_min);
		यदि (rc < 0 || (माप_प्रकार) rc > bufsz)
			फर्काऊ -1;
		rc = 1;

	} अन्यथा यदि (time_is_thisyear(&पंचांग, &tmnow)) {
		यदि (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = स_माला(buf, bufsz, "%b%d/%H:%M", &पंचांग);
		अन्यथा
			rc = स_माला(buf, bufsz, "%b%d", &पंचांग);
	} अन्यथा
		rc = स_माला(buf, bufsz, "%Y-%b%d", &पंचांग);

	फर्काऊ rc <= 0 ? -1 : 0;
}

पूर्णाङ्क strtimespec_relative(अचर संरचना timespec *ts, वर्ण *buf, माप_प्रकार bufsz)
{
	समय_प्रकार secs = ts->tv_sec;
	माप_प्रकार i, parts = 0;
	पूर्णाङ्क rc;

	यदि (bufsz)
		buf[0] = '\0';

	स्थिर अचर संरचना {
		अचर वर्ण * अचर suffix;
		पूर्णाङ्क width;
		int64_t secs;
	} table[] = {
		{ "y", 4, NSEC_PER_YEAR   / NSEC_PER_SEC },
		{ "d", 3, NSEC_PER_DAY    / NSEC_PER_SEC },
		{ "h", 2, NSEC_PER_HOUR   / NSEC_PER_SEC },
		{ "m", 2, NSEC_PER_MINUTE / NSEC_PER_SEC },
		{ "s", 2, NSEC_PER_SEC    / NSEC_PER_SEC },
	};

	लागि (i = 0; i < ARRAY_SIZE(table); i++) {
		यदि (secs >= table[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? table[i].width : 0,
				      secs / table[i].secs, table[i].suffix,
				      secs % table[i].secs ? " " : "");
			यदि (rc < 0 || (माप_प्रकार) rc > bufsz)
				गच्छ err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= table[i].secs;
		}
	}

	यदि (ts->tv_nsec) {
		यदि (ts->tv_nsec % NSEC_PER_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			यदि (rc < 0 || (माप_प्रकार) rc > bufsz)
				गच्छ err;
		} अन्यथा {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_PER_MSEC);
			यदि (rc < 0 || (माप_प्रकार) rc > bufsz)
				गच्छ err;
		}
	}

	फर्काऊ 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	फर्काऊ -1;
}

#ifndef HAVE_TIMEGM
समय_प्रकार timegm(संरचना पंचांग *पंचांग)
{
	अचर वर्ण *zone = दो_पर्या("TZ");
	समय_प्रकार ret;

	setenv("TZ", "", 1);
	tzset();
	ret = स_गढ़ो(पंचांग);
	यदि (zone)
		setenv("TZ", zone, 1);
	अन्यथा
		unsetenv("TZ");
	tzset();
	फर्काऊ ret;
}
#endif /* HAVE_TIMEGM */

#ifdef TEST_PROGRAM_TIMEUTILS

स्थिर पूर्णाङ्क run_unittest_timestamp(शून्य)
{
	पूर्णाङ्क rc = निकास_सफल;
	समय_प्रकार reference = 1674180427;
	स्थिर अचर संरचना testcase {
		अचर वर्ण * अचर आगत;
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

	लागि (माप_प्रकार i = 0; i < ARRAY_SIZE(testcases); i++) {
		संरचना testcase t = testcases[i];
		usec_t result;
		पूर्णाङ्क r = parse_timestamp_reference(reference, t.आगत, &result);
		यदि (r) {
			ख_लिखो(मानक_त्रुटि, "Could not parse '%s'\n", t.आगत);
			rc = निकास_त्रुटि;
		}

		यदि (result != t.expected) {
			ख_लिखो(मानक_त्रुटि, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.आगत, result, t.expected);
			rc = निकास_त्रुटि;
		}
	}

	फर्काऊ rc;
}

स्थिर पूर्णाङ्क run_unittest_format(शून्य)
{
	पूर्णाङ्क rc = निकास_सफल;
	अचर संरचना timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	वर्ण buf[FORMAT_TIMESTAMP_MAX];
	स्थिर अचर संरचना testcase {
		पूर्णाङ्क flags;
		अचर वर्ण * अचर expected;
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

	लागि (माप_प्रकार i = 0; i < ARRAY_SIZE(testcases); i++) {
		संरचना testcase t = testcases[i];
		पूर्णाङ्क r = strtimespec_iso(&ts, t.flags, buf, परिमाण(buf));
		यदि (r) {
			ख_लिखो(मानक_त्रुटि, "Could not format '%s'\n", t.expected);
			rc = निकास_त्रुटि;
		}

		यदि (म_भेद(buf, t.expected)) {
			ख_लिखो(मानक_त्रुटि, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = निकास_त्रुटि;
		}
	}

	फर्काऊ rc;
}

स्थिर पूर्णाङ्क run_unittest_format_relative(शून्य)
{
	पूर्णाङ्क rc = निकास_सफल;
	वर्ण buf[FORMAT_TIMESTAMP_MAX];
	स्थिर अचर संरचना testcase {
		संरचना timespec ts;
		अचर वर्ण * अचर expected;
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

	लागि (माप_प्रकार i = 0; i < ARRAY_SIZE(testcases); i++) {
		संरचना testcase t = testcases[i];
		पूर्णाङ्क r = strtimespec_relative(&t.ts, buf, परिमाण(buf));
		यदि (r) {
			ख_लिखो(मानक_त्रुटि, "Could not format '%s'\n", t.expected);
			rc = निकास_त्रुटि;
		}

		यदि (म_भेद(buf, t.expected)) {
			ख_लिखो(मानक_त्रुटि, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = निकास_त्रुटि;
		}
	}

	फर्काऊ rc;
}

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	संरचना timespec ts = { 0 };
	वर्ण buf[ISO_BUFSIZ];
	पूर्णाङ्क r;

	यदि (argc < 2) {
		ख_लिखो(मानक_त्रुटि, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		निकास(निकास_त्रुटि);
	}

	यदि (म_भेद(argv[1], "--unittest-timestamp") == 0)
		फर्काऊ run_unittest_timestamp();
	अन्यथा यदि (म_भेद(argv[1], "--unittest-format") == 0)
		फर्काऊ run_unittest_format();
	अन्यथा यदि (म_भेद(argv[1], "--unittest-format-relative") == 0)
		फर्काऊ run_unittest_format_relative();

	यदि (म_भेद(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		यदि (r)
			errx(निकास_त्रुटि, "Can not parse '%s': %s", argv[2], म_त्रुटि(-r));
		ts.tv_sec = (समय_प्रकार) (usec / USEC_PER_SEC);
		ts.tv_nsec = (usec % USEC_PER_SEC) * NSEC_PER_USEC;
	} अन्यथा {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		यदि (argc == 3)
			ts.tv_nsec = strtos64_or_err(argv[2], "failed to parse <usec>")
				     * NSEC_PER_USEC;
	}

	strtimespec_iso(&ts, ISO_DATE, buf, परिमाण(buf));
	म_लिखो("Date: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIME, buf, परिमाण(buf));
	म_लिखो("Time: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_DATE | ISO_TIME | ISO_COMMAUSEC | ISO_T,
		       buf, परिमाण(buf));
	म_लिखो("Full: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIMESTAMP_DOT, buf, परिमाण(buf));
	म_लिखो("Zone: '%s'\n", buf);

	strtimespec_relative(&ts, buf, परिमाण(buf));
	म_लिखो("Rel:  '%s'\n", buf);

	फर्काऊ निकास_सफल;
}

#endif /* TEST_PROGRAM_TIMEUTILS */
