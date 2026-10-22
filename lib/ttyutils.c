/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <unistd.h>

#నిర్వచితమైతే HAVE_GETTTYNAM
# include <ttyent.h>
#అయితే_ముగింపు

#అన్తర్భావ "c.h"
#అన్తర్భావ "ttyutils.h"

#నిర్వచితమైతే __linux__
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "linux"
#  endif
#  ఒకవేళ defined (__s390__) || defined (__s390x__)
#    define DEFAULT_TTYS0  "dumb"
#    define DEFAULT_TTY32  "ibm327x"
#    define DEFAULT_TTYS1  "vt220"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#లేదావేళ defined(__GNU__)
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "hurd"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#లేకపోతే
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#అయితే_ముగింపు

స్థిర పూర్ణాంకం get_env_int(స్థిరరాశి అక్షరం *name)
{
	స్థిరరాశి అక్షరం *cp = getenv(name);

	ఒకవేళ (cp) {
		అక్షరం *చివర = ఖాళీ;
		దీర్ఘ x;

		errno = 0;
		x = strtol(cp, &చివర, 10);

		ఒకవేళ (errno == 0 && చివర && *చివర == '\0' && చివర > cp &&
		    x > 0 && x <= గరిష్ఠ_పూర్ణాంకం)
			తిరిగి x;
	}

	తిరిగి -1;
}

పూర్ణాంకం get_terminal_dimension(పూర్ణాంకం *నిలువుల_సంఖ్య, పూర్ణాంకం *పంక్తులు)
{
	పూర్ణాంకం c = 0, l = 0;

#ఒకవేళ defined(TIOCGWINSZ)
	నిర్మాణం winsize	w_win;
	ఒకవేళ (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#లేదావేళ defined(TIOCGSIZE)
	నిర్మాణం ttysize	t_win;
	ఒకవేళ (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_నిలువుల_సంఖ్య;
		l = t_win.ts_పంక్తులు;
	}
#అయితే_ముగింపు
	ఒకవేళ (నిలువుల_సంఖ్య) {
		ఒకవేళ (c <= 0)
			c = get_env_int("COLUMNS");
		*నిలువుల_సంఖ్య = c;
	}
	ఒకవేళ (పంక్తులు) {
		ఒకవేళ (l <= 0)
			l = get_env_int("LINES");
		*పంక్తులు = l;
	}
	తిరిగి 0;
}

పూర్ణాంకం get_terminal_వెడల్పు(పూర్ణాంకం default_వెడల్పు)
{
	పూర్ణాంకం వెడల్పు = 0;

	get_terminal_dimension(&వెడల్పు, ఖాళీ);

	తిరిగి వెడల్పు > 0 ? వెడల్పు : default_వెడల్పు;
}

పూర్ణాంకం get_terminal_stdfd(శూన్య)
{
	ఒకవేళ (isatty(STDIN_FILENO))
		తిరిగి STDIN_FILENO;
	ఒకవేళ (isatty(STDOUT_FILENO))
		తిరిగి STDOUT_FILENO;
	ఒకవేళ (isatty(STDERR_FILENO))
		తిరిగి STDERR_FILENO;

	తిరిగి -EINVAL;
}

పూర్ణాంకం get_terminal_name(స్థిరరాశి అక్షరం **మార్గం,
		      స్థిరరాశి అక్షరం **name,
		      స్థిరరాశి అక్షరం **సంఖ్య)
{
	స్థిరరాశి అక్షరం *tty;
	స్థిరరాశి అక్షరం *p;
	పూర్ణాంకం fd;


	ఒకవేళ (name)
		*name = ఖాళీ;
	ఒకవేళ (మార్గం)
		*మార్గం = ఖాళీ;
	ఒకవేళ (సంఖ్య)
		*సంఖ్య = ఖాళీ;

	fd = get_terminal_stdfd();
	ఒకవేళ (fd < 0)
		తిరిగి fd;	/* error */

	tty = ttyname(fd);
	ఒకవేళ (!tty)
		తిరిగి -1;

	ఒకవేళ (మార్గం)
		*మార్గం = tty;
	ఒకవేళ (name || సంఖ్య)
		tty = అక్షరశ్రేణి_పోలిక(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	ఒకవేళ (name)
		*name = tty;
	ఒకవేళ (సంఖ్య) {
		కొరకు (p = tty; p && *p; p++) {
			ఒకవేళ (అంకెనా(*p)) {
				*సంఖ్య = p;
				ఆపు;
			}
		}
	}
	తిరిగి 0;
}

పూర్ణాంకం get_terminal_type(స్థిరరాశి అక్షరం **type)
{
	*type = getenv("TERM");
	ఒకవేళ (*type)
		తిరిగి -EINVAL;
	తిరిగి 0;
}

అక్షరం *get_terminal_default_type(స్థిరరాశి అక్షరం *ttyname, పూర్ణాంకం is_serial)
{
	ఒకవేళ (ttyname) {
#నిర్వచితమైతే HAVE_GETTTYNAM
		నిర్మాణం ttyent *ent = getttynam(ttyname);

		ఒకవేళ (ent && ent->ty_type)
			తిరిగి strdup(ent->ty_type);
#అయితే_ముగింపు

#ఒకవేళ defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		ఒకవేళ (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			తిరిగి strdup(DEFAULT_TTYS0);
		లేకపోతే ఒకవేళ (అక్షరశ్రేణి_పోలిక(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			తిరిగి strdup(DEFAULT_TTY32);
		లేకపోతే ఒకవేళ (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			తిరిగి strdup(DEFAULT_TTYS1);
#అయితే_ముగింపు
	}

	తిరిగి strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#నిర్వచితమైతే TEST_కార్యక్రమం_TTYUTILS
# include <stdlib.h>
పూర్ణాంకం ప్రధాన(శూన్య)
{
	స్థిరరాశి అక్షరం *మార్గం, *name, *అంకె;
	పూర్ణాంకం c, l;

	ఒకవేళ (get_terminal_name(&మార్గం, &name, &అంకె) == 0) {
		అక్షరం *పదం;

		fprintf(stderr, "tty path:      %s\n", మార్గం);
		fprintf(stderr, "tty name:      %s\n", name);
		fprintf(stderr, "tty number:    %s\n", అంకె);

		fprintf(stderr, "tty term:      %s\n", getenv("TERM"));

		పదం = get_terminal_default_type(name, 0);
		fprintf(stderr, "tty dflt term: %s\n", పదం);
		విడుదల(పదం);
	}
	get_terminal_dimension(&c, &l);
	fprintf(stderr,         "tty cols:      %d\n", c);
	fprintf(stderr,         "tty lines:     %d\n", l);


	తిరిగి EXIT_SUCCESS;
}
#అయితే_ముగింపు /* TEST_PROGRAM_TTYUTILS */
