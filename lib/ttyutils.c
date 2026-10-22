/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_GETTTYNAM
# include <ttyent.h>
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ttyutils.h"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "linux"
#  endif
#  ਜੇ defined (__s390__) || defined (__s390x__)
#    define DEFAULT_TTYS0  "dumb"
#    define DEFAULT_TTY32  "ibm327x"
#    define DEFAULT_TTYS1  "vt220"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#ਜਾਂਜੇ defined(__GNU__)
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "hurd"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#ਨਹੀਂਤਾਂ
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ get_env_int(ਅਚਲ ਅੱਖਰ *name)
{
	ਅਚਲ ਅੱਖਰ *cp = getenv(name);

	ਜੇ (cp) {
		ਅੱਖਰ *ਸਿਰਾ = ਨਿਰਾ;
		ਲੰਮਾ x;

		errno = 0;
		x = strtol(cp, &ਸਿਰਾ, 10);

		ਜੇ (errno == 0 && ਸਿਰਾ && *ਸਿਰਾ == '\0' && ਸਿਰਾ > cp &&
		    x > 0 && x <= ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ)
			ਵਾਪਸ x;
	}

	ਵਾਪਸ -1;
}

ਪੂਰਨ get_terminal_dimension(ਪੂਰਨ *ਖਾਨੇ_ਗਿਣਤੀ, ਪੂਰਨ *ਸਤਰਾਂ)
{
	ਪੂਰਨ c = 0, l = 0;

#ਜੇ defined(TIOCGWINSZ)
	ਬਣਤਰ winsize	w_win;
	ਜੇ (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#ਜਾਂਜੇ defined(TIOCGSIZE)
	ਬਣਤਰ ttysize	t_win;
	ਜੇ (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_ਖਾਨੇ_ਗਿਣਤੀ;
		l = t_win.ts_ਸਤਰਾਂ;
	}
#ਜੇ_ਅੰਤ
	ਜੇ (ਖਾਨੇ_ਗਿਣਤੀ) {
		ਜੇ (c <= 0)
			c = get_env_int("COLUMNS");
		*ਖਾਨੇ_ਗਿਣਤੀ = c;
	}
	ਜੇ (ਸਤਰਾਂ) {
		ਜੇ (l <= 0)
			l = get_env_int("LINES");
		*ਸਤਰਾਂ = l;
	}
	ਵਾਪਸ 0;
}

ਪੂਰਨ get_terminal_ਚੌੜਾਈ(ਪੂਰਨ default_ਚੌੜਾਈ)
{
	ਪੂਰਨ ਚੌੜਾਈ = 0;

	get_terminal_dimension(&ਚੌੜਾਈ, ਨਿਰਾ);

	ਵਾਪਸ ਚੌੜਾਈ > 0 ? ਚੌੜਾਈ : default_ਚੌੜਾਈ;
}

ਪੂਰਨ get_terminal_stdfd(ਖਾਲੀ)
{
	ਜੇ (isatty(STDIN_FILENO))
		ਵਾਪਸ STDIN_FILENO;
	ਜੇ (isatty(STDOUT_FILENO))
		ਵਾਪਸ STDOUT_FILENO;
	ਜੇ (isatty(STDERR_FILENO))
		ਵਾਪਸ STDERR_FILENO;

	ਵਾਪਸ -EINVAL;
}

ਪੂਰਨ get_terminal_name(ਅਚਲ ਅੱਖਰ **ਰਸਤਾ,
		      ਅਚਲ ਅੱਖਰ **name,
		      ਅਚਲ ਅੱਖਰ **ਸੰਖਿਆ)
{
	ਅਚਲ ਅੱਖਰ *tty;
	ਅਚਲ ਅੱਖਰ *p;
	ਪੂਰਨ fd;


	ਜੇ (name)
		*name = ਨਿਰਾ;
	ਜੇ (ਰਸਤਾ)
		*ਰਸਤਾ = ਨਿਰਾ;
	ਜੇ (ਸੰਖਿਆ)
		*ਸੰਖਿਆ = ਨਿਰਾ;

	fd = get_terminal_stdfd();
	ਜੇ (fd < 0)
		ਵਾਪਸ fd;	/* error */

	tty = ttyname(fd);
	ਜੇ (!tty)
		ਵਾਪਸ -1;

	ਜੇ (ਰਸਤਾ)
		*ਰਸਤਾ = tty;
	ਜੇ (name || ਸੰਖਿਆ)
		tty = ਲੜੀ_ਤੁਲਨਾ(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	ਜੇ (name)
		*name = tty;
	ਜੇ (ਸੰਖਿਆ) {
		ਲਈ (p = tty; p && *p; p++) {
			ਜੇ (ਅੰਕ_ਹੈ(*p)) {
				*ਸੰਖਿਆ = p;
				ਰੋਕੋ;
			}
		}
	}
	ਵਾਪਸ 0;
}

ਪੂਰਨ get_terminal_type(ਅਚਲ ਅੱਖਰ **type)
{
	*type = getenv("TERM");
	ਜੇ (*type)
		ਵਾਪਸ -EINVAL;
	ਵਾਪਸ 0;
}

ਅੱਖਰ *get_terminal_default_type(ਅਚਲ ਅੱਖਰ *ttyname, ਪੂਰਨ is_serial)
{
	ਜੇ (ttyname) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_GETTTYNAM
		ਬਣਤਰ ttyent *ent = getttynam(ttyname);

		ਜੇ (ent && ent->ty_type)
			ਵਾਪਸ strdup(ent->ty_type);
#ਜੇ_ਅੰਤ

#ਜੇ defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		ਜੇ (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			ਵਾਪਸ strdup(DEFAULT_TTYS0);
		ਨਹੀਂਤਾਂ ਜੇ (ਲੜੀ_ਤੁਲਨਾ(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			ਵਾਪਸ strdup(DEFAULT_TTY32);
		ਨਹੀਂਤਾਂ ਜੇ (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			ਵਾਪਸ strdup(DEFAULT_TTYS1);
#ਜੇ_ਅੰਤ
	}

	ਵਾਪਸ strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_TTYUTILS
# include <stdlib.h>
ਪੂਰਨ ਮੁੱਖ(ਖਾਲੀ)
{
	ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, *name, *ਅੰਕ;
	ਪੂਰਨ c, l;

	ਜੇ (get_terminal_name(&ਰਸਤਾ, &name, &ਅੰਕ) == 0) {
		ਅੱਖਰ *ਪਦ;

		fprintf(stderr, "tty path:      %s\n", ਰਸਤਾ);
		fprintf(stderr, "tty name:      %s\n", name);
		fprintf(stderr, "tty number:    %s\n", ਅੰਕ);

		fprintf(stderr, "tty term:      %s\n", getenv("TERM"));

		ਪਦ = get_terminal_default_type(name, 0);
		fprintf(stderr, "tty dflt term: %s\n", ਪਦ);
		ਮੁਕਤ(ਪਦ);
	}
	get_terminal_dimension(&c, &l);
	fprintf(stderr,         "tty cols:      %d\n", c);
	fprintf(stderr,         "tty lines:     %d\n", l);


	ਵਾਪਸ EXIT_SUCCESS;
}
#ਜੇ_ਅੰਤ /* TEST_PROGRAM_TTYUTILS */
