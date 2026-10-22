/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: 2012 Ondrej Oprala <ooprala@redhat.com>
 *          2012-2025 Karel Zak <kzak@redhat.com>
 */
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <dirent.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>

#ಯದಿ defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
# ಯದಿ defined(HAVE_NCURSESW_NCURSES_H)
#  include <ncursesw/ncurses.h>
# elif defined(HAVE_NCURSES_NCURSES_H)
#  include <ncurses/ncurses.h>
# elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
# endif
# ಯದಿ defined(HAVE_NCURSESW_TERM_H)
#  include <ncursesw/term.h>
# elif defined(HAVE_NCURSES_TERM_H)
#  include <ncurses/term.h>
# elif defined(HAVE_TERM_H)
#  include <term.h>
# endif
#endif

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "cctype.h"
#ಅನ್ತರ್ಭಾವ "colors.h"
#ಅನ್ತರ್ಭಾವ "nls.h"
#ಅನ್ತರ್ಭಾವ "pathnames.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"

#ಅನ್ತರ್ಭಾವ "debug.h"

/*
 * Default behavior, may be overridden by terminal-colors.d/{enable,disable}.
 */
#ifdef USE_COLORS_BY_DEFAULT
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_AUTO	/* check isatty() */
#ಅನ್ಯಥಾ
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_NEVER	/* no colors by default */
#endif

/*
 * terminal-colors.d debug stuff
 */
ಸ್ಥಿರ UL_DEBUG_DEFINE_MASK(termcolors);
UL_DEBUG_DEFINE_MASKNAMES(termcolors) = UL_DEBUG_EMPTY_MASKNAMES;

#define TERMCOLORS_DEBUG_INIT	(1 << 1)
#define TERMCOLORS_DEBUG_CONF	(1 << 2)
#define TERMCOLORS_DEBUG_SCHEME	(1 << 3)

#define DBG(m, x)       __UL_DBG(termcolors, TERMCOLORS_DEBUG_, m, x)
#define ON_DBG(m, x)    __UL_DBG_CALL(termcolors, TERMCOLORS_DEBUG_, m, x)

/*
 * terminal-colors.d file types
 */
ಗಣನಾ {
	UL_COLORFILE_DISABLE,		/* .disable */
	UL_COLORFILE_ENABLE,		/* .enable */
	UL_COLORFILE_SCHEME,		/* .scheme */

	__UL_COLORFILE_COUNT
};

ಸಂರಚನಾ ul_color_scheme {
	ವರ್ಣ *name;
	ವರ್ಣ *seq;
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
ಸಂರಚನಾ ul_color_ctl {
	ಧ್ರುವ ವರ್ಣ	*utilname;	/* util name */
	ಧ್ರುವ ವರ್ಣ	*termname;	/* terminal name ($TERM) */

	ವರ್ಣ		*sfile;	/* path to scheme */

	ಸಂರಚನಾ ul_color_scheme	*schemes;	/* array with color schemes */
	size_t			nschemes;	/* number of the items */
	size_t			schemes_sz;	/* number of the allocated items */

	ಪೂರ್ಣಾಙ್ಕ		mode;		/* UL_COLORMODE_* */
	unsigned ಪೂರ್ಣಾಙ್ಕ	has_colors : 1,	/* based on mode and scores[] */
			disabled   : 1, /* disable colors */
			cs_configured : 1, /* color schemes read */
			configured : 1; /* terminal-colors.d parsed */

	ಪೂರ್ಣಾಙ್ಕ		scores[__UL_COLORFILE_COUNT];	/* the best match */
};

/*
 * Control struct, globally shared.
 */
ಸ್ಥಿರ ಸಂರಚನಾ ul_color_ctl ul_colors;

ಸ್ಥಿರ ಶೂನ್ಯ colors_free_schemes(ಸಂರಚನಾ ul_color_ctl *cc);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ colors_read_schemes(ಸಂರಚನಾ ul_color_ctl *cc);

/*
 * qsort/bsearch buddy
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cmp_scheme_name(ಧ್ರುವ ಶೂನ್ಯ *a0, ಧ್ರುವ ಶೂನ್ಯ *b0)
{
	ಧ್ರುವ ಸಂರಚನಾ ul_color_scheme *a = (ಧ್ರುವ ಸಂರಚನಾ ul_color_scheme *) a0,
				     *b = (ಧ್ರುವ ಸಂರಚನಾ ul_color_scheme *) b0;
	ಪ್ರತ್ಯಾಗಮ strcmp(a->name, b->name);
}

/*
 * Resets control struct (note that we don't allocate the struct)
 */
ಸ್ಥಿರ ಶೂನ್ಯ colors_reset(ಸಂರಚನಾ ul_color_ctl *cc)
{
	ಯದಿ (!cc)
		ಪ್ರತ್ಯಾಗಮ;

	colors_free_schemes(cc);

	ಮುಕ್ತ(cc->sfile);

	cc->sfile = ರಿಕ್ತ;
	cc->utilname = ರಿಕ್ತ;
	cc->termname = ರಿಕ್ತ;
	cc->mode = UL_COLORMODE_UNDEF;

	memset(cc->scores, 0, ಪರಿಮಾಣ(cc->scores));
}

ಸ್ಥಿರ ಶೂನ್ಯ colors_debug(ಸಂರಚನಾ ul_color_ctl *cc)
{
	size_t i;

	ಯದಿ (!cc)
		ಪ್ರತ್ಯಾಗಮ;

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

	ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(cc->scores); i++)
		printf("\tscore %s = %d\n",
				i == UL_COLORFILE_DISABLE ? "disable" :
				i == UL_COLORFILE_ENABLE ? "enable" :
				i == UL_COLORFILE_SCHEME ? "scheme" : "???",
				cc->scores[i]);

	fputc('\n', stdout);

	ಪ್ರತಿ (i = 0; i < cc->nschemes; i++) {
		printf("\tscheme #%02zu ", i);
		color_scheme_enable(cc->schemes[i].name, ರಿಕ್ತ);
		fputs(cc->schemes[i].name, stdout);
		color_disable();
		fputc('\n', stdout);
	}
	fputc('\n', stdout);
}

/*
 * Parses [[<utilname>][@<termname>].]<type>
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ filename_to_tokens(ಧ್ರುವ ವರ್ಣ *str,
			      ಧ್ರುವ ವರ್ಣ **name, size_t *namesz,
			      ಧ್ರುವ ವರ್ಣ **term, size_t *termsz,
			      ಪೂರ್ಣಾಙ್ಕ  *filetype)
{
	ಧ್ರುವ ವರ್ಣ *type_start, *term_start, *p;

	ಯದಿ (!str || !*str || *str == '.' || strlen(str) > PATH_MAX)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	/* parse .type */
	p = strrchr(str, '.');
	type_start = p ? p + 1 : str;

	ಯದಿ (strcmp(type_start, "disable") == 0)
		*filetype = UL_COLORFILE_DISABLE;
	ಅನ್ಯಥಾ ಯದಿ (strcmp(type_start, "enable") == 0)
		*filetype = UL_COLORFILE_ENABLE;
	ಅನ್ಯಥಾ ಯದಿ (strcmp(type_start, "scheme") == 0)
		*filetype = UL_COLORFILE_SCHEME;
	ಅನ್ಯಥಾ {
		DBG(CONF, ul_debug("unknown type '%s'", type_start));
		ಪ್ರತ್ಯಾಗಮ 1;				/* unknown type */
	}

	ಯದಿ (type_start == str)
		ಪ್ರತ್ಯಾಗಮ 0;				/* "type" only */

	/* parse @termname */
	p = strchr(str, '@');
	term_start = p ? p + 1 : ರಿಕ್ತ;
	ಯದಿ (term_start) {
		*term = term_start;
		*termsz = type_start - term_start - 1;
		ಯದಿ (term_start - 1 == str)
			ಪ್ರತ್ಯಾಗಮ 0;			/* "@termname.type" */
	}

	/* parse utilname */
	p = term_start ? term_start : type_start;
	*name = str;
	*namesz = p - str - 1;

	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * Scans @dirname and select the best matches for UL_COLORFILE_* types.
 * The result is stored to cc->scores. The path to the best "scheme"
 * file is stored to cc->scheme.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ colors_readdir(ಸಂರಚನಾ ul_color_ctl *cc, ಧ್ರುವ ವರ್ಣ *dirname)
{
	DIR *dir;
	ಪೂರ್ಣಾಙ್ಕ rc = -ENOENT;
	ಸಂರಚನಾ dirent *d;
	ವರ್ಣ sfile[PATH_MAX] = { '\0' };
	size_t namesz, termsz;

	ಯದಿ (!dirname || !cc || !cc->utilname || !*cc->utilname)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	DBG(CONF, ul_debug("reading dir: '%s'", dirname));

	dir = opendir(dirname);
	ಯದಿ (!dir)
		ಪ್ರತ್ಯಾಗಮ -errno;

	namesz = strlen(cc->utilname);
	termsz = cc->termname ? strlen(cc->termname) : 0;

	ಯಾವತ್ ((d = readdir(dir))) {
		ಪೂರ್ಣಾಙ್ಕ type, score = 1;
		ಧ್ರುವ ವರ್ಣ *tk_name = ರಿಕ್ತ, *tk_term = ರಿಕ್ತ;
		size_t tk_namesz = 0, tk_termsz = 0;

		ಯದಿ (*d->d_name == '.')
			ಅನುವರ್ತ;
#ifdef _DIRENT_HAVE_D_TYPE
		ಯದಿ (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK &&
		    d->d_type != DT_REG)
			ಅನುವರ್ತ;
#endif
		ಯದಿ (filename_to_tokens(d->d_name,
				       &tk_name, &tk_namesz,
				       &tk_term, &tk_termsz, &type) != 0)
			ಅನುವರ್ತ;

		/* count theoretical score before we check names to avoid
		 * unnecessary strcmp() */
		ಯದಿ (tk_name)
			score += 20;
		ಯದಿ (tk_term)
			score += 10;

		DBG(CONF, ul_debug("item '%s': score=%d "
			"[cur: %d, name(%zu): %s, term(%zu): %s]",
			d->d_name, score, cc->scores[type],
			tk_namesz, tk_name,
			tk_termsz, tk_term));


		ಯದಿ (score < cc->scores[type])
			ಅನುವರ್ತ;

		/* filter out by names */
		ಯದಿ (tk_namesz && (tk_namesz != namesz ||
				 strncmp(tk_name, cc->utilname, namesz) != 0))
			ಅನುವರ್ತ;

		ಯದಿ (tk_termsz && (termsz == 0 || tk_termsz != termsz ||
				  strncmp(tk_term, cc->termname, termsz) != 0))
			ಅನುವರ್ತ;

		DBG(CONF, ul_debug("setting '%s' from %d -to-> %d",
					type == UL_COLORFILE_SCHEME ? "scheme" :
					type == UL_COLORFILE_DISABLE ? "disable" :
					type == UL_COLORFILE_ENABLE ? "enable" : "???",
					cc->scores[type], score));
		cc->scores[type] = score;
		rc = 0;
		ಯದಿ (type == UL_COLORFILE_SCHEME)
			strncpy(sfile, d->d_name, ಪರಿಮಾಣ(sfile));
	}

	ಯದಿ (*sfile) {
		sfile[ಪರಿಮಾಣ(sfile) - 1] = '\0';
		ಯದಿ (asprintf(&cc->sfile, "%s/%s", dirname, sfile) <= 0)
			rc = -ENOMEM;
	}

	closedir(dir);
	ಪ್ರತ್ಯಾಗಮ rc;
}

/* atexit() wrapper */
ಸ್ಥಿರ ಶೂನ್ಯ colors_deinit(ಶೂನ್ಯ)
{
	colors_reset(&ul_colors);
}

/*
 * Returns path to $XDG_CONFIG_HOME/terminal-colors.d
 */
ಸ್ಥಿರ ವರ್ಣ *colors_get_homedir(ವರ್ಣ *buf, size_t bufsz)
{
	ವರ್ಣ *p = getenv("XDG_CONFIG_HOME");

	ಯದಿ (p) {
		snprintf(buf, bufsz, "%s/" _PATH_TERMCOLORS_DIRNAME, p);
		ಪ್ರತ್ಯಾಗಮ buf;
	}

	p = getenv("HOME");
	ಯದಿ (p) {
		snprintf(buf, bufsz, "%s/.config/" _PATH_TERMCOLORS_DIRNAME, p);
		ಪ್ರತ್ಯಾಗಮ buf;
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/*
 * Adds one color sequence to array with color scheme.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ colors_add_scheme(ಸಂರಚನಾ ul_color_ctl *cc,
			     ಧ್ರುವ ವರ್ಣ *name,
			     ಧ್ರುವ ವರ್ಣ *seq0)
{
	ಸಂರಚನಾ ul_color_scheme *cs = ರಿಕ್ತ;
	ವರ್ಣ *seq = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ rc;

	ಯದಿ (!cc || !name || !*name || !seq0 || !*seq0)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	DBG(SCHEME, ul_debug("add '%s'", name));

	seq = color_get_sequence(seq0);
	ಯದಿ (!seq)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	rc = -ENOMEM;

	/* enlarge the array */
	ಯದಿ (cc->nschemes == cc->schemes_sz) {
		ಶೂನ್ಯ *tmp = reallocarray(cc->schemes, cc->nschemes + 10,
					 ಪರಿಮಾಣ(ಸಂರಚನಾ ul_color_scheme));
		ಯದಿ (!tmp)
			ಗಚ್ಛ err;
		cc->schemes = tmp;
		cc->schemes_sz = cc->nschemes + 10;
	}

	/* add a new item */
	cs = &cc->schemes[cc->nschemes];
	cs->seq = seq;
	cs->name = strdup(name);
	ಯದಿ (!cs->name)
		ಗಚ್ಛ err;

	cc->nschemes++;
	ಪ್ರತ್ಯಾಗಮ 0;
err:
	ಯದಿ (cs) {
		ಮುಕ್ತ(cs->seq);
		ಮುಕ್ತ(cs->name);
		cs->seq = cs->name = ರಿಕ್ತ;
	} ಅನ್ಯಥಾ
		ಮುಕ್ತ(seq);
	ಪ್ರತ್ಯಾಗಮ rc;
}

/*
 * Deallocates all regards to color schemes
 */
ಸ್ಥಿರ ಶೂನ್ಯ colors_free_schemes(ಸಂರಚನಾ ul_color_ctl *cc)
{
	size_t i;

	DBG(SCHEME, ul_debug("free scheme"));

	ಪ್ರತಿ (i = 0; i < cc->nschemes; i++) {
		ಮುಕ್ತ(cc->schemes[i].name);
		ಮುಕ್ತ(cc->schemes[i].seq);
	}

	ಮುಕ್ತ(cc->schemes);
	cc->schemes = ರಿಕ್ತ;
	cc->nschemes = 0;
	cc->schemes_sz = 0;
}

/*
 * The scheme configuration has to be sorted for bsearch
 */
ಸ್ಥಿರ ಶೂನ್ಯ colors_sort_schemes(ಸಂರಚನಾ ul_color_ctl *cc)
{
	ಯದಿ (!cc->nschemes)
		ಪ್ರತ್ಯಾಗಮ;

	DBG(SCHEME, ul_debug("sort scheme"));

	qsort(cc->schemes, cc->nschemes,
	      ಪರಿಮಾಣ(ಸಂರಚನಾ ul_color_scheme), cmp_scheme_name);
}

/*
 * Returns just one color scheme
 */
ಸ್ಥಿರ ಸಂರಚನಾ ul_color_scheme *colors_get_scheme(ಸಂರಚನಾ ul_color_ctl *cc,
						 ಧ್ರುವ ವರ್ಣ *name)
{
	ಸಂರಚನಾ ul_color_scheme ಕೀಲಿ = { .name = (ವರ್ಣ *) name}, *res;

	ಯದಿ (!cc || !name || !*name)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (!cc->cs_configured) {
		ಪೂರ್ಣಾಙ್ಕ rc = colors_read_schemes(cc);
		ಯದಿ (rc)
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}
	ಯದಿ (!cc->nschemes)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	DBG(SCHEME, ul_debug("search '%s'", name));

	res = bsearch(&ಕೀಲಿ, cc->schemes, cc->nschemes,
				ಪರಿಮಾಣ(ಸಂರಚನಾ ul_color_scheme),
				cmp_scheme_name);

	ಪ್ರತ್ಯಾಗಮ res && res->seq ? res  : ರಿಕ್ತ;
}

/*
 * Parses filenames in terminal-colors.d
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ colors_read_configuration(ಸಂರಚನಾ ul_color_ctl *cc)
{
	ಪೂರ್ಣಾಙ್ಕ rc = -ENOENT;
	ವರ್ಣ *dirname, buf[PATH_MAX];

	cc->termname = getenv("TERM");

	dirname = colors_get_homedir(buf, ಪರಿಮಾಣ(buf));
	ಯದಿ (dirname)
		rc = colors_readdir(cc, dirname);		/* ~/.config */
	ಯದಿ (rc == -EPERM || rc == -EACCES || rc == -ENOENT)
		rc = colors_readdir(cc, _PATH_TERMCOLORS_DIR);	/* /etc */

	cc->configured = 1;
	ಪ್ರತ್ಯಾಗಮ rc;
}

/*
 * Reads terminal-colors.d/ scheme file into array schemes
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ colors_read_schemes(ಸಂರಚನಾ ul_color_ctl *cc)
{
	ಪೂರ್ಣಾಙ್ಕ rc = 0;
	FILE *f = ರಿಕ್ತ;
	ವರ್ಣ buf[BUFSIZ],
	     cn[129], seq[129];

	ಯದಿ (!cc->configured)
		rc = colors_read_configuration(cc);

	cc->cs_configured = 1;

	ಯದಿ (rc || !cc->sfile)
		ಗಚ್ಛ done;

	DBG(SCHEME, ul_debug("reading file '%s'", cc->sfile));

	f = fopen(cc->sfile, "r" UL_CLOEXECSTR);
	ಯದಿ (!f) {
		rc = -errno;
		ಗಚ್ಛ done;
	}

	ಯಾವತ್ (fgets(buf, ಪರಿಮಾಣ(buf), f)) {
		ವರ್ಣ *p = strchr(buf, '\n');

		ಯದಿ (!p) {
			ಯದಿ (feof(f))
				p = strchr(buf, '\0');
			ಅನ್ಯಥಾ {
				rc = -errno;
				ಗಚ್ಛ done;
			}
		}
		*p = '\0';
		p = (ವರ್ಣ *) skip_blank(buf);
		ಯದಿ (*p == '\0' || *p == '#')
			ಅನುವರ್ತ;

		rc = sscanf(p, "%128[^ ] %128[^\n ]", cn, seq);
		ಯದಿ (rc == 2 && *cn && *seq) {
			rc = colors_add_scheme(cc, cn, seq);	/* set rc=0 on success */
			ಯದಿ (rc)
				ಗಚ್ಛ done;
		}
	}
	rc = 0;

done:
	ಯದಿ (f)
		fclose(f);
	colors_sort_schemes(cc);

	ಪ್ರತ್ಯಾಗಮ rc;
}


ಸ್ಥಿರ ಶೂನ್ಯ termcolors_init_debug(ಶೂನ್ಯ)
{
	__UL_INIT_DEBUG_FROM_ENV(termcolors, TERMCOLORS_DEBUG_, 0, TERMINAL_COLORS_DEBUG);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ colors_terminal_is_ready(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ ncolors = -1;

#ಯದಿ defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
	{
		ಪೂರ್ಣಾಙ್ಕ ret;

		/* setupterm() allocates memory, del_curterm() deallocates it */
		ಯದಿ (setupterm(ರಿಕ್ತ, STDOUT_FILENO, &ret) == 0 && ret == 1) {
			ncolors = tigetnum("colors");
			del_curterm(cur_term);
		}
	}
#endif
	ಯದಿ (1 < ncolors) {
		DBG(CONF, ul_debug("terminal is ready (supports %d colors)", ncolors));
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	DBG(CONF, ul_debug("terminal is NOT ready (no colors)"));
	ಪ್ರತ್ಯಾಗಮ 0;
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
ಪೂರ್ಣಾಙ್ಕ colors_init(ಪೂರ್ಣಾಙ್ಕ mode, ಧ್ರುವ ವರ್ಣ *name)
{
	ಪೂರ್ಣಾಙ್ಕ ready = -1;
	ಸಂರಚನಾ ul_color_ctl *cc = &ul_colors;

	cc->utilname = name;

	termcolors_init_debug();

	ಯದಿ (mode != UL_COLORMODE_ALWAYS && !isatty(STDOUT_FILENO))
		cc->mode = UL_COLORMODE_NEVER;
	ಅನ್ಯಥಾ
		cc->mode = mode;

	ಯದಿ (cc->mode == UL_COLORMODE_UNDEF
	    && getenv("NO_COLOR") == ರಿಕ್ತ
	    && (ready = colors_terminal_is_ready())) {

		ಪೂರ್ಣಾಙ್ಕ rc = colors_read_configuration(cc);
		ಯದಿ (rc)
			cc->mode = UL_COLORMODE_DEFAULT;
		ಅನ್ಯಥಾ {

			/* evaluate scores */
			ಯದಿ (cc->scores[UL_COLORFILE_DISABLE] >
			    cc->scores[UL_COLORFILE_ENABLE])
				cc->mode = UL_COLORMODE_NEVER;
			ಅನ್ಯಥಾ
				cc->mode = UL_COLORMODE_DEFAULT;

			atexit(colors_deinit);
		}
	}

	ವಿಕಲ್ಪ (cc->mode) {
	ಪಕ್ಷ UL_COLORMODE_AUTO:
		cc->has_colors = ready == -1 ? colors_terminal_is_ready() : ready;
		ಭಙ್ಗ;
	ಪಕ್ಷ UL_COLORMODE_ALWAYS:
		cc->has_colors = 1;
		ಭಙ್ಗ;
	ಪಕ್ಷ UL_COLORMODE_NEVER:
	ಸ್ವಭಾವ:
		cc->has_colors = 0;
	}

	ON_DBG(CONF, colors_debug(cc));

	ಪ್ರತ್ಯಾಗಮ cc->has_colors;
}

/*
 * Temporary disable colors (this setting is independent on terminal-colors.d/)
 */
ಶೂನ್ಯ colors_off(ಶೂನ್ಯ)
{
	ul_colors.disabled = 1;
}

/*
 * Enable colors
 */
ಶೂನ್ಯ colors_on(ಶೂನ್ಯ)
{
	ul_colors.disabled = 0;
}

/*
 * Is terminal-colors.d/ configured to use colors?
 */
ಪೂರ್ಣಾಙ್ಕ colors_wanted(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ ul_colors.has_colors;
}

/*
 * Returns mode
 */
ಪೂರ್ಣಾಙ್ಕ colors_mode(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ ul_colors.mode;
}

/*
 * Enable @seq color
 */
ಶೂನ್ಯ color_fenable(ಧ್ರುವ ವರ್ಣ *seq, FILE *f)
{
	ಯದಿ (!ul_colors.disabled && ul_colors.has_colors && seq)
		fputs(seq, f);
}

/*
 * Returns escape sequence by logical @name, if undefined then returns @dflt.
 */
ಧ್ರುವ ವರ್ಣ *color_scheme_get_sequence(ಧ್ರುವ ವರ್ಣ *name, ಧ್ರುವ ವರ್ಣ *dflt)
{
	ಸಂರಚನಾ ul_color_scheme *cs;

	ಯದಿ (ul_colors.disabled || !ul_colors.has_colors)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	cs = colors_get_scheme(&ul_colors, name);
	ಪ್ರತ್ಯಾಗಮ cs && cs->seq ? cs->seq : dflt;
}

/*
 * Enable color by logical @name, if undefined enable @dflt.
 */
ಶೂನ್ಯ color_scheme_fenable(ಧ್ರುವ ವರ್ಣ *name, ಧ್ರುವ ವರ್ಣ *dflt, FILE *f)
{
	ಧ್ರುವ ವರ್ಣ *seq = color_scheme_get_sequence(name, dflt);

	ಯದಿ (!seq)
		ಪ್ರತ್ಯಾಗಮ;
	color_fenable(seq, f);
}


/*
 * Disable previously enabled color
 */
ಶೂನ್ಯ color_fdisable(FILE *f)
{
	ಯದಿ (!ul_colors.disabled && ul_colors.has_colors)
		fputs(UL_COLOR_RESET, f);
}

/*
 * Get reset sequence
 */
ಧ್ರುವ ವರ್ಣ *color_get_disable_sequence(ಶೂನ್ಯ)
{
	ಯದಿ (!ul_colors.disabled && ul_colors.has_colors)
		ಪ್ರತ್ಯಾಗಮ UL_COLOR_RESET;
	ಅನ್ಯಥಾ
		ಪ್ರತ್ಯಾಗಮ "";
}

/*
 * Parses @str to return UL_COLORMODE_*
 */
ಪೂರ್ಣಾಙ್ಕ colormode_from_string(ಧ್ರುವ ವರ್ಣ *str)
{
	size_t i;
	ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ modes[] = {
		[UL_COLORMODE_AUTO]   = "auto",
		[UL_COLORMODE_NEVER]  = "never",
		[UL_COLORMODE_ALWAYS] = "always",
		[UL_COLORMODE_UNDEF] = ""
	};

	ಯದಿ (!str || !*str)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	ಪ್ರತಿಜ್ಞಾ(ARRAY_SIZE(modes) == __UL_NCOLORMODES);

	ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(modes); i++) {
		ಯದಿ (c_strcasecmp(str, modes[i]) == 0)
			ಪ್ರತ್ಯಾಗಮ i;
	}

	ಪ್ರತ್ಯಾಗಮ -EINVAL;
}

/*
 * Parses @str and exit(EXIT_FAILURE) on error
 */
ಪೂರ್ಣಾಙ್ಕ colormode_or_err(ಧ್ರುವ ವರ್ಣ *str)
{
	ಧ್ರುವ ವರ್ಣ *p = str && *str == '=' ? str + 1 : str;
	ಪೂರ್ಣಾಙ್ಕ colormode;

	colormode = colormode_from_string(p);
	ಯದಿ (colormode < 0)
		errx(EXIT_FAILURE, _("unsupported color mode: %s"), p);

	ಪ್ರತ್ಯಾಗಮ colormode;
}

#ifdef TEST_PROGRAM_COLORS
# include <getopt.h>
ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ option longopts[] = {
		{ "mode",	required_argument, ರಿಕ್ತ, 'm' },
		{ "color",	required_argument, ರಿಕ್ತ, 'c' },
		{ "color-scheme", required_argument, ರಿಕ್ತ, 'C' },
		{ "name",	required_argument, ರಿಕ್ತ, 'n' },
		{ ರಿಕ್ತ, 0, ರಿಕ್ತ, 0 }
	};
	ಪೂರ್ಣಾಙ್ಕ c, mode = UL_COLORMODE_UNDEF;	/* default */
	ಧ್ರುವ ವರ್ಣ *color = "red", *name = ರಿಕ್ತ, *color_scheme = ರಿಕ್ತ;
	ವರ್ಣ *seq = ರಿಕ್ತ;

	ಯಾವತ್ ((c = getopt_long(argc, argv, "C:c:m:n:", longopts, ರಿಕ್ತ)) != -1) {
		ವಿಕಲ್ಪ (c) {
		ಪಕ್ಷ 'c':
			color = optarg;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'C':
			color_scheme = optarg;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'm':
			mode = colormode_or_err(optarg);
			ಭಙ್ಗ;
		ಪಕ್ಷ 'n':
			name = optarg;
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			fprintf(stderr, "usage: %s [options]\n"
			" -m, --mode <auto|never|always>  default is undefined\n"
			" -c, --color <red|blue|...>      color for the test message\n"
			" -C, --color-scheme <name>       color for the test message\n"
			" -n, --name <utilname>           util name\n",
			program_invocation_short_name);
			ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
		}
	}

	colors_init(mode, name ? name : program_invocation_short_name);

	ಯದಿ (color_is_sequence(color))
		seq = strdup(color);
	ಅನ್ಯಥಾ
		seq = color_get_sequence(color);

	ಯದಿ (color_scheme)
		color_scheme_enable(color_scheme, seq);
	ಅನ್ಯಥಾ
		color_enable(seq);
	printf("Hello World!");
	color_disable();
	fputc('\n', stdout);

	ಮುಕ್ತ(seq);

	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_COLORS */

