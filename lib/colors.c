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
# اگر defined(HAVE_NCURSESW_رکن_H)
#  include <ncursesw/term.h>
# elif defined(HAVE_NCURSES_رکن_H)
#  include <ncurses/term.h>
# elif defined(HAVE_رکن_H)
#  include <term.h>
# endif
#اگر_ختم

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
#اگر_تعریف_شدہ USE_COLORS_BY_DEFAULT
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_AUTO	/* check isatty() */
#ورنہ
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_NEVER	/* no colors by default */
#اگر_ختم

/*
 * terminal-colors.d debug stuff
 */
ساکن UL_DEBUG_DEFINE_نقاب(termcolors);
UL_DEBUG_DEFINE_MASKNAMES(termcolors) = UL_DEBUG_خالی_خانے_MASKNAMES;

#تعریف_کرو TERMCOLORS_DEBUG_INIT	(1 << 1)
#تعریف_کرو TERMCOLORS_DEBUG_CONF	(1 << 2)
#تعریف_کرو TERMCOLORS_DEBUG_SCHEME	(1 << 3)

#تعریف_کرو DBG(m, x)       __UL_DBG(termcolors, TERMCOLORS_DEBUG_, m, x)
#تعریف_کرو ON_DBG(m, x)    __UL_DBG_CALL(termcolors, TERMCOLORS_DEBUG_, m, x)

/*
 * terminal-colors.d file types
 */
شمار {
	UL_COLORFILE_DISABLE,		/* .disable */
	UL_COLORFILE_ENABLE,		/* .enable */
	UL_COLORFILE_SCHEME,		/* .scheme */

	__UL_COLORFILE_COUNT
};

ساخت ul_color_scheme {
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
ساخت ul_color_ctl {
	ثابت حرف	*utilname;	/* util name */
	ثابت حرف	*termname;	/* terminal name ($TERM) */

	حرف		*sfile;	/* path to scheme */

	ساخت ul_color_scheme	*schemes;	/* array with color schemes */
	size_t			nschemes;	/* number of the items */
	size_t			schemes_sz;	/* number of the allocated items */

	صحیح		منوال;		/* UL_COLORMODE_* */
	غیرعلامتی صحیح	has_colors : 1,	/* based on mode and scores[] */
			disabled   : 1, /* disable colors */
			cs_configured : 1, /* color schemes read */
			configured : 1; /* terminal-colors.d parsed */

	صحیح		scores[__UL_COLORFILE_COUNT];	/* the best match */
};

/*
 * Control struct, globally shared.
 */
ساکن ساخت ul_color_ctl ul_colors;

ساکن خالی colors_free_schemes(ساخت ul_color_ctl *cc);
ساکن صحیح colors_read_schemes(ساخت ul_color_ctl *cc);

/*
 * qsort/bsearch buddy
 */
ساکن صحیح cmp_scheme_name(ثابت خالی *a0, ثابت خالی *b0)
{
	ثابت ساخت ul_color_scheme *a = (ثابت ساخت ul_color_scheme *) a0,
				     *b = (ثابت ساخت ul_color_scheme *) b0;
	واپس strcmp(a->name, b->name);
}

/*
 * Resets control struct (note that we don't allocate the struct)
 */
ساکن خالی colors_reset(ساخت ul_color_ctl *cc)
{
	اگر (!cc)
		واپس;

	colors_free_schemes(cc);

	آزاد(cc->sfile);

	cc->sfile = تہی;
	cc->utilname = تہی;
	cc->termname = تہی;
	cc->منوال = UL_COLORMODE_UNDEF;

	memset(cc->scores, 0, حجم(cc->scores));
}

ساکن خالی colors_debug(ساخت ul_color_ctl *cc)
{
	size_t i;

	اگر (!cc)
		واپس;

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

	برائے (i = 0; i < ARRAY_سائز(cc->scores); i++)
		لکھو("\tscore %s = %d\n",
				i == UL_COLORFILE_DISABLE ? "disable" :
				i == UL_COLORFILE_ENABLE ? "enable" :
				i == UL_COLORFILE_SCHEME ? "scheme" : "???",
				cc->scores[i]);

	fputc('\n', stdout);

	برائے (i = 0; i < cc->nschemes; i++) {
		لکھو("\tscheme #%02zu ", i);
		color_scheme_enable(cc->schemes[i].name, تہی);
		fputs(cc->schemes[i].name, stdout);
		color_disable();
		fputc('\n', stdout);
	}
	fputc('\n', stdout);
}

/*
 * Parses [[<utilname>][@<termname>].]<type>
 */
ساکن صحیح filename_مقصد_علامات(ثابت حرف *str,
			      ثابت حرف **name, size_t *namesz,
			      ثابت حرف **رکن, size_t *termsz,
			      صحیح  *filetype)
{
	ثابت حرف *type_آغاز, *رکن_آغاز, *p;

	اگر (!str || !*str || *str == '.' || لڑی_لمبائی(str) > PATH_MAX)
		واپس -EINVAL;

	/* parse .type */
	p = strrchr(str, '.');
	type_آغاز = p ? p + 1 : str;

	اگر (strcmp(type_آغاز, "disable") == 0)
		*filetype = UL_COLORFILE_DISABLE;
	ورنہ اگر (strcmp(type_آغاز, "enable") == 0)
		*filetype = UL_COLORFILE_ENABLE;
	ورنہ اگر (strcmp(type_آغاز, "scheme") == 0)
		*filetype = UL_COLORFILE_SCHEME;
	ورنہ {
		DBG(CONF, ul_debug("unknown type '%s'", type_آغاز));
		واپس 1;				/* unknown type */
	}

	اگر (type_آغاز == str)
		واپس 0;				/* "type" only */

	/* parse @termname */
	p = strchr(str, '@');
	رکن_آغاز = p ? p + 1 : تہی;
	اگر (رکن_آغاز) {
		*رکن = رکن_آغاز;
		*termsz = type_آغاز - رکن_آغاز - 1;
		اگر (رکن_آغاز - 1 == str)
			واپس 0;			/* "@termname.type" */
	}

	/* parse utilname */
	p = رکن_آغاز ? رکن_آغاز : type_آغاز;
	*name = str;
	*namesz = p - str - 1;

	واپس 0;
}

/*
 * Scans @dirname and select the best matches for UL_COLORFILE_* types.
 * The result is stored to cc->scores. The path to the best "scheme"
 * file is stored to cc->scheme.
 */
ساکن صحیح colors_readdir(ساخت ul_color_ctl *cc, ثابت حرف *dirname)
{
	DIR *dir;
	صحیح rc = -ENOENT;
	ساخت dirent *d;
	حرف sfile[PATH_MAX] = { '\0' };
	size_t namesz, termsz;

	اگر (!dirname || !cc || !cc->utilname || !*cc->utilname)
		واپس -EINVAL;

	DBG(CONF, ul_debug("reading dir: '%s'", dirname));

	dir = opendir(dirname);
	اگر (!dir)
		واپس -errno;

	namesz = لڑی_لمبائی(cc->utilname);
	termsz = cc->termname ? لڑی_لمبائی(cc->termname) : 0;

	جب ((d = readdir(dir))) {
		صحیح type, score = 1;
		ثابت حرف *tk_name = تہی, *tk_رکن = تہی;
		size_t tk_namesz = 0, tk_termsz = 0;

		اگر (*d->d_name == '.')
			جاری;
#اگر_تعریف_شدہ _DIRENT_HAVE_D_TYPE
		اگر (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK &&
		    d->d_type != DT_REG)
			جاری;
#اگر_ختم
		اگر (filename_مقصد_علامات(d->d_name,
				       &tk_name, &tk_namesz,
				       &tk_رکن, &tk_termsz, &type) != 0)
			جاری;

		/* count theoretical score before we check names to avoid
		 * unnecessary strcmp() */
		اگر (tk_name)
			score += 20;
		اگر (tk_رکن)
			score += 10;

		DBG(CONF, ul_debug("item '%s': score=%d "
			"[cur: %d, name(%zu): %s, term(%zu): %s]",
			d->d_name, score, cc->scores[type],
			tk_namesz, tk_name,
			tk_termsz, tk_رکن));


		اگر (score < cc->scores[type])
			جاری;

		/* filter out by names */
		اگر (tk_namesz && (tk_namesz != namesz ||
				 لڑی_موازنہ(tk_name, cc->utilname, namesz) != 0))
			جاری;

		اگر (tk_termsz && (termsz == 0 || tk_termsz != termsz ||
				  لڑی_موازنہ(tk_رکن, cc->termname, termsz) != 0))
			جاری;

		DBG(CONF, ul_debug("setting '%s' from %d -to-> %d",
					type == UL_COLORFILE_SCHEME ? "scheme" :
					type == UL_COLORFILE_DISABLE ? "disable" :
					type == UL_COLORFILE_ENABLE ? "enable" : "???",
					cc->scores[type], score));
		cc->scores[type] = score;
		rc = 0;
		اگر (type == UL_COLORFILE_SCHEME)
			strncpy(sfile, d->d_name, حجم(sfile));
	}

	اگر (*sfile) {
		sfile[حجم(sfile) - 1] = '\0';
		اگر (asprintf(&cc->sfile, "%s/%s", dirname, sfile) <= 0)
			rc = -ENOMEM;
	}

	closedir(dir);
	واپس rc;
}

/* atexit() wrapper */
ساکن خالی colors_deinit(خالی)
{
	colors_reset(&ul_colors);
}

/*
 * Returns path to $XDG_CONFIG_HOME/terminal-colors.d
 */
ساکن حرف *colors_get_homedir(حرف *buf, size_t bufsz)
{
	حرف *p = getenv("XDG_CONFIG_HOME");

	اگر (p) {
		snprintf(buf, bufsz, "%s/" راستہ_TERMCOLORS_DIRNAME, p);
		واپس buf;
	}

	p = getenv("HOME");
	اگر (p) {
		snprintf(buf, bufsz, "%s/.config/" راستہ_TERMCOLORS_DIRNAME, p);
		واپس buf;
	}

	واپس تہی;
}

/*
 * Adds one color sequence to array with color scheme.
 */
ساکن صحیح colors_جوڑو_scheme(ساخت ul_color_ctl *cc,
			     ثابت حرف *name,
			     ثابت حرف *seq0)
{
	ساخت ul_color_scheme *cs = تہی;
	حرف *seq = تہی;
	صحیح rc;

	اگر (!cc || !name || !*name || !seq0 || !*seq0)
		واپس -EINVAL;

	DBG(SCHEME, ul_debug("add '%s'", name));

	seq = color_get_sequence(seq0);
	اگر (!seq)
		واپس -EINVAL;
	rc = -ENOMEM;

	/* enlarge the array */
	اگر (cc->nschemes == cc->schemes_sz) {
		خالی *tmp = reallocarray(cc->schemes, cc->nschemes + 10,
					 حجم(ساخت ul_color_scheme));
		اگر (!tmp)
			جاؤ err;
		cc->schemes = tmp;
		cc->schemes_sz = cc->nschemes + 10;
	}

	/* add a new item */
	cs = &cc->schemes[cc->nschemes];
	cs->seq = seq;
	cs->name = strdup(name);
	اگر (!cs->name)
		جاؤ err;

	cc->nschemes++;
	واپس 0;
err:
	اگر (cs) {
		آزاد(cs->seq);
		آزاد(cs->name);
		cs->seq = cs->name = تہی;
	} ورنہ
		آزاد(seq);
	واپس rc;
}

/*
 * Deallocates all regards to color schemes
 */
ساکن خالی colors_free_schemes(ساخت ul_color_ctl *cc)
{
	size_t i;

	DBG(SCHEME, ul_debug("free scheme"));

	برائے (i = 0; i < cc->nschemes; i++) {
		آزاد(cc->schemes[i].name);
		آزاد(cc->schemes[i].seq);
	}

	آزاد(cc->schemes);
	cc->schemes = تہی;
	cc->nschemes = 0;
	cc->schemes_sz = 0;
}

/*
 * The scheme configuration has to be sorted for bsearch
 */
ساکن خالی colors_ترتیب_دو_schemes(ساخت ul_color_ctl *cc)
{
	اگر (!cc->nschemes)
		واپس;

	DBG(SCHEME, ul_debug("sort scheme"));

	qsort(cc->schemes, cc->nschemes,
	      حجم(ساخت ul_color_scheme), cmp_scheme_name);
}

/*
 * Returns just one color scheme
 */
ساکن ساخت ul_color_scheme *colors_get_scheme(ساخت ul_color_ctl *cc,
						 ثابت حرف *name)
{
	ساخت ul_color_scheme کلید = { .name = (حرف *) name}, *res;

	اگر (!cc || !name || !*name)
		واپس تہی;

	اگر (!cc->cs_configured) {
		صحیح rc = colors_read_schemes(cc);
		اگر (rc)
			واپس تہی;
	}
	اگر (!cc->nschemes)
		واپس تہی;

	DBG(SCHEME, ul_debug("search '%s'", name));

	res = bsearch(&کلید, cc->schemes, cc->nschemes,
				حجم(ساخت ul_color_scheme),
				cmp_scheme_name);

	واپس res && res->seq ? res  : تہی;
}

/*
 * Parses filenames in terminal-colors.d
 */
ساکن صحیح colors_read_configuration(ساخت ul_color_ctl *cc)
{
	صحیح rc = -ENOENT;
	حرف *dirname, buf[PATH_MAX];

	cc->termname = getenv("TERM");

	dirname = colors_get_homedir(buf, حجم(buf));
	اگر (dirname)
		rc = colors_readdir(cc, dirname);		/* ~/.config */
	اگر (rc == -EPERM || rc == -EACCES || rc == -ENOENT)
		rc = colors_readdir(cc, راستہ_TERMCOLORS_DIR);	/* /etc */

	cc->configured = 1;
	واپس rc;
}

/*
 * Reads terminal-colors.d/ scheme file into array schemes
 */
ساکن صحیح colors_read_schemes(ساخت ul_color_ctl *cc)
{
	صحیح rc = 0;
	FILE *f = تہی;
	حرف buf[BUFSIZ],
	     cn[129], seq[129];

	اگر (!cc->configured)
		rc = colors_read_configuration(cc);

	cc->cs_configured = 1;

	اگر (rc || !cc->sfile)
		جاؤ مکمل;

	DBG(SCHEME, ul_debug("reading file '%s'", cc->sfile));

	f = fopen(cc->sfile, "r" UL_CLOEXECSTR);
	اگر (!f) {
		rc = -errno;
		جاؤ مکمل;
	}

	جب (fgets(buf, حجم(buf), f)) {
		حرف *p = strchr(buf, '\n');

		اگر (!p) {
			اگر (feof(f))
				p = strchr(buf, '\0');
			ورنہ {
				rc = -errno;
				جاؤ مکمل;
			}
		}
		*p = '\0';
		p = (حرف *) skip_blank(buf);
		اگر (*p == '\0' || *p == '#')
			جاری;

		rc = لڑی_سے_پڑھو(p, "%128[^ ] %128[^\n ]", cn, seq);
		اگر (rc == 2 && *cn && *seq) {
			rc = colors_جوڑو_scheme(cc, cn, seq);	/* set rc=0 on success */
			اگر (rc)
				جاؤ مکمل;
		}
	}
	rc = 0;

مکمل:
	اگر (f)
		fclose(f);
	colors_ترتیب_دو_schemes(cc);

	واپس rc;
}


ساکن خالی termcolors_init_debug(خالی)
{
	__UL_INIT_DEBUG_FROM_ENV(termcolors, TERMCOLORS_DEBUG_, 0, TERMINAL_COLORS_DEBUG);
}

ساکن صحیح colors_terminal_is_ready(خالی)
{
	صحیح ncolors = -1;

#اگر defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
	{
		صحیح ret;

		/* setupterm() allocates memory, del_curterm() deallocates it */
		اگر (setupterm(تہی, STDOUT_FILENO, &ret) == 0 && ret == 1) {
			ncolors = tigetnum("colors");
			del_curterm(cur_رکن);
		}
	}
#اگر_ختم
	اگر (1 < ncolors) {
		DBG(CONF, ul_debug("terminal is ready (supports %d colors)", ncolors));
		واپس 1;
	}

	DBG(CONF, ul_debug("terminal is NOT ready (no colors)"));
	واپس 0;
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
صحیح colors_init(صحیح منوال, ثابت حرف *name)
{
	صحیح ready = -1;
	ساخت ul_color_ctl *cc = &ul_colors;

	cc->utilname = name;

	termcolors_init_debug();

	اگر (منوال != UL_COLORMODE_ALWAYS && !isatty(STDOUT_FILENO))
		cc->منوال = UL_COLORMODE_NEVER;
	ورنہ
		cc->منوال = منوال;

	اگر (cc->منوال == UL_COLORMODE_UNDEF
	    && getenv("NO_COLOR") == تہی
	    && (ready = colors_terminal_is_ready())) {

		صحیح rc = colors_read_configuration(cc);
		اگر (rc)
			cc->منوال = UL_COLORMODE_DEFAULT;
		ورنہ {

			/* evaluate scores */
			اگر (cc->scores[UL_COLORFILE_DISABLE] >
			    cc->scores[UL_COLORFILE_ENABLE])
				cc->منوال = UL_COLORMODE_NEVER;
			ورنہ
				cc->منوال = UL_COLORMODE_DEFAULT;

			atexit(colors_deinit);
		}
	}

	انتخاب (cc->منوال) {
	حالت UL_COLORMODE_AUTO:
		cc->has_colors = ready == -1 ? colors_terminal_is_ready() : ready;
		توڑو;
	حالت UL_COLORMODE_ALWAYS:
		cc->has_colors = 1;
		توڑو;
	حالت UL_COLORMODE_NEVER:
	طےشدہ:
		cc->has_colors = 0;
	}

	ON_DBG(CONF, colors_debug(cc));

	واپس cc->has_colors;
}

/*
 * Temporary disable colors (this setting is independent on terminal-colors.d/)
 */
خالی colors_off(خالی)
{
	ul_colors.disabled = 1;
}

/*
 * Enable colors
 */
خالی colors_on(خالی)
{
	ul_colors.disabled = 0;
}

/*
 * Is terminal-colors.d/ configured to use colors?
 */
صحیح colors_wanted(خالی)
{
	واپس ul_colors.has_colors;
}

/*
 * Returns mode
 */
صحیح colors_منوال(خالی)
{
	واپس ul_colors.منوال;
}

/*
 * Enable @seq color
 */
خالی color_fenable(ثابت حرف *seq, FILE *f)
{
	اگر (!ul_colors.disabled && ul_colors.has_colors && seq)
		fputs(seq, f);
}

/*
 * Returns escape sequence by logical @name, if undefined then returns @dflt.
 */
ثابت حرف *color_scheme_get_sequence(ثابت حرف *name, ثابت حرف *dflt)
{
	ساخت ul_color_scheme *cs;

	اگر (ul_colors.disabled || !ul_colors.has_colors)
		واپس تہی;

	cs = colors_get_scheme(&ul_colors, name);
	واپس cs && cs->seq ? cs->seq : dflt;
}

/*
 * Enable color by logical @name, if undefined enable @dflt.
 */
خالی color_scheme_fenable(ثابت حرف *name, ثابت حرف *dflt, FILE *f)
{
	ثابت حرف *seq = color_scheme_get_sequence(name, dflt);

	اگر (!seq)
		واپس;
	color_fenable(seq, f);
}


/*
 * Disable previously enabled color
 */
خالی color_fdisable(FILE *f)
{
	اگر (!ul_colors.disabled && ul_colors.has_colors)
		fputs(UL_COLOR_RESET, f);
}

/*
 * Get reset sequence
 */
ثابت حرف *color_get_disable_sequence(خالی)
{
	اگر (!ul_colors.disabled && ul_colors.has_colors)
		واپس UL_COLOR_RESET;
	ورنہ
		واپس "";
}

/*
 * Parses @str to return UL_COLORMODE_*
 */
صحیح colormode_from_string(ثابت حرف *str)
{
	size_t i;
	ساکن ثابت حرف *ثابت modes[] = {
		[UL_COLORMODE_AUTO]   = "auto",
		[UL_COLORMODE_NEVER]  = "never",
		[UL_COLORMODE_ALWAYS] = "always",
		[UL_COLORMODE_UNDEF] = ""
	};

	اگر (!str || !*str)
		واپس -EINVAL;

	أكد(ARRAY_سائز(modes) == __UL_NCOLORMODES);

	برائے (i = 0; i < ARRAY_سائز(modes); i++) {
		اگر (c_strcasecmp(str, modes[i]) == 0)
			واپس i;
	}

	واپس -EINVAL;
}

/*
 * Parses @str and exit(EXIT_FAILURE) on error
 */
صحیح colormode_or_err(ثابت حرف *str)
{
	ثابت حرف *p = str && *str == '=' ? str + 1 : str;
	صحیح colormode;

	colormode = colormode_from_string(p);
	اگر (colormode < 0)
		errx(EXIT_FAILURE, _("unsupported color mode: %s"), p);

	واپس colormode;
}

#اگر_تعریف_شدہ TEST_پروگرام_COLORS
# include <getopt.h>
صحیح مین(صحیح argc, حرف *argv[])
{
	ساکن ثابت ساخت option longopts[] = {
		{ "mode",	required_argument, تہی, 'm' },
		{ "color",	required_argument, تہی, 'c' },
		{ "color-scheme", required_argument, تہی, 'C' },
		{ "name",	required_argument, تہی, 'n' },
		{ تہی, 0, تہی, 0 }
	};
	صحیح c, منوال = UL_COLORMODE_UNDEF;	/* default */
	ثابت حرف *color = "red", *name = تہی, *color_scheme = تہی;
	حرف *seq = تہی;

	جب ((c = getopt_long(argc, argv, "C:c:m:n:", longopts, تہی)) != -1) {
		انتخاب (c) {
		حالت 'c':
			color = optarg;
			توڑو;
		حالت 'C':
			color_scheme = optarg;
			توڑو;
		حالت 'm':
			منوال = colormode_or_err(optarg);
			توڑو;
		حالت 'n':
			name = optarg;
			توڑو;
		طےشدہ:
			fprintf(stderr, "usage: %s [options]\n"
			" -m, --mode <auto|never|always>  default is undefined\n"
			" -c, --color <red|blue|...>      color for the test message\n"
			" -C, --color-scheme <name>       color for the test message\n"
			" -n, --name <utilname>           util name\n",
			پروگرام_invocation_short_name);
			واپس EXIT_FAILURE;
		}
	}

	colors_init(منوال, name ? name : پروگرام_invocation_short_name);

	اگر (color_is_sequence(color))
		seq = strdup(color);
	ورنہ
		seq = color_get_sequence(color);

	اگر (color_scheme)
		color_scheme_enable(color_scheme, seq);
	ورنہ
		color_enable(seq);
	لکھو("Hello World!");
	color_disable();
	fputc('\n', stdout);

	آزاد(seq);

	واپس EXIT_SUCCESS;
}
#اگر_ختم /* TEST_PROGRAM_COLORS */

