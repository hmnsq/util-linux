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

#ಆದರೆ defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
# ಆದರೆ defined(HAVE_NCURSESW_NCURSES_H)
#  include <ncursesw/ncurses.h>
# elif defined(HAVE_NCURSES_NCURSES_H)
#  include <ncurses/ncurses.h>
# elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
# endif
# ಆದರೆ defined(HAVE_NCURSESW_ಪದ_H)
#  include <ncursesw/term.h>
# elif defined(HAVE_NCURSES_ಪದ_H)
#  include <ncurses/term.h>
# elif defined(HAVE_ಪದ_H)
#  include <term.h>
# endif
#ಆದರೆ_ಅಂತ್ಯ

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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ USE_COLORS_BY_DEFAULT
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_AUTO	/* check isatty() */
#ಇಲ್ಲದಿದ್ದರೆ
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_NEVER	/* no colors by default */
#ಆದರೆ_ಅಂತ್ಯ

/*
 * terminal-colors.d debug stuff
 */
ಸ್ಥಿರ UL_DEBUG_DEFINE_ಮುಖವಾಡ(termcolors);
UL_DEBUG_DEFINE_MASKNAMES(termcolors) = UL_DEBUG_ಖಾಲಿಗಳು_MASKNAMES;

#ವ್ಯಾಖ್ಯಾನಿಸು TERMCOLORS_DEBUG_INIT	(1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು TERMCOLORS_DEBUG_CONF	(1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು TERMCOLORS_DEBUG_SCHEME	(1 << 3)

#ವ್ಯಾಖ್ಯಾನಿಸು DBG(m, x)       __UL_DBG(termcolors, TERMCOLORS_DEBUG_, m, x)
#ವ್ಯಾಖ್ಯಾನಿಸು ON_DBG(m, x)    __UL_DBG_CALL(termcolors, TERMCOLORS_DEBUG_, m, x)

/*
 * terminal-colors.d file types
 */
ಎಣಿಕೆ {
	UL_COLORFILE_DISABLE,		/* .disable */
	UL_COLORFILE_ENABLE,		/* .enable */
	UL_COLORFILE_SCHEME,		/* .scheme */

	__UL_COLORFILE_COUNT
};

ರಚನೆ ul_color_scheme {
	ಅಕ್ಷರ *name;
	ಅಕ್ಷರ *seq;
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
ರಚನೆ ul_color_ctl {
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ	*utilname;	/* util name */
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ	*termname;	/* terminal name ($TERM) */

	ಅಕ್ಷರ		*sfile;	/* path to scheme */

	ರಚನೆ ul_color_scheme	*schemes;	/* array with color schemes */
	size_t			nschemes;	/* number of the items */
	size_t			schemes_sz;	/* number of the allocated items */

	ಪೂರ್ಣಾಂಕ		ಬಹುಲಕ;		/* UL_COLORMODE_* */
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ	has_colors : 1,	/* based on mode and scores[] */
			disabled   : 1, /* disable colors */
			cs_configured : 1, /* color schemes read */
			configured : 1; /* terminal-colors.d parsed */

	ಪೂರ್ಣಾಂಕ		scores[__UL_COLORFILE_COUNT];	/* the best match */
};

/*
 * Control struct, globally shared.
 */
ಸ್ಥಿರ ರಚನೆ ul_color_ctl ul_colors;

ಸ್ಥಿರ ಶೂನ್ಯ colors_free_schemes(ರಚನೆ ul_color_ctl *cc);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ colors_read_schemes(ರಚನೆ ul_color_ctl *cc);

/*
 * qsort/bsearch buddy
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cmp_scheme_name(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *a0, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *b0)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ ul_color_scheme *a = (ಸ್ಥಿರಾಂಕ ರಚನೆ ul_color_scheme *) a0,
				     *b = (ಸ್ಥಿರಾಂಕ ರಚನೆ ul_color_scheme *) b0;
	ಹಿಂತಿರುಗಿ strcmp(a->name, b->name);
}

/*
 * Resets control struct (note that we don't allocate the struct)
 */
ಸ್ಥಿರ ಶೂನ್ಯ colors_reset(ರಚನೆ ul_color_ctl *cc)
{
	ಆದರೆ (!cc)
		ಹಿಂತಿರುಗಿ;

	colors_free_schemes(cc);

	ಬಿಡುಗಡೆ(cc->sfile);

	cc->sfile = ಖಾಲಿ;
	cc->utilname = ಖಾಲಿ;
	cc->termname = ಖಾಲಿ;
	cc->ಬಹುಲಕ = UL_COLORMODE_UNDEF;

	memset(cc->scores, 0, ಗಾತ್ರ(cc->scores));
}

ಸ್ಥಿರ ಶೂನ್ಯ colors_debug(ರಚನೆ ul_color_ctl *cc)
{
	size_t i;

	ಆದರೆ (!cc)
		ಹಿಂತಿರುಗಿ;

	ಬರೆ("Colors:\n");
	ಬರೆ("\tutilname = '%s'\n", cc->utilname);
	ಬರೆ("\ttermname = '%s'\n", cc->termname);
	ಬರೆ("\tscheme file = '%s'\n", cc->sfile);
	ಬರೆ("\tmode = %s\n",
			cc->ಬಹುಲಕ == UL_COLORMODE_UNDEF ? "undefined" :
			cc->ಬಹುಲಕ == UL_COLORMODE_AUTO ?  "auto" :
			cc->ಬಹುಲಕ == UL_COLORMODE_NEVER ? "never" :
			cc->ಬಹುಲಕ == UL_COLORMODE_ALWAYS ? "always" : "???");
	ಬರೆ("\thas_colors = %d\n", cc->has_colors);
	ಬರೆ("\tdisabled = %d\n", cc->disabled);
	ಬರೆ("\tconfigured = %d\n", cc->configured);
	ಬರೆ("\tcs configured = %d\n", cc->cs_configured);

	fputc('\n', stdout);

	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(cc->scores); i++)
		ಬರೆ("\tscore %s = %d\n",
				i == UL_COLORFILE_DISABLE ? "disable" :
				i == UL_COLORFILE_ENABLE ? "enable" :
				i == UL_COLORFILE_SCHEME ? "scheme" : "???",
				cc->scores[i]);

	fputc('\n', stdout);

	ಗಾಗಿ (i = 0; i < cc->nschemes; i++) {
		ಬರೆ("\tscheme #%02zu ", i);
		color_scheme_enable(cc->schemes[i].name, ಖಾಲಿ);
		fputs(cc->schemes[i].name, stdout);
		color_disable();
		fputc('\n', stdout);
	}
	fputc('\n', stdout);
}

/*
 * Parses [[<utilname>][@<termname>].]<type>
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ filename_ಗಮ್ಯ_ಟೋಕನುಗಳು(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str,
			      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **name, size_t *namesz,
			      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **ಪದ, size_t *termsz,
			      ಪೂರ್ಣಾಂಕ  *filetype)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *type_ಆರಂಭ, *ಪದ_ಆರಂಭ, *p;

	ಆದರೆ (!str || !*str || *str == '.' || ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str) > PATH_MAX)
		ಹಿಂತಿರುಗಿ -EINVAL;

	/* parse .type */
	p = strrchr(str, '.');
	type_ಆರಂಭ = p ? p + 1 : str;

	ಆದರೆ (strcmp(type_ಆರಂಭ, "disable") == 0)
		*filetype = UL_COLORFILE_DISABLE;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(type_ಆರಂಭ, "enable") == 0)
		*filetype = UL_COLORFILE_ENABLE;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(type_ಆರಂಭ, "scheme") == 0)
		*filetype = UL_COLORFILE_SCHEME;
	ಇಲ್ಲದಿದ್ದರೆ {
		DBG(CONF, ul_debug("unknown type '%s'", type_ಆರಂಭ));
		ಹಿಂತಿರುಗಿ 1;				/* unknown type */
	}

	ಆದರೆ (type_ಆರಂಭ == str)
		ಹಿಂತಿರುಗಿ 0;				/* "type" only */

	/* parse @termname */
	p = strchr(str, '@');
	ಪದ_ಆರಂಭ = p ? p + 1 : ಖಾಲಿ;
	ಆದರೆ (ಪದ_ಆರಂಭ) {
		*ಪದ = ಪದ_ಆರಂಭ;
		*termsz = type_ಆರಂಭ - ಪದ_ಆರಂಭ - 1;
		ಆದರೆ (ಪದ_ಆರಂಭ - 1 == str)
			ಹಿಂತಿರುಗಿ 0;			/* "@termname.type" */
	}

	/* parse utilname */
	p = ಪದ_ಆರಂಭ ? ಪದ_ಆರಂಭ : type_ಆರಂಭ;
	*name = str;
	*namesz = p - str - 1;

	ಹಿಂತಿರುಗಿ 0;
}

/*
 * Scans @dirname and select the best matches for UL_COLORFILE_* types.
 * The result is stored to cc->scores. The path to the best "scheme"
 * file is stored to cc->scheme.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ colors_readdir(ರಚನೆ ul_color_ctl *cc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dirname)
{
	DIR *dir;
	ಪೂರ್ಣಾಂಕ rc = -ENOENT;
	ರಚನೆ dirent *d;
	ಅಕ್ಷರ sfile[PATH_MAX] = { '\0' };
	size_t namesz, termsz;

	ಆದರೆ (!dirname || !cc || !cc->utilname || !*cc->utilname)
		ಹಿಂತಿರುಗಿ -EINVAL;

	DBG(CONF, ul_debug("reading dir: '%s'", dirname));

	dir = opendir(dirname);
	ಆದರೆ (!dir)
		ಹಿಂತಿರುಗಿ -errno;

	namesz = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(cc->utilname);
	termsz = cc->termname ? ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(cc->termname) : 0;

	ಆಗ ((d = readdir(dir))) {
		ಪೂರ್ಣಾಂಕ type, score = 1;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tk_name = ಖಾಲಿ, *tk_ಪದ = ಖಾಲಿ;
		size_t tk_namesz = 0, tk_termsz = 0;

		ಆದರೆ (*d->d_name == '.')
			ಮುಂದುವರಿ;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _DIRENT_HAVE_D_TYPE
		ಆದರೆ (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK &&
		    d->d_type != DT_REG)
			ಮುಂದುವರಿ;
#ಆದರೆ_ಅಂತ್ಯ
		ಆದರೆ (filename_ಗಮ್ಯ_ಟೋಕನುಗಳು(d->d_name,
				       &tk_name, &tk_namesz,
				       &tk_ಪದ, &tk_termsz, &type) != 0)
			ಮುಂದುವರಿ;

		/* count theoretical score before we check names to avoid
		 * unnecessary strcmp() */
		ಆದರೆ (tk_name)
			score += 20;
		ಆದರೆ (tk_ಪದ)
			score += 10;

		DBG(CONF, ul_debug("item '%s': score=%d "
			"[cur: %d, name(%zu): %s, term(%zu): %s]",
			d->d_name, score, cc->scores[type],
			tk_namesz, tk_name,
			tk_termsz, tk_ಪದ));


		ಆದರೆ (score < cc->scores[type])
			ಮುಂದುವರಿ;

		/* filter out by names */
		ಆದರೆ (tk_namesz && (tk_namesz != namesz ||
				 ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(tk_name, cc->utilname, namesz) != 0))
			ಮುಂದುವರಿ;

		ಆದರೆ (tk_termsz && (termsz == 0 || tk_termsz != termsz ||
				  ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(tk_ಪದ, cc->termname, termsz) != 0))
			ಮುಂದುವರಿ;

		DBG(CONF, ul_debug("setting '%s' from %d -to-> %d",
					type == UL_COLORFILE_SCHEME ? "scheme" :
					type == UL_COLORFILE_DISABLE ? "disable" :
					type == UL_COLORFILE_ENABLE ? "enable" : "???",
					cc->scores[type], score));
		cc->scores[type] = score;
		rc = 0;
		ಆದರೆ (type == UL_COLORFILE_SCHEME)
			strncpy(sfile, d->d_name, ಗಾತ್ರ(sfile));
	}

	ಆದರೆ (*sfile) {
		sfile[ಗಾತ್ರ(sfile) - 1] = '\0';
		ಆದರೆ (asprintf(&cc->sfile, "%s/%s", dirname, sfile) <= 0)
			rc = -ENOMEM;
	}

	closedir(dir);
	ಹಿಂತಿರುಗಿ rc;
}

/* atexit() wrapper */
ಸ್ಥಿರ ಶೂನ್ಯ colors_deinit(ಶೂನ್ಯ)
{
	colors_reset(&ul_colors);
}

/*
 * Returns path to $XDG_CONFIG_HOME/terminal-colors.d
 */
ಸ್ಥಿರ ಅಕ್ಷರ *colors_get_homedir(ಅಕ್ಷರ *buf, size_t bufsz)
{
	ಅಕ್ಷರ *p = getenv("XDG_CONFIG_HOME");

	ಆದರೆ (p) {
		snprintf(buf, bufsz, "%s/" ಮಾರ್ಗ_TERMCOLORS_DIRNAME, p);
		ಹಿಂತಿರುಗಿ buf;
	}

	p = getenv("HOME");
	ಆದರೆ (p) {
		snprintf(buf, bufsz, "%s/.config/" ಮಾರ್ಗ_TERMCOLORS_DIRNAME, p);
		ಹಿಂತಿರುಗಿ buf;
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/*
 * Adds one color sequence to array with color scheme.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ colors_ಕೂಡಿಸು_scheme(ರಚನೆ ul_color_ctl *cc,
			     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name,
			     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *seq0)
{
	ರಚನೆ ul_color_scheme *cs = ಖಾಲಿ;
	ಅಕ್ಷರ *seq = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ rc;

	ಆದರೆ (!cc || !name || !*name || !seq0 || !*seq0)
		ಹಿಂತಿರುಗಿ -EINVAL;

	DBG(SCHEME, ul_debug("add '%s'", name));

	seq = color_get_sequence(seq0);
	ಆದರೆ (!seq)
		ಹಿಂತಿರುಗಿ -EINVAL;
	rc = -ENOMEM;

	/* enlarge the array */
	ಆದರೆ (cc->nschemes == cc->schemes_sz) {
		ಶೂನ್ಯ *tmp = reallocarray(cc->schemes, cc->nschemes + 10,
					 ಗಾತ್ರ(ರಚನೆ ul_color_scheme));
		ಆದರೆ (!tmp)
			ಹೋಗು err;
		cc->schemes = tmp;
		cc->schemes_sz = cc->nschemes + 10;
	}

	/* add a new item */
	cs = &cc->schemes[cc->nschemes];
	cs->seq = seq;
	cs->name = strdup(name);
	ಆದರೆ (!cs->name)
		ಹೋಗು err;

	cc->nschemes++;
	ಹಿಂತಿರುಗಿ 0;
err:
	ಆದರೆ (cs) {
		ಬಿಡುಗಡೆ(cs->seq);
		ಬಿಡುಗಡೆ(cs->name);
		cs->seq = cs->name = ಖಾಲಿ;
	} ಇಲ್ಲದಿದ್ದರೆ
		ಬಿಡುಗಡೆ(seq);
	ಹಿಂತಿರುಗಿ rc;
}

/*
 * Deallocates all regards to color schemes
 */
ಸ್ಥಿರ ಶೂನ್ಯ colors_free_schemes(ರಚನೆ ul_color_ctl *cc)
{
	size_t i;

	DBG(SCHEME, ul_debug("free scheme"));

	ಗಾಗಿ (i = 0; i < cc->nschemes; i++) {
		ಬಿಡುಗಡೆ(cc->schemes[i].name);
		ಬಿಡುಗಡೆ(cc->schemes[i].seq);
	}

	ಬಿಡುಗಡೆ(cc->schemes);
	cc->schemes = ಖಾಲಿ;
	cc->nschemes = 0;
	cc->schemes_sz = 0;
}

/*
 * The scheme configuration has to be sorted for bsearch
 */
ಸ್ಥಿರ ಶೂನ್ಯ colors_ವಿಂಗಡಿಸು_schemes(ರಚನೆ ul_color_ctl *cc)
{
	ಆದರೆ (!cc->nschemes)
		ಹಿಂತಿರುಗಿ;

	DBG(SCHEME, ul_debug("sort scheme"));

	qsort(cc->schemes, cc->nschemes,
	      ಗಾತ್ರ(ರಚನೆ ul_color_scheme), cmp_scheme_name);
}

/*
 * Returns just one color scheme
 */
ಸ್ಥಿರ ರಚನೆ ul_color_scheme *colors_get_scheme(ರಚನೆ ul_color_ctl *cc,
						 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ರಚನೆ ul_color_scheme ಕೀಲಿ = { .name = (ಅಕ್ಷರ *) name}, *res;

	ಆದರೆ (!cc || !name || !*name)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (!cc->cs_configured) {
		ಪೂರ್ಣಾಂಕ rc = colors_read_schemes(cc);
		ಆದರೆ (rc)
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}
	ಆದರೆ (!cc->nschemes)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	DBG(SCHEME, ul_debug("search '%s'", name));

	res = bsearch(&ಕೀಲಿ, cc->schemes, cc->nschemes,
				ಗಾತ್ರ(ರಚನೆ ul_color_scheme),
				cmp_scheme_name);

	ಹಿಂತಿರುಗಿ res && res->seq ? res  : ಖಾಲಿ;
}

/*
 * Parses filenames in terminal-colors.d
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ colors_read_configuration(ರಚನೆ ul_color_ctl *cc)
{
	ಪೂರ್ಣಾಂಕ rc = -ENOENT;
	ಅಕ್ಷರ *dirname, buf[PATH_MAX];

	cc->termname = getenv("TERM");

	dirname = colors_get_homedir(buf, ಗಾತ್ರ(buf));
	ಆದರೆ (dirname)
		rc = colors_readdir(cc, dirname);		/* ~/.config */
	ಆದರೆ (rc == -EPERM || rc == -EACCES || rc == -ENOENT)
		rc = colors_readdir(cc, ಮಾರ್ಗ_TERMCOLORS_DIR);	/* /etc */

	cc->configured = 1;
	ಹಿಂತಿರುಗಿ rc;
}

/*
 * Reads terminal-colors.d/ scheme file into array schemes
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ colors_read_schemes(ರಚನೆ ul_color_ctl *cc)
{
	ಪೂರ್ಣಾಂಕ rc = 0;
	FILE *f = ಖಾಲಿ;
	ಅಕ್ಷರ buf[BUFSIZ],
	     cn[129], seq[129];

	ಆದರೆ (!cc->configured)
		rc = colors_read_configuration(cc);

	cc->cs_configured = 1;

	ಆದರೆ (rc || !cc->sfile)
		ಹೋಗು ಮುಗಿದ;

	DBG(SCHEME, ul_debug("reading file '%s'", cc->sfile));

	f = fopen(cc->sfile, "r" UL_CLOEXECSTR);
	ಆದರೆ (!f) {
		rc = -errno;
		ಹೋಗು ಮುಗಿದ;
	}

	ಆಗ (fgets(buf, ಗಾತ್ರ(buf), f)) {
		ಅಕ್ಷರ *p = strchr(buf, '\n');

		ಆದರೆ (!p) {
			ಆದರೆ (feof(f))
				p = strchr(buf, '\0');
			ಇಲ್ಲದಿದ್ದರೆ {
				rc = -errno;
				ಹೋಗು ಮುಗಿದ;
			}
		}
		*p = '\0';
		p = (ಅಕ್ಷರ *) skip_blank(buf);
		ಆದರೆ (*p == '\0' || *p == '#')
			ಮುಂದುವರಿ;

		rc = ಅಕ್ಷರಸರಣಿ_ಓದು(p, "%128[^ ] %128[^\n ]", cn, seq);
		ಆದರೆ (rc == 2 && *cn && *seq) {
			rc = colors_ಕೂಡಿಸು_scheme(cc, cn, seq);	/* set rc=0 on success */
			ಆದರೆ (rc)
				ಹೋಗು ಮುಗಿದ;
		}
	}
	rc = 0;

ಮುಗಿದ:
	ಆದರೆ (f)
		fclose(f);
	colors_ವಿಂಗಡಿಸು_schemes(cc);

	ಹಿಂತಿರುಗಿ rc;
}


ಸ್ಥಿರ ಶೂನ್ಯ termcolors_init_debug(ಶೂನ್ಯ)
{
	__UL_INIT_DEBUG_FROM_ENV(termcolors, TERMCOLORS_DEBUG_, 0, TERMINAL_COLORS_DEBUG);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ colors_terminal_is_ready(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ ncolors = -1;

#ಆದರೆ defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
	{
		ಪೂರ್ಣಾಂಕ ret;

		/* setupterm() allocates memory, del_curterm() deallocates it */
		ಆದರೆ (setupterm(ಖಾಲಿ, STDOUT_FILENO, &ret) == 0 && ret == 1) {
			ncolors = tigetnum("colors");
			del_curterm(cur_ಪದ);
		}
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (1 < ncolors) {
		DBG(CONF, ul_debug("terminal is ready (supports %d colors)", ncolors));
		ಹಿಂತಿರುಗಿ 1;
	}

	DBG(CONF, ul_debug("terminal is NOT ready (no colors)"));
	ಹಿಂತಿರುಗಿ 0;
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
ಪೂರ್ಣಾಂಕ colors_init(ಪೂರ್ಣಾಂಕ ಬಹುಲಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಪೂರ್ಣಾಂಕ ready = -1;
	ರಚನೆ ul_color_ctl *cc = &ul_colors;

	cc->utilname = name;

	termcolors_init_debug();

	ಆದರೆ (ಬಹುಲಕ != UL_COLORMODE_ALWAYS && !isatty(STDOUT_FILENO))
		cc->ಬಹುಲಕ = UL_COLORMODE_NEVER;
	ಇಲ್ಲದಿದ್ದರೆ
		cc->ಬಹುಲಕ = ಬಹುಲಕ;

	ಆದರೆ (cc->ಬಹುಲಕ == UL_COLORMODE_UNDEF
	    && getenv("NO_COLOR") == ಖಾಲಿ
	    && (ready = colors_terminal_is_ready())) {

		ಪೂರ್ಣಾಂಕ rc = colors_read_configuration(cc);
		ಆದರೆ (rc)
			cc->ಬಹುಲಕ = UL_COLORMODE_DEFAULT;
		ಇಲ್ಲದಿದ್ದರೆ {

			/* evaluate scores */
			ಆದರೆ (cc->scores[UL_COLORFILE_DISABLE] >
			    cc->scores[UL_COLORFILE_ENABLE])
				cc->ಬಹುಲಕ = UL_COLORMODE_NEVER;
			ಇಲ್ಲದಿದ್ದರೆ
				cc->ಬಹುಲಕ = UL_COLORMODE_DEFAULT;

			atexit(colors_deinit);
		}
	}

	ಆಯ್ಕೆ (cc->ಬಹುಲಕ) {
	ಸ್ಥಿತಿ UL_COLORMODE_AUTO:
		cc->has_colors = ready == -1 ? colors_terminal_is_ready() : ready;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ UL_COLORMODE_ALWAYS:
		cc->has_colors = 1;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ UL_COLORMODE_NEVER:
	ಪೂರ್ವನಿಯೋಜಿತ:
		cc->has_colors = 0;
	}

	ON_DBG(CONF, colors_debug(cc));

	ಹಿಂತಿರುಗಿ cc->has_colors;
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
ಪೂರ್ಣಾಂಕ colors_wanted(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ ul_colors.has_colors;
}

/*
 * Returns mode
 */
ಪೂರ್ಣಾಂಕ colors_ಬಹುಲಕ(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ ul_colors.ಬಹುಲಕ;
}

/*
 * Enable @seq color
 */
ಶೂನ್ಯ color_fenable(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *seq, FILE *f)
{
	ಆದರೆ (!ul_colors.disabled && ul_colors.has_colors && seq)
		fputs(seq, f);
}

/*
 * Returns escape sequence by logical @name, if undefined then returns @dflt.
 */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *color_scheme_get_sequence(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dflt)
{
	ರಚನೆ ul_color_scheme *cs;

	ಆದರೆ (ul_colors.disabled || !ul_colors.has_colors)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	cs = colors_get_scheme(&ul_colors, name);
	ಹಿಂತಿರುಗಿ cs && cs->seq ? cs->seq : dflt;
}

/*
 * Enable color by logical @name, if undefined enable @dflt.
 */
ಶೂನ್ಯ color_scheme_fenable(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dflt, FILE *f)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *seq = color_scheme_get_sequence(name, dflt);

	ಆದರೆ (!seq)
		ಹಿಂತಿರುಗಿ;
	color_fenable(seq, f);
}


/*
 * Disable previously enabled color
 */
ಶೂನ್ಯ color_fdisable(FILE *f)
{
	ಆದರೆ (!ul_colors.disabled && ul_colors.has_colors)
		fputs(UL_COLOR_RESET, f);
}

/*
 * Get reset sequence
 */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *color_get_disable_sequence(ಶೂನ್ಯ)
{
	ಆದರೆ (!ul_colors.disabled && ul_colors.has_colors)
		ಹಿಂತಿರುಗಿ UL_COLOR_RESET;
	ಇಲ್ಲದಿದ್ದರೆ
		ಹಿಂತಿರುಗಿ "";
}

/*
 * Parses @str to return UL_COLORMODE_*
 */
ಪೂರ್ಣಾಂಕ colormode_from_string(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	size_t i;
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ modes[] = {
		[UL_COLORMODE_AUTO]   = "auto",
		[UL_COLORMODE_NEVER]  = "never",
		[UL_COLORMODE_ALWAYS] = "always",
		[UL_COLORMODE_UNDEF] = ""
	};

	ಆದರೆ (!str || !*str)
		ಹಿಂತಿರುಗಿ -EINVAL;

	ಪ್ರತಿಜ್ಞಾ(ARRAY_ಗಾತ್ರ(modes) == __UL_NCOLORMODES);

	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(modes); i++) {
		ಆದರೆ (c_strcasecmp(str, modes[i]) == 0)
			ಹಿಂತಿರುಗಿ i;
	}

	ಹಿಂತಿರುಗಿ -EINVAL;
}

/*
 * Parses @str and exit(EXIT_FAILURE) on error
 */
ಪೂರ್ಣಾಂಕ colormode_or_err(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = str && *str == '=' ? str + 1 : str;
	ಪೂರ್ಣಾಂಕ colormode;

	colormode = colormode_from_string(p);
	ಆದರೆ (colormode < 0)
		errx(EXIT_FAILURE, _("unsupported color mode: %s"), p);

	ಹಿಂತಿರುಗಿ colormode;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_COLORS
# include <getopt.h>
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[] = {
		{ "mode",	required_argument, ಖಾಲಿ, 'm' },
		{ "color",	required_argument, ಖಾಲಿ, 'c' },
		{ "color-scheme", required_argument, ಖಾಲಿ, 'C' },
		{ "name",	required_argument, ಖಾಲಿ, 'n' },
		{ ಖಾಲಿ, 0, ಖಾಲಿ, 0 }
	};
	ಪೂರ್ಣಾಂಕ c, ಬಹುಲಕ = UL_COLORMODE_UNDEF;	/* default */
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *color = "red", *name = ಖಾಲಿ, *color_scheme = ಖಾಲಿ;
	ಅಕ್ಷರ *seq = ಖಾಲಿ;

	ಆಗ ((c = getopt_long(argc, argv, "C:c:m:n:", longopts, ಖಾಲಿ)) != -1) {
		ಆಯ್ಕೆ (c) {
		ಸ್ಥಿತಿ 'c':
			color = optarg;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'C':
			color_scheme = optarg;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'm':
			ಬಹುಲಕ = colormode_or_err(optarg);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'n':
			name = optarg;
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			fprintf(stderr, "usage: %s [options]\n"
			" -m, --mode <auto|never|always>  default is undefined\n"
			" -c, --color <red|blue|...>      color for the test message\n"
			" -C, --color-scheme <name>       color for the test message\n"
			" -n, --name <utilname>           util name\n",
			ಕಾರ್ಯಕ್ರಮ_invocation_short_name);
			ಹಿಂತಿರುಗಿ EXIT_FAILURE;
		}
	}

	colors_init(ಬಹುಲಕ, name ? name : ಕಾರ್ಯಕ್ರಮ_invocation_short_name);

	ಆದರೆ (color_is_sequence(color))
		seq = strdup(color);
	ಇಲ್ಲದಿದ್ದರೆ
		seq = color_get_sequence(color);

	ಆದರೆ (color_scheme)
		color_scheme_enable(color_scheme, seq);
	ಇಲ್ಲದಿದ್ದರೆ
		color_enable(seq);
	ಬರೆ("Hello World!");
	color_disable();
	fputc('\n', stdout);

	ಬಿಡುಗಡೆ(seq);

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
#ಆದರೆ_ಅಂತ್ಯ /* TEST_PROGRAM_COLORS */

