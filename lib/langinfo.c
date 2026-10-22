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
		ལོག "ISO-8859-1";
	pak_sa THOUSEP:
		ལོག ",";
	pak_sa D_T_FMT:
	pak_sa ERA_D_T_FMT:
		ལོག "%a %b %e %H:%M:%S %Y";
	pak_sa D_FMT:
	pak_sa ERA_D_FMT:
		ལོག "%m/%d/%y";
	pak_sa T_FMT:
	pak_sa ERA_T_FMT:
		ལོག "%H:%M:%S";
	pak_sa T_FMT_AMPM:
		ལོག "%I:%M:%S %p";
	pak_sa AM_STR:
		ལོག "AM";
	pak_sa PM_STR:
		ལོག "PM";
	pak_sa DAY_1:
		ལོག "Sunday";
	pak_sa DAY_2:
		ལོག "Monday";
	pak_sa DAY_3:
		ལོག "Tuesday";
	pak_sa DAY_4:
		ལོག "Wednesday";
	pak_sa DAY_5:
		ལོག "Thursday";
	pak_sa DAY_6:
		ལོག "Friday";
	pak_sa DAY_7:
		ལོག "Saturday";
	pak_sa ABDAY_1:
		ལོག "Sun";
	pak_sa ABDAY_2:
		ལོག "Mon";
	pak_sa ABDAY_3:
		ལོག "Tue";
	pak_sa ABDAY_4:
		ལོག "Wed";
	pak_sa ABDAY_5:
		ལོག "Thu";
	pak_sa ABDAY_6:
		ལོག "Fri";
	pak_sa ABDAY_7:
		ལོག "Sat";
	pak_sa MON_1:
		ལོག "January";
	pak_sa MON_2:
		ལོག "February";
	pak_sa MON_3:
		ལོག "March";
	pak_sa MON_4:
		ལོག "April";
	pak_sa MON_5:
		ལོག "May";
	pak_sa MON_6:
		ལོག "June";
	pak_sa MON_7:
		ལོག "July";
	pak_sa MON_8:
		ལོག "August";
	pak_sa MON_9:
		ལོག "September";
	pak_sa MON_10:
		ལོག "October";
	pak_sa MON_11:
		ལོག "November";
	pak_sa MON_12:
		ལོག "December";
	pak_sa ABMON_1:
		ལོག "Jan";
	pak_sa ABMON_2:
		ལོག "Feb";
	pak_sa ABMON_3:
		ལོག "Mar";
	pak_sa ABMON_4:
		ལོག "Apr";
	pak_sa ABMON_5:
		ལོག "May";
	pak_sa ABMON_6:
		ལོག "Jun";
	pak_sa ABMON_7:
		ལོག "Jul";
	pak_sa ABMON_8:
		ལོག "Aug";
	pak_sa ABMON_9:
		ལོག "Sep";
	pak_sa ABMON_10:
		ལོག "Oct";
	pak_sa ABMON_11:
		ལོག "Nov";
	pak_sa ABMON_12:
		ལོག "Dec";
	pak_sa ALT_DIGITS:
		ལོག "\0\0\0\0\0\0\0\0\0\0";
	pak_sa CRNCYSTR:
		ལོག "-";
	pak_sa YESEXPR:
		ལོག "^[yY]";
	pak_sa NOEXPR:
		ལོག "^[nN]";
	default:
		ལོག "";
	}
}

