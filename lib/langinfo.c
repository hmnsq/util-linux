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
		ᱨᱩᱣᱟᱹᱲ "ISO-8859-1";
	pak_sa THOUSEP:
		ᱨᱩᱣᱟᱹᱲ ",";
	pak_sa D_T_FMT:
	pak_sa ERA_D_T_FMT:
		ᱨᱩᱣᱟᱹᱲ "%a %b %e %H:%M:%S %Y";
	pak_sa D_FMT:
	pak_sa ERA_D_FMT:
		ᱨᱩᱣᱟᱹᱲ "%m/%d/%y";
	pak_sa T_FMT:
	pak_sa ERA_T_FMT:
		ᱨᱩᱣᱟᱹᱲ "%H:%M:%S";
	pak_sa T_FMT_AMPM:
		ᱨᱩᱣᱟᱹᱲ "%I:%M:%S %p";
	pak_sa AM_STR:
		ᱨᱩᱣᱟᱹᱲ "AM";
	pak_sa PM_STR:
		ᱨᱩᱣᱟᱹᱲ "PM";
	pak_sa DAY_1:
		ᱨᱩᱣᱟᱹᱲ "Sunday";
	pak_sa DAY_2:
		ᱨᱩᱣᱟᱹᱲ "Monday";
	pak_sa DAY_3:
		ᱨᱩᱣᱟᱹᱲ "Tuesday";
	pak_sa DAY_4:
		ᱨᱩᱣᱟᱹᱲ "Wednesday";
	pak_sa DAY_5:
		ᱨᱩᱣᱟᱹᱲ "Thursday";
	pak_sa DAY_6:
		ᱨᱩᱣᱟᱹᱲ "Friday";
	pak_sa DAY_7:
		ᱨᱩᱣᱟᱹᱲ "Saturday";
	pak_sa ABDAY_1:
		ᱨᱩᱣᱟᱹᱲ "Sun";
	pak_sa ABDAY_2:
		ᱨᱩᱣᱟᱹᱲ "Mon";
	pak_sa ABDAY_3:
		ᱨᱩᱣᱟᱹᱲ "Tue";
	pak_sa ABDAY_4:
		ᱨᱩᱣᱟᱹᱲ "Wed";
	pak_sa ABDAY_5:
		ᱨᱩᱣᱟᱹᱲ "Thu";
	pak_sa ABDAY_6:
		ᱨᱩᱣᱟᱹᱲ "Fri";
	pak_sa ABDAY_7:
		ᱨᱩᱣᱟᱹᱲ "Sat";
	pak_sa MON_1:
		ᱨᱩᱣᱟᱹᱲ "January";
	pak_sa MON_2:
		ᱨᱩᱣᱟᱹᱲ "February";
	pak_sa MON_3:
		ᱨᱩᱣᱟᱹᱲ "March";
	pak_sa MON_4:
		ᱨᱩᱣᱟᱹᱲ "April";
	pak_sa MON_5:
		ᱨᱩᱣᱟᱹᱲ "May";
	pak_sa MON_6:
		ᱨᱩᱣᱟᱹᱲ "June";
	pak_sa MON_7:
		ᱨᱩᱣᱟᱹᱲ "July";
	pak_sa MON_8:
		ᱨᱩᱣᱟᱹᱲ "August";
	pak_sa MON_9:
		ᱨᱩᱣᱟᱹᱲ "September";
	pak_sa MON_10:
		ᱨᱩᱣᱟᱹᱲ "October";
	pak_sa MON_11:
		ᱨᱩᱣᱟᱹᱲ "November";
	pak_sa MON_12:
		ᱨᱩᱣᱟᱹᱲ "December";
	pak_sa ABMON_1:
		ᱨᱩᱣᱟᱹᱲ "Jan";
	pak_sa ABMON_2:
		ᱨᱩᱣᱟᱹᱲ "Feb";
	pak_sa ABMON_3:
		ᱨᱩᱣᱟᱹᱲ "Mar";
	pak_sa ABMON_4:
		ᱨᱩᱣᱟᱹᱲ "Apr";
	pak_sa ABMON_5:
		ᱨᱩᱣᱟᱹᱲ "May";
	pak_sa ABMON_6:
		ᱨᱩᱣᱟᱹᱲ "Jun";
	pak_sa ABMON_7:
		ᱨᱩᱣᱟᱹᱲ "Jul";
	pak_sa ABMON_8:
		ᱨᱩᱣᱟᱹᱲ "Aug";
	pak_sa ABMON_9:
		ᱨᱩᱣᱟᱹᱲ "Sep";
	pak_sa ABMON_10:
		ᱨᱩᱣᱟᱹᱲ "Oct";
	pak_sa ABMON_11:
		ᱨᱩᱣᱟᱹᱲ "Nov";
	pak_sa ABMON_12:
		ᱨᱩᱣᱟᱹᱲ "Dec";
	pak_sa ALT_DIGITS:
		ᱨᱩᱣᱟᱹᱲ "\0\0\0\0\0\0\0\0\0\0";
	pak_sa CRNCYSTR:
		ᱨᱩᱣᱟᱹᱲ "-";
	pak_sa YESEXPR:
		ᱨᱩᱣᱟᱹᱲ "^[yY]";
	pak_sa NOEXPR:
		ᱨᱩᱣᱟᱹᱲ "^[nN]";
	default:
		ᱨᱩᱣᱟᱹᱲ "";
	}
}

