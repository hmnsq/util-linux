/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: 2012 Ondrej Oprala <ooprala@redhat.com>
 *          2012-2025 Karel Zak <kzak@redhat.com>
 */
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>

#ᮚᮦᮔ᮪ defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
# ᮚᮦᮔ᮪ defined(HAVE_NCURSESW_NCURSES_H)
#  include <ncursesw/ncurses.h>
# elif defined(HAVE_NCURSES_NCURSES_H)
#  include <ncurses/ncurses.h>
# elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
# endif
# ᮚᮦᮔ᮪ defined(HAVE_NCURSESW_TERM_H)
#  include <ncursesw/term.h>
# elif defined(HAVE_NCURSES_TERM_H)
#  include <ncurses/term.h>
# elif defined(HAVE_TERM_H)
#  include <term.h>
# endif
#endif

#include "c.h"
#include "cctype.h"
#include "colors.h"
#include "nls.h"
#include "pathnames.h"
#include "strutils.h"

#include "debug.h"

/*
 * Default behavior, may be overridden by terminal-colors.d/{enable,disable}.
 */
#ifdef USE_COLORS_BY_DEFAULT
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_AUTO	/* check isatty() */
#ᮜᮤᮚ
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_NEVER	/* no colors by default */
#endif

/*
 * terminal-colors.d debug stuff
 */
static UL_DEBUG_DEFINE_MASK(termcolors);
UL_DEBUG_DEFINE_MASKNAMES(termcolors) = UL_DEBUG_EMPTY_MASKNAMES;

#define TERMCOLORS_DEBUG_INIT	(1 << 1)
#define TERMCOLORS_DEBUG_CONF	(1 << 2)
#define TERMCOLORS_DEBUG_SCHEME	(1 << 3)

#define DBG(m, x)       __UL_DBG(termcolors, TERMCOLORS_DEBUG_, m, x)
#define ON_DBG(m, x)    __UL_DBG_CALL(termcolors, TERMCOLORS_DEBUG_, m, x)

/*
 * terminal-colors.d file types
 */
ga_nanaa {
	UL_COLORFILE_DISABLE,		/* .disable */
	UL_COLORFILE_ENABLE,		/* .enable */
	UL_COLORFILE_SCHEME,		/* .scheme */

	__UL_COLORFILE_COUNT
};

sa_nracanaa ul_color_scheme {
	var_na *name;
	var_na *seq;
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
sa_nracanaa ul_color_ctl {
	const var_na	*utilname;	/* util name */
	const var_na	*termname;	/* terminal name ($TERM) */

	var_na		*sfile;	/* path to scheme */

	sa_nracanaa ul_color_scheme	*schemes;	/* array with color schemes */
	size_t			nschemes;	/* number of the items */
	size_t			schemes_sz;	/* number of the allocated items */

	puur_naa_nka		mode;		/* UL_COLORMODE_* */
	unsigned puur_naa_nka	has_colors : 1,	/* based on mode and scores[] */
			disabled   : 1, /* disable colors */
			cs_configured : 1, /* color schemes read */
			configured : 1; /* terminal-colors.d parsed */

	puur_naa_nka		scores[__UL_COLORFILE_COUNT];	/* the best match */
};

/*
 * Control struct, globally shared.
 */
static sa_nracanaa ul_color_ctl ul_colors;

static void colors_free_schemes(sa_nracanaa ul_color_ctl *cc);
static puur_naa_nka colors_read_schemes(sa_nracanaa ul_color_ctl *cc);

/*
 * qsort/bsearch buddy
 */
static puur_naa_nka cmp_scheme_name(const void *a0, const void *b0)
{
	const sa_nracanaa ul_color_scheme *a = (const sa_nracanaa ul_color_scheme *) a0,
				     *b = (const sa_nracanaa ul_color_scheme *) b0;
	ᮝᮜᮤ strcmp(a->name, b->name);
}

/*
 * Resets control struct (note that we don't allocate the struct)
 */
static void colors_reset(sa_nracanaa ul_color_ctl *cc)
{
	ᮚᮦᮔ᮪ (!cc)
		ᮝᮜᮤ;

	colors_free_schemes(cc);

	free(cc->sfile);

	cc->sfile = NULL;
	cc->utilname = NULL;
	cc->termname = NULL;
	cc->mode = UL_COLORMODE_UNDEF;

	memset(cc->scores, 0, parimaa_na(cc->scores));
}

static void colors_debug(sa_nracanaa ul_color_ctl *cc)
{
	size_t i;

	ᮚᮦᮔ᮪ (!cc)
		ᮝᮜᮤ;

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

	for (i = 0; i < ARRAY_SIZE(cc->scores); i++)
		printf("\tscore %s = %d\n",
				i == UL_COLORFILE_DISABLE ? "disable" :
				i == UL_COLORFILE_ENABLE ? "enable" :
				i == UL_COLORFILE_SCHEME ? "scheme" : "???",
				cc->scores[i]);

	fputc('\n', stdout);

	for (i = 0; i < cc->nschemes; i++) {
		printf("\tscheme #%02zu ", i);
		color_scheme_enable(cc->schemes[i].name, NULL);
		fputs(cc->schemes[i].name, stdout);
		color_disable();
		fputc('\n', stdout);
	}
	fputc('\n', stdout);
}

/*
 * Parses [[<utilname>][@<termname>].]<type>
 */
static puur_naa_nka filename_to_tokens(const var_na *str,
			      const var_na **name, size_t *namesz,
			      const var_na **term, size_t *termsz,
			      puur_naa_nka  *filetype)
{
	const var_na *type_start, *term_start, *p;

	ᮚᮦᮔ᮪ (!str || !*str || *str == '.' || strlen(str) > PATH_MAX)
		ᮝᮜᮤ -EINVAL;

	/* parse .type */
	p = strrchr(str, '.');
	type_start = p ? p + 1 : str;

	ᮚᮦᮔ᮪ (strcmp(type_start, "disable") == 0)
		*filetype = UL_COLORFILE_DISABLE;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(type_start, "enable") == 0)
		*filetype = UL_COLORFILE_ENABLE;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(type_start, "scheme") == 0)
		*filetype = UL_COLORFILE_SCHEME;
	ᮜᮤᮚ {
		DBG(CONF, ul_debug("unknown type '%s'", type_start));
		ᮝᮜᮤ 1;				/* unknown type */
	}

	ᮚᮦᮔ᮪ (type_start == str)
		ᮝᮜᮤ 0;				/* "type" only */

	/* parse @termname */
	p = strchr(str, '@');
	term_start = p ? p + 1 : NULL;
	ᮚᮦᮔ᮪ (term_start) {
		*term = term_start;
		*termsz = type_start - term_start - 1;
		ᮚᮦᮔ᮪ (term_start - 1 == str)
			ᮝᮜᮤ 0;			/* "@termname.type" */
	}

	/* parse utilname */
	p = term_start ? term_start : type_start;
	*name = str;
	*namesz = p - str - 1;

	ᮝᮜᮤ 0;
}

/*
 * Scans @dirname and select the best matches for UL_COLORFILE_* types.
 * The result is stored to cc->scores. The path to the best "scheme"
 * file is stored to cc->scheme.
 */
static puur_naa_nka colors_readdir(sa_nracanaa ul_color_ctl *cc, const var_na *dirname)
{
	DIR *dir;
	puur_naa_nka rc = -ENOENT;
	sa_nracanaa dirent *d;
	var_na sfile[PATH_MAX] = { '\0' };
	size_t namesz, termsz;

	ᮚᮦᮔ᮪ (!dirname || !cc || !cc->utilname || !*cc->utilname)
		ᮝᮜᮤ -EINVAL;

	DBG(CONF, ul_debug("reading dir: '%s'", dirname));

	dir = opendir(dirname);
	ᮚᮦᮔ᮪ (!dir)
		ᮝᮜᮤ -errno;

	namesz = strlen(cc->utilname);
	termsz = cc->termname ? strlen(cc->termname) : 0;

	ᮞᮨᮓᮨᮜ ((d = readdir(dir))) {
		puur_naa_nka type, score = 1;
		const var_na *tk_name = NULL, *tk_term = NULL;
		size_t tk_namesz = 0, tk_termsz = 0;

		ᮚᮦᮔ᮪ (*d->d_name == '.')
			continue;
#ifdef _DIRENT_HAVE_D_TYPE
		ᮚᮦᮔ᮪ (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK &&
		    d->d_type != DT_REG)
			continue;
#endif
		ᮚᮦᮔ᮪ (filename_to_tokens(d->d_name,
				       &tk_name, &tk_namesz,
				       &tk_term, &tk_termsz, &type) != 0)
			continue;

		/* count theoretical score before we check names to avoid
		 * unnecessary strcmp() */
		ᮚᮦᮔ᮪ (tk_name)
			score += 20;
		ᮚᮦᮔ᮪ (tk_term)
			score += 10;

		DBG(CONF, ul_debug("item '%s': score=%d "
			"[cur: %d, name(%zu): %s, term(%zu): %s]",
			d->d_name, score, cc->scores[type],
			tk_namesz, tk_name,
			tk_termsz, tk_term));


		ᮚᮦᮔ᮪ (score < cc->scores[type])
			continue;

		/* filter out by names */
		ᮚᮦᮔ᮪ (tk_namesz && (tk_namesz != namesz ||
				 strncmp(tk_name, cc->utilname, namesz) != 0))
			continue;

		ᮚᮦᮔ᮪ (tk_termsz && (termsz == 0 || tk_termsz != termsz ||
				  strncmp(tk_term, cc->termname, termsz) != 0))
			continue;

		DBG(CONF, ul_debug("setting '%s' from %d -to-> %d",
					type == UL_COLORFILE_SCHEME ? "scheme" :
					type == UL_COLORFILE_DISABLE ? "disable" :
					type == UL_COLORFILE_ENABLE ? "enable" : "???",
					cc->scores[type], score));
		cc->scores[type] = score;
		rc = 0;
		ᮚᮦᮔ᮪ (type == UL_COLORFILE_SCHEME)
			strncpy(sfile, d->d_name, parimaa_na(sfile));
	}

	ᮚᮦᮔ᮪ (*sfile) {
		sfile[parimaa_na(sfile) - 1] = '\0';
		ᮚᮦᮔ᮪ (asprintf(&cc->sfile, "%s/%s", dirname, sfile) <= 0)
			rc = -ENOMEM;
	}

	closedir(dir);
	ᮝᮜᮤ rc;
}

/* atexit() wrapper */
static void colors_deinit(void)
{
	colors_reset(&ul_colors);
}

/*
 * Returns path to $XDG_CONFIG_HOME/terminal-colors.d
 */
static var_na *colors_get_homedir(var_na *buf, size_t bufsz)
{
	var_na *p = getenv("XDG_CONFIG_HOME");

	ᮚᮦᮔ᮪ (p) {
		snprintf(buf, bufsz, "%s/" _PATH_TERMCOLORS_DIRNAME, p);
		ᮝᮜᮤ buf;
	}

	p = getenv("HOME");
	ᮚᮦᮔ᮪ (p) {
		snprintf(buf, bufsz, "%s/.config/" _PATH_TERMCOLORS_DIRNAME, p);
		ᮝᮜᮤ buf;
	}

	ᮝᮜᮤ NULL;
}

/*
 * Adds one color sequence to array with color scheme.
 */
static puur_naa_nka colors_add_scheme(sa_nracanaa ul_color_ctl *cc,
			     const var_na *name,
			     const var_na *seq0)
{
	sa_nracanaa ul_color_scheme *cs = NULL;
	var_na *seq = NULL;
	puur_naa_nka rc;

	ᮚᮦᮔ᮪ (!cc || !name || !*name || !seq0 || !*seq0)
		ᮝᮜᮤ -EINVAL;

	DBG(SCHEME, ul_debug("add '%s'", name));

	seq = color_get_sequence(seq0);
	ᮚᮦᮔ᮪ (!seq)
		ᮝᮜᮤ -EINVAL;
	rc = -ENOMEM;

	/* enlarge the array */
	ᮚᮦᮔ᮪ (cc->nschemes == cc->schemes_sz) {
		void *tmp = reallocarray(cc->schemes, cc->nschemes + 10,
					 parimaa_na(sa_nracanaa ul_color_scheme));
		ᮚᮦᮔ᮪ (!tmp)
			goto err;
		cc->schemes = tmp;
		cc->schemes_sz = cc->nschemes + 10;
	}

	/* add a new item */
	cs = &cc->schemes[cc->nschemes];
	cs->seq = seq;
	cs->name = strdup(name);
	ᮚᮦᮔ᮪ (!cs->name)
		goto err;

	cc->nschemes++;
	ᮝᮜᮤ 0;
err:
	ᮚᮦᮔ᮪ (cs) {
		free(cs->seq);
		free(cs->name);
		cs->seq = cs->name = NULL;
	} ᮜᮤᮚ
		free(seq);
	ᮝᮜᮤ rc;
}

/*
 * Deallocates all regards to color schemes
 */
static void colors_free_schemes(sa_nracanaa ul_color_ctl *cc)
{
	size_t i;

	DBG(SCHEME, ul_debug("free scheme"));

	for (i = 0; i < cc->nschemes; i++) {
		free(cc->schemes[i].name);
		free(cc->schemes[i].seq);
	}

	free(cc->schemes);
	cc->schemes = NULL;
	cc->nschemes = 0;
	cc->schemes_sz = 0;
}

/*
 * The scheme configuration has to be sorted for bsearch
 */
static void colors_sort_schemes(sa_nracanaa ul_color_ctl *cc)
{
	ᮚᮦᮔ᮪ (!cc->nschemes)
		ᮝᮜᮤ;

	DBG(SCHEME, ul_debug("sort scheme"));

	qsort(cc->schemes, cc->nschemes,
	      parimaa_na(sa_nracanaa ul_color_scheme), cmp_scheme_name);
}

/*
 * Returns just one color scheme
 */
static sa_nracanaa ul_color_scheme *colors_get_scheme(sa_nracanaa ul_color_ctl *cc,
						 const var_na *name)
{
	sa_nracanaa ul_color_scheme key = { .name = (var_na *) name}, *res;

	ᮚᮦᮔ᮪ (!cc || !name || !*name)
		ᮝᮜᮤ NULL;

	ᮚᮦᮔ᮪ (!cc->cs_configured) {
		puur_naa_nka rc = colors_read_schemes(cc);
		ᮚᮦᮔ᮪ (rc)
			ᮝᮜᮤ NULL;
	}
	ᮚᮦᮔ᮪ (!cc->nschemes)
		ᮝᮜᮤ NULL;

	DBG(SCHEME, ul_debug("search '%s'", name));

	res = bsearch(&key, cc->schemes, cc->nschemes,
				parimaa_na(sa_nracanaa ul_color_scheme),
				cmp_scheme_name);

	ᮝᮜᮤ res && res->seq ? res  : NULL;
}

/*
 * Parses filenames in terminal-colors.d
 */
static puur_naa_nka colors_read_configuration(sa_nracanaa ul_color_ctl *cc)
{
	puur_naa_nka rc = -ENOENT;
	var_na *dirname, buf[PATH_MAX];

	cc->termname = getenv("TERM");

	dirname = colors_get_homedir(buf, parimaa_na(buf));
	ᮚᮦᮔ᮪ (dirname)
		rc = colors_readdir(cc, dirname);		/* ~/.config */
	ᮚᮦᮔ᮪ (rc == -EPERM || rc == -EACCES || rc == -ENOENT)
		rc = colors_readdir(cc, _PATH_TERMCOLORS_DIR);	/* /etc */

	cc->configured = 1;
	ᮝᮜᮤ rc;
}

/*
 * Reads terminal-colors.d/ scheme file into array schemes
 */
static puur_naa_nka colors_read_schemes(sa_nracanaa ul_color_ctl *cc)
{
	puur_naa_nka rc = 0;
	FILE *f = NULL;
	var_na buf[BUFSIZ],
	     cn[129], seq[129];

	ᮚᮦᮔ᮪ (!cc->configured)
		rc = colors_read_configuration(cc);

	cc->cs_configured = 1;

	ᮚᮦᮔ᮪ (rc || !cc->sfile)
		goto done;

	DBG(SCHEME, ul_debug("reading file '%s'", cc->sfile));

	f = fopen(cc->sfile, "r" UL_CLOEXECSTR);
	ᮚᮦᮔ᮪ (!f) {
		rc = -errno;
		goto done;
	}

	ᮞᮨᮓᮨᮜ (fgets(buf, parimaa_na(buf), f)) {
		var_na *p = strchr(buf, '\n');

		ᮚᮦᮔ᮪ (!p) {
			ᮚᮦᮔ᮪ (feof(f))
				p = strchr(buf, '\0');
			ᮜᮤᮚ {
				rc = -errno;
				goto done;
			}
		}
		*p = '\0';
		p = (var_na *) skip_blank(buf);
		ᮚᮦᮔ᮪ (*p == '\0' || *p == '#')
			continue;

		rc = sscanf(p, "%128[^ ] %128[^\n ]", cn, seq);
		ᮚᮦᮔ᮪ (rc == 2 && *cn && *seq) {
			rc = colors_add_scheme(cc, cn, seq);	/* set rc=0 on success */
			ᮚᮦᮔ᮪ (rc)
				goto done;
		}
	}
	rc = 0;

done:
	ᮚᮦᮔ᮪ (f)
		fclose(f);
	colors_sort_schemes(cc);

	ᮝᮜᮤ rc;
}


static void termcolors_init_debug(void)
{
	__UL_INIT_DEBUG_FROM_ENV(termcolors, TERMCOLORS_DEBUG_, 0, TERMINAL_COLORS_DEBUG);
}

static puur_naa_nka colors_terminal_is_ready(void)
{
	puur_naa_nka ncolors = -1;

#ᮚᮦᮔ᮪ defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
	{
		puur_naa_nka ret;

		/* setupterm() allocates memory, del_curterm() deallocates it */
		ᮚᮦᮔ᮪ (setupterm(NULL, STDOUT_FILENO, &ret) == 0 && ret == 1) {
			ncolors = tigetnum("colors");
			del_curterm(cur_term);
		}
	}
#endif
	ᮚᮦᮔ᮪ (1 < ncolors) {
		DBG(CONF, ul_debug("terminal is ready (supports %d colors)", ncolors));
		ᮝᮜᮤ 1;
	}

	DBG(CONF, ul_debug("terminal is NOT ready (no colors)"));
	ᮝᮜᮤ 0;
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
puur_naa_nka colors_init(puur_naa_nka mode, const var_na *name)
{
	puur_naa_nka ready = -1;
	sa_nracanaa ul_color_ctl *cc = &ul_colors;

	cc->utilname = name;

	termcolors_init_debug();

	ᮚᮦᮔ᮪ (mode != UL_COLORMODE_ALWAYS && !isatty(STDOUT_FILENO))
		cc->mode = UL_COLORMODE_NEVER;
	ᮜᮤᮚ
		cc->mode = mode;

	ᮚᮦᮔ᮪ (cc->mode == UL_COLORMODE_UNDEF
	    && getenv("NO_COLOR") == NULL
	    && (ready = colors_terminal_is_ready())) {

		puur_naa_nka rc = colors_read_configuration(cc);
		ᮚᮦᮔ᮪ (rc)
			cc->mode = UL_COLORMODE_DEFAULT;
		ᮜᮤᮚ {

			/* evaluate scores */
			ᮚᮦᮔ᮪ (cc->scores[UL_COLORFILE_DISABLE] >
			    cc->scores[UL_COLORFILE_ENABLE])
				cc->mode = UL_COLORMODE_NEVER;
			ᮜᮤᮚ
				cc->mode = UL_COLORMODE_DEFAULT;

			atexit(colors_deinit);
		}
	}

	switch (cc->mode) {
	pak_sa UL_COLORMODE_AUTO:
		cc->has_colors = ready == -1 ? colors_terminal_is_ready() : ready;
		bha_nga;
	pak_sa UL_COLORMODE_ALWAYS:
		cc->has_colors = 1;
		bha_nga;
	pak_sa UL_COLORMODE_NEVER:
	default:
		cc->has_colors = 0;
	}

	ON_DBG(CONF, colors_debug(cc));

	ᮝᮜᮤ cc->has_colors;
}

/*
 * Temporary disable colors (this setting is independent on terminal-colors.d/)
 */
void colors_off(void)
{
	ul_colors.disabled = 1;
}

/*
 * Enable colors
 */
void colors_on(void)
{
	ul_colors.disabled = 0;
}

/*
 * Is terminal-colors.d/ configured to use colors?
 */
puur_naa_nka colors_wanted(void)
{
	ᮝᮜᮤ ul_colors.has_colors;
}

/*
 * Returns mode
 */
puur_naa_nka colors_mode(void)
{
	ᮝᮜᮤ ul_colors.mode;
}

/*
 * Enable @seq color
 */
void color_fenable(const var_na *seq, FILE *f)
{
	ᮚᮦᮔ᮪ (!ul_colors.disabled && ul_colors.has_colors && seq)
		fputs(seq, f);
}

/*
 * Returns escape sequence by logical @name, if undefined then returns @dflt.
 */
const var_na *color_scheme_get_sequence(const var_na *name, const var_na *dflt)
{
	sa_nracanaa ul_color_scheme *cs;

	ᮚᮦᮔ᮪ (ul_colors.disabled || !ul_colors.has_colors)
		ᮝᮜᮤ NULL;

	cs = colors_get_scheme(&ul_colors, name);
	ᮝᮜᮤ cs && cs->seq ? cs->seq : dflt;
}

/*
 * Enable color by logical @name, if undefined enable @dflt.
 */
void color_scheme_fenable(const var_na *name, const var_na *dflt, FILE *f)
{
	const var_na *seq = color_scheme_get_sequence(name, dflt);

	ᮚᮦᮔ᮪ (!seq)
		ᮝᮜᮤ;
	color_fenable(seq, f);
}


/*
 * Disable previously enabled color
 */
void color_fdisable(FILE *f)
{
	ᮚᮦᮔ᮪ (!ul_colors.disabled && ul_colors.has_colors)
		fputs(UL_COLOR_RESET, f);
}

/*
 * Get reset sequence
 */
const var_na *color_get_disable_sequence(void)
{
	ᮚᮦᮔ᮪ (!ul_colors.disabled && ul_colors.has_colors)
		ᮝᮜᮤ UL_COLOR_RESET;
	ᮜᮤᮚ
		ᮝᮜᮤ "";
}

/*
 * Parses @str to return UL_COLORMODE_*
 */
puur_naa_nka colormode_from_string(const var_na *str)
{
	size_t i;
	static const var_na *const modes[] = {
		[UL_COLORMODE_AUTO]   = "auto",
		[UL_COLORMODE_NEVER]  = "never",
		[UL_COLORMODE_ALWAYS] = "always",
		[UL_COLORMODE_UNDEF] = ""
	};

	ᮚᮦᮔ᮪ (!str || !*str)
		ᮝᮜᮤ -EINVAL;

	assert(ARRAY_SIZE(modes) == __UL_NCOLORMODES);

	for (i = 0; i < ARRAY_SIZE(modes); i++) {
		ᮚᮦᮔ᮪ (c_strcasecmp(str, modes[i]) == 0)
			ᮝᮜᮤ i;
	}

	ᮝᮜᮤ -EINVAL;
}

/*
 * Parses @str and exit(EXIT_FAILURE) on error
 */
puur_naa_nka colormode_or_err(const var_na *str)
{
	const var_na *p = str && *str == '=' ? str + 1 : str;
	puur_naa_nka colormode;

	colormode = colormode_from_string(p);
	ᮚᮦᮔ᮪ (colormode < 0)
		errx(EXIT_FAILURE, _("unsupported color mode: %s"), p);

	ᮝᮜᮤ colormode;
}

#ifdef TEST_PROGRAM_COLORS
# include <getopt.h>
puur_naa_nka main(puur_naa_nka argc, var_na *argv[])
{
	static const sa_nracanaa option longopts[] = {
		{ "mode",	required_argument, NULL, 'm' },
		{ "color",	required_argument, NULL, 'c' },
		{ "color-scheme", required_argument, NULL, 'C' },
		{ "name",	required_argument, NULL, 'n' },
		{ NULL, 0, NULL, 0 }
	};
	puur_naa_nka c, mode = UL_COLORMODE_UNDEF;	/* default */
	const var_na *color = "red", *name = NULL, *color_scheme = NULL;
	var_na *seq = NULL;

	ᮞᮨᮓᮨᮜ ((c = getopt_long(argc, argv, "C:c:m:n:", longopts, NULL)) != -1) {
		switch (c) {
		pak_sa 'c':
			color = optarg;
			bha_nga;
		pak_sa 'C':
			color_scheme = optarg;
			bha_nga;
		pak_sa 'm':
			mode = colormode_or_err(optarg);
			bha_nga;
		pak_sa 'n':
			name = optarg;
			bha_nga;
		default:
			fprintf(stderr, "usage: %s [options]\n"
			" -m, --mode <auto|never|always>  default is undefined\n"
			" -c, --color <red|blue|...>      color for the test message\n"
			" -C, --color-scheme <name>       color for the test message\n"
			" -n, --name <utilname>           util name\n",
			program_invocation_short_name);
			ᮝᮜᮤ EXIT_FAILURE;
		}
	}

	colors_init(mode, name ? name : program_invocation_short_name);

	ᮚᮦᮔ᮪ (color_is_sequence(color))
		seq = strdup(color);
	ᮜᮤᮚ
		seq = color_get_sequence(color);

	ᮚᮦᮔ᮪ (color_scheme)
		color_scheme_enable(color_scheme, seq);
	ᮜᮤᮚ
		color_enable(seq);
	printf("Hello World!");
	color_disable();
	fputc('\n', stdout);

	free(seq);

	ᮝᮜᮤ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_COLORS */

