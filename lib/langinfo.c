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

ವರ್ಣ *langinfo_fallback(nl_item item)
{
	ವಿಕಲ್ಪ (item) {
	ಪಕ್ಷ CODESET:
		ಪ್ರತ್ಯಾಗಮ "ISO-8859-1";
	ಪಕ್ಷ THOUSEP:
		ಪ್ರತ್ಯಾಗಮ ",";
	ಪಕ್ಷ D_T_FMT:
	ಪಕ್ಷ ERA_D_T_FMT:
		ಪ್ರತ್ಯಾಗಮ "%a %b %e %H:%M:%S %Y";
	ಪಕ್ಷ D_FMT:
	ಪಕ್ಷ ERA_D_FMT:
		ಪ್ರತ್ಯಾಗಮ "%m/%d/%y";
	ಪಕ್ಷ T_FMT:
	ಪಕ್ಷ ERA_T_FMT:
		ಪ್ರತ್ಯಾಗಮ "%H:%M:%S";
	ಪಕ್ಷ T_FMT_AMPM:
		ಪ್ರತ್ಯಾಗಮ "%I:%M:%S %p";
	ಪಕ್ಷ AM_STR:
		ಪ್ರತ್ಯಾಗಮ "AM";
	ಪಕ್ಷ PM_STR:
		ಪ್ರತ್ಯಾಗಮ "PM";
	ಪಕ್ಷ DAY_1:
		ಪ್ರತ್ಯಾಗಮ "Sunday";
	ಪಕ್ಷ DAY_2:
		ಪ್ರತ್ಯಾಗಮ "Monday";
	ಪಕ್ಷ DAY_3:
		ಪ್ರತ್ಯಾಗಮ "Tuesday";
	ಪಕ್ಷ DAY_4:
		ಪ್ರತ್ಯಾಗಮ "Wednesday";
	ಪಕ್ಷ DAY_5:
		ಪ್ರತ್ಯಾಗಮ "Thursday";
	ಪಕ್ಷ DAY_6:
		ಪ್ರತ್ಯಾಗಮ "Friday";
	ಪಕ್ಷ DAY_7:
		ಪ್ರತ್ಯಾಗಮ "Saturday";
	ಪಕ್ಷ ABDAY_1:
		ಪ್ರತ್ಯಾಗಮ "Sun";
	ಪಕ್ಷ ABDAY_2:
		ಪ್ರತ್ಯಾಗಮ "Mon";
	ಪಕ್ಷ ABDAY_3:
		ಪ್ರತ್ಯಾಗಮ "Tue";
	ಪಕ್ಷ ABDAY_4:
		ಪ್ರತ್ಯಾಗಮ "Wed";
	ಪಕ್ಷ ABDAY_5:
		ಪ್ರತ್ಯಾಗಮ "Thu";
	ಪಕ್ಷ ABDAY_6:
		ಪ್ರತ್ಯಾಗಮ "Fri";
	ಪಕ್ಷ ABDAY_7:
		ಪ್ರತ್ಯಾಗಮ "Sat";
	ಪಕ್ಷ MON_1:
		ಪ್ರತ್ಯಾಗಮ "January";
	ಪಕ್ಷ MON_2:
		ಪ್ರತ್ಯಾಗಮ "February";
	ಪಕ್ಷ MON_3:
		ಪ್ರತ್ಯಾಗಮ "March";
	ಪಕ್ಷ MON_4:
		ಪ್ರತ್ಯಾಗಮ "April";
	ಪಕ್ಷ MON_5:
		ಪ್ರತ್ಯಾಗಮ "May";
	ಪಕ್ಷ MON_6:
		ಪ್ರತ್ಯಾಗಮ "June";
	ಪಕ್ಷ MON_7:
		ಪ್ರತ್ಯಾಗಮ "July";
	ಪಕ್ಷ MON_8:
		ಪ್ರತ್ಯಾಗಮ "August";
	ಪಕ್ಷ MON_9:
		ಪ್ರತ್ಯಾಗಮ "September";
	ಪಕ್ಷ MON_10:
		ಪ್ರತ್ಯಾಗಮ "October";
	ಪಕ್ಷ MON_11:
		ಪ್ರತ್ಯಾಗಮ "November";
	ಪಕ್ಷ MON_12:
		ಪ್ರತ್ಯಾಗಮ "December";
	ಪಕ್ಷ ABMON_1:
		ಪ್ರತ್ಯಾಗಮ "Jan";
	ಪಕ್ಷ ABMON_2:
		ಪ್ರತ್ಯಾಗಮ "Feb";
	ಪಕ್ಷ ABMON_3:
		ಪ್ರತ್ಯಾಗಮ "Mar";
	ಪಕ್ಷ ABMON_4:
		ಪ್ರತ್ಯಾಗಮ "Apr";
	ಪಕ್ಷ ABMON_5:
		ಪ್ರತ್ಯಾಗಮ "May";
	ಪಕ್ಷ ABMON_6:
		ಪ್ರತ್ಯಾಗಮ "Jun";
	ಪಕ್ಷ ABMON_7:
		ಪ್ರತ್ಯಾಗಮ "Jul";
	ಪಕ್ಷ ABMON_8:
		ಪ್ರತ್ಯಾಗಮ "Aug";
	ಪಕ್ಷ ABMON_9:
		ಪ್ರತ್ಯಾಗಮ "Sep";
	ಪಕ್ಷ ABMON_10:
		ಪ್ರತ್ಯಾಗಮ "Oct";
	ಪಕ್ಷ ABMON_11:
		ಪ್ರತ್ಯಾಗಮ "Nov";
	ಪಕ್ಷ ABMON_12:
		ಪ್ರತ್ಯಾಗಮ "Dec";
	ಪಕ್ಷ ALT_DIGITS:
		ಪ್ರತ್ಯಾಗಮ "\0\0\0\0\0\0\0\0\0\0";
	ಪಕ್ಷ CRNCYSTR:
		ಪ್ರತ್ಯಾಗಮ "-";
	ಪಕ್ಷ YESEXPR:
		ಪ್ರತ್ಯಾಗಮ "^[yY]";
	ಪಕ್ಷ NOEXPR:
		ಪ್ರತ್ಯಾಗಮ "^[nN]";
	ಸ್ವಭಾವ:
		ಪ್ರತ್ಯಾಗಮ "";
	}
}

