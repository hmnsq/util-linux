/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#شامل <ctype.h>
#شامل <unistd.h>

#اگر_تعریف_شدہ HAVE_GETTTYNAM
# include <ttyent.h>
#اگر_ختم

#شامل "c.h"
#شامل "ttyutils.h"

#اگر_تعریف_شدہ __linux__
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "linux"
#  endif
#  اگر defined (__s390__) || defined (__s390x__)
#    define DEFAULT_TTYS0  "dumb"
#    define DEFAULT_TTY32  "ibm327x"
#    define DEFAULT_TTYS1  "vt220"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#وگرنہ_اگر defined(__GNU__)
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "hurd"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#ورنہ
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#اگر_ختم

ساکن صحیح get_env_int(ثابت حرف *name)
{
	ثابت حرف *cp = getenv(name);

	اگر (cp) {
		حرف *انتہا = تہی;
		طویل x;

		errno = 0;
		x = strtol(cp, &انتہا, 10);

		اگر (errno == 0 && انتہا && *انتہا == '\0' && انتہا > cp &&
		    x > 0 && x <= زیادہ_سے_زیادہ_عدد)
			واپس x;
	}

	واپس -1;
}

صحیح get_terminal_dimension(صحیح *ستون_تعداد, صحیح *سطریں)
{
	صحیح c = 0, l = 0;

#اگر defined(TIOCGWINSZ)
	ساخت winsize	w_win;
	اگر (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#وگرنہ_اگر defined(TIOCGSIZE)
	ساخت ttysize	t_win;
	اگر (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_ستون_تعداد;
		l = t_win.ts_سطریں;
	}
#اگر_ختم
	اگر (ستون_تعداد) {
		اگر (c <= 0)
			c = get_env_int("COLUMNS");
		*ستون_تعداد = c;
	}
	اگر (سطریں) {
		اگر (l <= 0)
			l = get_env_int("LINES");
		*سطریں = l;
	}
	واپس 0;
}

صحیح get_terminal_چوڑائی(صحیح default_چوڑائی)
{
	صحیح چوڑائی = 0;

	get_terminal_dimension(&چوڑائی, تہی);

	واپس چوڑائی > 0 ? چوڑائی : default_چوڑائی;
}

صحیح get_terminal_stdfd(خالی)
{
	اگر (isatty(STDIN_FILENO))
		واپس STDIN_FILENO;
	اگر (isatty(STDOUT_FILENO))
		واپس STDOUT_FILENO;
	اگر (isatty(STDERR_FILENO))
		واپس STDERR_FILENO;

	واپس -EINVAL;
}

صحیح get_terminal_name(ثابت حرف **راستہ,
		      ثابت حرف **name,
		      ثابت حرف **نمبر)
{
	ثابت حرف *tty;
	ثابت حرف *p;
	صحیح fd;


	اگر (name)
		*name = تہی;
	اگر (راستہ)
		*راستہ = تہی;
	اگر (نمبر)
		*نمبر = تہی;

	fd = get_terminal_stdfd();
	اگر (fd < 0)
		واپس fd;	/* error */

	tty = ttyname(fd);
	اگر (!tty)
		واپس -1;

	اگر (راستہ)
		*راستہ = tty;
	اگر (name || نمبر)
		tty = لڑی_موازنہ(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	اگر (name)
		*name = tty;
	اگر (نمبر) {
		برائے (p = tty; p && *p; p++) {
			اگر (ہندسہ_ہے(*p)) {
				*نمبر = p;
				توڑو;
			}
		}
	}
	واپس 0;
}

صحیح get_terminal_type(ثابت حرف **type)
{
	*type = getenv("TERM");
	اگر (*type)
		واپس -EINVAL;
	واپس 0;
}

حرف *get_terminal_default_type(ثابت حرف *ttyname, صحیح is_serial)
{
	اگر (ttyname) {
#اگر_تعریف_شدہ HAVE_GETTTYNAM
		ساخت ttyent *ent = getttynam(ttyname);

		اگر (ent && ent->ty_type)
			واپس strdup(ent->ty_type);
#اگر_ختم

#اگر defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		اگر (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			واپس strdup(DEFAULT_TTYS0);
		ورنہ اگر (لڑی_موازنہ(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			واپس strdup(DEFAULT_TTY32);
		ورنہ اگر (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			واپس strdup(DEFAULT_TTYS1);
#اگر_ختم
	}

	واپس strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#اگر_تعریف_شدہ TEST_پروگرام_TTYUTILS
# include <stdlib.h>
صحیح مین(خالی)
{
	ثابت حرف *راستہ, *name, *عدد;
	صحیح c, l;

	اگر (get_terminal_name(&راستہ, &name, &عدد) == 0) {
		حرف *رکن;

		fprintf(stderr, "tty path:      %s\n", راستہ);
		fprintf(stderr, "tty name:      %s\n", name);
		fprintf(stderr, "tty number:    %s\n", عدد);

		fprintf(stderr, "tty term:      %s\n", getenv("TERM"));

		رکن = get_terminal_default_type(name, 0);
		fprintf(stderr, "tty dflt term: %s\n", رکن);
		آزاد(رکن);
	}
	get_terminal_dimension(&c, &l);
	fprintf(stderr,         "tty cols:      %d\n", c);
	fprintf(stderr,         "tty lines:     %d\n", l);


	واپس EXIT_SUCCESS;
}
#اگر_ختم /* TEST_PROGRAM_TTYUTILS */
