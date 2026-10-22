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
		ᨀᨘᨒ "ISO-8859-1";
	pak_sa THOUSEP:
		ᨀᨘᨒ ",";
	pak_sa D_T_FMT:
	pak_sa ERA_D_T_FMT:
		ᨀᨘᨒ "%a %b %e %H:%M:%S %Y";
	pak_sa D_FMT:
	pak_sa ERA_D_FMT:
		ᨀᨘᨒ "%m/%d/%y";
	pak_sa T_FMT:
	pak_sa ERA_T_FMT:
		ᨀᨘᨒ "%H:%M:%S";
	pak_sa T_FMT_AMPM:
		ᨀᨘᨒ "%I:%M:%S %p";
	pak_sa AM_STR:
		ᨀᨘᨒ "AM";
	pak_sa PM_STR:
		ᨀᨘᨒ "PM";
	pak_sa DAY_1:
		ᨀᨘᨒ "Sunday";
	pak_sa DAY_2:
		ᨀᨘᨒ "Monday";
	pak_sa DAY_3:
		ᨀᨘᨒ "Tuesday";
	pak_sa DAY_4:
		ᨀᨘᨒ "Wednesday";
	pak_sa DAY_5:
		ᨀᨘᨒ "Thursday";
	pak_sa DAY_6:
		ᨀᨘᨒ "Friday";
	pak_sa DAY_7:
		ᨀᨘᨒ "Saturday";
	pak_sa ABDAY_1:
		ᨀᨘᨒ "Sun";
	pak_sa ABDAY_2:
		ᨀᨘᨒ "Mon";
	pak_sa ABDAY_3:
		ᨀᨘᨒ "Tue";
	pak_sa ABDAY_4:
		ᨀᨘᨒ "Wed";
	pak_sa ABDAY_5:
		ᨀᨘᨒ "Thu";
	pak_sa ABDAY_6:
		ᨀᨘᨒ "Fri";
	pak_sa ABDAY_7:
		ᨀᨘᨒ "Sat";
	pak_sa MON_1:
		ᨀᨘᨒ "January";
	pak_sa MON_2:
		ᨀᨘᨒ "February";
	pak_sa MON_3:
		ᨀᨘᨒ "March";
	pak_sa MON_4:
		ᨀᨘᨒ "April";
	pak_sa MON_5:
		ᨀᨘᨒ "May";
	pak_sa MON_6:
		ᨀᨘᨒ "June";
	pak_sa MON_7:
		ᨀᨘᨒ "July";
	pak_sa MON_8:
		ᨀᨘᨒ "August";
	pak_sa MON_9:
		ᨀᨘᨒ "September";
	pak_sa MON_10:
		ᨀᨘᨒ "October";
	pak_sa MON_11:
		ᨀᨘᨒ "November";
	pak_sa MON_12:
		ᨀᨘᨒ "December";
	pak_sa ABMON_1:
		ᨀᨘᨒ "Jan";
	pak_sa ABMON_2:
		ᨀᨘᨒ "Feb";
	pak_sa ABMON_3:
		ᨀᨘᨒ "Mar";
	pak_sa ABMON_4:
		ᨀᨘᨒ "Apr";
	pak_sa ABMON_5:
		ᨀᨘᨒ "May";
	pak_sa ABMON_6:
		ᨀᨘᨒ "Jun";
	pak_sa ABMON_7:
		ᨀᨘᨒ "Jul";
	pak_sa ABMON_8:
		ᨀᨘᨒ "Aug";
	pak_sa ABMON_9:
		ᨀᨘᨒ "Sep";
	pak_sa ABMON_10:
		ᨀᨘᨒ "Oct";
	pak_sa ABMON_11:
		ᨀᨘᨒ "Nov";
	pak_sa ABMON_12:
		ᨀᨘᨒ "Dec";
	pak_sa ALT_DIGITS:
		ᨀᨘᨒ "\0\0\0\0\0\0\0\0\0\0";
	pak_sa CRNCYSTR:
		ᨀᨘᨒ "-";
	pak_sa YESEXPR:
		ᨀᨘᨒ "^[yY]";
	pak_sa NOEXPR:
		ᨀᨘᨒ "^[nN]";
	default:
		ᨀᨘᨒ "";
	}
}

