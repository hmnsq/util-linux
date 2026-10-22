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

#nếu defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
# nếu defined(HAVE_NCURSESW_NCURSES_H)
#  include <ncursesw/ncurses.h>
# elif defined(HAVE_NCURSES_NCURSES_H)
#  include <ncurses/ncurses.h>
# elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
# endif
# nếu defined(HAVE_NCURSESW_TERM_H)
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
#không_thì
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

	số_nguyên		mode;		/* UL_COLORMODE_* */
	unsigned số_nguyên	has_colors : 1,	/* based on mode and scores[] */
			disabled   : 1, /* disable colors */
			cs_configured : 1, /* color schemes read */
			configured : 1; /* terminal-colors.d parsed */

	số_nguyên		scores[__UL_COLORFILE_COUNT];	/* the best match */
};

/*
 * Control struct, globally shared.
 */
static struct ul_color_ctl ul_colors;

static void colors_free_schemes(struct ul_color_ctl *cc);
static số_nguyên colors_read_schemes(struct ul_color_ctl *cc);

/*
 * qsort/bsearch buddy
 */
static số_nguyên cmp_scheme_name(const void *a0, const void *b0)
{
	const struct ul_color_scheme *a = (const struct ul_color_scheme *) a0,
				     *b = (const struct ul_color_scheme *) b0;
	trả_về strcmp(a->name, b->name);
}

/*
 * Resets control struct (note that we don't allocate the struct)
 */
static void colors_reset(struct ul_color_ctl *cc)
{
	nếu (!cc)
		trả_về;

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

	nếu (!cc)
		trả_về;

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

	cho (i = 0; i < ARRAY_SIZE(cc->scores); i++)
		printf("\tscore %s = %d\n",
				i == UL_COLORFILE_DISABLE ? "disable" :
				i == UL_COLORFILE_ENABLE ? "enable" :
				i == UL_COLORFILE_SCHEME ? "scheme" : "???",
				cc->scores[i]);

	fputc('\n', stdout);

	cho (i = 0; i < cc->nschemes; i++) {
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
static số_nguyên filename_to_tokens(const char *str,
			      const char **name, size_t *namesz,
			      const char **term, size_t *termsz,
			      số_nguyên  *filetype)
{
	const char *type_start, *term_start, *p;

	nếu (!str || !*str || *str == '.' || strlen(str) > PATH_MAX)
		trả_về -EINVAL;

	/* parse .type */
	p = strrchr(str, '.');
	type_start = p ? p + 1 : str;

	nếu (strcmp(type_start, "disable") == 0)
		*filetype = UL_COLORFILE_DISABLE;
	không_thì nếu (strcmp(type_start, "enable") == 0)
		*filetype = UL_COLORFILE_ENABLE;
	không_thì nếu (strcmp(type_start, "scheme") == 0)
		*filetype = UL_COLORFILE_SCHEME;
	không_thì {
		DBG(CONF, ul_debug("unknown type '%s'", type_start));
		trả_về 1;				/* unknown type */
	}

	nếu (type_start == str)
		trả_về 0;				/* "type" only */

	/* parse @termname */
	p = strchr(str, '@');
	term_start = p ? p + 1 : NULL;
	nếu (term_start) {
		*term = term_start;
		*termsz = type_start - term_start - 1;
		nếu (term_start - 1 == str)
			trả_về 0;			/* "@termname.type" */
	}

	/* parse utilname */
	p = term_start ? term_start : type_start;
	*name = str;
	*namesz = p - str - 1;

	trả_về 0;
}

/*
 * Scans @dirname and select the best matches for UL_COLORFILE_* types.
 * The result is stored to cc->scores. The path to the best "scheme"
 * file is stored to cc->scheme.
 */
static số_nguyên colors_readdir(struct ul_color_ctl *cc, const char *dirname)
{
	DIR *dir;
	số_nguyên rc = -ENOENT;
	struct dirent *d;
	char sfile[PATH_MAX] = { '\0' };
	size_t namesz, termsz;

	nếu (!dirname || !cc || !cc->utilname || !*cc->utilname)
		trả_về -EINVAL;

	DBG(CONF, ul_debug("reading dir: '%s'", dirname));

	dir = opendir(dirname);
	nếu (!dir)
		trả_về -errno;

	namesz = strlen(cc->utilname);
	termsz = cc->termname ? strlen(cc->termname) : 0;

	trong_khi ((d = readdir(dir))) {
		số_nguyên type, score = 1;
		const char *tk_name = NULL, *tk_term = NULL;
		size_t tk_namesz = 0, tk_termsz = 0;

		nếu (*d->d_name == '.')
			continue;
#ifdef _DIRENT_HAVE_D_TYPE
		nếu (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK &&
		    d->d_type != DT_REG)
			continue;
#endif
		nếu (filename_to_tokens(d->d_name,
				       &tk_name, &tk_namesz,
				       &tk_term, &tk_termsz, &type) != 0)
			continue;

		/* count theoretical score before we check names to avoid
		 * unnecessary strcmp() */
		nếu (tk_name)
			score += 20;
		nếu (tk_term)
			score += 10;

		DBG(CONF, ul_debug("item '%s': score=%d "
			"[cur: %d, name(%zu): %s, term(%zu): %s]",
			d->d_name, score, cc->scores[type],
			tk_namesz, tk_name,
			tk_termsz, tk_term));


		nếu (score < cc->scores[type])
			continue;

		/* filter out by names */
		nếu (tk_namesz && (tk_namesz != namesz ||
				 strncmp(tk_name, cc->utilname, namesz) != 0))
			continue;

		nếu (tk_termsz && (termsz == 0 || tk_termsz != termsz ||
				  strncmp(tk_term, cc->termname, termsz) != 0))
			continue;

		DBG(CONF, ul_debug("setting '%s' from %d -to-> %d",
					type == UL_COLORFILE_SCHEME ? "scheme" :
					type == UL_COLORFILE_DISABLE ? "disable" :
					type == UL_COLORFILE_ENABLE ? "enable" : "???",
					cc->scores[type], score));
		cc->scores[type] = score;
		rc = 0;
		nếu (type == UL_COLORFILE_SCHEME)
			strncpy(sfile, d->d_name, sizeof(sfile));
	}

	nếu (*sfile) {
		sfile[sizeof(sfile) - 1] = '\0';
		nếu (asprintf(&cc->sfile, "%s/%s", dirname, sfile) <= 0)
			rc = -ENOMEM;
	}

	closedir(dir);
	trả_về rc;
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

	nếu (p) {
		snprintf(buf, bufsz, "%s/" _PATH_TERMCOLORS_DIRNAME, p);
		trả_về buf;
	}

	p = getenv("HOME");
	nếu (p) {
		snprintf(buf, bufsz, "%s/.config/" _PATH_TERMCOLORS_DIRNAME, p);
		trả_về buf;
	}

	trả_về NULL;
}

/*
 * Adds one color sequence to array with color scheme.
 */
static số_nguyên colors_add_scheme(struct ul_color_ctl *cc,
			     const char *name,
			     const char *seq0)
{
	struct ul_color_scheme *cs = NULL;
	char *seq = NULL;
	số_nguyên rc;

	nếu (!cc || !name || !*name || !seq0 || !*seq0)
		trả_về -EINVAL;

	DBG(SCHEME, ul_debug("add '%s'", name));

	seq = color_get_sequence(seq0);
	nếu (!seq)
		trả_về -EINVAL;
	rc = -ENOMEM;

	/* enlarge the array */
	nếu (cc->nschemes == cc->schemes_sz) {
		void *tmp = reallocarray(cc->schemes, cc->nschemes + 10,
					 sizeof(struct ul_color_scheme));
		nếu (!tmp)
			goto err;
		cc->schemes = tmp;
		cc->schemes_sz = cc->nschemes + 10;
	}

	/* add a new item */
	cs = &cc->schemes[cc->nschemes];
	cs->seq = seq;
	cs->name = strdup(name);
	nếu (!cs->name)
		goto err;

	cc->nschemes++;
	trả_về 0;
err:
	nếu (cs) {
		free(cs->seq);
		free(cs->name);
		cs->seq = cs->name = NULL;
	} không_thì
		free(seq);
	trả_về rc;
}

/*
 * Deallocates all regards to color schemes
 */
static void colors_free_schemes(struct ul_color_ctl *cc)
{
	size_t i;

	DBG(SCHEME, ul_debug("free scheme"));

	cho (i = 0; i < cc->nschemes; i++) {
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
	nếu (!cc->nschemes)
		trả_về;

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

	nếu (!cc || !name || !*name)
		trả_về NULL;

	nếu (!cc->cs_configured) {
		số_nguyên rc = colors_read_schemes(cc);
		nếu (rc)
			trả_về NULL;
	}
	nếu (!cc->nschemes)
		trả_về NULL;

	DBG(SCHEME, ul_debug("search '%s'", name));

	res = bsearch(&key, cc->schemes, cc->nschemes,
				sizeof(struct ul_color_scheme),
				cmp_scheme_name);

	trả_về res && res->seq ? res  : NULL;
}

/*
 * Parses filenames in terminal-colors.d
 */
static số_nguyên colors_read_configuration(struct ul_color_ctl *cc)
{
	số_nguyên rc = -ENOENT;
	char *dirname, buf[PATH_MAX];

	cc->termname = getenv("TERM");

	dirname = colors_get_homedir(buf, sizeof(buf));
	nếu (dirname)
		rc = colors_readdir(cc, dirname);		/* ~/.config */
	nếu (rc == -EPERM || rc == -EACCES || rc == -ENOENT)
		rc = colors_readdir(cc, _PATH_TERMCOLORS_DIR);	/* /etc */

	cc->configured = 1;
	trả_về rc;
}

/*
 * Reads terminal-colors.d/ scheme file into array schemes
 */
static số_nguyên colors_read_schemes(struct ul_color_ctl *cc)
{
	số_nguyên rc = 0;
	FILE *f = NULL;
	char buf[BUFSIZ],
	     cn[129], seq[129];

	nếu (!cc->configured)
		rc = colors_read_configuration(cc);

	cc->cs_configured = 1;

	nếu (rc || !cc->sfile)
		goto done;

	DBG(SCHEME, ul_debug("reading file '%s'", cc->sfile));

	f = fopen(cc->sfile, "r" UL_CLOEXECSTR);
	nếu (!f) {
		rc = -errno;
		goto done;
	}

	trong_khi (fgets(buf, sizeof(buf), f)) {
		char *p = strchr(buf, '\n');

		nếu (!p) {
			nếu (feof(f))
				p = strchr(buf, '\0');
			không_thì {
				rc = -errno;
				goto done;
			}
		}
		*p = '\0';
		p = (char *) skip_blank(buf);
		nếu (*p == '\0' || *p == '#')
			continue;

		rc = sscanf(p, "%128[^ ] %128[^\n ]", cn, seq);
		nếu (rc == 2 && *cn && *seq) {
			rc = colors_add_scheme(cc, cn, seq);	/* set rc=0 on success */
			nếu (rc)
				goto done;
		}
	}
	rc = 0;

done:
	nếu (f)
		fclose(f);
	colors_sort_schemes(cc);

	trả_về rc;
}


static void termcolors_init_debug(void)
{
	__UL_INIT_DEBUG_FROM_ENV(termcolors, TERMCOLORS_DEBUG_, 0, TERMINAL_COLORS_DEBUG);
}

static số_nguyên colors_terminal_is_ready(void)
{
	số_nguyên ncolors = -1;

#nếu defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
	{
		số_nguyên ret;

		/* setupterm() allocates memory, del_curterm() deallocates it */
		nếu (setupterm(NULL, STDOUT_FILENO, &ret) == 0 && ret == 1) {
			ncolors = tigetnum("colors");
			del_curterm(cur_term);
		}
	}
#endif
	nếu (1 < ncolors) {
		DBG(CONF, ul_debug("terminal is ready (supports %d colors)", ncolors));
		trả_về 1;
	}

	DBG(CONF, ul_debug("terminal is NOT ready (no colors)"));
	trả_về 0;
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
số_nguyên colors_init(số_nguyên mode, const char *name)
{
	số_nguyên ready = -1;
	struct ul_color_ctl *cc = &ul_colors;

	cc->utilname = name;

	termcolors_init_debug();

	nếu (mode != UL_COLORMODE_ALWAYS && !isatty(STDOUT_FILENO))
		cc->mode = UL_COLORMODE_NEVER;
	không_thì
		cc->mode = mode;

	nếu (cc->mode == UL_COLORMODE_UNDEF
	    && getenv("NO_COLOR") == NULL
	    && (ready = colors_terminal_is_ready())) {

		số_nguyên rc = colors_read_configuration(cc);
		nếu (rc)
			cc->mode = UL_COLORMODE_DEFAULT;
		không_thì {

			/* evaluate scores */
			nếu (cc->scores[UL_COLORFILE_DISABLE] >
			    cc->scores[UL_COLORFILE_ENABLE])
				cc->mode = UL_COLORMODE_NEVER;
			không_thì
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

	trả_về cc->has_colors;
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
số_nguyên colors_wanted(void)
{
	trả_về ul_colors.has_colors;
}

/*
 * Returns mode
 */
số_nguyên colors_mode(void)
{
	trả_về ul_colors.mode;
}

/*
 * Enable @seq color
 */
void color_fenable(const char *seq, FILE *f)
{
	nếu (!ul_colors.disabled && ul_colors.has_colors && seq)
		fputs(seq, f);
}

/*
 * Returns escape sequence by logical @name, if undefined then returns @dflt.
 */
const char *color_scheme_get_sequence(const char *name, const char *dflt)
{
	struct ul_color_scheme *cs;

	nếu (ul_colors.disabled || !ul_colors.has_colors)
		trả_về NULL;

	cs = colors_get_scheme(&ul_colors, name);
	trả_về cs && cs->seq ? cs->seq : dflt;
}

/*
 * Enable color by logical @name, if undefined enable @dflt.
 */
void color_scheme_fenable(const char *name, const char *dflt, FILE *f)
{
	const char *seq = color_scheme_get_sequence(name, dflt);

	nếu (!seq)
		trả_về;
	color_fenable(seq, f);
}


/*
 * Disable previously enabled color
 */
void color_fdisable(FILE *f)
{
	nếu (!ul_colors.disabled && ul_colors.has_colors)
		fputs(UL_COLOR_RESET, f);
}

/*
 * Get reset sequence
 */
const char *color_get_disable_sequence(void)
{
	nếu (!ul_colors.disabled && ul_colors.has_colors)
		trả_về UL_COLOR_RESET;
	không_thì
		trả_về "";
}

/*
 * Parses @str to return UL_COLORMODE_*
 */
số_nguyên colormode_from_string(const char *str)
{
	size_t i;
	static const char *const modes[] = {
		[UL_COLORMODE_AUTO]   = "auto",
		[UL_COLORMODE_NEVER]  = "never",
		[UL_COLORMODE_ALWAYS] = "always",
		[UL_COLORMODE_UNDEF] = ""
	};

	nếu (!str || !*str)
		trả_về -EINVAL;

	assert(ARRAY_SIZE(modes) == __UL_NCOLORMODES);

	cho (i = 0; i < ARRAY_SIZE(modes); i++) {
		nếu (c_strcasecmp(str, modes[i]) == 0)
			trả_về i;
	}

	trả_về -EINVAL;
}

/*
 * Parses @str and exit(EXIT_FAILURE) on error
 */
số_nguyên colormode_or_err(const char *str)
{
	const char *p = str && *str == '=' ? str + 1 : str;
	số_nguyên colormode;

	colormode = colormode_from_string(p);
	nếu (colormode < 0)
		errx(EXIT_FAILURE, _("unsupported color mode: %s"), p);

	trả_về colormode;
}

#ifdef TEST_PROGRAM_COLORS
# include <getopt.h>
số_nguyên main(số_nguyên argc, char *argv[])
{
	static const struct option longopts[] = {
		{ "mode",	required_argument, NULL, 'm' },
		{ "color",	required_argument, NULL, 'c' },
		{ "color-scheme", required_argument, NULL, 'C' },
		{ "name",	required_argument, NULL, 'n' },
		{ NULL, 0, NULL, 0 }
	};
	số_nguyên c, mode = UL_COLORMODE_UNDEF;	/* default */
	const char *color = "red", *name = NULL, *color_scheme = NULL;
	char *seq = NULL;

	trong_khi ((c = getopt_long(argc, argv, "C:c:m:n:", longopts, NULL)) != -1) {
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
			trả_về EXIT_FAILURE;
		}
	}

	colors_init(mode, name ? name : program_invocation_short_name);

	nếu (color_is_sequence(color))
		seq = strdup(color);
	không_thì
		seq = color_get_sequence(color);

	nếu (color_scheme)
		color_scheme_enable(color_scheme, seq);
	không_thì
		color_enable(seq);
	printf("Hello World!");
	color_disable();
	fputc('\n', stdout);

	free(seq);

	trả_về EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_COLORS */

