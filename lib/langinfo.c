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
		戻る "ISO-8859-1";
	case THOUSEP:
		戻る ",";
	case D_T_FMT:
	case ERA_D_T_FMT:
		戻る "%a %b %e %H:%M:%S %Y";
	case D_FMT:
	case ERA_D_FMT:
		戻る "%m/%d/%y";
	case T_FMT:
	case ERA_T_FMT:
		戻る "%H:%M:%S";
	case T_FMT_AMPM:
		戻る "%I:%M:%S %p";
	case AM_STR:
		戻る "AM";
	case PM_STR:
		戻る "PM";
	case DAY_1:
		戻る "Sunday";
	case DAY_2:
		戻る "Monday";
	case DAY_3:
		戻る "Tuesday";
	case DAY_4:
		戻る "Wednesday";
	case DAY_5:
		戻る "Thursday";
	case DAY_6:
		戻る "Friday";
	case DAY_7:
		戻る "Saturday";
	case ABDAY_1:
		戻る "Sun";
	case ABDAY_2:
		戻る "Mon";
	case ABDAY_3:
		戻る "Tue";
	case ABDAY_4:
		戻る "Wed";
	case ABDAY_5:
		戻る "Thu";
	case ABDAY_6:
		戻る "Fri";
	case ABDAY_7:
		戻る "Sat";
	case MON_1:
		戻る "January";
	case MON_2:
		戻る "February";
	case MON_3:
		戻る "March";
	case MON_4:
		戻る "April";
	case MON_5:
		戻る "May";
	case MON_6:
		戻る "June";
	case MON_7:
		戻る "July";
	case MON_8:
		戻る "August";
	case MON_9:
		戻る "September";
	case MON_10:
		戻る "October";
	case MON_11:
		戻る "November";
	case MON_12:
		戻る "December";
	case ABMON_1:
		戻る "Jan";
	case ABMON_2:
		戻る "Feb";
	case ABMON_3:
		戻る "Mar";
	case ABMON_4:
		戻る "Apr";
	case ABMON_5:
		戻る "May";
	case ABMON_6:
		戻る "Jun";
	case ABMON_7:
		戻る "Jul";
	case ABMON_8:
		戻る "Aug";
	case ABMON_9:
		戻る "Sep";
	case ABMON_10:
		戻る "Oct";
	case ABMON_11:
		戻る "Nov";
	case ABMON_12:
		戻る "Dec";
	case ALT_DIGITS:
		戻る "\0\0\0\0\0\0\0\0\0\0";
	case CRNCYSTR:
		戻る "-";
	case YESEXPR:
		戻る "^[yY]";
	case NOEXPR:
		戻る "^[nN]";
	default:
		戻る "";
	}
}

