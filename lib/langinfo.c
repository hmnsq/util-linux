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
		إرجاع "ISO-8859-1";
	حالة THOUSEP:
		إرجاع ",";
	حالة D_T_FMT:
	حالة ERA_D_T_FMT:
		إرجاع "%a %b %e %H:%M:%S %Y";
	حالة D_FMT:
	حالة ERA_D_FMT:
		إرجاع "%m/%d/%y";
	حالة T_FMT:
	حالة ERA_T_FMT:
		إرجاع "%H:%M:%S";
	حالة T_FMT_AMPM:
		إرجاع "%I:%M:%S %p";
	حالة AM_STR:
		إرجاع "AM";
	حالة PM_STR:
		إرجاع "PM";
	حالة DAY_1:
		إرجاع "Sunday";
	حالة DAY_2:
		إرجاع "Monday";
	حالة DAY_3:
		إرجاع "Tuesday";
	حالة DAY_4:
		إرجاع "Wednesday";
	حالة DAY_5:
		إرجاع "Thursday";
	حالة DAY_6:
		إرجاع "Friday";
	حالة DAY_7:
		إرجاع "Saturday";
	حالة ABDAY_1:
		إرجاع "Sun";
	حالة ABDAY_2:
		إرجاع "Mon";
	حالة ABDAY_3:
		إرجاع "Tue";
	حالة ABDAY_4:
		إرجاع "Wed";
	حالة ABDAY_5:
		إرجاع "Thu";
	حالة ABDAY_6:
		إرجاع "Fri";
	حالة ABDAY_7:
		إرجاع "Sat";
	حالة MON_1:
		إرجاع "January";
	حالة MON_2:
		إرجاع "February";
	حالة MON_3:
		إرجاع "March";
	حالة MON_4:
		إرجاع "April";
	حالة MON_5:
		إرجاع "May";
	حالة MON_6:
		إرجاع "June";
	حالة MON_7:
		إرجاع "July";
	حالة MON_8:
		إرجاع "August";
	حالة MON_9:
		إرجاع "September";
	حالة MON_10:
		إرجاع "October";
	حالة MON_11:
		إرجاع "November";
	حالة MON_12:
		إرجاع "December";
	حالة ABMON_1:
		إرجاع "Jan";
	حالة ABMON_2:
		إرجاع "Feb";
	حالة ABMON_3:
		إرجاع "Mar";
	حالة ABMON_4:
		إرجاع "Apr";
	حالة ABMON_5:
		إرجاع "May";
	حالة ABMON_6:
		إرجاع "Jun";
	حالة ABMON_7:
		إرجاع "Jul";
	حالة ABMON_8:
		إرجاع "Aug";
	حالة ABMON_9:
		إرجاع "Sep";
	حالة ABMON_10:
		إرجاع "Oct";
	حالة ABMON_11:
		إرجاع "Nov";
	حالة ABMON_12:
		إرجاع "Dec";
	حالة ALT_DIGITS:
		إرجاع "\0\0\0\0\0\0\0\0\0\0";
	حالة CRNCYSTR:
		إرجاع "-";
	حالة YESEXPR:
		إرجاع "^[yY]";
	حالة NOEXPR:
		إرجاع "^[nN]";
	افتراضي:
		إرجاع "";
	}
}

