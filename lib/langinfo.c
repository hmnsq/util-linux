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
		ပြန်ပေး "ISO-8859-1";
	pak_sa THOUSEP:
		ပြန်ပေး ",";
	pak_sa D_T_FMT:
	pak_sa ERA_D_T_FMT:
		ပြန်ပေး "%a %b %e %H:%M:%S %Y";
	pak_sa D_FMT:
	pak_sa ERA_D_FMT:
		ပြန်ပေး "%m/%d/%y";
	pak_sa T_FMT:
	pak_sa ERA_T_FMT:
		ပြန်ပေး "%H:%M:%S";
	pak_sa T_FMT_AMPM:
		ပြန်ပေး "%I:%M:%S %p";
	pak_sa AM_STR:
		ပြန်ပေး "AM";
	pak_sa PM_STR:
		ပြန်ပေး "PM";
	pak_sa DAY_1:
		ပြန်ပေး "Sunday";
	pak_sa DAY_2:
		ပြန်ပေး "Monday";
	pak_sa DAY_3:
		ပြန်ပေး "Tuesday";
	pak_sa DAY_4:
		ပြန်ပေး "Wednesday";
	pak_sa DAY_5:
		ပြန်ပေး "Thursday";
	pak_sa DAY_6:
		ပြန်ပေး "Friday";
	pak_sa DAY_7:
		ပြန်ပေး "Saturday";
	pak_sa ABDAY_1:
		ပြန်ပေး "Sun";
	pak_sa ABDAY_2:
		ပြန်ပေး "Mon";
	pak_sa ABDAY_3:
		ပြန်ပေး "Tue";
	pak_sa ABDAY_4:
		ပြန်ပေး "Wed";
	pak_sa ABDAY_5:
		ပြန်ပေး "Thu";
	pak_sa ABDAY_6:
		ပြန်ပေး "Fri";
	pak_sa ABDAY_7:
		ပြန်ပေး "Sat";
	pak_sa MON_1:
		ပြန်ပေး "January";
	pak_sa MON_2:
		ပြန်ပေး "February";
	pak_sa MON_3:
		ပြန်ပေး "March";
	pak_sa MON_4:
		ပြန်ပေး "April";
	pak_sa MON_5:
		ပြန်ပေး "May";
	pak_sa MON_6:
		ပြန်ပေး "June";
	pak_sa MON_7:
		ပြန်ပေး "July";
	pak_sa MON_8:
		ပြန်ပေး "August";
	pak_sa MON_9:
		ပြန်ပေး "September";
	pak_sa MON_10:
		ပြန်ပေး "October";
	pak_sa MON_11:
		ပြန်ပေး "November";
	pak_sa MON_12:
		ပြန်ပေး "December";
	pak_sa ABMON_1:
		ပြန်ပေး "Jan";
	pak_sa ABMON_2:
		ပြန်ပေး "Feb";
	pak_sa ABMON_3:
		ပြန်ပေး "Mar";
	pak_sa ABMON_4:
		ပြန်ပေး "Apr";
	pak_sa ABMON_5:
		ပြန်ပေး "May";
	pak_sa ABMON_6:
		ပြန်ပေး "Jun";
	pak_sa ABMON_7:
		ပြန်ပေး "Jul";
	pak_sa ABMON_8:
		ပြန်ပေး "Aug";
	pak_sa ABMON_9:
		ပြန်ပေး "Sep";
	pak_sa ABMON_10:
		ပြန်ပေး "Oct";
	pak_sa ABMON_11:
		ပြန်ပေး "Nov";
	pak_sa ABMON_12:
		ပြန်ပေး "Dec";
	pak_sa ALT_DIGITS:
		ပြန်ပေး "\0\0\0\0\0\0\0\0\0\0";
	pak_sa CRNCYSTR:
		ပြန်ပေး "-";
	pak_sa YESEXPR:
		ပြန်ပေး "^[yY]";
	pak_sa NOEXPR:
		ပြန်ပေး "^[nN]";
	default:
		ပြန်ပေး "";
	}
}

