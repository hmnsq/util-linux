/*
 * configs_file.c instantiates functions defined and described in configs_file.h
 */
#ضمن <errno.h>
#ضمن <sys/syslog.h>
#ضمن <sys/stat.h>
#ضمن <stdarg.h>
#ضمن <stdbool.h>
#ضمن <string.h>
#ضمن <stdlib.h>
#ضمن <stdio.h>
#إذا defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
#ضمن <dirent.h>
#د_که_پای

#ضمن "c.h"
#ضمن "configs.h"
#ضمن "strutils.h"
#ضمن "list.h"
#ضمن "fileutils.h"

بنية file_element {
	بنية list_سر file_list;
	حرف *filename;
};

ساكن __attribute__ ((__format__ (__printf__, 2, 0)))
	حرف *config_mk_لار(صحيح types, ثابت حرف *fmt, ...)
{
	va_list ap;
	حرف *لار = عدم;
	صحيح rc;

	errno = 0;

	va_start(ap, fmt);
	rc = vasprintf(&لار, fmt, ap);
	va_end(ap);

	إذا (rc > 0 && types) {
		بنية stat st;

		إذا (stat(لار, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			اذهب fail;

	}

	أرجع لار;
fail:
	حرر(لار);
	أرجع عدم;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
ساكن حرف *main_configs(ثابت حرف *ریښه,
			  ثابت حرف *project,
			  ثابت حرف *confname,
			  ثابت حرف *suffix)
{
	حرف *لار = عدم;

	إذا (suffix)
		لار = config_mk_لار(S_IFREG, "%s/%s/%s.%s",
				ریښه, project, confname, suffix);
	إذا (!لار)
		لار = config_mk_لار(S_IFREG, "%s/%s/%s",
				ریښه, project, confname);
	أرجع لار;
}

ساكن صحيح configs_refer_filename(بنية list_سر *list, حرف *filename, صحيح سر)
{
	بنية file_element *e;

	e = حافظه_صفر_ورکړه(1, حجم(*e));
	إذا (e == عدم)
		أرجع -ENOMEM;

	INIT_LIST_سر(&e->file_list);
	e->filename = filename;
	إذا (سر)
		list_جمع_کړه(&e->file_list, list);
	وإلا
		list_جمع_کړه_لکۍ(&e->file_list, list);
	أرجع 0;
}

#إذا defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

ساكن صحيح filter(ثابت بنية dirent *d)
{
#که_تعریف_شوی _DIRENT_HAVE_D_TYPE
	إذا (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		أرجع 0;
#د_که_پای
	إذا (is_dotdir_dirent(d))
		أرجع 0;

	أرجع 1;
}

ساكن صحيح read_dir(بنية list_سر *file_list,
		    ثابت حرف *project,
		    ثابت حرف *ریښه,
		    ثابت حرف *confname,
		    ثابت حرف *suffix)
{
	حرف *dirname = عدم;
	حرف *filename = عدم;
	صحيح dd = -1, nfiles = 0, i;
	صحيح ret = 0;
	بنية dirent **namelist = عدم;

	إذا (suffix)
		dirname = config_mk_لار(S_IFDIR, "%s/%s/%s.%s.d",
				ریښه, project, confname, suffix);
	إذا (!dirname)
		dirname = config_mk_لار(S_IFDIR, "%s/%s/%s.d",
				ریښه, project, confname);
	إذا (!dirname)
		أرجع errno == ENOMEM ? -ENOMEM : 0;

	dd = open(dirname, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	إذا (dd < 0)
		اذهب پای;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	إذا (nfiles <= 0)
		اذهب پای;

	لكل (i = 0; i < nfiles; i++) {
		بنية dirent *d = namelist[i];

		إذا (suffix) {
			ثابت حرف *p = ul_endswith(d->d_name, suffix);

			إذا (!p || p == d->d_name || *(p - 1) != '.')
				استمر;
		}

		إذا (asprintf(&filename, "%s/%s", dirname, d->d_name) < 0) {
			ret = -ENOMEM;
			اكسر;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		إذا (ret < 0) {
			حرر(filename);
			اكسر;
		}
	}

پای:
	لكل (i = 0; i < nfiles; i++)
		حرر(namelist[i]);
	حرر(namelist);
	حرر(dirname);
	إذا (dd >= 0)
		close(dd);
	أرجع ret;
}

#د_که_پای /* HAVE_SCANDIRAT */

ساكن صحيح config_cmp(بنية list_سر *a, بنية list_سر *b,
		      فراغ *معلومات __attribute__((__unused__)))
{
	بنية file_element *ea = list_ثبت(a, بنية file_element, file_list);
	بنية file_element *eb = list_ثبت(b, بنية file_element, file_list);
	حرف *na = ul_basename(ea->filename);
	حرف *nb = ul_basename(eb->filename);

	أرجع strcoll(na, nb);
}

ساكن فراغ free_list_ثبت(بنية file_element *element)
{
	حرر(element->filename);
	حرر(element);
}

ساكن صحيح config_یوځای_list(بنية list_سر *main_list,
			      بنية list_سر *new_list)
{
	بنية list_سر *n, *m, *بل;

	list_for_each_خوندي(n, بل, new_list) {
		بنية file_element *ne = list_ثبت(n, بنية file_element, file_list);
		حرف *nn = ul_basename(ne->filename);
		صحيح duplicate = 0;

		list_for_each(m, main_list) {
			بنية file_element *me = list_ثبت(m, بنية file_element, file_list);
			حرف *mn = ul_basename(me->filename);

			إذا (strcoll(mn, nn) == 0) {
				duplicate = 1;
				اكسر;
			}
		}

		إذا (!duplicate) {
			list_del_init(n);
			list_جمع_کړه_لکۍ(n, main_list);
		}
	}

	list_ترتیبول(main_list, config_cmp, عدم);
	أرجع 0;
}

صحيح ul_configs_file_list(بنية list_سر *file_list,
			 ثابت حرف *project,
			 ثابت حرف *etcdir,
			 ثابت حرف *rundir,
			 ثابت حرف *usrdir,
			 ثابت حرف *confname,
			 ثابت حرف *suffix)
{
	حرف *main_file = عدم;
	بنية list_سر etc_list;
	بنية list_سر پرلپسې_list;
	بنية list_سر usr_list;
	صحيح ret;

	INIT_LIST_سر(file_list);

	إذا (!confname)
		أرجع -ENOTEMPTY;

	/* Default is /etc */
	إذا (!etcdir)
		etcdir = لار_SYSCONFDIR;
	إذا (!rundir)
		rundir = "";
	إذا (!usrdir)
		usrdir = "";

	إذا (!project)
		project = "";

	/* Find "main" config file (but don't add to list yet) */
	/* Search order: /etc /run /usr */
	main_file = main_configs(etcdir, project, confname, suffix);
	إذا (main_file == عدم)
		main_file = main_configs(rundir, project, confname, suffix);
	إذا (main_file == عدم)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_سر(&etc_list);
	INIT_LIST_سر(&پرلپسې_list);
	INIT_LIST_سر(&usr_list);

#إذا defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	إذا (ret == -ENOMEM)
		اذهب پای;

	ret = read_dir(&پرلپسې_list, project, rundir, confname, suffix);
	إذا (ret == -ENOMEM)
		اذهب پای;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	إذا (ret == -ENOMEM)
		اذهب پای;
#د_که_پای

	/* Merge drop-in directories in priority order (high to low) */
	إذا ((ret = config_یوځای_list(file_list, &etc_list)) < 0 ||
	    (ret = config_یوځای_list(file_list, &پرلپسې_list)) < 0 ||
	    (ret = config_یوځای_list(file_list, &usr_list)) < 0)
		اذهب پای;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	إذا (main_file != عدم) {
		ret = configs_refer_filename(file_list, main_file, 1);
		إذا (ret < 0) {
			حرر(main_file);
			اذهب پای;
		}
	}

پای:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&پرلپسې_list);
	ul_configs_free_list(&usr_list);

	أرجع ret < 0 ? ret : (صحيح) list_شمېر_entries(file_list);
}

فراغ ul_configs_free_list(بنية list_سر *file_list)
{
	list_free(file_list, بنية file_element,  file_list, free_list_ثبت);
}

صحيح ul_configs_بل_filename(بنية list_سر *file_list,
			     بنية list_سر **current_ثبت,
			     حرف **name)
{
	بنية file_element *element = عدم;

	إذا (list_تشه(file_list) || *current_ثبت == file_list)
		أرجع 1;

	إذا (*current_ثبت == عدم)
		*current_ثبت = file_list->بل;

	element = list_ثبت(*current_ثبت, بنية file_element, file_list);
	*name = element->filename;
	*current_ثبت = (*current_ثبت)->بل;

	أرجع 0;
}

#که_تعریف_شوی TEST_پروګرام_CONFIGS
# include <getopt.h>

صحيح اصلي(صحيح argc, حرف *argv[])
{
	بنية list_سر file_list;
	بنية list_سر *current = عدم;
	حرف *name = عدم;
	ثابت حرف *etc_لار = عدم;
	ثابت حرف *پرلپسې_لار = عدم;
	ثابت حرف *usr_لار = عدم;
	ثابت حرف *project = عدم;
	ثابت حرف *config_name = عدم;
	ثابت حرف *config_suffix = عدم;
	ساكن ثابت بنية option longopts[] = {
		{ "etc", required_argument, عدم, 'e' },
		{ "run", required_argument, عدم, 'r' },
		{ "usr", required_argument, عدم, 'u' },
		{ "project", required_argument, عدم, 'p' },
		{ "name", required_argument, عدم, 'n' },
		{ "suffix", required_argument, عدم, 's' },
		{ "help", no_argument, عدم, 'h' },
		{ عدم, 0, عدم, 0 }
	};
	صحيح ch, شمېر;

	طالما ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, عدم)) != -1) {
		بدل (ch) {
		حالة 'e':
			etc_لار = optarg;
			اكسر;
		حالة 'r':
			پرلپسې_لار = optarg;
			اكسر;
		حالة 'u':
			usr_لار = optarg;
			اكسر;
		حالة 'p':
			project = optarg;
			اكسر;
		حالة 'n':
			config_name = optarg;
			اكسر;
		حالة 's':
			config_suffix = optarg;
			اكسر;
		حالة 'h':
			چاپ_کړه("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				پروګرام_invocation_short_name);
			أرجع EXIT_SUCCESS;
		افتراضي:
			أرجع EXIT_FAILURE;
		}
	}

	إذا (!config_name) {
		fprintf(stderr, "config name is required (use --name)\n");
		أرجع EXIT_FAILURE;
	}

	شمېر = ul_configs_file_list(&file_list, project, etc_لار, پرلپسې_لار, usr_لار,
				     config_name, config_suffix);
	إذا (شمېر < 0) {
		fprintf(stderr, "failed to get config file list: %d\n", شمېر);
		أرجع EXIT_FAILURE;
	}

	چاپ_کړه("Found %d configuration file(s):\n", شمېر);
	طالما (ul_configs_بل_filename(&file_list, &current, &name) == 0)
		چاپ_کړه("  %s\n", name);

	ul_configs_free_list(&file_list);
	أرجع EXIT_SUCCESS;
}
#د_که_پای /* TEST_PROGRAM_CONFIGS */
