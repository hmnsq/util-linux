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

अक्षर *langinfo_fallback(nl_item item)
{
	निवड (item) {
	स्थिती CODESET:
		परत "ISO-8859-1";
	स्थिती THOUSEP:
		परत ",";
	स्थिती D_T_FMT:
	स्थिती ERA_D_T_FMT:
		परत "%a %b %e %H:%M:%S %Y";
	स्थिती D_FMT:
	स्थिती ERA_D_FMT:
		परत "%m/%d/%y";
	स्थिती T_FMT:
	स्थिती ERA_T_FMT:
		परत "%H:%M:%S";
	स्थिती T_FMT_AMPM:
		परत "%I:%M:%S %p";
	स्थिती AM_STR:
		परत "AM";
	स्थिती PM_STR:
		परत "PM";
	स्थिती DAY_1:
		परत "Sunday";
	स्थिती DAY_2:
		परत "Monday";
	स्थिती DAY_3:
		परत "Tuesday";
	स्थिती DAY_4:
		परत "Wednesday";
	स्थिती DAY_5:
		परत "Thursday";
	स्थिती DAY_6:
		परत "Friday";
	स्थिती DAY_7:
		परत "Saturday";
	स्थिती ABDAY_1:
		परत "Sun";
	स्थिती ABDAY_2:
		परत "Mon";
	स्थिती ABDAY_3:
		परत "Tue";
	स्थिती ABDAY_4:
		परत "Wed";
	स्थिती ABDAY_5:
		परत "Thu";
	स्थिती ABDAY_6:
		परत "Fri";
	स्थिती ABDAY_7:
		परत "Sat";
	स्थिती MON_1:
		परत "January";
	स्थिती MON_2:
		परत "February";
	स्थिती MON_3:
		परत "March";
	स्थिती MON_4:
		परत "April";
	स्थिती MON_5:
		परत "May";
	स्थिती MON_6:
		परत "June";
	स्थिती MON_7:
		परत "July";
	स्थिती MON_8:
		परत "August";
	स्थिती MON_9:
		परत "September";
	स्थिती MON_10:
		परत "October";
	स्थिती MON_11:
		परत "November";
	स्थिती MON_12:
		परत "December";
	स्थिती ABMON_1:
		परत "Jan";
	स्थिती ABMON_2:
		परत "Feb";
	स्थिती ABMON_3:
		परत "Mar";
	स्थिती ABMON_4:
		परत "Apr";
	स्थिती ABMON_5:
		परत "May";
	स्थिती ABMON_6:
		परत "Jun";
	स्थिती ABMON_7:
		परत "Jul";
	स्थिती ABMON_8:
		परत "Aug";
	स्थिती ABMON_9:
		परत "Sep";
	स्थिती ABMON_10:
		परत "Oct";
	स्थिती ABMON_11:
		परत "Nov";
	स्थिती ABMON_12:
		परत "Dec";
	स्थिती ALT_DIGITS:
		परत "\0\0\0\0\0\0\0\0\0\0";
	स्थिती CRNCYSTR:
		परत "-";
	स्थिती YESEXPR:
		परत "^[yY]";
	स्थिती NOEXPR:
		परत "^[nN]";
	पूर्वनिर्धारित:
		परत "";
	}
}

