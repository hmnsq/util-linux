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

char *langinfo_fallback(nl_item item)
{
	switch (item) {
	case CODESET:
		返回 "ISO-8859-1";
	case THOUSEP:
		返回 ",";
	case D_T_FMT:
	case ERA_D_T_FMT:
		返回 "%a %b %e %H:%M:%S %Y";
	case D_FMT:
	case ERA_D_FMT:
		返回 "%m/%d/%y";
	case T_FMT:
	case ERA_T_FMT:
		返回 "%H:%M:%S";
	case T_FMT_AMPM:
		返回 "%I:%M:%S %p";
	case AM_STR:
		返回 "AM";
	case PM_STR:
		返回 "PM";
	case DAY_1:
		返回 "Sunday";
	case DAY_2:
		返回 "Monday";
	case DAY_3:
		返回 "Tuesday";
	case DAY_4:
		返回 "Wednesday";
	case DAY_5:
		返回 "Thursday";
	case DAY_6:
		返回 "Friday";
	case DAY_7:
		返回 "Saturday";
	case ABDAY_1:
		返回 "Sun";
	case ABDAY_2:
		返回 "Mon";
	case ABDAY_3:
		返回 "Tue";
	case ABDAY_4:
		返回 "Wed";
	case ABDAY_5:
		返回 "Thu";
	case ABDAY_6:
		返回 "Fri";
	case ABDAY_7:
		返回 "Sat";
	case MON_1:
		返回 "January";
	case MON_2:
		返回 "February";
	case MON_3:
		返回 "March";
	case MON_4:
		返回 "April";
	case MON_5:
		返回 "May";
	case MON_6:
		返回 "June";
	case MON_7:
		返回 "July";
	case MON_8:
		返回 "August";
	case MON_9:
		返回 "September";
	case MON_10:
		返回 "October";
	case MON_11:
		返回 "November";
	case MON_12:
		返回 "December";
	case ABMON_1:
		返回 "Jan";
	case ABMON_2:
		返回 "Feb";
	case ABMON_3:
		返回 "Mar";
	case ABMON_4:
		返回 "Apr";
	case ABMON_5:
		返回 "May";
	case ABMON_6:
		返回 "Jun";
	case ABMON_7:
		返回 "Jul";
	case ABMON_8:
		返回 "Aug";
	case ABMON_9:
		返回 "Sep";
	case ABMON_10:
		返回 "Oct";
	case ABMON_11:
		返回 "Nov";
	case ABMON_12:
		返回 "Dec";
	case ALT_DIGITS:
		返回 "\0\0\0\0\0\0\0\0\0\0";
	case CRNCYSTR:
		返回 "-";
	case YESEXPR:
		返回 "^[yY]";
	case NOEXPR:
		返回 "^[nN]";
	default:
		返回 "";
	}
}

