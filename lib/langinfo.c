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
		ပွၵ်ႈ "ISO-8859-1";
	pak_sa THOUSEP:
		ပွၵ်ႈ ",";
	pak_sa D_T_FMT:
	pak_sa ERA_D_T_FMT:
		ပွၵ်ႈ "%a %b %e %H:%M:%S %Y";
	pak_sa D_FMT:
	pak_sa ERA_D_FMT:
		ပွၵ်ႈ "%m/%d/%y";
	pak_sa T_FMT:
	pak_sa ERA_T_FMT:
		ပွၵ်ႈ "%H:%M:%S";
	pak_sa T_FMT_AMPM:
		ပွၵ်ႈ "%I:%M:%S %p";
	pak_sa AM_STR:
		ပွၵ်ႈ "AM";
	pak_sa PM_STR:
		ပွၵ်ႈ "PM";
	pak_sa DAY_1:
		ပွၵ်ႈ "Sunday";
	pak_sa DAY_2:
		ပွၵ်ႈ "Monday";
	pak_sa DAY_3:
		ပွၵ်ႈ "Tuesday";
	pak_sa DAY_4:
		ပွၵ်ႈ "Wednesday";
	pak_sa DAY_5:
		ပွၵ်ႈ "Thursday";
	pak_sa DAY_6:
		ပွၵ်ႈ "Friday";
	pak_sa DAY_7:
		ပွၵ်ႈ "Saturday";
	pak_sa ABDAY_1:
		ပွၵ်ႈ "Sun";
	pak_sa ABDAY_2:
		ပွၵ်ႈ "Mon";
	pak_sa ABDAY_3:
		ပွၵ်ႈ "Tue";
	pak_sa ABDAY_4:
		ပွၵ်ႈ "Wed";
	pak_sa ABDAY_5:
		ပွၵ်ႈ "Thu";
	pak_sa ABDAY_6:
		ပွၵ်ႈ "Fri";
	pak_sa ABDAY_7:
		ပွၵ်ႈ "Sat";
	pak_sa MON_1:
		ပွၵ်ႈ "January";
	pak_sa MON_2:
		ပွၵ်ႈ "February";
	pak_sa MON_3:
		ပွၵ်ႈ "March";
	pak_sa MON_4:
		ပွၵ်ႈ "April";
	pak_sa MON_5:
		ပွၵ်ႈ "May";
	pak_sa MON_6:
		ပွၵ်ႈ "June";
	pak_sa MON_7:
		ပွၵ်ႈ "July";
	pak_sa MON_8:
		ပွၵ်ႈ "August";
	pak_sa MON_9:
		ပွၵ်ႈ "September";
	pak_sa MON_10:
		ပွၵ်ႈ "October";
	pak_sa MON_11:
		ပွၵ်ႈ "November";
	pak_sa MON_12:
		ပွၵ်ႈ "December";
	pak_sa ABMON_1:
		ပွၵ်ႈ "Jan";
	pak_sa ABMON_2:
		ပွၵ်ႈ "Feb";
	pak_sa ABMON_3:
		ပွၵ်ႈ "Mar";
	pak_sa ABMON_4:
		ပွၵ်ႈ "Apr";
	pak_sa ABMON_5:
		ပွၵ်ႈ "May";
	pak_sa ABMON_6:
		ပွၵ်ႈ "Jun";
	pak_sa ABMON_7:
		ပွၵ်ႈ "Jul";
	pak_sa ABMON_8:
		ပွၵ်ႈ "Aug";
	pak_sa ABMON_9:
		ပွၵ်ႈ "Sep";
	pak_sa ABMON_10:
		ပွၵ်ႈ "Oct";
	pak_sa ABMON_11:
		ပွၵ်ႈ "Nov";
	pak_sa ABMON_12:
		ပွၵ်ႈ "Dec";
	pak_sa ALT_DIGITS:
		ပွၵ်ႈ "\0\0\0\0\0\0\0\0\0\0";
	pak_sa CRNCYSTR:
		ပွၵ်ႈ "-";
	pak_sa YESEXPR:
		ပွၵ်ႈ "^[yY]";
	pak_sa NOEXPR:
		ပွၵ်ႈ "^[nN]";
	default:
		ပွၵ်ႈ "";
	}
}

