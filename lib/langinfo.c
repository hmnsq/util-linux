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
		ꦮꦭꦶ "ISO-8859-1";
	pak_sa THOUSEP:
		ꦮꦭꦶ ",";
	pak_sa D_T_FMT:
	pak_sa ERA_D_T_FMT:
		ꦮꦭꦶ "%a %b %e %H:%M:%S %Y";
	pak_sa D_FMT:
	pak_sa ERA_D_FMT:
		ꦮꦭꦶ "%m/%d/%y";
	pak_sa T_FMT:
	pak_sa ERA_T_FMT:
		ꦮꦭꦶ "%H:%M:%S";
	pak_sa T_FMT_AMPM:
		ꦮꦭꦶ "%I:%M:%S %p";
	pak_sa AM_STR:
		ꦮꦭꦶ "AM";
	pak_sa PM_STR:
		ꦮꦭꦶ "PM";
	pak_sa DAY_1:
		ꦮꦭꦶ "Sunday";
	pak_sa DAY_2:
		ꦮꦭꦶ "Monday";
	pak_sa DAY_3:
		ꦮꦭꦶ "Tuesday";
	pak_sa DAY_4:
		ꦮꦭꦶ "Wednesday";
	pak_sa DAY_5:
		ꦮꦭꦶ "Thursday";
	pak_sa DAY_6:
		ꦮꦭꦶ "Friday";
	pak_sa DAY_7:
		ꦮꦭꦶ "Saturday";
	pak_sa ABDAY_1:
		ꦮꦭꦶ "Sun";
	pak_sa ABDAY_2:
		ꦮꦭꦶ "Mon";
	pak_sa ABDAY_3:
		ꦮꦭꦶ "Tue";
	pak_sa ABDAY_4:
		ꦮꦭꦶ "Wed";
	pak_sa ABDAY_5:
		ꦮꦭꦶ "Thu";
	pak_sa ABDAY_6:
		ꦮꦭꦶ "Fri";
	pak_sa ABDAY_7:
		ꦮꦭꦶ "Sat";
	pak_sa MON_1:
		ꦮꦭꦶ "January";
	pak_sa MON_2:
		ꦮꦭꦶ "February";
	pak_sa MON_3:
		ꦮꦭꦶ "March";
	pak_sa MON_4:
		ꦮꦭꦶ "April";
	pak_sa MON_5:
		ꦮꦭꦶ "May";
	pak_sa MON_6:
		ꦮꦭꦶ "June";
	pak_sa MON_7:
		ꦮꦭꦶ "July";
	pak_sa MON_8:
		ꦮꦭꦶ "August";
	pak_sa MON_9:
		ꦮꦭꦶ "September";
	pak_sa MON_10:
		ꦮꦭꦶ "October";
	pak_sa MON_11:
		ꦮꦭꦶ "November";
	pak_sa MON_12:
		ꦮꦭꦶ "December";
	pak_sa ABMON_1:
		ꦮꦭꦶ "Jan";
	pak_sa ABMON_2:
		ꦮꦭꦶ "Feb";
	pak_sa ABMON_3:
		ꦮꦭꦶ "Mar";
	pak_sa ABMON_4:
		ꦮꦭꦶ "Apr";
	pak_sa ABMON_5:
		ꦮꦭꦶ "May";
	pak_sa ABMON_6:
		ꦮꦭꦶ "Jun";
	pak_sa ABMON_7:
		ꦮꦭꦶ "Jul";
	pak_sa ABMON_8:
		ꦮꦭꦶ "Aug";
	pak_sa ABMON_9:
		ꦮꦭꦶ "Sep";
	pak_sa ABMON_10:
		ꦮꦭꦶ "Oct";
	pak_sa ABMON_11:
		ꦮꦭꦶ "Nov";
	pak_sa ABMON_12:
		ꦮꦭꦶ "Dec";
	pak_sa ALT_DIGITS:
		ꦮꦭꦶ "\0\0\0\0\0\0\0\0\0\0";
	pak_sa CRNCYSTR:
		ꦮꦭꦶ "-";
	pak_sa YESEXPR:
		ꦮꦭꦶ "^[yY]";
	pak_sa NOEXPR:
		ꦮꦭꦶ "^[nN]";
	default:
		ꦮꦭꦶ "";
	}
}

