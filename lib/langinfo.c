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
		प्रत्यागम "ISO-8859-1";
	पक्ष THOUSEP:
		प्रत्यागम ",";
	पक्ष D_T_FMT:
	पक्ष ERA_D_T_FMT:
		प्रत्यागम "%a %b %e %H:%M:%S %Y";
	पक्ष D_FMT:
	पक्ष ERA_D_FMT:
		प्रत्यागम "%m/%d/%y";
	पक्ष T_FMT:
	पक्ष ERA_T_FMT:
		प्रत्यागम "%H:%M:%S";
	पक्ष T_FMT_AMPM:
		प्रत्यागम "%I:%M:%S %p";
	पक्ष AM_STR:
		प्रत्यागम "AM";
	पक्ष PM_STR:
		प्रत्यागम "PM";
	पक्ष DAY_1:
		प्रत्यागम "Sunday";
	पक्ष DAY_2:
		प्रत्यागम "Monday";
	पक्ष DAY_3:
		प्रत्यागम "Tuesday";
	पक्ष DAY_4:
		प्रत्यागम "Wednesday";
	पक्ष DAY_5:
		प्रत्यागम "Thursday";
	पक्ष DAY_6:
		प्रत्यागम "Friday";
	पक्ष DAY_7:
		प्रत्यागम "Saturday";
	पक्ष ABDAY_1:
		प्रत्यागम "Sun";
	पक्ष ABDAY_2:
		प्रत्यागम "Mon";
	पक्ष ABDAY_3:
		प्रत्यागम "Tue";
	पक्ष ABDAY_4:
		प्रत्यागम "Wed";
	पक्ष ABDAY_5:
		प्रत्यागम "Thu";
	पक्ष ABDAY_6:
		प्रत्यागम "Fri";
	पक्ष ABDAY_7:
		प्रत्यागम "Sat";
	पक्ष MON_1:
		प्रत्यागम "January";
	पक्ष MON_2:
		प्रत्यागम "February";
	पक्ष MON_3:
		प्रत्यागम "March";
	पक्ष MON_4:
		प्रत्यागम "April";
	पक्ष MON_5:
		प्रत्यागम "May";
	पक्ष MON_6:
		प्रत्यागम "June";
	पक्ष MON_7:
		प्रत्यागम "July";
	पक्ष MON_8:
		प्रत्यागम "August";
	पक्ष MON_9:
		प्रत्यागम "September";
	पक्ष MON_10:
		प्रत्यागम "October";
	पक्ष MON_11:
		प्रत्यागम "November";
	पक्ष MON_12:
		प्रत्यागम "December";
	पक्ष ABMON_1:
		प्रत्यागम "Jan";
	पक्ष ABMON_2:
		प्रत्यागम "Feb";
	पक्ष ABMON_3:
		प्रत्यागम "Mar";
	पक्ष ABMON_4:
		प्रत्यागम "Apr";
	पक्ष ABMON_5:
		प्रत्यागम "May";
	पक्ष ABMON_6:
		प्रत्यागम "Jun";
	पक्ष ABMON_7:
		प्रत्यागम "Jul";
	पक्ष ABMON_8:
		प्रत्यागम "Aug";
	पक्ष ABMON_9:
		प्रत्यागम "Sep";
	पक्ष ABMON_10:
		प्रत्यागम "Oct";
	पक्ष ABMON_11:
		प्रत्यागम "Nov";
	पक्ष ABMON_12:
		प्रत्यागम "Dec";
	पक्ष ALT_DIGITS:
		प्रत्यागम "\0\0\0\0\0\0\0\0\0\0";
	पक्ष CRNCYSTR:
		प्रत्यागम "-";
	पक्ष YESEXPR:
		प्रत्यागम "^[yY]";
	पक्ष NOEXPR:
		प्रत्यागम "^[nN]";
	स्वभाव:
		प्रत्यागम "";
	}
}

