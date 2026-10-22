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
#  ຖ້າ defined (__s390__) || defined (__s390x__)
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
#ບໍ່ດັ່ງນັ້ນ
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#endif

static ຈຳນວນເຕັມ get_env_int(const var_na *name)
{
	const var_na *cp = getenv(name);

	ຖ້າ (cp) {
		var_na *end = NULL;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		ຖ້າ (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			ສົ່ງຄືນ x;
	}

	ສົ່ງຄືນ -1;
}

ຈຳນວນເຕັມ get_terminal_dimension(ຈຳນວນເຕັມ *cols, ຈຳນວນເຕັມ *lines)
{
	ຈຳນວນເຕັມ c = 0, l = 0;

#ຖ້າ defined(TIOCGWINSZ)
	sa_nracanaa winsize	w_win;
	ຖ້າ (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#elif defined(TIOCGSIZE)
	sa_nracanaa ttysize	t_win;
	ຖ້າ (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_lines;
	}
#endif
	ຖ້າ (cols) {
		ຖ້າ (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	ຖ້າ (lines) {
		ຖ້າ (l <= 0)
			l = get_env_int("LINES");
		*lines = l;
	}
	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ get_terminal_width(ຈຳນວນເຕັມ default_width)
{
	ຈຳນວນເຕັມ width = 0;

	get_terminal_dimension(&width, NULL);

	ສົ່ງຄືນ width > 0 ? width : default_width;
}

ຈຳນວນເຕັມ get_terminal_stdfd(void)
{
	ຖ້າ (isatty(STDIN_FILENO))
		ສົ່ງຄືນ STDIN_FILENO;
	ຖ້າ (isatty(STDOUT_FILENO))
		ສົ່ງຄືນ STDOUT_FILENO;
	ຖ້າ (isatty(STDERR_FILENO))
		ສົ່ງຄືນ STDERR_FILENO;

	ສົ່ງຄືນ -EINVAL;
}

ຈຳນວນເຕັມ get_terminal_name(const var_na **path,
		      const var_na **name,
		      const var_na **number)
{
	const var_na *tty;
	const var_na *p;
	ຈຳນວນເຕັມ fd;


	ຖ້າ (name)
		*name = NULL;
	ຖ້າ (path)
		*path = NULL;
	ຖ້າ (number)
		*number = NULL;

	fd = get_terminal_stdfd();
	ຖ້າ (fd < 0)
		ສົ່ງຄືນ fd;	/* error */

	tty = ttyname(fd);
	ຖ້າ (!tty)
		ສົ່ງຄືນ -1;

	ຖ້າ (path)
		*path = tty;
	ຖ້າ (name || number)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	ຖ້າ (name)
		*name = tty;
	ຖ້າ (number) {
		ສຳລັບ (p = tty; p && *p; p++) {
			ຖ້າ (isdigit(*p)) {
				*number = p;
				bha_nga;
			}
		}
	}
	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ get_terminal_type(const var_na **type)
{
	*type = getenv("TERM");
	ຖ້າ (*type)
		ສົ່ງຄືນ -EINVAL;
	ສົ່ງຄືນ 0;
}

var_na *get_terminal_default_type(const var_na *ttyname, ຈຳນວນເຕັມ is_serial)
{
	ຖ້າ (ttyname) {
#ifdef HAVE_GETTTYNAM
		sa_nracanaa ttyent *ent = getttynam(ttyname);

		ຖ້າ (ent && ent->ty_type)
			ສົ່ງຄືນ strdup(ent->ty_type);
#endif

#ຖ້າ defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		ຖ້າ (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			ສົ່ງຄືນ strdup(DEFAULT_TTYS0);
		ບໍ່ດັ່ງນັ້ນ ຖ້າ (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			ສົ່ງຄືນ strdup(DEFAULT_TTY32);
		ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			ສົ່ງຄືນ strdup(DEFAULT_TTYS1);
#endif
	}

	ສົ່ງຄືນ strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
ຈຳນວນເຕັມ main(void)
{
	const var_na *path, *name, *num;
	ຈຳນວນເຕັມ c, l;

	ຖ້າ (get_terminal_name(&path, &name, &num) == 0) {
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


	ສົ່ງຄືນ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
