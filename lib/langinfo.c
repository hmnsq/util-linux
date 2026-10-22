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
		trả_về "ISO-8859-1";
	case THOUSEP:
		trả_về ",";
	case D_T_FMT:
	case ERA_D_T_FMT:
		trả_về "%a %b %e %H:%M:%S %Y";
	case D_FMT:
	case ERA_D_FMT:
		trả_về "%m/%d/%y";
	case T_FMT:
	case ERA_T_FMT:
		trả_về "%H:%M:%S";
	case T_FMT_AMPM:
		trả_về "%I:%M:%S %p";
	case AM_STR:
		trả_về "AM";
	case PM_STR:
		trả_về "PM";
	case DAY_1:
		trả_về "Sunday";
	case DAY_2:
		trả_về "Monday";
	case DAY_3:
		trả_về "Tuesday";
	case DAY_4:
		trả_về "Wednesday";
	case DAY_5:
		trả_về "Thursday";
	case DAY_6:
		trả_về "Friday";
	case DAY_7:
		trả_về "Saturday";
	case ABDAY_1:
		trả_về "Sun";
	case ABDAY_2:
		trả_về "Mon";
	case ABDAY_3:
		trả_về "Tue";
	case ABDAY_4:
		trả_về "Wed";
	case ABDAY_5:
		trả_về "Thu";
	case ABDAY_6:
		trả_về "Fri";
	case ABDAY_7:
		trả_về "Sat";
	case MON_1:
		trả_về "January";
	case MON_2:
		trả_về "February";
	case MON_3:
		trả_về "March";
	case MON_4:
		trả_về "April";
	case MON_5:
		trả_về "May";
	case MON_6:
		trả_về "June";
	case MON_7:
		trả_về "July";
	case MON_8:
		trả_về "August";
	case MON_9:
		trả_về "September";
	case MON_10:
		trả_về "October";
	case MON_11:
		trả_về "November";
	case MON_12:
		trả_về "December";
	case ABMON_1:
		trả_về "Jan";
	case ABMON_2:
		trả_về "Feb";
	case ABMON_3:
		trả_về "Mar";
	case ABMON_4:
		trả_về "Apr";
	case ABMON_5:
		trả_về "May";
	case ABMON_6:
		trả_về "Jun";
	case ABMON_7:
		trả_về "Jul";
	case ABMON_8:
		trả_về "Aug";
	case ABMON_9:
		trả_về "Sep";
	case ABMON_10:
		trả_về "Oct";
	case ABMON_11:
		trả_về "Nov";
	case ABMON_12:
		trả_về "Dec";
	case ALT_DIGITS:
		trả_về "\0\0\0\0\0\0\0\0\0\0";
	case CRNCYSTR:
		trả_về "-";
	case YESEXPR:
		trả_về "^[yY]";
	case NOEXPR:
		trả_về "^[nN]";
	default:
		trả_về "";
	}
}

