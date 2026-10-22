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

#만약 defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
# 만약 defined(HAVE_NCURSESW_NCURSES_H)
#  include <ncursesw/ncurses.h>
# elif defined(HAVE_NCURSES_NCURSES_H)
#  include <ncurses/ncurses.h>
# elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
# endif
# 만약 defined(HAVE_NCURSESW_TERM_H)
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
#아니면
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

	정수		mode;		/* UL_COLORMODE_* */
	unsigned 정수	has_colors : 1,	/* based on mode and scores[] */
			disabled   : 1, /* disable colors */
			cs_configured : 1, /* color schemes read */
			configured : 1; /* terminal-colors.d parsed */

	정수		scores[__UL_COLORFILE_COUNT];	/* the best match */
};

/*
 * Control struct, globally shared.
 */
static struct ul_color_ctl ul_colors;

static void colors_free_schemes(struct ul_color_ctl *cc);
static 정수 colors_read_schemes(struct ul_color_ctl *cc);

/*
 * qsort/bsearch buddy
 */
static 정수 cmp_scheme_name(const void *a0, const void *b0)
{
	const struct ul_color_scheme *a = (const struct ul_color_scheme *) a0,
				     *b = (const struct ul_color_scheme *) b0;
	반환 strcmp(a->name, b->name);
}

/*
 * Resets control struct (note that we don't allocate the struct)
 */
static void colors_reset(struct ul_color_ctl *cc)
{
	만약 (!cc)
		반환;

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

	만약 (!cc)
		반환;

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

	위해 (i = 0; i < ARRAY_SIZE(cc->scores); i++)
		printf("\tscore %s = %d\n",
				i == UL_COLORFILE_DISABLE ? "disable" :
				i == UL_COLORFILE_ENABLE ? "enable" :
				i == UL_COLORFILE_SCHEME ? "scheme" : "???",
				cc->scores[i]);

	fputc('\n', stdout);

	위해 (i = 0; i < cc->nschemes; i++) {
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
static 정수 filename_to_tokens(const char *str,
			      const char **name, size_t *namesz,
			      const char **term, size_t *termsz,
			      정수  *filetype)
{
	const char *type_start, *term_start, *p;

	만약 (!str || !*str || *str == '.' || strlen(str) > PATH_MAX)
		반환 -EINVAL;

	/* parse .type */
	p = strrchr(str, '.');
	type_start = p ? p + 1 : str;

	만약 (strcmp(type_start, "disable") == 0)
		*filetype = UL_COLORFILE_DISABLE;
	아니면 만약 (strcmp(type_start, "enable") == 0)
		*filetype = UL_COLORFILE_ENABLE;
	아니면 만약 (strcmp(type_start, "scheme") == 0)
		*filetype = UL_COLORFILE_SCHEME;
	아니면 {
		DBG(CONF, ul_debug("unknown type '%s'", type_start));
		반환 1;				/* unknown type */
	}

	만약 (type_start == str)
		반환 0;				/* "type" only */

	/* parse @termname */
	p = strchr(str, '@');
	term_start = p ? p + 1 : NULL;
	만약 (term_start) {
		*term = term_start;
		*termsz = type_start - term_start - 1;
		만약 (term_start - 1 == str)
			반환 0;			/* "@termname.type" */
	}

	/* parse utilname */
	p = term_start ? term_start : type_start;
	*name = str;
	*namesz = p - str - 1;

	반환 0;
}

/*
 * Scans @dirname and select the best matches for UL_COLORFILE_* types.
 * The result is stored to cc->scores. The path to the best "scheme"
 * file is stored to cc->scheme.
 */
static 정수 colors_readdir(struct ul_color_ctl *cc, const char *dirname)
{
	DIR *dir;
	정수 rc = -ENOENT;
	struct dirent *d;
	char sfile[PATH_MAX] = { '\0' };
	size_t namesz, termsz;

	만약 (!dirname || !cc || !cc->utilname || !*cc->utilname)
		반환 -EINVAL;

	DBG(CONF, ul_debug("reading dir: '%s'", dirname));

	dir = opendir(dirname);
	만약 (!dir)
		반환 -errno;

	namesz = strlen(cc->utilname);
	termsz = cc->termname ? strlen(cc->termname) : 0;

	동안 ((d = readdir(dir))) {
		정수 type, score = 1;
		const char *tk_name = NULL, *tk_term = NULL;
		size_t tk_namesz = 0, tk_termsz = 0;

		만약 (*d->d_name == '.')
			continue;
#ifdef _DIRENT_HAVE_D_TYPE
		만약 (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK &&
		    d->d_type != DT_REG)
			continue;
#endif
		만약 (filename_to_tokens(d->d_name,
				       &tk_name, &tk_namesz,
				       &tk_term, &tk_termsz, &type) != 0)
			continue;

		/* count theoretical score before we check names to avoid
		 * unnecessary strcmp() */
		만약 (tk_name)
			score += 20;
		만약 (tk_term)
			score += 10;

		DBG(CONF, ul_debug("item '%s': score=%d "
			"[cur: %d, name(%zu): %s, term(%zu): %s]",
			d->d_name, score, cc->scores[type],
			tk_namesz, tk_name,
			tk_termsz, tk_term));


		만약 (score < cc->scores[type])
			continue;

		/* filter out by names */
		만약 (tk_namesz && (tk_namesz != namesz ||
				 strncmp(tk_name, cc->utilname, namesz) != 0))
			continue;

		만약 (tk_termsz && (termsz == 0 || tk_termsz != termsz ||
				  strncmp(tk_term, cc->termname, termsz) != 0))
			continue;

		DBG(CONF, ul_debug("setting '%s' from %d -to-> %d",
					type == UL_COLORFILE_SCHEME ? "scheme" :
					type == UL_COLORFILE_DISABLE ? "disable" :
					type == UL_COLORFILE_ENABLE ? "enable" : "???",
					cc->scores[type], score));
		cc->scores[type] = score;
		rc = 0;
		만약 (type == UL_COLORFILE_SCHEME)
			strncpy(sfile, d->d_name, sizeof(sfile));
	}

	만약 (*sfile) {
		sfile[sizeof(sfile) - 1] = '\0';
		만약 (asprintf(&cc->sfile, "%s/%s", dirname, sfile) <= 0)
			rc = -ENOMEM;
	}

	closedir(dir);
	반환 rc;
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

	만약 (p) {
		snprintf(buf, bufsz, "%s/" _PATH_TERMCOLORS_DIRNAME, p);
		반환 buf;
	}

	p = getenv("HOME");
	만약 (p) {
		snprintf(buf, bufsz, "%s/.config/" _PATH_TERMCOLORS_DIRNAME, p);
		반환 buf;
	}

	반환 NULL;
}

/*
 * Adds one color sequence to array with color scheme.
 */
static 정수 colors_add_scheme(struct ul_color_ctl *cc,
			     const char *name,
			     const char *seq0)
{
	struct ul_color_scheme *cs = NULL;
	char *seq = NULL;
	정수 rc;

	만약 (!cc || !name || !*name || !seq0 || !*seq0)
		반환 -EINVAL;

	DBG(SCHEME, ul_debug("add '%s'", name));

	seq = color_get_sequence(seq0);
	만약 (!seq)
		반환 -EINVAL;
	rc = -ENOMEM;

	/* enlarge the array */
	만약 (cc->nschemes == cc->schemes_sz) {
		void *tmp = reallocarray(cc->schemes, cc->nschemes + 10,
					 sizeof(struct ul_color_scheme));
		만약 (!tmp)
			goto err;
		cc->schemes = tmp;
		cc->schemes_sz = cc->nschemes + 10;
	}

	/* add a new item */
	cs = &cc->schemes[cc->nschemes];
	cs->seq = seq;
	cs->name = strdup(name);
	만약 (!cs->name)
		goto err;

	cc->nschemes++;
	반환 0;
err:
	만약 (cs) {
		free(cs->seq);
		free(cs->name);
		cs->seq = cs->name = NULL;
	} 아니면
		free(seq);
	반환 rc;
}

/*
 * Deallocates all regards to color schemes
 */
static void colors_free_schemes(struct ul_color_ctl *cc)
{
	size_t i;

	DBG(SCHEME, ul_debug("free scheme"));

	위해 (i = 0; i < cc->nschemes; i++) {
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
	만약 (!cc->nschemes)
		반환;

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

	만약 (!cc || !name || !*name)
		반환 NULL;

	만약 (!cc->cs_configured) {
		정수 rc = colors_read_schemes(cc);
		만약 (rc)
			반환 NULL;
	}
	만약 (!cc->nschemes)
		반환 NULL;

	DBG(SCHEME, ul_debug("search '%s'", name));

	res = bsearch(&key, cc->schemes, cc->nschemes,
				sizeof(struct ul_color_scheme),
				cmp_scheme_name);

	반환 res && res->seq ? res  : NULL;
}

/*
 * Parses filenames in terminal-colors.d
 */
static 정수 colors_read_configuration(struct ul_color_ctl *cc)
{
	정수 rc = -ENOENT;
	char *dirname, buf[PATH_MAX];

	cc->termname = getenv("TERM");

	dirname = colors_get_homedir(buf, sizeof(buf));
	만약 (dirname)
		rc = colors_readdir(cc, dirname);		/* ~/.config */
	만약 (rc == -EPERM || rc == -EACCES || rc == -ENOENT)
		rc = colors_readdir(cc, _PATH_TERMCOLORS_DIR);	/* /etc */

	cc->configured = 1;
	반환 rc;
}

/*
 * Reads terminal-colors.d/ scheme file into array schemes
 */
static 정수 colors_read_schemes(struct ul_color_ctl *cc)
{
	정수 rc = 0;
	FILE *f = NULL;
	char buf[BUFSIZ],
	     cn[129], seq[129];

	만약 (!cc->configured)
		rc = colors_read_configuration(cc);

	cc->cs_configured = 1;

	만약 (rc || !cc->sfile)
		goto done;

	DBG(SCHEME, ul_debug("reading file '%s'", cc->sfile));

	f = fopen(cc->sfile, "r" UL_CLOEXECSTR);
	만약 (!f) {
		rc = -errno;
		goto done;
	}

	동안 (fgets(buf, sizeof(buf), f)) {
		char *p = strchr(buf, '\n');

		만약 (!p) {
			만약 (feof(f))
				p = strchr(buf, '\0');
			아니면 {
				rc = -errno;
				goto done;
			}
		}
		*p = '\0';
		p = (char *) skip_blank(buf);
		만약 (*p == '\0' || *p == '#')
			continue;

		rc = sscanf(p, "%128[^ ] %128[^\n ]", cn, seq);
		만약 (rc == 2 && *cn && *seq) {
			rc = colors_add_scheme(cc, cn, seq);	/* set rc=0 on success */
			만약 (rc)
				goto done;
		}
	}
	rc = 0;

done:
	만약 (f)
		fclose(f);
	colors_sort_schemes(cc);

	반환 rc;
}


static void termcolors_init_debug(void)
{
	__UL_INIT_DEBUG_FROM_ENV(termcolors, TERMCOLORS_DEBUG_, 0, TERMINAL_COLORS_DEBUG);
}

static 정수 colors_terminal_is_ready(void)
{
	정수 ncolors = -1;

#만약 defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
	{
		정수 ret;

		/* setupterm() allocates memory, del_curterm() deallocates it */
		만약 (setupterm(NULL, STDOUT_FILENO, &ret) == 0 && ret == 1) {
			ncolors = tigetnum("colors");
			del_curterm(cur_term);
		}
	}
#endif
	만약 (1 < ncolors) {
		DBG(CONF, ul_debug("terminal is ready (supports %d colors)", ncolors));
		반환 1;
	}

	DBG(CONF, ul_debug("terminal is NOT ready (no colors)"));
	반환 0;
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
정수 colors_init(정수 mode, const char *name)
{
	정수 ready = -1;
	struct ul_color_ctl *cc = &ul_colors;

	cc->utilname = name;

	termcolors_init_debug();

	만약 (mode != UL_COLORMODE_ALWAYS && !isatty(STDOUT_FILENO))
		cc->mode = UL_COLORMODE_NEVER;
	아니면
		cc->mode = mode;

	만약 (cc->mode == UL_COLORMODE_UNDEF
	    && getenv("NO_COLOR") == NULL
	    && (ready = colors_terminal_is_ready())) {

		정수 rc = colors_read_configuration(cc);
		만약 (rc)
			cc->mode = UL_COLORMODE_DEFAULT;
		아니면 {

			/* evaluate scores */
			만약 (cc->scores[UL_COLORFILE_DISABLE] >
			    cc->scores[UL_COLORFILE_ENABLE])
				cc->mode = UL_COLORMODE_NEVER;
			아니면
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

	반환 cc->has_colors;
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
정수 colors_wanted(void)
{
	반환 ul_colors.has_colors;
}

/*
 * Returns mode
 */
정수 colors_mode(void)
{
	반환 ul_colors.mode;
}

/*
 * Enable @seq color
 */
void color_fenable(const char *seq, FILE *f)
{
	만약 (!ul_colors.disabled && ul_colors.has_colors && seq)
		fputs(seq, f);
}

/*
 * Returns escape sequence by logical @name, if undefined then returns @dflt.
 */
const char *color_scheme_get_sequence(const char *name, const char *dflt)
{
	struct ul_color_scheme *cs;

	만약 (ul_colors.disabled || !ul_colors.has_colors)
		반환 NULL;

	cs = colors_get_scheme(&ul_colors, name);
	반환 cs && cs->seq ? cs->seq : dflt;
}

/*
 * Enable color by logical @name, if undefined enable @dflt.
 */
void color_scheme_fenable(const char *name, const char *dflt, FILE *f)
{
	const char *seq = color_scheme_get_sequence(name, dflt);

	만약 (!seq)
		반환;
	color_fenable(seq, f);
}


/*
 * Disable previously enabled color
 */
void color_fdisable(FILE *f)
{
	만약 (!ul_colors.disabled && ul_colors.has_colors)
		fputs(UL_COLOR_RESET, f);
}

/*
 * Get reset sequence
 */
const char *color_get_disable_sequence(void)
{
	만약 (!ul_colors.disabled && ul_colors.has_colors)
		반환 UL_COLOR_RESET;
	아니면
		반환 "";
}

/*
 * Parses @str to return UL_COLORMODE_*
 */
정수 colormode_from_string(const char *str)
{
	size_t i;
	static const char *const modes[] = {
		[UL_COLORMODE_AUTO]   = "auto",
		[UL_COLORMODE_NEVER]  = "never",
		[UL_COLORMODE_ALWAYS] = "always",
		[UL_COLORMODE_UNDEF] = ""
	};

	만약 (!str || !*str)
		반환 -EINVAL;

	assert(ARRAY_SIZE(modes) == __UL_NCOLORMODES);

	위해 (i = 0; i < ARRAY_SIZE(modes); i++) {
		만약 (c_strcasecmp(str, modes[i]) == 0)
			반환 i;
	}

	반환 -EINVAL;
}

/*
 * Parses @str and exit(EXIT_FAILURE) on error
 */
정수 colormode_or_err(const char *str)
{
	const char *p = str && *str == '=' ? str + 1 : str;
	정수 colormode;

	colormode = colormode_from_string(p);
	만약 (colormode < 0)
		errx(EXIT_FAILURE, _("unsupported color mode: %s"), p);

	반환 colormode;
}

#ifdef TEST_PROGRAM_COLORS
# include <getopt.h>
정수 main(정수 argc, char *argv[])
{
	static const struct option longopts[] = {
		{ "mode",	required_argument, NULL, 'm' },
		{ "color",	required_argument, NULL, 'c' },
		{ "color-scheme", required_argument, NULL, 'C' },
		{ "name",	required_argument, NULL, 'n' },
		{ NULL, 0, NULL, 0 }
	};
	정수 c, mode = UL_COLORMODE_UNDEF;	/* default */
	const char *color = "red", *name = NULL, *color_scheme = NULL;
	char *seq = NULL;

	동안 ((c = getopt_long(argc, argv, "C:c:m:n:", longopts, NULL)) != -1) {
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
			반환 EXIT_FAILURE;
		}
	}

	colors_init(mode, name ? name : program_invocation_short_name);

	만약 (color_is_sequence(color))
		seq = strdup(color);
	아니면
		seq = color_get_sequence(color);

	만약 (color_scheme)
		color_scheme_enable(color_scheme, seq);
	아니면
		color_enable(seq);
	printf("Hello World!");
	color_disable();
	fputc('\n', stdout);

	free(seq);

	반환 EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_COLORS */

