/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: 2012 Ondrej Oprala <ooprala@redhat.com>
 *          2012-2025 Karel Zak <kzak@redhat.com>
 */
#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <dirent.h>
#അന്തര്ഭാവ <ctype.h>

#എങ്കിൽ defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
# എങ്കിൽ defined(HAVE_NCURSESW_NCURSES_H)
#  include <ncursesw/ncurses.h>
# elif defined(HAVE_NCURSES_NCURSES_H)
#  include <ncurses/ncurses.h>
# elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
# endif
# എങ്കിൽ defined(HAVE_NCURSESW_TERM_H)
#  include <ncursesw/term.h>
# elif defined(HAVE_NCURSES_TERM_H)
#  include <ncurses/term.h>
# elif defined(HAVE_TERM_H)
#  include <term.h>
# endif
#endif

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "cctype.h"
#അന്തര്ഭാവ "colors.h"
#അന്തര്ഭാവ "nls.h"
#അന്തര്ഭാവ "pathnames.h"
#അന്തര്ഭാവ "strutils.h"

#അന്തര്ഭാവ "debug.h"

/*
 * Default behavior, may be overridden by terminal-colors.d/{enable,disable}.
 */
#ifdef USE_COLORS_BY_DEFAULT
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_AUTO	/* check isatty() */
#അല്ലെങ്കിൽ
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_NEVER	/* no colors by default */
#endif

/*
 * terminal-colors.d debug stuff
 */
സ്ഥിതം UL_DEBUG_DEFINE_MASK(termcolors);
UL_DEBUG_DEFINE_MASKNAMES(termcolors) = UL_DEBUG_EMPTY_MASKNAMES;

#define TERMCOLORS_DEBUG_INIT	(1 << 1)
#define TERMCOLORS_DEBUG_CONF	(1 << 2)
#define TERMCOLORS_DEBUG_SCHEME	(1 << 3)

#define DBG(m, x)       __UL_DBG(termcolors, TERMCOLORS_DEBUG_, m, x)
#define ON_DBG(m, x)    __UL_DBG_CALL(termcolors, TERMCOLORS_DEBUG_, m, x)

/*
 * terminal-colors.d file types
 */
എണ്ണൽ {
	UL_COLORFILE_DISABLE,		/* .disable */
	UL_COLORFILE_ENABLE,		/* .enable */
	UL_COLORFILE_SCHEME,		/* .scheme */

	__UL_COLORFILE_COUNT
};

ഘടന ul_color_scheme {
	അക്ഷരം *name;
	അക്ഷരം *seq;
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
ഘടന ul_color_ctl {
	സ്ഥിരം അക്ഷരം	*utilname;	/* util name */
	സ്ഥിരം അക്ഷരം	*termname;	/* terminal name ($TERM) */

	അക്ഷരം		*sfile;	/* path to scheme */

	ഘടന ul_color_scheme	*schemes;	/* array with color schemes */
	size_t			nschemes;	/* number of the items */
	size_t			schemes_sz;	/* number of the allocated items */

	പൂർണ്ണസംഖ്യ		mode;		/* UL_COLORMODE_* */
	unsigned പൂർണ്ണസംഖ്യ	has_colors : 1,	/* based on mode and scores[] */
			disabled   : 1, /* disable colors */
			cs_configured : 1, /* color schemes read */
			configured : 1; /* terminal-colors.d parsed */

	പൂർണ്ണസംഖ്യ		scores[__UL_COLORFILE_COUNT];	/* the best match */
};

/*
 * Control struct, globally shared.
 */
സ്ഥിതം ഘടന ul_color_ctl ul_colors;

സ്ഥിതം ശൂന്യം colors_free_schemes(ഘടന ul_color_ctl *cc);
സ്ഥിതം പൂർണ്ണസംഖ്യ colors_read_schemes(ഘടന ul_color_ctl *cc);

/*
 * qsort/bsearch buddy
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ cmp_scheme_name(സ്ഥിരം ശൂന്യം *a0, സ്ഥിരം ശൂന്യം *b0)
{
	സ്ഥിരം ഘടന ul_color_scheme *a = (സ്ഥിരം ഘടന ul_color_scheme *) a0,
				     *b = (സ്ഥിരം ഘടന ul_color_scheme *) b0;
	തിരികെ strcmp(a->name, b->name);
}

/*
 * Resets control struct (note that we don't allocate the struct)
 */
സ്ഥിതം ശൂന്യം colors_reset(ഘടന ul_color_ctl *cc)
{
	എങ്കിൽ (!cc)
		തിരികെ;

	colors_free_schemes(cc);

	സ്വതന്ത്രം(cc->sfile);

	cc->sfile = ശൂന്യ;
	cc->utilname = ശൂന്യ;
	cc->termname = ശൂന്യ;
	cc->mode = UL_COLORMODE_UNDEF;

	memset(cc->scores, 0, വലുപ്പം(cc->scores));
}

സ്ഥിതം ശൂന്യം colors_debug(ഘടന ul_color_ctl *cc)
{
	size_t i;

	എങ്കിൽ (!cc)
		തിരികെ;

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

	വേണ്ടി (i = 0; i < ARRAY_SIZE(cc->scores); i++)
		printf("\tscore %s = %d\n",
				i == UL_COLORFILE_DISABLE ? "disable" :
				i == UL_COLORFILE_ENABLE ? "enable" :
				i == UL_COLORFILE_SCHEME ? "scheme" : "???",
				cc->scores[i]);

	fputc('\n', stdout);

	വേണ്ടി (i = 0; i < cc->nschemes; i++) {
		printf("\tscheme #%02zu ", i);
		color_scheme_enable(cc->schemes[i].name, ശൂന്യ);
		fputs(cc->schemes[i].name, stdout);
		color_disable();
		fputc('\n', stdout);
	}
	fputc('\n', stdout);
}

/*
 * Parses [[<utilname>][@<termname>].]<type>
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ filename_to_tokens(സ്ഥിരം അക്ഷരം *str,
			      സ്ഥിരം അക്ഷരം **name, size_t *namesz,
			      സ്ഥിരം അക്ഷരം **term, size_t *termsz,
			      പൂർണ്ണസംഖ്യ  *filetype)
{
	സ്ഥിരം അക്ഷരം *type_start, *term_start, *p;

	എങ്കിൽ (!str || !*str || *str == '.' || strlen(str) > PATH_MAX)
		തിരികെ -EINVAL;

	/* parse .type */
	p = strrchr(str, '.');
	type_start = p ? p + 1 : str;

	എങ്കിൽ (strcmp(type_start, "disable") == 0)
		*filetype = UL_COLORFILE_DISABLE;
	അല്ലെങ്കിൽ എങ്കിൽ (strcmp(type_start, "enable") == 0)
		*filetype = UL_COLORFILE_ENABLE;
	അല്ലെങ്കിൽ എങ്കിൽ (strcmp(type_start, "scheme") == 0)
		*filetype = UL_COLORFILE_SCHEME;
	അല്ലെങ്കിൽ {
		DBG(CONF, ul_debug("unknown type '%s'", type_start));
		തിരികെ 1;				/* unknown type */
	}

	എങ്കിൽ (type_start == str)
		തിരികെ 0;				/* "type" only */

	/* parse @termname */
	p = strchr(str, '@');
	term_start = p ? p + 1 : ശൂന്യ;
	എങ്കിൽ (term_start) {
		*term = term_start;
		*termsz = type_start - term_start - 1;
		എങ്കിൽ (term_start - 1 == str)
			തിരികെ 0;			/* "@termname.type" */
	}

	/* parse utilname */
	p = term_start ? term_start : type_start;
	*name = str;
	*namesz = p - str - 1;

	തിരികെ 0;
}

/*
 * Scans @dirname and select the best matches for UL_COLORFILE_* types.
 * The result is stored to cc->scores. The path to the best "scheme"
 * file is stored to cc->scheme.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ colors_readdir(ഘടന ul_color_ctl *cc, സ്ഥിരം അക്ഷരം *dirname)
{
	DIR *dir;
	പൂർണ്ണസംഖ്യ rc = -ENOENT;
	ഘടന dirent *d;
	അക്ഷരം sfile[PATH_MAX] = { '\0' };
	size_t namesz, termsz;

	എങ്കിൽ (!dirname || !cc || !cc->utilname || !*cc->utilname)
		തിരികെ -EINVAL;

	DBG(CONF, ul_debug("reading dir: '%s'", dirname));

	dir = opendir(dirname);
	എങ്കിൽ (!dir)
		തിരികെ -errno;

	namesz = strlen(cc->utilname);
	termsz = cc->termname ? strlen(cc->termname) : 0;

	ആകുമ്പോൾ ((d = readdir(dir))) {
		പൂർണ്ണസംഖ്യ type, score = 1;
		സ്ഥിരം അക്ഷരം *tk_name = ശൂന്യ, *tk_term = ശൂന്യ;
		size_t tk_namesz = 0, tk_termsz = 0;

		എങ്കിൽ (*d->d_name == '.')
			തുടരുക;
#ifdef _DIRENT_HAVE_D_TYPE
		എങ്കിൽ (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK &&
		    d->d_type != DT_REG)
			തുടരുക;
#endif
		എങ്കിൽ (filename_to_tokens(d->d_name,
				       &tk_name, &tk_namesz,
				       &tk_term, &tk_termsz, &type) != 0)
			തുടരുക;

		/* count theoretical score before we check names to avoid
		 * unnecessary strcmp() */
		എങ്കിൽ (tk_name)
			score += 20;
		എങ്കിൽ (tk_term)
			score += 10;

		DBG(CONF, ul_debug("item '%s': score=%d "
			"[cur: %d, name(%zu): %s, term(%zu): %s]",
			d->d_name, score, cc->scores[type],
			tk_namesz, tk_name,
			tk_termsz, tk_term));


		എങ്കിൽ (score < cc->scores[type])
			തുടരുക;

		/* filter out by names */
		എങ്കിൽ (tk_namesz && (tk_namesz != namesz ||
				 strncmp(tk_name, cc->utilname, namesz) != 0))
			തുടരുക;

		എങ്കിൽ (tk_termsz && (termsz == 0 || tk_termsz != termsz ||
				  strncmp(tk_term, cc->termname, termsz) != 0))
			തുടരുക;

		DBG(CONF, ul_debug("setting '%s' from %d -to-> %d",
					type == UL_COLORFILE_SCHEME ? "scheme" :
					type == UL_COLORFILE_DISABLE ? "disable" :
					type == UL_COLORFILE_ENABLE ? "enable" : "???",
					cc->scores[type], score));
		cc->scores[type] = score;
		rc = 0;
		എങ്കിൽ (type == UL_COLORFILE_SCHEME)
			strncpy(sfile, d->d_name, വലുപ്പം(sfile));
	}

	എങ്കിൽ (*sfile) {
		sfile[വലുപ്പം(sfile) - 1] = '\0';
		എങ്കിൽ (asprintf(&cc->sfile, "%s/%s", dirname, sfile) <= 0)
			rc = -ENOMEM;
	}

	closedir(dir);
	തിരികെ rc;
}

/* atexit() wrapper */
സ്ഥിതം ശൂന്യം colors_deinit(ശൂന്യം)
{
	colors_reset(&ul_colors);
}

/*
 * Returns path to $XDG_CONFIG_HOME/terminal-colors.d
 */
സ്ഥിതം അക്ഷരം *colors_get_homedir(അക്ഷരം *buf, size_t bufsz)
{
	അക്ഷരം *p = getenv("XDG_CONFIG_HOME");

	എങ്കിൽ (p) {
		snprintf(buf, bufsz, "%s/" _PATH_TERMCOLORS_DIRNAME, p);
		തിരികെ buf;
	}

	p = getenv("HOME");
	എങ്കിൽ (p) {
		snprintf(buf, bufsz, "%s/.config/" _PATH_TERMCOLORS_DIRNAME, p);
		തിരികെ buf;
	}

	തിരികെ ശൂന്യ;
}

/*
 * Adds one color sequence to array with color scheme.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ colors_add_scheme(ഘടന ul_color_ctl *cc,
			     സ്ഥിരം അക്ഷരം *name,
			     സ്ഥിരം അക്ഷരം *seq0)
{
	ഘടന ul_color_scheme *cs = ശൂന്യ;
	അക്ഷരം *seq = ശൂന്യ;
	പൂർണ്ണസംഖ്യ rc;

	എങ്കിൽ (!cc || !name || !*name || !seq0 || !*seq0)
		തിരികെ -EINVAL;

	DBG(SCHEME, ul_debug("add '%s'", name));

	seq = color_get_sequence(seq0);
	എങ്കിൽ (!seq)
		തിരികെ -EINVAL;
	rc = -ENOMEM;

	/* enlarge the array */
	എങ്കിൽ (cc->nschemes == cc->schemes_sz) {
		ശൂന്യം *tmp = reallocarray(cc->schemes, cc->nschemes + 10,
					 വലുപ്പം(ഘടന ul_color_scheme));
		എങ്കിൽ (!tmp)
			പോകുക err;
		cc->schemes = tmp;
		cc->schemes_sz = cc->nschemes + 10;
	}

	/* add a new item */
	cs = &cc->schemes[cc->nschemes];
	cs->seq = seq;
	cs->name = strdup(name);
	എങ്കിൽ (!cs->name)
		പോകുക err;

	cc->nschemes++;
	തിരികെ 0;
err:
	എങ്കിൽ (cs) {
		സ്വതന്ത്രം(cs->seq);
		സ്വതന്ത്രം(cs->name);
		cs->seq = cs->name = ശൂന്യ;
	} അല്ലെങ്കിൽ
		സ്വതന്ത്രം(seq);
	തിരികെ rc;
}

/*
 * Deallocates all regards to color schemes
 */
സ്ഥിതം ശൂന്യം colors_free_schemes(ഘടന ul_color_ctl *cc)
{
	size_t i;

	DBG(SCHEME, ul_debug("free scheme"));

	വേണ്ടി (i = 0; i < cc->nschemes; i++) {
		സ്വതന്ത്രം(cc->schemes[i].name);
		സ്വതന്ത്രം(cc->schemes[i].seq);
	}

	സ്വതന്ത്രം(cc->schemes);
	cc->schemes = ശൂന്യ;
	cc->nschemes = 0;
	cc->schemes_sz = 0;
}

/*
 * The scheme configuration has to be sorted for bsearch
 */
സ്ഥിതം ശൂന്യം colors_sort_schemes(ഘടന ul_color_ctl *cc)
{
	എങ്കിൽ (!cc->nschemes)
		തിരികെ;

	DBG(SCHEME, ul_debug("sort scheme"));

	qsort(cc->schemes, cc->nschemes,
	      വലുപ്പം(ഘടന ul_color_scheme), cmp_scheme_name);
}

/*
 * Returns just one color scheme
 */
സ്ഥിതം ഘടന ul_color_scheme *colors_get_scheme(ഘടന ul_color_ctl *cc,
						 സ്ഥിരം അക്ഷരം *name)
{
	ഘടന ul_color_scheme താക്കോൽ = { .name = (അക്ഷരം *) name}, *res;

	എങ്കിൽ (!cc || !name || !*name)
		തിരികെ ശൂന്യ;

	എങ്കിൽ (!cc->cs_configured) {
		പൂർണ്ണസംഖ്യ rc = colors_read_schemes(cc);
		എങ്കിൽ (rc)
			തിരികെ ശൂന്യ;
	}
	എങ്കിൽ (!cc->nschemes)
		തിരികെ ശൂന്യ;

	DBG(SCHEME, ul_debug("search '%s'", name));

	res = bsearch(&താക്കോൽ, cc->schemes, cc->nschemes,
				വലുപ്പം(ഘടന ul_color_scheme),
				cmp_scheme_name);

	തിരികെ res && res->seq ? res  : ശൂന്യ;
}

/*
 * Parses filenames in terminal-colors.d
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ colors_read_configuration(ഘടന ul_color_ctl *cc)
{
	പൂർണ്ണസംഖ്യ rc = -ENOENT;
	അക്ഷരം *dirname, buf[PATH_MAX];

	cc->termname = getenv("TERM");

	dirname = colors_get_homedir(buf, വലുപ്പം(buf));
	എങ്കിൽ (dirname)
		rc = colors_readdir(cc, dirname);		/* ~/.config */
	എങ്കിൽ (rc == -EPERM || rc == -EACCES || rc == -ENOENT)
		rc = colors_readdir(cc, _PATH_TERMCOLORS_DIR);	/* /etc */

	cc->configured = 1;
	തിരികെ rc;
}

/*
 * Reads terminal-colors.d/ scheme file into array schemes
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ colors_read_schemes(ഘടന ul_color_ctl *cc)
{
	പൂർണ്ണസംഖ്യ rc = 0;
	FILE *f = ശൂന്യ;
	അക്ഷരം buf[BUFSIZ],
	     cn[129], seq[129];

	എങ്കിൽ (!cc->configured)
		rc = colors_read_configuration(cc);

	cc->cs_configured = 1;

	എങ്കിൽ (rc || !cc->sfile)
		പോകുക done;

	DBG(SCHEME, ul_debug("reading file '%s'", cc->sfile));

	f = fopen(cc->sfile, "r" UL_CLOEXECSTR);
	എങ്കിൽ (!f) {
		rc = -errno;
		പോകുക done;
	}

	ആകുമ്പോൾ (fgets(buf, വലുപ്പം(buf), f)) {
		അക്ഷരം *p = strchr(buf, '\n');

		എങ്കിൽ (!p) {
			എങ്കിൽ (feof(f))
				p = strchr(buf, '\0');
			അല്ലെങ്കിൽ {
				rc = -errno;
				പോകുക done;
			}
		}
		*p = '\0';
		p = (അക്ഷരം *) skip_blank(buf);
		എങ്കിൽ (*p == '\0' || *p == '#')
			തുടരുക;

		rc = sscanf(p, "%128[^ ] %128[^\n ]", cn, seq);
		എങ്കിൽ (rc == 2 && *cn && *seq) {
			rc = colors_add_scheme(cc, cn, seq);	/* set rc=0 on success */
			എങ്കിൽ (rc)
				പോകുക done;
		}
	}
	rc = 0;

done:
	എങ്കിൽ (f)
		fclose(f);
	colors_sort_schemes(cc);

	തിരികെ rc;
}


സ്ഥിതം ശൂന്യം termcolors_init_debug(ശൂന്യം)
{
	__UL_INIT_DEBUG_FROM_ENV(termcolors, TERMCOLORS_DEBUG_, 0, TERMINAL_COLORS_DEBUG);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ colors_terminal_is_ready(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ ncolors = -1;

#എങ്കിൽ defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
	{
		പൂർണ്ണസംഖ്യ ret;

		/* setupterm() allocates memory, del_curterm() deallocates it */
		എങ്കിൽ (setupterm(ശൂന്യ, STDOUT_FILENO, &ret) == 0 && ret == 1) {
			ncolors = tigetnum("colors");
			del_curterm(cur_term);
		}
	}
#endif
	എങ്കിൽ (1 < ncolors) {
		DBG(CONF, ul_debug("terminal is ready (supports %d colors)", ncolors));
		തിരികെ 1;
	}

	DBG(CONF, ul_debug("terminal is NOT ready (no colors)"));
	തിരികെ 0;
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
പൂർണ്ണസംഖ്യ colors_init(പൂർണ്ണസംഖ്യ mode, സ്ഥിരം അക്ഷരം *name)
{
	പൂർണ്ണസംഖ്യ ready = -1;
	ഘടന ul_color_ctl *cc = &ul_colors;

	cc->utilname = name;

	termcolors_init_debug();

	എങ്കിൽ (mode != UL_COLORMODE_ALWAYS && !isatty(STDOUT_FILENO))
		cc->mode = UL_COLORMODE_NEVER;
	അല്ലെങ്കിൽ
		cc->mode = mode;

	എങ്കിൽ (cc->mode == UL_COLORMODE_UNDEF
	    && getenv("NO_COLOR") == ശൂന്യ
	    && (ready = colors_terminal_is_ready())) {

		പൂർണ്ണസംഖ്യ rc = colors_read_configuration(cc);
		എങ്കിൽ (rc)
			cc->mode = UL_COLORMODE_DEFAULT;
		അല്ലെങ്കിൽ {

			/* evaluate scores */
			എങ്കിൽ (cc->scores[UL_COLORFILE_DISABLE] >
			    cc->scores[UL_COLORFILE_ENABLE])
				cc->mode = UL_COLORMODE_NEVER;
			അല്ലെങ്കിൽ
				cc->mode = UL_COLORMODE_DEFAULT;

			atexit(colors_deinit);
		}
	}

	തിരഞ്ഞെടുപ്പ് (cc->mode) {
	അവസ്ഥ UL_COLORMODE_AUTO:
		cc->has_colors = ready == -1 ? colors_terminal_is_ready() : ready;
		നിർത്തുക;
	അവസ്ഥ UL_COLORMODE_ALWAYS:
		cc->has_colors = 1;
		നിർത്തുക;
	അവസ്ഥ UL_COLORMODE_NEVER:
	സ്ഥിരസ്ഥിതി:
		cc->has_colors = 0;
	}

	ON_DBG(CONF, colors_debug(cc));

	തിരികെ cc->has_colors;
}

/*
 * Temporary disable colors (this setting is independent on terminal-colors.d/)
 */
ശൂന്യം colors_off(ശൂന്യം)
{
	ul_colors.disabled = 1;
}

/*
 * Enable colors
 */
ശൂന്യം colors_on(ശൂന്യം)
{
	ul_colors.disabled = 0;
}

/*
 * Is terminal-colors.d/ configured to use colors?
 */
പൂർണ്ണസംഖ്യ colors_wanted(ശൂന്യം)
{
	തിരികെ ul_colors.has_colors;
}

/*
 * Returns mode
 */
പൂർണ്ണസംഖ്യ colors_mode(ശൂന്യം)
{
	തിരികെ ul_colors.mode;
}

/*
 * Enable @seq color
 */
ശൂന്യം color_fenable(സ്ഥിരം അക്ഷരം *seq, FILE *f)
{
	എങ്കിൽ (!ul_colors.disabled && ul_colors.has_colors && seq)
		fputs(seq, f);
}

/*
 * Returns escape sequence by logical @name, if undefined then returns @dflt.
 */
സ്ഥിരം അക്ഷരം *color_scheme_get_sequence(സ്ഥിരം അക്ഷരം *name, സ്ഥിരം അക്ഷരം *dflt)
{
	ഘടന ul_color_scheme *cs;

	എങ്കിൽ (ul_colors.disabled || !ul_colors.has_colors)
		തിരികെ ശൂന്യ;

	cs = colors_get_scheme(&ul_colors, name);
	തിരികെ cs && cs->seq ? cs->seq : dflt;
}

/*
 * Enable color by logical @name, if undefined enable @dflt.
 */
ശൂന്യം color_scheme_fenable(സ്ഥിരം അക്ഷരം *name, സ്ഥിരം അക്ഷരം *dflt, FILE *f)
{
	സ്ഥിരം അക്ഷരം *seq = color_scheme_get_sequence(name, dflt);

	എങ്കിൽ (!seq)
		തിരികെ;
	color_fenable(seq, f);
}


/*
 * Disable previously enabled color
 */
ശൂന്യം color_fdisable(FILE *f)
{
	എങ്കിൽ (!ul_colors.disabled && ul_colors.has_colors)
		fputs(UL_COLOR_RESET, f);
}

/*
 * Get reset sequence
 */
സ്ഥിരം അക്ഷരം *color_get_disable_sequence(ശൂന്യം)
{
	എങ്കിൽ (!ul_colors.disabled && ul_colors.has_colors)
		തിരികെ UL_COLOR_RESET;
	അല്ലെങ്കിൽ
		തിരികെ "";
}

/*
 * Parses @str to return UL_COLORMODE_*
 */
പൂർണ്ണസംഖ്യ colormode_from_string(സ്ഥിരം അക്ഷരം *str)
{
	size_t i;
	സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം modes[] = {
		[UL_COLORMODE_AUTO]   = "auto",
		[UL_COLORMODE_NEVER]  = "never",
		[UL_COLORMODE_ALWAYS] = "always",
		[UL_COLORMODE_UNDEF] = ""
	};

	എങ്കിൽ (!str || !*str)
		തിരികെ -EINVAL;

	പ്രതിജ്ഞാ(ARRAY_SIZE(modes) == __UL_NCOLORMODES);

	വേണ്ടി (i = 0; i < ARRAY_SIZE(modes); i++) {
		എങ്കിൽ (c_strcasecmp(str, modes[i]) == 0)
			തിരികെ i;
	}

	തിരികെ -EINVAL;
}

/*
 * Parses @str and exit(EXIT_FAILURE) on error
 */
പൂർണ്ണസംഖ്യ colormode_or_err(സ്ഥിരം അക്ഷരം *str)
{
	സ്ഥിരം അക്ഷരം *p = str && *str == '=' ? str + 1 : str;
	പൂർണ്ണസംഖ്യ colormode;

	colormode = colormode_from_string(p);
	എങ്കിൽ (colormode < 0)
		errx(EXIT_FAILURE, _("unsupported color mode: %s"), p);

	തിരികെ colormode;
}

#ifdef TEST_PROGRAM_COLORS
# include <getopt.h>
പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	സ്ഥിതം സ്ഥിരം ഘടന option longopts[] = {
		{ "mode",	required_argument, ശൂന്യ, 'm' },
		{ "color",	required_argument, ശൂന്യ, 'c' },
		{ "color-scheme", required_argument, ശൂന്യ, 'C' },
		{ "name",	required_argument, ശൂന്യ, 'n' },
		{ ശൂന്യ, 0, ശൂന്യ, 0 }
	};
	പൂർണ്ണസംഖ്യ c, mode = UL_COLORMODE_UNDEF;	/* default */
	സ്ഥിരം അക്ഷരം *color = "red", *name = ശൂന്യ, *color_scheme = ശൂന്യ;
	അക്ഷരം *seq = ശൂന്യ;

	ആകുമ്പോൾ ((c = getopt_long(argc, argv, "C:c:m:n:", longopts, ശൂന്യ)) != -1) {
		തിരഞ്ഞെടുപ്പ് (c) {
		അവസ്ഥ 'c':
			color = optarg;
			നിർത്തുക;
		അവസ്ഥ 'C':
			color_scheme = optarg;
			നിർത്തുക;
		അവസ്ഥ 'm':
			mode = colormode_or_err(optarg);
			നിർത്തുക;
		അവസ്ഥ 'n':
			name = optarg;
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			fprintf(stderr, "usage: %s [options]\n"
			" -m, --mode <auto|never|always>  default is undefined\n"
			" -c, --color <red|blue|...>      color for the test message\n"
			" -C, --color-scheme <name>       color for the test message\n"
			" -n, --name <utilname>           util name\n",
			program_invocation_short_name);
			തിരികെ EXIT_FAILURE;
		}
	}

	colors_init(mode, name ? name : program_invocation_short_name);

	എങ്കിൽ (color_is_sequence(color))
		seq = strdup(color);
	അല്ലെങ്കിൽ
		seq = color_get_sequence(color);

	എങ്കിൽ (color_scheme)
		color_scheme_enable(color_scheme, seq);
	അല്ലെങ്കിൽ
		color_enable(seq);
	printf("Hello World!");
	color_disable();
	fputc('\n', stdout);

	സ്വതന്ത്രം(seq);

	തിരികെ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_COLORS */

