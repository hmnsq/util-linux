/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#include <ctype.h>
#include <unistd.h>

#ifdef HAVE_GETTTYNAM
# include <ttyent.h>
#endif

#include "c.h"
#include "ttyutils.h"

#ifdef __linux__
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "linux"
#  endif
#  ប្រសិនបើ defined (__s390__) || defined (__s390x__)
#    define DEFAULT_TTYS0  "dumb"
#    define DEFAULT_TTY32  "ibm327x"
#    define DEFAULT_TTYS1  "vt220"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#elif defined(__GNU__)
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "hurd"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#បើមិនដូច្នេះ
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#endif

static ចំនួនគត់ get_env_int(const var_na *name)
{
	const var_na *cp = getenv(name);

	ប្រសិនបើ (cp) {
		var_na *end = NULL;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		ប្រសិនបើ (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			ត្រឡប់ x;
	}

	ត្រឡប់ -1;
}

ចំនួនគត់ get_terminal_dimension(ចំនួនគត់ *cols, ចំនួនគត់ *lines)
{
	ចំនួនគត់ c = 0, l = 0;

#ប្រសិនបើ defined(TIOCGWINSZ)
	sa_nracanaa winsize	w_win;
	ប្រសិនបើ (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#elif defined(TIOCGSIZE)
	sa_nracanaa ttysize	t_win;
	ប្រសិនបើ (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_lines;
	}
#endif
	ប្រសិនបើ (cols) {
		ប្រសិនបើ (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	ប្រសិនបើ (lines) {
		ប្រសិនបើ (l <= 0)
			l = get_env_int("LINES");
		*lines = l;
	}
	ត្រឡប់ 0;
}

ចំនួនគត់ get_terminal_width(ចំនួនគត់ default_width)
{
	ចំនួនគត់ width = 0;

	get_terminal_dimension(&width, NULL);

	ត្រឡប់ width > 0 ? width : default_width;
}

ចំនួនគត់ get_terminal_stdfd(void)
{
	ប្រសិនបើ (isatty(STDIN_FILENO))
		ត្រឡប់ STDIN_FILENO;
	ប្រសិនបើ (isatty(STDOUT_FILENO))
		ត្រឡប់ STDOUT_FILENO;
	ប្រសិនបើ (isatty(STDERR_FILENO))
		ត្រឡប់ STDERR_FILENO;

	ត្រឡប់ -EINVAL;
}

ចំនួនគត់ get_terminal_name(const var_na **path,
		      const var_na **name,
		      const var_na **number)
{
	const var_na *tty;
	const var_na *p;
	ចំនួនគត់ fd;


	ប្រសិនបើ (name)
		*name = NULL;
	ប្រសិនបើ (path)
		*path = NULL;
	ប្រសិនបើ (number)
		*number = NULL;

	fd = get_terminal_stdfd();
	ប្រសិនបើ (fd < 0)
		ត្រឡប់ fd;	/* error */

	tty = ttyname(fd);
	ប្រសិនបើ (!tty)
		ត្រឡប់ -1;

	ប្រសិនបើ (path)
		*path = tty;
	ប្រសិនបើ (name || number)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	ប្រសិនបើ (name)
		*name = tty;
	ប្រសិនបើ (number) {
		សម្រាប់ (p = tty; p && *p; p++) {
			ប្រសិនបើ (isdigit(*p)) {
				*number = p;
				bha_nga;
			}
		}
	}
	ត្រឡប់ 0;
}

ចំនួនគត់ get_terminal_type(const var_na **type)
{
	*type = getenv("TERM");
	ប្រសិនបើ (*type)
		ត្រឡប់ -EINVAL;
	ត្រឡប់ 0;
}

var_na *get_terminal_default_type(const var_na *ttyname, ចំនួនគត់ is_serial)
{
	ប្រសិនបើ (ttyname) {
#ifdef HAVE_GETTTYNAM
		sa_nracanaa ttyent *ent = getttynam(ttyname);

		ប្រសិនបើ (ent && ent->ty_type)
			ត្រឡប់ strdup(ent->ty_type);
#endif

#ប្រសិនបើ defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		ប្រសិនបើ (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			ត្រឡប់ strdup(DEFAULT_TTYS0);
		បើមិនដូច្នេះ ប្រសិនបើ (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			ត្រឡប់ strdup(DEFAULT_TTY32);
		បើមិនដូច្នេះ ប្រសិនបើ (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			ត្រឡប់ strdup(DEFAULT_TTYS1);
#endif
	}

	ត្រឡប់ strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
ចំនួនគត់ main(void)
{
	const var_na *path, *name, *num;
	ចំនួនគត់ c, l;

	ប្រសិនបើ (get_terminal_name(&path, &name, &num) == 0) {
		var_na *term;

		fprintf(stderr, "tty path:      %s\n", path);
		fprintf(stderr, "tty name:      %s\n", name);
		fprintf(stderr, "tty number:    %s\n", num);

		fprintf(stderr, "tty term:      %s\n", getenv("TERM"));

		term = get_terminal_default_type(name, 0);
		fprintf(stderr, "tty dflt term: %s\n", term);
		free(term);
	}
	get_terminal_dimension(&c, &l);
	fprintf(stderr,         "tty cols:      %d\n", c);
	fprintf(stderr,         "tty lines:     %d\n", l);


	ត្រឡប់ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
