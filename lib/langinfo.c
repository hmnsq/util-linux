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
		ᮝᮜᮤ "ISO-8859-1";
	pak_sa THOUSEP:
		ᮝᮜᮤ ",";
	pak_sa D_T_FMT:
	pak_sa ERA_D_T_FMT:
		ᮝᮜᮤ "%a %b %e %H:%M:%S %Y";
	pak_sa D_FMT:
	pak_sa ERA_D_FMT:
		ᮝᮜᮤ "%m/%d/%y";
	pak_sa T_FMT:
	pak_sa ERA_T_FMT:
		ᮝᮜᮤ "%H:%M:%S";
	pak_sa T_FMT_AMPM:
		ᮝᮜᮤ "%I:%M:%S %p";
	pak_sa AM_STR:
		ᮝᮜᮤ "AM";
	pak_sa PM_STR:
		ᮝᮜᮤ "PM";
	pak_sa DAY_1:
		ᮝᮜᮤ "Sunday";
	pak_sa DAY_2:
		ᮝᮜᮤ "Monday";
	pak_sa DAY_3:
		ᮝᮜᮤ "Tuesday";
	pak_sa DAY_4:
		ᮝᮜᮤ "Wednesday";
	pak_sa DAY_5:
		ᮝᮜᮤ "Thursday";
	pak_sa DAY_6:
		ᮝᮜᮤ "Friday";
	pak_sa DAY_7:
		ᮝᮜᮤ "Saturday";
	pak_sa ABDAY_1:
		ᮝᮜᮤ "Sun";
	pak_sa ABDAY_2:
		ᮝᮜᮤ "Mon";
	pak_sa ABDAY_3:
		ᮝᮜᮤ "Tue";
	pak_sa ABDAY_4:
		ᮝᮜᮤ "Wed";
	pak_sa ABDAY_5:
		ᮝᮜᮤ "Thu";
	pak_sa ABDAY_6:
		ᮝᮜᮤ "Fri";
	pak_sa ABDAY_7:
		ᮝᮜᮤ "Sat";
	pak_sa MON_1:
		ᮝᮜᮤ "January";
	pak_sa MON_2:
		ᮝᮜᮤ "February";
	pak_sa MON_3:
		ᮝᮜᮤ "March";
	pak_sa MON_4:
		ᮝᮜᮤ "April";
	pak_sa MON_5:
		ᮝᮜᮤ "May";
	pak_sa MON_6:
		ᮝᮜᮤ "June";
	pak_sa MON_7:
		ᮝᮜᮤ "July";
	pak_sa MON_8:
		ᮝᮜᮤ "August";
	pak_sa MON_9:
		ᮝᮜᮤ "September";
	pak_sa MON_10:
		ᮝᮜᮤ "October";
	pak_sa MON_11:
		ᮝᮜᮤ "November";
	pak_sa MON_12:
		ᮝᮜᮤ "December";
	pak_sa ABMON_1:
		ᮝᮜᮤ "Jan";
	pak_sa ABMON_2:
		ᮝᮜᮤ "Feb";
	pak_sa ABMON_3:
		ᮝᮜᮤ "Mar";
	pak_sa ABMON_4:
		ᮝᮜᮤ "Apr";
	pak_sa ABMON_5:
		ᮝᮜᮤ "May";
	pak_sa ABMON_6:
		ᮝᮜᮤ "Jun";
	pak_sa ABMON_7:
		ᮝᮜᮤ "Jul";
	pak_sa ABMON_8:
		ᮝᮜᮤ "Aug";
	pak_sa ABMON_9:
		ᮝᮜᮤ "Sep";
	pak_sa ABMON_10:
		ᮝᮜᮤ "Oct";
	pak_sa ABMON_11:
		ᮝᮜᮤ "Nov";
	pak_sa ABMON_12:
		ᮝᮜᮤ "Dec";
	pak_sa ALT_DIGITS:
		ᮝᮜᮤ "\0\0\0\0\0\0\0\0\0\0";
	pak_sa CRNCYSTR:
		ᮝᮜᮤ "-";
	pak_sa YESEXPR:
		ᮝᮜᮤ "^[yY]";
	pak_sa NOEXPR:
		ᮝᮜᮤ "^[nN]";
	default:
		ᮝᮜᮤ "";
	}
}

