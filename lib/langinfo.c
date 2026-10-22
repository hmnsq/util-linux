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
#அந்தர்பாவ "nls.h"

எழுத்து *langinfo_fallback(nl_item item)
{
	தேர்வு (item) {
	நிலை CODESET:
		திரும்பு "ISO-8859-1";
	நிலை THOUSEP:
		திரும்பு ",";
	நிலை D_T_FMT:
	நிலை ERA_D_T_FMT:
		திரும்பு "%a %b %e %H:%M:%S %Y";
	நிலை D_FMT:
	நிலை ERA_D_FMT:
		திரும்பு "%m/%d/%y";
	நிலை T_FMT:
	நிலை ERA_T_FMT:
		திரும்பு "%H:%M:%S";
	நிலை T_FMT_AMPM:
		திரும்பு "%I:%M:%S %p";
	நிலை AM_STR:
		திரும்பு "AM";
	நிலை PM_STR:
		திரும்பு "PM";
	நிலை DAY_1:
		திரும்பு "Sunday";
	நிலை DAY_2:
		திரும்பு "Monday";
	நிலை DAY_3:
		திரும்பு "Tuesday";
	நிலை DAY_4:
		திரும்பு "Wednesday";
	நிலை DAY_5:
		திரும்பு "Thursday";
	நிலை DAY_6:
		திரும்பு "Friday";
	நிலை DAY_7:
		திரும்பு "Saturday";
	நிலை ABDAY_1:
		திரும்பு "Sun";
	நிலை ABDAY_2:
		திரும்பு "Mon";
	நிலை ABDAY_3:
		திரும்பு "Tue";
	நிலை ABDAY_4:
		திரும்பு "Wed";
	நிலை ABDAY_5:
		திரும்பு "Thu";
	நிலை ABDAY_6:
		திரும்பு "Fri";
	நிலை ABDAY_7:
		திரும்பு "Sat";
	நிலை MON_1:
		திரும்பு "January";
	நிலை MON_2:
		திரும்பு "February";
	நிலை MON_3:
		திரும்பு "March";
	நிலை MON_4:
		திரும்பு "April";
	நிலை MON_5:
		திரும்பு "May";
	நிலை MON_6:
		திரும்பு "June";
	நிலை MON_7:
		திரும்பு "July";
	நிலை MON_8:
		திரும்பு "August";
	நிலை MON_9:
		திரும்பு "September";
	நிலை MON_10:
		திரும்பு "October";
	நிலை MON_11:
		திரும்பு "November";
	நிலை MON_12:
		திரும்பு "December";
	நிலை ABMON_1:
		திரும்பு "Jan";
	நிலை ABMON_2:
		திரும்பு "Feb";
	நிலை ABMON_3:
		திரும்பு "Mar";
	நிலை ABMON_4:
		திரும்பு "Apr";
	நிலை ABMON_5:
		திரும்பு "May";
	நிலை ABMON_6:
		திரும்பு "Jun";
	நிலை ABMON_7:
		திரும்பு "Jul";
	நிலை ABMON_8:
		திரும்பு "Aug";
	நிலை ABMON_9:
		திரும்பு "Sep";
	நிலை ABMON_10:
		திரும்பு "Oct";
	நிலை ABMON_11:
		திரும்பு "Nov";
	நிலை ABMON_12:
		திரும்பு "Dec";
	நிலை ALT_DIGITS:
		திரும்பு "\0\0\0\0\0\0\0\0\0\0";
	நிலை CRNCYSTR:
		திரும்பு "-";
	நிலை YESEXPR:
		திரும்பு "^[yY]";
	நிலை NOEXPR:
		திரும்பு "^[nN]";
	இயல்பு:
		திரும்பு "";
	}
}

