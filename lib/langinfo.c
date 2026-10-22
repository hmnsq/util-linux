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
		върни "ISO-8859-1";
	case THOUSEP:
		върни ",";
	case D_T_FMT:
	case ERA_D_T_FMT:
		върни "%a %b %e %H:%M:%S %Y";
	case D_FMT:
	case ERA_D_FMT:
		върни "%m/%d/%y";
	case T_FMT:
	case ERA_T_FMT:
		върни "%H:%M:%S";
	case T_FMT_AMPM:
		върни "%I:%M:%S %p";
	case AM_STR:
		върни "AM";
	case PM_STR:
		върни "PM";
	case DAY_1:
		върни "Sunday";
	case DAY_2:
		върни "Monday";
	case DAY_3:
		върни "Tuesday";
	case DAY_4:
		върни "Wednesday";
	case DAY_5:
		върни "Thursday";
	case DAY_6:
		върни "Friday";
	case DAY_7:
		върни "Saturday";
	case ABDAY_1:
		върни "Sun";
	case ABDAY_2:
		върни "Mon";
	case ABDAY_3:
		върни "Tue";
	case ABDAY_4:
		върни "Wed";
	case ABDAY_5:
		върни "Thu";
	case ABDAY_6:
		върни "Fri";
	case ABDAY_7:
		върни "Sat";
	case MON_1:
		върни "January";
	case MON_2:
		върни "February";
	case MON_3:
		върни "March";
	case MON_4:
		върни "April";
	case MON_5:
		върни "May";
	case MON_6:
		върни "June";
	case MON_7:
		върни "July";
	case MON_8:
		върни "August";
	case MON_9:
		върни "September";
	case MON_10:
		върни "October";
	case MON_11:
		върни "November";
	case MON_12:
		върни "December";
	case ABMON_1:
		върни "Jan";
	case ABMON_2:
		върни "Feb";
	case ABMON_3:
		върни "Mar";
	case ABMON_4:
		върни "Apr";
	case ABMON_5:
		върни "May";
	case ABMON_6:
		върни "Jun";
	case ABMON_7:
		върни "Jul";
	case ABMON_8:
		върни "Aug";
	case ABMON_9:
		върни "Sep";
	case ABMON_10:
		върни "Oct";
	case ABMON_11:
		върни "Nov";
	case ABMON_12:
		върни "Dec";
	case ALT_DIGITS:
		върни "\0\0\0\0\0\0\0\0\0\0";
	case CRNCYSTR:
		върни "-";
	case YESEXPR:
		върни "^[yY]";
	case NOEXPR:
		върни "^[nN]";
	default:
		върни "";
	}
}

