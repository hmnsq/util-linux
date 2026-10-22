/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#شامل <ctype.h>
#شامل <unistd.h>

#ifdef HAVE_GETTTYNAM
# include <ttyent.h>
#endif

#شامل "c.h"
#شامل "ttyutils.h"

#ifdef __linux__
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "linux"
#  endif
#  اگر defined (__s390__) || defined (__s390x__)
#    define DEFAULT_TTYS0  "dumb"
#    define DEFAULT_TTY32  "ibm327x"
#    define DEFAULT_TTYS1  "vt220"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#وگرنهاگر defined(__GNU__)
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "hurd"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#وگرنه
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#endif

ایستا صحیح get_env_int(ثابت نویسه *name)
{
	ثابت نویسه *cp = getenv(name);

	اگر (cp) {
		نویسه *end = پوچ;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		اگر (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			بازگشت x;
	}

	بازگشت -1;
}

صحیح get_terminal_dimension(صحیح *cols, صحیح *سطرها)
{
	صحیح c = 0, l = 0;

#اگر defined(TIOCGWINSZ)
	ساختار winsize	w_win;
	اگر (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#وگرنهاگر defined(TIOCGSIZE)
	ساختار ttysize	t_win;
	اگر (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_سطرها;
	}
#endif
	اگر (cols) {
		اگر (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	اگر (سطرها) {
		اگر (l <= 0)
			l = get_env_int("LINES");
		*سطرها = l;
	}
	بازگشت 0;
}

صحیح get_terminal_width(صحیح default_width)
{
	صحیح width = 0;

	get_terminal_dimension(&width, پوچ);

	بازگشت width > 0 ? width : default_width;
}

صحیح get_terminal_stdfd(تهی)
{
	اگر (isatty(STDIN_FILENO))
		بازگشت STDIN_FILENO;
	اگر (isatty(STDOUT_FILENO))
		بازگشت STDOUT_FILENO;
	اگر (isatty(STDERR_FILENO))
		بازگشت STDERR_FILENO;

	بازگشت -EINVAL;
}

صحیح get_terminal_name(ثابت نویسه **path,
		      ثابت نویسه **name,
		      ثابت نویسه **عدد)
{
	ثابت نویسه *tty;
	ثابت نویسه *p;
	صحیح fd;


	اگر (name)
		*name = پوچ;
	اگر (path)
		*path = پوچ;
	اگر (عدد)
		*عدد = پوچ;

	fd = get_terminal_stdfd();
	اگر (fd < 0)
		بازگشت fd;	/* error */

	tty = ttyname(fd);
	اگر (!tty)
		بازگشت -1;

	اگر (path)
		*path = tty;
	اگر (name || عدد)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	اگر (name)
		*name = tty;
	اگر (عدد) {
		برای (p = tty; p && *p; p++) {
			اگر (isdigit(*p)) {
				*عدد = p;
				بشکن;
			}
		}
	}
	بازگشت 0;
}

صحیح get_terminal_type(ثابت نویسه **type)
{
	*type = getenv("TERM");
	اگر (*type)
		بازگشت -EINVAL;
	بازگشت 0;
}

نویسه *get_terminal_default_type(ثابت نویسه *ttyname, صحیح is_serial)
{
	اگر (ttyname) {
#ifdef HAVE_GETTTYNAM
		ساختار ttyent *ent = getttynam(ttyname);

		اگر (ent && ent->ty_type)
			بازگشت strdup(ent->ty_type);
#endif

#اگر defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		اگر (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			بازگشت strdup(DEFAULT_TTYS0);
		وگرنه اگر (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			بازگشت strdup(DEFAULT_TTY32);
		وگرنه اگر (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			بازگشت strdup(DEFAULT_TTYS1);
#endif
	}

	بازگشت strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
صحیح اصلی(تهی)
{
	ثابت نویسه *path, *name, *رقم;
	صحیح c, l;

	اگر (get_terminal_name(&path, &name, &رقم) == 0) {
		نویسه *term;

		fprintf(stderr, "tty path:      %s\n", path);
		fprintf(stderr, "tty name:      %s\n", name);
		fprintf(stderr, "tty number:    %s\n", رقم);

		fprintf(stderr, "tty term:      %s\n", getenv("TERM"));

		term = get_terminal_default_type(name, 0);
		fprintf(stderr, "tty dflt term: %s\n", term);
		آزاد(term);
	}
	get_terminal_dimension(&c, &l);
	fprintf(stderr,         "tty cols:      %d\n", c);
	fprintf(stderr,         "tty lines:     %d\n", l);


	بازگشت EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
