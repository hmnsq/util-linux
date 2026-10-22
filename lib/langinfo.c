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
#ଅନ୍ତର୍ଭାଵ "nls.h"

ଅକ୍ଷର *langinfo_fallback(nl_item item)
{
	ବାଛ (item) {
	ସ୍ଥିତି CODESET:
		ଫେରନ୍ତୁ "ISO-8859-1";
	ସ୍ଥିତି THOUSEP:
		ଫେରନ୍ତୁ ",";
	ସ୍ଥିତି D_T_FMT:
	ସ୍ଥିତି ERA_D_T_FMT:
		ଫେରନ୍ତୁ "%a %b %e %H:%M:%S %Y";
	ସ୍ଥିତି D_FMT:
	ସ୍ଥିତି ERA_D_FMT:
		ଫେରନ୍ତୁ "%m/%d/%y";
	ସ୍ଥିତି T_FMT:
	ସ୍ଥିତି ERA_T_FMT:
		ଫେରନ୍ତୁ "%H:%M:%S";
	ସ୍ଥିତି T_FMT_AMPM:
		ଫେରନ୍ତୁ "%I:%M:%S %p";
	ସ୍ଥିତି AM_STR:
		ଫେରନ୍ତୁ "AM";
	ସ୍ଥିତି PM_STR:
		ଫେରନ୍ତୁ "PM";
	ସ୍ଥିତି DAY_1:
		ଫେରନ୍ତୁ "Sunday";
	ସ୍ଥିତି DAY_2:
		ଫେରନ୍ତୁ "Monday";
	ସ୍ଥିତି DAY_3:
		ଫେରନ୍ତୁ "Tuesday";
	ସ୍ଥିତି DAY_4:
		ଫେରନ୍ତୁ "Wednesday";
	ସ୍ଥିତି DAY_5:
		ଫେରନ୍ତୁ "Thursday";
	ସ୍ଥିତି DAY_6:
		ଫେରନ୍ତୁ "Friday";
	ସ୍ଥିତି DAY_7:
		ଫେରନ୍ତୁ "Saturday";
	ସ୍ଥିତି ABDAY_1:
		ଫେରନ୍ତୁ "Sun";
	ସ୍ଥିତି ABDAY_2:
		ଫେରନ୍ତୁ "Mon";
	ସ୍ଥିତି ABDAY_3:
		ଫେରନ୍ତୁ "Tue";
	ସ୍ଥିତି ABDAY_4:
		ଫେରନ୍ତୁ "Wed";
	ସ୍ଥିତି ABDAY_5:
		ଫେରନ୍ତୁ "Thu";
	ସ୍ଥିତି ABDAY_6:
		ଫେରନ୍ତୁ "Fri";
	ସ୍ଥିତି ABDAY_7:
		ଫେରନ୍ତୁ "Sat";
	ସ୍ଥିତି MON_1:
		ଫେରନ୍ତୁ "January";
	ସ୍ଥିତି MON_2:
		ଫେରନ୍ତୁ "February";
	ସ୍ଥିତି MON_3:
		ଫେରନ୍ତୁ "March";
	ସ୍ଥିତି MON_4:
		ଫେରନ୍ତୁ "April";
	ସ୍ଥିତି MON_5:
		ଫେରନ୍ତୁ "May";
	ସ୍ଥିତି MON_6:
		ଫେରନ୍ତୁ "June";
	ସ୍ଥିତି MON_7:
		ଫେରନ୍ତୁ "July";
	ସ୍ଥିତି MON_8:
		ଫେରନ୍ତୁ "August";
	ସ୍ଥିତି MON_9:
		ଫେରନ୍ତୁ "September";
	ସ୍ଥିତି MON_10:
		ଫେରନ୍ତୁ "October";
	ସ୍ଥିତି MON_11:
		ଫେରନ୍ତୁ "November";
	ସ୍ଥିତି MON_12:
		ଫେରନ୍ତୁ "December";
	ସ୍ଥିତି ABMON_1:
		ଫେରନ୍ତୁ "Jan";
	ସ୍ଥିତି ABMON_2:
		ଫେରନ୍ତୁ "Feb";
	ସ୍ଥିତି ABMON_3:
		ଫେରନ୍ତୁ "Mar";
	ସ୍ଥିତି ABMON_4:
		ଫେରନ୍ତୁ "Apr";
	ସ୍ଥିତି ABMON_5:
		ଫେରନ୍ତୁ "May";
	ସ୍ଥିତି ABMON_6:
		ଫେରନ୍ତୁ "Jun";
	ସ୍ଥିତି ABMON_7:
		ଫେରନ୍ତୁ "Jul";
	ସ୍ଥିତି ABMON_8:
		ଫେରନ୍ତୁ "Aug";
	ସ୍ଥିତି ABMON_9:
		ଫେରନ୍ତୁ "Sep";
	ସ୍ଥିତି ABMON_10:
		ଫେରନ୍ତୁ "Oct";
	ସ୍ଥିତି ABMON_11:
		ଫେରନ୍ତୁ "Nov";
	ସ୍ଥିତି ABMON_12:
		ଫେରନ୍ତୁ "Dec";
	ସ୍ଥିତି ALT_DIGITS:
		ଫେରନ୍ତୁ "\0\0\0\0\0\0\0\0\0\0";
	ସ୍ଥିତି CRNCYSTR:
		ଫେରନ୍ତୁ "-";
	ସ୍ଥିତି YESEXPR:
		ଫେରନ୍ତୁ "^[yY]";
	ସ୍ଥିତି NOEXPR:
		ଫେରନ୍ତୁ "^[nN]";
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଫେରନ୍ତୁ "";
	}
}

