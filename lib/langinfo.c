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
#ضمن "nls.h"

حرف *langinfo_fallback(nl_item item)
{
	بدل (item) {
	حالة CODESET:
		أرجع "ISO-8859-1";
	حالة THOUSEP:
		أرجع ",";
	حالة D_T_FMT:
	حالة ERA_D_T_FMT:
		أرجع "%a %b %e %H:%M:%S %Y";
	حالة D_FMT:
	حالة ERA_D_FMT:
		أرجع "%m/%d/%y";
	حالة T_FMT:
	حالة ERA_T_FMT:
		أرجع "%H:%M:%S";
	حالة T_FMT_AMPM:
		أرجع "%I:%M:%S %p";
	حالة AM_STR:
		أرجع "AM";
	حالة PM_STR:
		أرجع "PM";
	حالة DAY_1:
		أرجع "Sunday";
	حالة DAY_2:
		أرجع "Monday";
	حالة DAY_3:
		أرجع "Tuesday";
	حالة DAY_4:
		أرجع "Wednesday";
	حالة DAY_5:
		أرجع "Thursday";
	حالة DAY_6:
		أرجع "Friday";
	حالة DAY_7:
		أرجع "Saturday";
	حالة ABDAY_1:
		أرجع "Sun";
	حالة ABDAY_2:
		أرجع "Mon";
	حالة ABDAY_3:
		أرجع "Tue";
	حالة ABDAY_4:
		أرجع "Wed";
	حالة ABDAY_5:
		أرجع "Thu";
	حالة ABDAY_6:
		أرجع "Fri";
	حالة ABDAY_7:
		أرجع "Sat";
	حالة MON_1:
		أرجع "January";
	حالة MON_2:
		أرجع "February";
	حالة MON_3:
		أرجع "March";
	حالة MON_4:
		أرجع "April";
	حالة MON_5:
		أرجع "May";
	حالة MON_6:
		أرجع "June";
	حالة MON_7:
		أرجع "July";
	حالة MON_8:
		أرجع "August";
	حالة MON_9:
		أرجع "September";
	حالة MON_10:
		أرجع "October";
	حالة MON_11:
		أرجع "November";
	حالة MON_12:
		أرجع "December";
	حالة ABMON_1:
		أرجع "Jan";
	حالة ABMON_2:
		أرجع "Feb";
	حالة ABMON_3:
		أرجع "Mar";
	حالة ABMON_4:
		أرجع "Apr";
	حالة ABMON_5:
		أرجع "May";
	حالة ABMON_6:
		أرجع "Jun";
	حالة ABMON_7:
		أرجع "Jul";
	حالة ABMON_8:
		أرجع "Aug";
	حالة ABMON_9:
		أرجع "Sep";
	حالة ABMON_10:
		أرجع "Oct";
	حالة ABMON_11:
		أرجع "Nov";
	حالة ABMON_12:
		أرجع "Dec";
	حالة ALT_DIGITS:
		أرجع "\0\0\0\0\0\0\0\0\0\0";
	حالة CRNCYSTR:
		أرجع "-";
	حالة YESEXPR:
		أرجع "^[yY]";
	حالة NOEXPR:
		أرجع "^[nN]";
	افتراضي:
		أرجع "";
	}
}

