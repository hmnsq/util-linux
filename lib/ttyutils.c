/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ضمن <ctype.h>
#ضمن <unistd.h>

#اگر_تعریف_شده HAVE_GETTTYNAM
# include <ttyent.h>
#پایان_اگر

#ضمن "c.h"
#ضمن "ttyutils.h"

#اگر_تعریف_شده __linux__
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "linux"
#  endif
#  إذا defined (__s390__) || defined (__s390x__)
#    define DEFAULT_TTYS0  "dumb"
#    define DEFAULT_TTY32  "ibm327x"
#    define DEFAULT_TTYS1  "vt220"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#وإلاإذا defined(__GNU__)
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "hurd"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#وإلا
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#پایان_اگر

ساكن صحيح get_env_int(ثابت حرف *name)
{
	ثابت حرف *cp = getenv(name);

	إذا (cp) {
		حرف *انتها = عدم;
		دراز x;

		errno = 0;
		x = strtol(cp, &انتها, 10);

		إذا (errno == 0 && انتها && *انتها == '\0' && انتها > cp &&
		    x > 0 && x <= بیشترین_صحیح)
			أرجع x;
	}

	أرجع -1;
}

صحيح get_terminal_dimension(صحيح *تعداد_ستون, صحيح *سطرها)
{
	صحيح c = 0, l = 0;

#إذا defined(TIOCGWINSZ)
	بنية winsize	w_win;
	إذا (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#وإلاإذا defined(TIOCGSIZE)
	بنية ttysize	t_win;
	إذا (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_تعداد_ستون;
		l = t_win.ts_سطرها;
	}
#پایان_اگر
	إذا (تعداد_ستون) {
		إذا (c <= 0)
			c = get_env_int("COLUMNS");
		*تعداد_ستون = c;
	}
	إذا (سطرها) {
		إذا (l <= 0)
			l = get_env_int("LINES");
		*سطرها = l;
	}
	أرجع 0;
}

صحيح get_terminal_عرض(صحيح default_عرض)
{
	صحيح عرض = 0;

	get_terminal_dimension(&عرض, عدم);

	أرجع عرض > 0 ? عرض : default_عرض;
}

صحيح get_terminal_stdfd(فراغ)
{
	إذا (isatty(STDIN_FILENO))
		أرجع STDIN_FILENO;
	إذا (isatty(STDOUT_FILENO))
		أرجع STDOUT_FILENO;
	إذا (isatty(STDERR_FILENO))
		أرجع STDERR_FILENO;

	أرجع -EINVAL;
}

صحيح get_terminal_name(ثابت حرف **مسیر,
		      ثابت حرف **name,
		      ثابت حرف **عدد)
{
	ثابت حرف *tty;
	ثابت حرف *p;
	صحيح fd;


	إذا (name)
		*name = عدم;
	إذا (مسیر)
		*مسیر = عدم;
	إذا (عدد)
		*عدد = عدم;

	fd = get_terminal_stdfd();
	إذا (fd < 0)
		أرجع fd;	/* error */

	tty = ttyname(fd);
	إذا (!tty)
		أرجع -1;

	إذا (مسیر)
		*مسیر = tty;
	إذا (name || عدد)
		tty = مقایسه_رشته(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	إذا (name)
		*name = tty;
	إذا (عدد) {
		لكل (p = tty; p && *p; p++) {
			إذا (آیا_رقم(*p)) {
				*عدد = p;
				اكسر;
			}
		}
	}
	أرجع 0;
}

صحيح get_terminal_type(ثابت حرف **type)
{
	*type = getenv("TERM");
	إذا (*type)
		أرجع -EINVAL;
	أرجع 0;
}

حرف *get_terminal_default_type(ثابت حرف *ttyname, صحيح is_serial)
{
	إذا (ttyname) {
#اگر_تعریف_شده HAVE_GETTTYNAM
		بنية ttyent *ent = getttynam(ttyname);

		إذا (ent && ent->ty_type)
			أرجع strdup(ent->ty_type);
#پایان_اگر

#إذا defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		إذا (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			أرجع strdup(DEFAULT_TTYS0);
		وإلا إذا (مقایسه_رشته(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			أرجع strdup(DEFAULT_TTY32);
		وإلا إذا (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			أرجع strdup(DEFAULT_TTYS1);
#پایان_اگر
	}

	أرجع strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#اگر_تعریف_شده TEST_برنامه_TTYUTILS
# include <stdlib.h>
صحيح اصلی(فراغ)
{
	ثابت حرف *مسیر, *name, *رقم;
	صحيح c, l;

	إذا (get_terminal_name(&مسیر, &name, &رقم) == 0) {
		حرف *جمله;

		fprintf(stderr, "tty path:      %s\n", مسیر);
		fprintf(stderr, "tty name:      %s\n", name);
		fprintf(stderr, "tty number:    %s\n", رقم);

		fprintf(stderr, "tty term:      %s\n", getenv("TERM"));

		جمله = get_terminal_default_type(name, 0);
		fprintf(stderr, "tty dflt term: %s\n", جمله);
		حرر(جمله);
	}
	get_terminal_dimension(&c, &l);
	fprintf(stderr,         "tty cols:      %d\n", c);
	fprintf(stderr,         "tty lines:     %d\n", l);


	أرجع EXIT_SUCCESS;
}
#پایان_اگر /* TEST_PROGRAM_TTYUTILS */
