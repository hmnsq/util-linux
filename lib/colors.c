/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: 2012 Ondrej Oprala <ooprala@redhat.com>
 *          2012-2025 Karel Zak <kzak@redhat.com>
 */
#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <dirent.h>
#અન્તર્ભાવ <ctype.h>

#જો defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
# જો defined(HAVE_NCURSESW_NCURSES_H)
#  include <ncursesw/ncurses.h>
# elif defined(HAVE_NCURSES_NCURSES_H)
#  include <ncurses/ncurses.h>
# elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
# endif
# જો defined(HAVE_NCURSESW_TERM_H)
#  include <ncursesw/term.h>
# elif defined(HAVE_NCURSES_TERM_H)
#  include <ncurses/term.h>
# elif defined(HAVE_TERM_H)
#  include <term.h>
# endif
#endif

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "cctype.h"
#અન્તર્ભાવ "colors.h"
#અન્તર્ભાવ "nls.h"
#અન્તર્ભાવ "pathnames.h"
#અન્તર્ભાવ "strutils.h"

#અન્તર્ભાવ "debug.h"

/*
 * Default behavior, may be overridden by terminal-colors.d/{enable,disable}.
 */
#ifdef USE_COLORS_BY_DEFAULT
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_AUTO	/* check isatty() */
#નહીંતર
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_NEVER	/* no colors by default */
#endif

/*
 * terminal-colors.d debug stuff
 */
સ્થિર UL_DEBUG_DEFINE_MASK(termcolors);
UL_DEBUG_DEFINE_MASKNAMES(termcolors) = UL_DEBUG_EMPTY_MASKNAMES;

#define TERMCOLORS_DEBUG_INIT	(1 << 1)
#define TERMCOLORS_DEBUG_CONF	(1 << 2)
#define TERMCOLORS_DEBUG_SCHEME	(1 << 3)

#define DBG(m, x)       __UL_DBG(termcolors, TERMCOLORS_DEBUG_, m, x)
#define ON_DBG(m, x)    __UL_DBG_CALL(termcolors, TERMCOLORS_DEBUG_, m, x)

/*
 * terminal-colors.d file types
 */
ગણના {
	UL_COLORFILE_DISABLE,		/* .disable */
	UL_COLORFILE_ENABLE,		/* .enable */
	UL_COLORFILE_SCHEME,		/* .scheme */

	__UL_COLORFILE_COUNT
};

સંરચના ul_color_scheme {
	અક્ષર *name;
	અક્ષર *seq;
};

/*
 * Global colors control struct
 *
 * The terminal-colors.d/ evaluation is based on "scores":
 *
 *  filename                    score
 *  ---------------------------------------
 *  type			1
 *  @termname.type		10 + 1
 *  utilname.type		20 + 1
 *  utilname@termname.type	20 + 10 + 1
 *
 * the match with higher score wins. The score is per type.
 */
સંરચના ul_color_ctl {
	અચલ અક્ષર	*utilname;	/* util name */
	અચલ અક્ષર	*termname;	/* terminal name ($TERM) */

	અક્ષર		*sfile;	/* path to scheme */

	સંરચના ul_color_scheme	*schemes;	/* array with color schemes */
	size_t			nschemes;	/* number of the items */
	size_t			schemes_sz;	/* number of the allocated items */

	પૂર્ણાંક		mode;		/* UL_COLORMODE_* */
	unsigned પૂર્ણાંક	has_colors : 1,	/* based on mode and scores[] */
			disabled   : 1, /* disable colors */
			cs_configured : 1, /* color schemes read */
			configured : 1; /* terminal-colors.d parsed */

	પૂર્ણાંક		scores[__UL_COLORFILE_COUNT];	/* the best match */
};

/*
 * Control struct, globally shared.
 */
સ્થિર સંરચના ul_color_ctl ul_colors;

સ્થિર શૂન્ય colors_free_schemes(સંરચના ul_color_ctl *cc);
સ્થિર પૂર્ણાંક colors_read_schemes(સંરચના ul_color_ctl *cc);

/*
 * qsort/bsearch buddy
 */
સ્થિર પૂર્ણાંક cmp_scheme_name(અચલ શૂન્ય *a0, અચલ શૂન્ય *b0)
{
	અચલ સંરચના ul_color_scheme *a = (અચલ સંરચના ul_color_scheme *) a0,
				     *b = (અચલ સંરચના ul_color_scheme *) b0;
	પાછા strcmp(a->name, b->name);
}

/*
 * Resets control struct (note that we don't allocate the struct)
 */
સ્થિર શૂન્ય colors_reset(સંરચના ul_color_ctl *cc)
{
	જો (!cc)
		પાછા;

	colors_free_schemes(cc);

	મુક્ત(cc->sfile);

	cc->sfile = ખાલી;
	cc->utilname = ખાલી;
	cc->termname = ખાલી;
	cc->mode = UL_COLORMODE_UNDEF;

	memset(cc->scores, 0, કદ(cc->scores));
}

સ્થિર શૂન્ય colors_debug(સંરચના ul_color_ctl *cc)
{
	size_t i;

	જો (!cc)
		પાછા;

	printf("Colors:\n");
	printf("\tutilname = '%s'\n", cc->utilname);
	printf("\ttermname = '%s'\n", cc->termname);
	printf("\tscheme file = '%s'\n", cc->sfile);
	printf("\tmode = %s\n",
			cc->mode == UL_COLORMODE_UNDEF ? "undefined" :
			cc->mode == UL_COLORMODE_AUTO ?  "auto" :
			cc->mode == UL_COLORMODE_NEVER ? "never" :
			cc->mode == UL_COLORMODE_ALWAYS ? "always" : "???");
	printf("\thas_colors = %d\n", cc->has_colors);
	printf("\tdisabled = %d\n", cc->disabled);
	printf("\tconfigured = %d\n", cc->configured);
	printf("\tcs configured = %d\n", cc->cs_configured);

	fputc('\n', stdout);

	માટે (i = 0; i < ARRAY_SIZE(cc->scores); i++)
		printf("\tscore %s = %d\n",
				i == UL_COLORFILE_DISABLE ? "disable" :
				i == UL_COLORFILE_ENABLE ? "enable" :
				i == UL_COLORFILE_SCHEME ? "scheme" : "???",
				cc->scores[i]);

	fputc('\n', stdout);

	માટે (i = 0; i < cc->nschemes; i++) {
		printf("\tscheme #%02zu ", i);
		color_scheme_enable(cc->schemes[i].name, ખાલી);
		fputs(cc->schemes[i].name, stdout);
		color_disable();
		fputc('\n', stdout);
	}
	fputc('\n', stdout);
}

/*
 * Parses [[<utilname>][@<termname>].]<type>
 */
સ્થિર પૂર્ણાંક filename_to_tokens(અચલ અક્ષર *str,
			      અચલ અક્ષર **name, size_t *namesz,
			      અચલ અક્ષર **term, size_t *termsz,
			      પૂર્ણાંક  *filetype)
{
	અચલ અક્ષર *type_start, *term_start, *p;

	જો (!str || !*str || *str == '.' || strlen(str) > PATH_MAX)
		પાછા -EINVAL;

	/* parse .type */
	p = strrchr(str, '.');
	type_start = p ? p + 1 : str;

	જો (strcmp(type_start, "disable") == 0)
		*filetype = UL_COLORFILE_DISABLE;
	નહીંતર જો (strcmp(type_start, "enable") == 0)
		*filetype = UL_COLORFILE_ENABLE;
	નહીંતર જો (strcmp(type_start, "scheme") == 0)
		*filetype = UL_COLORFILE_SCHEME;
	નહીંતર {
		DBG(CONF, ul_debug("unknown type '%s'", type_start));
		પાછા 1;				/* unknown type */
	}

	જો (type_start == str)
		પાછા 0;				/* "type" only */

	/* parse @termname */
	p = strchr(str, '@');
	term_start = p ? p + 1 : ખાલી;
	જો (term_start) {
		*term = term_start;
		*termsz = type_start - term_start - 1;
		જો (term_start - 1 == str)
			પાછા 0;			/* "@termname.type" */
	}

	/* parse utilname */
	p = term_start ? term_start : type_start;
	*name = str;
	*namesz = p - str - 1;

	પાછા 0;
}

/*
 * Scans @dirname and select the best matches for UL_COLORFILE_* types.
 * The result is stored to cc->scores. The path to the best "scheme"
 * file is stored to cc->scheme.
 */
સ્થિર પૂર્ણાંક colors_readdir(સંરચના ul_color_ctl *cc, અચલ અક્ષર *dirname)
{
	DIR *dir;
	પૂર્ણાંક rc = -ENOENT;
	સંરચના dirent *d;
	અક્ષર sfile[PATH_MAX] = { '\0' };
	size_t namesz, termsz;

	જો (!dirname || !cc || !cc->utilname || !*cc->utilname)
		પાછા -EINVAL;

	DBG(CONF, ul_debug("reading dir: '%s'", dirname));

	dir = opendir(dirname);
	જો (!dir)
		પાછા -errno;

	namesz = strlen(cc->utilname);
	termsz = cc->termname ? strlen(cc->termname) : 0;

	જ્યારે ((d = readdir(dir))) {
		પૂર્ણાંક type, score = 1;
		અચલ અક્ષર *tk_name = ખાલી, *tk_term = ખાલી;
		size_t tk_namesz = 0, tk_termsz = 0;

		જો (*d->d_name == '.')
			ચાલુ;
#ifdef _DIRENT_HAVE_D_TYPE
		જો (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK &&
		    d->d_type != DT_REG)
			ચાલુ;
#endif
		જો (filename_to_tokens(d->d_name,
				       &tk_name, &tk_namesz,
				       &tk_term, &tk_termsz, &type) != 0)
			ચાલુ;

		/* count theoretical score before we check names to avoid
		 * unnecessary strcmp() */
		જો (tk_name)
			score += 20;
		જો (tk_term)
			score += 10;

		DBG(CONF, ul_debug("item '%s': score=%d "
			"[cur: %d, name(%zu): %s, term(%zu): %s]",
			d->d_name, score, cc->scores[type],
			tk_namesz, tk_name,
			tk_termsz, tk_term));


		જો (score < cc->scores[type])
			ચાલુ;

		/* filter out by names */
		જો (tk_namesz && (tk_namesz != namesz ||
				 strncmp(tk_name, cc->utilname, namesz) != 0))
			ચાલુ;

		જો (tk_termsz && (termsz == 0 || tk_termsz != termsz ||
				  strncmp(tk_term, cc->termname, termsz) != 0))
			ચાલુ;

		DBG(CONF, ul_debug("setting '%s' from %d -to-> %d",
					type == UL_COLORFILE_SCHEME ? "scheme" :
					type == UL_COLORFILE_DISABLE ? "disable" :
					type == UL_COLORFILE_ENABLE ? "enable" : "???",
					cc->scores[type], score));
		cc->scores[type] = score;
		rc = 0;
		જો (type == UL_COLORFILE_SCHEME)
			strncpy(sfile, d->d_name, કદ(sfile));
	}

	જો (*sfile) {
		sfile[કદ(sfile) - 1] = '\0';
		જો (asprintf(&cc->sfile, "%s/%s", dirname, sfile) <= 0)
			rc = -ENOMEM;
	}

	closedir(dir);
	પાછા rc;
}

/* atexit() wrapper */
સ્થિર શૂન્ય colors_deinit(શૂન્ય)
{
	colors_reset(&ul_colors);
}

/*
 * Returns path to $XDG_CONFIG_HOME/terminal-colors.d
 */
સ્થિર અક્ષર *colors_get_homedir(અક્ષર *buf, size_t bufsz)
{
	અક્ષર *p = getenv("XDG_CONFIG_HOME");

	જો (p) {
		snprintf(buf, bufsz, "%s/" _PATH_TERMCOLORS_DIRNAME, p);
		પાછા buf;
	}

	p = getenv("HOME");
	જો (p) {
		snprintf(buf, bufsz, "%s/.config/" _PATH_TERMCOLORS_DIRNAME, p);
		પાછા buf;
	}

	પાછા ખાલી;
}

/*
 * Adds one color sequence to array with color scheme.
 */
સ્થિર પૂર્ણાંક colors_add_scheme(સંરચના ul_color_ctl *cc,
			     અચલ અક્ષર *name,
			     અચલ અક્ષર *seq0)
{
	સંરચના ul_color_scheme *cs = ખાલી;
	અક્ષર *seq = ખાલી;
	પૂર્ણાંક rc;

	જો (!cc || !name || !*name || !seq0 || !*seq0)
		પાછા -EINVAL;

	DBG(SCHEME, ul_debug("add '%s'", name));

	seq = color_get_sequence(seq0);
	જો (!seq)
		પાછા -EINVAL;
	rc = -ENOMEM;

	/* enlarge the array */
	જો (cc->nschemes == cc->schemes_sz) {
		શૂન્ય *tmp = reallocarray(cc->schemes, cc->nschemes + 10,
					 કદ(સંરચના ul_color_scheme));
		જો (!tmp)
			જાઓ err;
		cc->schemes = tmp;
		cc->schemes_sz = cc->nschemes + 10;
	}

	/* add a new item */
	cs = &cc->schemes[cc->nschemes];
	cs->seq = seq;
	cs->name = strdup(name);
	જો (!cs->name)
		જાઓ err;

	cc->nschemes++;
	પાછા 0;
err:
	જો (cs) {
		મુક્ત(cs->seq);
		મુક્ત(cs->name);
		cs->seq = cs->name = ખાલી;
	} નહીંતર
		મુક્ત(seq);
	પાછા rc;
}

/*
 * Deallocates all regards to color schemes
 */
સ્થિર શૂન્ય colors_free_schemes(સંરચના ul_color_ctl *cc)
{
	size_t i;

	DBG(SCHEME, ul_debug("free scheme"));

	માટે (i = 0; i < cc->nschemes; i++) {
		મુક્ત(cc->schemes[i].name);
		મુક્ત(cc->schemes[i].seq);
	}

	મુક્ત(cc->schemes);
	cc->schemes = ખાલી;
	cc->nschemes = 0;
	cc->schemes_sz = 0;
}

/*
 * The scheme configuration has to be sorted for bsearch
 */
સ્થિર શૂન્ય colors_sort_schemes(સંરચના ul_color_ctl *cc)
{
	જો (!cc->nschemes)
		પાછા;

	DBG(SCHEME, ul_debug("sort scheme"));

	qsort(cc->schemes, cc->nschemes,
	      કદ(સંરચના ul_color_scheme), cmp_scheme_name);
}

/*
 * Returns just one color scheme
 */
સ્થિર સંરચના ul_color_scheme *colors_get_scheme(સંરચના ul_color_ctl *cc,
						 અચલ અક્ષર *name)
{
	સંરચના ul_color_scheme ચાવી = { .name = (અક્ષર *) name}, *res;

	જો (!cc || !name || !*name)
		પાછા ખાલી;

	જો (!cc->cs_configured) {
		પૂર્ણાંક rc = colors_read_schemes(cc);
		જો (rc)
			પાછા ખાલી;
	}
	જો (!cc->nschemes)
		પાછા ખાલી;

	DBG(SCHEME, ul_debug("search '%s'", name));

	res = bsearch(&ચાવી, cc->schemes, cc->nschemes,
				કદ(સંરચના ul_color_scheme),
				cmp_scheme_name);

	પાછા res && res->seq ? res  : ખાલી;
}

/*
 * Parses filenames in terminal-colors.d
 */
સ્થિર પૂર્ણાંક colors_read_configuration(સંરચના ul_color_ctl *cc)
{
	પૂર્ણાંક rc = -ENOENT;
	અક્ષર *dirname, buf[PATH_MAX];

	cc->termname = getenv("TERM");

	dirname = colors_get_homedir(buf, કદ(buf));
	જો (dirname)
		rc = colors_readdir(cc, dirname);		/* ~/.config */
	જો (rc == -EPERM || rc == -EACCES || rc == -ENOENT)
		rc = colors_readdir(cc, _PATH_TERMCOLORS_DIR);	/* /etc */

	cc->configured = 1;
	પાછા rc;
}

/*
 * Reads terminal-colors.d/ scheme file into array schemes
 */
સ્થિર પૂર્ણાંક colors_read_schemes(સંરચના ul_color_ctl *cc)
{
	પૂર્ણાંક rc = 0;
	FILE *f = ખાલી;
	અક્ષર buf[BUFSIZ],
	     cn[129], seq[129];

	જો (!cc->configured)
		rc = colors_read_configuration(cc);

	cc->cs_configured = 1;

	જો (rc || !cc->sfile)
		જાઓ done;

	DBG(SCHEME, ul_debug("reading file '%s'", cc->sfile));

	f = fopen(cc->sfile, "r" UL_CLOEXECSTR);
	જો (!f) {
		rc = -errno;
		જાઓ done;
	}

	જ્યારે (fgets(buf, કદ(buf), f)) {
		અક્ષર *p = strchr(buf, '\n');

		જો (!p) {
			જો (feof(f))
				p = strchr(buf, '\0');
			નહીંતર {
				rc = -errno;
				જાઓ done;
			}
		}
		*p = '\0';
		p = (અક્ષર *) skip_blank(buf);
		જો (*p == '\0' || *p == '#')
			ચાલુ;

		rc = sscanf(p, "%128[^ ] %128[^\n ]", cn, seq);
		જો (rc == 2 && *cn && *seq) {
			rc = colors_add_scheme(cc, cn, seq);	/* set rc=0 on success */
			જો (rc)
				જાઓ done;
		}
	}
	rc = 0;

done:
	જો (f)
		fclose(f);
	colors_sort_schemes(cc);

	પાછા rc;
}


સ્થિર શૂન્ય termcolors_init_debug(શૂન્ય)
{
	__UL_INIT_DEBUG_FROM_ENV(termcolors, TERMCOLORS_DEBUG_, 0, TERMINAL_COLORS_DEBUG);
}

સ્થિર પૂર્ણાંક colors_terminal_is_ready(શૂન્ય)
{
	પૂર્ણાંક ncolors = -1;

#જો defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
	{
		પૂર્ણાંક ret;

		/* setupterm() allocates memory, del_curterm() deallocates it */
		જો (setupterm(ખાલી, STDOUT_FILENO, &ret) == 0 && ret == 1) {
			ncolors = tigetnum("colors");
			del_curterm(cur_term);
		}
	}
#endif
	જો (1 < ncolors) {
		DBG(CONF, ul_debug("terminal is ready (supports %d colors)", ncolors));
		પાછા 1;
	}

	DBG(CONF, ul_debug("terminal is NOT ready (no colors)"));
	પાછા 0;
}

/**
 * colors_init:
 * @mode: UL_COLORMODE_*
 * @name: util argv[0]
 *
 * Initialize private color control struct and initialize the colors
 * status. The color schemes are parsed on demand by colors_get_scheme().
 *
 * Returns: >0 on success.
 */
પૂર્ણાંક colors_init(પૂર્ણાંક mode, અચલ અક્ષર *name)
{
	પૂર્ણાંક ready = -1;
	સંરચના ul_color_ctl *cc = &ul_colors;

	cc->utilname = name;

	termcolors_init_debug();

	જો (mode != UL_COLORMODE_ALWAYS && !isatty(STDOUT_FILENO))
		cc->mode = UL_COLORMODE_NEVER;
	નહીંતર
		cc->mode = mode;

	જો (cc->mode == UL_COLORMODE_UNDEF
	    && getenv("NO_COLOR") == ખાલી
	    && (ready = colors_terminal_is_ready())) {

		પૂર્ણાંક rc = colors_read_configuration(cc);
		જો (rc)
			cc->mode = UL_COLORMODE_DEFAULT;
		નહીંતર {

			/* evaluate scores */
			જો (cc->scores[UL_COLORFILE_DISABLE] >
			    cc->scores[UL_COLORFILE_ENABLE])
				cc->mode = UL_COLORMODE_NEVER;
			નહીંતર
				cc->mode = UL_COLORMODE_DEFAULT;

			atexit(colors_deinit);
		}
	}

	પસંદગી (cc->mode) {
	સ્થિતિ UL_COLORMODE_AUTO:
		cc->has_colors = ready == -1 ? colors_terminal_is_ready() : ready;
		તોડો;
	સ્થિતિ UL_COLORMODE_ALWAYS:
		cc->has_colors = 1;
		તોડો;
	સ્થિતિ UL_COLORMODE_NEVER:
	મૂળભૂત:
		cc->has_colors = 0;
	}

	ON_DBG(CONF, colors_debug(cc));

	પાછા cc->has_colors;
}

/*
 * Temporary disable colors (this setting is independent on terminal-colors.d/)
 */
શૂન્ય colors_off(શૂન્ય)
{
	ul_colors.disabled = 1;
}

/*
 * Enable colors
 */
શૂન્ય colors_on(શૂન્ય)
{
	ul_colors.disabled = 0;
}

/*
 * Is terminal-colors.d/ configured to use colors?
 */
પૂર્ણાંક colors_wanted(શૂન્ય)
{
	પાછા ul_colors.has_colors;
}

/*
 * Returns mode
 */
પૂર્ણાંક colors_mode(શૂન્ય)
{
	પાછા ul_colors.mode;
}

/*
 * Enable @seq color
 */
શૂન્ય color_fenable(અચલ અક્ષર *seq, FILE *f)
{
	જો (!ul_colors.disabled && ul_colors.has_colors && seq)
		fputs(seq, f);
}

/*
 * Returns escape sequence by logical @name, if undefined then returns @dflt.
 */
અચલ અક્ષર *color_scheme_get_sequence(અચલ અક્ષર *name, અચલ અક્ષર *dflt)
{
	સંરચના ul_color_scheme *cs;

	જો (ul_colors.disabled || !ul_colors.has_colors)
		પાછા ખાલી;

	cs = colors_get_scheme(&ul_colors, name);
	પાછા cs && cs->seq ? cs->seq : dflt;
}

/*
 * Enable color by logical @name, if undefined enable @dflt.
 */
શૂન્ય color_scheme_fenable(અચલ અક્ષર *name, અચલ અક્ષર *dflt, FILE *f)
{
	અચલ અક્ષર *seq = color_scheme_get_sequence(name, dflt);

	જો (!seq)
		પાછા;
	color_fenable(seq, f);
}


/*
 * Disable previously enabled color
 */
શૂન્ય color_fdisable(FILE *f)
{
	જો (!ul_colors.disabled && ul_colors.has_colors)
		fputs(UL_COLOR_RESET, f);
}

/*
 * Get reset sequence
 */
અચલ અક્ષર *color_get_disable_sequence(શૂન્ય)
{
	જો (!ul_colors.disabled && ul_colors.has_colors)
		પાછા UL_COLOR_RESET;
	નહીંતર
		પાછા "";
}

/*
 * Parses @str to return UL_COLORMODE_*
 */
પૂર્ણાંક colormode_from_string(અચલ અક્ષર *str)
{
	size_t i;
	સ્થિર અચલ અક્ષર *અચલ modes[] = {
		[UL_COLORMODE_AUTO]   = "auto",
		[UL_COLORMODE_NEVER]  = "never",
		[UL_COLORMODE_ALWAYS] = "always",
		[UL_COLORMODE_UNDEF] = ""
	};

	જો (!str || !*str)
		પાછા -EINVAL;

	પ્રતિજ્ઞા(ARRAY_SIZE(modes) == __UL_NCOLORMODES);

	માટે (i = 0; i < ARRAY_SIZE(modes); i++) {
		જો (c_strcasecmp(str, modes[i]) == 0)
			પાછા i;
	}

	પાછા -EINVAL;
}

/*
 * Parses @str and exit(EXIT_FAILURE) on error
 */
પૂર્ણાંક colormode_or_err(અચલ અક્ષર *str)
{
	અચલ અક્ષર *p = str && *str == '=' ? str + 1 : str;
	પૂર્ણાંક colormode;

	colormode = colormode_from_string(p);
	જો (colormode < 0)
		errx(EXIT_FAILURE, _("unsupported color mode: %s"), p);

	પાછા colormode;
}

#ifdef TEST_PROGRAM_COLORS
# include <getopt.h>
પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[])
{
	સ્થિર અચલ સંરચના option longopts[] = {
		{ "mode",	required_argument, ખાલી, 'm' },
		{ "color",	required_argument, ખાલી, 'c' },
		{ "color-scheme", required_argument, ખાલી, 'C' },
		{ "name",	required_argument, ખાલી, 'n' },
		{ ખાલી, 0, ખાલી, 0 }
	};
	પૂર્ણાંક c, mode = UL_COLORMODE_UNDEF;	/* default */
	અચલ અક્ષર *color = "red", *name = ખાલી, *color_scheme = ખાલી;
	અક્ષર *seq = ખાલી;

	જ્યારે ((c = getopt_long(argc, argv, "C:c:m:n:", longopts, ખાલી)) != -1) {
		પસંદગી (c) {
		સ્થિતિ 'c':
			color = optarg;
			તોડો;
		સ્થિતિ 'C':
			color_scheme = optarg;
			તોડો;
		સ્થિતિ 'm':
			mode = colormode_or_err(optarg);
			તોડો;
		સ્થિતિ 'n':
			name = optarg;
			તોડો;
		મૂળભૂત:
			fprintf(stderr, "usage: %s [options]\n"
			" -m, --mode <auto|never|always>  default is undefined\n"
			" -c, --color <red|blue|...>      color for the test message\n"
			" -C, --color-scheme <name>       color for the test message\n"
			" -n, --name <utilname>           util name\n",
			program_invocation_short_name);
			પાછા EXIT_FAILURE;
		}
	}

	colors_init(mode, name ? name : program_invocation_short_name);

	જો (color_is_sequence(color))
		seq = strdup(color);
	નહીંતર
		seq = color_get_sequence(color);

	જો (color_scheme)
		color_scheme_enable(color_scheme, seq);
	નહીંતર
		color_enable(seq);
	printf("Hello World!");
	color_disable();
	fputc('\n', stdout);

	મુક્ત(seq);

	પાછા EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_COLORS */

