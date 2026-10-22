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
		फर्काऊ "ISO-8859-1";
	पक्ष THOUSEP:
		फर्काऊ ",";
	पक्ष D_T_FMT:
	पक्ष ERA_D_T_FMT:
		फर्काऊ "%a %b %e %H:%M:%S %Y";
	पक्ष D_FMT:
	पक्ष ERA_D_FMT:
		फर्काऊ "%m/%d/%y";
	पक्ष T_FMT:
	पक्ष ERA_T_FMT:
		फर्काऊ "%H:%M:%S";
	पक्ष T_FMT_AMPM:
		फर्काऊ "%I:%M:%S %p";
	पक्ष AM_STR:
		फर्काऊ "AM";
	पक्ष PM_STR:
		फर्काऊ "PM";
	पक्ष DAY_1:
		फर्काऊ "Sunday";
	पक्ष DAY_2:
		फर्काऊ "Monday";
	पक्ष DAY_3:
		फर्काऊ "Tuesday";
	पक्ष DAY_4:
		फर्काऊ "Wednesday";
	पक्ष DAY_5:
		फर्काऊ "Thursday";
	पक्ष DAY_6:
		फर्काऊ "Friday";
	पक्ष DAY_7:
		फर्काऊ "Saturday";
	पक्ष ABDAY_1:
		फर्काऊ "Sun";
	पक्ष ABDAY_2:
		फर्काऊ "Mon";
	पक्ष ABDAY_3:
		फर्काऊ "Tue";
	पक्ष ABDAY_4:
		फर्काऊ "Wed";
	पक्ष ABDAY_5:
		फर्काऊ "Thu";
	पक्ष ABDAY_6:
		फर्काऊ "Fri";
	पक्ष ABDAY_7:
		फर्काऊ "Sat";
	पक्ष MON_1:
		फर्काऊ "January";
	पक्ष MON_2:
		फर्काऊ "February";
	पक्ष MON_3:
		फर्काऊ "March";
	पक्ष MON_4:
		फर्काऊ "April";
	पक्ष MON_5:
		फर्काऊ "May";
	पक्ष MON_6:
		फर्काऊ "June";
	पक्ष MON_7:
		फर्काऊ "July";
	पक्ष MON_8:
		फर्काऊ "August";
	पक्ष MON_9:
		फर्काऊ "September";
	पक्ष MON_10:
		फर्काऊ "October";
	पक्ष MON_11:
		फर्काऊ "November";
	पक्ष MON_12:
		फर्काऊ "December";
	पक्ष ABMON_1:
		फर्काऊ "Jan";
	पक्ष ABMON_2:
		फर्काऊ "Feb";
	पक्ष ABMON_3:
		फर्काऊ "Mar";
	पक्ष ABMON_4:
		फर्काऊ "Apr";
	पक्ष ABMON_5:
		फर्काऊ "May";
	पक्ष ABMON_6:
		फर्काऊ "Jun";
	पक्ष ABMON_7:
		फर्काऊ "Jul";
	पक्ष ABMON_8:
		फर्काऊ "Aug";
	पक्ष ABMON_9:
		फर्काऊ "Sep";
	पक्ष ABMON_10:
		फर्काऊ "Oct";
	पक्ष ABMON_11:
		फर्काऊ "Nov";
	पक्ष ABMON_12:
		फर्काऊ "Dec";
	पक्ष ALT_DIGITS:
		फर्काऊ "\0\0\0\0\0\0\0\0\0\0";
	पक्ष CRNCYSTR:
		फर्काऊ "-";
	पक्ष YESEXPR:
		फर्काऊ "^[yY]";
	पक्ष NOEXPR:
		फर्काऊ "^[nN]";
	स्वभाव:
		फर्काऊ "";
	}
}

