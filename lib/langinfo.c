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
#شامل "nls.h"

حرف *langinfo_fallback(nl_شے شے)
{
	انتخاب (شے) {
	حالت CODESET:
		واپس "ISO-8859-1";
	حالت THOUSEP:
		واپس ",";
	حالت D_T_FMT:
	حالت ERA_D_T_FMT:
		واپس "%a %b %e %H:%M:%S %Y";
	حالت D_FMT:
	حالت ERA_D_FMT:
		واپس "%m/%d/%y";
	حالت T_FMT:
	حالت ERA_T_FMT:
		واپس "%H:%M:%S";
	حالت T_FMT_AMPM:
		واپس "%I:%M:%S %p";
	حالت AM_STR:
		واپس "AM";
	حالت PM_STR:
		واپس "PM";
	حالت DAY_1:
		واپس "Sunday";
	حالت DAY_2:
		واپس "Monday";
	حالت DAY_3:
		واپس "Tuesday";
	حالت DAY_4:
		واپس "Wednesday";
	حالت DAY_5:
		واپس "Thursday";
	حالت DAY_6:
		واپس "Friday";
	حالت DAY_7:
		واپس "Saturday";
	حالت ABDAY_1:
		واپس "Sun";
	حالت ABDAY_2:
		واپس "Mon";
	حالت ABDAY_3:
		واپس "Tue";
	حالت ABDAY_4:
		واپس "Wed";
	حالت ABDAY_5:
		واپس "Thu";
	حالت ABDAY_6:
		واپس "Fri";
	حالت ABDAY_7:
		واپس "Sat";
	حالت MON_1:
		واپس "January";
	حالت MON_2:
		واپس "February";
	حالت MON_3:
		واپس "March";
	حالت MON_4:
		واپس "April";
	حالت MON_5:
		واپس "May";
	حالت MON_6:
		واپس "June";
	حالت MON_7:
		واپس "July";
	حالت MON_8:
		واپس "August";
	حالت MON_9:
		واپس "September";
	حالت MON_10:
		واپس "October";
	حالت MON_11:
		واپس "November";
	حالت MON_12:
		واپس "December";
	حالت ABMON_1:
		واپس "Jan";
	حالت ABMON_2:
		واپس "Feb";
	حالت ABMON_3:
		واپس "Mar";
	حالت ABMON_4:
		واپس "Apr";
	حالت ABMON_5:
		واپس "May";
	حالت ABMON_6:
		واپس "Jun";
	حالت ABMON_7:
		واپس "Jul";
	حالت ABMON_8:
		واپس "Aug";
	حالت ABMON_9:
		واپس "Sep";
	حالت ABMON_10:
		واپس "Oct";
	حالت ABMON_11:
		واپس "Nov";
	حالت ABMON_12:
		واپس "Dec";
	حالت ALT_DIGITS:
		واپس "\0\0\0\0\0\0\0\0\0\0";
	حالت CRNCYSTR:
		واپس "-";
	حالت YESEXPR:
		واپس "^[yY]";
	حالت NOEXPR:
		واپس "^[nN]";
	طےشدہ:
		واپس "";
	}
}

