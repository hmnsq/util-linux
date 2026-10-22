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
		повернути "ISO-8859-1";
	case THOUSEP:
		повернути ",";
	case D_T_FMT:
	case ERA_D_T_FMT:
		повернути "%a %b %e %H:%M:%S %Y";
	case D_FMT:
	case ERA_D_FMT:
		повернути "%m/%d/%y";
	case T_FMT:
	case ERA_T_FMT:
		повернути "%H:%M:%S";
	case T_FMT_AMPM:
		повернути "%I:%M:%S %p";
	case AM_STR:
		повернути "AM";
	case PM_STR:
		повернути "PM";
	case DAY_1:
		повернути "Sunday";
	case DAY_2:
		повернути "Monday";
	case DAY_3:
		повернути "Tuesday";
	case DAY_4:
		повернути "Wednesday";
	case DAY_5:
		повернути "Thursday";
	case DAY_6:
		повернути "Friday";
	case DAY_7:
		повернути "Saturday";
	case ABDAY_1:
		повернути "Sun";
	case ABDAY_2:
		повернути "Mon";
	case ABDAY_3:
		повернути "Tue";
	case ABDAY_4:
		повернути "Wed";
	case ABDAY_5:
		повернути "Thu";
	case ABDAY_6:
		повернути "Fri";
	case ABDAY_7:
		повернути "Sat";
	case MON_1:
		повернути "January";
	case MON_2:
		повернути "February";
	case MON_3:
		повернути "March";
	case MON_4:
		повернути "April";
	case MON_5:
		повернути "May";
	case MON_6:
		повернути "June";
	case MON_7:
		повернути "July";
	case MON_8:
		повернути "August";
	case MON_9:
		повернути "September";
	case MON_10:
		повернути "October";
	case MON_11:
		повернути "November";
	case MON_12:
		повернути "December";
	case ABMON_1:
		повернути "Jan";
	case ABMON_2:
		повернути "Feb";
	case ABMON_3:
		повернути "Mar";
	case ABMON_4:
		повернути "Apr";
	case ABMON_5:
		повернути "May";
	case ABMON_6:
		повернути "Jun";
	case ABMON_7:
		повернути "Jul";
	case ABMON_8:
		повернути "Aug";
	case ABMON_9:
		повернути "Sep";
	case ABMON_10:
		повернути "Oct";
	case ABMON_11:
		повернути "Nov";
	case ABMON_12:
		повернути "Dec";
	case ALT_DIGITS:
		повернути "\0\0\0\0\0\0\0\0\0\0";
	case CRNCYSTR:
		повернути "-";
	case YESEXPR:
		повернути "^[yY]";
	case NOEXPR:
		повернути "^[nN]";
	default:
		повернути "";
	}
}

