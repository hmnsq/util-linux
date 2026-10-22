/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: 2012 Ondrej Oprala <ooprala@redhat.com>
 *          2012-2025 Karel Zak <kzak@redhat.com>
 */
#అన్తర్భావ <assert.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <dirent.h>
#అన్తర్భావ <ctype.h>

#ఒకవేళ defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
# ఒకవేళ defined(HAVE_NCURSESW_NCURSES_H)
#  include <ncursesw/ncurses.h>
# elif defined(HAVE_NCURSES_NCURSES_H)
#  include <ncurses/ncurses.h>
# elif defined(HAVE_NCURSES_H)
#  include <ncurses.h>
# endif
# ఒకవేళ defined(HAVE_NCURSESW_పదం_H)
#  include <ncursesw/term.h>
# elif defined(HAVE_NCURSES_పదం_H)
#  include <ncurses/term.h>
# elif defined(HAVE_పదం_H)
#  include <term.h>
# endif
#అయితే_ముగింపు

#అన్తర్భావ "c.h"
#అన్తర్భావ "cctype.h"
#అన్తర్భావ "colors.h"
#అన్తర్భావ "nls.h"
#అన్తర్భావ "pathnames.h"
#అన్తర్భావ "strutils.h"

#అన్తర్భావ "debug.h"

/*
 * Default behavior, may be overridden by terminal-colors.d/{enable,disable}.
 */
#నిర్వచితమైతే USE_COLORS_BY_DEFAULT
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_AUTO	/* check isatty() */
#లేకపోతే
# define UL_COLORMODE_DEFAULT	UL_COLORMODE_NEVER	/* no colors by default */
#అయితే_ముగింపు

/*
 * terminal-colors.d debug stuff
 */
స్థిర UL_DEBUG_DEFINE_ముసుగు(termcolors);
UL_DEBUG_DEFINE_MASKNAMES(termcolors) = UL_DEBUG_ఖాళీలు_MASKNAMES;

#నిర్వచించు TERMCOLORS_DEBUG_INIT	(1 << 1)
#నిర్వచించు TERMCOLORS_DEBUG_CONF	(1 << 2)
#నిర్వచించు TERMCOLORS_DEBUG_SCHEME	(1 << 3)

#నిర్వచించు DBG(m, x)       __UL_DBG(termcolors, TERMCOLORS_DEBUG_, m, x)
#నిర్వచించు ON_DBG(m, x)    __UL_DBG_CALL(termcolors, TERMCOLORS_DEBUG_, m, x)

/*
 * terminal-colors.d file types
 */
గణన {
	UL_COLORFILE_DISABLE,		/* .disable */
	UL_COLORFILE_ENABLE,		/* .enable */
	UL_COLORFILE_SCHEME,		/* .scheme */

	__UL_COLORFILE_COUNT
};

నిర్మాణం ul_color_scheme {
	అక్షరం *name;
	అక్షరం *seq;
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
నిర్మాణం ul_color_ctl {
	స్థిరరాశి అక్షరం	*utilname;	/* util name */
	స్థిరరాశి అక్షరం	*termname;	/* terminal name ($TERM) */

	అక్షరం		*sfile;	/* path to scheme */

	నిర్మాణం ul_color_scheme	*schemes;	/* array with color schemes */
	size_t			nschemes;	/* number of the items */
	size_t			schemes_sz;	/* number of the allocated items */

	పూర్ణాంకం		బహుళకం;		/* UL_COLORMODE_* */
	గుర్తులేని పూర్ణాంకం	has_colors : 1,	/* based on mode and scores[] */
			disabled   : 1, /* disable colors */
			cs_configured : 1, /* color schemes read */
			configured : 1; /* terminal-colors.d parsed */

	పూర్ణాంకం		scores[__UL_COLORFILE_COUNT];	/* the best match */
};

/*
 * Control struct, globally shared.
 */
స్థిర నిర్మాణం ul_color_ctl ul_colors;

స్థిర శూన్య colors_free_schemes(నిర్మాణం ul_color_ctl *cc);
స్థిర పూర్ణాంకం colors_read_schemes(నిర్మాణం ul_color_ctl *cc);

/*
 * qsort/bsearch buddy
 */
స్థిర పూర్ణాంకం cmp_scheme_name(స్థిరరాశి శూన్య *a0, స్థిరరాశి శూన్య *b0)
{
	స్థిరరాశి నిర్మాణం ul_color_scheme *a = (స్థిరరాశి నిర్మాణం ul_color_scheme *) a0,
				     *b = (స్థిరరాశి నిర్మాణం ul_color_scheme *) b0;
	తిరిగి strcmp(a->name, b->name);
}

/*
 * Resets control struct (note that we don't allocate the struct)
 */
స్థిర శూన్య colors_reset(నిర్మాణం ul_color_ctl *cc)
{
	ఒకవేళ (!cc)
		తిరిగి;

	colors_free_schemes(cc);

	విడుదల(cc->sfile);

	cc->sfile = ఖాళీ;
	cc->utilname = ఖాళీ;
	cc->termname = ఖాళీ;
	cc->బహుళకం = UL_COLORMODE_UNDEF;

	memset(cc->scores, 0, పరిమాణం(cc->scores));
}

స్థిర శూన్య colors_debug(నిర్మాణం ul_color_ctl *cc)
{
	size_t i;

	ఒకవేళ (!cc)
		తిరిగి;

	రాయి("Colors:\n");
	రాయి("\tutilname = '%s'\n", cc->utilname);
	రాయి("\ttermname = '%s'\n", cc->termname);
	రాయి("\tscheme file = '%s'\n", cc->sfile);
	రాయి("\tmode = %s\n",
			cc->బహుళకం == UL_COLORMODE_UNDEF ? "undefined" :
			cc->బహుళకం == UL_COLORMODE_AUTO ?  "auto" :
			cc->బహుళకం == UL_COLORMODE_NEVER ? "never" :
			cc->బహుళకం == UL_COLORMODE_ALWAYS ? "always" : "???");
	రాయి("\thas_colors = %d\n", cc->has_colors);
	రాయి("\tdisabled = %d\n", cc->disabled);
	రాయి("\tconfigured = %d\n", cc->configured);
	రాయి("\tcs configured = %d\n", cc->cs_configured);

	fputc('\n', stdout);

	కొరకు (i = 0; i < ARRAY_కొలత(cc->scores); i++)
		రాయి("\tscore %s = %d\n",
				i == UL_COLORFILE_DISABLE ? "disable" :
				i == UL_COLORFILE_ENABLE ? "enable" :
				i == UL_COLORFILE_SCHEME ? "scheme" : "???",
				cc->scores[i]);

	fputc('\n', stdout);

	కొరకు (i = 0; i < cc->nschemes; i++) {
		రాయి("\tscheme #%02zu ", i);
		color_scheme_enable(cc->schemes[i].name, ఖాళీ);
		fputs(cc->schemes[i].name, stdout);
		color_disable();
		fputc('\n', stdout);
	}
	fputc('\n', stdout);
}

/*
 * Parses [[<utilname>][@<termname>].]<type>
 */
స్థిర పూర్ణాంకం filename_గమ్యం_టోకెన్లు(స్థిరరాశి అక్షరం *str,
			      స్థిరరాశి అక్షరం **name, size_t *namesz,
			      స్థిరరాశి అక్షరం **పదం, size_t *termsz,
			      పూర్ణాంకం  *filetype)
{
	స్థిరరాశి అక్షరం *type_ప్రారంభం, *పదం_ప్రారంభం, *p;

	ఒకవేళ (!str || !*str || *str == '.' || అక్షరశ్రేణి_పొడవు(str) > PATH_MAX)
		తిరిగి -EINVAL;

	/* parse .type */
	p = strrchr(str, '.');
	type_ప్రారంభం = p ? p + 1 : str;

	ఒకవేళ (strcmp(type_ప్రారంభం, "disable") == 0)
		*filetype = UL_COLORFILE_DISABLE;
	లేకపోతే ఒకవేళ (strcmp(type_ప్రారంభం, "enable") == 0)
		*filetype = UL_COLORFILE_ENABLE;
	లేకపోతే ఒకవేళ (strcmp(type_ప్రారంభం, "scheme") == 0)
		*filetype = UL_COLORFILE_SCHEME;
	లేకపోతే {
		DBG(CONF, ul_debug("unknown type '%s'", type_ప్రారంభం));
		తిరిగి 1;				/* unknown type */
	}

	ఒకవేళ (type_ప్రారంభం == str)
		తిరిగి 0;				/* "type" only */

	/* parse @termname */
	p = strchr(str, '@');
	పదం_ప్రారంభం = p ? p + 1 : ఖాళీ;
	ఒకవేళ (పదం_ప్రారంభం) {
		*పదం = పదం_ప్రారంభం;
		*termsz = type_ప్రారంభం - పదం_ప్రారంభం - 1;
		ఒకవేళ (పదం_ప్రారంభం - 1 == str)
			తిరిగి 0;			/* "@termname.type" */
	}

	/* parse utilname */
	p = పదం_ప్రారంభం ? పదం_ప్రారంభం : type_ప్రారంభం;
	*name = str;
	*namesz = p - str - 1;

	తిరిగి 0;
}

/*
 * Scans @dirname and select the best matches for UL_COLORFILE_* types.
 * The result is stored to cc->scores. The path to the best "scheme"
 * file is stored to cc->scheme.
 */
స్థిర పూర్ణాంకం colors_readdir(నిర్మాణం ul_color_ctl *cc, స్థిరరాశి అక్షరం *dirname)
{
	DIR *dir;
	పూర్ణాంకం rc = -ENOENT;
	నిర్మాణం dirent *d;
	అక్షరం sfile[PATH_MAX] = { '\0' };
	size_t namesz, termsz;

	ఒకవేళ (!dirname || !cc || !cc->utilname || !*cc->utilname)
		తిరిగి -EINVAL;

	DBG(CONF, ul_debug("reading dir: '%s'", dirname));

	dir = opendir(dirname);
	ఒకవేళ (!dir)
		తిరిగి -errno;

	namesz = అక్షరశ్రేణి_పొడవు(cc->utilname);
	termsz = cc->termname ? అక్షరశ్రేణి_పొడవు(cc->termname) : 0;

	అయితే ((d = readdir(dir))) {
		పూర్ణాంకం type, score = 1;
		స్థిరరాశి అక్షరం *tk_name = ఖాళీ, *tk_పదం = ఖాళీ;
		size_t tk_namesz = 0, tk_termsz = 0;

		ఒకవేళ (*d->d_name == '.')
			కొనసాగు;
#నిర్వచితమైతే _DIRENT_HAVE_D_TYPE
		ఒకవేళ (d->d_type != DT_UNKNOWN && d->d_type != DT_LNK &&
		    d->d_type != DT_REG)
			కొనసాగు;
#అయితే_ముగింపు
		ఒకవేళ (filename_గమ్యం_టోకెన్లు(d->d_name,
				       &tk_name, &tk_namesz,
				       &tk_పదం, &tk_termsz, &type) != 0)
			కొనసాగు;

		/* count theoretical score before we check names to avoid
		 * unnecessary strcmp() */
		ఒకవేళ (tk_name)
			score += 20;
		ఒకవేళ (tk_పదం)
			score += 10;

		DBG(CONF, ul_debug("item '%s': score=%d "
			"[cur: %d, name(%zu): %s, term(%zu): %s]",
			d->d_name, score, cc->scores[type],
			tk_namesz, tk_name,
			tk_termsz, tk_పదం));


		ఒకవేళ (score < cc->scores[type])
			కొనసాగు;

		/* filter out by names */
		ఒకవేళ (tk_namesz && (tk_namesz != namesz ||
				 అక్షరశ్రేణి_పోలిక(tk_name, cc->utilname, namesz) != 0))
			కొనసాగు;

		ఒకవేళ (tk_termsz && (termsz == 0 || tk_termsz != termsz ||
				  అక్షరశ్రేణి_పోలిక(tk_పదం, cc->termname, termsz) != 0))
			కొనసాగు;

		DBG(CONF, ul_debug("setting '%s' from %d -to-> %d",
					type == UL_COLORFILE_SCHEME ? "scheme" :
					type == UL_COLORFILE_DISABLE ? "disable" :
					type == UL_COLORFILE_ENABLE ? "enable" : "???",
					cc->scores[type], score));
		cc->scores[type] = score;
		rc = 0;
		ఒకవేళ (type == UL_COLORFILE_SCHEME)
			strncpy(sfile, d->d_name, పరిమాణం(sfile));
	}

	ఒకవేళ (*sfile) {
		sfile[పరిమాణం(sfile) - 1] = '\0';
		ఒకవేళ (asprintf(&cc->sfile, "%s/%s", dirname, sfile) <= 0)
			rc = -ENOMEM;
	}

	closedir(dir);
	తిరిగి rc;
}

/* atexit() wrapper */
స్థిర శూన్య colors_deinit(శూన్య)
{
	colors_reset(&ul_colors);
}

/*
 * Returns path to $XDG_CONFIG_HOME/terminal-colors.d
 */
స్థిర అక్షరం *colors_get_homedir(అక్షరం *buf, size_t bufsz)
{
	అక్షరం *p = getenv("XDG_CONFIG_HOME");

	ఒకవేళ (p) {
		snprintf(buf, bufsz, "%s/" మార్గం_TERMCOLORS_DIRNAME, p);
		తిరిగి buf;
	}

	p = getenv("HOME");
	ఒకవేళ (p) {
		snprintf(buf, bufsz, "%s/.config/" మార్గం_TERMCOLORS_DIRNAME, p);
		తిరిగి buf;
	}

	తిరిగి ఖాళీ;
}

/*
 * Adds one color sequence to array with color scheme.
 */
స్థిర పూర్ణాంకం colors_కూడు_scheme(నిర్మాణం ul_color_ctl *cc,
			     స్థిరరాశి అక్షరం *name,
			     స్థిరరాశి అక్షరం *seq0)
{
	నిర్మాణం ul_color_scheme *cs = ఖాళీ;
	అక్షరం *seq = ఖాళీ;
	పూర్ణాంకం rc;

	ఒకవేళ (!cc || !name || !*name || !seq0 || !*seq0)
		తిరిగి -EINVAL;

	DBG(SCHEME, ul_debug("add '%s'", name));

	seq = color_get_sequence(seq0);
	ఒకవేళ (!seq)
		తిరిగి -EINVAL;
	rc = -ENOMEM;

	/* enlarge the array */
	ఒకవేళ (cc->nschemes == cc->schemes_sz) {
		శూన్య *tmp = reallocarray(cc->schemes, cc->nschemes + 10,
					 పరిమాణం(నిర్మాణం ul_color_scheme));
		ఒకవేళ (!tmp)
			వెళ్ళు err;
		cc->schemes = tmp;
		cc->schemes_sz = cc->nschemes + 10;
	}

	/* add a new item */
	cs = &cc->schemes[cc->nschemes];
	cs->seq = seq;
	cs->name = strdup(name);
	ఒకవేళ (!cs->name)
		వెళ్ళు err;

	cc->nschemes++;
	తిరిగి 0;
err:
	ఒకవేళ (cs) {
		విడుదల(cs->seq);
		విడుదల(cs->name);
		cs->seq = cs->name = ఖాళీ;
	} లేకపోతే
		విడుదల(seq);
	తిరిగి rc;
}

/*
 * Deallocates all regards to color schemes
 */
స్థిర శూన్య colors_free_schemes(నిర్మాణం ul_color_ctl *cc)
{
	size_t i;

	DBG(SCHEME, ul_debug("free scheme"));

	కొరకు (i = 0; i < cc->nschemes; i++) {
		విడుదల(cc->schemes[i].name);
		విడుదల(cc->schemes[i].seq);
	}

	విడుదల(cc->schemes);
	cc->schemes = ఖాళీ;
	cc->nschemes = 0;
	cc->schemes_sz = 0;
}

/*
 * The scheme configuration has to be sorted for bsearch
 */
స్థిర శూన్య colors_క్రమీకరించు_schemes(నిర్మాణం ul_color_ctl *cc)
{
	ఒకవేళ (!cc->nschemes)
		తిరిగి;

	DBG(SCHEME, ul_debug("sort scheme"));

	qsort(cc->schemes, cc->nschemes,
	      పరిమాణం(నిర్మాణం ul_color_scheme), cmp_scheme_name);
}

/*
 * Returns just one color scheme
 */
స్థిర నిర్మాణం ul_color_scheme *colors_get_scheme(నిర్మాణం ul_color_ctl *cc,
						 స్థిరరాశి అక్షరం *name)
{
	నిర్మాణం ul_color_scheme కీలకం = { .name = (అక్షరం *) name}, *res;

	ఒకవేళ (!cc || !name || !*name)
		తిరిగి ఖాళీ;

	ఒకవేళ (!cc->cs_configured) {
		పూర్ణాంకం rc = colors_read_schemes(cc);
		ఒకవేళ (rc)
			తిరిగి ఖాళీ;
	}
	ఒకవేళ (!cc->nschemes)
		తిరిగి ఖాళీ;

	DBG(SCHEME, ul_debug("search '%s'", name));

	res = bsearch(&కీలకం, cc->schemes, cc->nschemes,
				పరిమాణం(నిర్మాణం ul_color_scheme),
				cmp_scheme_name);

	తిరిగి res && res->seq ? res  : ఖాళీ;
}

/*
 * Parses filenames in terminal-colors.d
 */
స్థిర పూర్ణాంకం colors_read_configuration(నిర్మాణం ul_color_ctl *cc)
{
	పూర్ణాంకం rc = -ENOENT;
	అక్షరం *dirname, buf[PATH_MAX];

	cc->termname = getenv("TERM");

	dirname = colors_get_homedir(buf, పరిమాణం(buf));
	ఒకవేళ (dirname)
		rc = colors_readdir(cc, dirname);		/* ~/.config */
	ఒకవేళ (rc == -EPERM || rc == -EACCES || rc == -ENOENT)
		rc = colors_readdir(cc, మార్గం_TERMCOLORS_DIR);	/* /etc */

	cc->configured = 1;
	తిరిగి rc;
}

/*
 * Reads terminal-colors.d/ scheme file into array schemes
 */
స్థిర పూర్ణాంకం colors_read_schemes(నిర్మాణం ul_color_ctl *cc)
{
	పూర్ణాంకం rc = 0;
	FILE *f = ఖాళీ;
	అక్షరం buf[BUFSIZ],
	     cn[129], seq[129];

	ఒకవేళ (!cc->configured)
		rc = colors_read_configuration(cc);

	cc->cs_configured = 1;

	ఒకవేళ (rc || !cc->sfile)
		వెళ్ళు పూర్తి;

	DBG(SCHEME, ul_debug("reading file '%s'", cc->sfile));

	f = fopen(cc->sfile, "r" UL_CLOEXECSTR);
	ఒకవేళ (!f) {
		rc = -errno;
		వెళ్ళు పూర్తి;
	}

	అయితే (fgets(buf, పరిమాణం(buf), f)) {
		అక్షరం *p = strchr(buf, '\n');

		ఒకవేళ (!p) {
			ఒకవేళ (feof(f))
				p = strchr(buf, '\0');
			లేకపోతే {
				rc = -errno;
				వెళ్ళు పూర్తి;
			}
		}
		*p = '\0';
		p = (అక్షరం *) skip_blank(buf);
		ఒకవేళ (*p == '\0' || *p == '#')
			కొనసాగు;

		rc = అక్షరశ్రేణి_చదువు(p, "%128[^ ] %128[^\n ]", cn, seq);
		ఒకవేళ (rc == 2 && *cn && *seq) {
			rc = colors_కూడు_scheme(cc, cn, seq);	/* set rc=0 on success */
			ఒకవేళ (rc)
				వెళ్ళు పూర్తి;
		}
	}
	rc = 0;

పూర్తి:
	ఒకవేళ (f)
		fclose(f);
	colors_క్రమీకరించు_schemes(cc);

	తిరిగి rc;
}


స్థిర శూన్య termcolors_init_debug(శూన్య)
{
	__UL_INIT_DEBUG_FROM_ENV(termcolors, TERMCOLORS_DEBUG_, 0, TERMINAL_COLORS_DEBUG);
}

స్థిర పూర్ణాంకం colors_terminal_is_ready(శూన్య)
{
	పూర్ణాంకం ncolors = -1;

#ఒకవేళ defined(HAVE_LIBNCURSES) || defined(HAVE_LIBNCURSESW)
	{
		పూర్ణాంకం ret;

		/* setupterm() allocates memory, del_curterm() deallocates it */
		ఒకవేళ (setupterm(ఖాళీ, STDOUT_FILENO, &ret) == 0 && ret == 1) {
			ncolors = tigetnum("colors");
			del_curterm(cur_పదం);
		}
	}
#అయితే_ముగింపు
	ఒకవేళ (1 < ncolors) {
		DBG(CONF, ul_debug("terminal is ready (supports %d colors)", ncolors));
		తిరిగి 1;
	}

	DBG(CONF, ul_debug("terminal is NOT ready (no colors)"));
	తిరిగి 0;
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
పూర్ణాంకం colors_init(పూర్ణాంకం బహుళకం, స్థిరరాశి అక్షరం *name)
{
	పూర్ణాంకం ready = -1;
	నిర్మాణం ul_color_ctl *cc = &ul_colors;

	cc->utilname = name;

	termcolors_init_debug();

	ఒకవేళ (బహుళకం != UL_COLORMODE_ALWAYS && !isatty(STDOUT_FILENO))
		cc->బహుళకం = UL_COLORMODE_NEVER;
	లేకపోతే
		cc->బహుళకం = బహుళకం;

	ఒకవేళ (cc->బహుళకం == UL_COLORMODE_UNDEF
	    && getenv("NO_COLOR") == ఖాళీ
	    && (ready = colors_terminal_is_ready())) {

		పూర్ణాంకం rc = colors_read_configuration(cc);
		ఒకవేళ (rc)
			cc->బహుళకం = UL_COLORMODE_DEFAULT;
		లేకపోతే {

			/* evaluate scores */
			ఒకవేళ (cc->scores[UL_COLORFILE_DISABLE] >
			    cc->scores[UL_COLORFILE_ENABLE])
				cc->బహుళకం = UL_COLORMODE_NEVER;
			లేకపోతే
				cc->బహుళకం = UL_COLORMODE_DEFAULT;

			atexit(colors_deinit);
		}
	}

	ఎంపిక (cc->బహుళకం) {
	స్థితి UL_COLORMODE_AUTO:
		cc->has_colors = ready == -1 ? colors_terminal_is_ready() : ready;
		ఆపు;
	స్థితి UL_COLORMODE_ALWAYS:
		cc->has_colors = 1;
		ఆపు;
	స్థితి UL_COLORMODE_NEVER:
	అప్రమేయ:
		cc->has_colors = 0;
	}

	ON_DBG(CONF, colors_debug(cc));

	తిరిగి cc->has_colors;
}

/*
 * Temporary disable colors (this setting is independent on terminal-colors.d/)
 */
శూన్య colors_off(శూన్య)
{
	ul_colors.disabled = 1;
}

/*
 * Enable colors
 */
శూన్య colors_on(శూన్య)
{
	ul_colors.disabled = 0;
}

/*
 * Is terminal-colors.d/ configured to use colors?
 */
పూర్ణాంకం colors_wanted(శూన్య)
{
	తిరిగి ul_colors.has_colors;
}

/*
 * Returns mode
 */
పూర్ణాంకం colors_బహుళకం(శూన్య)
{
	తిరిగి ul_colors.బహుళకం;
}

/*
 * Enable @seq color
 */
శూన్య color_fenable(స్థిరరాశి అక్షరం *seq, FILE *f)
{
	ఒకవేళ (!ul_colors.disabled && ul_colors.has_colors && seq)
		fputs(seq, f);
}

/*
 * Returns escape sequence by logical @name, if undefined then returns @dflt.
 */
స్థిరరాశి అక్షరం *color_scheme_get_sequence(స్థిరరాశి అక్షరం *name, స్థిరరాశి అక్షరం *dflt)
{
	నిర్మాణం ul_color_scheme *cs;

	ఒకవేళ (ul_colors.disabled || !ul_colors.has_colors)
		తిరిగి ఖాళీ;

	cs = colors_get_scheme(&ul_colors, name);
	తిరిగి cs && cs->seq ? cs->seq : dflt;
}

/*
 * Enable color by logical @name, if undefined enable @dflt.
 */
శూన్య color_scheme_fenable(స్థిరరాశి అక్షరం *name, స్థిరరాశి అక్షరం *dflt, FILE *f)
{
	స్థిరరాశి అక్షరం *seq = color_scheme_get_sequence(name, dflt);

	ఒకవేళ (!seq)
		తిరిగి;
	color_fenable(seq, f);
}


/*
 * Disable previously enabled color
 */
శూన్య color_fdisable(FILE *f)
{
	ఒకవేళ (!ul_colors.disabled && ul_colors.has_colors)
		fputs(UL_COLOR_RESET, f);
}

/*
 * Get reset sequence
 */
స్థిరరాశి అక్షరం *color_get_disable_sequence(శూన్య)
{
	ఒకవేళ (!ul_colors.disabled && ul_colors.has_colors)
		తిరిగి UL_COLOR_RESET;
	లేకపోతే
		తిరిగి "";
}

/*
 * Parses @str to return UL_COLORMODE_*
 */
పూర్ణాంకం colormode_from_string(స్థిరరాశి అక్షరం *str)
{
	size_t i;
	స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి modes[] = {
		[UL_COLORMODE_AUTO]   = "auto",
		[UL_COLORMODE_NEVER]  = "never",
		[UL_COLORMODE_ALWAYS] = "always",
		[UL_COLORMODE_UNDEF] = ""
	};

	ఒకవేళ (!str || !*str)
		తిరిగి -EINVAL;

	ప్రతిజ్ఞా(ARRAY_కొలత(modes) == __UL_NCOLORMODES);

	కొరకు (i = 0; i < ARRAY_కొలత(modes); i++) {
		ఒకవేళ (c_strcasecmp(str, modes[i]) == 0)
			తిరిగి i;
	}

	తిరిగి -EINVAL;
}

/*
 * Parses @str and exit(EXIT_FAILURE) on error
 */
పూర్ణాంకం colormode_or_err(స్థిరరాశి అక్షరం *str)
{
	స్థిరరాశి అక్షరం *p = str && *str == '=' ? str + 1 : str;
	పూర్ణాంకం colormode;

	colormode = colormode_from_string(p);
	ఒకవేళ (colormode < 0)
		errx(EXIT_FAILURE, _("unsupported color mode: %s"), p);

	తిరిగి colormode;
}

#నిర్వచితమైతే TEST_కార్యక్రమం_COLORS
# include <getopt.h>
పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[])
{
	స్థిర స్థిరరాశి నిర్మాణం option longopts[] = {
		{ "mode",	required_argument, ఖాళీ, 'm' },
		{ "color",	required_argument, ఖాళీ, 'c' },
		{ "color-scheme", required_argument, ఖాళీ, 'C' },
		{ "name",	required_argument, ఖాళీ, 'n' },
		{ ఖాళీ, 0, ఖాళీ, 0 }
	};
	పూర్ణాంకం c, బహుళకం = UL_COLORMODE_UNDEF;	/* default */
	స్థిరరాశి అక్షరం *color = "red", *name = ఖాళీ, *color_scheme = ఖాళీ;
	అక్షరం *seq = ఖాళీ;

	అయితే ((c = getopt_long(argc, argv, "C:c:m:n:", longopts, ఖాళీ)) != -1) {
		ఎంపిక (c) {
		స్థితి 'c':
			color = optarg;
			ఆపు;
		స్థితి 'C':
			color_scheme = optarg;
			ఆపు;
		స్థితి 'm':
			బహుళకం = colormode_or_err(optarg);
			ఆపు;
		స్థితి 'n':
			name = optarg;
			ఆపు;
		అప్రమేయ:
			fprintf(stderr, "usage: %s [options]\n"
			" -m, --mode <auto|never|always>  default is undefined\n"
			" -c, --color <red|blue|...>      color for the test message\n"
			" -C, --color-scheme <name>       color for the test message\n"
			" -n, --name <utilname>           util name\n",
			కార్యక్రమం_invocation_short_name);
			తిరిగి EXIT_FAILURE;
		}
	}

	colors_init(బహుళకం, name ? name : కార్యక్రమం_invocation_short_name);

	ఒకవేళ (color_is_sequence(color))
		seq = strdup(color);
	లేకపోతే
		seq = color_get_sequence(color);

	ఒకవేళ (color_scheme)
		color_scheme_enable(color_scheme, seq);
	లేకపోతే
		color_enable(seq);
	రాయి("Hello World!");
	color_disable();
	fputc('\n', stdout);

	విడుదల(seq);

	తిరిగి EXIT_SUCCESS;
}
#అయితే_ముగింపు /* TEST_PROGRAM_COLORS */

