/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <unistd.h>

#ifdef HAVE_GETTTYNAM
# include <ttyent.h>
#endif

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "ttyutils.h"

#ifdef __linux__
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "linux"
#  endif
#  જો defined (__s390__) || defined (__s390x__)
#    define DEFAULT_TTYS0  "dumb"
#    define DEFAULT_TTY32  "ibm327x"
#    define DEFAULT_TTYS1  "vt220"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#અથવાજો defined(__GNU__)
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "hurd"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#નહીંતર
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#endif

સ્થિર પૂર્ણાંક get_env_int(અચલ અક્ષર *name)
{
	અચલ અક્ષર *cp = getenv(name);

	જો (cp) {
		અક્ષર *end = ખાલી;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		જો (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			પાછા x;
	}

	પાછા -1;
}

પૂર્ણાંક get_terminal_dimension(પૂર્ણાંક *cols, પૂર્ણાંક *લીટીઓ)
{
	પૂર્ણાંક c = 0, l = 0;

#જો defined(TIOCGWINSZ)
	સંરચના winsize	w_win;
	જો (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#અથવાજો defined(TIOCGSIZE)
	સંરચના ttysize	t_win;
	જો (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_લીટીઓ;
	}
#endif
	જો (cols) {
		જો (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	જો (લીટીઓ) {
		જો (l <= 0)
			l = get_env_int("LINES");
		*લીટીઓ = l;
	}
	પાછા 0;
}

પૂર્ણાંક get_terminal_width(પૂર્ણાંક default_width)
{
	પૂર્ણાંક width = 0;

	get_terminal_dimension(&width, ખાલી);

	પાછા width > 0 ? width : default_width;
}

પૂર્ણાંક get_terminal_stdfd(શૂન્ય)
{
	જો (isatty(STDIN_FILENO))
		પાછા STDIN_FILENO;
	જો (isatty(STDOUT_FILENO))
		પાછા STDOUT_FILENO;
	જો (isatty(STDERR_FILENO))
		પાછા STDERR_FILENO;

	પાછા -EINVAL;
}

પૂર્ણાંક get_terminal_name(અચલ અક્ષર **path,
		      અચલ અક્ષર **name,
		      અચલ અક્ષર **સંખ્યા)
{
	અચલ અક્ષર *tty;
	અચલ અક્ષર *p;
	પૂર્ણાંક fd;


	જો (name)
		*name = ખાલી;
	જો (path)
		*path = ખાલી;
	જો (સંખ્યા)
		*સંખ્યા = ખાલી;

	fd = get_terminal_stdfd();
	જો (fd < 0)
		પાછા fd;	/* error */

	tty = ttyname(fd);
	જો (!tty)
		પાછા -1;

	જો (path)
		*path = tty;
	જો (name || સંખ્યા)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	જો (name)
		*name = tty;
	જો (સંખ્યા) {
		માટે (p = tty; p && *p; p++) {
			જો (isdigit(*p)) {
				*સંખ્યા = p;
				તોડો;
			}
		}
	}
	પાછા 0;
}

પૂર્ણાંક get_terminal_type(અચલ અક્ષર **type)
{
	*type = getenv("TERM");
	જો (*type)
		પાછા -EINVAL;
	પાછા 0;
}

અક્ષર *get_terminal_default_type(અચલ અક્ષર *ttyname, પૂર્ણાંક is_serial)
{
	જો (ttyname) {
#ifdef HAVE_GETTTYNAM
		સંરચના ttyent *ent = getttynam(ttyname);

		જો (ent && ent->ty_type)
			પાછા strdup(ent->ty_type);
#endif

#જો defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		જો (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			પાછા strdup(DEFAULT_TTYS0);
		નહીંતર જો (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			પાછા strdup(DEFAULT_TTY32);
		નહીંતર જો (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			પાછા strdup(DEFAULT_TTYS1);
#endif
	}

	પાછા strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
પૂર્ણાંક મુખ્ય(શૂન્ય)
{
	અચલ અક્ષર *path, *name, *અંક;
	પૂર્ણાંક c, l;

	જો (get_terminal_name(&path, &name, &અંક) == 0) {
		અક્ષર *term;

		fprintf(stderr, "tty path:      %s\n", path);
		fprintf(stderr, "tty name:      %s\n", name);
		fprintf(stderr, "tty number:    %s\n", અંક);

		fprintf(stderr, "tty term:      %s\n", getenv("TERM"));

		term = get_terminal_default_type(name, 0);
		fprintf(stderr, "tty dflt term: %s\n", term);
		મુક્ત(term);
	}
	get_terminal_dimension(&c, &l);
	fprintf(stderr,         "tty cols:      %d\n", c);
	fprintf(stderr,         "tty lines:     %d\n", l);


	પાછા EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
