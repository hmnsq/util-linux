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
		вернуть "ISO-8859-1";
	case THOUSEP:
		вернуть ",";
	case D_T_FMT:
	case ERA_D_T_FMT:
		вернуть "%a %b %e %H:%M:%S %Y";
	case D_FMT:
	case ERA_D_FMT:
		вернуть "%m/%d/%y";
	case T_FMT:
	case ERA_T_FMT:
		вернуть "%H:%M:%S";
	case T_FMT_AMPM:
		вернуть "%I:%M:%S %p";
	case AM_STR:
		вернуть "AM";
	case PM_STR:
		вернуть "PM";
	case DAY_1:
		вернуть "Sunday";
	case DAY_2:
		вернуть "Monday";
	case DAY_3:
		вернуть "Tuesday";
	case DAY_4:
		вернуть "Wednesday";
	case DAY_5:
		вернуть "Thursday";
	case DAY_6:
		вернуть "Friday";
	case DAY_7:
		вернуть "Saturday";
	case ABDAY_1:
		вернуть "Sun";
	case ABDAY_2:
		вернуть "Mon";
	case ABDAY_3:
		вернуть "Tue";
	case ABDAY_4:
		вернуть "Wed";
	case ABDAY_5:
		вернуть "Thu";
	case ABDAY_6:
		вернуть "Fri";
	case ABDAY_7:
		вернуть "Sat";
	case MON_1:
		вернуть "January";
	case MON_2:
		вернуть "February";
	case MON_3:
		вернуть "March";
	case MON_4:
		вернуть "April";
	case MON_5:
		вернуть "May";
	case MON_6:
		вернуть "June";
	case MON_7:
		вернуть "July";
	case MON_8:
		вернуть "August";
	case MON_9:
		вернуть "September";
	case MON_10:
		вернуть "October";
	case MON_11:
		вернуть "November";
	case MON_12:
		вернуть "December";
	case ABMON_1:
		вернуть "Jan";
	case ABMON_2:
		вернуть "Feb";
	case ABMON_3:
		вернуть "Mar";
	case ABMON_4:
		вернуть "Apr";
	case ABMON_5:
		вернуть "May";
	case ABMON_6:
		вернуть "Jun";
	case ABMON_7:
		вернуть "Jul";
	case ABMON_8:
		вернуть "Aug";
	case ABMON_9:
		вернуть "Sep";
	case ABMON_10:
		вернуть "Oct";
	case ABMON_11:
		вернуть "Nov";
	case ABMON_12:
		вернуть "Dec";
	case ALT_DIGITS:
		вернуть "\0\0\0\0\0\0\0\0\0\0";
	case CRNCYSTR:
		вернуть "-";
	case YESEXPR:
		вернуть "^[yY]";
	case NOEXPR:
		вернуть "^[nN]";
	default:
		вернуть "";
	}
}

