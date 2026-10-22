/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: 2012 Ondrej Oprala <ooprala@redhat.com>
 *          2012-2025 Karel Zak <kzak@redhat.com>
 */
#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <dirent.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>

#ଯଦି defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
# ଯଦି defined(HAVE_NCURSESW_NCURSES_H)
#  include <ncursesw/ncurses.h>
# elif defined(HAVE_NCURSES_NCURSES_H)
#  include <ncurses/ncurses.h>
# elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
# endif
# ଯଦି defined(HAVE_NCURSESW_TERM_H)
#  include <ncursesw/term.h>
# elif defined(HAVE_NCURSES_TERM_H)
#  include <ncurses/term.h>
# elif defined(HAVE_TERM_H)
#  include <term.h>
# endif
#endif

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "cctype.h"
#ଅନ୍ତର୍ଭାଵ "colors.h"
#ଅନ୍ତର୍ଭାଵ "nls.h"
#ଅନ୍ତର୍ଭାଵ "pathnames.h"
#ଅନ୍ତର୍ଭାଵ "strutils.h"

#ଅନ୍ତର୍ଭାଵ "debug.h"

/*
 * Default behavior, may be overridden by terminal-colors.d/{enable,disable}.
 */
#ifdef USE_COLORS_BY_DEFAULT
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_AUTO	/* check isatty() */
#ଅନ୍ୟଥା
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_NEVER	/* no colors by default */
#endif

/*
 * terminal-colors.d debug stuff
 */
ସ୍ଥିର UL_DEBUG_DEFINE_MASK(termcolors);
UL_DEBUG_DEFINE_MASKNAMES(termcolors) = UL_DEBUG_EMPTY_MASKNAMES;

#define TERMCOLORS_DEBUG_INIT	(1 << 1)
#define TERMCOLORS_DEBUG_CONF	(1 << 2)
#define TERMCOLORS_DEBUG_SCHEME	(1 << 3)

#define DBG(m, x)       __UL_DBG(termcolors, TERMCOLORS_DEBUG_, m, x)
#define ON_DBG(m, x)    __UL_DBG_CALL(termcolors, TERMCOLORS_DEBUG_, m, x)

/*
 * terminal-colors.d file types
 */
ଗଣନା {
	UL_COLORFILE_DISABLE,		/* .disable */
	UL_COLORFILE_ENABLE,		/* .enable */
	UL_COLORFILE_SCHEME,		/* .scheme */

	__UL_COLORFILE_COUNT
};

ସଂରଚନା ul_color_scheme {
	ଅକ୍ଷର *name;
	ଅକ୍ଷର *seq;
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
ସଂରଚନା ul_color_ctl {
	ଅଚଳ ଅକ୍ଷର	*utilname;	/* util name */
	ଅଚଳ ଅକ୍ଷର	*termname;	/* terminal name ($TERM) */

	ଅକ୍ଷର		*sfile;	/* path to scheme */

	ସଂରଚନା ul_color_scheme	*schemes;	/* array with color schemes */
	size_t			nschemes;	/* number of the items */
	size_t			schemes_sz;	/* number of the allocated items */

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା		mode;		/* UL_COLORMODE_* */
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା	has_colors : 1,	/* based on mode and scores[] */
			disabled   : 1, /* disable colors */
			cs_configured : 1, /* color schemes read */
			configured : 1; /* terminal-colors.d parsed */

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା		scores[__UL_COLORFILE_COUNT];	/* the best match */
};

/*
 * Control struct, globally shared.
 */
ସ୍ଥିର ସଂରଚନା ul_color_ctl ul_colors;

ସ୍ଥିର ଶୂନ୍ୟ colors_free_schemes(ସଂରଚନା ul_color_ctl *cc);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା colors_read_schemes(ସଂରଚନା ul_color_ctl *cc);

/*
 * qsort/bsearch buddy
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmp_scheme_name(ଅଚଳ ଶୂନ୍ୟ *a0, ଅଚଳ ଶୂନ୍ୟ *b0)
{
	ଅଚଳ ସଂରଚନା ul_color_scheme *a = (ଅଚଳ ସଂରଚନା ul_color_scheme *) a0,
				     *b = (ଅଚଳ ସଂରଚନା ul_color_scheme *) b0;
	ଫେରନ୍ତୁ strcmp(a->name, b->name);
}

/*
 * Resets control struct (note that we don't allocate the struct)
 */
ସ୍ଥିର ଶୂନ୍ୟ colors_reset(ସଂରଚନା ul_color_ctl *cc)
{
	ଯଦି (!cc)
		ଫେରନ୍ତୁ;

	colors_free_schemes(cc);

	ମୁକ୍ତ(cc->sfile);

	cc->sfile = ଖାଲି;
	cc->utilname = ଖାଲି;
	cc->termname = ଖାଲି;
	cc->mode = UL_COLORMODE_UNDEF;

	memset(cc->scores, 0, ଆକାର(cc->scores));
}

ସ୍ଥିର ଶୂନ୍ୟ colors_debug(ସଂରଚନା ul_color_ctl *cc)
{
	size_t i;

	ଯଦି (!cc)
		ଫେରନ୍ତୁ;

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

	ପାଇଁ (i = 0; i < ARRAY_SIZE(cc->scores); i++)
		printf("\tscore %s = %d\n",
				i == UL_COLORFILE_DISABLE ? "disable" :
				i == UL_COLORFILE_ENABLE ? "enable" :
				i == UL_COLORFILE_SCHEME ? "scheme" : "???",
				cc->scores[i]);

	fputc('\n', stdout);

	ପାଇଁ (i = 0; i < cc->nschemes; i++) {
		printf("\tscheme #%02zu ", i);
		color_scheme_enable(cc->schemes[i].name, ଖାଲି);
		fputs(cc->schemes[i].name, stdout);
		color_disable();
		fputc('\n', stdout);
	}
	fputc('\n', stdout);
}

/*
 * Parses [[<utilname>][@<termname>].]<type>
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା filename_to_tokens(ଅଚଳ ଅକ୍ଷର *str,
			      ଅଚଳ ଅକ୍ଷର **name, size_t *namesz,
			      ଅଚଳ ଅକ୍ଷର **term, size_t *termsz,
			      ପୂର୍ଣ୍ଣସଂଖ୍ୟା  *filetype)
{
	ଅଚଳ ଅକ୍ଷର *type_start, *term_start, *p;

	ଯଦି (!str || !*str || *str == '.' || strlen(str) > PATH_MAX)
		ଫେରନ୍ତୁ -EINVAL;

	/* parse .type */
	p = strrchr(str, '.');
	type_start = p ? p + 1 : str;

	ଯଦି (strcmp(type_start, "disable") == 0)
		*filetype = UL_COLORFILE_DISABLE;
	ଅନ୍ୟଥା ଯଦି (strcmp(type_start, "enable") == 0)
		*filetype = UL_COLORFILE_ENABLE;
	ଅନ୍ୟଥା ଯଦି (strcmp(type_start, "scheme") == 0)
		*filetype = UL_COLORFILE_SCHEME;
	ଅନ୍ୟଥା {
		DBG(CONF, ul_debug("unknown type '%s'", type_start));
		ଫେରନ୍ତୁ 1;				/* unknown type */
	}

	ଯଦି (type_start == str)
		ଫେରନ୍ତୁ 0;				/* "type" only */

	/* parse @termname */
	p = strchr(str, '@');
	term_start = p ? p + 1 : ଖାଲି;
	ଯଦି (term_start) {
		*term = term_start;
		*termsz = type_start - term_start - 1;
		ଯଦି (term_start - 1 == str)
			ଫେରନ୍ତୁ 0;			/* "@termname.type" */
	}

	/* parse utilname */
	p = term_start ? term_start : type_start;
	*name = str;
	*namesz = p - str - 1;

	ଫେରନ୍ତୁ 0;
}

/*
 * Scans @dirname and select the best matches for UL_COLORFILE_* types.
 * The result is stored to cc->scores. The path to the best "scheme"
 * file is stored to cc->scheme.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା colors_readdir(ସଂରଚନା ul_color_ctl *cc, ଅଚଳ ଅକ୍ଷର *dirname)
{
	DIR *dir;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = -ENOENT;
	ସଂରଚନା dirent *d;
	ଅକ୍ଷର sfile[PATH_MAX] = { '\0' };
	size_t namesz, termsz;

	ଯଦି (!dirname || !cc || !cc->utilname || !*cc->utilname)
		ଫେରନ୍ତୁ -EINVAL;

	DBG(CONF, ul_debug("reading dir: '%s'", dirname));

	dir = opendir(dirname);
	ଯଦି (!dir)
		ଫେରନ୍ତୁ -errno;

	namesz = strlen(cc->utilname);
	termsz = cc->termname ? strlen(cc->termname) : 0;

	ଯେତେବେଳେ ((d = readdir(dir))) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା type, score = 1;
		ଅଚଳ ଅକ୍ଷର *tk_name = ଖାଲି, *tk_term = ଖାଲି;
		size_t tk_namesz = 0, tk_termsz = 0;

		ଯଦି (*d->d_name == '.')
			ଜାରି;
#ifdef _DIRENT_HAVE_D_TYPE
		ଯଦି (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK &&
		    d->d_type != DT_REG)
			ଜାରି;
#endif
		ଯଦି (filename_to_tokens(d->d_name,
				       &tk_name, &tk_namesz,
				       &tk_term, &tk_termsz, &type) != 0)
			ଜାରି;

		/* count theoretical score before we check names to avoid
		 * unnecessary strcmp() */
		ଯଦି (tk_name)
			score += 20;
		ଯଦି (tk_term)
			score += 10;

		DBG(CONF, ul_debug("item '%s': score=%d "
			"[cur: %d, name(%zu): %s, term(%zu): %s]",
			d->d_name, score, cc->scores[type],
			tk_namesz, tk_name,
			tk_termsz, tk_term));


		ଯଦି (score < cc->scores[type])
			ଜାରି;

		/* filter out by names */
		ଯଦି (tk_namesz && (tk_namesz != namesz ||
				 strncmp(tk_name, cc->utilname, namesz) != 0))
			ଜାରି;

		ଯଦି (tk_termsz && (termsz == 0 || tk_termsz != termsz ||
				  strncmp(tk_term, cc->termname, termsz) != 0))
			ଜାରି;

		DBG(CONF, ul_debug("setting '%s' from %d -to-> %d",
					type == UL_COLORFILE_SCHEME ? "scheme" :
					type == UL_COLORFILE_DISABLE ? "disable" :
					type == UL_COLORFILE_ENABLE ? "enable" : "???",
					cc->scores[type], score));
		cc->scores[type] = score;
		rc = 0;
		ଯଦି (type == UL_COLORFILE_SCHEME)
			strncpy(sfile, d->d_name, ଆକାର(sfile));
	}

	ଯଦି (*sfile) {
		sfile[ଆକାର(sfile) - 1] = '\0';
		ଯଦି (asprintf(&cc->sfile, "%s/%s", dirname, sfile) <= 0)
			rc = -ENOMEM;
	}

	closedir(dir);
	ଫେରନ୍ତୁ rc;
}

/* atexit() wrapper */
ସ୍ଥିର ଶୂନ୍ୟ colors_deinit(ଶୂନ୍ୟ)
{
	colors_reset(&ul_colors);
}

/*
 * Returns path to $XDG_CONFIG_HOME/terminal-colors.d
 */
ସ୍ଥିର ଅକ୍ଷର *colors_get_homedir(ଅକ୍ଷର *buf, size_t bufsz)
{
	ଅକ୍ଷର *p = getenv("XDG_CONFIG_HOME");

	ଯଦି (p) {
		snprintf(buf, bufsz, "%s/" _PATH_TERMCOLORS_DIRNAME, p);
		ଫେରନ୍ତୁ buf;
	}

	p = getenv("HOME");
	ଯଦି (p) {
		snprintf(buf, bufsz, "%s/.config/" _PATH_TERMCOLORS_DIRNAME, p);
		ଫେରନ୍ତୁ buf;
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

/*
 * Adds one color sequence to array with color scheme.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା colors_add_scheme(ସଂରଚନା ul_color_ctl *cc,
			     ଅଚଳ ଅକ୍ଷର *name,
			     ଅଚଳ ଅକ୍ଷର *seq0)
{
	ସଂରଚନା ul_color_scheme *cs = ଖାଲି;
	ଅକ୍ଷର *seq = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	ଯଦି (!cc || !name || !*name || !seq0 || !*seq0)
		ଫେରନ୍ତୁ -EINVAL;

	DBG(SCHEME, ul_debug("add '%s'", name));

	seq = color_get_sequence(seq0);
	ଯଦି (!seq)
		ଫେରନ୍ତୁ -EINVAL;
	rc = -ENOMEM;

	/* enlarge the array */
	ଯଦି (cc->nschemes == cc->schemes_sz) {
		ଶୂନ୍ୟ *tmp = reallocarray(cc->schemes, cc->nschemes + 10,
					 ଆକାର(ସଂରଚନା ul_color_scheme));
		ଯଦି (!tmp)
			ଯାଅ err;
		cc->schemes = tmp;
		cc->schemes_sz = cc->nschemes + 10;
	}

	/* add a new item */
	cs = &cc->schemes[cc->nschemes];
	cs->seq = seq;
	cs->name = strdup(name);
	ଯଦି (!cs->name)
		ଯାଅ err;

	cc->nschemes++;
	ଫେରନ୍ତୁ 0;
err:
	ଯଦି (cs) {
		ମୁକ୍ତ(cs->seq);
		ମୁକ୍ତ(cs->name);
		cs->seq = cs->name = ଖାଲି;
	} ଅନ୍ୟଥା
		ମୁକ୍ତ(seq);
	ଫେରନ୍ତୁ rc;
}

/*
 * Deallocates all regards to color schemes
 */
ସ୍ଥିର ଶୂନ୍ୟ colors_free_schemes(ସଂରଚନା ul_color_ctl *cc)
{
	size_t i;

	DBG(SCHEME, ul_debug("free scheme"));

	ପାଇଁ (i = 0; i < cc->nschemes; i++) {
		ମୁକ୍ତ(cc->schemes[i].name);
		ମୁକ୍ତ(cc->schemes[i].seq);
	}

	ମୁକ୍ତ(cc->schemes);
	cc->schemes = ଖାଲି;
	cc->nschemes = 0;
	cc->schemes_sz = 0;
}

/*
 * The scheme configuration has to be sorted for bsearch
 */
ସ୍ଥିର ଶୂନ୍ୟ colors_sort_schemes(ସଂରଚନା ul_color_ctl *cc)
{
	ଯଦି (!cc->nschemes)
		ଫେରନ୍ତୁ;

	DBG(SCHEME, ul_debug("sort scheme"));

	qsort(cc->schemes, cc->nschemes,
	      ଆକାର(ସଂରଚନା ul_color_scheme), cmp_scheme_name);
}

/*
 * Returns just one color scheme
 */
ସ୍ଥିର ସଂରଚନା ul_color_scheme *colors_get_scheme(ସଂରଚନା ul_color_ctl *cc,
						 ଅଚଳ ଅକ୍ଷର *name)
{
	ସଂରଚନା ul_color_scheme ଚାବି = { .name = (ଅକ୍ଷର *) name}, *res;

	ଯଦି (!cc || !name || !*name)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (!cc->cs_configured) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = colors_read_schemes(cc);
		ଯଦି (rc)
			ଫେରନ୍ତୁ ଖାଲି;
	}
	ଯଦି (!cc->nschemes)
		ଫେରନ୍ତୁ ଖାଲି;

	DBG(SCHEME, ul_debug("search '%s'", name));

	res = bsearch(&ଚାବି, cc->schemes, cc->nschemes,
				ଆକାର(ସଂରଚନା ul_color_scheme),
				cmp_scheme_name);

	ଫେରନ୍ତୁ res && res->seq ? res  : ଖାଲି;
}

/*
 * Parses filenames in terminal-colors.d
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା colors_read_configuration(ସଂରଚନା ul_color_ctl *cc)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = -ENOENT;
	ଅକ୍ଷର *dirname, buf[PATH_MAX];

	cc->termname = getenv("TERM");

	dirname = colors_get_homedir(buf, ଆକାର(buf));
	ଯଦି (dirname)
		rc = colors_readdir(cc, dirname);		/* ~/.config */
	ଯଦି (rc == -EPERM || rc == -EACCES || rc == -ENOENT)
		rc = colors_readdir(cc, _PATH_TERMCOLORS_DIR);	/* /etc */

	cc->configured = 1;
	ଫେରନ୍ତୁ rc;
}

/*
 * Reads terminal-colors.d/ scheme file into array schemes
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା colors_read_schemes(ସଂରଚନା ul_color_ctl *cc)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;
	FILE *f = ଖାଲି;
	ଅକ୍ଷର buf[BUFSIZ],
	     cn[129], seq[129];

	ଯଦି (!cc->configured)
		rc = colors_read_configuration(cc);

	cc->cs_configured = 1;

	ଯଦି (rc || !cc->sfile)
		ଯାଅ done;

	DBG(SCHEME, ul_debug("reading file '%s'", cc->sfile));

	f = fopen(cc->sfile, "r" UL_CLOEXECSTR);
	ଯଦି (!f) {
		rc = -errno;
		ଯାଅ done;
	}

	ଯେତେବେଳେ (fgets(buf, ଆକାର(buf), f)) {
		ଅକ୍ଷର *p = strchr(buf, '\n');

		ଯଦି (!p) {
			ଯଦି (feof(f))
				p = strchr(buf, '\0');
			ଅନ୍ୟଥା {
				rc = -errno;
				ଯାଅ done;
			}
		}
		*p = '\0';
		p = (ଅକ୍ଷର *) skip_blank(buf);
		ଯଦି (*p == '\0' || *p == '#')
			ଜାରି;

		rc = sscanf(p, "%128[^ ] %128[^\n ]", cn, seq);
		ଯଦି (rc == 2 && *cn && *seq) {
			rc = colors_add_scheme(cc, cn, seq);	/* set rc=0 on success */
			ଯଦି (rc)
				ଯାଅ done;
		}
	}
	rc = 0;

done:
	ଯଦି (f)
		fclose(f);
	colors_sort_schemes(cc);

	ଫେରନ୍ତୁ rc;
}


ସ୍ଥିର ଶୂନ୍ୟ termcolors_init_debug(ଶୂନ୍ୟ)
{
	__UL_INIT_DEBUG_FROM_ENV(termcolors, TERMCOLORS_DEBUG_, 0, TERMINAL_COLORS_DEBUG);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା colors_terminal_is_ready(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ncolors = -1;

#ଯଦି defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
	{
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

		/* setupterm() allocates memory, del_curterm() deallocates it */
		ଯଦି (setupterm(ଖାଲି, STDOUT_FILENO, &ret) == 0 && ret == 1) {
			ncolors = tigetnum("colors");
			del_curterm(cur_term);
		}
	}
#endif
	ଯଦି (1 < ncolors) {
		DBG(CONF, ul_debug("terminal is ready (supports %d colors)", ncolors));
		ଫେରନ୍ତୁ 1;
	}

	DBG(CONF, ul_debug("terminal is NOT ready (no colors)"));
	ଫେରନ୍ତୁ 0;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା colors_init(ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode, ଅଚଳ ଅକ୍ଷର *name)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ready = -1;
	ସଂରଚନା ul_color_ctl *cc = &ul_colors;

	cc->utilname = name;

	termcolors_init_debug();

	ଯଦି (mode != UL_COLORMODE_ALWAYS && !isatty(STDOUT_FILENO))
		cc->mode = UL_COLORMODE_NEVER;
	ଅନ୍ୟଥା
		cc->mode = mode;

	ଯଦି (cc->mode == UL_COLORMODE_UNDEF
	    && getenv("NO_COLOR") == ଖାଲି
	    && (ready = colors_terminal_is_ready())) {

		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = colors_read_configuration(cc);
		ଯଦି (rc)
			cc->mode = UL_COLORMODE_DEFAULT;
		ଅନ୍ୟଥା {

			/* evaluate scores */
			ଯଦି (cc->scores[UL_COLORFILE_DISABLE] >
			    cc->scores[UL_COLORFILE_ENABLE])
				cc->mode = UL_COLORMODE_NEVER;
			ଅନ୍ୟଥା
				cc->mode = UL_COLORMODE_DEFAULT;

			atexit(colors_deinit);
		}
	}

	ବାଛ (cc->mode) {
	ସ୍ଥିତି UL_COLORMODE_AUTO:
		cc->has_colors = ready == -1 ? colors_terminal_is_ready() : ready;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି UL_COLORMODE_ALWAYS:
		cc->has_colors = 1;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି UL_COLORMODE_NEVER:
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		cc->has_colors = 0;
	}

	ON_DBG(CONF, colors_debug(cc));

	ଫେରନ୍ତୁ cc->has_colors;
}

/*
 * Temporary disable colors (this setting is independent on terminal-colors.d/)
 */
ଶୂନ୍ୟ colors_off(ଶୂନ୍ୟ)
{
	ul_colors.disabled = 1;
}

/*
 * Enable colors
 */
ଶୂନ୍ୟ colors_on(ଶୂନ୍ୟ)
{
	ul_colors.disabled = 0;
}

/*
 * Is terminal-colors.d/ configured to use colors?
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା colors_wanted(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ ul_colors.has_colors;
}

/*
 * Returns mode
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା colors_mode(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ ul_colors.mode;
}

/*
 * Enable @seq color
 */
ଶୂନ୍ୟ color_fenable(ଅଚଳ ଅକ୍ଷର *seq, FILE *f)
{
	ଯଦି (!ul_colors.disabled && ul_colors.has_colors && seq)
		fputs(seq, f);
}

/*
 * Returns escape sequence by logical @name, if undefined then returns @dflt.
 */
ଅଚଳ ଅକ୍ଷର *color_scheme_get_sequence(ଅଚଳ ଅକ୍ଷର *name, ଅଚଳ ଅକ୍ଷର *dflt)
{
	ସଂରଚନା ul_color_scheme *cs;

	ଯଦି (ul_colors.disabled || !ul_colors.has_colors)
		ଫେରନ୍ତୁ ଖାଲି;

	cs = colors_get_scheme(&ul_colors, name);
	ଫେରନ୍ତୁ cs && cs->seq ? cs->seq : dflt;
}

/*
 * Enable color by logical @name, if undefined enable @dflt.
 */
ଶୂନ୍ୟ color_scheme_fenable(ଅଚଳ ଅକ୍ଷର *name, ଅଚଳ ଅକ୍ଷର *dflt, FILE *f)
{
	ଅଚଳ ଅକ୍ଷର *seq = color_scheme_get_sequence(name, dflt);

	ଯଦି (!seq)
		ଫେରନ୍ତୁ;
	color_fenable(seq, f);
}


/*
 * Disable previously enabled color
 */
ଶୂନ୍ୟ color_fdisable(FILE *f)
{
	ଯଦି (!ul_colors.disabled && ul_colors.has_colors)
		fputs(UL_COLOR_RESET, f);
}

/*
 * Get reset sequence
 */
ଅଚଳ ଅକ୍ଷର *color_get_disable_sequence(ଶୂନ୍ୟ)
{
	ଯଦି (!ul_colors.disabled && ul_colors.has_colors)
		ଫେରନ୍ତୁ UL_COLOR_RESET;
	ଅନ୍ୟଥା
		ଫେରନ୍ତୁ "";
}

/*
 * Parses @str to return UL_COLORMODE_*
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା colormode_from_string(ଅଚଳ ଅକ୍ଷର *str)
{
	size_t i;
	ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ modes[] = {
		[UL_COLORMODE_AUTO]   = "auto",
		[UL_COLORMODE_NEVER]  = "never",
		[UL_COLORMODE_ALWAYS] = "always",
		[UL_COLORMODE_UNDEF] = ""
	};

	ଯଦି (!str || !*str)
		ଫେରନ୍ତୁ -EINVAL;

	ପ୍ରତିଜ୍ଞା(ARRAY_SIZE(modes) == __UL_NCOLORMODES);

	ପାଇଁ (i = 0; i < ARRAY_SIZE(modes); i++) {
		ଯଦି (c_strcasecmp(str, modes[i]) == 0)
			ଫେରନ୍ତୁ i;
	}

	ଫେରନ୍ତୁ -EINVAL;
}

/*
 * Parses @str and exit(EXIT_FAILURE) on error
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା colormode_or_err(ଅଚଳ ଅକ୍ଷର *str)
{
	ଅଚଳ ଅକ୍ଷର *p = str && *str == '=' ? str + 1 : str;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା colormode;

	colormode = colormode_from_string(p);
	ଯଦି (colormode < 0)
		errx(EXIT_FAILURE, _("unsupported color mode: %s"), p);

	ଫେରନ୍ତୁ colormode;
}

#ifdef TEST_PROGRAM_COLORS
# include <getopt.h>
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ସ୍ଥିର ଅଚଳ ସଂରଚନା option longopts[] = {
		{ "mode",	required_argument, ଖାଲି, 'm' },
		{ "color",	required_argument, ଖାଲି, 'c' },
		{ "color-scheme", required_argument, ଖାଲି, 'C' },
		{ "name",	required_argument, ଖାଲି, 'n' },
		{ ଖାଲି, 0, ଖାଲି, 0 }
	};
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା c, mode = UL_COLORMODE_UNDEF;	/* default */
	ଅଚଳ ଅକ୍ଷର *color = "red", *name = ଖାଲି, *color_scheme = ଖାଲି;
	ଅକ୍ଷର *seq = ଖାଲି;

	ଯେତେବେଳେ ((c = getopt_long(argc, argv, "C:c:m:n:", longopts, ଖାଲି)) != -1) {
		ବାଛ (c) {
		ସ୍ଥିତି 'c':
			color = optarg;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'C':
			color_scheme = optarg;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'm':
			mode = colormode_or_err(optarg);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'n':
			name = optarg;
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			fprintf(stderr, "usage: %s [options]\n"
			" -m, --mode <auto|never|always>  default is undefined\n"
			" -c, --color <red|blue|...>      color for the test message\n"
			" -C, --color-scheme <name>       color for the test message\n"
			" -n, --name <utilname>           util name\n",
			program_invocation_short_name);
			ଫେରନ୍ତୁ EXIT_FAILURE;
		}
	}

	colors_init(mode, name ? name : program_invocation_short_name);

	ଯଦି (color_is_sequence(color))
		seq = strdup(color);
	ଅନ୍ୟଥା
		seq = color_get_sequence(color);

	ଯଦି (color_scheme)
		color_scheme_enable(color_scheme, seq);
	ଅନ୍ୟଥା
		color_enable(seq);
	printf("Hello World!");
	color_disable();
	fputc('\n', stdout);

	ମୁକ୍ତ(seq);

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_COLORS */

