/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <unistd.h>

#ifdef HAVE_GETTTYNAM
# include <ttyent.h>
#endif

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "ttyutils.h"

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
#নতুবা_যদি defined(__GNU__)
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

স্থির পূর্ণসংখ্যা get_env_int(ধ্রুবক অক্ষর *name)
{
	ধ্রুবক অক্ষর *cp = getenv(name);

	যদি (cp) {
		অক্ষর *end = ফাঁকা;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		যদি (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			ফেরত x;
	}

	ফেরত -1;
}

পূর্ণসংখ্যা get_terminal_dimension(পূর্ণসংখ্যা *cols, পূর্ণসংখ্যা *সারিগুলি)
{
	পূর্ণসংখ্যা c = 0, l = 0;

#যদি defined(TIOCGWINSZ)
	কাঠামো winsize	w_win;
	যদি (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#নতুবা_যদি defined(TIOCGSIZE)
	কাঠামো ttysize	t_win;
	যদি (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_সারিগুলি;
	}
#endif
	যদি (cols) {
		যদি (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	যদি (সারিগুলি) {
		যদি (l <= 0)
			l = get_env_int("LINES");
		*সারিগুলি = l;
	}
	ফেরত 0;
}

পূর্ণসংখ্যা get_terminal_width(পূর্ণসংখ্যা default_width)
{
	পূর্ণসংখ্যা width = 0;

	get_terminal_dimension(&width, ফাঁকা);

	ফেরত width > 0 ? width : default_width;
}

পূর্ণসংখ্যা get_terminal_stdfd(শূন্য)
{
	যদি (isatty(STDIN_FILENO))
		ফেরত STDIN_FILENO;
	যদি (isatty(STDOUT_FILENO))
		ফেরত STDOUT_FILENO;
	যদি (isatty(STDERR_FILENO))
		ফেরত STDERR_FILENO;

	ফেরত -EINVAL;
}

পূর্ণসংখ্যা get_terminal_name(ধ্রুবক অক্ষর **path,
		      ধ্রুবক অক্ষর **name,
		      ধ্রুবক অক্ষর **সংখ্যা)
{
	ধ্রুবক অক্ষর *tty;
	ধ্রুবক অক্ষর *p;
	পূর্ণসংখ্যা fd;


	যদি (name)
		*name = ফাঁকা;
	যদি (path)
		*path = ফাঁকা;
	যদি (সংখ্যা)
		*সংখ্যা = ফাঁকা;

	fd = get_terminal_stdfd();
	যদি (fd < 0)
		ফেরত fd;	/* error */

	tty = ttyname(fd);
	যদি (!tty)
		ফেরত -1;

	যদি (path)
		*path = tty;
	যদি (name || সংখ্যা)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	যদি (name)
		*name = tty;
	যদি (সংখ্যা) {
		জন্য (p = tty; p && *p; p++) {
			যদি (isdigit(*p)) {
				*সংখ্যা = p;
				বিরতি;
			}
		}
	}
	ফেরত 0;
}

পূর্ণসংখ্যা get_terminal_type(ধ্রুবক অক্ষর **type)
{
	*type = getenv("TERM");
	যদি (*type)
		ফেরত -EINVAL;
	ফেরত 0;
}

অক্ষর *get_terminal_default_type(ধ্রুবক অক্ষর *ttyname, পূর্ণসংখ্যা is_serial)
{
	যদি (ttyname) {
#ifdef HAVE_GETTTYNAM
		কাঠামো ttyent *ent = getttynam(ttyname);

		যদি (ent && ent->ty_type)
			ফেরত strdup(ent->ty_type);
#endif

#যদি defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		যদি (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			ফেরত strdup(DEFAULT_TTYS0);
		অন্যথা যদি (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			ফেরত strdup(DEFAULT_TTY32);
		অন্যথা যদি (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			ফেরত strdup(DEFAULT_TTYS1);
#endif
	}

	ফেরত strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
পূর্ণসংখ্যা প্রধান(শূন্য)
{
	ধ্রুবক অক্ষর *path, *name, *অঙ্ক;
	পূর্ণসংখ্যা c, l;

	যদি (get_terminal_name(&path, &name, &অঙ্ক) == 0) {
		অক্ষর *term;

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


	ফেরত EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
