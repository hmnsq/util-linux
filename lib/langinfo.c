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

نویسه *langinfo_fallback(nl_item item)
{
	انتخاب (item) {
	حالت CODESET:
		بازگشت "ISO-8859-1";
	حالت THOUSEP:
		بازگشت ",";
	حالت D_T_FMT:
	حالت ERA_D_T_FMT:
		بازگشت "%a %b %e %H:%M:%S %Y";
	حالت D_FMT:
	حالت ERA_D_FMT:
		بازگشت "%m/%d/%y";
	حالت T_FMT:
	حالت ERA_T_FMT:
		بازگشت "%H:%M:%S";
	حالت T_FMT_AMPM:
		بازگشت "%I:%M:%S %p";
	حالت AM_STR:
		بازگشت "AM";
	حالت PM_STR:
		بازگشت "PM";
	حالت DAY_1:
		بازگشت "Sunday";
	حالت DAY_2:
		بازگشت "Monday";
	حالت DAY_3:
		بازگشت "Tuesday";
	حالت DAY_4:
		بازگشت "Wednesday";
	حالت DAY_5:
		بازگشت "Thursday";
	حالت DAY_6:
		بازگشت "Friday";
	حالت DAY_7:
		بازگشت "Saturday";
	حالت ABDAY_1:
		بازگشت "Sun";
	حالت ABDAY_2:
		بازگشت "Mon";
	حالت ABDAY_3:
		بازگشت "Tue";
	حالت ABDAY_4:
		بازگشت "Wed";
	حالت ABDAY_5:
		بازگشت "Thu";
	حالت ABDAY_6:
		بازگشت "Fri";
	حالت ABDAY_7:
		بازگشت "Sat";
	حالت MON_1:
		بازگشت "January";
	حالت MON_2:
		بازگشت "February";
	حالت MON_3:
		بازگشت "March";
	حالت MON_4:
		بازگشت "April";
	حالت MON_5:
		بازگشت "May";
	حالت MON_6:
		بازگشت "June";
	حالت MON_7:
		بازگشت "July";
	حالت MON_8:
		بازگشت "August";
	حالت MON_9:
		بازگشت "September";
	حالت MON_10:
		بازگشت "October";
	حالت MON_11:
		بازگشت "November";
	حالت MON_12:
		بازگشت "December";
	حالت ABMON_1:
		بازگشت "Jan";
	حالت ABMON_2:
		بازگشت "Feb";
	حالت ABMON_3:
		بازگشت "Mar";
	حالت ABMON_4:
		بازگشت "Apr";
	حالت ABMON_5:
		بازگشت "May";
	حالت ABMON_6:
		بازگشت "Jun";
	حالت ABMON_7:
		بازگشت "Jul";
	حالت ABMON_8:
		بازگشت "Aug";
	حالت ABMON_9:
		بازگشت "Sep";
	حالت ABMON_10:
		بازگشت "Oct";
	حالت ABMON_11:
		بازگشت "Nov";
	حالت ABMON_12:
		بازگشت "Dec";
	حالت ALT_DIGITS:
		بازگشت "\0\0\0\0\0\0\0\0\0\0";
	حالت CRNCYSTR:
		بازگشت "-";
	حالت YESEXPR:
		بازگشت "^[yY]";
	حالت NOEXPR:
		بازگشت "^[nN]";
	پیشفرض:
		بازگشت "";
	}
}

