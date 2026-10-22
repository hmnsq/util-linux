/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <unistd.h>

#यदि_परिभाषितम् HAVE_GETTTYNAM
# include <ttyent.h>
#यदि_अन्तः

#अन्तर्भाव "c.h"
#अन्तर्भाव "ttyutils.h"

#यदि_परिभाषितम् __linux__
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "linux"
#  endif
#  यदि defined (__s390__) || defined (__s390x__)
#    define DEFAULT_TTYS0  "dumb"
#    define DEFAULT_TTY32  "ibm327x"
#    define DEFAULT_TTYS1  "vt220"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#नोचेद्यदि defined(__GNU__)
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "hurd"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#अन्यथा
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#यदि_अन्तः

स्थिर पूर्णाङ्क get_env_int(ध्रुव वर्ण *name)
{
	ध्रुव वर्ण *cp = getenv(name);

	यदि (cp) {
		वर्ण *अन्तः = रिक्त;
		दीर्घ x;

		errno = 0;
		x = strtol(cp, &अन्तः, 10);

		यदि (errno == 0 && अन्तः && *अन्तः == '\0' && अन्तः > cp &&
		    x > 0 && x <= अधिकतम_पूर्णाङ्कः)
			प्रत्यागम x;
	}

	प्रत्यागम -1;
}

पूर्णाङ्क get_terminal_dimension(पूर्णाङ्क *स्तम्भसङ्ख्या, पूर्णाङ्क *पङ्क्तयः)
{
	पूर्णाङ्क c = 0, l = 0;

#यदि defined(TIOCGWINSZ)
	संरचना winsize	w_win;
	यदि (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#नोचेद्यदि defined(TIOCGSIZE)
	संरचना ttysize	t_win;
	यदि (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_स्तम्भसङ्ख्या;
		l = t_win.ts_पङ्क्तयः;
	}
#यदि_अन्तः
	यदि (स्तम्भसङ्ख्या) {
		यदि (c <= 0)
			c = get_env_int("COLUMNS");
		*स्तम्भसङ्ख्या = c;
	}
	यदि (पङ्क्तयः) {
		यदि (l <= 0)
			l = get_env_int("LINES");
		*पङ्क्तयः = l;
	}
	प्रत्यागम 0;
}

पूर्णाङ्क get_terminal_विस्तारः(पूर्णाङ्क default_विस्तारः)
{
	पूर्णाङ्क विस्तारः = 0;

	get_terminal_dimension(&विस्तारः, रिक्त);

	प्रत्यागम विस्तारः > 0 ? विस्तारः : default_विस्तारः;
}

पूर्णाङ्क get_terminal_stdfd(शून्य)
{
	यदि (isatty(STDIN_FILENO))
		प्रत्यागम STDIN_FILENO;
	यदि (isatty(STDOUT_FILENO))
		प्रत्यागम STDOUT_FILENO;
	यदि (isatty(STDERR_FILENO))
		प्रत्यागम STDERR_FILENO;

	प्रत्यागम -EINVAL;
}

पूर्णाङ्क get_terminal_name(ध्रुव वर्ण **मार्गः,
		      ध्रुव वर्ण **name,
		      ध्रुव वर्ण **सङ्ख्या)
{
	ध्रुव वर्ण *tty;
	ध्रुव वर्ण *p;
	पूर्णाङ्क fd;


	यदि (name)
		*name = रिक्त;
	यदि (मार्गः)
		*मार्गः = रिक्त;
	यदि (सङ्ख्या)
		*सङ्ख्या = रिक्त;

	fd = get_terminal_stdfd();
	यदि (fd < 0)
		प्रत्यागम fd;	/* error */

	tty = ttyname(fd);
	यदि (!tty)
		प्रत्यागम -1;

	यदि (मार्गः)
		*मार्गः = tty;
	यदि (name || सङ्ख्या)
		tty = वाक्ये_तुलय(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	यदि (name)
		*name = tty;
	यदि (सङ्ख्या) {
		प्रति (p = tty; p && *p; p++) {
			यदि (अङ्कः_वा(*p)) {
				*सङ्ख्या = p;
				भङ्ग;
			}
		}
	}
	प्रत्यागम 0;
}

पूर्णाङ्क get_terminal_type(ध्रुव वर्ण **type)
{
	*type = getenv("TERM");
	यदि (*type)
		प्रत्यागम -EINVAL;
	प्रत्यागम 0;
}

वर्ण *get_terminal_default_type(ध्रुव वर्ण *ttyname, पूर्णाङ्क is_serial)
{
	यदि (ttyname) {
#यदि_परिभाषितम् HAVE_GETTTYNAM
		संरचना ttyent *ent = getttynam(ttyname);

		यदि (ent && ent->ty_type)
			प्रत्यागम strdup(ent->ty_type);
#यदि_अन्तः

#यदि defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		यदि (वाक्यं_तुलय(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			प्रत्यागम strdup(DEFAULT_TTYS0);
		अन्यथा यदि (वाक्ये_तुलय(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			प्रत्यागम strdup(DEFAULT_TTY32);
		अन्यथा यदि (वाक्यं_तुलय(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			प्रत्यागम strdup(DEFAULT_TTYS1);
#यदि_अन्तः
	}

	प्रत्यागम strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#यदि_परिभाषितम् TEST_क्रमादेशः_TTYUTILS
# include <stdlib.h>
पूर्णाङ्क मुख्यम्(शून्य)
{
	ध्रुव वर्ण *मार्गः, *name, *अङ्कः;
	पूर्णाङ्क c, l;

	यदि (get_terminal_name(&मार्गः, &name, &अङ्कः) == 0) {
		वर्ण *पदांशः;

		fprintf(stderr, "tty path:      %s\n", मार्गः);
		fprintf(stderr, "tty name:      %s\n", name);
		fprintf(stderr, "tty number:    %s\n", अङ्कः);

		fprintf(stderr, "tty term:      %s\n", getenv("TERM"));

		पदांशः = get_terminal_default_type(name, 0);
		fprintf(stderr, "tty dflt term: %s\n", पदांशः);
		मुक्त(पदांशः);
	}
	get_terminal_dimension(&c, &l);
	fprintf(stderr,         "tty cols:      %d\n", c);
	fprintf(stderr,         "tty lines:     %d\n", l);


	प्रत्यागम EXIT_SUCCESS;
}
#यदि_अन्तः /* TEST_PROGRAM_TTYUTILS */
