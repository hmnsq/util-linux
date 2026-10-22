/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_GETTTYNAM
# include <ttyent.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "ttyutils.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "linux"
#  endif
#  ಆದರೆ defined (__s390__) || defined (__s390x__)
#    define DEFAULT_TTYS0  "dumb"
#    define DEFAULT_TTY32  "ibm327x"
#    define DEFAULT_TTYS1  "vt220"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#ಅಥವಾಆದರೆ defined(__GNU__)
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "hurd"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#ಇಲ್ಲದಿದ್ದರೆ
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_env_int(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cp = getenv(name);

	ಆದರೆ (cp) {
		ಅಕ್ಷರ *ಕೊನೆ = ಖಾಲಿ;
		ದೀರ್ಘ x;

		errno = 0;
		x = strtol(cp, &ಕೊನೆ, 10);

		ಆದರೆ (errno == 0 && ಕೊನೆ && *ಕೊನೆ == '\0' && ಕೊನೆ > cp &&
		    x > 0 && x <= ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ)
			ಹಿಂತಿರುಗಿ x;
	}

	ಹಿಂತಿರುಗಿ -1;
}

ಪೂರ್ಣಾಂಕ get_terminal_dimension(ಪೂರ್ಣಾಂಕ *ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ, ಪೂರ್ಣಾಂಕ *ಸಾಲುಗಳು)
{
	ಪೂರ್ಣಾಂಕ c = 0, l = 0;

#ಆದರೆ defined(TIOCGWINSZ)
	ರಚನೆ winsize	w_win;
	ಆದರೆ (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#ಅಥವಾಆದರೆ defined(TIOCGSIZE)
	ರಚನೆ ttysize	t_win;
	ಆದರೆ (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ;
		l = t_win.ts_ಸಾಲುಗಳು;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ) {
		ಆದರೆ (c <= 0)
			c = get_env_int("COLUMNS");
		*ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ = c;
	}
	ಆದರೆ (ಸಾಲುಗಳು) {
		ಆದರೆ (l <= 0)
			l = get_env_int("LINES");
		*ಸಾಲುಗಳು = l;
	}
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ get_terminal_ಅಗಲ(ಪೂರ್ಣಾಂಕ default_ಅಗಲ)
{
	ಪೂರ್ಣಾಂಕ ಅಗಲ = 0;

	get_terminal_dimension(&ಅಗಲ, ಖಾಲಿ);

	ಹಿಂತಿರುಗಿ ಅಗಲ > 0 ? ಅಗಲ : default_ಅಗಲ;
}

ಪೂರ್ಣಾಂಕ get_terminal_stdfd(ಶೂನ್ಯ)
{
	ಆದರೆ (isatty(STDIN_FILENO))
		ಹಿಂತಿರುಗಿ STDIN_FILENO;
	ಆದರೆ (isatty(STDOUT_FILENO))
		ಹಿಂತಿರುಗಿ STDOUT_FILENO;
	ಆದರೆ (isatty(STDERR_FILENO))
		ಹಿಂತಿರುಗಿ STDERR_FILENO;

	ಹಿಂತಿರುಗಿ -EINVAL;
}

ಪೂರ್ಣಾಂಕ get_terminal_name(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **ಮಾರ್ಗ,
		      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **name,
		      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **ಸಂಖ್ಯೆ)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tty;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
	ಪೂರ್ಣಾಂಕ fd;


	ಆದರೆ (name)
		*name = ಖಾಲಿ;
	ಆದರೆ (ಮಾರ್ಗ)
		*ಮಾರ್ಗ = ಖಾಲಿ;
	ಆದರೆ (ಸಂಖ್ಯೆ)
		*ಸಂಖ್ಯೆ = ಖಾಲಿ;

	fd = get_terminal_stdfd();
	ಆದರೆ (fd < 0)
		ಹಿಂತಿರುಗಿ fd;	/* error */

	tty = ttyname(fd);
	ಆದರೆ (!tty)
		ಹಿಂತಿರುಗಿ -1;

	ಆದರೆ (ಮಾರ್ಗ)
		*ಮಾರ್ಗ = tty;
	ಆದರೆ (name || ಸಂಖ್ಯೆ)
		tty = ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	ಆದರೆ (name)
		*name = tty;
	ಆದರೆ (ಸಂಖ್ಯೆ) {
		ಗಾಗಿ (p = tty; p && *p; p++) {
			ಆದರೆ (ಅಂಕಿಯೇ(*p)) {
				*ಸಂಖ್ಯೆ = p;
				ನಿಲ್ಲಿಸು;
			}
		}
	}
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ get_terminal_type(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **type)
{
	*type = getenv("TERM");
	ಆದರೆ (*type)
		ಹಿಂತಿರುಗಿ -EINVAL;
	ಹಿಂತಿರುಗಿ 0;
}

ಅಕ್ಷರ *get_terminal_default_type(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ttyname, ಪೂರ್ಣಾಂಕ is_serial)
{
	ಆದರೆ (ttyname) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_GETTTYNAM
		ರಚನೆ ttyent *ent = getttynam(ttyname);

		ಆದರೆ (ent && ent->ty_type)
			ಹಿಂತಿರುಗಿ strdup(ent->ty_type);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		ಆದರೆ (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			ಹಿಂತಿರುಗಿ strdup(DEFAULT_TTYS0);
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			ಹಿಂತಿರುಗಿ strdup(DEFAULT_TTY32);
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			ಹಿಂತಿರುಗಿ strdup(DEFAULT_TTYS1);
#ಆದರೆ_ಅಂತ್ಯ
	}

	ಹಿಂತಿರುಗಿ strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_TTYUTILS
# include <stdlib.h>
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಶೂನ್ಯ)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, *name, *ಅಂಕೆ;
	ಪೂರ್ಣಾಂಕ c, l;

	ಆದರೆ (get_terminal_name(&ಮಾರ್ಗ, &name, &ಅಂಕೆ) == 0) {
		ಅಕ್ಷರ *ಪದ;

		fprintf(stderr, "tty path:      %s\n", ಮಾರ್ಗ);
		fprintf(stderr, "tty name:      %s\n", name);
		fprintf(stderr, "tty number:    %s\n", ಅಂಕೆ);

		fprintf(stderr, "tty term:      %s\n", getenv("TERM"));

		ಪದ = get_terminal_default_type(name, 0);
		fprintf(stderr, "tty dflt term: %s\n", ಪದ);
		ಬಿಡುಗಡೆ(ಪದ);
	}
	get_terminal_dimension(&c, &l);
	fprintf(stderr,         "tty cols:      %d\n", c);
	fprintf(stderr,         "tty lines:     %d\n", l);


	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
#ಆದರೆ_ಅಂತ್ಯ /* TEST_PROGRAM_TTYUTILS */
