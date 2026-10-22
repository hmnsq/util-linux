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
#అన్తర్భావ "nls.h"

అక్షరం *langinfo_fallback(nl_అంశం అంశం)
{
	ఎంపిక (అంశం) {
	స్థితి CODESET:
		తిరిగి "ISO-8859-1";
	స్థితి THOUSEP:
		తిరిగి ",";
	స్థితి D_T_FMT:
	స్థితి ERA_D_T_FMT:
		తిరిగి "%a %b %e %H:%M:%S %Y";
	స్థితి D_FMT:
	స్థితి ERA_D_FMT:
		తిరిగి "%m/%d/%y";
	స్థితి T_FMT:
	స్థితి ERA_T_FMT:
		తిరిగి "%H:%M:%S";
	స్థితి T_FMT_AMPM:
		తిరిగి "%I:%M:%S %p";
	స్థితి AM_STR:
		తిరిగి "AM";
	స్థితి PM_STR:
		తిరిగి "PM";
	స్థితి DAY_1:
		తిరిగి "Sunday";
	స్థితి DAY_2:
		తిరిగి "Monday";
	స్థితి DAY_3:
		తిరిగి "Tuesday";
	స్థితి DAY_4:
		తిరిగి "Wednesday";
	స్థితి DAY_5:
		తిరిగి "Thursday";
	స్థితి DAY_6:
		తిరిగి "Friday";
	స్థితి DAY_7:
		తిరిగి "Saturday";
	స్థితి ABDAY_1:
		తిరిగి "Sun";
	స్థితి ABDAY_2:
		తిరిగి "Mon";
	స్థితి ABDAY_3:
		తిరిగి "Tue";
	స్థితి ABDAY_4:
		తిరిగి "Wed";
	స్థితి ABDAY_5:
		తిరిగి "Thu";
	స్థితి ABDAY_6:
		తిరిగి "Fri";
	స్థితి ABDAY_7:
		తిరిగి "Sat";
	స్థితి MON_1:
		తిరిగి "January";
	స్థితి MON_2:
		తిరిగి "February";
	స్థితి MON_3:
		తిరిగి "March";
	స్థితి MON_4:
		తిరిగి "April";
	స్థితి MON_5:
		తిరిగి "May";
	స్థితి MON_6:
		తిరిగి "June";
	స్థితి MON_7:
		తిరిగి "July";
	స్థితి MON_8:
		తిరిగి "August";
	స్థితి MON_9:
		తిరిగి "September";
	స్థితి MON_10:
		తిరిగి "October";
	స్థితి MON_11:
		తిరిగి "November";
	స్థితి MON_12:
		తిరిగి "December";
	స్థితి ABMON_1:
		తిరిగి "Jan";
	స్థితి ABMON_2:
		తిరిగి "Feb";
	స్థితి ABMON_3:
		తిరిగి "Mar";
	స్థితి ABMON_4:
		తిరిగి "Apr";
	స్థితి ABMON_5:
		తిరిగి "May";
	స్థితి ABMON_6:
		తిరిగి "Jun";
	స్థితి ABMON_7:
		తిరిగి "Jul";
	స్థితి ABMON_8:
		తిరిగి "Aug";
	స్థితి ABMON_9:
		తిరిగి "Sep";
	స్థితి ABMON_10:
		తిరిగి "Oct";
	స్థితి ABMON_11:
		తిరిగి "Nov";
	స్థితి ABMON_12:
		తిరిగి "Dec";
	స్థితి ALT_DIGITS:
		తిరిగి "\0\0\0\0\0\0\0\0\0\0";
	స్థితి CRNCYSTR:
		తిరిగి "-";
	స్థితి YESEXPR:
		తిరిగి "^[yY]";
	స్థితి NOEXPR:
		తిరిగి "^[nN]";
	అప్రమేయ:
		తిరిగి "";
	}
}

