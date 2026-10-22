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
#अन्तर्भाव "nls.h"

वर्ण *langinfo_fallback(nl_item item)
{
	विकल्प (item) {
	पक्ष CODESET:
		पटिनिवत्त "ISO-8859-1";
	पक्ष THOUSEP:
		पटिनिवत्त ",";
	पक्ष D_T_FMT:
	पक्ष ERA_D_T_FMT:
		पटिनिवत्त "%a %b %e %H:%M:%S %Y";
	पक्ष D_FMT:
	पक्ष ERA_D_FMT:
		पटिनिवत्त "%m/%d/%y";
	पक्ष T_FMT:
	पक्ष ERA_T_FMT:
		पटिनिवत्त "%H:%M:%S";
	पक्ष T_FMT_AMPM:
		पटिनिवत्त "%I:%M:%S %p";
	पक्ष AM_STR:
		पटिनिवत्त "AM";
	पक्ष PM_STR:
		पटिनिवत्त "PM";
	पक्ष DAY_1:
		पटिनिवत्त "Sunday";
	पक्ष DAY_2:
		पटिनिवत्त "Monday";
	पक्ष DAY_3:
		पटिनिवत्त "Tuesday";
	पक्ष DAY_4:
		पटिनिवत्त "Wednesday";
	पक्ष DAY_5:
		पटिनिवत्त "Thursday";
	पक्ष DAY_6:
		पटिनिवत्त "Friday";
	पक्ष DAY_7:
		पटिनिवत्त "Saturday";
	पक्ष ABDAY_1:
		पटिनिवत्त "Sun";
	पक्ष ABDAY_2:
		पटिनिवत्त "Mon";
	पक्ष ABDAY_3:
		पटिनिवत्त "Tue";
	पक्ष ABDAY_4:
		पटिनिवत्त "Wed";
	पक्ष ABDAY_5:
		पटिनिवत्त "Thu";
	पक्ष ABDAY_6:
		पटिनिवत्त "Fri";
	पक्ष ABDAY_7:
		पटिनिवत्त "Sat";
	पक्ष MON_1:
		पटिनिवत्त "January";
	पक्ष MON_2:
		पटिनिवत्त "February";
	पक्ष MON_3:
		पटिनिवत्त "March";
	पक्ष MON_4:
		पटिनिवत्त "April";
	पक्ष MON_5:
		पटिनिवत्त "May";
	पक्ष MON_6:
		पटिनिवत्त "June";
	पक्ष MON_7:
		पटिनिवत्त "July";
	पक्ष MON_8:
		पटिनिवत्त "August";
	पक्ष MON_9:
		पटिनिवत्त "September";
	पक्ष MON_10:
		पटिनिवत्त "October";
	पक्ष MON_11:
		पटिनिवत्त "November";
	पक्ष MON_12:
		पटिनिवत्त "December";
	पक्ष ABMON_1:
		पटिनिवत्त "Jan";
	पक्ष ABMON_2:
		पटिनिवत्त "Feb";
	पक्ष ABMON_3:
		पटिनिवत्त "Mar";
	पक्ष ABMON_4:
		पटिनिवत्त "Apr";
	पक्ष ABMON_5:
		पटिनिवत्त "May";
	पक्ष ABMON_6:
		पटिनिवत्त "Jun";
	पक्ष ABMON_7:
		पटिनिवत्त "Jul";
	पक्ष ABMON_8:
		पटिनिवत्त "Aug";
	पक्ष ABMON_9:
		पटिनिवत्त "Sep";
	पक्ष ABMON_10:
		पटिनिवत्त "Oct";
	पक्ष ABMON_11:
		पटिनिवत्त "Nov";
	पक्ष ABMON_12:
		पटिनिवत्त "Dec";
	पक्ष ALT_DIGITS:
		पटिनिवत्त "\0\0\0\0\0\0\0\0\0\0";
	पक्ष CRNCYSTR:
		पटिनिवत्त "-";
	पक्ष YESEXPR:
		पटिनिवत्त "^[yY]";
	पक्ष NOEXPR:
		पटिनिवत्त "^[nN]";
	स्वभाव:
		पटिनिवत्त "";
	}
}

