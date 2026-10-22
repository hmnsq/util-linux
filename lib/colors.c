/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: 2012 Ondrej Oprala <ooprala@redhat.com>
 *          2012-2025 Karel Zak <kzak@redhat.com>
 */
#अन्तर्भाव <assert.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <dirent.h>
#अन्तर्भाव <ctype.h>

#यदि defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
# यदि defined(HAVE_NCURSESW_NCURSES_H)
#  include <ncursesw/ncurses.h>
# elif defined(HAVE_NCURSES_NCURSES_H)
#  include <ncurses/ncurses.h>
# elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
# endif
# यदि defined(HAVE_NCURSESW_पदांशः_H)
#  include <ncursesw/term.h>
# elif defined(HAVE_NCURSES_पदांशः_H)
#  include <ncurses/term.h>
# elif defined(HAVE_पदांशः_H)
#  include <term.h>
# endif
#यदि_अन्तः

#अन्तर्भाव "c.h"
#अन्तर्भाव "cctype.h"
#अन्तर्भाव "colors.h"
#अन्तर्भाव "nls.h"
#अन्तर्भाव "pathnames.h"
#अन्तर्भाव "strutils.h"

#अन्तर्भाव "debug.h"

/*
 * Default behavior, may be overridden by terminal-colors.d/{enable,disable}.
 */
#यदि_परिभाषितम् USE_COLORS_BY_DEFAULT
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_AUTO	/* check isatty() */
#अन्यथा
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_NEVER	/* no colors by default */
#यदि_अन्तः

/*
 * terminal-colors.d debug stuff
 */
स्थिर UL_DEBUG_DEFINE_आवरणम्(termcolors);
UL_DEBUG_DEFINE_MASKNAMES(termcolors) = UL_DEBUG_रिक्तानि_MASKNAMES;

#परिभाषय TERMCOLORS_DEBUG_INIT	(1 << 1)
#परिभाषय TERMCOLORS_DEBUG_CONF	(1 << 2)
#परिभाषय TERMCOLORS_DEBUG_SCHEME	(1 << 3)

#परिभाषय DBG(m, x)       __UL_DBG(termcolors, TERMCOLORS_DEBUG_, m, x)
#परिभाषय ON_DBG(m, x)    __UL_DBG_CALL(termcolors, TERMCOLORS_DEBUG_, m, x)

/*
 * terminal-colors.d file types
 */
गणना {
	UL_COLORFILE_DISABLE,		/* .disable */
	UL_COLORFILE_ENABLE,		/* .enable */
	UL_COLORFILE_SCHEME,		/* .scheme */

	__UL_COLORFILE_COUNT
};

संरचना ul_color_scheme {
	वर्ण *name;
	वर्ण *seq;
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
संरचना ul_color_ctl {
	ध्रुव वर्ण	*utilname;	/* util name */
	ध्रुव वर्ण	*termname;	/* terminal name ($TERM) */

	वर्ण		*sfile;	/* path to scheme */

	संरचना ul_color_scheme	*schemes;	/* array with color schemes */
	size_t			nschemes;	/* number of the items */
	size_t			schemes_sz;	/* number of the allocated items */

	पूर्णाङ्क		बहुलकम्;		/* UL_COLORMODE_* */
	अचिह्नित पूर्णाङ्क	has_colors : 1,	/* based on mode and scores[] */
			disabled   : 1, /* disable colors */
			cs_configured : 1, /* color schemes read */
			configured : 1; /* terminal-colors.d parsed */

	पूर्णाङ्क		scores[__UL_COLORFILE_COUNT];	/* the best match */
};

/*
 * Control struct, globally shared.
 */
स्थिर संरचना ul_color_ctl ul_colors;

स्थिर शून्य colors_free_schemes(संरचना ul_color_ctl *cc);
स्थिर पूर्णाङ्क colors_read_schemes(संरचना ul_color_ctl *cc);

/*
 * qsort/bsearch buddy
 */
स्थिर पूर्णाङ्क cmp_scheme_name(ध्रुव शून्य *a0, ध्रुव शून्य *b0)
{
	ध्रुव संरचना ul_color_scheme *a = (ध्रुव संरचना ul_color_scheme *) a0,
				     *b = (ध्रुव संरचना ul_color_scheme *) b0;
	प्रत्यागम वाक्यं_तुलय(a->name, b->name);
}

/*
 * Resets control struct (note that we don't allocate the struct)
 */
स्थिर शून्य colors_reset(संरचना ul_color_ctl *cc)
{
	यदि (!cc)
		प्रत्यागम;

	colors_free_schemes(cc);

	मुक्त(cc->sfile);

	cc->sfile = रिक्त;
	cc->utilname = रिक्त;
	cc->termname = रिक्त;
	cc->बहुलकम् = UL_COLORMODE_UNDEF;

	स्मृतिं_पूरय(cc->scores, 0, परिमाण(cc->scores));
}

स्थिर शून्य colors_debug(संरचना ul_color_ctl *cc)
{
	size_t i;

	यदि (!cc)
		प्रत्यागम;

	प्रारूप_लिख("Colors:\n");
	प्रारूप_लिख("\tutilname = '%s'\n", cc->utilname);
	प्रारूप_लिख("\ttermname = '%s'\n", cc->termname);
	प्रारूप_लिख("\tscheme file = '%s'\n", cc->sfile);
	प्रारूप_लिख("\tmode = %s\n",
			cc->बहुलकम् == UL_COLORMODE_UNDEF ? "undefined" :
			cc->बहुलकम् == UL_COLORMODE_AUTO ?  "auto" :
			cc->बहुलकम् == UL_COLORMODE_NEVER ? "never" :
			cc->बहुलकम् == UL_COLORMODE_ALWAYS ? "always" : "???");
	प्रारूप_लिख("\thas_colors = %d\n", cc->has_colors);
	प्रारूप_लिख("\tdisabled = %d\n", cc->disabled);
	प्रारूप_लिख("\tconfigured = %d\n", cc->configured);
	प्रारूप_लिख("\tcs configured = %d\n", cc->cs_configured);

	fputc('\n', stdout);

	प्रति (i = 0; i < ARRAY_आकार(cc->scores); i++)
		प्रारूप_लिख("\tscore %s = %d\n",
				i == UL_COLORFILE_DISABLE ? "disable" :
				i == UL_COLORFILE_ENABLE ? "enable" :
				i == UL_COLORFILE_SCHEME ? "scheme" : "???",
				cc->scores[i]);

	fputc('\n', stdout);

	प्रति (i = 0; i < cc->nschemes; i++) {
		प्रारूप_लिख("\tscheme #%02zu ", i);
		color_scheme_enable(cc->schemes[i].name, रिक्त);
		fputs(cc->schemes[i].name, stdout);
		color_disable();
		fputc('\n', stdout);
	}
	fputc('\n', stdout);
}

/*
 * Parses [[<utilname>][@<termname>].]<type>
 */
स्थिर पूर्णाङ्क filename_गन्तव्यम्_tokens(ध्रुव वर्ण *str,
			      ध्रुव वर्ण **name, size_t *namesz,
			      ध्रुव वर्ण **पदांशः, size_t *termsz,
			      पूर्णाङ्क  *filetype)
{
	ध्रुव वर्ण *type_start, *पदांशः_start, *p;

	यदि (!str || !*str || *str == '.' || वाक्यदैर्घ्यम्(str) > PATH_MAX)
		प्रत्यागम -EINVAL;

	/* parse .type */
	p = strrchr(str, '.');
	type_start = p ? p + 1 : str;

	यदि (वाक्यं_तुलय(type_start, "disable") == 0)
		*filetype = UL_COLORFILE_DISABLE;
	अन्यथा यदि (वाक्यं_तुलय(type_start, "enable") == 0)
		*filetype = UL_COLORFILE_ENABLE;
	अन्यथा यदि (वाक्यं_तुलय(type_start, "scheme") == 0)
		*filetype = UL_COLORFILE_SCHEME;
	अन्यथा {
		DBG(CONF, ul_debug("unknown type '%s'", type_start));
		प्रत्यागम 1;				/* unknown type */
	}

	यदि (type_start == str)
		प्रत्यागम 0;				/* "type" only */

	/* parse @termname */
	p = strchr(str, '@');
	पदांशः_start = p ? p + 1 : रिक्त;
	यदि (पदांशः_start) {
		*पदांशः = पदांशः_start;
		*termsz = type_start - पदांशः_start - 1;
		यदि (पदांशः_start - 1 == str)
			प्रत्यागम 0;			/* "@termname.type" */
	}

	/* parse utilname */
	p = पदांशः_start ? पदांशः_start : type_start;
	*name = str;
	*namesz = p - str - 1;

	प्रत्यागम 0;
}

/*
 * Scans @dirname and select the best matches for UL_COLORFILE_* types.
 * The result is stored to cc->scores. The path to the best "scheme"
 * file is stored to cc->scheme.
 */
स्थिर पूर्णाङ्क colors_readdir(संरचना ul_color_ctl *cc, ध्रुव वर्ण *dirname)
{
	DIR *dir;
	पूर्णाङ्क rc = -ENOENT;
	संरचना dirent *d;
	वर्ण sfile[PATH_MAX] = { '\0' };
	size_t namesz, termsz;

	यदि (!dirname || !cc || !cc->utilname || !*cc->utilname)
		प्रत्यागम -EINVAL;

	DBG(CONF, ul_debug("reading dir: '%s'", dirname));

	dir = opendir(dirname);
	यदि (!dir)
		प्रत्यागम -errno;

	namesz = वाक्यदैर्घ्यम्(cc->utilname);
	termsz = cc->termname ? वाक्यदैर्घ्यम्(cc->termname) : 0;

	यावत् ((d = readdir(dir))) {
		पूर्णाङ्क type, score = 1;
		ध्रुव वर्ण *tk_name = रिक्त, *tk_पदांशः = रिक्त;
		size_t tk_namesz = 0, tk_termsz = 0;

		यदि (*d->d_name == '.')
			अनुवर्त;
#यदि_परिभाषितम् _DIRENT_HAVE_D_TYPE
		यदि (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK &&
		    d->d_type != DT_REG)
			अनुवर्त;
#यदि_अन्तः
		यदि (filename_गन्तव्यम्_tokens(d->d_name,
				       &tk_name, &tk_namesz,
				       &tk_पदांशः, &tk_termsz, &type) != 0)
			अनुवर्त;

		/* count theoretical score before we check names to avoid
		 * unnecessary strcmp() */
		यदि (tk_name)
			score += 20;
		यदि (tk_पदांशः)
			score += 10;

		DBG(CONF, ul_debug("item '%s': score=%d "
			"[cur: %d, name(%zu): %s, term(%zu): %s]",
			d->d_name, score, cc->scores[type],
			tk_namesz, tk_name,
			tk_termsz, tk_पदांशः));


		यदि (score < cc->scores[type])
			अनुवर्त;

		/* filter out by names */
		यदि (tk_namesz && (tk_namesz != namesz ||
				 वाक्ये_तुलय(tk_name, cc->utilname, namesz) != 0))
			अनुवर्त;

		यदि (tk_termsz && (termsz == 0 || tk_termsz != termsz ||
				  वाक्ये_तुलय(tk_पदांशः, cc->termname, termsz) != 0))
			अनुवर्त;

		DBG(CONF, ul_debug("setting '%s' from %d -to-> %d",
					type == UL_COLORFILE_SCHEME ? "scheme" :
					type == UL_COLORFILE_DISABLE ? "disable" :
					type == UL_COLORFILE_ENABLE ? "enable" : "???",
					cc->scores[type], score));
		cc->scores[type] = score;
		rc = 0;
		यदि (type == UL_COLORFILE_SCHEME)
			strncpy(sfile, d->d_name, परिमाण(sfile));
	}

	यदि (*sfile) {
		sfile[परिमाण(sfile) - 1] = '\0';
		यदि (asprintf(&cc->sfile, "%s/%s", dirname, sfile) <= 0)
			rc = -ENOMEM;
	}

	closedir(dir);
	प्रत्यागम rc;
}

/* atexit() wrapper */
स्थिर शून्य colors_deinit(शून्य)
{
	colors_reset(&ul_colors);
}

/*
 * Returns path to $XDG_CONFIG_HOME/terminal-colors.d
 */
स्थिर वर्ण *colors_get_homedir(वर्ण *buf, size_t bufsz)
{
	वर्ण *p = getenv("XDG_CONFIG_HOME");

	यदि (p) {
		snprintf(buf, bufsz, "%s/" मार्गः_TERMCOLORS_DIRNAME, p);
		प्रत्यागम buf;
	}

	p = getenv("HOME");
	यदि (p) {
		snprintf(buf, bufsz, "%s/.config/" मार्गः_TERMCOLORS_DIRNAME, p);
		प्रत्यागम buf;
	}

	प्रत्यागम रिक्त;
}

/*
 * Adds one color sequence to array with color scheme.
 */
स्थिर पूर्णाङ्क colors_सङ्कलय_scheme(संरचना ul_color_ctl *cc,
			     ध्रुव वर्ण *name,
			     ध्रुव वर्ण *seq0)
{
	संरचना ul_color_scheme *cs = रिक्त;
	वर्ण *seq = रिक्त;
	पूर्णाङ्क rc;

	यदि (!cc || !name || !*name || !seq0 || !*seq0)
		प्रत्यागम -EINVAL;

	DBG(SCHEME, ul_debug("add '%s'", name));

	seq = color_get_sequence(seq0);
	यदि (!seq)
		प्रत्यागम -EINVAL;
	rc = -ENOMEM;

	/* enlarge the array */
	यदि (cc->nschemes == cc->schemes_sz) {
		शून्य *tmp = reallocarray(cc->schemes, cc->nschemes + 10,
					 परिमाण(संरचना ul_color_scheme));
		यदि (!tmp)
			गच्छ err;
		cc->schemes = tmp;
		cc->schemes_sz = cc->nschemes + 10;
	}

	/* add a new item */
	cs = &cc->schemes[cc->nschemes];
	cs->seq = seq;
	cs->name = strdup(name);
	यदि (!cs->name)
		गच्छ err;

	cc->nschemes++;
	प्रत्यागम 0;
err:
	यदि (cs) {
		मुक्त(cs->seq);
		मुक्त(cs->name);
		cs->seq = cs->name = रिक्त;
	} अन्यथा
		मुक्त(seq);
	प्रत्यागम rc;
}

/*
 * Deallocates all regards to color schemes
 */
स्थिर शून्य colors_free_schemes(संरचना ul_color_ctl *cc)
{
	size_t i;

	DBG(SCHEME, ul_debug("free scheme"));

	प्रति (i = 0; i < cc->nschemes; i++) {
		मुक्त(cc->schemes[i].name);
		मुक्त(cc->schemes[i].seq);
	}

	मुक्त(cc->schemes);
	cc->schemes = रिक्त;
	cc->nschemes = 0;
	cc->schemes_sz = 0;
}

/*
 * The scheme configuration has to be sorted for bsearch
 */
स्थिर शून्य colors_क्रमय_schemes(संरचना ul_color_ctl *cc)
{
	यदि (!cc->nschemes)
		प्रत्यागम;

	DBG(SCHEME, ul_debug("sort scheme"));

	शीघ्रं_क्रमय(cc->schemes, cc->nschemes,
	      परिमाण(संरचना ul_color_scheme), cmp_scheme_name);
}

/*
 * Returns just one color scheme
 */
स्थिर संरचना ul_color_scheme *colors_get_scheme(संरचना ul_color_ctl *cc,
						 ध्रुव वर्ण *name)
{
	संरचना ul_color_scheme कुञ्चिका = { .name = (वर्ण *) name}, *res;

	यदि (!cc || !name || !*name)
		प्रत्यागम रिक्त;

	यदि (!cc->cs_configured) {
		पूर्णाङ्क rc = colors_read_schemes(cc);
		यदि (rc)
			प्रत्यागम रिक्त;
	}
	यदि (!cc->nschemes)
		प्रत्यागम रिक्त;

	DBG(SCHEME, ul_debug("search '%s'", name));

	res = bsearch(&कुञ्चिका, cc->schemes, cc->nschemes,
				परिमाण(संरचना ul_color_scheme),
				cmp_scheme_name);

	प्रत्यागम res && res->seq ? res  : रिक्त;
}

/*
 * Parses filenames in terminal-colors.d
 */
स्थिर पूर्णाङ्क colors_read_configuration(संरचना ul_color_ctl *cc)
{
	पूर्णाङ्क rc = -ENOENT;
	वर्ण *dirname, buf[PATH_MAX];

	cc->termname = getenv("TERM");

	dirname = colors_get_homedir(buf, परिमाण(buf));
	यदि (dirname)
		rc = colors_readdir(cc, dirname);		/* ~/.config */
	यदि (rc == -EPERM || rc == -EACCES || rc == -ENOENT)
		rc = colors_readdir(cc, मार्गः_TERMCOLORS_DIR);	/* /etc */

	cc->configured = 1;
	प्रत्यागम rc;
}

/*
 * Reads terminal-colors.d/ scheme file into array schemes
 */
स्थिर पूर्णाङ्क colors_read_schemes(संरचना ul_color_ctl *cc)
{
	पूर्णाङ्क rc = 0;
	FILE *f = रिक्त;
	वर्ण buf[BUFSIZ],
	     cn[129], seq[129];

	यदि (!cc->configured)
		rc = colors_read_configuration(cc);

	cc->cs_configured = 1;

	यदि (rc || !cc->sfile)
		गच्छ समाप्तम्;

	DBG(SCHEME, ul_debug("reading file '%s'", cc->sfile));

	f = fopen(cc->sfile, "r" UL_CLOEXECSTR);
	यदि (!f) {
		rc = -errno;
		गच्छ समाप्तम्;
	}

	यावत् (fgets(buf, परिमाण(buf), f)) {
		वर्ण *p = strchr(buf, '\n');

		यदि (!p) {
			यदि (feof(f))
				p = strchr(buf, '\0');
			अन्यथा {
				rc = -errno;
				गच्छ समाप्तम्;
			}
		}
		*p = '\0';
		p = (वर्ण *) skip_blank(buf);
		यदि (*p == '\0' || *p == '#')
			अनुवर्त;

		rc = वाक्यात्_पठ(p, "%128[^ ] %128[^\n ]", cn, seq);
		यदि (rc == 2 && *cn && *seq) {
			rc = colors_सङ्कलय_scheme(cc, cn, seq);	/* set rc=0 on success */
			यदि (rc)
				गच्छ समाप्तम्;
		}
	}
	rc = 0;

समाप्तम्:
	यदि (f)
		fclose(f);
	colors_क्रमय_schemes(cc);

	प्रत्यागम rc;
}


स्थिर शून्य termcolors_init_debug(शून्य)
{
	__UL_INIT_DEBUG_FROM_ENV(termcolors, TERMCOLORS_DEBUG_, 0, TERMINAL_COLORS_DEBUG);
}

स्थिर पूर्णाङ्क colors_terminal_is_ready(शून्य)
{
	पूर्णाङ्क ncolors = -1;

#यदि defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
	{
		पूर्णाङ्क ret;

		/* setupterm() allocates memory, del_curterm() deallocates it */
		यदि (setupterm(रिक्त, STDOUT_FILENO, &ret) == 0 && ret == 1) {
			ncolors = tigetnum("colors");
			del_curterm(cur_पदांशः);
		}
	}
#यदि_अन्तः
	यदि (1 < ncolors) {
		DBG(CONF, ul_debug("terminal is ready (supports %d colors)", ncolors));
		प्रत्यागम 1;
	}

	DBG(CONF, ul_debug("terminal is NOT ready (no colors)"));
	प्रत्यागम 0;
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
पूर्णाङ्क colors_init(पूर्णाङ्क बहुलकम्, ध्रुव वर्ण *name)
{
	पूर्णाङ्क ready = -1;
	संरचना ul_color_ctl *cc = &ul_colors;

	cc->utilname = name;

	termcolors_init_debug();

	यदि (बहुलकम् != UL_COLORMODE_ALWAYS && !isatty(STDOUT_FILENO))
		cc->बहुलकम् = UL_COLORMODE_NEVER;
	अन्यथा
		cc->बहुलकम् = बहुलकम्;

	यदि (cc->बहुलकम् == UL_COLORMODE_UNDEF
	    && getenv("NO_COLOR") == रिक्त
	    && (ready = colors_terminal_is_ready())) {

		पूर्णाङ्क rc = colors_read_configuration(cc);
		यदि (rc)
			cc->बहुलकम् = UL_COLORMODE_DEFAULT;
		अन्यथा {

			/* evaluate scores */
			यदि (cc->scores[UL_COLORFILE_DISABLE] >
			    cc->scores[UL_COLORFILE_ENABLE])
				cc->बहुलकम् = UL_COLORMODE_NEVER;
			अन्यथा
				cc->बहुलकम् = UL_COLORMODE_DEFAULT;

			atexit(colors_deinit);
		}
	}

	विकल्प (cc->बहुलकम्) {
	पक्ष UL_COLORMODE_AUTO:
		cc->has_colors = ready == -1 ? colors_terminal_is_ready() : ready;
		भङ्ग;
	पक्ष UL_COLORMODE_ALWAYS:
		cc->has_colors = 1;
		भङ्ग;
	पक्ष UL_COLORMODE_NEVER:
	स्वभाव:
		cc->has_colors = 0;
	}

	ON_DBG(CONF, colors_debug(cc));

	प्रत्यागम cc->has_colors;
}

/*
 * Temporary disable colors (this setting is independent on terminal-colors.d/)
 */
शून्य colors_off(शून्य)
{
	ul_colors.disabled = 1;
}

/*
 * Enable colors
 */
शून्य colors_on(शून्य)
{
	ul_colors.disabled = 0;
}

/*
 * Is terminal-colors.d/ configured to use colors?
 */
पूर्णाङ्क colors_wanted(शून्य)
{
	प्रत्यागम ul_colors.has_colors;
}

/*
 * Returns mode
 */
पूर्णाङ्क colors_बहुलकम्(शून्य)
{
	प्रत्यागम ul_colors.बहुलकम्;
}

/*
 * Enable @seq color
 */
शून्य color_fenable(ध्रुव वर्ण *seq, FILE *f)
{
	यदि (!ul_colors.disabled && ul_colors.has_colors && seq)
		fputs(seq, f);
}

/*
 * Returns escape sequence by logical @name, if undefined then returns @dflt.
 */
ध्रुव वर्ण *color_scheme_get_sequence(ध्रुव वर्ण *name, ध्रुव वर्ण *dflt)
{
	संरचना ul_color_scheme *cs;

	यदि (ul_colors.disabled || !ul_colors.has_colors)
		प्रत्यागम रिक्त;

	cs = colors_get_scheme(&ul_colors, name);
	प्रत्यागम cs && cs->seq ? cs->seq : dflt;
}

/*
 * Enable color by logical @name, if undefined enable @dflt.
 */
शून्य color_scheme_fenable(ध्रुव वर्ण *name, ध्रुव वर्ण *dflt, FILE *f)
{
	ध्रुव वर्ण *seq = color_scheme_get_sequence(name, dflt);

	यदि (!seq)
		प्रत्यागम;
	color_fenable(seq, f);
}


/*
 * Disable previously enabled color
 */
शून्य color_fdisable(FILE *f)
{
	यदि (!ul_colors.disabled && ul_colors.has_colors)
		fputs(UL_COLOR_RESET, f);
}

/*
 * Get reset sequence
 */
ध्रुव वर्ण *color_get_disable_sequence(शून्य)
{
	यदि (!ul_colors.disabled && ul_colors.has_colors)
		प्रत्यागम UL_COLOR_RESET;
	अन्यथा
		प्रत्यागम "";
}

/*
 * Parses @str to return UL_COLORMODE_*
 */
पूर्णाङ्क colormode_from_string(ध्रुव वर्ण *str)
{
	size_t i;
	स्थिर ध्रुव वर्ण *ध्रुव modes[] = {
		[UL_COLORMODE_AUTO]   = "auto",
		[UL_COLORMODE_NEVER]  = "never",
		[UL_COLORMODE_ALWAYS] = "always",
		[UL_COLORMODE_UNDEF] = ""
	};

	यदि (!str || !*str)
		प्रत्यागम -EINVAL;

	प्रतिज्ञा(ARRAY_आकार(modes) == __UL_NCOLORMODES);

	प्रति (i = 0; i < ARRAY_आकार(modes); i++) {
		यदि (c_strcasecmp(str, modes[i]) == 0)
			प्रत्यागम i;
	}

	प्रत्यागम -EINVAL;
}

/*
 * Parses @str and exit(EXIT_FAILURE) on error
 */
पूर्णाङ्क colormode_or_err(ध्रुव वर्ण *str)
{
	ध्रुव वर्ण *p = str && *str == '=' ? str + 1 : str;
	पूर्णाङ्क colormode;

	colormode = colormode_from_string(p);
	यदि (colormode < 0)
		errx(EXIT_FAILURE, _("unsupported color mode: %s"), p);

	प्रत्यागम colormode;
}

#यदि_परिभाषितम् TEST_क्रमादेशः_COLORS
# include <getopt.h>
पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण *argv[])
{
	स्थिर ध्रुव संरचना option longopts[] = {
		{ "mode",	required_argument, रिक्त, 'm' },
		{ "color",	required_argument, रिक्त, 'c' },
		{ "color-scheme", required_argument, रिक्त, 'C' },
		{ "name",	required_argument, रिक्त, 'n' },
		{ रिक्त, 0, रिक्त, 0 }
	};
	पूर्णाङ्क c, बहुलकम् = UL_COLORMODE_UNDEF;	/* default */
	ध्रुव वर्ण *color = "red", *name = रिक्त, *color_scheme = रिक्त;
	वर्ण *seq = रिक्त;

	यावत् ((c = getopt_long(argc, argv, "C:c:m:n:", longopts, रिक्त)) != -1) {
		विकल्प (c) {
		पक्ष 'c':
			color = optarg;
			भङ्ग;
		पक्ष 'C':
			color_scheme = optarg;
			भङ्ग;
		पक्ष 'm':
			बहुलकम् = colormode_or_err(optarg);
			भङ्ग;
		पक्ष 'n':
			name = optarg;
			भङ्ग;
		स्वभाव:
			fprintf(stderr, "usage: %s [options]\n"
			" -m, --mode <auto|never|always>  default is undefined\n"
			" -c, --color <red|blue|...>      color for the test message\n"
			" -C, --color-scheme <name>       color for the test message\n"
			" -n, --name <utilname>           util name\n",
			क्रमादेशः_invocation_short_name);
			प्रत्यागम EXIT_FAILURE;
		}
	}

	colors_init(बहुलकम्, name ? name : क्रमादेशः_invocation_short_name);

	यदि (color_is_sequence(color))
		seq = strdup(color);
	अन्यथा
		seq = color_get_sequence(color);

	यदि (color_scheme)
		color_scheme_enable(color_scheme, seq);
	अन्यथा
		color_enable(seq);
	प्रारूप_लिख("Hello World!");
	color_disable();
	fputc('\n', stdout);

	मुक्त(seq);

	प्रत्यागम EXIT_SUCCESS;
}
#यदि_अन्तः /* TEST_PROGRAM_COLORS */

