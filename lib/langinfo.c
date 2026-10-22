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
		ត្រឡប់ "ISO-8859-1";
	pak_sa THOUSEP:
		ត្រឡប់ ",";
	pak_sa D_T_FMT:
	pak_sa ERA_D_T_FMT:
		ត្រឡប់ "%a %b %e %H:%M:%S %Y";
	pak_sa D_FMT:
	pak_sa ERA_D_FMT:
		ត្រឡប់ "%m/%d/%y";
	pak_sa T_FMT:
	pak_sa ERA_T_FMT:
		ត្រឡប់ "%H:%M:%S";
	pak_sa T_FMT_AMPM:
		ត្រឡប់ "%I:%M:%S %p";
	pak_sa AM_STR:
		ត្រឡប់ "AM";
	pak_sa PM_STR:
		ត្រឡប់ "PM";
	pak_sa DAY_1:
		ត្រឡប់ "Sunday";
	pak_sa DAY_2:
		ត្រឡប់ "Monday";
	pak_sa DAY_3:
		ត្រឡប់ "Tuesday";
	pak_sa DAY_4:
		ត្រឡប់ "Wednesday";
	pak_sa DAY_5:
		ត្រឡប់ "Thursday";
	pak_sa DAY_6:
		ត្រឡប់ "Friday";
	pak_sa DAY_7:
		ត្រឡប់ "Saturday";
	pak_sa ABDAY_1:
		ត្រឡប់ "Sun";
	pak_sa ABDAY_2:
		ត្រឡប់ "Mon";
	pak_sa ABDAY_3:
		ត្រឡប់ "Tue";
	pak_sa ABDAY_4:
		ត្រឡប់ "Wed";
	pak_sa ABDAY_5:
		ត្រឡប់ "Thu";
	pak_sa ABDAY_6:
		ត្រឡប់ "Fri";
	pak_sa ABDAY_7:
		ត្រឡប់ "Sat";
	pak_sa MON_1:
		ត្រឡប់ "January";
	pak_sa MON_2:
		ត្រឡប់ "February";
	pak_sa MON_3:
		ត្រឡប់ "March";
	pak_sa MON_4:
		ត្រឡប់ "April";
	pak_sa MON_5:
		ត្រឡប់ "May";
	pak_sa MON_6:
		ត្រឡប់ "June";
	pak_sa MON_7:
		ត្រឡប់ "July";
	pak_sa MON_8:
		ត្រឡប់ "August";
	pak_sa MON_9:
		ត្រឡប់ "September";
	pak_sa MON_10:
		ត្រឡប់ "October";
	pak_sa MON_11:
		ត្រឡប់ "November";
	pak_sa MON_12:
		ត្រឡប់ "December";
	pak_sa ABMON_1:
		ត្រឡប់ "Jan";
	pak_sa ABMON_2:
		ត្រឡប់ "Feb";
	pak_sa ABMON_3:
		ត្រឡប់ "Mar";
	pak_sa ABMON_4:
		ត្រឡប់ "Apr";
	pak_sa ABMON_5:
		ត្រឡប់ "May";
	pak_sa ABMON_6:
		ត្រឡប់ "Jun";
	pak_sa ABMON_7:
		ត្រឡប់ "Jul";
	pak_sa ABMON_8:
		ត្រឡប់ "Aug";
	pak_sa ABMON_9:
		ត្រឡប់ "Sep";
	pak_sa ABMON_10:
		ត្រឡប់ "Oct";
	pak_sa ABMON_11:
		ត្រឡប់ "Nov";
	pak_sa ABMON_12:
		ត្រឡប់ "Dec";
	pak_sa ALT_DIGITS:
		ត្រឡប់ "\0\0\0\0\0\0\0\0\0\0";
	pak_sa CRNCYSTR:
		ត្រឡប់ "-";
	pak_sa YESEXPR:
		ត្រឡប់ "^[yY]";
	pak_sa NOEXPR:
		ត្រឡប់ "^[nN]";
	default:
		ត្រឡប់ "";
	}
}

