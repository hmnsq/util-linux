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
#অন্তর্ভাৱ "nls.h"

ৱর্ণ *langinfo_fallback(nl_item item)
{
	ৱিকল্প (item) {
	পক্ষ CODESET:
		ঘূৰাই "ISO-8859-1";
	পক্ষ THOUSEP:
		ঘূৰাই ",";
	পক্ষ D_T_FMT:
	পক্ষ ERA_D_T_FMT:
		ঘূৰাই "%a %b %e %H:%M:%S %Y";
	পক্ষ D_FMT:
	পক্ষ ERA_D_FMT:
		ঘূৰাই "%m/%d/%y";
	পক্ষ T_FMT:
	পক্ষ ERA_T_FMT:
		ঘূৰাই "%H:%M:%S";
	পক্ষ T_FMT_AMPM:
		ঘূৰাই "%I:%M:%S %p";
	পক্ষ AM_STR:
		ঘূৰাই "AM";
	পক্ষ PM_STR:
		ঘূৰাই "PM";
	পক্ষ DAY_1:
		ঘূৰাই "Sunday";
	পক্ষ DAY_2:
		ঘূৰাই "Monday";
	পক্ষ DAY_3:
		ঘূৰাই "Tuesday";
	পক্ষ DAY_4:
		ঘূৰাই "Wednesday";
	পক্ষ DAY_5:
		ঘূৰাই "Thursday";
	পক্ষ DAY_6:
		ঘূৰাই "Friday";
	পক্ষ DAY_7:
		ঘূৰাই "Saturday";
	পক্ষ ABDAY_1:
		ঘূৰাই "Sun";
	পক্ষ ABDAY_2:
		ঘূৰাই "Mon";
	পক্ষ ABDAY_3:
		ঘূৰাই "Tue";
	পক্ষ ABDAY_4:
		ঘূৰাই "Wed";
	পক্ষ ABDAY_5:
		ঘূৰাই "Thu";
	পক্ষ ABDAY_6:
		ঘূৰাই "Fri";
	পক্ষ ABDAY_7:
		ঘূৰাই "Sat";
	পক্ষ MON_1:
		ঘূৰাই "January";
	পক্ষ MON_2:
		ঘূৰাই "February";
	পক্ষ MON_3:
		ঘূৰাই "March";
	পক্ষ MON_4:
		ঘূৰাই "April";
	পক্ষ MON_5:
		ঘূৰাই "May";
	পক্ষ MON_6:
		ঘূৰাই "June";
	পক্ষ MON_7:
		ঘূৰাই "July";
	পক্ষ MON_8:
		ঘূৰাই "August";
	পক্ষ MON_9:
		ঘূৰাই "September";
	পক্ষ MON_10:
		ঘূৰাই "October";
	পক্ষ MON_11:
		ঘূৰাই "November";
	পক্ষ MON_12:
		ঘূৰাই "December";
	পক্ষ ABMON_1:
		ঘূৰাই "Jan";
	পক্ষ ABMON_2:
		ঘূৰাই "Feb";
	পক্ষ ABMON_3:
		ঘূৰাই "Mar";
	পক্ষ ABMON_4:
		ঘূৰাই "Apr";
	পক্ষ ABMON_5:
		ঘূৰাই "May";
	পক্ষ ABMON_6:
		ঘূৰাই "Jun";
	পক্ষ ABMON_7:
		ঘূৰাই "Jul";
	পক্ষ ABMON_8:
		ঘূৰাই "Aug";
	পক্ষ ABMON_9:
		ঘূৰাই "Sep";
	পক্ষ ABMON_10:
		ঘূৰাই "Oct";
	পক্ষ ABMON_11:
		ঘূৰাই "Nov";
	পক্ষ ABMON_12:
		ঘূৰাই "Dec";
	পক্ষ ALT_DIGITS:
		ঘূৰাই "\0\0\0\0\0\0\0\0\0\0";
	পক্ষ CRNCYSTR:
		ঘূৰাই "-";
	পক্ষ YESEXPR:
		ঘূৰাই "^[yY]";
	পক্ষ NOEXPR:
		ঘূৰাই "^[nN]";
	স্ৱভাৱ:
		ঘূৰাই "";
	}
}

