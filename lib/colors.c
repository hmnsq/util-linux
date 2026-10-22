/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: 2012 Ondrej Oprala <ooprala@redhat.com>
 *          2012-2025 Karel Zak <kzak@redhat.com>
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dirent.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>

#ਜੇ defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
# ਜੇ defined(HAVE_NCURSESW_NCURSES_H)
#  include <ncursesw/ncurses.h>
# elif defined(HAVE_NCURSES_NCURSES_H)
#  include <ncurses/ncurses.h>
# elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
# endif
# ਜੇ defined(HAVE_NCURSESW_ਪਦ_H)
#  include <ncursesw/term.h>
# elif defined(HAVE_NCURSES_ਪਦ_H)
#  include <ncurses/term.h>
# elif defined(HAVE_ਪਦ_H)
#  include <term.h>
# endif
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "cctype.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "colors.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "nls.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "pathnames.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "strutils.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "debug.h"

/*
 * Default behavior, may be overridden by terminal-colors.d/{enable,disable}.
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ USE_COLORS_BY_DEFAULT
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_AUTO	/* check isatty() */
#ਨਹੀਂਤਾਂ
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_NEVER	/* no colors by default */
#ਜੇ_ਅੰਤ

/*
 * terminal-colors.d debug stuff
 */
ਸਥਿਰ UL_DEBUG_DEFINE_ਨਕਾਬ(termcolors);
UL_DEBUG_DEFINE_MASKNAMES(termcolors) = UL_DEBUG_ਖਾਲੀ_ਥਾਵਾਂ_MASKNAMES;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TERMCOLORS_DEBUG_INIT	(1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TERMCOLORS_DEBUG_CONF	(1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TERMCOLORS_DEBUG_SCHEME	(1 << 3)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG(m, x)       __UL_DBG(termcolors, TERMCOLORS_DEBUG_, m, x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ON_DBG(m, x)    __UL_DBG_CALL(termcolors, TERMCOLORS_DEBUG_, m, x)

/*
 * terminal-colors.d file types
 */
ਗਿਣਤੀ {
	UL_COLORFILE_DISABLE,		/* .disable */
	UL_COLORFILE_ENABLE,		/* .enable */
	UL_COLORFILE_SCHEME,		/* .scheme */

	__UL_COLORFILE_COUNT
};

ਬਣਤਰ ul_color_scheme {
	ਅੱਖਰ *name;
	ਅੱਖਰ *seq;
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
ਬਣਤਰ ul_color_ctl {
	ਅਚਲ ਅੱਖਰ	*utilname;	/* util name */
	ਅਚਲ ਅੱਖਰ	*termname;	/* terminal name ($TERM) */

	ਅੱਖਰ		*sfile;	/* path to scheme */

	ਬਣਤਰ ul_color_scheme	*schemes;	/* array with color schemes */
	size_t			nschemes;	/* number of the items */
	size_t			schemes_sz;	/* number of the allocated items */

	ਪੂਰਨ		ਬਹੁਲਕ;		/* UL_COLORMODE_* */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ	has_colors : 1,	/* based on mode and scores[] */
			disabled   : 1, /* disable colors */
			cs_configured : 1, /* color schemes read */
			configured : 1; /* terminal-colors.d parsed */

	ਪੂਰਨ		scores[__UL_COLORFILE_COUNT];	/* the best match */
};

/*
 * Control struct, globally shared.
 */
ਸਥਿਰ ਬਣਤਰ ul_color_ctl ul_colors;

ਸਥਿਰ ਖਾਲੀ colors_free_schemes(ਬਣਤਰ ul_color_ctl *cc);
ਸਥਿਰ ਪੂਰਨ colors_read_schemes(ਬਣਤਰ ul_color_ctl *cc);

/*
 * qsort/bsearch buddy
 */
ਸਥਿਰ ਪੂਰਨ cmp_scheme_name(ਅਚਲ ਖਾਲੀ *a0, ਅਚਲ ਖਾਲੀ *b0)
{
	ਅਚਲ ਬਣਤਰ ul_color_scheme *a = (ਅਚਲ ਬਣਤਰ ul_color_scheme *) a0,
				     *b = (ਅਚਲ ਬਣਤਰ ul_color_scheme *) b0;
	ਵਾਪਸ strcmp(a->name, b->name);
}

/*
 * Resets control struct (note that we don't allocate the struct)
 */
ਸਥਿਰ ਖਾਲੀ colors_reset(ਬਣਤਰ ul_color_ctl *cc)
{
	ਜੇ (!cc)
		ਵਾਪਸ;

	colors_free_schemes(cc);

	ਮੁਕਤ(cc->sfile);

	cc->sfile = ਨਿਰਾ;
	cc->utilname = ਨਿਰਾ;
	cc->termname = ਨਿਰਾ;
	cc->ਬਹੁਲਕ = UL_COLORMODE_UNDEF;

	memset(cc->scores, 0, ਆਕਾਰ(cc->scores));
}

ਸਥਿਰ ਖਾਲੀ colors_debug(ਬਣਤਰ ul_color_ctl *cc)
{
	size_t i;

	ਜੇ (!cc)
		ਵਾਪਸ;

	ਲਿਖੋ("Colors:\n");
	ਲਿਖੋ("\tutilname = '%s'\n", cc->utilname);
	ਲਿਖੋ("\ttermname = '%s'\n", cc->termname);
	ਲਿਖੋ("\tscheme file = '%s'\n", cc->sfile);
	ਲਿਖੋ("\tmode = %s\n",
			cc->ਬਹੁਲਕ == UL_COLORMODE_UNDEF ? "undefined" :
			cc->ਬਹੁਲਕ == UL_COLORMODE_AUTO ?  "auto" :
			cc->ਬਹੁਲਕ == UL_COLORMODE_NEVER ? "never" :
			cc->ਬਹੁਲਕ == UL_COLORMODE_ALWAYS ? "always" : "???");
	ਲਿਖੋ("\thas_colors = %d\n", cc->has_colors);
	ਲਿਖੋ("\tdisabled = %d\n", cc->disabled);
	ਲਿਖੋ("\tconfigured = %d\n", cc->configured);
	ਲਿਖੋ("\tcs configured = %d\n", cc->cs_configured);

	fputc('\n', stdout);

	ਲਈ (i = 0; i < ARRAY_ਮਾਪ(cc->scores); i++)
		ਲਿਖੋ("\tscore %s = %d\n",
				i == UL_COLORFILE_DISABLE ? "disable" :
				i == UL_COLORFILE_ENABLE ? "enable" :
				i == UL_COLORFILE_SCHEME ? "scheme" : "???",
				cc->scores[i]);

	fputc('\n', stdout);

	ਲਈ (i = 0; i < cc->nschemes; i++) {
		ਲਿਖੋ("\tscheme #%02zu ", i);
		color_scheme_enable(cc->schemes[i].name, ਨਿਰਾ);
		fputs(cc->schemes[i].name, stdout);
		color_disable();
		fputc('\n', stdout);
	}
	fputc('\n', stdout);
}

/*
 * Parses [[<utilname>][@<termname>].]<type>
 */
ਸਥਿਰ ਪੂਰਨ filename_ਮੰਜ਼ਿਲ_ਟੋਕਨ(ਅਚਲ ਅੱਖਰ *str,
			      ਅਚਲ ਅੱਖਰ **name, size_t *namesz,
			      ਅਚਲ ਅੱਖਰ **ਪਦ, size_t *termsz,
			      ਪੂਰਨ  *filetype)
{
	ਅਚਲ ਅੱਖਰ *type_ਸ਼ੁਰੂ, *ਪਦ_ਸ਼ੁਰੂ, *p;

	ਜੇ (!str || !*str || *str == '.' || ਲੜੀ_ਲੰਬਾਈ(str) > PATH_MAX)
		ਵਾਪਸ -EINVAL;

	/* parse .type */
	p = strrchr(str, '.');
	type_ਸ਼ੁਰੂ = p ? p + 1 : str;

	ਜੇ (strcmp(type_ਸ਼ੁਰੂ, "disable") == 0)
		*filetype = UL_COLORFILE_DISABLE;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(type_ਸ਼ੁਰੂ, "enable") == 0)
		*filetype = UL_COLORFILE_ENABLE;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(type_ਸ਼ੁਰੂ, "scheme") == 0)
		*filetype = UL_COLORFILE_SCHEME;
	ਨਹੀਂਤਾਂ {
		DBG(CONF, ul_debug("unknown type '%s'", type_ਸ਼ੁਰੂ));
		ਵਾਪਸ 1;				/* unknown type */
	}

	ਜੇ (type_ਸ਼ੁਰੂ == str)
		ਵਾਪਸ 0;				/* "type" only */

	/* parse @termname */
	p = strchr(str, '@');
	ਪਦ_ਸ਼ੁਰੂ = p ? p + 1 : ਨਿਰਾ;
	ਜੇ (ਪਦ_ਸ਼ੁਰੂ) {
		*ਪਦ = ਪਦ_ਸ਼ੁਰੂ;
		*termsz = type_ਸ਼ੁਰੂ - ਪਦ_ਸ਼ੁਰੂ - 1;
		ਜੇ (ਪਦ_ਸ਼ੁਰੂ - 1 == str)
			ਵਾਪਸ 0;			/* "@termname.type" */
	}

	/* parse utilname */
	p = ਪਦ_ਸ਼ੁਰੂ ? ਪਦ_ਸ਼ੁਰੂ : type_ਸ਼ੁਰੂ;
	*name = str;
	*namesz = p - str - 1;

	ਵਾਪਸ 0;
}

/*
 * Scans @dirname and select the best matches for UL_COLORFILE_* types.
 * The result is stored to cc->scores. The path to the best "scheme"
 * file is stored to cc->scheme.
 */
ਸਥਿਰ ਪੂਰਨ colors_readdir(ਬਣਤਰ ul_color_ctl *cc, ਅਚਲ ਅੱਖਰ *dirname)
{
	DIR *dir;
	ਪੂਰਨ rc = -ENOENT;
	ਬਣਤਰ dirent *d;
	ਅੱਖਰ sfile[PATH_MAX] = { '\0' };
	size_t namesz, termsz;

	ਜੇ (!dirname || !cc || !cc->utilname || !*cc->utilname)
		ਵਾਪਸ -EINVAL;

	DBG(CONF, ul_debug("reading dir: '%s'", dirname));

	dir = opendir(dirname);
	ਜੇ (!dir)
		ਵਾਪਸ -errno;

	namesz = ਲੜੀ_ਲੰਬਾਈ(cc->utilname);
	termsz = cc->termname ? ਲੜੀ_ਲੰਬਾਈ(cc->termname) : 0;

	ਜਦੋਂ ((d = readdir(dir))) {
		ਪੂਰਨ type, score = 1;
		ਅਚਲ ਅੱਖਰ *tk_name = ਨਿਰਾ, *tk_ਪਦ = ਨਿਰਾ;
		size_t tk_namesz = 0, tk_termsz = 0;

		ਜੇ (*d->d_name == '.')
			ਜਾਰੀ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _DIRENT_HAVE_D_TYPE
		ਜੇ (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK &&
		    d->d_type != DT_REG)
			ਜਾਰੀ;
#ਜੇ_ਅੰਤ
		ਜੇ (filename_ਮੰਜ਼ਿਲ_ਟੋਕਨ(d->d_name,
				       &tk_name, &tk_namesz,
				       &tk_ਪਦ, &tk_termsz, &type) != 0)
			ਜਾਰੀ;

		/* count theoretical score before we check names to avoid
		 * unnecessary strcmp() */
		ਜੇ (tk_name)
			score += 20;
		ਜੇ (tk_ਪਦ)
			score += 10;

		DBG(CONF, ul_debug("item '%s': score=%d "
			"[cur: %d, name(%zu): %s, term(%zu): %s]",
			d->d_name, score, cc->scores[type],
			tk_namesz, tk_name,
			tk_termsz, tk_ਪਦ));


		ਜੇ (score < cc->scores[type])
			ਜਾਰੀ;

		/* filter out by names */
		ਜੇ (tk_namesz && (tk_namesz != namesz ||
				 ਲੜੀ_ਤੁਲਨਾ(tk_name, cc->utilname, namesz) != 0))
			ਜਾਰੀ;

		ਜੇ (tk_termsz && (termsz == 0 || tk_termsz != termsz ||
				  ਲੜੀ_ਤੁਲਨਾ(tk_ਪਦ, cc->termname, termsz) != 0))
			ਜਾਰੀ;

		DBG(CONF, ul_debug("setting '%s' from %d -to-> %d",
					type == UL_COLORFILE_SCHEME ? "scheme" :
					type == UL_COLORFILE_DISABLE ? "disable" :
					type == UL_COLORFILE_ENABLE ? "enable" : "???",
					cc->scores[type], score));
		cc->scores[type] = score;
		rc = 0;
		ਜੇ (type == UL_COLORFILE_SCHEME)
			strncpy(sfile, d->d_name, ਆਕਾਰ(sfile));
	}

	ਜੇ (*sfile) {
		sfile[ਆਕਾਰ(sfile) - 1] = '\0';
		ਜੇ (asprintf(&cc->sfile, "%s/%s", dirname, sfile) <= 0)
			rc = -ENOMEM;
	}

	closedir(dir);
	ਵਾਪਸ rc;
}

/* atexit() wrapper */
ਸਥਿਰ ਖਾਲੀ colors_deinit(ਖਾਲੀ)
{
	colors_reset(&ul_colors);
}

/*
 * Returns path to $XDG_CONFIG_HOME/terminal-colors.d
 */
ਸਥਿਰ ਅੱਖਰ *colors_get_homedir(ਅੱਖਰ *buf, size_t bufsz)
{
	ਅੱਖਰ *p = getenv("XDG_CONFIG_HOME");

	ਜੇ (p) {
		snprintf(buf, bufsz, "%s/" ਰਸਤਾ_TERMCOLORS_DIRNAME, p);
		ਵਾਪਸ buf;
	}

	p = getenv("HOME");
	ਜੇ (p) {
		snprintf(buf, bufsz, "%s/.config/" ਰਸਤਾ_TERMCOLORS_DIRNAME, p);
		ਵਾਪਸ buf;
	}

	ਵਾਪਸ ਨਿਰਾ;
}

/*
 * Adds one color sequence to array with color scheme.
 */
ਸਥਿਰ ਪੂਰਨ colors_ਜੋੜੋ_scheme(ਬਣਤਰ ul_color_ctl *cc,
			     ਅਚਲ ਅੱਖਰ *name,
			     ਅਚਲ ਅੱਖਰ *seq0)
{
	ਬਣਤਰ ul_color_scheme *cs = ਨਿਰਾ;
	ਅੱਖਰ *seq = ਨਿਰਾ;
	ਪੂਰਨ rc;

	ਜੇ (!cc || !name || !*name || !seq0 || !*seq0)
		ਵਾਪਸ -EINVAL;

	DBG(SCHEME, ul_debug("add '%s'", name));

	seq = color_get_sequence(seq0);
	ਜੇ (!seq)
		ਵਾਪਸ -EINVAL;
	rc = -ENOMEM;

	/* enlarge the array */
	ਜੇ (cc->nschemes == cc->schemes_sz) {
		ਖਾਲੀ *tmp = reallocarray(cc->schemes, cc->nschemes + 10,
					 ਆਕਾਰ(ਬਣਤਰ ul_color_scheme));
		ਜੇ (!tmp)
			ਜਾਓ err;
		cc->schemes = tmp;
		cc->schemes_sz = cc->nschemes + 10;
	}

	/* add a new item */
	cs = &cc->schemes[cc->nschemes];
	cs->seq = seq;
	cs->name = strdup(name);
	ਜੇ (!cs->name)
		ਜਾਓ err;

	cc->nschemes++;
	ਵਾਪਸ 0;
err:
	ਜੇ (cs) {
		ਮੁਕਤ(cs->seq);
		ਮੁਕਤ(cs->name);
		cs->seq = cs->name = ਨਿਰਾ;
	} ਨਹੀਂਤਾਂ
		ਮੁਕਤ(seq);
	ਵਾਪਸ rc;
}

/*
 * Deallocates all regards to color schemes
 */
ਸਥਿਰ ਖਾਲੀ colors_free_schemes(ਬਣਤਰ ul_color_ctl *cc)
{
	size_t i;

	DBG(SCHEME, ul_debug("free scheme"));

	ਲਈ (i = 0; i < cc->nschemes; i++) {
		ਮੁਕਤ(cc->schemes[i].name);
		ਮੁਕਤ(cc->schemes[i].seq);
	}

	ਮੁਕਤ(cc->schemes);
	cc->schemes = ਨਿਰਾ;
	cc->nschemes = 0;
	cc->schemes_sz = 0;
}

/*
 * The scheme configuration has to be sorted for bsearch
 */
ਸਥਿਰ ਖਾਲੀ colors_ਛਾਂਟੋ_schemes(ਬਣਤਰ ul_color_ctl *cc)
{
	ਜੇ (!cc->nschemes)
		ਵਾਪਸ;

	DBG(SCHEME, ul_debug("sort scheme"));

	qsort(cc->schemes, cc->nschemes,
	      ਆਕਾਰ(ਬਣਤਰ ul_color_scheme), cmp_scheme_name);
}

/*
 * Returns just one color scheme
 */
ਸਥਿਰ ਬਣਤਰ ul_color_scheme *colors_get_scheme(ਬਣਤਰ ul_color_ctl *cc,
						 ਅਚਲ ਅੱਖਰ *name)
{
	ਬਣਤਰ ul_color_scheme ਕੁੰਜੀ = { .name = (ਅੱਖਰ *) name}, *res;

	ਜੇ (!cc || !name || !*name)
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (!cc->cs_configured) {
		ਪੂਰਨ rc = colors_read_schemes(cc);
		ਜੇ (rc)
			ਵਾਪਸ ਨਿਰਾ;
	}
	ਜੇ (!cc->nschemes)
		ਵਾਪਸ ਨਿਰਾ;

	DBG(SCHEME, ul_debug("search '%s'", name));

	res = bsearch(&ਕੁੰਜੀ, cc->schemes, cc->nschemes,
				ਆਕਾਰ(ਬਣਤਰ ul_color_scheme),
				cmp_scheme_name);

	ਵਾਪਸ res && res->seq ? res  : ਨਿਰਾ;
}

/*
 * Parses filenames in terminal-colors.d
 */
ਸਥਿਰ ਪੂਰਨ colors_read_configuration(ਬਣਤਰ ul_color_ctl *cc)
{
	ਪੂਰਨ rc = -ENOENT;
	ਅੱਖਰ *dirname, buf[PATH_MAX];

	cc->termname = getenv("TERM");

	dirname = colors_get_homedir(buf, ਆਕਾਰ(buf));
	ਜੇ (dirname)
		rc = colors_readdir(cc, dirname);		/* ~/.config */
	ਜੇ (rc == -EPERM || rc == -EACCES || rc == -ENOENT)
		rc = colors_readdir(cc, ਰਸਤਾ_TERMCOLORS_DIR);	/* /etc */

	cc->configured = 1;
	ਵਾਪਸ rc;
}

/*
 * Reads terminal-colors.d/ scheme file into array schemes
 */
ਸਥਿਰ ਪੂਰਨ colors_read_schemes(ਬਣਤਰ ul_color_ctl *cc)
{
	ਪੂਰਨ rc = 0;
	FILE *f = ਨਿਰਾ;
	ਅੱਖਰ buf[BUFSIZ],
	     cn[129], seq[129];

	ਜੇ (!cc->configured)
		rc = colors_read_configuration(cc);

	cc->cs_configured = 1;

	ਜੇ (rc || !cc->sfile)
		ਜਾਓ ਪੂਰਾ;

	DBG(SCHEME, ul_debug("reading file '%s'", cc->sfile));

	f = fopen(cc->sfile, "r" UL_CLOEXECSTR);
	ਜੇ (!f) {
		rc = -errno;
		ਜਾਓ ਪੂਰਾ;
	}

	ਜਦੋਂ (fgets(buf, ਆਕਾਰ(buf), f)) {
		ਅੱਖਰ *p = strchr(buf, '\n');

		ਜੇ (!p) {
			ਜੇ (feof(f))
				p = strchr(buf, '\0');
			ਨਹੀਂਤਾਂ {
				rc = -errno;
				ਜਾਓ ਪੂਰਾ;
			}
		}
		*p = '\0';
		p = (ਅੱਖਰ *) skip_blank(buf);
		ਜੇ (*p == '\0' || *p == '#')
			ਜਾਰੀ;

		rc = ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(p, "%128[^ ] %128[^\n ]", cn, seq);
		ਜੇ (rc == 2 && *cn && *seq) {
			rc = colors_ਜੋੜੋ_scheme(cc, cn, seq);	/* set rc=0 on success */
			ਜੇ (rc)
				ਜਾਓ ਪੂਰਾ;
		}
	}
	rc = 0;

ਪੂਰਾ:
	ਜੇ (f)
		fclose(f);
	colors_ਛਾਂਟੋ_schemes(cc);

	ਵਾਪਸ rc;
}


ਸਥਿਰ ਖਾਲੀ termcolors_init_debug(ਖਾਲੀ)
{
	__UL_INIT_DEBUG_FROM_ENV(termcolors, TERMCOLORS_DEBUG_, 0, TERMINAL_COLORS_DEBUG);
}

ਸਥਿਰ ਪੂਰਨ colors_terminal_is_ready(ਖਾਲੀ)
{
	ਪੂਰਨ ncolors = -1;

#ਜੇ defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
	{
		ਪੂਰਨ ret;

		/* setupterm() allocates memory, del_curterm() deallocates it */
		ਜੇ (setupterm(ਨਿਰਾ, STDOUT_FILENO, &ret) == 0 && ret == 1) {
			ncolors = tigetnum("colors");
			del_curterm(cur_ਪਦ);
		}
	}
#ਜੇ_ਅੰਤ
	ਜੇ (1 < ncolors) {
		DBG(CONF, ul_debug("terminal is ready (supports %d colors)", ncolors));
		ਵਾਪਸ 1;
	}

	DBG(CONF, ul_debug("terminal is NOT ready (no colors)"));
	ਵਾਪਸ 0;
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
ਪੂਰਨ colors_init(ਪੂਰਨ ਬਹੁਲਕ, ਅਚਲ ਅੱਖਰ *name)
{
	ਪੂਰਨ ready = -1;
	ਬਣਤਰ ul_color_ctl *cc = &ul_colors;

	cc->utilname = name;

	termcolors_init_debug();

	ਜੇ (ਬਹੁਲਕ != UL_COLORMODE_ALWAYS && !isatty(STDOUT_FILENO))
		cc->ਬਹੁਲਕ = UL_COLORMODE_NEVER;
	ਨਹੀਂਤਾਂ
		cc->ਬਹੁਲਕ = ਬਹੁਲਕ;

	ਜੇ (cc->ਬਹੁਲਕ == UL_COLORMODE_UNDEF
	    && getenv("NO_COLOR") == ਨਿਰਾ
	    && (ready = colors_terminal_is_ready())) {

		ਪੂਰਨ rc = colors_read_configuration(cc);
		ਜੇ (rc)
			cc->ਬਹੁਲਕ = UL_COLORMODE_DEFAULT;
		ਨਹੀਂਤਾਂ {

			/* evaluate scores */
			ਜੇ (cc->scores[UL_COLORFILE_DISABLE] >
			    cc->scores[UL_COLORFILE_ENABLE])
				cc->ਬਹੁਲਕ = UL_COLORMODE_NEVER;
			ਨਹੀਂਤਾਂ
				cc->ਬਹੁਲਕ = UL_COLORMODE_DEFAULT;

			atexit(colors_deinit);
		}
	}

	ਚੋਣ (cc->ਬਹੁਲਕ) {
	ਹਾਲਤ UL_COLORMODE_AUTO:
		cc->has_colors = ready == -1 ? colors_terminal_is_ready() : ready;
		ਰੋਕੋ;
	ਹਾਲਤ UL_COLORMODE_ALWAYS:
		cc->has_colors = 1;
		ਰੋਕੋ;
	ਹਾਲਤ UL_COLORMODE_NEVER:
	ਮੂਲ:
		cc->has_colors = 0;
	}

	ON_DBG(CONF, colors_debug(cc));

	ਵਾਪਸ cc->has_colors;
}

/*
 * Temporary disable colors (this setting is independent on terminal-colors.d/)
 */
ਖਾਲੀ colors_off(ਖਾਲੀ)
{
	ul_colors.disabled = 1;
}

/*
 * Enable colors
 */
ਖਾਲੀ colors_on(ਖਾਲੀ)
{
	ul_colors.disabled = 0;
}

/*
 * Is terminal-colors.d/ configured to use colors?
 */
ਪੂਰਨ colors_wanted(ਖਾਲੀ)
{
	ਵਾਪਸ ul_colors.has_colors;
}

/*
 * Returns mode
 */
ਪੂਰਨ colors_ਬਹੁਲਕ(ਖਾਲੀ)
{
	ਵਾਪਸ ul_colors.ਬਹੁਲਕ;
}

/*
 * Enable @seq color
 */
ਖਾਲੀ color_fenable(ਅਚਲ ਅੱਖਰ *seq, FILE *f)
{
	ਜੇ (!ul_colors.disabled && ul_colors.has_colors && seq)
		fputs(seq, f);
}

/*
 * Returns escape sequence by logical @name, if undefined then returns @dflt.
 */
ਅਚਲ ਅੱਖਰ *color_scheme_get_sequence(ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *dflt)
{
	ਬਣਤਰ ul_color_scheme *cs;

	ਜੇ (ul_colors.disabled || !ul_colors.has_colors)
		ਵਾਪਸ ਨਿਰਾ;

	cs = colors_get_scheme(&ul_colors, name);
	ਵਾਪਸ cs && cs->seq ? cs->seq : dflt;
}

/*
 * Enable color by logical @name, if undefined enable @dflt.
 */
ਖਾਲੀ color_scheme_fenable(ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *dflt, FILE *f)
{
	ਅਚਲ ਅੱਖਰ *seq = color_scheme_get_sequence(name, dflt);

	ਜੇ (!seq)
		ਵਾਪਸ;
	color_fenable(seq, f);
}


/*
 * Disable previously enabled color
 */
ਖਾਲੀ color_fdisable(FILE *f)
{
	ਜੇ (!ul_colors.disabled && ul_colors.has_colors)
		fputs(UL_COLOR_RESET, f);
}

/*
 * Get reset sequence
 */
ਅਚਲ ਅੱਖਰ *color_get_disable_sequence(ਖਾਲੀ)
{
	ਜੇ (!ul_colors.disabled && ul_colors.has_colors)
		ਵਾਪਸ UL_COLOR_RESET;
	ਨਹੀਂਤਾਂ
		ਵਾਪਸ "";
}

/*
 * Parses @str to return UL_COLORMODE_*
 */
ਪੂਰਨ colormode_from_string(ਅਚਲ ਅੱਖਰ *str)
{
	size_t i;
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ modes[] = {
		[UL_COLORMODE_AUTO]   = "auto",
		[UL_COLORMODE_NEVER]  = "never",
		[UL_COLORMODE_ALWAYS] = "always",
		[UL_COLORMODE_UNDEF] = ""
	};

	ਜੇ (!str || !*str)
		ਵਾਪਸ -EINVAL;

	ਪ੍ਰਤਿਜ੍ਞਾ(ARRAY_ਮਾਪ(modes) == __UL_NCOLORMODES);

	ਲਈ (i = 0; i < ARRAY_ਮਾਪ(modes); i++) {
		ਜੇ (c_strcasecmp(str, modes[i]) == 0)
			ਵਾਪਸ i;
	}

	ਵਾਪਸ -EINVAL;
}

/*
 * Parses @str and exit(EXIT_FAILURE) on error
 */
ਪੂਰਨ colormode_or_err(ਅਚਲ ਅੱਖਰ *str)
{
	ਅਚਲ ਅੱਖਰ *p = str && *str == '=' ? str + 1 : str;
	ਪੂਰਨ colormode;

	colormode = colormode_from_string(p);
	ਜੇ (colormode < 0)
		errx(EXIT_FAILURE, _("unsupported color mode: %s"), p);

	ਵਾਪਸ colormode;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_COLORS
# include <getopt.h>
ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ option longopts[] = {
		{ "mode",	required_argument, ਨਿਰਾ, 'm' },
		{ "color",	required_argument, ਨਿਰਾ, 'c' },
		{ "color-scheme", required_argument, ਨਿਰਾ, 'C' },
		{ "name",	required_argument, ਨਿਰਾ, 'n' },
		{ ਨਿਰਾ, 0, ਨਿਰਾ, 0 }
	};
	ਪੂਰਨ c, ਬਹੁਲਕ = UL_COLORMODE_UNDEF;	/* default */
	ਅਚਲ ਅੱਖਰ *color = "red", *name = ਨਿਰਾ, *color_scheme = ਨਿਰਾ;
	ਅੱਖਰ *seq = ਨਿਰਾ;

	ਜਦੋਂ ((c = getopt_long(argc, argv, "C:c:m:n:", longopts, ਨਿਰਾ)) != -1) {
		ਚੋਣ (c) {
		ਹਾਲਤ 'c':
			color = optarg;
			ਰੋਕੋ;
		ਹਾਲਤ 'C':
			color_scheme = optarg;
			ਰੋਕੋ;
		ਹਾਲਤ 'm':
			ਬਹੁਲਕ = colormode_or_err(optarg);
			ਰੋਕੋ;
		ਹਾਲਤ 'n':
			name = optarg;
			ਰੋਕੋ;
		ਮੂਲ:
			fprintf(stderr, "usage: %s [options]\n"
			" -m, --mode <auto|never|always>  default is undefined\n"
			" -c, --color <red|blue|...>      color for the test message\n"
			" -C, --color-scheme <name>       color for the test message\n"
			" -n, --name <utilname>           util name\n",
			ਪ੍ਰੋਗਰਾਮ_invocation_short_name);
			ਵਾਪਸ EXIT_FAILURE;
		}
	}

	colors_init(ਬਹੁਲਕ, name ? name : ਪ੍ਰੋਗਰਾਮ_invocation_short_name);

	ਜੇ (color_is_sequence(color))
		seq = strdup(color);
	ਨਹੀਂਤਾਂ
		seq = color_get_sequence(color);

	ਜੇ (color_scheme)
		color_scheme_enable(color_scheme, seq);
	ਨਹੀਂਤਾਂ
		color_enable(seq);
	ਲਿਖੋ("Hello World!");
	color_disable();
	fputc('\n', stdout);

	ਮੁਕਤ(seq);

	ਵਾਪਸ EXIT_SUCCESS;
}
#ਜੇ_ਅੰਤ /* TEST_PROGRAM_COLORS */

