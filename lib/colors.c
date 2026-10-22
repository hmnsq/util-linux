/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: 2012 Ondrej Oprala <ooprala@redhat.com>
 *          2012-2025 Karel Zak <kzak@redhat.com>
 */
#ضمن <assert.h>
#ضمن <sys/stat.h>
#ضمن <sys/types.h>
#ضمن <dirent.h>
#ضمن <ctype.h>

#אם defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
# אם defined(HAVE_NCURSESW_NCURSES_H)
#  include <ncursesw/ncurses.h>
# elif defined(HAVE_NCURSES_NCURSES_H)
#  include <ncurses/ncurses.h>
# elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
# endif
# אם defined(HAVE_NCURSESW_TERM_H)
#  include <ncursesw/term.h>
# elif defined(HAVE_NCURSES_TERM_H)
#  include <ncurses/term.h>
# elif defined(HAVE_TERM_H)
#  include <term.h>
# endif
#endif

#ضمن "c.h"
#ضمن "cctype.h"
#ضمن "colors.h"
#ضمن "nls.h"
#ضمن "pathnames.h"
#ضمن "strutils.h"

#ضمن "debug.h"

/*
 * Default behavior, may be overridden by terminal-colors.d/{enable,disable}.
 */
#ifdef USE_COLORS_BY_DEFAULT
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_AUTO	/* check isatty() */
#אחרת
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_NEVER	/* no colors by default */
#endif

/*
 * terminal-colors.d debug stuff
 */
ساكن UL_DEBUG_DEFINE_MASK(termcolors);
UL_DEBUG_DEFINE_MASKNAMES(termcolors) = UL_DEBUG_EMPTY_MASKNAMES;

#define TERMCOLORS_DEBUG_INIT	(1 << 1)
#define TERMCOLORS_DEBUG_CONF	(1 << 2)
#define TERMCOLORS_DEBUG_SCHEME	(1 << 3)

#define DBG(m, x)       __UL_DBG(termcolors, TERMCOLORS_DEBUG_, m, x)
#define ON_DBG(m, x)    __UL_DBG_CALL(termcolors, TERMCOLORS_DEBUG_, m, x)

/*
 * terminal-colors.d file types
 */
تعداد {
	UL_COLORFILE_DISABLE,		/* .disable */
	UL_COLORFILE_ENABLE,		/* .enable */
	UL_COLORFILE_SCHEME,		/* .scheme */

	__UL_COLORFILE_COUNT
};

بنية ul_color_scheme {
	حرف *name;
	حرف *seq;
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
بنية ul_color_ctl {
	ثابت حرف	*utilname;	/* util name */
	ثابت حرف	*termname;	/* terminal name ($TERM) */

	حرف		*sfile;	/* path to scheme */

	بنية ul_color_scheme	*schemes;	/* array with color schemes */
	size_t			nschemes;	/* number of the items */
	size_t			schemes_sz;	/* number of the allocated items */

	صحيح		mode;		/* UL_COLORMODE_* */
	unsigned صحيح	has_colors : 1,	/* based on mode and scores[] */
			disabled   : 1, /* disable colors */
			cs_configured : 1, /* color schemes read */
			configured : 1; /* terminal-colors.d parsed */

	صحيح		scores[__UL_COLORFILE_COUNT];	/* the best match */
};

/*
 * Control struct, globally shared.
 */
ساكن بنية ul_color_ctl ul_colors;

ساكن فراغ colors_free_schemes(بنية ul_color_ctl *cc);
ساكن صحيح colors_read_schemes(بنية ul_color_ctl *cc);

/*
 * qsort/bsearch buddy
 */
ساكن صحيح cmp_scheme_name(ثابت فراغ *a0, ثابت فراغ *b0)
{
	ثابت بنية ul_color_scheme *a = (ثابت بنية ul_color_scheme *) a0,
				     *b = (ثابت بنية ul_color_scheme *) b0;
	החזר strcmp(a->name, b->name);
}

/*
 * Resets control struct (note that we don't allocate the struct)
 */
ساكن فراغ colors_reset(بنية ul_color_ctl *cc)
{
	אם (!cc)
		החזר;

	colors_free_schemes(cc);

	حرر(cc->sfile);

	cc->sfile = عدم;
	cc->utilname = عدم;
	cc->termname = عدم;
	cc->mode = UL_COLORMODE_UNDEF;

	memset(cc->scores, 0, حجم(cc->scores));
}

ساكن فراغ colors_debug(بنية ul_color_ctl *cc)
{
	size_t i;

	אם (!cc)
		החזר;

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

	עבור (i = 0; i < ARRAY_SIZE(cc->scores); i++)
		printf("\tscore %s = %d\n",
				i == UL_COLORFILE_DISABLE ? "disable" :
				i == UL_COLORFILE_ENABLE ? "enable" :
				i == UL_COLORFILE_SCHEME ? "scheme" : "???",
				cc->scores[i]);

	fputc('\n', stdout);

	עבור (i = 0; i < cc->nschemes; i++) {
		printf("\tscheme #%02zu ", i);
		color_scheme_enable(cc->schemes[i].name, عدم);
		fputs(cc->schemes[i].name, stdout);
		color_disable();
		fputc('\n', stdout);
	}
	fputc('\n', stdout);
}

/*
 * Parses [[<utilname>][@<termname>].]<type>
 */
ساكن صحيح filename_to_tokens(ثابت حرف *str,
			      ثابت حرف **name, size_t *namesz,
			      ثابت حرف **term, size_t *termsz,
			      صحيح  *filetype)
{
	ثابت حرف *type_start, *term_start, *p;

	אם (!str || !*str || *str == '.' || strlen(str) > PATH_MAX)
		החזר -EINVAL;

	/* parse .type */
	p = strrchr(str, '.');
	type_start = p ? p + 1 : str;

	אם (strcmp(type_start, "disable") == 0)
		*filetype = UL_COLORFILE_DISABLE;
	אחרת אם (strcmp(type_start, "enable") == 0)
		*filetype = UL_COLORFILE_ENABLE;
	אחרת אם (strcmp(type_start, "scheme") == 0)
		*filetype = UL_COLORFILE_SCHEME;
	אחרת {
		DBG(CONF, ul_debug("unknown type '%s'", type_start));
		החזר 1;				/* unknown type */
	}

	אם (type_start == str)
		החזר 0;				/* "type" only */

	/* parse @termname */
	p = strchr(str, '@');
	term_start = p ? p + 1 : عدم;
	אם (term_start) {
		*term = term_start;
		*termsz = type_start - term_start - 1;
		אם (term_start - 1 == str)
			החזר 0;			/* "@termname.type" */
	}

	/* parse utilname */
	p = term_start ? term_start : type_start;
	*name = str;
	*namesz = p - str - 1;

	החזר 0;
}

/*
 * Scans @dirname and select the best matches for UL_COLORFILE_* types.
 * The result is stored to cc->scores. The path to the best "scheme"
 * file is stored to cc->scheme.
 */
ساكن صحيح colors_readdir(بنية ul_color_ctl *cc, ثابت حرف *dirname)
{
	DIR *dir;
	صحيح rc = -ENOENT;
	بنية dirent *d;
	حرف sfile[PATH_MAX] = { '\0' };
	size_t namesz, termsz;

	אם (!dirname || !cc || !cc->utilname || !*cc->utilname)
		החזר -EINVAL;

	DBG(CONF, ul_debug("reading dir: '%s'", dirname));

	dir = opendir(dirname);
	אם (!dir)
		החזר -errno;

	namesz = strlen(cc->utilname);
	termsz = cc->termname ? strlen(cc->termname) : 0;

	כלעוד ((d = readdir(dir))) {
		صحيح type, score = 1;
		ثابت حرف *tk_name = عدم, *tk_term = عدم;
		size_t tk_namesz = 0, tk_termsz = 0;

		אם (*d->d_name == '.')
			استمر;
#ifdef _DIRENT_HAVE_D_TYPE
		אם (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK &&
		    d->d_type != DT_REG)
			استمر;
#endif
		אם (filename_to_tokens(d->d_name,
				       &tk_name, &tk_namesz,
				       &tk_term, &tk_termsz, &type) != 0)
			استمر;

		/* count theoretical score before we check names to avoid
		 * unnecessary strcmp() */
		אם (tk_name)
			score += 20;
		אם (tk_term)
			score += 10;

		DBG(CONF, ul_debug("item '%s': score=%d "
			"[cur: %d, name(%zu): %s, term(%zu): %s]",
			d->d_name, score, cc->scores[type],
			tk_namesz, tk_name,
			tk_termsz, tk_term));


		אם (score < cc->scores[type])
			استمر;

		/* filter out by names */
		אם (tk_namesz && (tk_namesz != namesz ||
				 strncmp(tk_name, cc->utilname, namesz) != 0))
			استمر;

		אם (tk_termsz && (termsz == 0 || tk_termsz != termsz ||
				  strncmp(tk_term, cc->termname, termsz) != 0))
			استمر;

		DBG(CONF, ul_debug("setting '%s' from %d -to-> %d",
					type == UL_COLORFILE_SCHEME ? "scheme" :
					type == UL_COLORFILE_DISABLE ? "disable" :
					type == UL_COLORFILE_ENABLE ? "enable" : "???",
					cc->scores[type], score));
		cc->scores[type] = score;
		rc = 0;
		אם (type == UL_COLORFILE_SCHEME)
			strncpy(sfile, d->d_name, حجم(sfile));
	}

	אם (*sfile) {
		sfile[حجم(sfile) - 1] = '\0';
		אם (asprintf(&cc->sfile, "%s/%s", dirname, sfile) <= 0)
			rc = -ENOMEM;
	}

	closedir(dir);
	החזר rc;
}

/* atexit() wrapper */
ساكن فراغ colors_deinit(فراغ)
{
	colors_reset(&ul_colors);
}

/*
 * Returns path to $XDG_CONFIG_HOME/terminal-colors.d
 */
ساكن حرف *colors_get_homedir(حرف *buf, size_t bufsz)
{
	حرف *p = getenv("XDG_CONFIG_HOME");

	אם (p) {
		snprintf(buf, bufsz, "%s/" _PATH_TERMCOLORS_DIRNAME, p);
		החזר buf;
	}

	p = getenv("HOME");
	אם (p) {
		snprintf(buf, bufsz, "%s/.config/" _PATH_TERMCOLORS_DIRNAME, p);
		החזר buf;
	}

	החזר عدم;
}

/*
 * Adds one color sequence to array with color scheme.
 */
ساكن صحيح colors_add_scheme(بنية ul_color_ctl *cc,
			     ثابت حرف *name,
			     ثابت حرف *seq0)
{
	بنية ul_color_scheme *cs = عدم;
	حرف *seq = عدم;
	صحيح rc;

	אם (!cc || !name || !*name || !seq0 || !*seq0)
		החזר -EINVAL;

	DBG(SCHEME, ul_debug("add '%s'", name));

	seq = color_get_sequence(seq0);
	אם (!seq)
		החזר -EINVAL;
	rc = -ENOMEM;

	/* enlarge the array */
	אם (cc->nschemes == cc->schemes_sz) {
		فراغ *tmp = reallocarray(cc->schemes, cc->nschemes + 10,
					 حجم(بنية ul_color_scheme));
		אם (!tmp)
			اذهب err;
		cc->schemes = tmp;
		cc->schemes_sz = cc->nschemes + 10;
	}

	/* add a new item */
	cs = &cc->schemes[cc->nschemes];
	cs->seq = seq;
	cs->name = strdup(name);
	אם (!cs->name)
		اذهب err;

	cc->nschemes++;
	החזר 0;
err:
	אם (cs) {
		حرر(cs->seq);
		حرر(cs->name);
		cs->seq = cs->name = عدم;
	} אחרת
		حرر(seq);
	החזר rc;
}

/*
 * Deallocates all regards to color schemes
 */
ساكن فراغ colors_free_schemes(بنية ul_color_ctl *cc)
{
	size_t i;

	DBG(SCHEME, ul_debug("free scheme"));

	עבור (i = 0; i < cc->nschemes; i++) {
		حرر(cc->schemes[i].name);
		حرر(cc->schemes[i].seq);
	}

	حرر(cc->schemes);
	cc->schemes = عدم;
	cc->nschemes = 0;
	cc->schemes_sz = 0;
}

/*
 * The scheme configuration has to be sorted for bsearch
 */
ساكن فراغ colors_sort_schemes(بنية ul_color_ctl *cc)
{
	אם (!cc->nschemes)
		החזר;

	DBG(SCHEME, ul_debug("sort scheme"));

	qsort(cc->schemes, cc->nschemes,
	      حجم(بنية ul_color_scheme), cmp_scheme_name);
}

/*
 * Returns just one color scheme
 */
ساكن بنية ul_color_scheme *colors_get_scheme(بنية ul_color_ctl *cc,
						 ثابت حرف *name)
{
	بنية ul_color_scheme key = { .name = (حرف *) name}, *res;

	אם (!cc || !name || !*name)
		החזר عدم;

	אם (!cc->cs_configured) {
		صحيح rc = colors_read_schemes(cc);
		אם (rc)
			החזר عدم;
	}
	אם (!cc->nschemes)
		החזר عدم;

	DBG(SCHEME, ul_debug("search '%s'", name));

	res = bsearch(&key, cc->schemes, cc->nschemes,
				حجم(بنية ul_color_scheme),
				cmp_scheme_name);

	החזר res && res->seq ? res  : عدم;
}

/*
 * Parses filenames in terminal-colors.d
 */
ساكن صحيح colors_read_configuration(بنية ul_color_ctl *cc)
{
	صحيح rc = -ENOENT;
	حرف *dirname, buf[PATH_MAX];

	cc->termname = getenv("TERM");

	dirname = colors_get_homedir(buf, حجم(buf));
	אם (dirname)
		rc = colors_readdir(cc, dirname);		/* ~/.config */
	אם (rc == -EPERM || rc == -EACCES || rc == -ENOENT)
		rc = colors_readdir(cc, _PATH_TERMCOLORS_DIR);	/* /etc */

	cc->configured = 1;
	החזר rc;
}

/*
 * Reads terminal-colors.d/ scheme file into array schemes
 */
ساكن صحيح colors_read_schemes(بنية ul_color_ctl *cc)
{
	صحيح rc = 0;
	FILE *f = عدم;
	حرف buf[BUFSIZ],
	     cn[129], seq[129];

	אם (!cc->configured)
		rc = colors_read_configuration(cc);

	cc->cs_configured = 1;

	אם (rc || !cc->sfile)
		اذهب done;

	DBG(SCHEME, ul_debug("reading file '%s'", cc->sfile));

	f = fopen(cc->sfile, "r" UL_CLOEXECSTR);
	אם (!f) {
		rc = -errno;
		اذهب done;
	}

	כלעוד (fgets(buf, حجم(buf), f)) {
		حرف *p = strchr(buf, '\n');

		אם (!p) {
			אם (feof(f))
				p = strchr(buf, '\0');
			אחרת {
				rc = -errno;
				اذهب done;
			}
		}
		*p = '\0';
		p = (حرف *) skip_blank(buf);
		אם (*p == '\0' || *p == '#')
			استمر;

		rc = sscanf(p, "%128[^ ] %128[^\n ]", cn, seq);
		אם (rc == 2 && *cn && *seq) {
			rc = colors_add_scheme(cc, cn, seq);	/* set rc=0 on success */
			אם (rc)
				اذهب done;
		}
	}
	rc = 0;

done:
	אם (f)
		fclose(f);
	colors_sort_schemes(cc);

	החזר rc;
}


ساكن فراغ termcolors_init_debug(فراغ)
{
	__UL_INIT_DEBUG_FROM_ENV(termcolors, TERMCOLORS_DEBUG_, 0, TERMINAL_COLORS_DEBUG);
}

ساكن صحيح colors_terminal_is_ready(فراغ)
{
	صحيح ncolors = -1;

#אם defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
	{
		صحيح ret;

		/* setupterm() allocates memory, del_curterm() deallocates it */
		אם (setupterm(عدم, STDOUT_FILENO, &ret) == 0 && ret == 1) {
			ncolors = tigetnum("colors");
			del_curterm(cur_term);
		}
	}
#endif
	אם (1 < ncolors) {
		DBG(CONF, ul_debug("terminal is ready (supports %d colors)", ncolors));
		החזר 1;
	}

	DBG(CONF, ul_debug("terminal is NOT ready (no colors)"));
	החזר 0;
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
صحيح colors_init(صحيح mode, ثابت حرف *name)
{
	صحيح ready = -1;
	بنية ul_color_ctl *cc = &ul_colors;

	cc->utilname = name;

	termcolors_init_debug();

	אם (mode != UL_COLORMODE_ALWAYS && !isatty(STDOUT_FILENO))
		cc->mode = UL_COLORMODE_NEVER;
	אחרת
		cc->mode = mode;

	אם (cc->mode == UL_COLORMODE_UNDEF
	    && getenv("NO_COLOR") == عدم
	    && (ready = colors_terminal_is_ready())) {

		صحيح rc = colors_read_configuration(cc);
		אם (rc)
			cc->mode = UL_COLORMODE_DEFAULT;
		אחרת {

			/* evaluate scores */
			אם (cc->scores[UL_COLORFILE_DISABLE] >
			    cc->scores[UL_COLORFILE_ENABLE])
				cc->mode = UL_COLORMODE_NEVER;
			אחרת
				cc->mode = UL_COLORMODE_DEFAULT;

			atexit(colors_deinit);
		}
	}

	بدل (cc->mode) {
	حالة UL_COLORMODE_AUTO:
		cc->has_colors = ready == -1 ? colors_terminal_is_ready() : ready;
		اكسر;
	حالة UL_COLORMODE_ALWAYS:
		cc->has_colors = 1;
		اكسر;
	حالة UL_COLORMODE_NEVER:
	افتراضي:
		cc->has_colors = 0;
	}

	ON_DBG(CONF, colors_debug(cc));

	החזר cc->has_colors;
}

/*
 * Temporary disable colors (this setting is independent on terminal-colors.d/)
 */
فراغ colors_off(فراغ)
{
	ul_colors.disabled = 1;
}

/*
 * Enable colors
 */
فراغ colors_on(فراغ)
{
	ul_colors.disabled = 0;
}

/*
 * Is terminal-colors.d/ configured to use colors?
 */
صحيح colors_wanted(فراغ)
{
	החזר ul_colors.has_colors;
}

/*
 * Returns mode
 */
صحيح colors_mode(فراغ)
{
	החזר ul_colors.mode;
}

/*
 * Enable @seq color
 */
فراغ color_fenable(ثابت حرف *seq, FILE *f)
{
	אם (!ul_colors.disabled && ul_colors.has_colors && seq)
		fputs(seq, f);
}

/*
 * Returns escape sequence by logical @name, if undefined then returns @dflt.
 */
ثابت حرف *color_scheme_get_sequence(ثابت حرف *name, ثابت حرف *dflt)
{
	بنية ul_color_scheme *cs;

	אם (ul_colors.disabled || !ul_colors.has_colors)
		החזר عدم;

	cs = colors_get_scheme(&ul_colors, name);
	החזר cs && cs->seq ? cs->seq : dflt;
}

/*
 * Enable color by logical @name, if undefined enable @dflt.
 */
فراغ color_scheme_fenable(ثابت حرف *name, ثابت حرف *dflt, FILE *f)
{
	ثابت حرف *seq = color_scheme_get_sequence(name, dflt);

	אם (!seq)
		החזר;
	color_fenable(seq, f);
}


/*
 * Disable previously enabled color
 */
فراغ color_fdisable(FILE *f)
{
	אם (!ul_colors.disabled && ul_colors.has_colors)
		fputs(UL_COLOR_RESET, f);
}

/*
 * Get reset sequence
 */
ثابت حرف *color_get_disable_sequence(فراغ)
{
	אם (!ul_colors.disabled && ul_colors.has_colors)
		החזר UL_COLOR_RESET;
	אחרת
		החזר "";
}

/*
 * Parses @str to return UL_COLORMODE_*
 */
صحيح colormode_from_string(ثابت حرف *str)
{
	size_t i;
	ساكن ثابت حرف *ثابت modes[] = {
		[UL_COLORMODE_AUTO]   = "auto",
		[UL_COLORMODE_NEVER]  = "never",
		[UL_COLORMODE_ALWAYS] = "always",
		[UL_COLORMODE_UNDEF] = ""
	};

	אם (!str || !*str)
		החזר -EINVAL;

	أكد(ARRAY_SIZE(modes) == __UL_NCOLORMODES);

	עבור (i = 0; i < ARRAY_SIZE(modes); i++) {
		אם (c_strcasecmp(str, modes[i]) == 0)
			החזר i;
	}

	החזר -EINVAL;
}

/*
 * Parses @str and exit(EXIT_FAILURE) on error
 */
صحيح colormode_or_err(ثابت حرف *str)
{
	ثابت حرف *p = str && *str == '=' ? str + 1 : str;
	صحيح colormode;

	colormode = colormode_from_string(p);
	אם (colormode < 0)
		errx(EXIT_FAILURE, _("unsupported color mode: %s"), p);

	החזר colormode;
}

#ifdef TEST_PROGRAM_COLORS
# include <getopt.h>
صحيح main(صحيح argc, حرف *argv[])
{
	ساكن ثابت بنية option longopts[] = {
		{ "mode",	required_argument, عدم, 'm' },
		{ "color",	required_argument, عدم, 'c' },
		{ "color-scheme", required_argument, عدم, 'C' },
		{ "name",	required_argument, عدم, 'n' },
		{ عدم, 0, عدم, 0 }
	};
	صحيح c, mode = UL_COLORMODE_UNDEF;	/* default */
	ثابت حرف *color = "red", *name = عدم, *color_scheme = عدم;
	حرف *seq = عدم;

	כלעוד ((c = getopt_long(argc, argv, "C:c:m:n:", longopts, عدم)) != -1) {
		بدل (c) {
		حالة 'c':
			color = optarg;
			اكسر;
		حالة 'C':
			color_scheme = optarg;
			اكسر;
		حالة 'm':
			mode = colormode_or_err(optarg);
			اكسر;
		حالة 'n':
			name = optarg;
			اكسر;
		افتراضي:
			fprintf(stderr, "usage: %s [options]\n"
			" -m, --mode <auto|never|always>  default is undefined\n"
			" -c, --color <red|blue|...>      color for the test message\n"
			" -C, --color-scheme <name>       color for the test message\n"
			" -n, --name <utilname>           util name\n",
			program_invocation_short_name);
			החזר EXIT_FAILURE;
		}
	}

	colors_init(mode, name ? name : program_invocation_short_name);

	אם (color_is_sequence(color))
		seq = strdup(color);
	אחרת
		seq = color_get_sequence(color);

	אם (color_scheme)
		color_scheme_enable(color_scheme, seq);
	אחרת
		color_enable(seq);
	printf("Hello World!");
	color_disable();
	fputc('\n', stdout);

	حرر(seq);

	החזר EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_COLORS */

