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
#અન્તર્ભાવ "nls.h"

અક્ષર *langinfo_fallback(nl_item item)
{
	પસંદગી (item) {
	સ્થિતિ CODESET:
		પાછા "ISO-8859-1";
	સ્થિતિ THOUSEP:
		પાછા ",";
	સ્થિતિ D_T_FMT:
	સ્થિતિ ERA_D_T_FMT:
		પાછા "%a %b %e %H:%M:%S %Y";
	સ્થિતિ D_FMT:
	સ્થિતિ ERA_D_FMT:
		પાછા "%m/%d/%y";
	સ્થિતિ T_FMT:
	સ્થિતિ ERA_T_FMT:
		પાછા "%H:%M:%S";
	સ્થિતિ T_FMT_AMPM:
		પાછા "%I:%M:%S %p";
	સ્થિતિ AM_STR:
		પાછા "AM";
	સ્થિતિ PM_STR:
		પાછા "PM";
	સ્થિતિ DAY_1:
		પાછા "Sunday";
	સ્થિતિ DAY_2:
		પાછા "Monday";
	સ્થિતિ DAY_3:
		પાછા "Tuesday";
	સ્થિતિ DAY_4:
		પાછા "Wednesday";
	સ્થિતિ DAY_5:
		પાછા "Thursday";
	સ્થિતિ DAY_6:
		પાછા "Friday";
	સ્થિતિ DAY_7:
		પાછા "Saturday";
	સ્થિતિ ABDAY_1:
		પાછા "Sun";
	સ્થિતિ ABDAY_2:
		પાછા "Mon";
	સ્થિતિ ABDAY_3:
		પાછા "Tue";
	સ્થિતિ ABDAY_4:
		પાછા "Wed";
	સ્થિતિ ABDAY_5:
		પાછા "Thu";
	સ્થિતિ ABDAY_6:
		પાછા "Fri";
	સ્થિતિ ABDAY_7:
		પાછા "Sat";
	સ્થિતિ MON_1:
		પાછા "January";
	સ્થિતિ MON_2:
		પાછા "February";
	સ્થિતિ MON_3:
		પાછા "March";
	સ્થિતિ MON_4:
		પાછા "April";
	સ્થિતિ MON_5:
		પાછા "May";
	સ્થિતિ MON_6:
		પાછા "June";
	સ્થિતિ MON_7:
		પાછા "July";
	સ્થિતિ MON_8:
		પાછા "August";
	સ્થિતિ MON_9:
		પાછા "September";
	સ્થિતિ MON_10:
		પાછા "October";
	સ્થિતિ MON_11:
		પાછા "November";
	સ્થિતિ MON_12:
		પાછા "December";
	સ્થિતિ ABMON_1:
		પાછા "Jan";
	સ્થિતિ ABMON_2:
		પાછા "Feb";
	સ્થિતિ ABMON_3:
		પાછા "Mar";
	સ્થિતિ ABMON_4:
		પાછા "Apr";
	સ્થિતિ ABMON_5:
		પાછા "May";
	સ્થિતિ ABMON_6:
		પાછા "Jun";
	સ્થિતિ ABMON_7:
		પાછા "Jul";
	સ્થિતિ ABMON_8:
		પાછા "Aug";
	સ્થિતિ ABMON_9:
		પાછા "Sep";
	સ્થિતિ ABMON_10:
		પાછા "Oct";
	સ્થિતિ ABMON_11:
		પાછા "Nov";
	સ્થિતિ ABMON_12:
		પાછા "Dec";
	સ્થિતિ ALT_DIGITS:
		પાછા "\0\0\0\0\0\0\0\0\0\0";
	સ્થિતિ CRNCYSTR:
		પાછા "-";
	સ્થિતિ YESEXPR:
		પાછા "^[yY]";
	સ્થિતિ NOEXPR:
		પાછા "^[nN]";
	મૂળભૂત:
		પાછા "";
	}
}

