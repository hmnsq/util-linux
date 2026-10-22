/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#অন্তর্ভাৱ <ctype.h>
#অন্তর্ভাৱ <unistd.h>

#ifdef HAVE_GETTTYNAM
# include <ttyent.h>
#endif

#অন্তর্ভাৱ "c.h"
#অন্তর্ভাৱ "ttyutils.h"

#ifdef __linux__
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "linux"
#  endif
#  যদি defined (__s390__) || defined (__s390x__)
#    define DEFAULT_TTYS0  "dumb"
#    define DEFAULT_TTY32  "ibm327x"
#    define DEFAULT_TTYS1  "vt220"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#নোচেদ্যদি defined(__GNU__)
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "hurd"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#অন্যথা
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#endif

স্থির পূর্ণাঙ্ক get_env_int(ধ্রুৱ ৱর্ণ *name)
{
	ধ্রুৱ ৱর্ণ *cp = getenv(name);

	যদি (cp) {
		ৱর্ণ *end = রিক্ত;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		যদি (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			ঘূৰাই x;
	}

	ঘূৰাই -1;
}

পূর্ণাঙ্ক get_terminal_dimension(পূর্ণাঙ্ক *cols, পূর্ণাঙ্ক *সাৰিগুলি)
{
	পূর্ণাঙ্ক c = 0, l = 0;

#যদি defined(TIOCGWINSZ)
	সংরচনা winsize	w_win;
	যদি (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#নোচেদ্যদি defined(TIOCGSIZE)
	সংরচনা ttysize	t_win;
	যদি (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_সাৰিগুলি;
	}
#endif
	যদি (cols) {
		যদি (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	যদি (সাৰিগুলি) {
		যদি (l <= 0)
			l = get_env_int("LINES");
		*সাৰিগুলি = l;
	}
	ঘূৰাই 0;
}

পূর্ণাঙ্ক get_terminal_width(পূর্ণাঙ্ক default_width)
{
	পূর্ণাঙ্ক width = 0;

	get_terminal_dimension(&width, রিক্ত);

	ঘূৰাই width > 0 ? width : default_width;
}

পূর্ণাঙ্ক get_terminal_stdfd(শূন্য)
{
	যদি (isatty(STDIN_FILENO))
		ঘূৰাই STDIN_FILENO;
	যদি (isatty(STDOUT_FILENO))
		ঘূৰাই STDOUT_FILENO;
	যদি (isatty(STDERR_FILENO))
		ঘূৰাই STDERR_FILENO;

	ঘূৰাই -EINVAL;
}

পূর্ণাঙ্ক get_terminal_name(ধ্রুৱ ৱর্ণ **path,
		      ধ্রুৱ ৱর্ণ **name,
		      ধ্রুৱ ৱর্ণ **সংখ্যা)
{
	ধ্রুৱ ৱর্ণ *tty;
	ধ্রুৱ ৱর্ণ *p;
	পূর্ণাঙ্ক fd;


	যদি (name)
		*name = রিক্ত;
	যদি (path)
		*path = রিক্ত;
	যদি (সংখ্যা)
		*সংখ্যা = রিক্ত;

	fd = get_terminal_stdfd();
	যদি (fd < 0)
		ঘূৰাই fd;	/* error */

	tty = ttyname(fd);
	যদি (!tty)
		ঘূৰাই -1;

	যদি (path)
		*path = tty;
	যদি (name || সংখ্যা)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	যদি (name)
		*name = tty;
	যদি (সংখ্যা) {
		বাবে (p = tty; p && *p; p++) {
			যদি (isdigit(*p)) {
				*সংখ্যা = p;
				ভঙ্গ;
			}
		}
	}
	ঘূৰাই 0;
}

পূর্ণাঙ্ক get_terminal_type(ধ্রুৱ ৱর্ণ **type)
{
	*type = getenv("TERM");
	যদি (*type)
		ঘূৰাই -EINVAL;
	ঘূৰাই 0;
}

ৱর্ণ *get_terminal_default_type(ধ্রুৱ ৱর্ণ *ttyname, পূর্ণাঙ্ক is_serial)
{
	যদি (ttyname) {
#ifdef HAVE_GETTTYNAM
		সংরচনা ttyent *ent = getttynam(ttyname);

		যদি (ent && ent->ty_type)
			ঘূৰাই strdup(ent->ty_type);
#endif

#যদি defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		যদি (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			ঘূৰাই strdup(DEFAULT_TTYS0);
		অন্যথা যদি (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			ঘূৰাই strdup(DEFAULT_TTY32);
		অন্যথা যদি (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			ঘূৰাই strdup(DEFAULT_TTYS1);
#endif
	}

	ঘূৰাই strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
পূর্ণাঙ্ক main(শূন্য)
{
	ধ্রুৱ ৱর্ণ *path, *name, *অঙ্ক;
	পূর্ণাঙ্ক c, l;

	যদি (get_terminal_name(&path, &name, &অঙ্ক) == 0) {
		ৱর্ণ *term;

		fprintf(stderr, "tty path:      %s\n", path);
		fprintf(stderr, "tty name:      %s\n", name);
		fprintf(stderr, "tty number:    %s\n", অঙ্ক);

		fprintf(stderr, "tty term:      %s\n", getenv("TERM"));

		term = get_terminal_default_type(name, 0);
		fprintf(stderr, "tty dflt term: %s\n", term);
		মুক্ত(term);
	}
	get_terminal_dimension(&c, &l);
	fprintf(stderr,         "tty cols:      %d\n", c);
	fprintf(stderr,         "tty lines:     %d\n", l);


	ঘূৰাই EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
