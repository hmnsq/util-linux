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

#जर defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
# जर defined(HAVE_NCURSESW_NCURSES_H)
#  include <ncursesw/ncurses.h>
# elif defined(HAVE_NCURSES_NCURSES_H)
#  include <ncurses/ncurses.h>
# elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
# endif
# जर defined(HAVE_NCURSESW_TERM_H)
#  include <ncursesw/term.h>
# elif defined(HAVE_NCURSES_TERM_H)
#  include <ncurses/term.h>
# elif defined(HAVE_TERM_H)
#  include <term.h>
# endif
#endif

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
#ifdef USE_COLORS_BY_DEFAULT
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_AUTO	/* check isatty() */
#अन्यथा
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_NEVER	/* no colors by default */
#endif

/*
 * terminal-colors.d debug stuff
 */
स्थिर UL_DEBUG_DEFINE_MASK(termcolors);
UL_DEBUG_DEFINE_MASKNAMES(termcolors) = UL_DEBUG_EMPTY_MASKNAMES;

#define TERMCOLORS_DEBUG_INIT	(1 << 1)
#define TERMCOLORS_DEBUG_CONF	(1 << 2)
#define TERMCOLORS_DEBUG_SCHEME	(1 << 3)

#define DBG(m, x)       __UL_DBG(termcolors, TERMCOLORS_DEBUG_, m, x)
#define ON_DBG(m, x)    __UL_DBG_CALL(termcolors, TERMCOLORS_DEBUG_, m, x)

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
	अक्षर *name;
	अक्षर *seq;
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
	स्थिरांक अक्षर	*utilname;	/* util name */
	स्थिरांक अक्षर	*termname;	/* terminal name ($TERM) */

	अक्षर		*sfile;	/* path to scheme */

	संरचना ul_color_scheme	*schemes;	/* array with color schemes */
	माप_प्रकार			nschemes;	/* number of the items */
	माप_प्रकार			schemes_sz;	/* number of the allocated items */

	पूर्णांक		mode;		/* UL_COLORMODE_* */
	अचिन्हित पूर्णांक	has_colors : 1,	/* based on mode and scores[] */
			disabled   : 1, /* disable colors */
			cs_configured : 1, /* color schemes read */
			configured : 1; /* terminal-colors.d parsed */

	पूर्णांक		scores[__UL_COLORFILE_COUNT];	/* the best match */
};

/*
 * Control struct, globally shared.
 */
स्थिर संरचना ul_color_ctl ul_colors;

स्थिर शून्य colors_free_schemes(संरचना ul_color_ctl *cc);
स्थिर पूर्णांक colors_read_schemes(संरचना ul_color_ctl *cc);

/*
 * qsort/bsearch buddy
 */
स्थिर पूर्णांक cmp_scheme_name(स्थिरांक शून्य *a0, स्थिरांक शून्य *b0)
{
	स्थिरांक संरचना ul_color_scheme *a = (स्थिरांक संरचना ul_color_scheme *) a0,
				     *b = (स्थिरांक संरचना ul_color_scheme *) b0;
	परत म_भेद(a->name, b->name);
}

/*
 * Resets control struct (note that we don't allocate the struct)
 */
स्थिर शून्य colors_reset(संरचना ul_color_ctl *cc)
{
	जर (!cc)
		परत;

	colors_free_schemes(cc);

	मुक्त(cc->sfile);

	cc->sfile = रिक्त;
	cc->utilname = रिक्त;
	cc->termname = रिक्त;
	cc->mode = UL_COLORMODE_UNDEF;

	स_रखो(cc->scores, 0, आकार(cc->scores));
}

स्थिर शून्य colors_debug(संरचना ul_color_ctl *cc)
{
	माप_प्रकार i;

	जर (!cc)
		परत;

	म_लिखो("Colors:\n");
	म_लिखो("\tutilname = '%s'\n", cc->utilname);
	म_लिखो("\ttermname = '%s'\n", cc->termname);
	म_लिखो("\tscheme file = '%s'\n", cc->sfile);
	म_लिखो("\tmode = %s\n",
			cc->mode == UL_COLORMODE_UNDEF ? "undefined" :
			cc->mode == UL_COLORMODE_AUTO ?  "auto" :
			cc->mode == UL_COLORMODE_NEVER ? "never" :
			cc->mode == UL_COLORMODE_ALWAYS ? "always" : "???");
	म_लिखो("\thas_colors = %d\n", cc->has_colors);
	म_लिखो("\tdisabled = %d\n", cc->disabled);
	म_लिखो("\tconfigured = %d\n", cc->configured);
	म_लिखो("\tcs configured = %d\n", cc->cs_configured);

	ख_अक्षर_दो('\n', मानक_निकास);

	साठी (i = 0; i < ARRAY_SIZE(cc->scores); i++)
		म_लिखो("\tscore %s = %d\n",
				i == UL_COLORFILE_DISABLE ? "disable" :
				i == UL_COLORFILE_ENABLE ? "enable" :
				i == UL_COLORFILE_SCHEME ? "scheme" : "???",
				cc->scores[i]);

	ख_अक्षर_दो('\n', मानक_निकास);

	साठी (i = 0; i < cc->nschemes; i++) {
		म_लिखो("\tscheme #%02zu ", i);
		color_scheme_enable(cc->schemes[i].name, रिक्त);
		ख_माला_दो(cc->schemes[i].name, मानक_निकास);
		color_disable();
		ख_अक्षर_दो('\n', मानक_निकास);
	}
	ख_अक्षर_दो('\n', मानक_निकास);
}

/*
 * Parses [[<utilname>][@<termname>].]<type>
 */
स्थिर पूर्णांक filename_to_tokens(स्थिरांक अक्षर *str,
			      स्थिरांक अक्षर **name, माप_प्रकार *namesz,
			      स्थिरांक अक्षर **term, माप_प्रकार *termsz,
			      पूर्णांक  *filetype)
{
	स्थिरांक अक्षर *type_start, *term_start, *p;

	जर (!str || !*str || *str == '.' || म_माप(str) > PATH_MAX)
		परत -EINVAL;

	/* parse .type */
	p = म_खोजप(str, '.');
	type_start = p ? p + 1 : str;

	जर (म_भेद(type_start, "disable") == 0)
		*filetype = UL_COLORFILE_DISABLE;
	अन्यथा जर (म_भेद(type_start, "enable") == 0)
		*filetype = UL_COLORFILE_ENABLE;
	अन्यथा जर (म_भेद(type_start, "scheme") == 0)
		*filetype = UL_COLORFILE_SCHEME;
	अन्यथा {
		DBG(CONF, ul_debug("unknown type '%s'", type_start));
		परत 1;				/* unknown type */
	}

	जर (type_start == str)
		परत 0;				/* "type" only */

	/* parse @termname */
	p = म_अक्षर(str, '@');
	term_start = p ? p + 1 : रिक्त;
	जर (term_start) {
		*term = term_start;
		*termsz = type_start - term_start - 1;
		जर (term_start - 1 == str)
			परत 0;			/* "@termname.type" */
	}

	/* parse utilname */
	p = term_start ? term_start : type_start;
	*name = str;
	*namesz = p - str - 1;

	परत 0;
}

/*
 * Scans @dirname and select the best matches for UL_COLORFILE_* types.
 * The result is stored to cc->scores. The path to the best "scheme"
 * file is stored to cc->scheme.
 */
स्थिर पूर्णांक colors_readdir(संरचना ul_color_ctl *cc, स्थिरांक अक्षर *स_नाम)
{
	सूची *dir;
	पूर्णांक rc = -ENOENT;
	संरचना dirent *d;
	अक्षर sfile[PATH_MAX] = { '\0' };
	माप_प्रकार namesz, termsz;

	जर (!स_नाम || !cc || !cc->utilname || !*cc->utilname)
		परत -EINVAL;

	DBG(CONF, ul_debug("reading dir: '%s'", स_नाम));

	dir = सूची_खोलो(स_नाम);
	जर (!dir)
		परत -त्रुटि_सं;

	namesz = म_माप(cc->utilname);
	termsz = cc->termname ? म_माप(cc->termname) : 0;

	पर्यंत ((d = सूची_पढ़ो(dir))) {
		पूर्णांक type, score = 1;
		स्थिरांक अक्षर *tk_name = रिक्त, *tk_term = रिक्त;
		माप_प्रकार tk_namesz = 0, tk_termsz = 0;

		जर (*d->d_name == '.')
			सुरूठेवा;
#ifdef _DIRENT_HAVE_D_TYPE
		जर (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK &&
		    d->d_type != DT_REG)
			सुरूठेवा;
#endif
		जर (filename_to_tokens(d->d_name,
				       &tk_name, &tk_namesz,
				       &tk_term, &tk_termsz, &type) != 0)
			सुरूठेवा;

		/* count theoretical score before we check names to avoid
		 * unnecessary strcmp() */
		जर (tk_name)
			score += 20;
		जर (tk_term)
			score += 10;

		DBG(CONF, ul_debug("item '%s': score=%d "
			"[cur: %d, name(%zu): %s, term(%zu): %s]",
			d->d_name, score, cc->scores[type],
			tk_namesz, tk_name,
			tk_termsz, tk_term));


		जर (score < cc->scores[type])
			सुरूठेवा;

		/* filter out by names */
		जर (tk_namesz && (tk_namesz != namesz ||
				 म_भेदन(tk_name, cc->utilname, namesz) != 0))
			सुरूठेवा;

		जर (tk_termsz && (termsz == 0 || tk_termsz != termsz ||
				  म_भेदन(tk_term, cc->termname, termsz) != 0))
			सुरूठेवा;

		DBG(CONF, ul_debug("setting '%s' from %d -to-> %d",
					type == UL_COLORFILE_SCHEME ? "scheme" :
					type == UL_COLORFILE_DISABLE ? "disable" :
					type == UL_COLORFILE_ENABLE ? "enable" : "???",
					cc->scores[type], score));
		cc->scores[type] = score;
		rc = 0;
		जर (type == UL_COLORFILE_SCHEME)
			म_नकलन(sfile, d->d_name, आकार(sfile));
	}

	जर (*sfile) {
		sfile[आकार(sfile) - 1] = '\0';
		जर (asprintf(&cc->sfile, "%s/%s", स_नाम, sfile) <= 0)
			rc = -ENOMEM;
	}

	बंद_सूची(dir);
	परत rc;
}

/* atexit() wrapper */
स्थिर शून्य colors_deinit(शून्य)
{
	colors_reset(&ul_colors);
}

/*
 * Returns path to $XDG_CONFIG_HOME/terminal-colors.d
 */
स्थिर अक्षर *colors_get_homedir(अक्षर *buf, माप_प्रकार bufsz)
{
	अक्षर *p = दो_पर्या("XDG_CONFIG_HOME");

	जर (p) {
		snprintf(buf, bufsz, "%s/" _PATH_TERMCOLORS_DIRNAME, p);
		परत buf;
	}

	p = दो_पर्या("HOME");
	जर (p) {
		snprintf(buf, bufsz, "%s/.config/" _PATH_TERMCOLORS_DIRNAME, p);
		परत buf;
	}

	परत रिक्त;
}

/*
 * Adds one color sequence to array with color scheme.
 */
स्थिर पूर्णांक colors_add_scheme(संरचना ul_color_ctl *cc,
			     स्थिरांक अक्षर *name,
			     स्थिरांक अक्षर *seq0)
{
	संरचना ul_color_scheme *cs = रिक्त;
	अक्षर *seq = रिक्त;
	पूर्णांक rc;

	जर (!cc || !name || !*name || !seq0 || !*seq0)
		परत -EINVAL;

	DBG(SCHEME, ul_debug("add '%s'", name));

	seq = color_get_sequence(seq0);
	जर (!seq)
		परत -EINVAL;
	rc = -ENOMEM;

	/* enlarge the array */
	जर (cc->nschemes == cc->schemes_sz) {
		शून्य *tmp = reallocarray(cc->schemes, cc->nschemes + 10,
					 आकार(संरचना ul_color_scheme));
		जर (!tmp)
			जा err;
		cc->schemes = tmp;
		cc->schemes_sz = cc->nschemes + 10;
	}

	/* add a new item */
	cs = &cc->schemes[cc->nschemes];
	cs->seq = seq;
	cs->name = strdup(name);
	जर (!cs->name)
		जा err;

	cc->nschemes++;
	परत 0;
err:
	जर (cs) {
		मुक्त(cs->seq);
		मुक्त(cs->name);
		cs->seq = cs->name = रिक्त;
	} अन्यथा
		मुक्त(seq);
	परत rc;
}

/*
 * Deallocates all regards to color schemes
 */
स्थिर शून्य colors_free_schemes(संरचना ul_color_ctl *cc)
{
	माप_प्रकार i;

	DBG(SCHEME, ul_debug("free scheme"));

	साठी (i = 0; i < cc->nschemes; i++) {
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
स्थिर शून्य colors_sort_schemes(संरचना ul_color_ctl *cc)
{
	जर (!cc->nschemes)
		परत;

	DBG(SCHEME, ul_debug("sort scheme"));

	क्विक(cc->schemes, cc->nschemes,
	      आकार(संरचना ul_color_scheme), cmp_scheme_name);
}

/*
 * Returns just one color scheme
 */
स्थिर संरचना ul_color_scheme *colors_get_scheme(संरचना ul_color_ctl *cc,
						 स्थिरांक अक्षर *name)
{
	संरचना ul_color_scheme किल्ली = { .name = (अक्षर *) name}, *res;

	जर (!cc || !name || !*name)
		परत रिक्त;

	जर (!cc->cs_configured) {
		पूर्णांक rc = colors_read_schemes(cc);
		जर (rc)
			परत रिक्त;
	}
	जर (!cc->nschemes)
		परत रिक्त;

	DBG(SCHEME, ul_debug("search '%s'", name));

	res = द्वा_खोज(&किल्ली, cc->schemes, cc->nschemes,
				आकार(संरचना ul_color_scheme),
				cmp_scheme_name);

	परत res && res->seq ? res  : रिक्त;
}

/*
 * Parses filenames in terminal-colors.d
 */
स्थिर पूर्णांक colors_read_configuration(संरचना ul_color_ctl *cc)
{
	पूर्णांक rc = -ENOENT;
	अक्षर *स_नाम, buf[PATH_MAX];

	cc->termname = दो_पर्या("TERM");

	स_नाम = colors_get_homedir(buf, आकार(buf));
	जर (स_नाम)
		rc = colors_readdir(cc, स_नाम);		/* ~/.config */
	जर (rc == -EPERM || rc == -EACCES || rc == -ENOENT)
		rc = colors_readdir(cc, _PATH_TERMCOLORS_DIR);	/* /etc */

	cc->configured = 1;
	परत rc;
}

/*
 * Reads terminal-colors.d/ scheme file into array schemes
 */
स्थिर पूर्णांक colors_read_schemes(संरचना ul_color_ctl *cc)
{
	पूर्णांक rc = 0;
	खाता *f = रिक्त;
	अक्षर buf[बफ_मान],
	     cn[129], seq[129];

	जर (!cc->configured)
		rc = colors_read_configuration(cc);

	cc->cs_configured = 1;

	जर (rc || !cc->sfile)
		जा done;

	DBG(SCHEME, ul_debug("reading file '%s'", cc->sfile));

	f = ख_खोलो(cc->sfile, "r" UL_CLOEXECSTR);
	जर (!f) {
		rc = -त्रुटि_सं;
		जा done;
	}

	पर्यंत (ख_माला_लो(buf, आकार(buf), f)) {
		अक्षर *p = म_अक्षर(buf, '\n');

		जर (!p) {
			जर (ख_पूर्ण(f))
				p = म_अक्षर(buf, '\0');
			अन्यथा {
				rc = -त्रुटि_सं;
				जा done;
			}
		}
		*p = '\0';
		p = (अक्षर *) skip_blank(buf);
		जर (*p == '\0' || *p == '#')
			सुरूठेवा;

		rc = माला_पूछो(p, "%128[^ ] %128[^\n ]", cn, seq);
		जर (rc == 2 && *cn && *seq) {
			rc = colors_add_scheme(cc, cn, seq);	/* set rc=0 on success */
			जर (rc)
				जा done;
		}
	}
	rc = 0;

done:
	जर (f)
		ख_बंद(f);
	colors_sort_schemes(cc);

	परत rc;
}


स्थिर शून्य termcolors_init_debug(शून्य)
{
	__UL_INIT_DEBUG_FROM_ENV(termcolors, TERMCOLORS_DEBUG_, 0, TERMINAL_COLORS_DEBUG);
}

स्थिर पूर्णांक colors_terminal_is_ready(शून्य)
{
	पूर्णांक ncolors = -1;

#जर defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
	{
		पूर्णांक ret;

		/* setupterm() allocates memory, del_curterm() deallocates it */
		जर (setupterm(रिक्त, STDOUT_FILENO, &ret) == 0 && ret == 1) {
			ncolors = tigetnum("colors");
			del_curterm(cur_term);
		}
	}
#endif
	जर (1 < ncolors) {
		DBG(CONF, ul_debug("terminal is ready (supports %d colors)", ncolors));
		परत 1;
	}

	DBG(CONF, ul_debug("terminal is NOT ready (no colors)"));
	परत 0;
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
पूर्णांक colors_init(पूर्णांक mode, स्थिरांक अक्षर *name)
{
	पूर्णांक ready = -1;
	संरचना ul_color_ctl *cc = &ul_colors;

	cc->utilname = name;

	termcolors_init_debug();

	जर (mode != UL_COLORMODE_ALWAYS && !isatty(STDOUT_FILENO))
		cc->mode = UL_COLORMODE_NEVER;
	अन्यथा
		cc->mode = mode;

	जर (cc->mode == UL_COLORMODE_UNDEF
	    && दो_पर्या("NO_COLOR") == रिक्त
	    && (ready = colors_terminal_is_ready())) {

		पूर्णांक rc = colors_read_configuration(cc);
		जर (rc)
			cc->mode = UL_COLORMODE_DEFAULT;
		अन्यथा {

			/* evaluate scores */
			जर (cc->scores[UL_COLORFILE_DISABLE] >
			    cc->scores[UL_COLORFILE_ENABLE])
				cc->mode = UL_COLORMODE_NEVER;
			अन्यथा
				cc->mode = UL_COLORMODE_DEFAULT;

			निकास_पर(colors_deinit);
		}
	}

	निवड (cc->mode) {
	स्थिती UL_COLORMODE_AUTO:
		cc->has_colors = ready == -1 ? colors_terminal_is_ready() : ready;
		थांबा;
	स्थिती UL_COLORMODE_ALWAYS:
		cc->has_colors = 1;
		थांबा;
	स्थिती UL_COLORMODE_NEVER:
	पूर्वनिर्धारित:
		cc->has_colors = 0;
	}

	ON_DBG(CONF, colors_debug(cc));

	परत cc->has_colors;
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
पूर्णांक colors_wanted(शून्य)
{
	परत ul_colors.has_colors;
}

/*
 * Returns mode
 */
पूर्णांक colors_mode(शून्य)
{
	परत ul_colors.mode;
}

/*
 * Enable @seq color
 */
शून्य color_fenable(स्थिरांक अक्षर *seq, खाता *f)
{
	जर (!ul_colors.disabled && ul_colors.has_colors && seq)
		ख_माला_दो(seq, f);
}

/*
 * Returns escape sequence by logical @name, if undefined then returns @dflt.
 */
स्थिरांक अक्षर *color_scheme_get_sequence(स्थिरांक अक्षर *name, स्थिरांक अक्षर *dflt)
{
	संरचना ul_color_scheme *cs;

	जर (ul_colors.disabled || !ul_colors.has_colors)
		परत रिक्त;

	cs = colors_get_scheme(&ul_colors, name);
	परत cs && cs->seq ? cs->seq : dflt;
}

/*
 * Enable color by logical @name, if undefined enable @dflt.
 */
शून्य color_scheme_fenable(स्थिरांक अक्षर *name, स्थिरांक अक्षर *dflt, खाता *f)
{
	स्थिरांक अक्षर *seq = color_scheme_get_sequence(name, dflt);

	जर (!seq)
		परत;
	color_fenable(seq, f);
}


/*
 * Disable previously enabled color
 */
शून्य color_fdisable(खाता *f)
{
	जर (!ul_colors.disabled && ul_colors.has_colors)
		ख_माला_दो(UL_COLOR_RESET, f);
}

/*
 * Get reset sequence
 */
स्थिरांक अक्षर *color_get_disable_sequence(शून्य)
{
	जर (!ul_colors.disabled && ul_colors.has_colors)
		परत UL_COLOR_RESET;
	अन्यथा
		परत "";
}

/*
 * Parses @str to return UL_COLORMODE_*
 */
पूर्णांक colormode_from_string(स्थिरांक अक्षर *str)
{
	माप_प्रकार i;
	स्थिर स्थिरांक अक्षर *स्थिरांक modes[] = {
		[UL_COLORMODE_AUTO]   = "auto",
		[UL_COLORMODE_NEVER]  = "never",
		[UL_COLORMODE_ALWAYS] = "always",
		[UL_COLORMODE_UNDEF] = ""
	};

	जर (!str || !*str)
		परत -EINVAL;

	प्रतिज्ञा(ARRAY_SIZE(modes) == __UL_NCOLORMODES);

	साठी (i = 0; i < ARRAY_SIZE(modes); i++) {
		जर (c_strcasecmp(str, modes[i]) == 0)
			परत i;
	}

	परत -EINVAL;
}

/*
 * Parses @str and exit(EXIT_FAILURE) on error
 */
पूर्णांक colormode_or_err(स्थिरांक अक्षर *str)
{
	स्थिरांक अक्षर *p = str && *str == '=' ? str + 1 : str;
	पूर्णांक colormode;

	colormode = colormode_from_string(p);
	जर (colormode < 0)
		errx(निकास_त्रुटि, _("unsupported color mode: %s"), p);

	परत colormode;
}

#ifdef TEST_PROGRAM_COLORS
# include <getopt.h>
पूर्णांक main(पूर्णांक argc, अक्षर *argv[])
{
	स्थिर स्थिरांक संरचना option longopts[] = {
		{ "mode",	required_argument, रिक्त, 'm' },
		{ "color",	required_argument, रिक्त, 'c' },
		{ "color-scheme", required_argument, रिक्त, 'C' },
		{ "name",	required_argument, रिक्त, 'n' },
		{ रिक्त, 0, रिक्त, 0 }
	};
	पूर्णांक c, mode = UL_COLORMODE_UNDEF;	/* default */
	स्थिरांक अक्षर *color = "red", *name = रिक्त, *color_scheme = रिक्त;
	अक्षर *seq = रिक्त;

	पर्यंत ((c = getopt_long(argc, argv, "C:c:m:n:", longopts, रिक्त)) != -1) {
		निवड (c) {
		स्थिती 'c':
			color = optarg;
			थांबा;
		स्थिती 'C':
			color_scheme = optarg;
			थांबा;
		स्थिती 'm':
			mode = colormode_or_err(optarg);
			थांबा;
		स्थिती 'n':
			name = optarg;
			थांबा;
		पूर्वनिर्धारित:
			ख_लिखो(मानक_त्रुटि, "usage: %s [options]\n"
			" -m, --mode <auto|never|always>  default is undefined\n"
			" -c, --color <red|blue|...>      color for the test message\n"
			" -C, --color-scheme <name>       color for the test message\n"
			" -n, --name <utilname>           util name\n",
			program_invocation_short_name);
			परत निकास_त्रुटि;
		}
	}

	colors_init(mode, name ? name : program_invocation_short_name);

	जर (color_is_sequence(color))
		seq = strdup(color);
	अन्यथा
		seq = color_get_sequence(color);

	जर (color_scheme)
		color_scheme_enable(color_scheme, seq);
	अन्यथा
		color_enable(seq);
	म_लिखो("Hello World!");
	color_disable();
	ख_अक्षर_दो('\n', मानक_निकास);

	मुक्त(seq);

	परत निकास_सफल;
}
#endif /* TEST_PROGRAM_COLORS */

