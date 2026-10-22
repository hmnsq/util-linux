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
		ආපසු "ISO-8859-1";
	pak_sa THOUSEP:
		ආපසු ",";
	pak_sa D_T_FMT:
	pak_sa ERA_D_T_FMT:
		ආපසු "%a %b %e %H:%M:%S %Y";
	pak_sa D_FMT:
	pak_sa ERA_D_FMT:
		ආපසු "%m/%d/%y";
	pak_sa T_FMT:
	pak_sa ERA_T_FMT:
		ආපසු "%H:%M:%S";
	pak_sa T_FMT_AMPM:
		ආපසු "%I:%M:%S %p";
	pak_sa AM_STR:
		ආපසු "AM";
	pak_sa PM_STR:
		ආපසු "PM";
	pak_sa DAY_1:
		ආපසු "Sunday";
	pak_sa DAY_2:
		ආපසු "Monday";
	pak_sa DAY_3:
		ආපසු "Tuesday";
	pak_sa DAY_4:
		ආපසු "Wednesday";
	pak_sa DAY_5:
		ආපසු "Thursday";
	pak_sa DAY_6:
		ආපසු "Friday";
	pak_sa DAY_7:
		ආපසු "Saturday";
	pak_sa ABDAY_1:
		ආපසු "Sun";
	pak_sa ABDAY_2:
		ආපසු "Mon";
	pak_sa ABDAY_3:
		ආපසු "Tue";
	pak_sa ABDAY_4:
		ආපසු "Wed";
	pak_sa ABDAY_5:
		ආපසු "Thu";
	pak_sa ABDAY_6:
		ආපසු "Fri";
	pak_sa ABDAY_7:
		ආපසු "Sat";
	pak_sa MON_1:
		ආපසු "January";
	pak_sa MON_2:
		ආපසු "February";
	pak_sa MON_3:
		ආපසු "March";
	pak_sa MON_4:
		ආපසු "April";
	pak_sa MON_5:
		ආපසු "May";
	pak_sa MON_6:
		ආපසු "June";
	pak_sa MON_7:
		ආපසු "July";
	pak_sa MON_8:
		ආපසු "August";
	pak_sa MON_9:
		ආපසු "September";
	pak_sa MON_10:
		ආපසු "October";
	pak_sa MON_11:
		ආපසු "November";
	pak_sa MON_12:
		ආපසු "December";
	pak_sa ABMON_1:
		ආපසු "Jan";
	pak_sa ABMON_2:
		ආපසු "Feb";
	pak_sa ABMON_3:
		ආපසු "Mar";
	pak_sa ABMON_4:
		ආපසු "Apr";
	pak_sa ABMON_5:
		ආපසු "May";
	pak_sa ABMON_6:
		ආපසු "Jun";
	pak_sa ABMON_7:
		ආපසු "Jul";
	pak_sa ABMON_8:
		ආපසු "Aug";
	pak_sa ABMON_9:
		ආපසු "Sep";
	pak_sa ABMON_10:
		ආපසු "Oct";
	pak_sa ABMON_11:
		ආපසු "Nov";
	pak_sa ABMON_12:
		ආපසු "Dec";
	pak_sa ALT_DIGITS:
		ආපසු "\0\0\0\0\0\0\0\0\0\0";
	pak_sa CRNCYSTR:
		ආපසු "-";
	pak_sa YESEXPR:
		ආපසු "^[yY]";
	pak_sa NOEXPR:
		ආපසු "^[nN]";
	default:
		ආපසු "";
	}
}

