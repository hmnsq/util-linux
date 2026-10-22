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
#অন্তর্ভাব "nls.h"

বর্ণ *langinfo_fallback(nl_item item)
{
	বিকল্প (item) {
	পক্ষ CODESET:
		প্রত্যাগম "ISO-8859-1";
	পক্ষ THOUSEP:
		প্রত্যাগম ",";
	পক্ষ D_T_FMT:
	পক্ষ ERA_D_T_FMT:
		প্রত্যাগম "%a %b %e %H:%M:%S %Y";
	পক্ষ D_FMT:
	পক্ষ ERA_D_FMT:
		প্রত্যাগম "%m/%d/%y";
	পক্ষ T_FMT:
	পক্ষ ERA_T_FMT:
		প্রত্যাগম "%H:%M:%S";
	পক্ষ T_FMT_AMPM:
		প্রত্যাগম "%I:%M:%S %p";
	পক্ষ AM_STR:
		প্রত্যাগম "AM";
	পক্ষ PM_STR:
		প্রত্যাগম "PM";
	পক্ষ DAY_1:
		প্রত্যাগম "Sunday";
	পক্ষ DAY_2:
		প্রত্যাগম "Monday";
	পক্ষ DAY_3:
		প্রত্যাগম "Tuesday";
	পক্ষ DAY_4:
		প্রত্যাগম "Wednesday";
	পক্ষ DAY_5:
		প্রত্যাগম "Thursday";
	পক্ষ DAY_6:
		প্রত্যাগম "Friday";
	পক্ষ DAY_7:
		প্রত্যাগম "Saturday";
	পক্ষ ABDAY_1:
		প্রত্যাগম "Sun";
	পক্ষ ABDAY_2:
		প্রত্যাগম "Mon";
	পক্ষ ABDAY_3:
		প্রত্যাগম "Tue";
	পক্ষ ABDAY_4:
		প্রত্যাগম "Wed";
	পক্ষ ABDAY_5:
		প্রত্যাগম "Thu";
	পক্ষ ABDAY_6:
		প্রত্যাগম "Fri";
	পক্ষ ABDAY_7:
		প্রত্যাগম "Sat";
	পক্ষ MON_1:
		প্রত্যাগম "January";
	পক্ষ MON_2:
		প্রত্যাগম "February";
	পক্ষ MON_3:
		প্রত্যাগম "March";
	পক্ষ MON_4:
		প্রত্যাগম "April";
	পক্ষ MON_5:
		প্রত্যাগম "May";
	পক্ষ MON_6:
		প্রত্যাগম "June";
	পক্ষ MON_7:
		প্রত্যাগম "July";
	পক্ষ MON_8:
		প্রত্যাগম "August";
	পক্ষ MON_9:
		প্রত্যাগম "September";
	পক্ষ MON_10:
		প্রত্যাগম "October";
	পক্ষ MON_11:
		প্রত্যাগম "November";
	পক্ষ MON_12:
		প্রত্যাগম "December";
	পক্ষ ABMON_1:
		প্রত্যাগম "Jan";
	পক্ষ ABMON_2:
		প্রত্যাগম "Feb";
	পক্ষ ABMON_3:
		প্রত্যাগম "Mar";
	পক্ষ ABMON_4:
		প্রত্যাগম "Apr";
	পক্ষ ABMON_5:
		প্রত্যাগম "May";
	পক্ষ ABMON_6:
		প্রত্যাগম "Jun";
	পক্ষ ABMON_7:
		প্রত্যাগম "Jul";
	পক্ষ ABMON_8:
		প্রত্যাগম "Aug";
	পক্ষ ABMON_9:
		প্রত্যাগম "Sep";
	পক্ষ ABMON_10:
		প্রত্যাগম "Oct";
	পক্ষ ABMON_11:
		প্রত্যাগম "Nov";
	পক্ষ ABMON_12:
		প্রত্যাগম "Dec";
	পক্ষ ALT_DIGITS:
		প্রত্যাগম "\0\0\0\0\0\0\0\0\0\0";
	পক্ষ CRNCYSTR:
		প্রত্যাগম "-";
	পক্ষ YESEXPR:
		প্রত্যাগম "^[yY]";
	পক্ষ NOEXPR:
		প্রত্যাগম "^[nN]";
	স্বভাব:
		প্রত্যাগম "";
	}
}

