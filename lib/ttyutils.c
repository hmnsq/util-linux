/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <unistd.h>

#ifdef HAVE_GETTTYNAM
# include <ttyent.h>
#endif

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "ttyutils.h"

#ifdef __linux__
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "linux"
#  endif
#  എങ്കിൽ defined (__s390__) || defined (__s390x__)
#    define DEFAULT_TTYS0  "dumb"
#    define DEFAULT_TTY32  "ibm327x"
#    define DEFAULT_TTYS1  "vt220"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#അല്ലെങ്കിലെങ്കിൽ defined(__GNU__)
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "hurd"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#അല്ലെങ്കിൽ
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ get_env_int(സ്ഥിരം അക്ഷരം *name)
{
	സ്ഥിരം അക്ഷരം *cp = getenv(name);

	എങ്കിൽ (cp) {
		അക്ഷരം *end = ശൂന്യ;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		എങ്കിൽ (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			തിരികെ x;
	}

	തിരികെ -1;
}

പൂർണ്ണസംഖ്യ get_terminal_dimension(പൂർണ്ണസംഖ്യ *cols, പൂർണ്ണസംഖ്യ *വരികൾ)
{
	പൂർണ്ണസംഖ്യ c = 0, l = 0;

#എങ്കിൽ defined(TIOCGWINSZ)
	ഘടന winsize	w_win;
	എങ്കിൽ (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#അല്ലെങ്കിലെങ്കിൽ defined(TIOCGSIZE)
	ഘടന ttysize	t_win;
	എങ്കിൽ (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_വരികൾ;
	}
#endif
	എങ്കിൽ (cols) {
		എങ്കിൽ (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	എങ്കിൽ (വരികൾ) {
		എങ്കിൽ (l <= 0)
			l = get_env_int("LINES");
		*വരികൾ = l;
	}
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ get_terminal_width(പൂർണ്ണസംഖ്യ default_width)
{
	പൂർണ്ണസംഖ്യ width = 0;

	get_terminal_dimension(&width, ശൂന്യ);

	തിരികെ width > 0 ? width : default_width;
}

പൂർണ്ണസംഖ്യ get_terminal_stdfd(ശൂന്യം)
{
	എങ്കിൽ (isatty(STDIN_FILENO))
		തിരികെ STDIN_FILENO;
	എങ്കിൽ (isatty(STDOUT_FILENO))
		തിരികെ STDOUT_FILENO;
	എങ്കിൽ (isatty(STDERR_FILENO))
		തിരികെ STDERR_FILENO;

	തിരികെ -EINVAL;
}

പൂർണ്ണസംഖ്യ get_terminal_name(സ്ഥിരം അക്ഷരം **path,
		      സ്ഥിരം അക്ഷരം **name,
		      സ്ഥിരം അക്ഷരം **സംഖ്യ)
{
	സ്ഥിരം അക്ഷരം *tty;
	സ്ഥിരം അക്ഷരം *p;
	പൂർണ്ണസംഖ്യ fd;


	എങ്കിൽ (name)
		*name = ശൂന്യ;
	എങ്കിൽ (path)
		*path = ശൂന്യ;
	എങ്കിൽ (സംഖ്യ)
		*സംഖ്യ = ശൂന്യ;

	fd = get_terminal_stdfd();
	എങ്കിൽ (fd < 0)
		തിരികെ fd;	/* error */

	tty = ttyname(fd);
	എങ്കിൽ (!tty)
		തിരികെ -1;

	എങ്കിൽ (path)
		*path = tty;
	എങ്കിൽ (name || സംഖ്യ)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	എങ്കിൽ (name)
		*name = tty;
	എങ്കിൽ (സംഖ്യ) {
		വേണ്ടി (p = tty; p && *p; p++) {
			എങ്കിൽ (isdigit(*p)) {
				*സംഖ്യ = p;
				നിർത്തുക;
			}
		}
	}
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ get_terminal_type(സ്ഥിരം അക്ഷരം **type)
{
	*type = getenv("TERM");
	എങ്കിൽ (*type)
		തിരികെ -EINVAL;
	തിരികെ 0;
}

അക്ഷരം *get_terminal_default_type(സ്ഥിരം അക്ഷരം *ttyname, പൂർണ്ണസംഖ്യ is_serial)
{
	എങ്കിൽ (ttyname) {
#ifdef HAVE_GETTTYNAM
		ഘടന ttyent *ent = getttynam(ttyname);

		എങ്കിൽ (ent && ent->ty_type)
			തിരികെ strdup(ent->ty_type);
#endif

#എങ്കിൽ defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		എങ്കിൽ (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			തിരികെ strdup(DEFAULT_TTYS0);
		അല്ലെങ്കിൽ എങ്കിൽ (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			തിരികെ strdup(DEFAULT_TTY32);
		അല്ലെങ്കിൽ എങ്കിൽ (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			തിരികെ strdup(DEFAULT_TTYS1);
#endif
	}

	തിരികെ strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
പൂർണ്ണസംഖ്യ പ്രധാന(ശൂന്യം)
{
	സ്ഥിരം അക്ഷരം *path, *name, *അക്കം;
	പൂർണ്ണസംഖ്യ c, l;

	എങ്കിൽ (get_terminal_name(&path, &name, &അക്കം) == 0) {
		അക്ഷരം *term;

		fprintf(stderr, "tty path:      %s\n", path);
		fprintf(stderr, "tty name:      %s\n", name);
		fprintf(stderr, "tty number:    %s\n", അക്കം);

		fprintf(stderr, "tty term:      %s\n", getenv("TERM"));

		term = get_terminal_default_type(name, 0);
		fprintf(stderr, "tty dflt term: %s\n", term);
		സ്വതന്ത്രം(term);
	}
	get_terminal_dimension(&c, &l);
	fprintf(stderr,         "tty cols:      %d\n", c);
	fprintf(stderr,         "tty lines:     %d\n", l);


	തിരികെ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
