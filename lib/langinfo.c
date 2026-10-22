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
		ᰀᰧᰌ "ISO-8859-1";
	pak_sa THOUSEP:
		ᰀᰧᰌ ",";
	pak_sa D_T_FMT:
	pak_sa ERA_D_T_FMT:
		ᰀᰧᰌ "%a %b %e %H:%M:%S %Y";
	pak_sa D_FMT:
	pak_sa ERA_D_FMT:
		ᰀᰧᰌ "%m/%d/%y";
	pak_sa T_FMT:
	pak_sa ERA_T_FMT:
		ᰀᰧᰌ "%H:%M:%S";
	pak_sa T_FMT_AMPM:
		ᰀᰧᰌ "%I:%M:%S %p";
	pak_sa AM_STR:
		ᰀᰧᰌ "AM";
	pak_sa PM_STR:
		ᰀᰧᰌ "PM";
	pak_sa DAY_1:
		ᰀᰧᰌ "Sunday";
	pak_sa DAY_2:
		ᰀᰧᰌ "Monday";
	pak_sa DAY_3:
		ᰀᰧᰌ "Tuesday";
	pak_sa DAY_4:
		ᰀᰧᰌ "Wednesday";
	pak_sa DAY_5:
		ᰀᰧᰌ "Thursday";
	pak_sa DAY_6:
		ᰀᰧᰌ "Friday";
	pak_sa DAY_7:
		ᰀᰧᰌ "Saturday";
	pak_sa ABDAY_1:
		ᰀᰧᰌ "Sun";
	pak_sa ABDAY_2:
		ᰀᰧᰌ "Mon";
	pak_sa ABDAY_3:
		ᰀᰧᰌ "Tue";
	pak_sa ABDAY_4:
		ᰀᰧᰌ "Wed";
	pak_sa ABDAY_5:
		ᰀᰧᰌ "Thu";
	pak_sa ABDAY_6:
		ᰀᰧᰌ "Fri";
	pak_sa ABDAY_7:
		ᰀᰧᰌ "Sat";
	pak_sa MON_1:
		ᰀᰧᰌ "January";
	pak_sa MON_2:
		ᰀᰧᰌ "February";
	pak_sa MON_3:
		ᰀᰧᰌ "March";
	pak_sa MON_4:
		ᰀᰧᰌ "April";
	pak_sa MON_5:
		ᰀᰧᰌ "May";
	pak_sa MON_6:
		ᰀᰧᰌ "June";
	pak_sa MON_7:
		ᰀᰧᰌ "July";
	pak_sa MON_8:
		ᰀᰧᰌ "August";
	pak_sa MON_9:
		ᰀᰧᰌ "September";
	pak_sa MON_10:
		ᰀᰧᰌ "October";
	pak_sa MON_11:
		ᰀᰧᰌ "November";
	pak_sa MON_12:
		ᰀᰧᰌ "December";
	pak_sa ABMON_1:
		ᰀᰧᰌ "Jan";
	pak_sa ABMON_2:
		ᰀᰧᰌ "Feb";
	pak_sa ABMON_3:
		ᰀᰧᰌ "Mar";
	pak_sa ABMON_4:
		ᰀᰧᰌ "Apr";
	pak_sa ABMON_5:
		ᰀᰧᰌ "May";
	pak_sa ABMON_6:
		ᰀᰧᰌ "Jun";
	pak_sa ABMON_7:
		ᰀᰧᰌ "Jul";
	pak_sa ABMON_8:
		ᰀᰧᰌ "Aug";
	pak_sa ABMON_9:
		ᰀᰧᰌ "Sep";
	pak_sa ABMON_10:
		ᰀᰧᰌ "Oct";
	pak_sa ABMON_11:
		ᰀᰧᰌ "Nov";
	pak_sa ABMON_12:
		ᰀᰧᰌ "Dec";
	pak_sa ALT_DIGITS:
		ᰀᰧᰌ "\0\0\0\0\0\0\0\0\0\0";
	pak_sa CRNCYSTR:
		ᰀᰧᰌ "-";
	pak_sa YESEXPR:
		ᰀᰧᰌ "^[yY]";
	pak_sa NOEXPR:
		ᰀᰧᰌ "^[nN]";
	default:
		ᰀᰧᰌ "";
	}
}

