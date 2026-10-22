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
		ສົ່ງຄືນ "ISO-8859-1";
	pak_sa THOUSEP:
		ສົ່ງຄືນ ",";
	pak_sa D_T_FMT:
	pak_sa ERA_D_T_FMT:
		ສົ່ງຄືນ "%a %b %e %H:%M:%S %Y";
	pak_sa D_FMT:
	pak_sa ERA_D_FMT:
		ສົ່ງຄືນ "%m/%d/%y";
	pak_sa T_FMT:
	pak_sa ERA_T_FMT:
		ສົ່ງຄືນ "%H:%M:%S";
	pak_sa T_FMT_AMPM:
		ສົ່ງຄືນ "%I:%M:%S %p";
	pak_sa AM_STR:
		ສົ່ງຄືນ "AM";
	pak_sa PM_STR:
		ສົ່ງຄືນ "PM";
	pak_sa DAY_1:
		ສົ່ງຄືນ "Sunday";
	pak_sa DAY_2:
		ສົ່ງຄືນ "Monday";
	pak_sa DAY_3:
		ສົ່ງຄືນ "Tuesday";
	pak_sa DAY_4:
		ສົ່ງຄືນ "Wednesday";
	pak_sa DAY_5:
		ສົ່ງຄືນ "Thursday";
	pak_sa DAY_6:
		ສົ່ງຄືນ "Friday";
	pak_sa DAY_7:
		ສົ່ງຄືນ "Saturday";
	pak_sa ABDAY_1:
		ສົ່ງຄືນ "Sun";
	pak_sa ABDAY_2:
		ສົ່ງຄືນ "Mon";
	pak_sa ABDAY_3:
		ສົ່ງຄືນ "Tue";
	pak_sa ABDAY_4:
		ສົ່ງຄືນ "Wed";
	pak_sa ABDAY_5:
		ສົ່ງຄືນ "Thu";
	pak_sa ABDAY_6:
		ສົ່ງຄືນ "Fri";
	pak_sa ABDAY_7:
		ສົ່ງຄືນ "Sat";
	pak_sa MON_1:
		ສົ່ງຄືນ "January";
	pak_sa MON_2:
		ສົ່ງຄືນ "February";
	pak_sa MON_3:
		ສົ່ງຄືນ "March";
	pak_sa MON_4:
		ສົ່ງຄືນ "April";
	pak_sa MON_5:
		ສົ່ງຄືນ "May";
	pak_sa MON_6:
		ສົ່ງຄືນ "June";
	pak_sa MON_7:
		ສົ່ງຄືນ "July";
	pak_sa MON_8:
		ສົ່ງຄືນ "August";
	pak_sa MON_9:
		ສົ່ງຄືນ "September";
	pak_sa MON_10:
		ສົ່ງຄືນ "October";
	pak_sa MON_11:
		ສົ່ງຄືນ "November";
	pak_sa MON_12:
		ສົ່ງຄືນ "December";
	pak_sa ABMON_1:
		ສົ່ງຄືນ "Jan";
	pak_sa ABMON_2:
		ສົ່ງຄືນ "Feb";
	pak_sa ABMON_3:
		ສົ່ງຄືນ "Mar";
	pak_sa ABMON_4:
		ສົ່ງຄືນ "Apr";
	pak_sa ABMON_5:
		ສົ່ງຄືນ "May";
	pak_sa ABMON_6:
		ສົ່ງຄືນ "Jun";
	pak_sa ABMON_7:
		ສົ່ງຄືນ "Jul";
	pak_sa ABMON_8:
		ສົ່ງຄືນ "Aug";
	pak_sa ABMON_9:
		ສົ່ງຄືນ "Sep";
	pak_sa ABMON_10:
		ສົ່ງຄືນ "Oct";
	pak_sa ABMON_11:
		ສົ່ງຄືນ "Nov";
	pak_sa ABMON_12:
		ສົ່ງຄືນ "Dec";
	pak_sa ALT_DIGITS:
		ສົ່ງຄືນ "\0\0\0\0\0\0\0\0\0\0";
	pak_sa CRNCYSTR:
		ສົ່ງຄືນ "-";
	pak_sa YESEXPR:
		ສົ່ງຄືນ "^[yY]";
	pak_sa NOEXPR:
		ສົ່ງຄືນ "^[nN]";
	default:
		ສົ່ງຄືນ "";
	}
}

