/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: 2012 Ondrej Oprala <ooprala@redhat.com>
 *          2012-2025 Karel Zak <kzak@redhat.com>
 */
#অন্তর্ভাৱ <assert.h>
#অন্তর্ভাৱ <sys/stat.h>
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <dirent.h>
#অন্তর্ভাৱ <ctype.h>

#যদি defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
# যদি defined(HAVE_NCURSESW_NCURSES_H)
#  include <ncursesw/ncurses.h>
# elif defined(HAVE_NCURSES_NCURSES_H)
#  include <ncurses/ncurses.h>
# elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
# endif
# যদি defined(HAVE_NCURSESW_TERM_H)
#  include <ncursesw/term.h>
# elif defined(HAVE_NCURSES_TERM_H)
#  include <ncurses/term.h>
# elif defined(HAVE_TERM_H)
#  include <term.h>
# endif
#endif

#অন্তর্ভাৱ "c.h"
#অন্তর্ভাৱ "cctype.h"
#অন্তর্ভাৱ "colors.h"
#অন্তর্ভাৱ "nls.h"
#অন্তর্ভাৱ "pathnames.h"
#অন্তর্ভাৱ "strutils.h"

#অন্তর্ভাৱ "debug.h"

/*
 * Default behavior, may be overridden by terminal-colors.d/{enable,disable}.
 */
#ifdef USE_COLORS_BY_DEFAULT
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_AUTO	/* check isatty() */
#অন্যথা
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_NEVER	/* no colors by default */
#endif

/*
 * terminal-colors.d debug stuff
 */
স্থির UL_DEBUG_DEFINE_MASK(termcolors);
UL_DEBUG_DEFINE_MASKNAMES(termcolors) = UL_DEBUG_EMPTY_MASKNAMES;

#define TERMCOLORS_DEBUG_INIT	(1 << 1)
#define TERMCOLORS_DEBUG_CONF	(1 << 2)
#define TERMCOLORS_DEBUG_SCHEME	(1 << 3)

#define DBG(m, x)       __UL_DBG(termcolors, TERMCOLORS_DEBUG_, m, x)
#define ON_DBG(m, x)    __UL_DBG_CALL(termcolors, TERMCOLORS_DEBUG_, m, x)

/*
 * terminal-colors.d file types
 */
গণনা {
	UL_COLORFILE_DISABLE,		/* .disable */
	UL_COLORFILE_ENABLE,		/* .enable */
	UL_COLORFILE_SCHEME,		/* .scheme */

	__UL_COLORFILE_COUNT
};

সংরচনা ul_color_scheme {
	ৱর্ণ *name;
	ৱর্ণ *seq;
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
সংরচনা ul_color_ctl {
	ধ্রুৱ ৱর্ণ	*utilname;	/* util name */
	ধ্রুৱ ৱর্ণ	*termname;	/* terminal name ($TERM) */

	ৱর্ণ		*sfile;	/* path to scheme */

	সংরচনা ul_color_scheme	*schemes;	/* array with color schemes */
	size_t			nschemes;	/* number of the items */
	size_t			schemes_sz;	/* number of the allocated items */

	পূর্ণাঙ্ক		mode;		/* UL_COLORMODE_* */
	unsigned পূর্ণাঙ্ক	has_colors : 1,	/* based on mode and scores[] */
			disabled   : 1, /* disable colors */
			cs_configured : 1, /* color schemes read */
			configured : 1; /* terminal-colors.d parsed */

	পূর্ণাঙ্ক		scores[__UL_COLORFILE_COUNT];	/* the best match */
};

/*
 * Control struct, globally shared.
 */
স্থির সংরচনা ul_color_ctl ul_colors;

স্থির শূন্য colors_free_schemes(সংরচনা ul_color_ctl *cc);
স্থির পূর্ণাঙ্ক colors_read_schemes(সংরচনা ul_color_ctl *cc);

/*
 * qsort/bsearch buddy
 */
স্থির পূর্ণাঙ্ক cmp_scheme_name(ধ্রুৱ শূন্য *a0, ধ্রুৱ শূন্য *b0)
{
	ধ্রুৱ সংরচনা ul_color_scheme *a = (ধ্রুৱ সংরচনা ul_color_scheme *) a0,
				     *b = (ধ্রুৱ সংরচনা ul_color_scheme *) b0;
	ঘূৰাই strcmp(a->name, b->name);
}

/*
 * Resets control struct (note that we don't allocate the struct)
 */
স্থির শূন্য colors_reset(সংরচনা ul_color_ctl *cc)
{
	যদি (!cc)
		ঘূৰাই;

	colors_free_schemes(cc);

	মুক্ত(cc->sfile);

	cc->sfile = রিক্ত;
	cc->utilname = রিক্ত;
	cc->termname = রিক্ত;
	cc->mode = UL_COLORMODE_UNDEF;

	memset(cc->scores, 0, পরিমাণ(cc->scores));
}

স্থির শূন্য colors_debug(সংরচনা ul_color_ctl *cc)
{
	size_t i;

	যদি (!cc)
		ঘূৰাই;

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

	বাবে (i = 0; i < ARRAY_SIZE(cc->scores); i++)
		printf("\tscore %s = %d\n",
				i == UL_COLORFILE_DISABLE ? "disable" :
				i == UL_COLORFILE_ENABLE ? "enable" :
				i == UL_COLORFILE_SCHEME ? "scheme" : "???",
				cc->scores[i]);

	fputc('\n', stdout);

	বাবে (i = 0; i < cc->nschemes; i++) {
		printf("\tscheme #%02zu ", i);
		color_scheme_enable(cc->schemes[i].name, রিক্ত);
		fputs(cc->schemes[i].name, stdout);
		color_disable();
		fputc('\n', stdout);
	}
	fputc('\n', stdout);
}

/*
 * Parses [[<utilname>][@<termname>].]<type>
 */
স্থির পূর্ণাঙ্ক filename_to_tokens(ধ্রুৱ ৱর্ণ *str,
			      ধ্রুৱ ৱর্ণ **name, size_t *namesz,
			      ধ্রুৱ ৱর্ণ **term, size_t *termsz,
			      পূর্ণাঙ্ক  *filetype)
{
	ধ্রুৱ ৱর্ণ *type_start, *term_start, *p;

	যদি (!str || !*str || *str == '.' || strlen(str) > PATH_MAX)
		ঘূৰাই -EINVAL;

	/* parse .type */
	p = strrchr(str, '.');
	type_start = p ? p + 1 : str;

	যদি (strcmp(type_start, "disable") == 0)
		*filetype = UL_COLORFILE_DISABLE;
	অন্যথা যদি (strcmp(type_start, "enable") == 0)
		*filetype = UL_COLORFILE_ENABLE;
	অন্যথা যদি (strcmp(type_start, "scheme") == 0)
		*filetype = UL_COLORFILE_SCHEME;
	অন্যথা {
		DBG(CONF, ul_debug("unknown type '%s'", type_start));
		ঘূৰাই 1;				/* unknown type */
	}

	যদি (type_start == str)
		ঘূৰাই 0;				/* "type" only */

	/* parse @termname */
	p = strchr(str, '@');
	term_start = p ? p + 1 : রিক্ত;
	যদি (term_start) {
		*term = term_start;
		*termsz = type_start - term_start - 1;
		যদি (term_start - 1 == str)
			ঘূৰাই 0;			/* "@termname.type" */
	}

	/* parse utilname */
	p = term_start ? term_start : type_start;
	*name = str;
	*namesz = p - str - 1;

	ঘূৰাই 0;
}

/*
 * Scans @dirname and select the best matches for UL_COLORFILE_* types.
 * The result is stored to cc->scores. The path to the best "scheme"
 * file is stored to cc->scheme.
 */
স্থির পূর্ণাঙ্ক colors_readdir(সংরচনা ul_color_ctl *cc, ধ্রুৱ ৱর্ণ *dirname)
{
	DIR *dir;
	পূর্ণাঙ্ক rc = -ENOENT;
	সংরচনা dirent *d;
	ৱর্ণ sfile[PATH_MAX] = { '\0' };
	size_t namesz, termsz;

	যদি (!dirname || !cc || !cc->utilname || !*cc->utilname)
		ঘূৰাই -EINVAL;

	DBG(CONF, ul_debug("reading dir: '%s'", dirname));

	dir = opendir(dirname);
	যদি (!dir)
		ঘূৰাই -errno;

	namesz = strlen(cc->utilname);
	termsz = cc->termname ? strlen(cc->termname) : 0;

	যেতিয়ালৈ ((d = readdir(dir))) {
		পূর্ণাঙ্ক type, score = 1;
		ধ্রুৱ ৱর্ণ *tk_name = রিক্ত, *tk_term = রিক্ত;
		size_t tk_namesz = 0, tk_termsz = 0;

		যদি (*d->d_name == '.')
			অনুৱর্ত;
#ifdef _DIRENT_HAVE_D_TYPE
		যদি (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK &&
		    d->d_type != DT_REG)
			অনুৱর্ত;
#endif
		যদি (filename_to_tokens(d->d_name,
				       &tk_name, &tk_namesz,
				       &tk_term, &tk_termsz, &type) != 0)
			অনুৱর্ত;

		/* count theoretical score before we check names to avoid
		 * unnecessary strcmp() */
		যদি (tk_name)
			score += 20;
		যদি (tk_term)
			score += 10;

		DBG(CONF, ul_debug("item '%s': score=%d "
			"[cur: %d, name(%zu): %s, term(%zu): %s]",
			d->d_name, score, cc->scores[type],
			tk_namesz, tk_name,
			tk_termsz, tk_term));


		যদি (score < cc->scores[type])
			অনুৱর্ত;

		/* filter out by names */
		যদি (tk_namesz && (tk_namesz != namesz ||
				 strncmp(tk_name, cc->utilname, namesz) != 0))
			অনুৱর্ত;

		যদি (tk_termsz && (termsz == 0 || tk_termsz != termsz ||
				  strncmp(tk_term, cc->termname, termsz) != 0))
			অনুৱর্ত;

		DBG(CONF, ul_debug("setting '%s' from %d -to-> %d",
					type == UL_COLORFILE_SCHEME ? "scheme" :
					type == UL_COLORFILE_DISABLE ? "disable" :
					type == UL_COLORFILE_ENABLE ? "enable" : "???",
					cc->scores[type], score));
		cc->scores[type] = score;
		rc = 0;
		যদি (type == UL_COLORFILE_SCHEME)
			strncpy(sfile, d->d_name, পরিমাণ(sfile));
	}

	যদি (*sfile) {
		sfile[পরিমাণ(sfile) - 1] = '\0';
		যদি (asprintf(&cc->sfile, "%s/%s", dirname, sfile) <= 0)
			rc = -ENOMEM;
	}

	closedir(dir);
	ঘূৰাই rc;
}

/* atexit() wrapper */
স্থির শূন্য colors_deinit(শূন্য)
{
	colors_reset(&ul_colors);
}

/*
 * Returns path to $XDG_CONFIG_HOME/terminal-colors.d
 */
স্থির ৱর্ণ *colors_get_homedir(ৱর্ণ *buf, size_t bufsz)
{
	ৱর্ণ *p = getenv("XDG_CONFIG_HOME");

	যদি (p) {
		snprintf(buf, bufsz, "%s/" _PATH_TERMCOLORS_DIRNAME, p);
		ঘূৰাই buf;
	}

	p = getenv("HOME");
	যদি (p) {
		snprintf(buf, bufsz, "%s/.config/" _PATH_TERMCOLORS_DIRNAME, p);
		ঘূৰাই buf;
	}

	ঘূৰাই রিক্ত;
}

/*
 * Adds one color sequence to array with color scheme.
 */
স্থির পূর্ণাঙ্ক colors_add_scheme(সংরচনা ul_color_ctl *cc,
			     ধ্রুৱ ৱর্ণ *name,
			     ধ্রুৱ ৱর্ণ *seq0)
{
	সংরচনা ul_color_scheme *cs = রিক্ত;
	ৱর্ণ *seq = রিক্ত;
	পূর্ণাঙ্ক rc;

	যদি (!cc || !name || !*name || !seq0 || !*seq0)
		ঘূৰাই -EINVAL;

	DBG(SCHEME, ul_debug("add '%s'", name));

	seq = color_get_sequence(seq0);
	যদি (!seq)
		ঘূৰাই -EINVAL;
	rc = -ENOMEM;

	/* enlarge the array */
	যদি (cc->nschemes == cc->schemes_sz) {
		শূন্য *tmp = reallocarray(cc->schemes, cc->nschemes + 10,
					 পরিমাণ(সংরচনা ul_color_scheme));
		যদি (!tmp)
			গচ্ছ err;
		cc->schemes = tmp;
		cc->schemes_sz = cc->nschemes + 10;
	}

	/* add a new item */
	cs = &cc->schemes[cc->nschemes];
	cs->seq = seq;
	cs->name = strdup(name);
	যদি (!cs->name)
		গচ্ছ err;

	cc->nschemes++;
	ঘূৰাই 0;
err:
	যদি (cs) {
		মুক্ত(cs->seq);
		মুক্ত(cs->name);
		cs->seq = cs->name = রিক্ত;
	} অন্যথা
		মুক্ত(seq);
	ঘূৰাই rc;
}

/*
 * Deallocates all regards to color schemes
 */
স্থির শূন্য colors_free_schemes(সংরচনা ul_color_ctl *cc)
{
	size_t i;

	DBG(SCHEME, ul_debug("free scheme"));

	বাবে (i = 0; i < cc->nschemes; i++) {
		মুক্ত(cc->schemes[i].name);
		মুক্ত(cc->schemes[i].seq);
	}

	মুক্ত(cc->schemes);
	cc->schemes = রিক্ত;
	cc->nschemes = 0;
	cc->schemes_sz = 0;
}

/*
 * The scheme configuration has to be sorted for bsearch
 */
স্থির শূন্য colors_sort_schemes(সংরচনা ul_color_ctl *cc)
{
	যদি (!cc->nschemes)
		ঘূৰাই;

	DBG(SCHEME, ul_debug("sort scheme"));

	qsort(cc->schemes, cc->nschemes,
	      পরিমাণ(সংরচনা ul_color_scheme), cmp_scheme_name);
}

/*
 * Returns just one color scheme
 */
স্থির সংরচনা ul_color_scheme *colors_get_scheme(সংরচনা ul_color_ctl *cc,
						 ধ্রুৱ ৱর্ণ *name)
{
	সংরচনা ul_color_scheme চাবি = { .name = (ৱর্ণ *) name}, *res;

	যদি (!cc || !name || !*name)
		ঘূৰাই রিক্ত;

	যদি (!cc->cs_configured) {
		পূর্ণাঙ্ক rc = colors_read_schemes(cc);
		যদি (rc)
			ঘূৰাই রিক্ত;
	}
	যদি (!cc->nschemes)
		ঘূৰাই রিক্ত;

	DBG(SCHEME, ul_debug("search '%s'", name));

	res = bsearch(&চাবি, cc->schemes, cc->nschemes,
				পরিমাণ(সংরচনা ul_color_scheme),
				cmp_scheme_name);

	ঘূৰাই res && res->seq ? res  : রিক্ত;
}

/*
 * Parses filenames in terminal-colors.d
 */
স্থির পূর্ণাঙ্ক colors_read_configuration(সংরচনা ul_color_ctl *cc)
{
	পূর্ণাঙ্ক rc = -ENOENT;
	ৱর্ণ *dirname, buf[PATH_MAX];

	cc->termname = getenv("TERM");

	dirname = colors_get_homedir(buf, পরিমাণ(buf));
	যদি (dirname)
		rc = colors_readdir(cc, dirname);		/* ~/.config */
	যদি (rc == -EPERM || rc == -EACCES || rc == -ENOENT)
		rc = colors_readdir(cc, _PATH_TERMCOLORS_DIR);	/* /etc */

	cc->configured = 1;
	ঘূৰাই rc;
}

/*
 * Reads terminal-colors.d/ scheme file into array schemes
 */
স্থির পূর্ণাঙ্ক colors_read_schemes(সংরচনা ul_color_ctl *cc)
{
	পূর্ণাঙ্ক rc = 0;
	FILE *f = রিক্ত;
	ৱর্ণ buf[BUFSIZ],
	     cn[129], seq[129];

	যদি (!cc->configured)
		rc = colors_read_configuration(cc);

	cc->cs_configured = 1;

	যদি (rc || !cc->sfile)
		গচ্ছ done;

	DBG(SCHEME, ul_debug("reading file '%s'", cc->sfile));

	f = fopen(cc->sfile, "r" UL_CLOEXECSTR);
	যদি (!f) {
		rc = -errno;
		গচ্ছ done;
	}

	যেতিয়ালৈ (fgets(buf, পরিমাণ(buf), f)) {
		ৱর্ণ *p = strchr(buf, '\n');

		যদি (!p) {
			যদি (feof(f))
				p = strchr(buf, '\0');
			অন্যথা {
				rc = -errno;
				গচ্ছ done;
			}
		}
		*p = '\0';
		p = (ৱর্ণ *) skip_blank(buf);
		যদি (*p == '\0' || *p == '#')
			অনুৱর্ত;

		rc = sscanf(p, "%128[^ ] %128[^\n ]", cn, seq);
		যদি (rc == 2 && *cn && *seq) {
			rc = colors_add_scheme(cc, cn, seq);	/* set rc=0 on success */
			যদি (rc)
				গচ্ছ done;
		}
	}
	rc = 0;

done:
	যদি (f)
		fclose(f);
	colors_sort_schemes(cc);

	ঘূৰাই rc;
}


স্থির শূন্য termcolors_init_debug(শূন্য)
{
	__UL_INIT_DEBUG_FROM_ENV(termcolors, TERMCOLORS_DEBUG_, 0, TERMINAL_COLORS_DEBUG);
}

স্থির পূর্ণাঙ্ক colors_terminal_is_ready(শূন্য)
{
	পূর্ণাঙ্ক ncolors = -1;

#যদি defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
	{
		পূর্ণাঙ্ক ret;

		/* setupterm() allocates memory, del_curterm() deallocates it */
		যদি (setupterm(রিক্ত, STDOUT_FILENO, &ret) == 0 && ret == 1) {
			ncolors = tigetnum("colors");
			del_curterm(cur_term);
		}
	}
#endif
	যদি (1 < ncolors) {
		DBG(CONF, ul_debug("terminal is ready (supports %d colors)", ncolors));
		ঘূৰাই 1;
	}

	DBG(CONF, ul_debug("terminal is NOT ready (no colors)"));
	ঘূৰাই 0;
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
পূর্ণাঙ্ক colors_init(পূর্ণাঙ্ক mode, ধ্রুৱ ৱর্ণ *name)
{
	পূর্ণাঙ্ক ready = -1;
	সংরচনা ul_color_ctl *cc = &ul_colors;

	cc->utilname = name;

	termcolors_init_debug();

	যদি (mode != UL_COLORMODE_ALWAYS && !isatty(STDOUT_FILENO))
		cc->mode = UL_COLORMODE_NEVER;
	অন্যথা
		cc->mode = mode;

	যদি (cc->mode == UL_COLORMODE_UNDEF
	    && getenv("NO_COLOR") == রিক্ত
	    && (ready = colors_terminal_is_ready())) {

		পূর্ণাঙ্ক rc = colors_read_configuration(cc);
		যদি (rc)
			cc->mode = UL_COLORMODE_DEFAULT;
		অন্যথা {

			/* evaluate scores */
			যদি (cc->scores[UL_COLORFILE_DISABLE] >
			    cc->scores[UL_COLORFILE_ENABLE])
				cc->mode = UL_COLORMODE_NEVER;
			অন্যথা
				cc->mode = UL_COLORMODE_DEFAULT;

			atexit(colors_deinit);
		}
	}

	ৱিকল্প (cc->mode) {
	পক্ষ UL_COLORMODE_AUTO:
		cc->has_colors = ready == -1 ? colors_terminal_is_ready() : ready;
		ভঙ্গ;
	পক্ষ UL_COLORMODE_ALWAYS:
		cc->has_colors = 1;
		ভঙ্গ;
	পক্ষ UL_COLORMODE_NEVER:
	স্ৱভাৱ:
		cc->has_colors = 0;
	}

	ON_DBG(CONF, colors_debug(cc));

	ঘূৰাই cc->has_colors;
}

/*
 * Temporary disable colors (this setting is independent on terminal-colors.d/)
 */
শূন্য colors_off(শূন্য)
{
	ul_colors.disabled = 1;
}

/*
 * Enable colors
 */
শূন্য colors_on(শূন্য)
{
	ul_colors.disabled = 0;
}

/*
 * Is terminal-colors.d/ configured to use colors?
 */
পূর্ণাঙ্ক colors_wanted(শূন্য)
{
	ঘূৰাই ul_colors.has_colors;
}

/*
 * Returns mode
 */
পূর্ণাঙ্ক colors_mode(শূন্য)
{
	ঘূৰাই ul_colors.mode;
}

/*
 * Enable @seq color
 */
শূন্য color_fenable(ধ্রুৱ ৱর্ণ *seq, FILE *f)
{
	যদি (!ul_colors.disabled && ul_colors.has_colors && seq)
		fputs(seq, f);
}

/*
 * Returns escape sequence by logical @name, if undefined then returns @dflt.
 */
ধ্রুৱ ৱর্ণ *color_scheme_get_sequence(ধ্রুৱ ৱর্ণ *name, ধ্রুৱ ৱর্ণ *dflt)
{
	সংরচনা ul_color_scheme *cs;

	যদি (ul_colors.disabled || !ul_colors.has_colors)
		ঘূৰাই রিক্ত;

	cs = colors_get_scheme(&ul_colors, name);
	ঘূৰাই cs && cs->seq ? cs->seq : dflt;
}

/*
 * Enable color by logical @name, if undefined enable @dflt.
 */
শূন্য color_scheme_fenable(ধ্রুৱ ৱর্ণ *name, ধ্রুৱ ৱর্ণ *dflt, FILE *f)
{
	ধ্রুৱ ৱর্ণ *seq = color_scheme_get_sequence(name, dflt);

	যদি (!seq)
		ঘূৰাই;
	color_fenable(seq, f);
}


/*
 * Disable previously enabled color
 */
শূন্য color_fdisable(FILE *f)
{
	যদি (!ul_colors.disabled && ul_colors.has_colors)
		fputs(UL_COLOR_RESET, f);
}

/*
 * Get reset sequence
 */
ধ্রুৱ ৱর্ণ *color_get_disable_sequence(শূন্য)
{
	যদি (!ul_colors.disabled && ul_colors.has_colors)
		ঘূৰাই UL_COLOR_RESET;
	অন্যথা
		ঘূৰাই "";
}

/*
 * Parses @str to return UL_COLORMODE_*
 */
পূর্ণাঙ্ক colormode_from_string(ধ্রুৱ ৱর্ণ *str)
{
	size_t i;
	স্থির ধ্রুৱ ৱর্ণ *ধ্রুৱ modes[] = {
		[UL_COLORMODE_AUTO]   = "auto",
		[UL_COLORMODE_NEVER]  = "never",
		[UL_COLORMODE_ALWAYS] = "always",
		[UL_COLORMODE_UNDEF] = ""
	};

	যদি (!str || !*str)
		ঘূৰাই -EINVAL;

	প্রতিজ্ঞা(ARRAY_SIZE(modes) == __UL_NCOLORMODES);

	বাবে (i = 0; i < ARRAY_SIZE(modes); i++) {
		যদি (c_strcasecmp(str, modes[i]) == 0)
			ঘূৰাই i;
	}

	ঘূৰাই -EINVAL;
}

/*
 * Parses @str and exit(EXIT_FAILURE) on error
 */
পূর্ণাঙ্ক colormode_or_err(ধ্রুৱ ৱর্ণ *str)
{
	ধ্রুৱ ৱর্ণ *p = str && *str == '=' ? str + 1 : str;
	পূর্ণাঙ্ক colormode;

	colormode = colormode_from_string(p);
	যদি (colormode < 0)
		errx(EXIT_FAILURE, _("unsupported color mode: %s"), p);

	ঘূৰাই colormode;
}

#ifdef TEST_PROGRAM_COLORS
# include <getopt.h>
পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ *argv[])
{
	স্থির ধ্রুৱ সংরচনা option longopts[] = {
		{ "mode",	required_argument, রিক্ত, 'm' },
		{ "color",	required_argument, রিক্ত, 'c' },
		{ "color-scheme", required_argument, রিক্ত, 'C' },
		{ "name",	required_argument, রিক্ত, 'n' },
		{ রিক্ত, 0, রিক্ত, 0 }
	};
	পূর্ণাঙ্ক c, mode = UL_COLORMODE_UNDEF;	/* default */
	ধ্রুৱ ৱর্ণ *color = "red", *name = রিক্ত, *color_scheme = রিক্ত;
	ৱর্ণ *seq = রিক্ত;

	যেতিয়ালৈ ((c = getopt_long(argc, argv, "C:c:m:n:", longopts, রিক্ত)) != -1) {
		ৱিকল্প (c) {
		পক্ষ 'c':
			color = optarg;
			ভঙ্গ;
		পক্ষ 'C':
			color_scheme = optarg;
			ভঙ্গ;
		পক্ষ 'm':
			mode = colormode_or_err(optarg);
			ভঙ্গ;
		পক্ষ 'n':
			name = optarg;
			ভঙ্গ;
		স্ৱভাৱ:
			fprintf(stderr, "usage: %s [options]\n"
			" -m, --mode <auto|never|always>  default is undefined\n"
			" -c, --color <red|blue|...>      color for the test message\n"
			" -C, --color-scheme <name>       color for the test message\n"
			" -n, --name <utilname>           util name\n",
			program_invocation_short_name);
			ঘূৰাই EXIT_FAILURE;
		}
	}

	colors_init(mode, name ? name : program_invocation_short_name);

	যদি (color_is_sequence(color))
		seq = strdup(color);
	অন্যথা
		seq = color_get_sequence(color);

	যদি (color_scheme)
		color_scheme_enable(color_scheme, seq);
	অন্যথা
		color_enable(seq);
	printf("Hello World!");
	color_disable();
	fputc('\n', stdout);

	মুক্ত(seq);

	ঘূৰাই EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_COLORS */

