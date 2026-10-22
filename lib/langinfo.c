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
#അന്തര്ഭാവ "nls.h"

അക്ഷരം *langinfo_fallback(nl_item item)
{
	തിരഞ്ഞെടുപ്പ് (item) {
	അവസ്ഥ CODESET:
		തിരികെ "ISO-8859-1";
	അവസ്ഥ THOUSEP:
		തിരികെ ",";
	അവസ്ഥ D_T_FMT:
	അവസ്ഥ ERA_D_T_FMT:
		തിരികെ "%a %b %e %H:%M:%S %Y";
	അവസ്ഥ D_FMT:
	അവസ്ഥ ERA_D_FMT:
		തിരികെ "%m/%d/%y";
	അവസ്ഥ T_FMT:
	അവസ്ഥ ERA_T_FMT:
		തിരികെ "%H:%M:%S";
	അവസ്ഥ T_FMT_AMPM:
		തിരികെ "%I:%M:%S %p";
	അവസ്ഥ AM_STR:
		തിരികെ "AM";
	അവസ്ഥ PM_STR:
		തിരികെ "PM";
	അവസ്ഥ DAY_1:
		തിരികെ "Sunday";
	അവസ്ഥ DAY_2:
		തിരികെ "Monday";
	അവസ്ഥ DAY_3:
		തിരികെ "Tuesday";
	അവസ്ഥ DAY_4:
		തിരികെ "Wednesday";
	അവസ്ഥ DAY_5:
		തിരികെ "Thursday";
	അവസ്ഥ DAY_6:
		തിരികെ "Friday";
	അവസ്ഥ DAY_7:
		തിരികെ "Saturday";
	അവസ്ഥ ABDAY_1:
		തിരികെ "Sun";
	അവസ്ഥ ABDAY_2:
		തിരികെ "Mon";
	അവസ്ഥ ABDAY_3:
		തിരികെ "Tue";
	അവസ്ഥ ABDAY_4:
		തിരികെ "Wed";
	അവസ്ഥ ABDAY_5:
		തിരികെ "Thu";
	അവസ്ഥ ABDAY_6:
		തിരികെ "Fri";
	അവസ്ഥ ABDAY_7:
		തിരികെ "Sat";
	അവസ്ഥ MON_1:
		തിരികെ "January";
	അവസ്ഥ MON_2:
		തിരികെ "February";
	അവസ്ഥ MON_3:
		തിരികെ "March";
	അവസ്ഥ MON_4:
		തിരികെ "April";
	അവസ്ഥ MON_5:
		തിരികെ "May";
	അവസ്ഥ MON_6:
		തിരികെ "June";
	അവസ്ഥ MON_7:
		തിരികെ "July";
	അവസ്ഥ MON_8:
		തിരികെ "August";
	അവസ്ഥ MON_9:
		തിരികെ "September";
	അവസ്ഥ MON_10:
		തിരികെ "October";
	അവസ്ഥ MON_11:
		തിരികെ "November";
	അവസ്ഥ MON_12:
		തിരികെ "December";
	അവസ്ഥ ABMON_1:
		തിരികെ "Jan";
	അവസ്ഥ ABMON_2:
		തിരികെ "Feb";
	അവസ്ഥ ABMON_3:
		തിരികെ "Mar";
	അവസ്ഥ ABMON_4:
		തിരികെ "Apr";
	അവസ്ഥ ABMON_5:
		തിരികെ "May";
	അവസ്ഥ ABMON_6:
		തിരികെ "Jun";
	അവസ്ഥ ABMON_7:
		തിരികെ "Jul";
	അവസ്ഥ ABMON_8:
		തിരികെ "Aug";
	അവസ്ഥ ABMON_9:
		തിരികെ "Sep";
	അവസ്ഥ ABMON_10:
		തിരികെ "Oct";
	അവസ്ഥ ABMON_11:
		തിരികെ "Nov";
	അവസ്ഥ ABMON_12:
		തിരികെ "Dec";
	അവസ്ഥ ALT_DIGITS:
		തിരികെ "\0\0\0\0\0\0\0\0\0\0";
	അവസ്ഥ CRNCYSTR:
		തിരികെ "-";
	അവസ്ഥ YESEXPR:
		തിരികെ "^[yY]";
	അവസ്ഥ NOEXPR:
		തിരികെ "^[nN]";
	സ്ഥിരസ്ഥിതി:
		തിരികെ "";
	}
}

