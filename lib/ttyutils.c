/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ضمن <ctype.h>
#ضمن <unistd.h>

#جيڪڏهن_وصف_ٿيل HAVE_GETTTYNAM
# include <ttyent.h>
#جيڪڏهن_پورو

#ضمن "c.h"
#ضمن "ttyutils.h"

#جيڪڏهن_وصف_ٿيل __linux__
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
#جيڪڏهن_پورو

ساكن صحيح get_env_int(ثابت حرف *name)
{
	ثابت حرف *cp = getenv(name);

	إذا (cp) {
		حرف *پڇاڙي = عدم;
		ڊگهو x;

		errno = 0;
		x = strtol(cp, &پڇاڙي, 10);

		إذا (errno == 0 && پڇاڙي && *پڇاڙي == '\0' && پڇاڙي > cp &&
		    x > 0 && x <= وڌ_کان_وڌ_عدد)
			أرجع x;
	}

	أرجع -1;
}

صحيح get_terminal_dimension(صحيح *کانن_ڳڻپ, صحيح *سٽون)
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
		c = t_win.ts_کانن_ڳڻپ;
		l = t_win.ts_سٽون;
	}
#جيڪڏهن_پورو
	إذا (کانن_ڳڻپ) {
		إذا (c <= 0)
			c = get_env_int("COLUMNS");
		*کانن_ڳڻپ = c;
	}
	إذا (سٽون) {
		إذا (l <= 0)
			l = get_env_int("LINES");
		*سٽون = l;
	}
	أرجع 0;
}

صحيح get_terminal_ويڪر(صحيح default_ويڪر)
{
	صحيح ويڪر = 0;

	get_terminal_dimension(&ويڪر, عدم);

	أرجع ويڪر > 0 ? ويڪر : default_ويڪر;
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

صحيح get_terminal_name(ثابت حرف **رستو,
		      ثابت حرف **name,
		      ثابت حرف **عدد)
{
	ثابت حرف *tty;
	ثابت حرف *p;
	صحيح fd;


	إذا (name)
		*name = عدم;
	إذا (رستو)
		*رستو = عدم;
	إذا (عدد)
		*عدد = عدم;

	fd = get_terminal_stdfd();
	إذا (fd < 0)
		أرجع fd;	/* error */

	tty = ttyname(fd);
	إذا (!tty)
		أرجع -1;

	إذا (رستو)
		*رستو = tty;
	إذا (name || عدد)
		tty = لڙي_ڀيٽ(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	إذا (name)
		*name = tty;
	إذا (عدد) {
		لكل (p = tty; p && *p; p++) {
			إذا (انگ_آهي(*p)) {
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
#جيڪڏهن_وصف_ٿيل HAVE_GETTTYNAM
		بنية ttyent *ent = getttynam(ttyname);

		إذا (ent && ent->ty_type)
			أرجع strdup(ent->ty_type);
#جيڪڏهن_پورو

#إذا defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		إذا (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			أرجع strdup(DEFAULT_TTYS0);
		وإلا إذا (لڙي_ڀيٽ(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			أرجع strdup(DEFAULT_TTY32);
		وإلا إذا (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			أرجع strdup(DEFAULT_TTYS1);
#جيڪڏهن_پورو
	}

	أرجع strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#جيڪڏهن_وصف_ٿيل TEST_پروگرام_TTYUTILS
# include <stdlib.h>
صحيح مکيه(فراغ)
{
	ثابت حرف *رستو, *name, *انگ;
	صحيح c, l;

	إذا (get_terminal_name(&رستو, &name, &انگ) == 0) {
		حرف *رڪن;

		fprintf(stderr, "tty path:      %s\n", رستو);
		fprintf(stderr, "tty name:      %s\n", name);
		fprintf(stderr, "tty number:    %s\n", انگ);

		fprintf(stderr, "tty term:      %s\n", getenv("TERM"));

		رڪن = get_terminal_default_type(name, 0);
		fprintf(stderr, "tty dflt term: %s\n", رڪن);
		حرر(رڪن);
	}
	get_terminal_dimension(&c, &l);
	fprintf(stderr,         "tty cols:      %d\n", c);
	fprintf(stderr,         "tty lines:     %d\n", l);


	أرجع EXIT_SUCCESS;
}
#جيڪڏهن_پورو /* TEST_PROGRAM_TTYUTILS */
