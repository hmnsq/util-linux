/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: 2012 Ondrej Oprala <ooprala@redhat.com>
 *          2012-2025 Karel Zak <kzak@redhat.com>
 */
#شامل <assert.h>
#شامل <sys/stat.h>
#شامل <sys/types.h>
#شامل <dirent.h>
#شامل <ctype.h>

#اگر defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
# اگر defined(HAVE_NCURSESW_NCURSES_H)
#  include <ncursesw/ncurses.h>
# elif defined(HAVE_NCURSES_NCURSES_H)
#  include <ncurses/ncurses.h>
# elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
# endif
# اگر defined(HAVE_NCURSESW_TERM_H)
#  include <ncursesw/term.h>
# elif defined(HAVE_NCURSES_TERM_H)
#  include <ncurses/term.h>
# elif defined(HAVE_TERM_H)
#  include <term.h>
# endif
#endif

#شامل "c.h"
#شامل "cctype.h"
#شامل "colors.h"
#شامل "nls.h"
#شامل "pathnames.h"
#شامل "strutils.h"

#شامل "debug.h"

/*
 * Default behavior, may be overridden by terminal-colors.d/{enable,disable}.
 */
#ifdef USE_COLORS_BY_DEFAULT
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_AUTO	/* check isatty() */
#وگرنه
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_NEVER	/* no colors by default */
#endif

/*
 * terminal-colors.d debug stuff
 */
ایستا UL_DEBUG_DEFINE_MASK(termcolors);
UL_DEBUG_DEFINE_MASKNAMES(termcolors) = UL_DEBUG_EMPTY_MASKNAMES;

#define TERMCOLORS_DEBUG_INIT	(1 << 1)
#define TERMCOLORS_DEBUG_CONF	(1 << 2)
#define TERMCOLORS_DEBUG_SCHEME	(1 << 3)

#define DBG(m, x)       __UL_DBG(termcolors, TERMCOLORS_DEBUG_, m, x)
#define ON_DBG(m, x)    __UL_DBG_CALL(termcolors, TERMCOLORS_DEBUG_, m, x)

/*
 * terminal-colors.d file types
 */
شمارش {
	UL_COLORFILE_DISABLE,		/* .disable */
	UL_COLORFILE_ENABLE,		/* .enable */
	UL_COLORFILE_SCHEME,		/* .scheme */

	__UL_COLORFILE_COUNT
};

ساختار ul_color_scheme {
	نویسه *name;
	نویسه *seq;
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
ساختار ul_color_ctl {
	ثابت نویسه	*utilname;	/* util name */
	ثابت نویسه	*termname;	/* terminal name ($TERM) */

	نویسه		*sfile;	/* path to scheme */

	ساختار ul_color_scheme	*schemes;	/* array with color schemes */
	size_t			nschemes;	/* number of the items */
	size_t			schemes_sz;	/* number of the allocated items */

	صحیح		mode;		/* UL_COLORMODE_* */
	unsigned صحیح	has_colors : 1,	/* based on mode and scores[] */
			disabled   : 1, /* disable colors */
			cs_configured : 1, /* color schemes read */
			configured : 1; /* terminal-colors.d parsed */

	صحیح		scores[__UL_COLORFILE_COUNT];	/* the best match */
};

/*
 * Control struct, globally shared.
 */
ایستا ساختار ul_color_ctl ul_colors;

ایستا تهی colors_free_schemes(ساختار ul_color_ctl *cc);
ایستا صحیح colors_read_schemes(ساختار ul_color_ctl *cc);

/*
 * qsort/bsearch buddy
 */
ایستا صحیح cmp_scheme_name(ثابت تهی *a0, ثابت تهی *b0)
{
	ثابت ساختار ul_color_scheme *a = (ثابت ساختار ul_color_scheme *) a0,
				     *b = (ثابت ساختار ul_color_scheme *) b0;
	بازگشت strcmp(a->name, b->name);
}

/*
 * Resets control struct (note that we don't allocate the struct)
 */
ایستا تهی colors_reset(ساختار ul_color_ctl *cc)
{
	اگر (!cc)
		بازگشت;

	colors_free_schemes(cc);

	آزاد(cc->sfile);

	cc->sfile = پوچ;
	cc->utilname = پوچ;
	cc->termname = پوچ;
	cc->mode = UL_COLORMODE_UNDEF;

	memset(cc->scores, 0, اندازه(cc->scores));
}

ایستا تهی colors_debug(ساختار ul_color_ctl *cc)
{
	size_t i;

	اگر (!cc)
		بازگشت;

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

	برای (i = 0; i < ARRAY_SIZE(cc->scores); i++)
		printf("\tscore %s = %d\n",
				i == UL_COLORFILE_DISABLE ? "disable" :
				i == UL_COLORFILE_ENABLE ? "enable" :
				i == UL_COLORFILE_SCHEME ? "scheme" : "???",
				cc->scores[i]);

	fputc('\n', stdout);

	برای (i = 0; i < cc->nschemes; i++) {
		printf("\tscheme #%02zu ", i);
		color_scheme_enable(cc->schemes[i].name, پوچ);
		fputs(cc->schemes[i].name, stdout);
		color_disable();
		fputc('\n', stdout);
	}
	fputc('\n', stdout);
}

/*
 * Parses [[<utilname>][@<termname>].]<type>
 */
ایستا صحیح filename_to_tokens(ثابت نویسه *str,
			      ثابت نویسه **name, size_t *namesz,
			      ثابت نویسه **term, size_t *termsz,
			      صحیح  *filetype)
{
	ثابت نویسه *type_start, *term_start, *p;

	اگر (!str || !*str || *str == '.' || strlen(str) > PATH_MAX)
		بازگشت -EINVAL;

	/* parse .type */
	p = strrchr(str, '.');
	type_start = p ? p + 1 : str;

	اگر (strcmp(type_start, "disable") == 0)
		*filetype = UL_COLORFILE_DISABLE;
	وگرنه اگر (strcmp(type_start, "enable") == 0)
		*filetype = UL_COLORFILE_ENABLE;
	وگرنه اگر (strcmp(type_start, "scheme") == 0)
		*filetype = UL_COLORFILE_SCHEME;
	وگرنه {
		DBG(CONF, ul_debug("unknown type '%s'", type_start));
		بازگشت 1;				/* unknown type */
	}

	اگر (type_start == str)
		بازگشت 0;				/* "type" only */

	/* parse @termname */
	p = strchr(str, '@');
	term_start = p ? p + 1 : پوچ;
	اگر (term_start) {
		*term = term_start;
		*termsz = type_start - term_start - 1;
		اگر (term_start - 1 == str)
			بازگشت 0;			/* "@termname.type" */
	}

	/* parse utilname */
	p = term_start ? term_start : type_start;
	*name = str;
	*namesz = p - str - 1;

	بازگشت 0;
}

/*
 * Scans @dirname and select the best matches for UL_COLORFILE_* types.
 * The result is stored to cc->scores. The path to the best "scheme"
 * file is stored to cc->scheme.
 */
ایستا صحیح colors_readdir(ساختار ul_color_ctl *cc, ثابت نویسه *dirname)
{
	DIR *dir;
	صحیح rc = -ENOENT;
	ساختار dirent *d;
	نویسه sfile[PATH_MAX] = { '\0' };
	size_t namesz, termsz;

	اگر (!dirname || !cc || !cc->utilname || !*cc->utilname)
		بازگشت -EINVAL;

	DBG(CONF, ul_debug("reading dir: '%s'", dirname));

	dir = opendir(dirname);
	اگر (!dir)
		بازگشت -errno;

	namesz = strlen(cc->utilname);
	termsz = cc->termname ? strlen(cc->termname) : 0;

	وقتی ((d = readdir(dir))) {
		صحیح type, score = 1;
		ثابت نویسه *tk_name = پوچ, *tk_term = پوچ;
		size_t tk_namesz = 0, tk_termsz = 0;

		اگر (*d->d_name == '.')
			ادامه;
#ifdef _DIRENT_HAVE_D_TYPE
		اگر (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK &&
		    d->d_type != DT_REG)
			ادامه;
#endif
		اگر (filename_to_tokens(d->d_name,
				       &tk_name, &tk_namesz,
				       &tk_term, &tk_termsz, &type) != 0)
			ادامه;

		/* count theoretical score before we check names to avoid
		 * unnecessary strcmp() */
		اگر (tk_name)
			score += 20;
		اگر (tk_term)
			score += 10;

		DBG(CONF, ul_debug("item '%s': score=%d "
			"[cur: %d, name(%zu): %s, term(%zu): %s]",
			d->d_name, score, cc->scores[type],
			tk_namesz, tk_name,
			tk_termsz, tk_term));


		اگر (score < cc->scores[type])
			ادامه;

		/* filter out by names */
		اگر (tk_namesz && (tk_namesz != namesz ||
				 strncmp(tk_name, cc->utilname, namesz) != 0))
			ادامه;

		اگر (tk_termsz && (termsz == 0 || tk_termsz != termsz ||
				  strncmp(tk_term, cc->termname, termsz) != 0))
			ادامه;

		DBG(CONF, ul_debug("setting '%s' from %d -to-> %d",
					type == UL_COLORFILE_SCHEME ? "scheme" :
					type == UL_COLORFILE_DISABLE ? "disable" :
					type == UL_COLORFILE_ENABLE ? "enable" : "???",
					cc->scores[type], score));
		cc->scores[type] = score;
		rc = 0;
		اگر (type == UL_COLORFILE_SCHEME)
			strncpy(sfile, d->d_name, اندازه(sfile));
	}

	اگر (*sfile) {
		sfile[اندازه(sfile) - 1] = '\0';
		اگر (asprintf(&cc->sfile, "%s/%s", dirname, sfile) <= 0)
			rc = -ENOMEM;
	}

	closedir(dir);
	بازگشت rc;
}

/* atexit() wrapper */
ایستا تهی colors_deinit(تهی)
{
	colors_reset(&ul_colors);
}

/*
 * Returns path to $XDG_CONFIG_HOME/terminal-colors.d
 */
ایستا نویسه *colors_get_homedir(نویسه *buf, size_t bufsz)
{
	نویسه *p = getenv("XDG_CONFIG_HOME");

	اگر (p) {
		snprintf(buf, bufsz, "%s/" _PATH_TERMCOLORS_DIRNAME, p);
		بازگشت buf;
	}

	p = getenv("HOME");
	اگر (p) {
		snprintf(buf, bufsz, "%s/.config/" _PATH_TERMCOLORS_DIRNAME, p);
		بازگشت buf;
	}

	بازگشت پوچ;
}

/*
 * Adds one color sequence to array with color scheme.
 */
ایستا صحیح colors_add_scheme(ساختار ul_color_ctl *cc,
			     ثابت نویسه *name,
			     ثابت نویسه *seq0)
{
	ساختار ul_color_scheme *cs = پوچ;
	نویسه *seq = پوچ;
	صحیح rc;

	اگر (!cc || !name || !*name || !seq0 || !*seq0)
		بازگشت -EINVAL;

	DBG(SCHEME, ul_debug("add '%s'", name));

	seq = color_get_sequence(seq0);
	اگر (!seq)
		بازگشت -EINVAL;
	rc = -ENOMEM;

	/* enlarge the array */
	اگر (cc->nschemes == cc->schemes_sz) {
		تهی *tmp = reallocarray(cc->schemes, cc->nschemes + 10,
					 اندازه(ساختار ul_color_scheme));
		اگر (!tmp)
			برو err;
		cc->schemes = tmp;
		cc->schemes_sz = cc->nschemes + 10;
	}

	/* add a new item */
	cs = &cc->schemes[cc->nschemes];
	cs->seq = seq;
	cs->name = strdup(name);
	اگر (!cs->name)
		برو err;

	cc->nschemes++;
	بازگشت 0;
err:
	اگر (cs) {
		آزاد(cs->seq);
		آزاد(cs->name);
		cs->seq = cs->name = پوچ;
	} وگرنه
		آزاد(seq);
	بازگشت rc;
}

/*
 * Deallocates all regards to color schemes
 */
ایستا تهی colors_free_schemes(ساختار ul_color_ctl *cc)
{
	size_t i;

	DBG(SCHEME, ul_debug("free scheme"));

	برای (i = 0; i < cc->nschemes; i++) {
		آزاد(cc->schemes[i].name);
		آزاد(cc->schemes[i].seq);
	}

	آزاد(cc->schemes);
	cc->schemes = پوچ;
	cc->nschemes = 0;
	cc->schemes_sz = 0;
}

/*
 * The scheme configuration has to be sorted for bsearch
 */
ایستا تهی colors_sort_schemes(ساختار ul_color_ctl *cc)
{
	اگر (!cc->nschemes)
		بازگشت;

	DBG(SCHEME, ul_debug("sort scheme"));

	qsort(cc->schemes, cc->nschemes,
	      اندازه(ساختار ul_color_scheme), cmp_scheme_name);
}

/*
 * Returns just one color scheme
 */
ایستا ساختار ul_color_scheme *colors_get_scheme(ساختار ul_color_ctl *cc,
						 ثابت نویسه *name)
{
	ساختار ul_color_scheme کلید = { .name = (نویسه *) name}, *res;

	اگر (!cc || !name || !*name)
		بازگشت پوچ;

	اگر (!cc->cs_configured) {
		صحیح rc = colors_read_schemes(cc);
		اگر (rc)
			بازگشت پوچ;
	}
	اگر (!cc->nschemes)
		بازگشت پوچ;

	DBG(SCHEME, ul_debug("search '%s'", name));

	res = bsearch(&کلید, cc->schemes, cc->nschemes,
				اندازه(ساختار ul_color_scheme),
				cmp_scheme_name);

	بازگشت res && res->seq ? res  : پوچ;
}

/*
 * Parses filenames in terminal-colors.d
 */
ایستا صحیح colors_read_configuration(ساختار ul_color_ctl *cc)
{
	صحیح rc = -ENOENT;
	نویسه *dirname, buf[PATH_MAX];

	cc->termname = getenv("TERM");

	dirname = colors_get_homedir(buf, اندازه(buf));
	اگر (dirname)
		rc = colors_readdir(cc, dirname);		/* ~/.config */
	اگر (rc == -EPERM || rc == -EACCES || rc == -ENOENT)
		rc = colors_readdir(cc, _PATH_TERMCOLORS_DIR);	/* /etc */

	cc->configured = 1;
	بازگشت rc;
}

/*
 * Reads terminal-colors.d/ scheme file into array schemes
 */
ایستا صحیح colors_read_schemes(ساختار ul_color_ctl *cc)
{
	صحیح rc = 0;
	FILE *f = پوچ;
	نویسه buf[BUFSIZ],
	     cn[129], seq[129];

	اگر (!cc->configured)
		rc = colors_read_configuration(cc);

	cc->cs_configured = 1;

	اگر (rc || !cc->sfile)
		برو done;

	DBG(SCHEME, ul_debug("reading file '%s'", cc->sfile));

	f = fopen(cc->sfile, "r" UL_CLOEXECSTR);
	اگر (!f) {
		rc = -errno;
		برو done;
	}

	وقتی (fgets(buf, اندازه(buf), f)) {
		نویسه *p = strchr(buf, '\n');

		اگر (!p) {
			اگر (feof(f))
				p = strchr(buf, '\0');
			وگرنه {
				rc = -errno;
				برو done;
			}
		}
		*p = '\0';
		p = (نویسه *) skip_blank(buf);
		اگر (*p == '\0' || *p == '#')
			ادامه;

		rc = sscanf(p, "%128[^ ] %128[^\n ]", cn, seq);
		اگر (rc == 2 && *cn && *seq) {
			rc = colors_add_scheme(cc, cn, seq);	/* set rc=0 on success */
			اگر (rc)
				برو done;
		}
	}
	rc = 0;

done:
	اگر (f)
		fclose(f);
	colors_sort_schemes(cc);

	بازگشت rc;
}


ایستا تهی termcolors_init_debug(تهی)
{
	__UL_INIT_DEBUG_FROM_ENV(termcolors, TERMCOLORS_DEBUG_, 0, TERMINAL_COLORS_DEBUG);
}

ایستا صحیح colors_terminal_is_ready(تهی)
{
	صحیح ncolors = -1;

#اگر defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
	{
		صحیح ret;

		/* setupterm() allocates memory, del_curterm() deallocates it */
		اگر (setupterm(پوچ, STDOUT_FILENO, &ret) == 0 && ret == 1) {
			ncolors = tigetnum("colors");
			del_curterm(cur_term);
		}
	}
#endif
	اگر (1 < ncolors) {
		DBG(CONF, ul_debug("terminal is ready (supports %d colors)", ncolors));
		بازگشت 1;
	}

	DBG(CONF, ul_debug("terminal is NOT ready (no colors)"));
	بازگشت 0;
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
صحیح colors_init(صحیح mode, ثابت نویسه *name)
{
	صحیح ready = -1;
	ساختار ul_color_ctl *cc = &ul_colors;

	cc->utilname = name;

	termcolors_init_debug();

	اگر (mode != UL_COLORMODE_ALWAYS && !isatty(STDOUT_FILENO))
		cc->mode = UL_COLORMODE_NEVER;
	وگرنه
		cc->mode = mode;

	اگر (cc->mode == UL_COLORMODE_UNDEF
	    && getenv("NO_COLOR") == پوچ
	    && (ready = colors_terminal_is_ready())) {

		صحیح rc = colors_read_configuration(cc);
		اگر (rc)
			cc->mode = UL_COLORMODE_DEFAULT;
		وگرنه {

			/* evaluate scores */
			اگر (cc->scores[UL_COLORFILE_DISABLE] >
			    cc->scores[UL_COLORFILE_ENABLE])
				cc->mode = UL_COLORMODE_NEVER;
			وگرنه
				cc->mode = UL_COLORMODE_DEFAULT;

			atexit(colors_deinit);
		}
	}

	انتخاب (cc->mode) {
	حالت UL_COLORMODE_AUTO:
		cc->has_colors = ready == -1 ? colors_terminal_is_ready() : ready;
		بشکن;
	حالت UL_COLORMODE_ALWAYS:
		cc->has_colors = 1;
		بشکن;
	حالت UL_COLORMODE_NEVER:
	پیشفرض:
		cc->has_colors = 0;
	}

	ON_DBG(CONF, colors_debug(cc));

	بازگشت cc->has_colors;
}

/*
 * Temporary disable colors (this setting is independent on terminal-colors.d/)
 */
تهی colors_off(تهی)
{
	ul_colors.disabled = 1;
}

/*
 * Enable colors
 */
تهی colors_on(تهی)
{
	ul_colors.disabled = 0;
}

/*
 * Is terminal-colors.d/ configured to use colors?
 */
صحیح colors_wanted(تهی)
{
	بازگشت ul_colors.has_colors;
}

/*
 * Returns mode
 */
صحیح colors_mode(تهی)
{
	بازگشت ul_colors.mode;
}

/*
 * Enable @seq color
 */
تهی color_fenable(ثابت نویسه *seq, FILE *f)
{
	اگر (!ul_colors.disabled && ul_colors.has_colors && seq)
		fputs(seq, f);
}

/*
 * Returns escape sequence by logical @name, if undefined then returns @dflt.
 */
ثابت نویسه *color_scheme_get_sequence(ثابت نویسه *name, ثابت نویسه *dflt)
{
	ساختار ul_color_scheme *cs;

	اگر (ul_colors.disabled || !ul_colors.has_colors)
		بازگشت پوچ;

	cs = colors_get_scheme(&ul_colors, name);
	بازگشت cs && cs->seq ? cs->seq : dflt;
}

/*
 * Enable color by logical @name, if undefined enable @dflt.
 */
تهی color_scheme_fenable(ثابت نویسه *name, ثابت نویسه *dflt, FILE *f)
{
	ثابت نویسه *seq = color_scheme_get_sequence(name, dflt);

	اگر (!seq)
		بازگشت;
	color_fenable(seq, f);
}


/*
 * Disable previously enabled color
 */
تهی color_fdisable(FILE *f)
{
	اگر (!ul_colors.disabled && ul_colors.has_colors)
		fputs(UL_COLOR_RESET, f);
}

/*
 * Get reset sequence
 */
ثابت نویسه *color_get_disable_sequence(تهی)
{
	اگر (!ul_colors.disabled && ul_colors.has_colors)
		بازگشت UL_COLOR_RESET;
	وگرنه
		بازگشت "";
}

/*
 * Parses @str to return UL_COLORMODE_*
 */
صحیح colormode_from_string(ثابت نویسه *str)
{
	size_t i;
	ایستا ثابت نویسه *ثابت modes[] = {
		[UL_COLORMODE_AUTO]   = "auto",
		[UL_COLORMODE_NEVER]  = "never",
		[UL_COLORMODE_ALWAYS] = "always",
		[UL_COLORMODE_UNDEF] = ""
	};

	اگر (!str || !*str)
		بازگشت -EINVAL;

	أكد(ARRAY_SIZE(modes) == __UL_NCOLORMODES);

	برای (i = 0; i < ARRAY_SIZE(modes); i++) {
		اگر (c_strcasecmp(str, modes[i]) == 0)
			بازگشت i;
	}

	بازگشت -EINVAL;
}

/*
 * Parses @str and exit(EXIT_FAILURE) on error
 */
صحیح colormode_or_err(ثابت نویسه *str)
{
	ثابت نویسه *p = str && *str == '=' ? str + 1 : str;
	صحیح colormode;

	colormode = colormode_from_string(p);
	اگر (colormode < 0)
		errx(EXIT_FAILURE, _("unsupported color mode: %s"), p);

	بازگشت colormode;
}

#ifdef TEST_PROGRAM_COLORS
# include <getopt.h>
صحیح اصلی(صحیح argc, نویسه *argv[])
{
	ایستا ثابت ساختار option longopts[] = {
		{ "mode",	required_argument, پوچ, 'm' },
		{ "color",	required_argument, پوچ, 'c' },
		{ "color-scheme", required_argument, پوچ, 'C' },
		{ "name",	required_argument, پوچ, 'n' },
		{ پوچ, 0, پوچ, 0 }
	};
	صحیح c, mode = UL_COLORMODE_UNDEF;	/* default */
	ثابت نویسه *color = "red", *name = پوچ, *color_scheme = پوچ;
	نویسه *seq = پوچ;

	وقتی ((c = getopt_long(argc, argv, "C:c:m:n:", longopts, پوچ)) != -1) {
		انتخاب (c) {
		حالت 'c':
			color = optarg;
			بشکن;
		حالت 'C':
			color_scheme = optarg;
			بشکن;
		حالت 'm':
			mode = colormode_or_err(optarg);
			بشکن;
		حالت 'n':
			name = optarg;
			بشکن;
		پیشفرض:
			fprintf(stderr, "usage: %s [options]\n"
			" -m, --mode <auto|never|always>  default is undefined\n"
			" -c, --color <red|blue|...>      color for the test message\n"
			" -C, --color-scheme <name>       color for the test message\n"
			" -n, --name <utilname>           util name\n",
			program_invocation_short_name);
			بازگشت EXIT_FAILURE;
		}
	}

	colors_init(mode, name ? name : program_invocation_short_name);

	اگر (color_is_sequence(color))
		seq = strdup(color);
	وگرنه
		seq = color_get_sequence(color);

	اگر (color_scheme)
		color_scheme_enable(color_scheme, seq);
	وگرنه
		color_enable(seq);
	printf("Hello World!");
	color_disable();
	fputc('\n', stdout);

	آزاد(seq);

	بازگشت EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_COLORS */

