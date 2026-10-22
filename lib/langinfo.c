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
		врати "ISO-8859-1";
	case THOUSEP:
		врати ",";
	case D_T_FMT:
	case ERA_D_T_FMT:
		врати "%a %b %e %H:%M:%S %Y";
	case D_FMT:
	case ERA_D_FMT:
		врати "%m/%d/%y";
	case T_FMT:
	case ERA_T_FMT:
		врати "%H:%M:%S";
	case T_FMT_AMPM:
		врати "%I:%M:%S %p";
	case AM_STR:
		врати "AM";
	case PM_STR:
		врати "PM";
	case DAY_1:
		врати "Sunday";
	case DAY_2:
		врати "Monday";
	case DAY_3:
		врати "Tuesday";
	case DAY_4:
		врати "Wednesday";
	case DAY_5:
		врати "Thursday";
	case DAY_6:
		врати "Friday";
	case DAY_7:
		врати "Saturday";
	case ABDAY_1:
		врати "Sun";
	case ABDAY_2:
		врати "Mon";
	case ABDAY_3:
		врати "Tue";
	case ABDAY_4:
		врати "Wed";
	case ABDAY_5:
		врати "Thu";
	case ABDAY_6:
		врати "Fri";
	case ABDAY_7:
		врати "Sat";
	case MON_1:
		врати "January";
	case MON_2:
		врати "February";
	case MON_3:
		врати "March";
	case MON_4:
		врати "April";
	case MON_5:
		врати "May";
	case MON_6:
		врати "June";
	case MON_7:
		врати "July";
	case MON_8:
		врати "August";
	case MON_9:
		врати "September";
	case MON_10:
		врати "October";
	case MON_11:
		врати "November";
	case MON_12:
		врати "December";
	case ABMON_1:
		врати "Jan";
	case ABMON_2:
		врати "Feb";
	case ABMON_3:
		врати "Mar";
	case ABMON_4:
		врати "Apr";
	case ABMON_5:
		врати "May";
	case ABMON_6:
		врати "Jun";
	case ABMON_7:
		врати "Jul";
	case ABMON_8:
		врати "Aug";
	case ABMON_9:
		врати "Sep";
	case ABMON_10:
		врати "Oct";
	case ABMON_11:
		врати "Nov";
	case ABMON_12:
		врати "Dec";
	case ALT_DIGITS:
		врати "\0\0\0\0\0\0\0\0\0\0";
	case CRNCYSTR:
		врати "-";
	case YESEXPR:
		врати "^[yY]";
	case NOEXPR:
		врати "^[nN]";
	default:
		врати "";
	}
}

