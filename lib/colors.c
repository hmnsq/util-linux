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

#إذا defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
# إذا defined(HAVE_NCURSESW_NCURSES_H)
#  include <ncursesw/ncurses.h>
# elif defined(HAVE_NCURSES_NCURSES_H)
#  include <ncurses/ncurses.h>
# elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
# endif
# إذا defined(HAVE_NCURSESW_رکن_H)
#  include <ncursesw/term.h>
# elif defined(HAVE_NCURSES_رکن_H)
#  include <ncurses/term.h>
# elif defined(HAVE_رکن_H)
#  include <term.h>
# endif
#جے_انت

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
#جے_تعریف_شدہ USE_COLORS_BY_DEFAULT
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_AUTO	/* check isatty() */
#وإلا
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_NEVER	/* no colors by default */
#جے_انت

/*
 * terminal-colors.d debug stuff
 */
ساكن UL_DEBUG_DEFINE_نقاب(termcolors);
UL_DEBUG_DEFINE_MASKNAMES(termcolors) = UL_DEBUG_خالی_تھاواں_MASKNAMES;

#تعریف_کرو TERMCOLORS_DEBUG_INIT	(1 << 1)
#تعریف_کرو TERMCOLORS_DEBUG_CONF	(1 << 2)
#تعریف_کرو TERMCOLORS_DEBUG_SCHEME	(1 << 3)

#تعریف_کرو DBG(m, x)       __UL_DBG(termcolors, TERMCOLORS_DEBUG_, m, x)
#تعریف_کرو ON_DBG(m, x)    __UL_DBG_CALL(termcolors, TERMCOLORS_DEBUG_, m, x)

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

	صحيح		منوال;		/* UL_COLORMODE_* */
	بنانشانی صحيح	has_colors : 1,	/* based on mode and scores[] */
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
	أرجع strcmp(a->name, b->name);
}

/*
 * Resets control struct (note that we don't allocate the struct)
 */
ساكن فراغ colors_reset(بنية ul_color_ctl *cc)
{
	إذا (!cc)
		أرجع;

	colors_free_schemes(cc);

	حرر(cc->sfile);

	cc->sfile = عدم;
	cc->utilname = عدم;
	cc->termname = عدم;
	cc->منوال = UL_COLORMODE_UNDEF;

	memset(cc->scores, 0, حجم(cc->scores));
}

ساكن فراغ colors_debug(بنية ul_color_ctl *cc)
{
	size_t i;

	إذا (!cc)
		أرجع;

	لکھو("Colors:\n");
	لکھو("\tutilname = '%s'\n", cc->utilname);
	لکھو("\ttermname = '%s'\n", cc->termname);
	لکھو("\tscheme file = '%s'\n", cc->sfile);
	لکھو("\tmode = %s\n",
			cc->منوال == UL_COLORMODE_UNDEF ? "undefined" :
			cc->منوال == UL_COLORMODE_AUTO ?  "auto" :
			cc->منوال == UL_COLORMODE_NEVER ? "never" :
			cc->منوال == UL_COLORMODE_ALWAYS ? "always" : "???");
	لکھو("\thas_colors = %d\n", cc->has_colors);
	لکھو("\tdisabled = %d\n", cc->disabled);
	لکھو("\tconfigured = %d\n", cc->configured);
	لکھو("\tcs configured = %d\n", cc->cs_configured);

	fputc('\n', stdout);

	لكل (i = 0; i < ARRAY_ناپ(cc->scores); i++)
		لکھو("\tscore %s = %d\n",
				i == UL_COLORFILE_DISABLE ? "disable" :
				i == UL_COLORFILE_ENABLE ? "enable" :
				i == UL_COLORFILE_SCHEME ? "scheme" : "???",
				cc->scores[i]);

	fputc('\n', stdout);

	لكل (i = 0; i < cc->nschemes; i++) {
		لکھو("\tscheme #%02zu ", i);
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
ساكن صحيح filename_منزل_ٹوکن(ثابت حرف *str,
			      ثابت حرف **name, size_t *namesz,
			      ثابت حرف **رکن, size_t *termsz,
			      صحيح  *filetype)
{
	ثابت حرف *type_شروع, *رکن_شروع, *p;

	إذا (!str || !*str || *str == '.' || لڑی_لمبائی(str) > PATH_MAX)
		أرجع -EINVAL;

	/* parse .type */
	p = strrchr(str, '.');
	type_شروع = p ? p + 1 : str;

	إذا (strcmp(type_شروع, "disable") == 0)
		*filetype = UL_COLORFILE_DISABLE;
	وإلا إذا (strcmp(type_شروع, "enable") == 0)
		*filetype = UL_COLORFILE_ENABLE;
	وإلا إذا (strcmp(type_شروع, "scheme") == 0)
		*filetype = UL_COLORFILE_SCHEME;
	وإلا {
		DBG(CONF, ul_debug("unknown type '%s'", type_شروع));
		أرجع 1;				/* unknown type */
	}

	إذا (type_شروع == str)
		أرجع 0;				/* "type" only */

	/* parse @termname */
	p = strchr(str, '@');
	رکن_شروع = p ? p + 1 : عدم;
	إذا (رکن_شروع) {
		*رکن = رکن_شروع;
		*termsz = type_شروع - رکن_شروع - 1;
		إذا (رکن_شروع - 1 == str)
			أرجع 0;			/* "@termname.type" */
	}

	/* parse utilname */
	p = رکن_شروع ? رکن_شروع : type_شروع;
	*name = str;
	*namesz = p - str - 1;

	أرجع 0;
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

	إذا (!dirname || !cc || !cc->utilname || !*cc->utilname)
		أرجع -EINVAL;

	DBG(CONF, ul_debug("reading dir: '%s'", dirname));

	dir = opendir(dirname);
	إذا (!dir)
		أرجع -errno;

	namesz = لڑی_لمبائی(cc->utilname);
	termsz = cc->termname ? لڑی_لمبائی(cc->termname) : 0;

	طالما ((d = readdir(dir))) {
		صحيح type, score = 1;
		ثابت حرف *tk_name = عدم, *tk_رکن = عدم;
		size_t tk_namesz = 0, tk_termsz = 0;

		إذا (*d->d_name == '.')
			استمر;
#جے_تعریف_شدہ _DIRENT_HAVE_D_TYPE
		إذا (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK &&
		    d->d_type != DT_REG)
			استمر;
#جے_انت
		إذا (filename_منزل_ٹوکن(d->d_name,
				       &tk_name, &tk_namesz,
				       &tk_رکن, &tk_termsz, &type) != 0)
			استمر;

		/* count theoretical score before we check names to avoid
		 * unnecessary strcmp() */
		إذا (tk_name)
			score += 20;
		إذا (tk_رکن)
			score += 10;

		DBG(CONF, ul_debug("item '%s': score=%d "
			"[cur: %d, name(%zu): %s, term(%zu): %s]",
			d->d_name, score, cc->scores[type],
			tk_namesz, tk_name,
			tk_termsz, tk_رکن));


		إذا (score < cc->scores[type])
			استمر;

		/* filter out by names */
		إذا (tk_namesz && (tk_namesz != namesz ||
				 لڑی_مقابلہ(tk_name, cc->utilname, namesz) != 0))
			استمر;

		إذا (tk_termsz && (termsz == 0 || tk_termsz != termsz ||
				  لڑی_مقابلہ(tk_رکن, cc->termname, termsz) != 0))
			استمر;

		DBG(CONF, ul_debug("setting '%s' from %d -to-> %d",
					type == UL_COLORFILE_SCHEME ? "scheme" :
					type == UL_COLORFILE_DISABLE ? "disable" :
					type == UL_COLORFILE_ENABLE ? "enable" : "???",
					cc->scores[type], score));
		cc->scores[type] = score;
		rc = 0;
		إذا (type == UL_COLORFILE_SCHEME)
			strncpy(sfile, d->d_name, حجم(sfile));
	}

	إذا (*sfile) {
		sfile[حجم(sfile) - 1] = '\0';
		إذا (asprintf(&cc->sfile, "%s/%s", dirname, sfile) <= 0)
			rc = -ENOMEM;
	}

	closedir(dir);
	أرجع rc;
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

	إذا (p) {
		snprintf(buf, bufsz, "%s/" رستہ_TERMCOLORS_DIRNAME, p);
		أرجع buf;
	}

	p = getenv("HOME");
	إذا (p) {
		snprintf(buf, bufsz, "%s/.config/" رستہ_TERMCOLORS_DIRNAME, p);
		أرجع buf;
	}

	أرجع عدم;
}

/*
 * Adds one color sequence to array with color scheme.
 */
ساكن صحيح colors_جوڑو_scheme(بنية ul_color_ctl *cc,
			     ثابت حرف *name,
			     ثابت حرف *seq0)
{
	بنية ul_color_scheme *cs = عدم;
	حرف *seq = عدم;
	صحيح rc;

	إذا (!cc || !name || !*name || !seq0 || !*seq0)
		أرجع -EINVAL;

	DBG(SCHEME, ul_debug("add '%s'", name));

	seq = color_get_sequence(seq0);
	إذا (!seq)
		أرجع -EINVAL;
	rc = -ENOMEM;

	/* enlarge the array */
	إذا (cc->nschemes == cc->schemes_sz) {
		فراغ *tmp = reallocarray(cc->schemes, cc->nschemes + 10,
					 حجم(بنية ul_color_scheme));
		إذا (!tmp)
			اذهب err;
		cc->schemes = tmp;
		cc->schemes_sz = cc->nschemes + 10;
	}

	/* add a new item */
	cs = &cc->schemes[cc->nschemes];
	cs->seq = seq;
	cs->name = strdup(name);
	إذا (!cs->name)
		اذهب err;

	cc->nschemes++;
	أرجع 0;
err:
	إذا (cs) {
		حرر(cs->seq);
		حرر(cs->name);
		cs->seq = cs->name = عدم;
	} وإلا
		حرر(seq);
	أرجع rc;
}

/*
 * Deallocates all regards to color schemes
 */
ساكن فراغ colors_free_schemes(بنية ul_color_ctl *cc)
{
	size_t i;

	DBG(SCHEME, ul_debug("free scheme"));

	لكل (i = 0; i < cc->nschemes; i++) {
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
ساكن فراغ colors_ترتیب_دیو_schemes(بنية ul_color_ctl *cc)
{
	إذا (!cc->nschemes)
		أرجع;

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
	بنية ul_color_scheme کلید = { .name = (حرف *) name}, *res;

	إذا (!cc || !name || !*name)
		أرجع عدم;

	إذا (!cc->cs_configured) {
		صحيح rc = colors_read_schemes(cc);
		إذا (rc)
			أرجع عدم;
	}
	إذا (!cc->nschemes)
		أرجع عدم;

	DBG(SCHEME, ul_debug("search '%s'", name));

	res = bsearch(&کلید, cc->schemes, cc->nschemes,
				حجم(بنية ul_color_scheme),
				cmp_scheme_name);

	أرجع res && res->seq ? res  : عدم;
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
	إذا (dirname)
		rc = colors_readdir(cc, dirname);		/* ~/.config */
	إذا (rc == -EPERM || rc == -EACCES || rc == -ENOENT)
		rc = colors_readdir(cc, رستہ_TERMCOLORS_DIR);	/* /etc */

	cc->configured = 1;
	أرجع rc;
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

	إذا (!cc->configured)
		rc = colors_read_configuration(cc);

	cc->cs_configured = 1;

	إذا (rc || !cc->sfile)
		اذهب پورا;

	DBG(SCHEME, ul_debug("reading file '%s'", cc->sfile));

	f = fopen(cc->sfile, "r" UL_CLOEXECSTR);
	إذا (!f) {
		rc = -errno;
		اذهب پورا;
	}

	طالما (fgets(buf, حجم(buf), f)) {
		حرف *p = strchr(buf, '\n');

		إذا (!p) {
			إذا (feof(f))
				p = strchr(buf, '\0');
			وإلا {
				rc = -errno;
				اذهب پورا;
			}
		}
		*p = '\0';
		p = (حرف *) skip_blank(buf);
		إذا (*p == '\0' || *p == '#')
			استمر;

		rc = لڑی_توں_پڑھو(p, "%128[^ ] %128[^\n ]", cn, seq);
		إذا (rc == 2 && *cn && *seq) {
			rc = colors_جوڑو_scheme(cc, cn, seq);	/* set rc=0 on success */
			إذا (rc)
				اذهب پورا;
		}
	}
	rc = 0;

پورا:
	إذا (f)
		fclose(f);
	colors_ترتیب_دیو_schemes(cc);

	أرجع rc;
}


ساكن فراغ termcolors_init_debug(فراغ)
{
	__UL_INIT_DEBUG_FROM_ENV(termcolors, TERMCOLORS_DEBUG_, 0, TERMINAL_COLORS_DEBUG);
}

ساكن صحيح colors_terminal_is_ready(فراغ)
{
	صحيح ncolors = -1;

#إذا defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
	{
		صحيح ret;

		/* setupterm() allocates memory, del_curterm() deallocates it */
		إذا (setupterm(عدم, STDOUT_FILENO, &ret) == 0 && ret == 1) {
			ncolors = tigetnum("colors");
			del_curterm(cur_رکن);
		}
	}
#جے_انت
	إذا (1 < ncolors) {
		DBG(CONF, ul_debug("terminal is ready (supports %d colors)", ncolors));
		أرجع 1;
	}

	DBG(CONF, ul_debug("terminal is NOT ready (no colors)"));
	أرجع 0;
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
صحيح colors_init(صحيح منوال, ثابت حرف *name)
{
	صحيح ready = -1;
	بنية ul_color_ctl *cc = &ul_colors;

	cc->utilname = name;

	termcolors_init_debug();

	إذا (منوال != UL_COLORMODE_ALWAYS && !isatty(STDOUT_FILENO))
		cc->منوال = UL_COLORMODE_NEVER;
	وإلا
		cc->منوال = منوال;

	إذا (cc->منوال == UL_COLORMODE_UNDEF
	    && getenv("NO_COLOR") == عدم
	    && (ready = colors_terminal_is_ready())) {

		صحيح rc = colors_read_configuration(cc);
		إذا (rc)
			cc->منوال = UL_COLORMODE_DEFAULT;
		وإلا {

			/* evaluate scores */
			إذا (cc->scores[UL_COLORFILE_DISABLE] >
			    cc->scores[UL_COLORFILE_ENABLE])
				cc->منوال = UL_COLORMODE_NEVER;
			وإلا
				cc->منوال = UL_COLORMODE_DEFAULT;

			atexit(colors_deinit);
		}
	}

	بدل (cc->منوال) {
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

	أرجع cc->has_colors;
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
	أرجع ul_colors.has_colors;
}

/*
 * Returns mode
 */
صحيح colors_منوال(فراغ)
{
	أرجع ul_colors.منوال;
}

/*
 * Enable @seq color
 */
فراغ color_fenable(ثابت حرف *seq, FILE *f)
{
	إذا (!ul_colors.disabled && ul_colors.has_colors && seq)
		fputs(seq, f);
}

/*
 * Returns escape sequence by logical @name, if undefined then returns @dflt.
 */
ثابت حرف *color_scheme_get_sequence(ثابت حرف *name, ثابت حرف *dflt)
{
	بنية ul_color_scheme *cs;

	إذا (ul_colors.disabled || !ul_colors.has_colors)
		أرجع عدم;

	cs = colors_get_scheme(&ul_colors, name);
	أرجع cs && cs->seq ? cs->seq : dflt;
}

/*
 * Enable color by logical @name, if undefined enable @dflt.
 */
فراغ color_scheme_fenable(ثابت حرف *name, ثابت حرف *dflt, FILE *f)
{
	ثابت حرف *seq = color_scheme_get_sequence(name, dflt);

	إذا (!seq)
		أرجع;
	color_fenable(seq, f);
}


/*
 * Disable previously enabled color
 */
فراغ color_fdisable(FILE *f)
{
	إذا (!ul_colors.disabled && ul_colors.has_colors)
		fputs(UL_COLOR_RESET, f);
}

/*
 * Get reset sequence
 */
ثابت حرف *color_get_disable_sequence(فراغ)
{
	إذا (!ul_colors.disabled && ul_colors.has_colors)
		أرجع UL_COLOR_RESET;
	وإلا
		أرجع "";
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

	إذا (!str || !*str)
		أرجع -EINVAL;

	أكد(ARRAY_ناپ(modes) == __UL_NCOLORMODES);

	لكل (i = 0; i < ARRAY_ناپ(modes); i++) {
		إذا (c_strcasecmp(str, modes[i]) == 0)
			أرجع i;
	}

	أرجع -EINVAL;
}

/*
 * Parses @str and exit(EXIT_FAILURE) on error
 */
صحيح colormode_or_err(ثابت حرف *str)
{
	ثابت حرف *p = str && *str == '=' ? str + 1 : str;
	صحيح colormode;

	colormode = colormode_from_string(p);
	إذا (colormode < 0)
		errx(EXIT_FAILURE, _("unsupported color mode: %s"), p);

	أرجع colormode;
}

#جے_تعریف_شدہ TEST_پروگرام_COLORS
# include <getopt.h>
صحيح مکھ(صحيح argc, حرف *argv[])
{
	ساكن ثابت بنية option longopts[] = {
		{ "mode",	required_argument, عدم, 'm' },
		{ "color",	required_argument, عدم, 'c' },
		{ "color-scheme", required_argument, عدم, 'C' },
		{ "name",	required_argument, عدم, 'n' },
		{ عدم, 0, عدم, 0 }
	};
	صحيح c, منوال = UL_COLORMODE_UNDEF;	/* default */
	ثابت حرف *color = "red", *name = عدم, *color_scheme = عدم;
	حرف *seq = عدم;

	طالما ((c = getopt_long(argc, argv, "C:c:m:n:", longopts, عدم)) != -1) {
		بدل (c) {
		حالة 'c':
			color = optarg;
			اكسر;
		حالة 'C':
			color_scheme = optarg;
			اكسر;
		حالة 'm':
			منوال = colormode_or_err(optarg);
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
			پروگرام_invocation_short_name);
			أرجع EXIT_FAILURE;
		}
	}

	colors_init(منوال, name ? name : پروگرام_invocation_short_name);

	إذا (color_is_sequence(color))
		seq = strdup(color);
	وإلا
		seq = color_get_sequence(color);

	إذا (color_scheme)
		color_scheme_enable(color_scheme, seq);
	وإلا
		color_enable(seq);
	لکھو("Hello World!");
	color_disable();
	fputc('\n', stdout);

	حرر(seq);

	أرجع EXIT_SUCCESS;
}
#جے_انت /* TEST_PROGRAM_COLORS */

