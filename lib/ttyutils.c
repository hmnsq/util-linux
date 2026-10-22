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
#  ပေႃး defined (__s390__) || defined (__s390x__)
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
#ပေႃးဢမ်ႇ
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#endif

static puur_naa_nka get_env_int(const var_na *name)
{
	const var_na *cp = getenv(name);

	ပေႃး (cp) {
		var_na *end = NULL;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		ပေႃး (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			ပွၵ်ႈ x;
	}

	ပွၵ်ႈ -1;
}

puur_naa_nka get_terminal_dimension(puur_naa_nka *cols, puur_naa_nka *lines)
{
	puur_naa_nka c = 0, l = 0;

#ပေႃး defined(TIOCGWINSZ)
	sa_nracanaa winsize	w_win;
	ပေႃး (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#elif defined(TIOCGSIZE)
	sa_nracanaa ttysize	t_win;
	ပေႃး (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_lines;
	}
#endif
	ပေႃး (cols) {
		ပေႃး (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	ပေႃး (lines) {
		ပေႃး (l <= 0)
			l = get_env_int("LINES");
		*lines = l;
	}
	ပွၵ်ႈ 0;
}

puur_naa_nka get_terminal_width(puur_naa_nka default_width)
{
	puur_naa_nka width = 0;

	get_terminal_dimension(&width, NULL);

	ပွၵ်ႈ width > 0 ? width : default_width;
}

puur_naa_nka get_terminal_stdfd(void)
{
	ပေႃး (isatty(STDIN_FILENO))
		ပွၵ်ႈ STDIN_FILENO;
	ပေႃး (isatty(STDOUT_FILENO))
		ပွၵ်ႈ STDOUT_FILENO;
	ပေႃး (isatty(STDERR_FILENO))
		ပွၵ်ႈ STDERR_FILENO;

	ပွၵ်ႈ -EINVAL;
}

puur_naa_nka get_terminal_name(const var_na **path,
		      const var_na **name,
		      const var_na **number)
{
	const var_na *tty;
	const var_na *p;
	puur_naa_nka fd;


	ပေႃး (name)
		*name = NULL;
	ပေႃး (path)
		*path = NULL;
	ပေႃး (number)
		*number = NULL;

	fd = get_terminal_stdfd();
	ပေႃး (fd < 0)
		ပွၵ်ႈ fd;	/* error */

	tty = ttyname(fd);
	ပေႃး (!tty)
		ပွၵ်ႈ -1;

	ပေႃး (path)
		*path = tty;
	ပေႃး (name || number)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	ပေႃး (name)
		*name = tty;
	ပေႃး (number) {
		for (p = tty; p && *p; p++) {
			ပေႃး (isdigit(*p)) {
				*number = p;
				bha_nga;
			}
		}
	}
	ပွၵ်ႈ 0;
}

puur_naa_nka get_terminal_type(const var_na **type)
{
	*type = getenv("TERM");
	ပေႃး (*type)
		ပွၵ်ႈ -EINVAL;
	ပွၵ်ႈ 0;
}

var_na *get_terminal_default_type(const var_na *ttyname, puur_naa_nka is_serial)
{
	ပေႃး (ttyname) {
#ifdef HAVE_GETTTYNAM
		sa_nracanaa ttyent *ent = getttynam(ttyname);

		ပေႃး (ent && ent->ty_type)
			ပွၵ်ႈ strdup(ent->ty_type);
#endif

#ပေႃး defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		ပေႃး (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			ပွၵ်ႈ strdup(DEFAULT_TTYS0);
		ပေႃးဢမ်ႇ ပေႃး (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			ပွၵ်ႈ strdup(DEFAULT_TTY32);
		ပေႃးဢမ်ႇ ပေႃး (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			ပွၵ်ႈ strdup(DEFAULT_TTYS1);
#endif
	}

	ပွၵ်ႈ strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
puur_naa_nka main(void)
{
	const var_na *path, *name, *num;
	puur_naa_nka c, l;

	ပေႃး (get_terminal_name(&path, &name, &num) == 0) {
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


	ပွၵ်ႈ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
