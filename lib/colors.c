/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: 2012 Ondrej Oprala <ooprala@redhat.com>
 *          2012-2025 Karel Zak <kzak@redhat.com>
 */
#समावेश <assert.h>
#समावेश <sys/stat.h>
#समावेश <sys/types.h>
#समावेश <dirent.h>
#समावेश <ctype.h>

#अगर defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
# अगर defined(HAVE_NCURSESW_NCURSES_H)
#  include <ncursesw/ncurses.h>
# elif defined(HAVE_NCURSES_NCURSES_H)
#  include <ncurses/ncurses.h>
# elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
# endif
# अगर defined(HAVE_NCURSESW_TERM_H)
#  include <ncursesw/term.h>
# elif defined(HAVE_NCURSES_TERM_H)
#  include <ncurses/term.h>
# elif defined(HAVE_TERM_H)
#  include <term.h>
# endif
#पूर्ण_अगर

#समावेश "c.h"
#समावेश "cctype.h"
#समावेश "colors.h"
#समावेश "nls.h"
#समावेश "pathnames.h"
#समावेश "strutils.h"

#समावेश "debug.h"

/*
 * Default behavior, may be overridden by terminal-colors.d/{enable,disable}.
 */
#अगर_घोषित USE_COLORS_BY_DEFAULT
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_AUTO	/* check isatty() */
#अन्यथा
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_NEVER	/* no colors by default */
#पूर्ण_अगर

/*
 * terminal-colors.d debug stuff
 */
अटल UL_DEBUG_DEFINE_MASK(termcolors);
UL_DEBUG_DEFINE_MASKNAMES(termcolors) = UL_DEBUG_EMPTY_MASKNAMES;

#घोषणा TERMCOLORS_DEBUG_INIT	(1 << 1)
#घोषणा TERMCOLORS_DEBUG_CONF	(1 << 2)
#घोषणा TERMCOLORS_DEBUG_SCHEME	(1 << 3)

#घोषणा DBG(m, x)       __UL_DBG(termcolors, TERMCOLORS_DEBUG_, m, x)
#घोषणा ON_DBG(m, x)    __UL_DBG_CALL(termcolors, TERMCOLORS_DEBUG_, m, x)

/*
 * terminal-colors.d file types
 */
क्रमागत {
	UL_COLORFILE_DISABLE,		/* .disable */
	UL_COLORFILE_ENABLE,		/* .enable */
	UL_COLORFILE_SCHEME,		/* .scheme */

	__UL_COLORFILE_COUNT
};

काष्ठा ul_color_scheme {
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
काष्ठा ul_color_ctl {
	स्थिर अक्षर	*utilname;	/* util name */
	स्थिर अक्षर	*termname;	/* terminal name ($TERM) */

	अक्षर		*sfile;	/* path to scheme */

	काष्ठा ul_color_scheme	*schemes;	/* array with color schemes */
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
अटल काष्ठा ul_color_ctl ul_colors;

अटल व्योम colors_free_schemes(काष्ठा ul_color_ctl *cc);
अटल पूर्णांक colors_read_schemes(काष्ठा ul_color_ctl *cc);

/*
 * qsort/bsearch buddy
 */
अटल पूर्णांक cmp_scheme_name(स्थिर व्योम *a0, स्थिर व्योम *b0)
{
	स्थिर काष्ठा ul_color_scheme *a = (स्थिर काष्ठा ul_color_scheme *) a0,
				     *b = (स्थिर काष्ठा ul_color_scheme *) b0;
	वापस म_भेद(a->name, b->name);
}

/*
 * Resets control struct (note that we don't allocate the struct)
 */
अटल व्योम colors_reset(काष्ठा ul_color_ctl *cc)
{
	अगर (!cc)
		वापस;

	colors_free_schemes(cc);

	मुक्त(cc->sfile);

	cc->sfile = शून्य;
	cc->utilname = शून्य;
	cc->termname = शून्य;
	cc->mode = UL_COLORMODE_UNDEF;

	स_रखो(cc->scores, 0, माप(cc->scores));
}

अटल व्योम colors_debug(काष्ठा ul_color_ctl *cc)
{
	माप_प्रकार i;

	अगर (!cc)
		वापस;

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

	क्रम (i = 0; i < ARRAY_आकार(cc->scores); i++)
		म_लिखो("\tscore %s = %d\n",
				i == UL_COLORFILE_DISABLE ? "disable" :
				i == UL_COLORFILE_ENABLE ? "enable" :
				i == UL_COLORFILE_SCHEME ? "scheme" : "???",
				cc->scores[i]);

	ख_अक्षर_दो('\n', मानक_निकास);

	क्रम (i = 0; i < cc->nschemes; i++) {
		म_लिखो("\tscheme #%02zu ", i);
		color_scheme_enable(cc->schemes[i].name, शून्य);
		ख_माला_दो(cc->schemes[i].name, मानक_निकास);
		color_disable();
		ख_अक्षर_दो('\n', मानक_निकास);
	}
	ख_अक्षर_दो('\n', मानक_निकास);
}

/*
 * Parses [[<utilname>][@<termname>].]<type>
 */
अटल पूर्णांक filename_to_tokens(स्थिर अक्षर *str,
			      स्थिर अक्षर **name, माप_प्रकार *namesz,
			      स्थिर अक्षर **term, माप_प्रकार *termsz,
			      पूर्णांक  *filetype)
{
	स्थिर अक्षर *type_start, *term_start, *p;

	अगर (!str || !*str || *str == '.' || म_माप(str) > PATH_MAX)
		वापस -EINVAL;

	/* parse .type */
	p = म_खोजप(str, '.');
	type_start = p ? p + 1 : str;

	अगर (म_भेद(type_start, "disable") == 0)
		*filetype = UL_COLORFILE_DISABLE;
	अन्यथा अगर (म_भेद(type_start, "enable") == 0)
		*filetype = UL_COLORFILE_ENABLE;
	अन्यथा अगर (म_भेद(type_start, "scheme") == 0)
		*filetype = UL_COLORFILE_SCHEME;
	अन्यथा {
		DBG(CONF, ul_debug("unknown type '%s'", type_start));
		वापस 1;				/* unknown type */
	}

	अगर (type_start == str)
		वापस 0;				/* "type" only */

	/* parse @termname */
	p = म_अक्षर(str, '@');
	term_start = p ? p + 1 : शून्य;
	अगर (term_start) {
		*term = term_start;
		*termsz = type_start - term_start - 1;
		अगर (term_start - 1 == str)
			वापस 0;			/* "@termname.type" */
	}

	/* parse utilname */
	p = term_start ? term_start : type_start;
	*name = str;
	*namesz = p - str - 1;

	वापस 0;
}

/*
 * Scans @dirname and select the best matches for UL_COLORFILE_* types.
 * The result is stored to cc->scores. The path to the best "scheme"
 * file is stored to cc->scheme.
 */
अटल पूर्णांक colors_readdir(काष्ठा ul_color_ctl *cc, स्थिर अक्षर *स_नाम)
{
	सूची *dir;
	पूर्णांक rc = -ENOENT;
	काष्ठा dirent *d;
	अक्षर sfile[PATH_MAX] = { '\0' };
	माप_प्रकार namesz, termsz;

	अगर (!स_नाम || !cc || !cc->utilname || !*cc->utilname)
		वापस -EINVAL;

	DBG(CONF, ul_debug("reading dir: '%s'", स_नाम));

	dir = सूची_खोलो(स_नाम);
	अगर (!dir)
		वापस -त्रुटि_सं;

	namesz = म_माप(cc->utilname);
	termsz = cc->termname ? म_माप(cc->termname) : 0;

	जबतक ((d = सूची_पढ़ो(dir))) {
		पूर्णांक type, score = 1;
		स्थिर अक्षर *tk_name = शून्य, *tk_term = शून्य;
		माप_प्रकार tk_namesz = 0, tk_termsz = 0;

		अगर (*d->d_name == '.')
			जारी;
#अगर_घोषित _DIRENT_HAVE_D_TYPE
		अगर (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK &&
		    d->d_type != DT_REG)
			जारी;
#पूर्ण_अगर
		अगर (filename_to_tokens(d->d_name,
				       &tk_name, &tk_namesz,
				       &tk_term, &tk_termsz, &type) != 0)
			जारी;

		/* count theoretical score before we check names to avoid
		 * unnecessary strcmp() */
		अगर (tk_name)
			score += 20;
		अगर (tk_term)
			score += 10;

		DBG(CONF, ul_debug("item '%s': score=%d "
			"[cur: %d, name(%zu): %s, term(%zu): %s]",
			d->d_name, score, cc->scores[type],
			tk_namesz, tk_name,
			tk_termsz, tk_term));


		अगर (score < cc->scores[type])
			जारी;

		/* filter out by names */
		अगर (tk_namesz && (tk_namesz != namesz ||
				 म_भेदन(tk_name, cc->utilname, namesz) != 0))
			जारी;

		अगर (tk_termsz && (termsz == 0 || tk_termsz != termsz ||
				  म_भेदन(tk_term, cc->termname, termsz) != 0))
			जारी;

		DBG(CONF, ul_debug("setting '%s' from %d -to-> %d",
					type == UL_COLORFILE_SCHEME ? "scheme" :
					type == UL_COLORFILE_DISABLE ? "disable" :
					type == UL_COLORFILE_ENABLE ? "enable" : "???",
					cc->scores[type], score));
		cc->scores[type] = score;
		rc = 0;
		अगर (type == UL_COLORFILE_SCHEME)
			म_नकलन(sfile, d->d_name, माप(sfile));
	}

	अगर (*sfile) {
		sfile[माप(sfile) - 1] = '\0';
		अगर (asprintf(&cc->sfile, "%s/%s", स_नाम, sfile) <= 0)
			rc = -ENOMEM;
	}

	बंद_सूची(dir);
	वापस rc;
}

/* atexit() wrapper */
अटल व्योम colors_deinit(व्योम)
{
	colors_reset(&ul_colors);
}

/*
 * Returns path to $XDG_CONFIG_HOME/terminal-colors.d
 */
अटल अक्षर *colors_get_homedir(अक्षर *buf, माप_प्रकार bufsz)
{
	अक्षर *p = दो_पर्या("XDG_CONFIG_HOME");

	अगर (p) {
		snprintf(buf, bufsz, "%s/" _PATH_TERMCOLORS_DIRNAME, p);
		वापस buf;
	}

	p = दो_पर्या("HOME");
	अगर (p) {
		snprintf(buf, bufsz, "%s/.config/" _PATH_TERMCOLORS_DIRNAME, p);
		वापस buf;
	}

	वापस शून्य;
}

/*
 * Adds one color sequence to array with color scheme.
 */
अटल पूर्णांक colors_add_scheme(काष्ठा ul_color_ctl *cc,
			     स्थिर अक्षर *name,
			     स्थिर अक्षर *seq0)
{
	काष्ठा ul_color_scheme *cs = शून्य;
	अक्षर *seq = शून्य;
	पूर्णांक rc;

	अगर (!cc || !name || !*name || !seq0 || !*seq0)
		वापस -EINVAL;

	DBG(SCHEME, ul_debug("add '%s'", name));

	seq = color_get_sequence(seq0);
	अगर (!seq)
		वापस -EINVAL;
	rc = -ENOMEM;

	/* enlarge the array */
	अगर (cc->nschemes == cc->schemes_sz) {
		व्योम *tmp = reallocarray(cc->schemes, cc->nschemes + 10,
					 माप(काष्ठा ul_color_scheme));
		अगर (!tmp)
			जाओ err;
		cc->schemes = tmp;
		cc->schemes_sz = cc->nschemes + 10;
	}

	/* add a new item */
	cs = &cc->schemes[cc->nschemes];
	cs->seq = seq;
	cs->name = strdup(name);
	अगर (!cs->name)
		जाओ err;

	cc->nschemes++;
	वापस 0;
err:
	अगर (cs) {
		मुक्त(cs->seq);
		मुक्त(cs->name);
		cs->seq = cs->name = शून्य;
	} अन्यथा
		मुक्त(seq);
	वापस rc;
}

/*
 * Deallocates all regards to color schemes
 */
अटल व्योम colors_free_schemes(काष्ठा ul_color_ctl *cc)
{
	माप_प्रकार i;

	DBG(SCHEME, ul_debug("free scheme"));

	क्रम (i = 0; i < cc->nschemes; i++) {
		मुक्त(cc->schemes[i].name);
		मुक्त(cc->schemes[i].seq);
	}

	मुक्त(cc->schemes);
	cc->schemes = शून्य;
	cc->nschemes = 0;
	cc->schemes_sz = 0;
}

/*
 * The scheme configuration has to be sorted for bsearch
 */
अटल व्योम colors_sort_schemes(काष्ठा ul_color_ctl *cc)
{
	अगर (!cc->nschemes)
		वापस;

	DBG(SCHEME, ul_debug("sort scheme"));

	क्विक(cc->schemes, cc->nschemes,
	      माप(काष्ठा ul_color_scheme), cmp_scheme_name);
}

/*
 * Returns just one color scheme
 */
अटल काष्ठा ul_color_scheme *colors_get_scheme(काष्ठा ul_color_ctl *cc,
						 स्थिर अक्षर *name)
{
	काष्ठा ul_color_scheme कुंजी = { .name = (अक्षर *) name}, *res;

	अगर (!cc || !name || !*name)
		वापस शून्य;

	अगर (!cc->cs_configured) {
		पूर्णांक rc = colors_read_schemes(cc);
		अगर (rc)
			वापस शून्य;
	}
	अगर (!cc->nschemes)
		वापस शून्य;

	DBG(SCHEME, ul_debug("search '%s'", name));

	res = द्वा_खोज(&कुंजी, cc->schemes, cc->nschemes,
				माप(काष्ठा ul_color_scheme),
				cmp_scheme_name);

	वापस res && res->seq ? res  : शून्य;
}

/*
 * Parses filenames in terminal-colors.d
 */
अटल पूर्णांक colors_read_configuration(काष्ठा ul_color_ctl *cc)
{
	पूर्णांक rc = -ENOENT;
	अक्षर *स_नाम, buf[PATH_MAX];

	cc->termname = दो_पर्या("TERM");

	स_नाम = colors_get_homedir(buf, माप(buf));
	अगर (स_नाम)
		rc = colors_readdir(cc, स_नाम);		/* ~/.config */
	अगर (rc == -EPERM || rc == -EACCES || rc == -ENOENT)
		rc = colors_readdir(cc, _PATH_TERMCOLORS_DIR);	/* /etc */

	cc->configured = 1;
	वापस rc;
}

/*
 * Reads terminal-colors.d/ scheme file into array schemes
 */
अटल पूर्णांक colors_read_schemes(काष्ठा ul_color_ctl *cc)
{
	पूर्णांक rc = 0;
	खाता *f = शून्य;
	अक्षर buf[बफ_मान],
	     cn[129], seq[129];

	अगर (!cc->configured)
		rc = colors_read_configuration(cc);

	cc->cs_configured = 1;

	अगर (rc || !cc->sfile)
		जाओ done;

	DBG(SCHEME, ul_debug("reading file '%s'", cc->sfile));

	f = ख_खोलो(cc->sfile, "r" UL_CLOEXECSTR);
	अगर (!f) {
		rc = -त्रुटि_सं;
		जाओ done;
	}

	जबतक (ख_माला_लो(buf, माप(buf), f)) {
		अक्षर *p = म_अक्षर(buf, '\n');

		अगर (!p) {
			अगर (ख_पूर्ण(f))
				p = म_अक्षर(buf, '\0');
			अन्यथा {
				rc = -त्रुटि_सं;
				जाओ done;
			}
		}
		*p = '\0';
		p = (अक्षर *) skip_blank(buf);
		अगर (*p == '\0' || *p == '#')
			जारी;

		rc = माला_पूछो(p, "%128[^ ] %128[^\n ]", cn, seq);
		अगर (rc == 2 && *cn && *seq) {
			rc = colors_add_scheme(cc, cn, seq);	/* set rc=0 on success */
			अगर (rc)
				जाओ done;
		}
	}
	rc = 0;

done:
	अगर (f)
		ख_बंद(f);
	colors_sort_schemes(cc);

	वापस rc;
}


अटल व्योम termcolors_init_debug(व्योम)
{
	__UL_INIT_DEBUG_FROM_ENV(termcolors, TERMCOLORS_DEBUG_, 0, TERMINAL_COLORS_DEBUG);
}

अटल पूर्णांक colors_terminal_is_ready(व्योम)
{
	पूर्णांक ncolors = -1;

#अगर defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
	{
		पूर्णांक ret;

		/* setupterm() allocates memory, del_curterm() deallocates it */
		अगर (setupterm(शून्य, STDOUT_FILENO, &ret) == 0 && ret == 1) {
			ncolors = tigetnum("colors");
			del_curterm(cur_term);
		}
	}
#पूर्ण_अगर
	अगर (1 < ncolors) {
		DBG(CONF, ul_debug("terminal is ready (supports %d colors)", ncolors));
		वापस 1;
	}

	DBG(CONF, ul_debug("terminal is NOT ready (no colors)"));
	वापस 0;
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
पूर्णांक colors_init(पूर्णांक mode, स्थिर अक्षर *name)
{
	पूर्णांक ready = -1;
	काष्ठा ul_color_ctl *cc = &ul_colors;

	cc->utilname = name;

	termcolors_init_debug();

	अगर (mode != UL_COLORMODE_ALWAYS && !isatty(STDOUT_FILENO))
		cc->mode = UL_COLORMODE_NEVER;
	अन्यथा
		cc->mode = mode;

	अगर (cc->mode == UL_COLORMODE_UNDEF
	    && दो_पर्या("NO_COLOR") == शून्य
	    && (ready = colors_terminal_is_ready())) {

		पूर्णांक rc = colors_read_configuration(cc);
		अगर (rc)
			cc->mode = UL_COLORMODE_DEFAULT;
		अन्यथा {

			/* evaluate scores */
			अगर (cc->scores[UL_COLORFILE_DISABLE] >
			    cc->scores[UL_COLORFILE_ENABLE])
				cc->mode = UL_COLORMODE_NEVER;
			अन्यथा
				cc->mode = UL_COLORMODE_DEFAULT;

			निकास_पर(colors_deinit);
		}
	}

	चयन (cc->mode) {
	हाल UL_COLORMODE_AUTO:
		cc->has_colors = ready == -1 ? colors_terminal_is_ready() : ready;
		अवरोध;
	हाल UL_COLORMODE_ALWAYS:
		cc->has_colors = 1;
		अवरोध;
	हाल UL_COLORMODE_NEVER:
	शेष:
		cc->has_colors = 0;
	}

	ON_DBG(CONF, colors_debug(cc));

	वापस cc->has_colors;
}

/*
 * Temporary disable colors (this setting is independent on terminal-colors.d/)
 */
व्योम colors_off(व्योम)
{
	ul_colors.disabled = 1;
}

/*
 * Enable colors
 */
व्योम colors_on(व्योम)
{
	ul_colors.disabled = 0;
}

/*
 * Is terminal-colors.d/ configured to use colors?
 */
पूर्णांक colors_wanted(व्योम)
{
	वापस ul_colors.has_colors;
}

/*
 * Returns mode
 */
पूर्णांक colors_mode(व्योम)
{
	वापस ul_colors.mode;
}

/*
 * Enable @seq color
 */
व्योम color_fenable(स्थिर अक्षर *seq, खाता *f)
{
	अगर (!ul_colors.disabled && ul_colors.has_colors && seq)
		ख_माला_दो(seq, f);
}

/*
 * Returns escape sequence by logical @name, if undefined then returns @dflt.
 */
स्थिर अक्षर *color_scheme_get_sequence(स्थिर अक्षर *name, स्थिर अक्षर *dflt)
{
	काष्ठा ul_color_scheme *cs;

	अगर (ul_colors.disabled || !ul_colors.has_colors)
		वापस शून्य;

	cs = colors_get_scheme(&ul_colors, name);
	वापस cs && cs->seq ? cs->seq : dflt;
}

/*
 * Enable color by logical @name, if undefined enable @dflt.
 */
व्योम color_scheme_fenable(स्थिर अक्षर *name, स्थिर अक्षर *dflt, खाता *f)
{
	स्थिर अक्षर *seq = color_scheme_get_sequence(name, dflt);

	अगर (!seq)
		वापस;
	color_fenable(seq, f);
}


/*
 * Disable previously enabled color
 */
व्योम color_fdisable(खाता *f)
{
	अगर (!ul_colors.disabled && ul_colors.has_colors)
		ख_माला_दो(UL_COLOR_RESET, f);
}

/*
 * Get reset sequence
 */
स्थिर अक्षर *color_get_disable_sequence(व्योम)
{
	अगर (!ul_colors.disabled && ul_colors.has_colors)
		वापस UL_COLOR_RESET;
	अन्यथा
		वापस "";
}

/*
 * Parses @str to return UL_COLORMODE_*
 */
पूर्णांक colormode_from_string(स्थिर अक्षर *str)
{
	माप_प्रकार i;
	अटल स्थिर अक्षर *स्थिर modes[] = {
		[UL_COLORMODE_AUTO]   = "auto",
		[UL_COLORMODE_NEVER]  = "never",
		[UL_COLORMODE_ALWAYS] = "always",
		[UL_COLORMODE_UNDEF] = ""
	};

	अगर (!str || !*str)
		वापस -EINVAL;

	निश्चित(ARRAY_आकार(modes) == __UL_NCOLORMODES);

	क्रम (i = 0; i < ARRAY_आकार(modes); i++) {
		अगर (c_strcasecmp(str, modes[i]) == 0)
			वापस i;
	}

	वापस -EINVAL;
}

/*
 * Parses @str and exit(EXIT_FAILURE) on error
 */
पूर्णांक colormode_or_err(स्थिर अक्षर *str)
{
	स्थिर अक्षर *p = str && *str == '=' ? str + 1 : str;
	पूर्णांक colormode;

	colormode = colormode_from_string(p);
	अगर (colormode < 0)
		errx(निकास_त्रुटि, _("unsupported color mode: %s"), p);

	वापस colormode;
}

#अगर_घोषित TEST_PROGRAM_COLORS
# include <getopt.h>
पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[])
{
	अटल स्थिर काष्ठा option longopts[] = {
		{ "mode",	required_argument, शून्य, 'm' },
		{ "color",	required_argument, शून्य, 'c' },
		{ "color-scheme", required_argument, शून्य, 'C' },
		{ "name",	required_argument, शून्य, 'n' },
		{ शून्य, 0, शून्य, 0 }
	};
	पूर्णांक c, mode = UL_COLORMODE_UNDEF;	/* default */
	स्थिर अक्षर *color = "red", *name = शून्य, *color_scheme = शून्य;
	अक्षर *seq = शून्य;

	जबतक ((c = getopt_long(argc, argv, "C:c:m:n:", longopts, शून्य)) != -1) {
		चयन (c) {
		हाल 'c':
			color = optarg;
			अवरोध;
		हाल 'C':
			color_scheme = optarg;
			अवरोध;
		हाल 'm':
			mode = colormode_or_err(optarg);
			अवरोध;
		हाल 'n':
			name = optarg;
			अवरोध;
		शेष:
			ख_लिखो(मानक_त्रुटि, "usage: %s [options]\n"
			" -m, --mode <auto|never|always>  default is undefined\n"
			" -c, --color <red|blue|...>      color for the test message\n"
			" -C, --color-scheme <name>       color for the test message\n"
			" -n, --name <utilname>           util name\n",
			program_invocation_short_name);
			वापस निकास_त्रुटि;
		}
	}

	colors_init(mode, name ? name : program_invocation_short_name);

	अगर (color_is_sequence(color))
		seq = strdup(color);
	अन्यथा
		seq = color_get_sequence(color);

	अगर (color_scheme)
		color_scheme_enable(color_scheme, seq);
	अन्यथा
		color_enable(seq);
	म_लिखो("Hello World!");
	color_disable();
	ख_अक्षर_दो('\n', मानक_निकास);

	मुक्त(seq);

	वापस निकास_सफल;
}
#पूर्ण_अगर /* TEST_PROGRAM_COLORS */

