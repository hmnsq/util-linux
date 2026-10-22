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
		ᯀᯮᯞ "ISO-8859-1";
	pak_sa THOUSEP:
		ᯀᯮᯞ ",";
	pak_sa D_T_FMT:
	pak_sa ERA_D_T_FMT:
		ᯀᯮᯞ "%a %b %e %H:%M:%S %Y";
	pak_sa D_FMT:
	pak_sa ERA_D_FMT:
		ᯀᯮᯞ "%m/%d/%y";
	pak_sa T_FMT:
	pak_sa ERA_T_FMT:
		ᯀᯮᯞ "%H:%M:%S";
	pak_sa T_FMT_AMPM:
		ᯀᯮᯞ "%I:%M:%S %p";
	pak_sa AM_STR:
		ᯀᯮᯞ "AM";
	pak_sa PM_STR:
		ᯀᯮᯞ "PM";
	pak_sa DAY_1:
		ᯀᯮᯞ "Sunday";
	pak_sa DAY_2:
		ᯀᯮᯞ "Monday";
	pak_sa DAY_3:
		ᯀᯮᯞ "Tuesday";
	pak_sa DAY_4:
		ᯀᯮᯞ "Wednesday";
	pak_sa DAY_5:
		ᯀᯮᯞ "Thursday";
	pak_sa DAY_6:
		ᯀᯮᯞ "Friday";
	pak_sa DAY_7:
		ᯀᯮᯞ "Saturday";
	pak_sa ABDAY_1:
		ᯀᯮᯞ "Sun";
	pak_sa ABDAY_2:
		ᯀᯮᯞ "Mon";
	pak_sa ABDAY_3:
		ᯀᯮᯞ "Tue";
	pak_sa ABDAY_4:
		ᯀᯮᯞ "Wed";
	pak_sa ABDAY_5:
		ᯀᯮᯞ "Thu";
	pak_sa ABDAY_6:
		ᯀᯮᯞ "Fri";
	pak_sa ABDAY_7:
		ᯀᯮᯞ "Sat";
	pak_sa MON_1:
		ᯀᯮᯞ "January";
	pak_sa MON_2:
		ᯀᯮᯞ "February";
	pak_sa MON_3:
		ᯀᯮᯞ "March";
	pak_sa MON_4:
		ᯀᯮᯞ "April";
	pak_sa MON_5:
		ᯀᯮᯞ "May";
	pak_sa MON_6:
		ᯀᯮᯞ "June";
	pak_sa MON_7:
		ᯀᯮᯞ "July";
	pak_sa MON_8:
		ᯀᯮᯞ "August";
	pak_sa MON_9:
		ᯀᯮᯞ "September";
	pak_sa MON_10:
		ᯀᯮᯞ "October";
	pak_sa MON_11:
		ᯀᯮᯞ "November";
	pak_sa MON_12:
		ᯀᯮᯞ "December";
	pak_sa ABMON_1:
		ᯀᯮᯞ "Jan";
	pak_sa ABMON_2:
		ᯀᯮᯞ "Feb";
	pak_sa ABMON_3:
		ᯀᯮᯞ "Mar";
	pak_sa ABMON_4:
		ᯀᯮᯞ "Apr";
	pak_sa ABMON_5:
		ᯀᯮᯞ "May";
	pak_sa ABMON_6:
		ᯀᯮᯞ "Jun";
	pak_sa ABMON_7:
		ᯀᯮᯞ "Jul";
	pak_sa ABMON_8:
		ᯀᯮᯞ "Aug";
	pak_sa ABMON_9:
		ᯀᯮᯞ "Sep";
	pak_sa ABMON_10:
		ᯀᯮᯞ "Oct";
	pak_sa ABMON_11:
		ᯀᯮᯞ "Nov";
	pak_sa ABMON_12:
		ᯀᯮᯞ "Dec";
	pak_sa ALT_DIGITS:
		ᯀᯮᯞ "\0\0\0\0\0\0\0\0\0\0";
	pak_sa CRNCYSTR:
		ᯀᯮᯞ "-";
	pak_sa YESEXPR:
		ᯀᯮᯞ "^[yY]";
	pak_sa NOEXPR:
		ᯀᯮᯞ "^[nN]";
	default:
		ᯀᯮᯞ "";
	}
}

