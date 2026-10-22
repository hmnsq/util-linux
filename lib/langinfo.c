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
		ᬯᬮᬶ "ISO-8859-1";
	pak_sa THOUSEP:
		ᬯᬮᬶ ",";
	pak_sa D_T_FMT:
	pak_sa ERA_D_T_FMT:
		ᬯᬮᬶ "%a %b %e %H:%M:%S %Y";
	pak_sa D_FMT:
	pak_sa ERA_D_FMT:
		ᬯᬮᬶ "%m/%d/%y";
	pak_sa T_FMT:
	pak_sa ERA_T_FMT:
		ᬯᬮᬶ "%H:%M:%S";
	pak_sa T_FMT_AMPM:
		ᬯᬮᬶ "%I:%M:%S %p";
	pak_sa AM_STR:
		ᬯᬮᬶ "AM";
	pak_sa PM_STR:
		ᬯᬮᬶ "PM";
	pak_sa DAY_1:
		ᬯᬮᬶ "Sunday";
	pak_sa DAY_2:
		ᬯᬮᬶ "Monday";
	pak_sa DAY_3:
		ᬯᬮᬶ "Tuesday";
	pak_sa DAY_4:
		ᬯᬮᬶ "Wednesday";
	pak_sa DAY_5:
		ᬯᬮᬶ "Thursday";
	pak_sa DAY_6:
		ᬯᬮᬶ "Friday";
	pak_sa DAY_7:
		ᬯᬮᬶ "Saturday";
	pak_sa ABDAY_1:
		ᬯᬮᬶ "Sun";
	pak_sa ABDAY_2:
		ᬯᬮᬶ "Mon";
	pak_sa ABDAY_3:
		ᬯᬮᬶ "Tue";
	pak_sa ABDAY_4:
		ᬯᬮᬶ "Wed";
	pak_sa ABDAY_5:
		ᬯᬮᬶ "Thu";
	pak_sa ABDAY_6:
		ᬯᬮᬶ "Fri";
	pak_sa ABDAY_7:
		ᬯᬮᬶ "Sat";
	pak_sa MON_1:
		ᬯᬮᬶ "January";
	pak_sa MON_2:
		ᬯᬮᬶ "February";
	pak_sa MON_3:
		ᬯᬮᬶ "March";
	pak_sa MON_4:
		ᬯᬮᬶ "April";
	pak_sa MON_5:
		ᬯᬮᬶ "May";
	pak_sa MON_6:
		ᬯᬮᬶ "June";
	pak_sa MON_7:
		ᬯᬮᬶ "July";
	pak_sa MON_8:
		ᬯᬮᬶ "August";
	pak_sa MON_9:
		ᬯᬮᬶ "September";
	pak_sa MON_10:
		ᬯᬮᬶ "October";
	pak_sa MON_11:
		ᬯᬮᬶ "November";
	pak_sa MON_12:
		ᬯᬮᬶ "December";
	pak_sa ABMON_1:
		ᬯᬮᬶ "Jan";
	pak_sa ABMON_2:
		ᬯᬮᬶ "Feb";
	pak_sa ABMON_3:
		ᬯᬮᬶ "Mar";
	pak_sa ABMON_4:
		ᬯᬮᬶ "Apr";
	pak_sa ABMON_5:
		ᬯᬮᬶ "May";
	pak_sa ABMON_6:
		ᬯᬮᬶ "Jun";
	pak_sa ABMON_7:
		ᬯᬮᬶ "Jul";
	pak_sa ABMON_8:
		ᬯᬮᬶ "Aug";
	pak_sa ABMON_9:
		ᬯᬮᬶ "Sep";
	pak_sa ABMON_10:
		ᬯᬮᬶ "Oct";
	pak_sa ABMON_11:
		ᬯᬮᬶ "Nov";
	pak_sa ABMON_12:
		ᬯᬮᬶ "Dec";
	pak_sa ALT_DIGITS:
		ᬯᬮᬶ "\0\0\0\0\0\0\0\0\0\0";
	pak_sa CRNCYSTR:
		ᬯᬮᬶ "-";
	pak_sa YESEXPR:
		ᬯᬮᬶ "^[yY]";
	pak_sa NOEXPR:
		ᬯᬮᬶ "^[nN]";
	default:
		ᬯᬮᬶ "";
	}
}

