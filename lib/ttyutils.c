/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ifdef HAVE_GETTTYNAM
# include <ttyent.h>
#endif

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "ttyutils.h"

#ifdef __linux__
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "linux"
#  endif
#  ଯଦି defined (__s390__) || defined (__s390x__)
#    define DEFAULT_TTYS0  "dumb"
#    define DEFAULT_TTY32  "ibm327x"
#    define DEFAULT_TTYS1  "vt220"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#ନଚେତଯଦି defined(__GNU__)
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "hurd"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#ଅନ୍ୟଥା
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_env_int(ଅଚଳ ଅକ୍ଷର *name)
{
	ଅଚଳ ଅକ୍ଷର *cp = getenv(name);

	ଯଦି (cp) {
		ଅକ୍ଷର *end = ଖାଲି;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		ଯଦି (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			ଫେରନ୍ତୁ x;
	}

	ଫେରନ୍ତୁ -1;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_terminal_dimension(ପୂର୍ଣ୍ଣସଂଖ୍ୟା *cols, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *ଧାଡ଼ିଗୁଡ଼ିକ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା c = 0, l = 0;

#ଯଦି defined(TIOCGWINSZ)
	ସଂରଚନା winsize	w_win;
	ଯଦି (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#ନଚେତଯଦି defined(TIOCGSIZE)
	ସଂରଚନା ttysize	t_win;
	ଯଦି (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_ଧାଡ଼ିଗୁଡ଼ିକ;
	}
#endif
	ଯଦି (cols) {
		ଯଦି (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	ଯଦି (ଧାଡ଼ିଗୁଡ଼ିକ) {
		ଯଦି (l <= 0)
			l = get_env_int("LINES");
		*ଧାଡ଼ିଗୁଡ଼ିକ = l;
	}
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_terminal_width(ପୂର୍ଣ୍ଣସଂଖ୍ୟା default_width)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା width = 0;

	get_terminal_dimension(&width, ଖାଲି);

	ଫେରନ୍ତୁ width > 0 ? width : default_width;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_terminal_stdfd(ଶୂନ୍ୟ)
{
	ଯଦି (isatty(STDIN_FILENO))
		ଫେରନ୍ତୁ STDIN_FILENO;
	ଯଦି (isatty(STDOUT_FILENO))
		ଫେରନ୍ତୁ STDOUT_FILENO;
	ଯଦି (isatty(STDERR_FILENO))
		ଫେରନ୍ତୁ STDERR_FILENO;

	ଫେରନ୍ତୁ -EINVAL;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_terminal_name(ଅଚଳ ଅକ୍ଷର **path,
		      ଅଚଳ ଅକ୍ଷର **name,
		      ଅଚଳ ଅକ୍ଷର **ସଂଖ୍ୟା)
{
	ଅଚଳ ଅକ୍ଷର *tty;
	ଅଚଳ ଅକ୍ଷର *p;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;


	ଯଦି (name)
		*name = ଖାଲି;
	ଯଦି (path)
		*path = ଖାଲି;
	ଯଦି (ସଂଖ୍ୟା)
		*ସଂଖ୍ୟା = ଖାଲି;

	fd = get_terminal_stdfd();
	ଯଦି (fd < 0)
		ଫେରନ୍ତୁ fd;	/* error */

	tty = ttyname(fd);
	ଯଦି (!tty)
		ଫେରନ୍ତୁ -1;

	ଯଦି (path)
		*path = tty;
	ଯଦି (name || ସଂଖ୍ୟା)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	ଯଦି (name)
		*name = tty;
	ଯଦି (ସଂଖ୍ୟା) {
		ପାଇଁ (p = tty; p && *p; p++) {
			ଯଦି (isdigit(*p)) {
				*ସଂଖ୍ୟା = p;
				ଭାଙ୍ଗ;
			}
		}
	}
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_terminal_type(ଅଚଳ ଅକ୍ଷର **type)
{
	*type = getenv("TERM");
	ଯଦି (*type)
		ଫେରନ୍ତୁ -EINVAL;
	ଫେରନ୍ତୁ 0;
}

ଅକ୍ଷର *get_terminal_default_type(ଅଚଳ ଅକ୍ଷର *ttyname, ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_serial)
{
	ଯଦି (ttyname) {
#ifdef HAVE_GETTTYNAM
		ସଂରଚନା ttyent *ent = getttynam(ttyname);

		ଯଦି (ent && ent->ty_type)
			ଫେରନ୍ତୁ strdup(ent->ty_type);
#endif

#ଯଦି defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		ଯଦି (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			ଫେରନ୍ତୁ strdup(DEFAULT_TTYS0);
		ଅନ୍ୟଥା ଯଦି (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			ଫେରନ୍ତୁ strdup(DEFAULT_TTY32);
		ଅନ୍ୟଥା ଯଦି (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			ଫେରନ୍ତୁ strdup(DEFAULT_TTYS1);
#endif
	}

	ଫେରନ୍ତୁ strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ଶୂନ୍ୟ)
{
	ଅଚଳ ଅକ୍ଷର *path, *name, *ଅଙ୍କ;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା c, l;

	ଯଦି (get_terminal_name(&path, &name, &ଅଙ୍କ) == 0) {
		ଅକ୍ଷର *term;

		fprintf(stderr, "tty path:      %s\n", path);
		fprintf(stderr, "tty name:      %s\n", name);
		fprintf(stderr, "tty number:    %s\n", ଅଙ୍କ);

		fprintf(stderr, "tty term:      %s\n", getenv("TERM"));

		term = get_terminal_default_type(name, 0);
		fprintf(stderr, "tty dflt term: %s\n", term);
		ମୁକ୍ତ(term);
	}
	get_terminal_dimension(&c, &l);
	fprintf(stderr,         "tty cols:      %d\n", c);
	fprintf(stderr,         "tty lines:     %d\n", l);


	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
