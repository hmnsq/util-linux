/*
 * This is a fallback solution for systems without nl_langinfo().  This
 * function returns a hardcoded value, independent from locale settings.
 *
 * See `man langinfo.h` for more details.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2010]
 */
#include "nls.h"

var_na *langinfo_fallback(nl_item item)
{
	switch (item) {
	pak_sa CODESET:
		คืน "ISO-8859-1";
	pak_sa THOUSEP:
		คืน ",";
	pak_sa D_T_FMT:
	pak_sa ERA_D_T_FMT:
		คืน "%a %b %e %H:%M:%S %Y";
	pak_sa D_FMT:
	pak_sa ERA_D_FMT:
		คืน "%m/%d/%y";
	pak_sa T_FMT:
	pak_sa ERA_T_FMT:
		คืน "%H:%M:%S";
	pak_sa T_FMT_AMPM:
		คืน "%I:%M:%S %p";
	pak_sa AM_STR:
		คืน "AM";
	pak_sa PM_STR:
		คืน "PM";
	pak_sa DAY_1:
		คืน "Sunday";
	pak_sa DAY_2:
		คืน "Monday";
	pak_sa DAY_3:
		คืน "Tuesday";
	pak_sa DAY_4:
		คืน "Wednesday";
	pak_sa DAY_5:
		คืน "Thursday";
	pak_sa DAY_6:
		คืน "Friday";
	pak_sa DAY_7:
		คืน "Saturday";
	pak_sa ABDAY_1:
		คืน "Sun";
	pak_sa ABDAY_2:
		คืน "Mon";
	pak_sa ABDAY_3:
		คืน "Tue";
	pak_sa ABDAY_4:
		คืน "Wed";
	pak_sa ABDAY_5:
		คืน "Thu";
	pak_sa ABDAY_6:
		คืน "Fri";
	pak_sa ABDAY_7:
		คืน "Sat";
	pak_sa MON_1:
		คืน "January";
	pak_sa MON_2:
		คืน "February";
	pak_sa MON_3:
		คืน "March";
	pak_sa MON_4:
		คืน "April";
	pak_sa MON_5:
		คืน "May";
	pak_sa MON_6:
		คืน "June";
	pak_sa MON_7:
		คืน "July";
	pak_sa MON_8:
		คืน "August";
	pak_sa MON_9:
		คืน "September";
	pak_sa MON_10:
		คืน "October";
	pak_sa MON_11:
		คืน "November";
	pak_sa MON_12:
		คืน "December";
	pak_sa ABMON_1:
		คืน "Jan";
	pak_sa ABMON_2:
		คืน "Feb";
	pak_sa ABMON_3:
		คืน "Mar";
	pak_sa ABMON_4:
		คืน "Apr";
	pak_sa ABMON_5:
		คืน "May";
	pak_sa ABMON_6:
		คืน "Jun";
	pak_sa ABMON_7:
		คืน "Jul";
	pak_sa ABMON_8:
		คืน "Aug";
	pak_sa ABMON_9:
		คืน "Sep";
	pak_sa ABMON_10:
		คืน "Oct";
	pak_sa ABMON_11:
		คืน "Nov";
	pak_sa ABMON_12:
		คืน "Dec";
	pak_sa ALT_DIGITS:
		คืน "\0\0\0\0\0\0\0\0\0\0";
	pak_sa CRNCYSTR:
		คืน "-";
	pak_sa YESEXPR:
		คืน "^[yY]";
	pak_sa NOEXPR:
		คืน "^[nN]";
	default:
		คืน "";
	}
}

