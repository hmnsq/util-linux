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
#ਅਨ੍ਤਰ੍ਭਾਵ "nls.h"

ਅੱਖਰ *langinfo_fallback(nl_ਵਸਤੂ ਵਸਤੂ)
{
	ਚੋਣ (ਵਸਤੂ) {
	ਹਾਲਤ CODESET:
		ਵਾਪਸ "ISO-8859-1";
	ਹਾਲਤ THOUSEP:
		ਵਾਪਸ ",";
	ਹਾਲਤ D_T_FMT:
	ਹਾਲਤ ERA_D_T_FMT:
		ਵਾਪਸ "%a %b %e %H:%M:%S %Y";
	ਹਾਲਤ D_FMT:
	ਹਾਲਤ ERA_D_FMT:
		ਵਾਪਸ "%m/%d/%y";
	ਹਾਲਤ T_FMT:
	ਹਾਲਤ ERA_T_FMT:
		ਵਾਪਸ "%H:%M:%S";
	ਹਾਲਤ T_FMT_AMPM:
		ਵਾਪਸ "%I:%M:%S %p";
	ਹਾਲਤ AM_STR:
		ਵਾਪਸ "AM";
	ਹਾਲਤ PM_STR:
		ਵਾਪਸ "PM";
	ਹਾਲਤ DAY_1:
		ਵਾਪਸ "Sunday";
	ਹਾਲਤ DAY_2:
		ਵਾਪਸ "Monday";
	ਹਾਲਤ DAY_3:
		ਵਾਪਸ "Tuesday";
	ਹਾਲਤ DAY_4:
		ਵਾਪਸ "Wednesday";
	ਹਾਲਤ DAY_5:
		ਵਾਪਸ "Thursday";
	ਹਾਲਤ DAY_6:
		ਵਾਪਸ "Friday";
	ਹਾਲਤ DAY_7:
		ਵਾਪਸ "Saturday";
	ਹਾਲਤ ABDAY_1:
		ਵਾਪਸ "Sun";
	ਹਾਲਤ ABDAY_2:
		ਵਾਪਸ "Mon";
	ਹਾਲਤ ABDAY_3:
		ਵਾਪਸ "Tue";
	ਹਾਲਤ ABDAY_4:
		ਵਾਪਸ "Wed";
	ਹਾਲਤ ABDAY_5:
		ਵਾਪਸ "Thu";
	ਹਾਲਤ ABDAY_6:
		ਵਾਪਸ "Fri";
	ਹਾਲਤ ABDAY_7:
		ਵਾਪਸ "Sat";
	ਹਾਲਤ MON_1:
		ਵਾਪਸ "January";
	ਹਾਲਤ MON_2:
		ਵਾਪਸ "February";
	ਹਾਲਤ MON_3:
		ਵਾਪਸ "March";
	ਹਾਲਤ MON_4:
		ਵਾਪਸ "April";
	ਹਾਲਤ MON_5:
		ਵਾਪਸ "May";
	ਹਾਲਤ MON_6:
		ਵਾਪਸ "June";
	ਹਾਲਤ MON_7:
		ਵਾਪਸ "July";
	ਹਾਲਤ MON_8:
		ਵਾਪਸ "August";
	ਹਾਲਤ MON_9:
		ਵਾਪਸ "September";
	ਹਾਲਤ MON_10:
		ਵਾਪਸ "October";
	ਹਾਲਤ MON_11:
		ਵਾਪਸ "November";
	ਹਾਲਤ MON_12:
		ਵਾਪਸ "December";
	ਹਾਲਤ ABMON_1:
		ਵਾਪਸ "Jan";
	ਹਾਲਤ ABMON_2:
		ਵਾਪਸ "Feb";
	ਹਾਲਤ ABMON_3:
		ਵਾਪਸ "Mar";
	ਹਾਲਤ ABMON_4:
		ਵਾਪਸ "Apr";
	ਹਾਲਤ ABMON_5:
		ਵਾਪਸ "May";
	ਹਾਲਤ ABMON_6:
		ਵਾਪਸ "Jun";
	ਹਾਲਤ ABMON_7:
		ਵਾਪਸ "Jul";
	ਹਾਲਤ ABMON_8:
		ਵਾਪਸ "Aug";
	ਹਾਲਤ ABMON_9:
		ਵਾਪਸ "Sep";
	ਹਾਲਤ ABMON_10:
		ਵਾਪਸ "Oct";
	ਹਾਲਤ ABMON_11:
		ਵਾਪਸ "Nov";
	ਹਾਲਤ ABMON_12:
		ਵਾਪਸ "Dec";
	ਹਾਲਤ ALT_DIGITS:
		ਵਾਪਸ "\0\0\0\0\0\0\0\0\0\0";
	ਹਾਲਤ CRNCYSTR:
		ਵਾਪਸ "-";
	ਹਾਲਤ YESEXPR:
		ਵਾਪਸ "^[yY]";
	ਹਾਲਤ NOEXPR:
		ਵਾਪਸ "^[nN]";
	ਮੂਲ:
		ਵਾਪਸ "";
	}
}

