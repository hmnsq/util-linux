/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <unistd.h>

#ifdef HAVE_GETTTYNAM
# include <ttyent.h>
#endif

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "ttyutils.h"

#ifdef __linux__
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "linux"
#  endif
#  எனில் defined (__s390__) || defined (__s390x__)
#    define DEFAULT_TTYS0  "dumb"
#    define DEFAULT_TTY32  "ibm327x"
#    define DEFAULT_TTYS1  "vt220"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#அல்லதெனில் defined(__GNU__)
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "hurd"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#இல்லையெனில்
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#endif

நிலையான முழு get_env_int(மாறிலி எழுத்து *name)
{
	மாறிலி எழுத்து *cp = getenv(name);

	எனில் (cp) {
		எழுத்து *end = வெற்று;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		எனில் (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			திரும்பு x;
	}

	திரும்பு -1;
}

முழு get_terminal_dimension(முழு *cols, முழு *வரிகள்)
{
	முழு c = 0, l = 0;

#எனில் defined(TIOCGWINSZ)
	கட்டமைப்பு winsize	w_win;
	எனில் (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#அல்லதெனில் defined(TIOCGSIZE)
	கட்டமைப்பு ttysize	t_win;
	எனில் (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_வரிகள்;
	}
#endif
	எனில் (cols) {
		எனில் (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	எனில் (வரிகள்) {
		எனில் (l <= 0)
			l = get_env_int("LINES");
		*வரிகள் = l;
	}
	திரும்பு 0;
}

முழு get_terminal_width(முழு default_width)
{
	முழு width = 0;

	get_terminal_dimension(&width, வெற்று);

	திரும்பு width > 0 ? width : default_width;
}

முழு get_terminal_stdfd(வெறுமை)
{
	எனில் (isatty(STDIN_FILENO))
		திரும்பு STDIN_FILENO;
	எனில் (isatty(STDOUT_FILENO))
		திரும்பு STDOUT_FILENO;
	எனில் (isatty(STDERR_FILENO))
		திரும்பு STDERR_FILENO;

	திரும்பு -EINVAL;
}

முழு get_terminal_name(மாறிலி எழுத்து **path,
		      மாறிலி எழுத்து **name,
		      மாறிலி எழுத்து **எண்)
{
	மாறிலி எழுத்து *tty;
	மாறிலி எழுத்து *p;
	முழு fd;


	எனில் (name)
		*name = வெற்று;
	எனில் (path)
		*path = வெற்று;
	எனில் (எண்)
		*எண் = வெற்று;

	fd = get_terminal_stdfd();
	எனில் (fd < 0)
		திரும்பு fd;	/* error */

	tty = ttyname(fd);
	எனில் (!tty)
		திரும்பு -1;

	எனில் (path)
		*path = tty;
	எனில் (name || எண்)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	எனில் (name)
		*name = tty;
	எனில் (எண்) {
		க்கு (p = tty; p && *p; p++) {
			எனில் (isdigit(*p)) {
				*எண் = p;
				நிறுத்து;
			}
		}
	}
	திரும்பு 0;
}

முழு get_terminal_type(மாறிலி எழுத்து **type)
{
	*type = getenv("TERM");
	எனில் (*type)
		திரும்பு -EINVAL;
	திரும்பு 0;
}

எழுத்து *get_terminal_default_type(மாறிலி எழுத்து *ttyname, முழு is_serial)
{
	எனில் (ttyname) {
#ifdef HAVE_GETTTYNAM
		கட்டமைப்பு ttyent *ent = getttynam(ttyname);

		எனில் (ent && ent->ty_type)
			திரும்பு strdup(ent->ty_type);
#endif

#எனில் defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		எனில் (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			திரும்பு strdup(DEFAULT_TTYS0);
		இல்லையெனில் எனில் (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			திரும்பு strdup(DEFAULT_TTY32);
		இல்லையெனில் எனில் (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			திரும்பு strdup(DEFAULT_TTYS1);
#endif
	}

	திரும்பு strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
முழு முதன்மை(வெறுமை)
{
	மாறிலி எழுத்து *path, *name, *இலக்கம்;
	முழு c, l;

	எனில் (get_terminal_name(&path, &name, &இலக்கம்) == 0) {
		எழுத்து *term;

		fprintf(stderr, "tty path:      %s\n", path);
		fprintf(stderr, "tty name:      %s\n", name);
		fprintf(stderr, "tty number:    %s\n", இலக்கம்);

		fprintf(stderr, "tty term:      %s\n", getenv("TERM"));

		term = get_terminal_default_type(name, 0);
		fprintf(stderr, "tty dflt term: %s\n", term);
		விடுவி(term);
	}
	get_terminal_dimension(&c, &l);
	fprintf(stderr,         "tty cols:      %d\n", c);
	fprintf(stderr,         "tty lines:     %d\n", l);


	திரும்பு EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
