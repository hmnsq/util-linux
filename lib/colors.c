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

#ако defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
# ако defined(HAVE_NCURSESW_NCURSES_H)
#  include <ncursesw/ncurses.h>
# elif defined(HAVE_NCURSES_NCURSES_H)
#  include <ncurses/ncurses.h>
# elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
# endif
# ако defined(HAVE_NCURSESW_TERM_H)
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
#иначе
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
enum {
	UL_COLORFILE_DISABLE,		/* .disable */
	UL_COLORFILE_ENABLE,		/* .enable */
	UL_COLORFILE_SCHEME,		/* .scheme */

	__UL_COLORFILE_COUNT
};

struct ul_color_scheme {
	char *name;
	char *seq;
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
struct ul_color_ctl {
	const char	*utilname;	/* util name */
	const char	*termname;	/* terminal name ($TERM) */

	char		*sfile;	/* path to scheme */

	struct ul_color_scheme	*schemes;	/* array with color schemes */
	size_t			nschemes;	/* number of the items */
	size_t			schemes_sz;	/* number of the allocated items */

	цяло		mode;		/* UL_COLORMODE_* */
	unsigned цяло	has_colors : 1,	/* based on mode and scores[] */
			disabled   : 1, /* disable colors */
			cs_configured : 1, /* color schemes read */
			configured : 1; /* terminal-colors.d parsed */

	цяло		scores[__UL_COLORFILE_COUNT];	/* the best match */
};

/*
 * Control struct, globally shared.
 */
static struct ul_color_ctl ul_colors;

static void colors_free_schemes(struct ul_color_ctl *cc);
static цяло colors_read_schemes(struct ul_color_ctl *cc);

/*
 * qsort/bsearch buddy
 */
static цяло cmp_scheme_name(const void *a0, const void *b0)
{
	const struct ul_color_scheme *a = (const struct ul_color_scheme *) a0,
				     *b = (const struct ul_color_scheme *) b0;
	върни strcmp(a->name, b->name);
}

/*
 * Resets control struct (note that we don't allocate the struct)
 */
static void colors_reset(struct ul_color_ctl *cc)
{
	ако (!cc)
		върни;

	colors_free_schemes(cc);

	free(cc->sfile);

	cc->sfile = NULL;
	cc->utilname = NULL;
	cc->termname = NULL;
	cc->mode = UL_COLORMODE_UNDEF;

	memset(cc->scores, 0, sizeof(cc->scores));
}

static void colors_debug(struct ul_color_ctl *cc)
{
	size_t i;

	ако (!cc)
		върни;

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

	за (i = 0; i < ARRAY_SIZE(cc->scores); i++)
		printf("\tscore %s = %d\n",
				i == UL_COLORFILE_DISABLE ? "disable" :
				i == UL_COLORFILE_ENABLE ? "enable" :
				i == UL_COLORFILE_SCHEME ? "scheme" : "???",
				cc->scores[i]);

	fputc('\n', stdout);

	за (i = 0; i < cc->nschemes; i++) {
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
static цяло filename_to_tokens(const char *str,
			      const char **name, size_t *namesz,
			      const char **term, size_t *termsz,
			      цяло  *filetype)
{
	const char *type_start, *term_start, *p;

	ако (!str || !*str || *str == '.' || strlen(str) > PATH_MAX)
		върни -EINVAL;

	/* parse .type */
	p = strrchr(str, '.');
	type_start = p ? p + 1 : str;

	ако (strcmp(type_start, "disable") == 0)
		*filetype = UL_COLORFILE_DISABLE;
	иначе ако (strcmp(type_start, "enable") == 0)
		*filetype = UL_COLORFILE_ENABLE;
	иначе ако (strcmp(type_start, "scheme") == 0)
		*filetype = UL_COLORFILE_SCHEME;
	иначе {
		DBG(CONF, ul_debug("unknown type '%s'", type_start));
		върни 1;				/* unknown type */
	}

	ако (type_start == str)
		върни 0;				/* "type" only */

	/* parse @termname */
	p = strchr(str, '@');
	term_start = p ? p + 1 : NULL;
	ако (term_start) {
		*term = term_start;
		*termsz = type_start - term_start - 1;
		ако (term_start - 1 == str)
			върни 0;			/* "@termname.type" */
	}

	/* parse utilname */
	p = term_start ? term_start : type_start;
	*name = str;
	*namesz = p - str - 1;

	върни 0;
}

/*
 * Scans @dirname and select the best matches for UL_COLORFILE_* types.
 * The result is stored to cc->scores. The path to the best "scheme"
 * file is stored to cc->scheme.
 */
static цяло colors_readdir(struct ul_color_ctl *cc, const char *dirname)
{
	DIR *dir;
	цяло rc = -ENOENT;
	struct dirent *d;
	char sfile[PATH_MAX] = { '\0' };
	size_t namesz, termsz;

	ако (!dirname || !cc || !cc->utilname || !*cc->utilname)
		върни -EINVAL;

	DBG(CONF, ul_debug("reading dir: '%s'", dirname));

	dir = opendir(dirname);
	ако (!dir)
		върни -errno;

	namesz = strlen(cc->utilname);
	termsz = cc->termname ? strlen(cc->termname) : 0;

	докато ((d = readdir(dir))) {
		цяло type, score = 1;
		const char *tk_name = NULL, *tk_term = NULL;
		size_t tk_namesz = 0, tk_termsz = 0;

		ако (*d->d_name == '.')
			continue;
#ifdef _DIRENT_HAVE_D_TYPE
		ако (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK &&
		    d->d_type != DT_REG)
			continue;
#endif
		ако (filename_to_tokens(d->d_name,
				       &tk_name, &tk_namesz,
				       &tk_term, &tk_termsz, &type) != 0)
			continue;

		/* count theoretical score before we check names to avoid
		 * unnecessary strcmp() */
		ако (tk_name)
			score += 20;
		ако (tk_term)
			score += 10;

		DBG(CONF, ul_debug("item '%s': score=%d "
			"[cur: %d, name(%zu): %s, term(%zu): %s]",
			d->d_name, score, cc->scores[type],
			tk_namesz, tk_name,
			tk_termsz, tk_term));


		ако (score < cc->scores[type])
			continue;

		/* filter out by names */
		ако (tk_namesz && (tk_namesz != namesz ||
				 strncmp(tk_name, cc->utilname, namesz) != 0))
			continue;

		ако (tk_termsz && (termsz == 0 || tk_termsz != termsz ||
				  strncmp(tk_term, cc->termname, termsz) != 0))
			continue;

		DBG(CONF, ul_debug("setting '%s' from %d -to-> %d",
					type == UL_COLORFILE_SCHEME ? "scheme" :
					type == UL_COLORFILE_DISABLE ? "disable" :
					type == UL_COLORFILE_ENABLE ? "enable" : "???",
					cc->scores[type], score));
		cc->scores[type] = score;
		rc = 0;
		ако (type == UL_COLORFILE_SCHEME)
			strncpy(sfile, d->d_name, sizeof(sfile));
	}

	ако (*sfile) {
		sfile[sizeof(sfile) - 1] = '\0';
		ако (asprintf(&cc->sfile, "%s/%s", dirname, sfile) <= 0)
			rc = -ENOMEM;
	}

	closedir(dir);
	върни rc;
}

/* atexit() wrapper */
static void colors_deinit(void)
{
	colors_reset(&ul_colors);
}

/*
 * Returns path to $XDG_CONFIG_HOME/terminal-colors.d
 */
static char *colors_get_homedir(char *buf, size_t bufsz)
{
	char *p = getenv("XDG_CONFIG_HOME");

	ако (p) {
		snprintf(buf, bufsz, "%s/" _PATH_TERMCOLORS_DIRNAME, p);
		върни buf;
	}

	p = getenv("HOME");
	ако (p) {
		snprintf(buf, bufsz, "%s/.config/" _PATH_TERMCOLORS_DIRNAME, p);
		върни buf;
	}

	върни NULL;
}

/*
 * Adds one color sequence to array with color scheme.
 */
static цяло colors_add_scheme(struct ul_color_ctl *cc,
			     const char *name,
			     const char *seq0)
{
	struct ul_color_scheme *cs = NULL;
	char *seq = NULL;
	цяло rc;

	ако (!cc || !name || !*name || !seq0 || !*seq0)
		върни -EINVAL;

	DBG(SCHEME, ul_debug("add '%s'", name));

	seq = color_get_sequence(seq0);
	ако (!seq)
		върни -EINVAL;
	rc = -ENOMEM;

	/* enlarge the array */
	ако (cc->nschemes == cc->schemes_sz) {
		void *tmp = reallocarray(cc->schemes, cc->nschemes + 10,
					 sizeof(struct ul_color_scheme));
		ако (!tmp)
			goto err;
		cc->schemes = tmp;
		cc->schemes_sz = cc->nschemes + 10;
	}

	/* add a new item */
	cs = &cc->schemes[cc->nschemes];
	cs->seq = seq;
	cs->name = strdup(name);
	ако (!cs->name)
		goto err;

	cc->nschemes++;
	върни 0;
err:
	ако (cs) {
		free(cs->seq);
		free(cs->name);
		cs->seq = cs->name = NULL;
	} иначе
		free(seq);
	върни rc;
}

/*
 * Deallocates all regards to color schemes
 */
static void colors_free_schemes(struct ul_color_ctl *cc)
{
	size_t i;

	DBG(SCHEME, ul_debug("free scheme"));

	за (i = 0; i < cc->nschemes; i++) {
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
static void colors_sort_schemes(struct ul_color_ctl *cc)
{
	ако (!cc->nschemes)
		върни;

	DBG(SCHEME, ul_debug("sort scheme"));

	qsort(cc->schemes, cc->nschemes,
	      sizeof(struct ul_color_scheme), cmp_scheme_name);
}

/*
 * Returns just one color scheme
 */
static struct ul_color_scheme *colors_get_scheme(struct ul_color_ctl *cc,
						 const char *name)
{
	struct ul_color_scheme key = { .name = (char *) name}, *res;

	ако (!cc || !name || !*name)
		върни NULL;

	ако (!cc->cs_configured) {
		цяло rc = colors_read_schemes(cc);
		ако (rc)
			върни NULL;
	}
	ако (!cc->nschemes)
		върни NULL;

	DBG(SCHEME, ul_debug("search '%s'", name));

	res = bsearch(&key, cc->schemes, cc->nschemes,
				sizeof(struct ul_color_scheme),
				cmp_scheme_name);

	върни res && res->seq ? res  : NULL;
}

/*
 * Parses filenames in terminal-colors.d
 */
static цяло colors_read_configuration(struct ul_color_ctl *cc)
{
	цяло rc = -ENOENT;
	char *dirname, buf[PATH_MAX];

	cc->termname = getenv("TERM");

	dirname = colors_get_homedir(buf, sizeof(buf));
	ако (dirname)
		rc = colors_readdir(cc, dirname);		/* ~/.config */
	ако (rc == -EPERM || rc == -EACCES || rc == -ENOENT)
		rc = colors_readdir(cc, _PATH_TERMCOLORS_DIR);	/* /etc */

	cc->configured = 1;
	върни rc;
}

/*
 * Reads terminal-colors.d/ scheme file into array schemes
 */
static цяло colors_read_schemes(struct ul_color_ctl *cc)
{
	цяло rc = 0;
	FILE *f = NULL;
	char buf[BUFSIZ],
	     cn[129], seq[129];

	ако (!cc->configured)
		rc = colors_read_configuration(cc);

	cc->cs_configured = 1;

	ако (rc || !cc->sfile)
		goto done;

	DBG(SCHEME, ul_debug("reading file '%s'", cc->sfile));

	f = fopen(cc->sfile, "r" UL_CLOEXECSTR);
	ако (!f) {
		rc = -errno;
		goto done;
	}

	докато (fgets(buf, sizeof(buf), f)) {
		char *p = strchr(buf, '\n');

		ако (!p) {
			ако (feof(f))
				p = strchr(buf, '\0');
			иначе {
				rc = -errno;
				goto done;
			}
		}
		*p = '\0';
		p = (char *) skip_blank(buf);
		ако (*p == '\0' || *p == '#')
			continue;

		rc = sscanf(p, "%128[^ ] %128[^\n ]", cn, seq);
		ако (rc == 2 && *cn && *seq) {
			rc = colors_add_scheme(cc, cn, seq);	/* set rc=0 on success */
			ако (rc)
				goto done;
		}
	}
	rc = 0;

done:
	ако (f)
		fclose(f);
	colors_sort_schemes(cc);

	върни rc;
}


static void termcolors_init_debug(void)
{
	__UL_INIT_DEBUG_FROM_ENV(termcolors, TERMCOLORS_DEBUG_, 0, TERMINAL_COLORS_DEBUG);
}

static цяло colors_terminal_is_ready(void)
{
	цяло ncolors = -1;

#ако defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
	{
		цяло ret;

		/* setupterm() allocates memory, del_curterm() deallocates it */
		ако (setupterm(NULL, STDOUT_FILENO, &ret) == 0 && ret == 1) {
			ncolors = tigetnum("colors");
			del_curterm(cur_term);
		}
	}
#endif
	ако (1 < ncolors) {
		DBG(CONF, ul_debug("terminal is ready (supports %d colors)", ncolors));
		върни 1;
	}

	DBG(CONF, ul_debug("terminal is NOT ready (no colors)"));
	върни 0;
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
цяло colors_init(цяло mode, const char *name)
{
	цяло ready = -1;
	struct ul_color_ctl *cc = &ul_colors;

	cc->utilname = name;

	termcolors_init_debug();

	ако (mode != UL_COLORMODE_ALWAYS && !isatty(STDOUT_FILENO))
		cc->mode = UL_COLORMODE_NEVER;
	иначе
		cc->mode = mode;

	ако (cc->mode == UL_COLORMODE_UNDEF
	    && getenv("NO_COLOR") == NULL
	    && (ready = colors_terminal_is_ready())) {

		цяло rc = colors_read_configuration(cc);
		ако (rc)
			cc->mode = UL_COLORMODE_DEFAULT;
		иначе {

			/* evaluate scores */
			ако (cc->scores[UL_COLORFILE_DISABLE] >
			    cc->scores[UL_COLORFILE_ENABLE])
				cc->mode = UL_COLORMODE_NEVER;
			иначе
				cc->mode = UL_COLORMODE_DEFAULT;

			atexit(colors_deinit);
		}
	}

	switch (cc->mode) {
	case UL_COLORMODE_AUTO:
		cc->has_colors = ready == -1 ? colors_terminal_is_ready() : ready;
		break;
	case UL_COLORMODE_ALWAYS:
		cc->has_colors = 1;
		break;
	case UL_COLORMODE_NEVER:
	default:
		cc->has_colors = 0;
	}

	ON_DBG(CONF, colors_debug(cc));

	върни cc->has_colors;
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
цяло colors_wanted(void)
{
	върни ul_colors.has_colors;
}

/*
 * Returns mode
 */
цяло colors_mode(void)
{
	върни ul_colors.mode;
}

/*
 * Enable @seq color
 */
void color_fenable(const char *seq, FILE *f)
{
	ако (!ul_colors.disabled && ul_colors.has_colors && seq)
		fputs(seq, f);
}

/*
 * Returns escape sequence by logical @name, if undefined then returns @dflt.
 */
const char *color_scheme_get_sequence(const char *name, const char *dflt)
{
	struct ul_color_scheme *cs;

	ако (ul_colors.disabled || !ul_colors.has_colors)
		върни NULL;

	cs = colors_get_scheme(&ul_colors, name);
	върни cs && cs->seq ? cs->seq : dflt;
}

/*
 * Enable color by logical @name, if undefined enable @dflt.
 */
void color_scheme_fenable(const char *name, const char *dflt, FILE *f)
{
	const char *seq = color_scheme_get_sequence(name, dflt);

	ако (!seq)
		върни;
	color_fenable(seq, f);
}


/*
 * Disable previously enabled color
 */
void color_fdisable(FILE *f)
{
	ако (!ul_colors.disabled && ul_colors.has_colors)
		fputs(UL_COLOR_RESET, f);
}

/*
 * Get reset sequence
 */
const char *color_get_disable_sequence(void)
{
	ако (!ul_colors.disabled && ul_colors.has_colors)
		върни UL_COLOR_RESET;
	иначе
		върни "";
}

/*
 * Parses @str to return UL_COLORMODE_*
 */
цяло colormode_from_string(const char *str)
{
	size_t i;
	static const char *const modes[] = {
		[UL_COLORMODE_AUTO]   = "auto",
		[UL_COLORMODE_NEVER]  = "never",
		[UL_COLORMODE_ALWAYS] = "always",
		[UL_COLORMODE_UNDEF] = ""
	};

	ако (!str || !*str)
		върни -EINVAL;

	assert(ARRAY_SIZE(modes) == __UL_NCOLORMODES);

	за (i = 0; i < ARRAY_SIZE(modes); i++) {
		ако (c_strcasecmp(str, modes[i]) == 0)
			върни i;
	}

	върни -EINVAL;
}

/*
 * Parses @str and exit(EXIT_FAILURE) on error
 */
цяло colormode_or_err(const char *str)
{
	const char *p = str && *str == '=' ? str + 1 : str;
	цяло colormode;

	colormode = colormode_from_string(p);
	ако (colormode < 0)
		errx(EXIT_FAILURE, _("unsupported color mode: %s"), p);

	върни colormode;
}

#ifdef TEST_PROGRAM_COLORS
# include <getopt.h>
цяло main(цяло argc, char *argv[])
{
	static const struct option longopts[] = {
		{ "mode",	required_argument, NULL, 'm' },
		{ "color",	required_argument, NULL, 'c' },
		{ "color-scheme", required_argument, NULL, 'C' },
		{ "name",	required_argument, NULL, 'n' },
		{ NULL, 0, NULL, 0 }
	};
	цяло c, mode = UL_COLORMODE_UNDEF;	/* default */
	const char *color = "red", *name = NULL, *color_scheme = NULL;
	char *seq = NULL;

	докато ((c = getopt_long(argc, argv, "C:c:m:n:", longopts, NULL)) != -1) {
		switch (c) {
		case 'c':
			color = optarg;
			break;
		case 'C':
			color_scheme = optarg;
			break;
		case 'm':
			mode = colormode_or_err(optarg);
			break;
		case 'n':
			name = optarg;
			break;
		default:
			fprintf(stderr, "usage: %s [options]\n"
			" -m, --mode <auto|never|always>  default is undefined\n"
			" -c, --color <red|blue|...>      color for the test message\n"
			" -C, --color-scheme <name>       color for the test message\n"
			" -n, --name <utilname>           util name\n",
			program_invocation_short_name);
			върни EXIT_FAILURE;
		}
	}

	colors_init(mode, name ? name : program_invocation_short_name);

	ако (color_is_sequence(color))
		seq = strdup(color);
	иначе
		seq = color_get_sequence(color);

	ако (color_scheme)
		color_scheme_enable(color_scheme, seq);
	иначе
		color_enable(seq);
	printf("Hello World!");
	color_disable();
	fputc('\n', stdout);

	free(seq);

	върни EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_COLORS */

