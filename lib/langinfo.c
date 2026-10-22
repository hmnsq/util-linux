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
		החזר "ISO-8859-1";
	حالة THOUSEP:
		החזר ",";
	حالة D_T_FMT:
	حالة ERA_D_T_FMT:
		החזר "%a %b %e %H:%M:%S %Y";
	حالة D_FMT:
	حالة ERA_D_FMT:
		החזר "%m/%d/%y";
	حالة T_FMT:
	حالة ERA_T_FMT:
		החזר "%H:%M:%S";
	حالة T_FMT_AMPM:
		החזר "%I:%M:%S %p";
	حالة AM_STR:
		החזר "AM";
	حالة PM_STR:
		החזר "PM";
	حالة DAY_1:
		החזר "Sunday";
	حالة DAY_2:
		החזר "Monday";
	حالة DAY_3:
		החזר "Tuesday";
	حالة DAY_4:
		החזר "Wednesday";
	حالة DAY_5:
		החזר "Thursday";
	حالة DAY_6:
		החזר "Friday";
	حالة DAY_7:
		החזר "Saturday";
	حالة ABDAY_1:
		החזר "Sun";
	حالة ABDAY_2:
		החזר "Mon";
	حالة ABDAY_3:
		החזר "Tue";
	حالة ABDAY_4:
		החזר "Wed";
	حالة ABDAY_5:
		החזר "Thu";
	حالة ABDAY_6:
		החזר "Fri";
	حالة ABDAY_7:
		החזר "Sat";
	حالة MON_1:
		החזר "January";
	حالة MON_2:
		החזר "February";
	حالة MON_3:
		החזר "March";
	حالة MON_4:
		החזר "April";
	حالة MON_5:
		החזר "May";
	حالة MON_6:
		החזר "June";
	حالة MON_7:
		החזר "July";
	حالة MON_8:
		החזר "August";
	حالة MON_9:
		החזר "September";
	حالة MON_10:
		החזר "October";
	حالة MON_11:
		החזר "November";
	حالة MON_12:
		החזר "December";
	حالة ABMON_1:
		החזר "Jan";
	حالة ABMON_2:
		החזר "Feb";
	حالة ABMON_3:
		החזר "Mar";
	حالة ABMON_4:
		החזר "Apr";
	حالة ABMON_5:
		החזר "May";
	حالة ABMON_6:
		החזר "Jun";
	حالة ABMON_7:
		החזר "Jul";
	حالة ABMON_8:
		החזר "Aug";
	حالة ABMON_9:
		החזר "Sep";
	حالة ABMON_10:
		החזר "Oct";
	حالة ABMON_11:
		החזר "Nov";
	حالة ABMON_12:
		החזר "Dec";
	حالة ALT_DIGITS:
		החזר "\0\0\0\0\0\0\0\0\0\0";
	حالة CRNCYSTR:
		החזר "-";
	حالة YESEXPR:
		החזר "^[yY]";
	حالة NOEXPR:
		החזר "^[nN]";
	افتراضي:
		החזר "";
	}
}

