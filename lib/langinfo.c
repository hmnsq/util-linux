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

অক্ষর *langinfo_fallback(nl_item item)
{
	নির্বাচন (item) {
	ক্ষেত্র CODESET:
		ফেরত "ISO-8859-1";
	ক্ষেত্র THOUSEP:
		ফেরত ",";
	ক্ষেত্র D_T_FMT:
	ক্ষেত্র ERA_D_T_FMT:
		ফেরত "%a %b %e %H:%M:%S %Y";
	ক্ষেত্র D_FMT:
	ক্ষেত্র ERA_D_FMT:
		ফেরত "%m/%d/%y";
	ক্ষেত্র T_FMT:
	ক্ষেত্র ERA_T_FMT:
		ফেরত "%H:%M:%S";
	ক্ষেত্র T_FMT_AMPM:
		ফেরত "%I:%M:%S %p";
	ক্ষেত্র AM_STR:
		ফেরত "AM";
	ক্ষেত্র PM_STR:
		ফেরত "PM";
	ক্ষেত্র DAY_1:
		ফেরত "Sunday";
	ক্ষেত্র DAY_2:
		ফেরত "Monday";
	ক্ষেত্র DAY_3:
		ফেরত "Tuesday";
	ক্ষেত্র DAY_4:
		ফেরত "Wednesday";
	ক্ষেত্র DAY_5:
		ফেরত "Thursday";
	ক্ষেত্র DAY_6:
		ফেরত "Friday";
	ক্ষেত্র DAY_7:
		ফেরত "Saturday";
	ক্ষেত্র ABDAY_1:
		ফেরত "Sun";
	ক্ষেত্র ABDAY_2:
		ফেরত "Mon";
	ক্ষেত্র ABDAY_3:
		ফেরত "Tue";
	ক্ষেত্র ABDAY_4:
		ফেরত "Wed";
	ক্ষেত্র ABDAY_5:
		ফেরত "Thu";
	ক্ষেত্র ABDAY_6:
		ফেরত "Fri";
	ক্ষেত্র ABDAY_7:
		ফেরত "Sat";
	ক্ষেত্র MON_1:
		ফেরত "January";
	ক্ষেত্র MON_2:
		ফেরত "February";
	ক্ষেত্র MON_3:
		ফেরত "March";
	ক্ষেত্র MON_4:
		ফেরত "April";
	ক্ষেত্র MON_5:
		ফেরত "May";
	ক্ষেত্র MON_6:
		ফেরত "June";
	ক্ষেত্র MON_7:
		ফেরত "July";
	ক্ষেত্র MON_8:
		ফেরত "August";
	ক্ষেত্র MON_9:
		ফেরত "September";
	ক্ষেত্র MON_10:
		ফেরত "October";
	ক্ষেত্র MON_11:
		ফেরত "November";
	ক্ষেত্র MON_12:
		ফেরত "December";
	ক্ষেত্র ABMON_1:
		ফেরত "Jan";
	ক্ষেত্র ABMON_2:
		ফেরত "Feb";
	ক্ষেত্র ABMON_3:
		ফেরত "Mar";
	ক্ষেত্র ABMON_4:
		ফেরত "Apr";
	ক্ষেত্র ABMON_5:
		ফেরত "May";
	ক্ষেত্র ABMON_6:
		ফেরত "Jun";
	ক্ষেত্র ABMON_7:
		ফেরত "Jul";
	ক্ষেত্র ABMON_8:
		ফেরত "Aug";
	ক্ষেত্র ABMON_9:
		ফেরত "Sep";
	ক্ষেত্র ABMON_10:
		ফেরত "Oct";
	ক্ষেত্র ABMON_11:
		ফেরত "Nov";
	ক্ষেত্র ABMON_12:
		ফেরত "Dec";
	ক্ষেত্র ALT_DIGITS:
		ফেরত "\0\0\0\0\0\0\0\0\0\0";
	ক্ষেত্র CRNCYSTR:
		ফেরত "-";
	ক্ষেত্র YESEXPR:
		ফেরত "^[yY]";
	ক্ষেত্র NOEXPR:
		ফেরত "^[nN]";
	পূর্বনির্ধারিত:
		ফেরত "";
	}
}

