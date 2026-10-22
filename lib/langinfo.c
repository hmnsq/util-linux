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
		ܦܢܐ "ISO-8859-1";
	حالة THOUSEP:
		ܦܢܐ ",";
	حالة D_T_FMT:
	حالة ERA_D_T_FMT:
		ܦܢܐ "%a %b %e %H:%M:%S %Y";
	حالة D_FMT:
	حالة ERA_D_FMT:
		ܦܢܐ "%m/%d/%y";
	حالة T_FMT:
	حالة ERA_T_FMT:
		ܦܢܐ "%H:%M:%S";
	حالة T_FMT_AMPM:
		ܦܢܐ "%I:%M:%S %p";
	حالة AM_STR:
		ܦܢܐ "AM";
	حالة PM_STR:
		ܦܢܐ "PM";
	حالة DAY_1:
		ܦܢܐ "Sunday";
	حالة DAY_2:
		ܦܢܐ "Monday";
	حالة DAY_3:
		ܦܢܐ "Tuesday";
	حالة DAY_4:
		ܦܢܐ "Wednesday";
	حالة DAY_5:
		ܦܢܐ "Thursday";
	حالة DAY_6:
		ܦܢܐ "Friday";
	حالة DAY_7:
		ܦܢܐ "Saturday";
	حالة ABDAY_1:
		ܦܢܐ "Sun";
	حالة ABDAY_2:
		ܦܢܐ "Mon";
	حالة ABDAY_3:
		ܦܢܐ "Tue";
	حالة ABDAY_4:
		ܦܢܐ "Wed";
	حالة ABDAY_5:
		ܦܢܐ "Thu";
	حالة ABDAY_6:
		ܦܢܐ "Fri";
	حالة ABDAY_7:
		ܦܢܐ "Sat";
	حالة MON_1:
		ܦܢܐ "January";
	حالة MON_2:
		ܦܢܐ "February";
	حالة MON_3:
		ܦܢܐ "March";
	حالة MON_4:
		ܦܢܐ "April";
	حالة MON_5:
		ܦܢܐ "May";
	حالة MON_6:
		ܦܢܐ "June";
	حالة MON_7:
		ܦܢܐ "July";
	حالة MON_8:
		ܦܢܐ "August";
	حالة MON_9:
		ܦܢܐ "September";
	حالة MON_10:
		ܦܢܐ "October";
	حالة MON_11:
		ܦܢܐ "November";
	حالة MON_12:
		ܦܢܐ "December";
	حالة ABMON_1:
		ܦܢܐ "Jan";
	حالة ABMON_2:
		ܦܢܐ "Feb";
	حالة ABMON_3:
		ܦܢܐ "Mar";
	حالة ABMON_4:
		ܦܢܐ "Apr";
	حالة ABMON_5:
		ܦܢܐ "May";
	حالة ABMON_6:
		ܦܢܐ "Jun";
	حالة ABMON_7:
		ܦܢܐ "Jul";
	حالة ABMON_8:
		ܦܢܐ "Aug";
	حالة ABMON_9:
		ܦܢܐ "Sep";
	حالة ABMON_10:
		ܦܢܐ "Oct";
	حالة ABMON_11:
		ܦܢܐ "Nov";
	حالة ABMON_12:
		ܦܢܐ "Dec";
	حالة ALT_DIGITS:
		ܦܢܐ "\0\0\0\0\0\0\0\0\0\0";
	حالة CRNCYSTR:
		ܦܢܐ "-";
	حالة YESEXPR:
		ܦܢܐ "^[yY]";
	حالة NOEXPR:
		ܦܢܐ "^[nN]";
	افتراضي:
		ܦܢܐ "";
	}
}

