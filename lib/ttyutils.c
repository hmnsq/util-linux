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
#  အကယ်၍ defined (__s390__) || defined (__s390x__)
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
#မဟုတ်ပါက
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#endif

static ကိန်းပြည့် get_env_int(const var_na *name)
{
	const var_na *cp = getenv(name);

	အကယ်၍ (cp) {
		var_na *end = NULL;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		အကယ်၍ (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			ပြန်ပေး x;
	}

	ပြန်ပေး -1;
}

ကိန်းပြည့် get_terminal_dimension(ကိန်းပြည့် *cols, ကိန်းပြည့် *lines)
{
	ကိန်းပြည့် c = 0, l = 0;

#အကယ်၍ defined(TIOCGWINSZ)
	sa_nracanaa winsize	w_win;
	အကယ်၍ (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#elif defined(TIOCGSIZE)
	sa_nracanaa ttysize	t_win;
	အကယ်၍ (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_lines;
	}
#endif
	အကယ်၍ (cols) {
		အကယ်၍ (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	အကယ်၍ (lines) {
		အကယ်၍ (l <= 0)
			l = get_env_int("LINES");
		*lines = l;
	}
	ပြန်ပေး 0;
}

ကိန်းပြည့် get_terminal_width(ကိန်းပြည့် default_width)
{
	ကိန်းပြည့် width = 0;

	get_terminal_dimension(&width, NULL);

	ပြန်ပေး width > 0 ? width : default_width;
}

ကိန်းပြည့် get_terminal_stdfd(void)
{
	အကယ်၍ (isatty(STDIN_FILENO))
		ပြန်ပေး STDIN_FILENO;
	အကယ်၍ (isatty(STDOUT_FILENO))
		ပြန်ပေး STDOUT_FILENO;
	အကယ်၍ (isatty(STDERR_FILENO))
		ပြန်ပေး STDERR_FILENO;

	ပြန်ပေး -EINVAL;
}

ကိန်းပြည့် get_terminal_name(const var_na **path,
		      const var_na **name,
		      const var_na **number)
{
	const var_na *tty;
	const var_na *p;
	ကိန်းပြည့် fd;


	အကယ်၍ (name)
		*name = NULL;
	အကယ်၍ (path)
		*path = NULL;
	အကယ်၍ (number)
		*number = NULL;

	fd = get_terminal_stdfd();
	အကယ်၍ (fd < 0)
		ပြန်ပေး fd;	/* error */

	tty = ttyname(fd);
	အကယ်၍ (!tty)
		ပြန်ပေး -1;

	အကယ်၍ (path)
		*path = tty;
	အကယ်၍ (name || number)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	အကယ်၍ (name)
		*name = tty;
	အကယ်၍ (number) {
		အတွက် (p = tty; p && *p; p++) {
			အကယ်၍ (isdigit(*p)) {
				*number = p;
				bha_nga;
			}
		}
	}
	ပြန်ပေး 0;
}

ကိန်းပြည့် get_terminal_type(const var_na **type)
{
	*type = getenv("TERM");
	အကယ်၍ (*type)
		ပြန်ပေး -EINVAL;
	ပြန်ပေး 0;
}

var_na *get_terminal_default_type(const var_na *ttyname, ကိန်းပြည့် is_serial)
{
	အကယ်၍ (ttyname) {
#ifdef HAVE_GETTTYNAM
		sa_nracanaa ttyent *ent = getttynam(ttyname);

		အကယ်၍ (ent && ent->ty_type)
			ပြန်ပေး strdup(ent->ty_type);
#endif

#အကယ်၍ defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		အကယ်၍ (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			ပြန်ပေး strdup(DEFAULT_TTYS0);
		မဟုတ်ပါက အကယ်၍ (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			ပြန်ပေး strdup(DEFAULT_TTY32);
		မဟုတ်ပါက အကယ်၍ (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			ပြန်ပေး strdup(DEFAULT_TTYS1);
#endif
	}

	ပြန်ပေး strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
ကိန်းပြည့် main(void)
{
	const var_na *path, *name, *num;
	ကိန်းပြည့် c, l;

	အကယ်၍ (get_terminal_name(&path, &name, &num) == 0) {
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


	ပြန်ပေး EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
