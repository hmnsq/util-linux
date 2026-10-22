/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ifdef HAVE_GETTTYNAM
# include <ttyent.h>
#endif

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "ttyutils.h"

#ifdef __linux__
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "linux"
#  endif
#  ಯದಿ defined (__s390__) || defined (__s390x__)
#    define DEFAULT_TTYS0  "dumb"
#    define DEFAULT_TTY32  "ibm327x"
#    define DEFAULT_TTYS1  "vt220"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#ನೋಚೇದ್ಯದಿ defined(__GNU__)
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "hurd"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#ಅನ್ಯಥಾ
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#endif

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ get_env_int(ಧ್ರುವ ವರ್ಣ *name)
{
	ಧ್ರುವ ವರ್ಣ *cp = getenv(name);

	ಯದಿ (cp) {
		ವರ್ಣ *end = ರಿಕ್ತ;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		ಯದಿ (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			ಪ್ರತ್ಯಾಗಮ x;
	}

	ಪ್ರತ್ಯಾಗಮ -1;
}

ಪೂರ್ಣಾಙ್ಕ get_terminal_dimension(ಪೂರ್ಣಾಙ್ಕ *cols, ಪೂರ್ಣಾಙ್ಕ *ಸಾಲುಗಳು)
{
	ಪೂರ್ಣಾಙ್ಕ c = 0, l = 0;

#ಯದಿ defined(TIOCGWINSZ)
	ಸಂರಚನಾ winsize	w_win;
	ಯದಿ (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#ನೋಚೇದ್ಯದಿ defined(TIOCGSIZE)
	ಸಂರಚನಾ ttysize	t_win;
	ಯದಿ (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_ಸಾಲುಗಳು;
	}
#endif
	ಯದಿ (cols) {
		ಯದಿ (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	ಯದಿ (ಸಾಲುಗಳು) {
		ಯದಿ (l <= 0)
			l = get_env_int("LINES");
		*ಸಾಲುಗಳು = l;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ get_terminal_width(ಪೂರ್ಣಾಙ್ಕ default_width)
{
	ಪೂರ್ಣಾಙ್ಕ width = 0;

	get_terminal_dimension(&width, ರಿಕ್ತ);

	ಪ್ರತ್ಯಾಗಮ width > 0 ? width : default_width;
}

ಪೂರ್ಣಾಙ್ಕ get_terminal_stdfd(ಶೂನ್ಯ)
{
	ಯದಿ (isatty(STDIN_FILENO))
		ಪ್ರತ್ಯಾಗಮ STDIN_FILENO;
	ಯದಿ (isatty(STDOUT_FILENO))
		ಪ್ರತ್ಯಾಗಮ STDOUT_FILENO;
	ಯದಿ (isatty(STDERR_FILENO))
		ಪ್ರತ್ಯಾಗಮ STDERR_FILENO;

	ಪ್ರತ್ಯಾಗಮ -EINVAL;
}

ಪೂರ್ಣಾಙ್ಕ get_terminal_name(ಧ್ರುವ ವರ್ಣ **path,
		      ಧ್ರುವ ವರ್ಣ **name,
		      ಧ್ರುವ ವರ್ಣ **ಸಂಖ್ಯೆ)
{
	ಧ್ರುವ ವರ್ಣ *tty;
	ಧ್ರುವ ವರ್ಣ *p;
	ಪೂರ್ಣಾಙ್ಕ fd;


	ಯದಿ (name)
		*name = ರಿಕ್ತ;
	ಯದಿ (path)
		*path = ರಿಕ್ತ;
	ಯದಿ (ಸಂಖ್ಯೆ)
		*ಸಂಖ್ಯೆ = ರಿಕ್ತ;

	fd = get_terminal_stdfd();
	ಯದಿ (fd < 0)
		ಪ್ರತ್ಯಾಗಮ fd;	/* error */

	tty = ttyname(fd);
	ಯದಿ (!tty)
		ಪ್ರತ್ಯಾಗಮ -1;

	ಯದಿ (path)
		*path = tty;
	ಯದಿ (name || ಸಂಖ್ಯೆ)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	ಯದಿ (name)
		*name = tty;
	ಯದಿ (ಸಂಖ್ಯೆ) {
		ಪ್ರತಿ (p = tty; p && *p; p++) {
			ಯದಿ (isdigit(*p)) {
				*ಸಂಖ್ಯೆ = p;
				ಭಙ್ಗ;
			}
		}
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ get_terminal_type(ಧ್ರುವ ವರ್ಣ **type)
{
	*type = getenv("TERM");
	ಯದಿ (*type)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ವರ್ಣ *get_terminal_default_type(ಧ್ರುವ ವರ್ಣ *ttyname, ಪೂರ್ಣಾಙ್ಕ is_serial)
{
	ಯದಿ (ttyname) {
#ifdef HAVE_GETTTYNAM
		ಸಂರಚನಾ ttyent *ent = getttynam(ttyname);

		ಯದಿ (ent && ent->ty_type)
			ಪ್ರತ್ಯಾಗಮ strdup(ent->ty_type);
#endif

#ಯದಿ defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		ಯದಿ (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			ಪ್ರತ್ಯಾಗಮ strdup(DEFAULT_TTYS0);
		ಅನ್ಯಥಾ ಯದಿ (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			ಪ್ರತ್ಯಾಗಮ strdup(DEFAULT_TTY32);
		ಅನ್ಯಥಾ ಯದಿ (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			ಪ್ರತ್ಯಾಗಮ strdup(DEFAULT_TTYS1);
#endif
	}

	ಪ್ರತ್ಯಾಗಮ strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
ಪೂರ್ಣಾಙ್ಕ main(ಶೂನ್ಯ)
{
	ಧ್ರುವ ವರ್ಣ *path, *name, *ಅಂಕೆ;
	ಪೂರ್ಣಾಙ್ಕ c, l;

	ಯದಿ (get_terminal_name(&path, &name, &ಅಂಕೆ) == 0) {
		ವರ್ಣ *term;

		fprintf(stderr, "tty path:      %s\n", path);
		fprintf(stderr, "tty name:      %s\n", name);
		fprintf(stderr, "tty number:    %s\n", ಅಂಕೆ);

		fprintf(stderr, "tty term:      %s\n", getenv("TERM"));

		term = get_terminal_default_type(name, 0);
		fprintf(stderr, "tty dflt term: %s\n", term);
		ಮುಕ್ತ(term);
	}
	get_terminal_dimension(&c, &l);
	fprintf(stderr,         "tty cols:      %d\n", c);
	fprintf(stderr,         "tty lines:     %d\n", l);


	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
