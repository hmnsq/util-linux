/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: 2012 Ondrej Oprala <ooprala@redhat.com>
 *          2012-2025 Karel Zak <kzak@redhat.com>
 */
#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <dirent.h>
#அந்தர்பாவ <ctype.h>

#எனில் defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
# எனில் defined(HAVE_NCURSESW_NCURSES_H)
#  include <ncursesw/ncurses.h>
# elif defined(HAVE_NCURSES_NCURSES_H)
#  include <ncurses/ncurses.h>
# elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
# endif
# எனில் defined(HAVE_NCURSESW_TERM_H)
#  include <ncursesw/term.h>
# elif defined(HAVE_NCURSES_TERM_H)
#  include <ncurses/term.h>
# elif defined(HAVE_TERM_H)
#  include <term.h>
# endif
#endif

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "cctype.h"
#அந்தர்பாவ "colors.h"
#அந்தர்பாவ "nls.h"
#அந்தர்பாவ "pathnames.h"
#அந்தர்பாவ "strutils.h"

#அந்தர்பாவ "debug.h"

/*
 * Default behavior, may be overridden by terminal-colors.d/{enable,disable}.
 */
#ifdef USE_COLORS_BY_DEFAULT
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_AUTO	/* check isatty() */
#இல்லையெனில்
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_NEVER	/* no colors by default */
#endif

/*
 * terminal-colors.d debug stuff
 */
நிலையான UL_DEBUG_DEFINE_MASK(termcolors);
UL_DEBUG_DEFINE_MASKNAMES(termcolors) = UL_DEBUG_EMPTY_MASKNAMES;

#define TERMCOLORS_DEBUG_INIT	(1 << 1)
#define TERMCOLORS_DEBUG_CONF	(1 << 2)
#define TERMCOLORS_DEBUG_SCHEME	(1 << 3)

#define DBG(m, x)       __UL_DBG(termcolors, TERMCOLORS_DEBUG_, m, x)
#define ON_DBG(m, x)    __UL_DBG_CALL(termcolors, TERMCOLORS_DEBUG_, m, x)

/*
 * terminal-colors.d file types
 */
எண்ணிக்கை {
	UL_COLORFILE_DISABLE,		/* .disable */
	UL_COLORFILE_ENABLE,		/* .enable */
	UL_COLORFILE_SCHEME,		/* .scheme */

	__UL_COLORFILE_COUNT
};

கட்டமைப்பு ul_color_scheme {
	எழுத்து *name;
	எழுத்து *seq;
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
கட்டமைப்பு ul_color_ctl {
	மாறிலி எழுத்து	*utilname;	/* util name */
	மாறிலி எழுத்து	*termname;	/* terminal name ($TERM) */

	எழுத்து		*sfile;	/* path to scheme */

	கட்டமைப்பு ul_color_scheme	*schemes;	/* array with color schemes */
	size_t			nschemes;	/* number of the items */
	size_t			schemes_sz;	/* number of the allocated items */

	முழு		mode;		/* UL_COLORMODE_* */
	unsigned முழு	has_colors : 1,	/* based on mode and scores[] */
			disabled   : 1, /* disable colors */
			cs_configured : 1, /* color schemes read */
			configured : 1; /* terminal-colors.d parsed */

	முழு		scores[__UL_COLORFILE_COUNT];	/* the best match */
};

/*
 * Control struct, globally shared.
 */
நிலையான கட்டமைப்பு ul_color_ctl ul_colors;

நிலையான வெறுமை colors_free_schemes(கட்டமைப்பு ul_color_ctl *cc);
நிலையான முழு colors_read_schemes(கட்டமைப்பு ul_color_ctl *cc);

/*
 * qsort/bsearch buddy
 */
நிலையான முழு cmp_scheme_name(மாறிலி வெறுமை *a0, மாறிலி வெறுமை *b0)
{
	மாறிலி கட்டமைப்பு ul_color_scheme *a = (மாறிலி கட்டமைப்பு ul_color_scheme *) a0,
				     *b = (மாறிலி கட்டமைப்பு ul_color_scheme *) b0;
	திரும்பு strcmp(a->name, b->name);
}

/*
 * Resets control struct (note that we don't allocate the struct)
 */
நிலையான வெறுமை colors_reset(கட்டமைப்பு ul_color_ctl *cc)
{
	எனில் (!cc)
		திரும்பு;

	colors_free_schemes(cc);

	விடுவி(cc->sfile);

	cc->sfile = வெற்று;
	cc->utilname = வெற்று;
	cc->termname = வெற்று;
	cc->mode = UL_COLORMODE_UNDEF;

	memset(cc->scores, 0, அளவு(cc->scores));
}

நிலையான வெறுமை colors_debug(கட்டமைப்பு ul_color_ctl *cc)
{
	size_t i;

	எனில் (!cc)
		திரும்பு;

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

	க்கு (i = 0; i < ARRAY_SIZE(cc->scores); i++)
		printf("\tscore %s = %d\n",
				i == UL_COLORFILE_DISABLE ? "disable" :
				i == UL_COLORFILE_ENABLE ? "enable" :
				i == UL_COLORFILE_SCHEME ? "scheme" : "???",
				cc->scores[i]);

	fputc('\n', stdout);

	க்கு (i = 0; i < cc->nschemes; i++) {
		printf("\tscheme #%02zu ", i);
		color_scheme_enable(cc->schemes[i].name, வெற்று);
		fputs(cc->schemes[i].name, stdout);
		color_disable();
		fputc('\n', stdout);
	}
	fputc('\n', stdout);
}

/*
 * Parses [[<utilname>][@<termname>].]<type>
 */
நிலையான முழு filename_to_tokens(மாறிலி எழுத்து *str,
			      மாறிலி எழுத்து **name, size_t *namesz,
			      மாறிலி எழுத்து **term, size_t *termsz,
			      முழு  *filetype)
{
	மாறிலி எழுத்து *type_start, *term_start, *p;

	எனில் (!str || !*str || *str == '.' || strlen(str) > PATH_MAX)
		திரும்பு -EINVAL;

	/* parse .type */
	p = strrchr(str, '.');
	type_start = p ? p + 1 : str;

	எனில் (strcmp(type_start, "disable") == 0)
		*filetype = UL_COLORFILE_DISABLE;
	இல்லையெனில் எனில் (strcmp(type_start, "enable") == 0)
		*filetype = UL_COLORFILE_ENABLE;
	இல்லையெனில் எனில் (strcmp(type_start, "scheme") == 0)
		*filetype = UL_COLORFILE_SCHEME;
	இல்லையெனில் {
		DBG(CONF, ul_debug("unknown type '%s'", type_start));
		திரும்பு 1;				/* unknown type */
	}

	எனில் (type_start == str)
		திரும்பு 0;				/* "type" only */

	/* parse @termname */
	p = strchr(str, '@');
	term_start = p ? p + 1 : வெற்று;
	எனில் (term_start) {
		*term = term_start;
		*termsz = type_start - term_start - 1;
		எனில் (term_start - 1 == str)
			திரும்பு 0;			/* "@termname.type" */
	}

	/* parse utilname */
	p = term_start ? term_start : type_start;
	*name = str;
	*namesz = p - str - 1;

	திரும்பு 0;
}

/*
 * Scans @dirname and select the best matches for UL_COLORFILE_* types.
 * The result is stored to cc->scores. The path to the best "scheme"
 * file is stored to cc->scheme.
 */
நிலையான முழு colors_readdir(கட்டமைப்பு ul_color_ctl *cc, மாறிலி எழுத்து *dirname)
{
	DIR *dir;
	முழு rc = -ENOENT;
	கட்டமைப்பு dirent *d;
	எழுத்து sfile[PATH_MAX] = { '\0' };
	size_t namesz, termsz;

	எனில் (!dirname || !cc || !cc->utilname || !*cc->utilname)
		திரும்பு -EINVAL;

	DBG(CONF, ul_debug("reading dir: '%s'", dirname));

	dir = opendir(dirname);
	எனில் (!dir)
		திரும்பு -errno;

	namesz = strlen(cc->utilname);
	termsz = cc->termname ? strlen(cc->termname) : 0;

	போது ((d = readdir(dir))) {
		முழு type, score = 1;
		மாறிலி எழுத்து *tk_name = வெற்று, *tk_term = வெற்று;
		size_t tk_namesz = 0, tk_termsz = 0;

		எனில் (*d->d_name == '.')
			தொடர்;
#ifdef _DIRENT_HAVE_D_TYPE
		எனில் (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK &&
		    d->d_type != DT_REG)
			தொடர்;
#endif
		எனில் (filename_to_tokens(d->d_name,
				       &tk_name, &tk_namesz,
				       &tk_term, &tk_termsz, &type) != 0)
			தொடர்;

		/* count theoretical score before we check names to avoid
		 * unnecessary strcmp() */
		எனில் (tk_name)
			score += 20;
		எனில் (tk_term)
			score += 10;

		DBG(CONF, ul_debug("item '%s': score=%d "
			"[cur: %d, name(%zu): %s, term(%zu): %s]",
			d->d_name, score, cc->scores[type],
			tk_namesz, tk_name,
			tk_termsz, tk_term));


		எனில் (score < cc->scores[type])
			தொடர்;

		/* filter out by names */
		எனில் (tk_namesz && (tk_namesz != namesz ||
				 strncmp(tk_name, cc->utilname, namesz) != 0))
			தொடர்;

		எனில் (tk_termsz && (termsz == 0 || tk_termsz != termsz ||
				  strncmp(tk_term, cc->termname, termsz) != 0))
			தொடர்;

		DBG(CONF, ul_debug("setting '%s' from %d -to-> %d",
					type == UL_COLORFILE_SCHEME ? "scheme" :
					type == UL_COLORFILE_DISABLE ? "disable" :
					type == UL_COLORFILE_ENABLE ? "enable" : "???",
					cc->scores[type], score));
		cc->scores[type] = score;
		rc = 0;
		எனில் (type == UL_COLORFILE_SCHEME)
			strncpy(sfile, d->d_name, அளவு(sfile));
	}

	எனில் (*sfile) {
		sfile[அளவு(sfile) - 1] = '\0';
		எனில் (asprintf(&cc->sfile, "%s/%s", dirname, sfile) <= 0)
			rc = -ENOMEM;
	}

	closedir(dir);
	திரும்பு rc;
}

/* atexit() wrapper */
நிலையான வெறுமை colors_deinit(வெறுமை)
{
	colors_reset(&ul_colors);
}

/*
 * Returns path to $XDG_CONFIG_HOME/terminal-colors.d
 */
நிலையான எழுத்து *colors_get_homedir(எழுத்து *buf, size_t bufsz)
{
	எழுத்து *p = getenv("XDG_CONFIG_HOME");

	எனில் (p) {
		snprintf(buf, bufsz, "%s/" _PATH_TERMCOLORS_DIRNAME, p);
		திரும்பு buf;
	}

	p = getenv("HOME");
	எனில் (p) {
		snprintf(buf, bufsz, "%s/.config/" _PATH_TERMCOLORS_DIRNAME, p);
		திரும்பு buf;
	}

	திரும்பு வெற்று;
}

/*
 * Adds one color sequence to array with color scheme.
 */
நிலையான முழு colors_add_scheme(கட்டமைப்பு ul_color_ctl *cc,
			     மாறிலி எழுத்து *name,
			     மாறிலி எழுத்து *seq0)
{
	கட்டமைப்பு ul_color_scheme *cs = வெற்று;
	எழுத்து *seq = வெற்று;
	முழு rc;

	எனில் (!cc || !name || !*name || !seq0 || !*seq0)
		திரும்பு -EINVAL;

	DBG(SCHEME, ul_debug("add '%s'", name));

	seq = color_get_sequence(seq0);
	எனில் (!seq)
		திரும்பு -EINVAL;
	rc = -ENOMEM;

	/* enlarge the array */
	எனில் (cc->nschemes == cc->schemes_sz) {
		வெறுமை *tmp = reallocarray(cc->schemes, cc->nschemes + 10,
					 அளவு(கட்டமைப்பு ul_color_scheme));
		எனில் (!tmp)
			செல் err;
		cc->schemes = tmp;
		cc->schemes_sz = cc->nschemes + 10;
	}

	/* add a new item */
	cs = &cc->schemes[cc->nschemes];
	cs->seq = seq;
	cs->name = strdup(name);
	எனில் (!cs->name)
		செல் err;

	cc->nschemes++;
	திரும்பு 0;
err:
	எனில் (cs) {
		விடுவி(cs->seq);
		விடுவி(cs->name);
		cs->seq = cs->name = வெற்று;
	} இல்லையெனில்
		விடுவி(seq);
	திரும்பு rc;
}

/*
 * Deallocates all regards to color schemes
 */
நிலையான வெறுமை colors_free_schemes(கட்டமைப்பு ul_color_ctl *cc)
{
	size_t i;

	DBG(SCHEME, ul_debug("free scheme"));

	க்கு (i = 0; i < cc->nschemes; i++) {
		விடுவி(cc->schemes[i].name);
		விடுவி(cc->schemes[i].seq);
	}

	விடுவி(cc->schemes);
	cc->schemes = வெற்று;
	cc->nschemes = 0;
	cc->schemes_sz = 0;
}

/*
 * The scheme configuration has to be sorted for bsearch
 */
நிலையான வெறுமை colors_sort_schemes(கட்டமைப்பு ul_color_ctl *cc)
{
	எனில் (!cc->nschemes)
		திரும்பு;

	DBG(SCHEME, ul_debug("sort scheme"));

	qsort(cc->schemes, cc->nschemes,
	      அளவு(கட்டமைப்பு ul_color_scheme), cmp_scheme_name);
}

/*
 * Returns just one color scheme
 */
நிலையான கட்டமைப்பு ul_color_scheme *colors_get_scheme(கட்டமைப்பு ul_color_ctl *cc,
						 மாறிலி எழுத்து *name)
{
	கட்டமைப்பு ul_color_scheme சாவி = { .name = (எழுத்து *) name}, *res;

	எனில் (!cc || !name || !*name)
		திரும்பு வெற்று;

	எனில் (!cc->cs_configured) {
		முழு rc = colors_read_schemes(cc);
		எனில் (rc)
			திரும்பு வெற்று;
	}
	எனில் (!cc->nschemes)
		திரும்பு வெற்று;

	DBG(SCHEME, ul_debug("search '%s'", name));

	res = bsearch(&சாவி, cc->schemes, cc->nschemes,
				அளவு(கட்டமைப்பு ul_color_scheme),
				cmp_scheme_name);

	திரும்பு res && res->seq ? res  : வெற்று;
}

/*
 * Parses filenames in terminal-colors.d
 */
நிலையான முழு colors_read_configuration(கட்டமைப்பு ul_color_ctl *cc)
{
	முழு rc = -ENOENT;
	எழுத்து *dirname, buf[PATH_MAX];

	cc->termname = getenv("TERM");

	dirname = colors_get_homedir(buf, அளவு(buf));
	எனில் (dirname)
		rc = colors_readdir(cc, dirname);		/* ~/.config */
	எனில் (rc == -EPERM || rc == -EACCES || rc == -ENOENT)
		rc = colors_readdir(cc, _PATH_TERMCOLORS_DIR);	/* /etc */

	cc->configured = 1;
	திரும்பு rc;
}

/*
 * Reads terminal-colors.d/ scheme file into array schemes
 */
நிலையான முழு colors_read_schemes(கட்டமைப்பு ul_color_ctl *cc)
{
	முழு rc = 0;
	FILE *f = வெற்று;
	எழுத்து buf[BUFSIZ],
	     cn[129], seq[129];

	எனில் (!cc->configured)
		rc = colors_read_configuration(cc);

	cc->cs_configured = 1;

	எனில் (rc || !cc->sfile)
		செல் done;

	DBG(SCHEME, ul_debug("reading file '%s'", cc->sfile));

	f = fopen(cc->sfile, "r" UL_CLOEXECSTR);
	எனில் (!f) {
		rc = -errno;
		செல் done;
	}

	போது (fgets(buf, அளவு(buf), f)) {
		எழுத்து *p = strchr(buf, '\n');

		எனில் (!p) {
			எனில் (feof(f))
				p = strchr(buf, '\0');
			இல்லையெனில் {
				rc = -errno;
				செல் done;
			}
		}
		*p = '\0';
		p = (எழுத்து *) skip_blank(buf);
		எனில் (*p == '\0' || *p == '#')
			தொடர்;

		rc = sscanf(p, "%128[^ ] %128[^\n ]", cn, seq);
		எனில் (rc == 2 && *cn && *seq) {
			rc = colors_add_scheme(cc, cn, seq);	/* set rc=0 on success */
			எனில் (rc)
				செல் done;
		}
	}
	rc = 0;

done:
	எனில் (f)
		fclose(f);
	colors_sort_schemes(cc);

	திரும்பு rc;
}


நிலையான வெறுமை termcolors_init_debug(வெறுமை)
{
	__UL_INIT_DEBUG_FROM_ENV(termcolors, TERMCOLORS_DEBUG_, 0, TERMINAL_COLORS_DEBUG);
}

நிலையான முழு colors_terminal_is_ready(வெறுமை)
{
	முழு ncolors = -1;

#எனில் defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
	{
		முழு ret;

		/* setupterm() allocates memory, del_curterm() deallocates it */
		எனில் (setupterm(வெற்று, STDOUT_FILENO, &ret) == 0 && ret == 1) {
			ncolors = tigetnum("colors");
			del_curterm(cur_term);
		}
	}
#endif
	எனில் (1 < ncolors) {
		DBG(CONF, ul_debug("terminal is ready (supports %d colors)", ncolors));
		திரும்பு 1;
	}

	DBG(CONF, ul_debug("terminal is NOT ready (no colors)"));
	திரும்பு 0;
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
முழு colors_init(முழு mode, மாறிலி எழுத்து *name)
{
	முழு ready = -1;
	கட்டமைப்பு ul_color_ctl *cc = &ul_colors;

	cc->utilname = name;

	termcolors_init_debug();

	எனில் (mode != UL_COLORMODE_ALWAYS && !isatty(STDOUT_FILENO))
		cc->mode = UL_COLORMODE_NEVER;
	இல்லையெனில்
		cc->mode = mode;

	எனில் (cc->mode == UL_COLORMODE_UNDEF
	    && getenv("NO_COLOR") == வெற்று
	    && (ready = colors_terminal_is_ready())) {

		முழு rc = colors_read_configuration(cc);
		எனில் (rc)
			cc->mode = UL_COLORMODE_DEFAULT;
		இல்லையெனில் {

			/* evaluate scores */
			எனில் (cc->scores[UL_COLORFILE_DISABLE] >
			    cc->scores[UL_COLORFILE_ENABLE])
				cc->mode = UL_COLORMODE_NEVER;
			இல்லையெனில்
				cc->mode = UL_COLORMODE_DEFAULT;

			atexit(colors_deinit);
		}
	}

	தேர்வு (cc->mode) {
	நிலை UL_COLORMODE_AUTO:
		cc->has_colors = ready == -1 ? colors_terminal_is_ready() : ready;
		நிறுத்து;
	நிலை UL_COLORMODE_ALWAYS:
		cc->has_colors = 1;
		நிறுத்து;
	நிலை UL_COLORMODE_NEVER:
	இயல்பு:
		cc->has_colors = 0;
	}

	ON_DBG(CONF, colors_debug(cc));

	திரும்பு cc->has_colors;
}

/*
 * Temporary disable colors (this setting is independent on terminal-colors.d/)
 */
வெறுமை colors_off(வெறுமை)
{
	ul_colors.disabled = 1;
}

/*
 * Enable colors
 */
வெறுமை colors_on(வெறுமை)
{
	ul_colors.disabled = 0;
}

/*
 * Is terminal-colors.d/ configured to use colors?
 */
முழு colors_wanted(வெறுமை)
{
	திரும்பு ul_colors.has_colors;
}

/*
 * Returns mode
 */
முழு colors_mode(வெறுமை)
{
	திரும்பு ul_colors.mode;
}

/*
 * Enable @seq color
 */
வெறுமை color_fenable(மாறிலி எழுத்து *seq, FILE *f)
{
	எனில் (!ul_colors.disabled && ul_colors.has_colors && seq)
		fputs(seq, f);
}

/*
 * Returns escape sequence by logical @name, if undefined then returns @dflt.
 */
மாறிலி எழுத்து *color_scheme_get_sequence(மாறிலி எழுத்து *name, மாறிலி எழுத்து *dflt)
{
	கட்டமைப்பு ul_color_scheme *cs;

	எனில் (ul_colors.disabled || !ul_colors.has_colors)
		திரும்பு வெற்று;

	cs = colors_get_scheme(&ul_colors, name);
	திரும்பு cs && cs->seq ? cs->seq : dflt;
}

/*
 * Enable color by logical @name, if undefined enable @dflt.
 */
வெறுமை color_scheme_fenable(மாறிலி எழுத்து *name, மாறிலி எழுத்து *dflt, FILE *f)
{
	மாறிலி எழுத்து *seq = color_scheme_get_sequence(name, dflt);

	எனில் (!seq)
		திரும்பு;
	color_fenable(seq, f);
}


/*
 * Disable previously enabled color
 */
வெறுமை color_fdisable(FILE *f)
{
	எனில் (!ul_colors.disabled && ul_colors.has_colors)
		fputs(UL_COLOR_RESET, f);
}

/*
 * Get reset sequence
 */
மாறிலி எழுத்து *color_get_disable_sequence(வெறுமை)
{
	எனில் (!ul_colors.disabled && ul_colors.has_colors)
		திரும்பு UL_COLOR_RESET;
	இல்லையெனில்
		திரும்பு "";
}

/*
 * Parses @str to return UL_COLORMODE_*
 */
முழு colormode_from_string(மாறிலி எழுத்து *str)
{
	size_t i;
	நிலையான மாறிலி எழுத்து *மாறிலி modes[] = {
		[UL_COLORMODE_AUTO]   = "auto",
		[UL_COLORMODE_NEVER]  = "never",
		[UL_COLORMODE_ALWAYS] = "always",
		[UL_COLORMODE_UNDEF] = ""
	};

	எனில் (!str || !*str)
		திரும்பு -EINVAL;

	ப்ரதிஜ்ஞா(ARRAY_SIZE(modes) == __UL_NCOLORMODES);

	க்கு (i = 0; i < ARRAY_SIZE(modes); i++) {
		எனில் (c_strcasecmp(str, modes[i]) == 0)
			திரும்பு i;
	}

	திரும்பு -EINVAL;
}

/*
 * Parses @str and exit(EXIT_FAILURE) on error
 */
முழு colormode_or_err(மாறிலி எழுத்து *str)
{
	மாறிலி எழுத்து *p = str && *str == '=' ? str + 1 : str;
	முழு colormode;

	colormode = colormode_from_string(p);
	எனில் (colormode < 0)
		errx(EXIT_FAILURE, _("unsupported color mode: %s"), p);

	திரும்பு colormode;
}

#ifdef TEST_PROGRAM_COLORS
# include <getopt.h>
முழு முதன்மை(முழு argc, எழுத்து *argv[])
{
	நிலையான மாறிலி கட்டமைப்பு option longopts[] = {
		{ "mode",	required_argument, வெற்று, 'm' },
		{ "color",	required_argument, வெற்று, 'c' },
		{ "color-scheme", required_argument, வெற்று, 'C' },
		{ "name",	required_argument, வெற்று, 'n' },
		{ வெற்று, 0, வெற்று, 0 }
	};
	முழு c, mode = UL_COLORMODE_UNDEF;	/* default */
	மாறிலி எழுத்து *color = "red", *name = வெற்று, *color_scheme = வெற்று;
	எழுத்து *seq = வெற்று;

	போது ((c = getopt_long(argc, argv, "C:c:m:n:", longopts, வெற்று)) != -1) {
		தேர்வு (c) {
		நிலை 'c':
			color = optarg;
			நிறுத்து;
		நிலை 'C':
			color_scheme = optarg;
			நிறுத்து;
		நிலை 'm':
			mode = colormode_or_err(optarg);
			நிறுத்து;
		நிலை 'n':
			name = optarg;
			நிறுத்து;
		இயல்பு:
			fprintf(stderr, "usage: %s [options]\n"
			" -m, --mode <auto|never|always>  default is undefined\n"
			" -c, --color <red|blue|...>      color for the test message\n"
			" -C, --color-scheme <name>       color for the test message\n"
			" -n, --name <utilname>           util name\n",
			program_invocation_short_name);
			திரும்பு EXIT_FAILURE;
		}
	}

	colors_init(mode, name ? name : program_invocation_short_name);

	எனில் (color_is_sequence(color))
		seq = strdup(color);
	இல்லையெனில்
		seq = color_get_sequence(color);

	எனில் (color_scheme)
		color_scheme_enable(color_scheme, seq);
	இல்லையெனில்
		color_enable(seq);
	printf("Hello World!");
	color_disable();
	fputc('\n', stdout);

	விடுவி(seq);

	திரும்பு EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_COLORS */

