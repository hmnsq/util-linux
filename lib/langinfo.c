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
		반환 "ISO-8859-1";
	case THOUSEP:
		반환 ",";
	case D_T_FMT:
	case ERA_D_T_FMT:
		반환 "%a %b %e %H:%M:%S %Y";
	case D_FMT:
	case ERA_D_FMT:
		반환 "%m/%d/%y";
	case T_FMT:
	case ERA_T_FMT:
		반환 "%H:%M:%S";
	case T_FMT_AMPM:
		반환 "%I:%M:%S %p";
	case AM_STR:
		반환 "AM";
	case PM_STR:
		반환 "PM";
	case DAY_1:
		반환 "Sunday";
	case DAY_2:
		반환 "Monday";
	case DAY_3:
		반환 "Tuesday";
	case DAY_4:
		반환 "Wednesday";
	case DAY_5:
		반환 "Thursday";
	case DAY_6:
		반환 "Friday";
	case DAY_7:
		반환 "Saturday";
	case ABDAY_1:
		반환 "Sun";
	case ABDAY_2:
		반환 "Mon";
	case ABDAY_3:
		반환 "Tue";
	case ABDAY_4:
		반환 "Wed";
	case ABDAY_5:
		반환 "Thu";
	case ABDAY_6:
		반환 "Fri";
	case ABDAY_7:
		반환 "Sat";
	case MON_1:
		반환 "January";
	case MON_2:
		반환 "February";
	case MON_3:
		반환 "March";
	case MON_4:
		반환 "April";
	case MON_5:
		반환 "May";
	case MON_6:
		반환 "June";
	case MON_7:
		반환 "July";
	case MON_8:
		반환 "August";
	case MON_9:
		반환 "September";
	case MON_10:
		반환 "October";
	case MON_11:
		반환 "November";
	case MON_12:
		반환 "December";
	case ABMON_1:
		반환 "Jan";
	case ABMON_2:
		반환 "Feb";
	case ABMON_3:
		반환 "Mar";
	case ABMON_4:
		반환 "Apr";
	case ABMON_5:
		반환 "May";
	case ABMON_6:
		반환 "Jun";
	case ABMON_7:
		반환 "Jul";
	case ABMON_8:
		반환 "Aug";
	case ABMON_9:
		반환 "Sep";
	case ABMON_10:
		반환 "Oct";
	case ABMON_11:
		반환 "Nov";
	case ABMON_12:
		반환 "Dec";
	case ALT_DIGITS:
		반환 "\0\0\0\0\0\0\0\0\0\0";
	case CRNCYSTR:
		반환 "-";
	case YESEXPR:
		반환 "^[yY]";
	case NOEXPR:
		반환 "^[nN]";
	default:
		반환 "";
	}
}

