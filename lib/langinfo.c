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
#ಅನ್ತರ್ಭಾವ "nls.h"

ಅಕ್ಷರ *langinfo_fallback(nl_ಅಂಶ ಅಂಶ)
{
	ಆಯ್ಕೆ (ಅಂಶ) {
	ಸ್ಥಿತಿ CODESET:
		ಹಿಂತಿರುಗಿ "ISO-8859-1";
	ಸ್ಥಿತಿ THOUSEP:
		ಹಿಂತಿರುಗಿ ",";
	ಸ್ಥಿತಿ D_T_FMT:
	ಸ್ಥಿತಿ ERA_D_T_FMT:
		ಹಿಂತಿರುಗಿ "%a %b %e %H:%M:%S %Y";
	ಸ್ಥಿತಿ D_FMT:
	ಸ್ಥಿತಿ ERA_D_FMT:
		ಹಿಂತಿರುಗಿ "%m/%d/%y";
	ಸ್ಥಿತಿ T_FMT:
	ಸ್ಥಿತಿ ERA_T_FMT:
		ಹಿಂತಿರುಗಿ "%H:%M:%S";
	ಸ್ಥಿತಿ T_FMT_AMPM:
		ಹಿಂತಿರುಗಿ "%I:%M:%S %p";
	ಸ್ಥಿತಿ AM_STR:
		ಹಿಂತಿರುಗಿ "AM";
	ಸ್ಥಿತಿ PM_STR:
		ಹಿಂತಿರುಗಿ "PM";
	ಸ್ಥಿತಿ DAY_1:
		ಹಿಂತಿರುಗಿ "Sunday";
	ಸ್ಥಿತಿ DAY_2:
		ಹಿಂತಿರುಗಿ "Monday";
	ಸ್ಥಿತಿ DAY_3:
		ಹಿಂತಿರುಗಿ "Tuesday";
	ಸ್ಥಿತಿ DAY_4:
		ಹಿಂತಿರುಗಿ "Wednesday";
	ಸ್ಥಿತಿ DAY_5:
		ಹಿಂತಿರುಗಿ "Thursday";
	ಸ್ಥಿತಿ DAY_6:
		ಹಿಂತಿರುಗಿ "Friday";
	ಸ್ಥಿತಿ DAY_7:
		ಹಿಂತಿರುಗಿ "Saturday";
	ಸ್ಥಿತಿ ABDAY_1:
		ಹಿಂತಿರುಗಿ "Sun";
	ಸ್ಥಿತಿ ABDAY_2:
		ಹಿಂತಿರುಗಿ "Mon";
	ಸ್ಥಿತಿ ABDAY_3:
		ಹಿಂತಿರುಗಿ "Tue";
	ಸ್ಥಿತಿ ABDAY_4:
		ಹಿಂತಿರುಗಿ "Wed";
	ಸ್ಥಿತಿ ABDAY_5:
		ಹಿಂತಿರುಗಿ "Thu";
	ಸ್ಥಿತಿ ABDAY_6:
		ಹಿಂತಿರುಗಿ "Fri";
	ಸ್ಥಿತಿ ABDAY_7:
		ಹಿಂತಿರುಗಿ "Sat";
	ಸ್ಥಿತಿ MON_1:
		ಹಿಂತಿರುಗಿ "January";
	ಸ್ಥಿತಿ MON_2:
		ಹಿಂತಿರುಗಿ "February";
	ಸ್ಥಿತಿ MON_3:
		ಹಿಂತಿರುಗಿ "March";
	ಸ್ಥಿತಿ MON_4:
		ಹಿಂತಿರುಗಿ "April";
	ಸ್ಥಿತಿ MON_5:
		ಹಿಂತಿರುಗಿ "May";
	ಸ್ಥಿತಿ MON_6:
		ಹಿಂತಿರುಗಿ "June";
	ಸ್ಥಿತಿ MON_7:
		ಹಿಂತಿರುಗಿ "July";
	ಸ್ಥಿತಿ MON_8:
		ಹಿಂತಿರುಗಿ "August";
	ಸ್ಥಿತಿ MON_9:
		ಹಿಂತಿರುಗಿ "September";
	ಸ್ಥಿತಿ MON_10:
		ಹಿಂತಿರುಗಿ "October";
	ಸ್ಥಿತಿ MON_11:
		ಹಿಂತಿರುಗಿ "November";
	ಸ್ಥಿತಿ MON_12:
		ಹಿಂತಿರುಗಿ "December";
	ಸ್ಥಿತಿ ABMON_1:
		ಹಿಂತಿರುಗಿ "Jan";
	ಸ್ಥಿತಿ ABMON_2:
		ಹಿಂತಿರುಗಿ "Feb";
	ಸ್ಥಿತಿ ABMON_3:
		ಹಿಂತಿರುಗಿ "Mar";
	ಸ್ಥಿತಿ ABMON_4:
		ಹಿಂತಿರುಗಿ "Apr";
	ಸ್ಥಿತಿ ABMON_5:
		ಹಿಂತಿರುಗಿ "May";
	ಸ್ಥಿತಿ ABMON_6:
		ಹಿಂತಿರುಗಿ "Jun";
	ಸ್ಥಿತಿ ABMON_7:
		ಹಿಂತಿರುಗಿ "Jul";
	ಸ್ಥಿತಿ ABMON_8:
		ಹಿಂತಿರುಗಿ "Aug";
	ಸ್ಥಿತಿ ABMON_9:
		ಹಿಂತಿರುಗಿ "Sep";
	ಸ್ಥಿತಿ ABMON_10:
		ಹಿಂತಿರುಗಿ "Oct";
	ಸ್ಥಿತಿ ABMON_11:
		ಹಿಂತಿರುಗಿ "Nov";
	ಸ್ಥಿತಿ ABMON_12:
		ಹಿಂತಿರುಗಿ "Dec";
	ಸ್ಥಿತಿ ALT_DIGITS:
		ಹಿಂತಿರುಗಿ "\0\0\0\0\0\0\0\0\0\0";
	ಸ್ಥಿತಿ CRNCYSTR:
		ಹಿಂತಿರುಗಿ "-";
	ಸ್ಥಿತಿ YESEXPR:
		ಹಿಂತಿರುಗಿ "^[yY]";
	ಸ್ಥಿತಿ NOEXPR:
		ಹಿಂತಿರುಗಿ "^[nN]";
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಹಿಂತಿರುಗಿ "";
	}
}

