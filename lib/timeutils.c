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
#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <sys/time.h>
#അന്തര്ഭാവ <inttypes.h>

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "nls.h"
#അന്തര്ഭാവ "strutils.h"
#അന്തര്ഭാവ "timeutils.h"

#define WHITESPACE " \t\n\r"

#define streq(a,b) (strcmp((a),(b)) == 0)

സ്ഥിതം പൂർണ്ണസംഖ്യ parse_sec(സ്ഥിരം അക്ഷരം *t, usec_t *usec)
{
	സ്ഥിതം സ്ഥിരം ഘടന {
		സ്ഥിരം അക്ഷരം *suffix;
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

	സ്ഥിരം അക്ഷരം *p;
	usec_t r = 0;
	പൂർണ്ണസംഖ്യ something = FALSE;

	പ്രതിജ്ഞാ(t);
	പ്രതിജ്ഞാ(usec);

	p = t;
	വേണ്ടി (;;) {
		long long l, z = 0;
		അക്ഷരം *e;
		unsigned i, n = 0;

		p += strspn(p, WHITESPACE);

		എങ്കിൽ (*p == 0) {
			എങ്കിൽ (!something)
				തിരികെ -EINVAL;

			നിർത്തുക;
		}

		errno = 0;
		l = strtoll(p, &e, 10);

		എങ്കിൽ (errno > 0)
			തിരികെ -errno;

		എങ്കിൽ (l < 0)
			തിരികെ -ERANGE;

		എങ്കിൽ (*e == '.') {
			അക്ഷരം *b = e + 1;

			errno = 0;
			z = strtoll(b, &e, 10);
			എങ്കിൽ (errno > 0)
				തിരികെ -errno;

			എങ്കിൽ (z < 0)
				തിരികെ -ERANGE;

			എങ്കിൽ (e == b)
				തിരികെ -EINVAL;

			n = e - b;

		} അല്ലെങ്കിൽ എങ്കിൽ (e == p)
			തിരികെ -EINVAL;

		e += strspn(e, WHITESPACE);

		വേണ്ടി (i = 0; i < ARRAY_SIZE(table); i++)
			എങ്കിൽ (ul_startswith(e, table[i].suffix)) {
				usec_t k = (usec_t) z * table[i].usec;

				വേണ്ടി (; n > 0; n--)
					k /= 10;

				r += (usec_t) l *table[i].usec + k;
				p = e + strlen(table[i].suffix);

				something = TRUE;
				നിർത്തുക;
			}

		എങ്കിൽ (i >= ARRAY_SIZE(table))
			തിരികെ -EINVAL;

	}

	*usec = r;

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ parse_subseconds(സ്ഥിരം അക്ഷരം *t, usec_t *usec)
{
	usec_t ret = 0;
	usec_t factor = USEC_PER_SEC / 10;

	എങ്കിൽ (*t != '.' && *t != ',')
		തിരികെ -1;

	ആകുമ്പോൾ (*(++t)) {
		എങ്കിൽ (!isdigit(*t) || factor < 1)
			തിരികെ -1;

		ret += ((usec_t) *t - '0') * factor;
		factor /= 10;
	}

	*usec = ret;
	തിരികെ 0;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *parse_epoch_seconds(സ്ഥിരം അക്ഷരം *t, ഘടന tm *tm)
{
	int64_t s;
	time_t st;
	പൂർണ്ണസംഖ്യ f, c;

	f = sscanf(t, "%"SCNd64"%n", &s, &c);
	എങ്കിൽ (f < 1)
		തിരികെ ശൂന്യ;
	st = s;
	എങ്കിൽ ((int64_t) st < s)
		തിരികെ ശൂന്യ;
	എങ്കിൽ (!localtime_r(&st, tm))
		തിരികെ ശൂന്യ;
	തിരികെ t + c;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ parse_timestamp_reference(time_t x, സ്ഥിരം അക്ഷരം *t, usec_t *usec)
{
	സ്ഥിതം സ്ഥിരം ഘടന {
		സ്ഥിരം അക്ഷരം *name;
		സ്ഥിരം പൂർണ്ണസംഖ്യ nr;
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

	സ്ഥിരം അക്ഷരം *k;
	ഘടന tm tm, copy;
	usec_t plus = 0, minus = 0, ret = 0;
	പൂർണ്ണസംഖ്യ r, weekday = -1;
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

	പ്രതിജ്ഞാ(t);
	പ്രതിജ്ഞാ(usec);

	localtime_r(&x, &tm);
	tm.tm_isdst = -1;

	എങ്കിൽ (streq(t, "now"))
		പോകുക finish;

	അല്ലെങ്കിൽ എങ്കിൽ (streq(t, "today")) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		പോകുക finish;

	} അല്ലെങ്കിൽ എങ്കിൽ (streq(t, "yesterday")) {
		tm.tm_mday--;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		പോകുക finish;

	} അല്ലെങ്കിൽ എങ്കിൽ (streq(t, "tomorrow")) {
		tm.tm_mday++;
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		പോകുക finish;

	} അല്ലെങ്കിൽ എങ്കിൽ (t[0] == '+') {

		r = parse_sec(t + 1, &plus);
		എങ്കിൽ (r < 0)
			തിരികെ r;

		പോകുക finish;
	} അല്ലെങ്കിൽ എങ്കിൽ (t[0] == '-') {

		r = parse_sec(t + 1, &minus);
		എങ്കിൽ (r < 0)
			തിരികെ r;

		പോകുക finish;
	} അല്ലെങ്കിൽ എങ്കിൽ (t[0] == '@') {
		k = parse_epoch_seconds(t + 1, &tm);
		എങ്കിൽ (k && *k == 0)
			പോകുക finish;
		അല്ലെങ്കിൽ എങ്കിൽ (k && parse_subseconds(k, &ret) == 0)
			പോകുക finish;

		തിരികെ -EINVAL;
	} അല്ലെങ്കിൽ എങ്കിൽ (ul_endswith(t, " ago")) {
		അക്ഷരം *z;

		z = strndup(t, strlen(t) - 4);
		എങ്കിൽ (!z)
			തിരികെ -ENOMEM;

		r = parse_sec(z, &minus);
		സ്വതന്ത്രം(z);
		എങ്കിൽ (r < 0)
			തിരികെ r;

		പോകുക finish;
	}

	വേണ്ടി (i = 0; i < ARRAY_SIZE(day_nr); i++) {
		size_t skip;

		എങ്കിൽ (!startswith_no_case(t, day_nr[i].name))
			തുടരുക;

		skip = strlen(day_nr[i].name);
		എങ്കിൽ (t[skip] != ' ')
			തുടരുക;

		weekday = day_nr[i].nr;
		t += skip + 1;
		നിർത്തുക;
	}

	copy = tm;
	k = strptime(t, "%y-%m-%d %H:%M:%S", &tm);
	എങ്കിൽ (k && *k == 0)
		പോകുക finish;
	അല്ലെങ്കിൽ എങ്കിൽ (k && parse_subseconds(k, &ret) == 0)
		പോകുക finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M:%S", &tm);
	എങ്കിൽ (k && *k == 0)
		പോകുക finish;
	അല്ലെങ്കിൽ എങ്കിൽ (k && parse_subseconds(k, &ret) == 0)
		പോകുക finish;

	tm = copy;
	k = strptime(t, "%Y-%m-%dT%H:%M:%S", &tm);
	എങ്കിൽ (k && *k == 0)
		പോകുക finish;
	അല്ലെങ്കിൽ എങ്കിൽ (k && parse_subseconds(k, &ret) == 0)
		പോകുക finish;

	tm = copy;
	k = strptime(t, "%y-%m-%d %H:%M", &tm);
	എങ്കിൽ (k && *k == 0) {
		tm.tm_sec = 0;
		പോകുക finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d %H:%M", &tm);
	എങ്കിൽ (k && *k == 0) {
		tm.tm_sec = 0;
		പോകുക finish;
	}

	tm = copy;
	k = strptime(t, "%y-%m-%d", &tm);
	എങ്കിൽ (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		പോകുക finish;
	}

	tm = copy;
	k = strptime(t, "%Y-%m-%d", &tm);
	എങ്കിൽ (k && *k == 0) {
		tm.tm_sec = tm.tm_min = tm.tm_hour = 0;
		പോകുക finish;
	}

	tm = copy;
	k = strptime(t, "%H:%M:%S", &tm);
	എങ്കിൽ (k && *k == 0)
		പോകുക finish;
	അല്ലെങ്കിൽ എങ്കിൽ (k && parse_subseconds(k, &ret) == 0)
		പോകുക finish;

	tm = copy;
	k = strptime(t, "%H:%M", &tm);
	എങ്കിൽ (k && *k == 0) {
		tm.tm_sec = 0;
		പോകുക finish;
	}

	tm = copy;
	k = strptime(t, "%Y%m%d%H%M%S", &tm);
	എങ്കിൽ (k && *k == 0)
		പോകുക finish;
	അല്ലെങ്കിൽ എങ്കിൽ (k && parse_subseconds(k, &ret) == 0)
		പോകുക finish;

	തിരികെ -EINVAL;

 finish:
	x = mktime(&tm);
	എങ്കിൽ (x == (time_t)-1)
		തിരികെ -EINVAL;

	എങ്കിൽ (weekday >= 0 && tm.tm_wday != weekday)
		തിരികെ -EINVAL;

	ret += (usec_t) x * USEC_PER_SEC;

	എങ്കിൽ (minus > ret)
		തിരികെ -ERANGE;
	എങ്കിൽ ((ret + plus) < ret)
		തിരികെ -ERANGE;

	ret += plus;
	ret -= minus;

	*usec = ret;

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ ul_parse_timestamp(സ്ഥിരം അക്ഷരം *t, usec_t *usec)
{
	തിരികെ parse_timestamp_reference(time(ശൂന്യ), t, usec);
}

/* Returns the difference in seconds between its argument and GMT. If if TP is
 * invalid or no DST information is available default to UTC, that is, zero.
 * tzset is called so, for example, 'TZ="UTC" hwclock' will work as expected.
 * Derived from glibc/time/strftime_l.c
 */
പൂർണ്ണസംഖ്യ get_gmtoff(സ്ഥിരം ഘടന tm *tp)
{
	എങ്കിൽ (tp->tm_isdst < 0)
		തിരികെ 0;

#എങ്കിൽ HAVE_TM_GMTOFF
	തിരികെ tp->tm_gmtoff;
#അല്ലെങ്കിൽ
	ഘടന tm tm;
	ഘടന tm gtm;
	ഘടന tm ltm = *tp;
	time_t lt;

	tzset();
	lt = mktime(&ltm);
	/* Check if mktime returning -1 is an error or a valid time_t */
	എങ്കിൽ (lt == (time_t) -1) {
		എങ്കിൽ (! localtime_r(&lt, &tm)
			|| ((ltm.tm_sec ^ tm.tm_sec)
			    | (ltm.tm_min ^ tm.tm_min)
			    | (ltm.tm_hour ^ tm.tm_hour)
			    | (ltm.tm_mday ^ tm.tm_mday)
			    | (ltm.tm_mon ^ tm.tm_mon)
			    | (ltm.tm_year ^ tm.tm_year)))
			തിരികെ 0;
	}

	എങ്കിൽ (! gmtime_r(&lt, &gtm))
		തിരികെ 0;

	/* Calculate the GMT offset, that is, the difference between the
	 * TP argument (ltm) and GMT (gtm).
	 *
	 * Compute intervening leap days correctly even if year is negative.
	 * Take care to avoid int overflow in leap day calculations, but it's OK
	 * to assume that A and B are close to each other.
	 */
	പൂർണ്ണസംഖ്യ a4 = (ltm.tm_year >> 2) + (1900 >> 2) - ! (ltm.tm_year & 3);
	പൂർണ്ണസംഖ്യ b4 = (gtm.tm_year >> 2) + (1900 >> 2) - ! (gtm.tm_year & 3);
	പൂർണ്ണസംഖ്യ a100 = a4 / 25 - (a4 % 25 < 0);
	പൂർണ്ണസംഖ്യ b100 = b4 / 25 - (b4 % 25 < 0);
	പൂർണ്ണസംഖ്യ a400 = a100 >> 2;
	പൂർണ്ണസംഖ്യ b400 = b100 >> 2;
	പൂർണ്ണസംഖ്യ intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

	പൂർണ്ണസംഖ്യ years = ltm.tm_year - gtm.tm_year;
	പൂർണ്ണസംഖ്യ days = (365 * years + intervening_leap_days
		    + (ltm.tm_yday - gtm.tm_yday));

	തിരികെ (60 * (60 * (24 * days + (ltm.tm_hour - gtm.tm_hour))
		+ (ltm.tm_min - gtm.tm_min)) + (ltm.tm_sec - gtm.tm_sec));
#endif
}

സ്ഥിതം പൂർണ്ണസംഖ്യ format_iso_time(സ്ഥിരം ഘടന tm *tm, uint32_t nsec, പൂർണ്ണസംഖ്യ flags, അക്ഷരം *buf, size_t bufsz)
{
	uint32_t usec = nsec / NSEC_PER_USEC;
	അക്ഷരം *p = buf;
	പൂർണ്ണസംഖ്യ len;

	എങ്കിൽ (flags & ISO_DATE) {
		len = snprintf(p, bufsz, "%4ld-%.2d-%.2d",
			       tm->tm_year + (long) 1900,
			       tm->tm_mon + 1, tm->tm_mday);
		എങ്കിൽ (len < 0 || (size_t) len > bufsz)
			പോകുക err;
		bufsz -= len;
		p += len;
	}

	എങ്കിൽ ((flags & ISO_DATE) && (flags & ISO_TIME)) {
		എങ്കിൽ (bufsz < 1)
			പോകുക err;
		*p++ = (flags & ISO_T) ? 'T' : ' ';
		bufsz--;
	}

	എങ്കിൽ (flags & ISO_TIME) {
		len = snprintf(p, bufsz, "%02d:%02d:%02d", tm->tm_hour,
			       tm->tm_min, tm->tm_sec);
		എങ്കിൽ (len < 0 || (size_t) len > bufsz)
			പോകുക err;
		bufsz -= len;
		p += len;
	}

	എങ്കിൽ (flags & ISO_DOTNSEC) {
		len = snprintf(p, bufsz, ".%09"PRIu32, nsec);
		എങ്കിൽ (len < 0 || (size_t) len > bufsz)
			പോകുക err;
		bufsz -= len;
		p += len;

	} അല്ലെങ്കിൽ എങ്കിൽ (flags & ISO_COMMANSEC) {
		len = snprintf(p, bufsz, ",%09"PRIu32, nsec);
		എങ്കിൽ (len < 0 || (size_t) len > bufsz)
			പോകുക err;
		bufsz -= len;
		p += len;
	} അല്ലെങ്കിൽ എങ്കിൽ (flags & ISO_DOTUSEC) {
		len = snprintf(p, bufsz, ".%06"PRIu32, usec);
		എങ്കിൽ (len < 0 || (size_t) len > bufsz)
			പോകുക err;
		bufsz -= len;
		p += len;

	} അല്ലെങ്കിൽ എങ്കിൽ (flags & ISO_COMMAUSEC) {
		len = snprintf(p, bufsz, ",%06"PRIu32, usec);
		എങ്കിൽ (len < 0 || (size_t) len > bufsz)
			പോകുക err;
		bufsz -= len;
		p += len;
	}

	എങ്കിൽ (flags & ISO_TIMEZONE) {
		പൂർണ്ണസംഖ്യ tmin  = get_gmtoff(tm) / 60;
		പൂർണ്ണസംഖ്യ zhour = tmin / 60;
		പൂർണ്ണസംഖ്യ zmin  = abs(tmin % 60);
		len = snprintf(p, bufsz, "%+03d:%02d", zhour,zmin);
		എങ്കിൽ (len < 0 || (size_t) len > bufsz)
			പോകുക err;
	}
	തിരികെ 0;
 err:
	warnx(_("format_iso_time: buffer overflow."));
	തിരികെ -1;
}

/* timespec to ISO 8601 */
പൂർണ്ണസംഖ്യ strtimespec_iso(സ്ഥിരം ഘടന timespec *ts, പൂർണ്ണസംഖ്യ flags, അക്ഷരം *buf, size_t bufsz)
{
	ഘടന tm tm;
	ഘടന tm *rc;

	എങ്കിൽ (flags & ISO_GMTIME)
		rc = gmtime_r(&ts->tv_sec, &tm);
	അല്ലെങ്കിൽ
		rc = localtime_r(&ts->tv_sec, &tm);

	എങ്കിൽ (rc)
		തിരികെ format_iso_time(&tm, ts->tv_nsec, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)(ts->tv_sec));
	തിരികെ -1;
}

/* timeval to ISO 8601 */
പൂർണ്ണസംഖ്യ strtimeval_iso(സ്ഥിരം ഘടന timeval *tv, പൂർണ്ണസംഖ്യ flags, അക്ഷരം *buf, size_t bufsz)
{
	ഘടന timespec ts = {
		.tv_sec = tv->tv_sec,
		.tv_nsec = tv->tv_usec * NSEC_PER_USEC,
	};

	തിരികെ strtimespec_iso(&ts, flags, buf, bufsz);
}

/* struct tm to ISO 8601 */
പൂർണ്ണസംഖ്യ strtm_iso(സ്ഥിരം ഘടന tm *tm, പൂർണ്ണസംഖ്യ flags, അക്ഷരം *buf, size_t bufsz)
{
	തിരികെ format_iso_time(tm, 0, flags, buf, bufsz);
}

/* time_t to ISO 8601 */
പൂർണ്ണസംഖ്യ strtime_iso(സ്ഥിരം time_t *t, പൂർണ്ണസംഖ്യ flags, അക്ഷരം *buf, size_t bufsz)
{
	ഘടന tm tm;
	ഘടന tm *rc;

	എങ്കിൽ (flags & ISO_GMTIME)
		rc = gmtime_r(t, &tm);
	അല്ലെങ്കിൽ
		rc = localtime_r(t, &tm);

	എങ്കിൽ (rc)
		തിരികെ format_iso_time(&tm, 0, flags, buf, bufsz);

	warnx(_("time %jd is out of range."), (intmax_t)*t);
	തിരികെ -1;
}

/* relative time functions */
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ time_is_thisyear(ഘടന tm സ്ഥിരം *സ്ഥിരം tm,
				   ഘടന tm സ്ഥിരം *സ്ഥിരം tmnow)
{
	തിരികെ tm->tm_year == tmnow->tm_year;
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ time_is_today(ഘടന tm സ്ഥിരം *സ്ഥിരം tm,
				ഘടന tm സ്ഥിരം *സ്ഥിരം tmnow)
{
	തിരികെ (tm->tm_yday == tmnow->tm_yday &&
		time_is_thisyear(tm, tmnow));
}

പൂർണ്ണസംഖ്യ strtime_short(സ്ഥിരം time_t *t, ഘടന timeval *now, പൂർണ്ണസംഖ്യ flags, അക്ഷരം *buf, size_t bufsz)
{
	ഘടന tm tm, tmnow;
	പൂർണ്ണസംഖ്യ rc = 0;

	എങ്കിൽ (now->tv_sec == 0)
		gettimeofday(now, ശൂന്യ);

	localtime_r(t, &tm);
	localtime_r(&now->tv_sec, &tmnow);

	എങ്കിൽ (time_is_today(&tm, &tmnow)) {
		rc = snprintf(buf, bufsz, "%02d:%02d", tm.tm_hour, tm.tm_min);
		എങ്കിൽ (rc < 0 || (size_t) rc > bufsz)
			തിരികെ -1;
		rc = 1;

	} അല്ലെങ്കിൽ എങ്കിൽ (time_is_thisyear(&tm, &tmnow)) {
		എങ്കിൽ (flags & UL_SHORTTIME_THISYEAR_HHMM)
			rc = strftime(buf, bufsz, "%b%d/%H:%M", &tm);
		അല്ലെങ്കിൽ
			rc = strftime(buf, bufsz, "%b%d", &tm);
	} അല്ലെങ്കിൽ
		rc = strftime(buf, bufsz, "%Y-%b%d", &tm);

	തിരികെ rc <= 0 ? -1 : 0;
}

പൂർണ്ണസംഖ്യ strtimespec_relative(സ്ഥിരം ഘടന timespec *ts, അക്ഷരം *buf, size_t bufsz)
{
	time_t secs = ts->tv_sec;
	size_t i, parts = 0;
	പൂർണ്ണസംഖ്യ rc;

	എങ്കിൽ (bufsz)
		buf[0] = '\0';

	സ്ഥിതം സ്ഥിരം ഘടന {
		സ്ഥിരം അക്ഷരം * സ്ഥിരം suffix;
		പൂർണ്ണസംഖ്യ width;
		int64_t secs;
	} table[] = {
		{ "y", 4, NSEC_PER_YEAR   / NSEC_PER_SEC },
		{ "d", 3, NSEC_PER_DAY    / NSEC_PER_SEC },
		{ "h", 2, NSEC_PER_HOUR   / NSEC_PER_SEC },
		{ "m", 2, NSEC_PER_MINUTE / NSEC_PER_SEC },
		{ "s", 2, NSEC_PER_SEC    / NSEC_PER_SEC },
	};

	വേണ്ടി (i = 0; i < ARRAY_SIZE(table); i++) {
		എങ്കിൽ (secs >= table[i].secs) {
			rc = snprintf(buf, bufsz,
				      "%*"PRId64"%s%s",
				      parts ? table[i].width : 0,
				      secs / table[i].secs, table[i].suffix,
				      secs % table[i].secs ? " " : "");
			എങ്കിൽ (rc < 0 || (size_t) rc > bufsz)
				പോകുക err;
			parts++;
			buf += rc;
			bufsz -= rc;
			secs %= table[i].secs;
		}
	}

	എങ്കിൽ (ts->tv_nsec) {
		എങ്കിൽ (ts->tv_nsec % NSEC_PER_MSEC) {
			rc = snprintf(buf, bufsz, "%*ldns",
				      parts ? 10 : 0, ts->tv_nsec);
			എങ്കിൽ (rc < 0 || (size_t) rc > bufsz)
				പോകുക err;
		} അല്ലെങ്കിൽ {
			rc = snprintf(buf, bufsz, "%*llums",
				      parts ? 4 : 0, ts->tv_nsec / NSEC_PER_MSEC);
			എങ്കിൽ (rc < 0 || (size_t) rc > bufsz)
				പോകുക err;
		}
	}

	തിരികെ 0;
 err:
	warnx(_("format_reltime: buffer overflow."));
	തിരികെ -1;
}

#ifndef HAVE_TIMEGM
time_t timegm(ഘടന tm *tm)
{
	സ്ഥിരം അക്ഷരം *zone = getenv("TZ");
	time_t ret;

	setenv("TZ", "", 1);
	tzset();
	ret = mktime(tm);
	എങ്കിൽ (zone)
		setenv("TZ", zone, 1);
	അല്ലെങ്കിൽ
		unsetenv("TZ");
	tzset();
	തിരികെ ret;
}
#endif /* HAVE_TIMEGM */

#ifdef TEST_PROGRAM_TIMEUTILS

സ്ഥിതം പൂർണ്ണസംഖ്യ run_unittest_timestamp(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ rc = EXIT_SUCCESS;
	time_t reference = 1674180427;
	സ്ഥിതം സ്ഥിരം ഘടന testcase {
		സ്ഥിരം അക്ഷരം * സ്ഥിരം നിവേശം;
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

	വേണ്ടി (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		ഘടന testcase t = testcases[i];
		usec_t result;
		പൂർണ്ണസംഖ്യ r = parse_timestamp_reference(reference, t.നിവേശം, &result);
		എങ്കിൽ (r) {
			fprintf(stderr, "Could not parse '%s'\n", t.നിവേശം);
			rc = EXIT_FAILURE;
		}

		എങ്കിൽ (result != t.expected) {
			fprintf(stderr, "#%02zu %-25s: %"PRIu64" != %"PRIu64"\n",
				i, t.നിവേശം, result, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	തിരികെ rc;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ run_unittest_format(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ rc = EXIT_SUCCESS;
	സ്ഥിരം ഘടന timespec ts = {
		.tv_sec = 1674180427,
		.tv_nsec = 12345,
	};
	അക്ഷരം buf[FORMAT_TIMESTAMP_MAX];
	സ്ഥിതം സ്ഥിരം ഘടന testcase {
		പൂർണ്ണസംഖ്യ flags;
		സ്ഥിരം അക്ഷരം * സ്ഥിരം expected;
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

	വേണ്ടി (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		ഘടന testcase t = testcases[i];
		പൂർണ്ണസംഖ്യ r = strtimespec_iso(&ts, t.flags, buf, വലുപ്പം(buf));
		എങ്കിൽ (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		എങ്കിൽ (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu %-20s != %-20s\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	തിരികെ rc;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ run_unittest_format_relative(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ rc = EXIT_SUCCESS;
	അക്ഷരം buf[FORMAT_TIMESTAMP_MAX];
	സ്ഥിതം സ്ഥിരം ഘടന testcase {
		ഘടന timespec ts;
		സ്ഥിരം അക്ഷരം * സ്ഥിരം expected;
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

	വേണ്ടി (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
		ഘടന testcase t = testcases[i];
		പൂർണ്ണസംഖ്യ r = strtimespec_relative(&t.ts, buf, വലുപ്പം(buf));
		എങ്കിൽ (r) {
			fprintf(stderr, "Could not format '%s'\n", t.expected);
			rc = EXIT_FAILURE;
		}

		എങ്കിൽ (strcmp(buf, t.expected)) {
			fprintf(stderr, "#%02zu '%-20s' != '%-20s'\n", i, buf, t.expected);
			rc = EXIT_FAILURE;
		}
	}

	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	ഘടന timespec ts = { 0 };
	അക്ഷരം buf[ISO_BUFSIZ];
	പൂർണ്ണസംഖ്യ r;

	എങ്കിൽ (argc < 2) {
		fprintf(stderr, "usage: %s [<time> [<usec>]] | [--timestamp <str>] | [--unittest-timestamp]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	എങ്കിൽ (strcmp(argv[1], "--unittest-timestamp") == 0)
		തിരികെ run_unittest_timestamp();
	അല്ലെങ്കിൽ എങ്കിൽ (strcmp(argv[1], "--unittest-format") == 0)
		തിരികെ run_unittest_format();
	അല്ലെങ്കിൽ എങ്കിൽ (strcmp(argv[1], "--unittest-format-relative") == 0)
		തിരികെ run_unittest_format_relative();

	എങ്കിൽ (strcmp(argv[1], "--timestamp") == 0) {
		usec_t usec = 0;

		r = ul_parse_timestamp(argv[2], &usec);
		എങ്കിൽ (r)
			errx(EXIT_FAILURE, "Can not parse '%s': %s", argv[2], strerror(-r));
		ts.tv_sec = (time_t) (usec / USEC_PER_SEC);
		ts.tv_nsec = (usec % USEC_PER_SEC) * NSEC_PER_USEC;
	} അല്ലെങ്കിൽ {
		ts.tv_sec = strtos64_or_err(argv[1], "failed to parse <time>");
		എങ്കിൽ (argc == 3)
			ts.tv_nsec = strtos64_or_err(argv[2], "failed to parse <usec>")
				     * NSEC_PER_USEC;
	}

	strtimespec_iso(&ts, ISO_DATE, buf, വലുപ്പം(buf));
	printf("Date: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIME, buf, വലുപ്പം(buf));
	printf("Time: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_DATE | ISO_TIME | ISO_COMMAUSEC | ISO_T,
		       buf, വലുപ്പം(buf));
	printf("Full: '%s'\n", buf);

	strtimespec_iso(&ts, ISO_TIMESTAMP_DOT, buf, വലുപ്പം(buf));
	printf("Zone: '%s'\n", buf);

	strtimespec_relative(&ts, buf, വലുപ്പം(buf));
	printf("Rel:  '%s'\n", buf);

	തിരികെ EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_TIMEUTILS */
