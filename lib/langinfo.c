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
#समावेश "nls.h"

अक्षर *langinfo_fallback(nl_item item)
{
	चयन (item) {
	हाल CODESET:
		वापस "ISO-8859-1";
	हाल THOUSEP:
		वापस ",";
	हाल D_T_FMT:
	हाल ERA_D_T_FMT:
		वापस "%a %b %e %H:%M:%S %Y";
	हाल D_FMT:
	हाल ERA_D_FMT:
		वापस "%m/%d/%y";
	हाल T_FMT:
	हाल ERA_T_FMT:
		वापस "%H:%M:%S";
	हाल T_FMT_AMPM:
		वापस "%I:%M:%S %p";
	हाल AM_STR:
		वापस "AM";
	हाल PM_STR:
		वापस "PM";
	हाल DAY_1:
		वापस "Sunday";
	हाल DAY_2:
		वापस "Monday";
	हाल DAY_3:
		वापस "Tuesday";
	हाल DAY_4:
		वापस "Wednesday";
	हाल DAY_5:
		वापस "Thursday";
	हाल DAY_6:
		वापस "Friday";
	हाल DAY_7:
		वापस "Saturday";
	हाल ABDAY_1:
		वापस "Sun";
	हाल ABDAY_2:
		वापस "Mon";
	हाल ABDAY_3:
		वापस "Tue";
	हाल ABDAY_4:
		वापस "Wed";
	हाल ABDAY_5:
		वापस "Thu";
	हाल ABDAY_6:
		वापस "Fri";
	हाल ABDAY_7:
		वापस "Sat";
	हाल MON_1:
		वापस "January";
	हाल MON_2:
		वापस "February";
	हाल MON_3:
		वापस "March";
	हाल MON_4:
		वापस "April";
	हाल MON_5:
		वापस "May";
	हाल MON_6:
		वापस "June";
	हाल MON_7:
		वापस "July";
	हाल MON_8:
		वापस "August";
	हाल MON_9:
		वापस "September";
	हाल MON_10:
		वापस "October";
	हाल MON_11:
		वापस "November";
	हाल MON_12:
		वापस "December";
	हाल ABMON_1:
		वापस "Jan";
	हाल ABMON_2:
		वापस "Feb";
	हाल ABMON_3:
		वापस "Mar";
	हाल ABMON_4:
		वापस "Apr";
	हाल ABMON_5:
		वापस "May";
	हाल ABMON_6:
		वापस "Jun";
	हाल ABMON_7:
		वापस "Jul";
	हाल ABMON_8:
		वापस "Aug";
	हाल ABMON_9:
		वापस "Sep";
	हाल ABMON_10:
		वापस "Oct";
	हाल ABMON_11:
		वापस "Nov";
	हाल ABMON_12:
		वापस "Dec";
	हाल ALT_DIGITS:
		वापस "\0\0\0\0\0\0\0\0\0\0";
	हाल CRNCYSTR:
		वापस "-";
	हाल YESEXPR:
		वापस "^[yY]";
	हाल NOEXPR:
		वापस "^[nN]";
	शेष:
		वापस "";
	}
}

