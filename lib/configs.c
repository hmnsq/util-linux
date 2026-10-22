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
#جے_انت

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
	حرف *config_mk_رستہ(صحيح types, ثابت حرف *fmt, ...)
{
	va_list ap;
	حرف *رستہ = عدم;
	صحيح rc;

	errno = 0;

	va_start(ap, fmt);
	rc = vasprintf(&رستہ, fmt, ap);
	va_end(ap);

	إذا (rc > 0 && types) {
		بنية stat st;

		إذا (stat(رستہ, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			اذهب fail;

	}

	أرجع رستہ;
fail:
	حرر(رستہ);
	أرجع عدم;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
ساكن حرف *main_configs(ثابت حرف *جڑ,
			  ثابت حرف *project,
			  ثابت حرف *confname,
			  ثابت حرف *suffix)
{
	حرف *رستہ = عدم;

	إذا (suffix)
		رستہ = config_mk_رستہ(S_IFREG, "%s/%s/%s.%s",
				جڑ, project, confname, suffix);
	إذا (!رستہ)
		رستہ = config_mk_رستہ(S_IFREG, "%s/%s/%s",
				جڑ, project, confname);
	أرجع رستہ;
}

ساكن صحيح configs_refer_filename(بنية list_سر *list, حرف *filename, صحيح سر)
{
	بنية file_element *e;

	e = صفر_حافظہ_دیو(1, حجم(*e));
	إذا (e == عدم)
		أرجع -ENOMEM;

	INIT_LIST_سر(&e->file_list);
	e->filename = filename;
	إذا (سر)
		list_جوڑو(&e->file_list, list);
	وإلا
		list_جوڑو_پوچھ(&e->file_list, list);
	أرجع 0;
}

#إذا defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

ساكن صحيح filter(ثابت بنية dirent *d)
{
#جے_تعریف_شدہ _DIRENT_HAVE_D_TYPE
	إذا (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		أرجع 0;
#جے_انت
	إذا (is_dotdir_dirent(d))
		أرجع 0;

	أرجع 1;
}

ساكن صحيح read_dir(بنية list_سر *file_list,
		    ثابت حرف *project,
		    ثابت حرف *جڑ,
		    ثابت حرف *confname,
		    ثابت حرف *suffix)
{
	حرف *dirname = عدم;
	حرف *filename = عدم;
	صحيح dd = -1, nfiles = 0, i;
	صحيح ret = 0;
	بنية dirent **namelist = عدم;

	إذا (suffix)
		dirname = config_mk_رستہ(S_IFDIR, "%s/%s/%s.%s.d",
				جڑ, project, confname, suffix);
	إذا (!dirname)
		dirname = config_mk_رستہ(S_IFDIR, "%s/%s/%s.d",
				جڑ, project, confname);
	إذا (!dirname)
		أرجع errno == ENOMEM ? -ENOMEM : 0;

	dd = open(dirname, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	إذا (dd < 0)
		اذهب انت;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	إذا (nfiles <= 0)
		اذهب انت;

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

انت:
	لكل (i = 0; i < nfiles; i++)
		حرر(namelist[i]);
	حرر(namelist);
	حرر(dirname);
	إذا (dd >= 0)
		close(dd);
	أرجع ret;
}

#جے_انت /* HAVE_SCANDIRAT */

ساكن صحيح config_cmp(بنية list_سر *a, بنية list_سر *b,
		      فراغ *انکڑے __attribute__((__unused__)))
{
	بنية file_element *ea = list_اندراج(a, بنية file_element, file_list);
	بنية file_element *eb = list_اندراج(b, بنية file_element, file_list);
	حرف *na = ul_basename(ea->filename);
	حرف *nb = ul_basename(eb->filename);

	أرجع strcoll(na, nb);
}

ساكن فراغ free_list_اندراج(بنية file_element *element)
{
	حرر(element->filename);
	حرر(element);
}

ساكن صحيح config_ملاؤ_list(بنية list_سر *main_list,
			      بنية list_سر *new_list)
{
	بنية list_سر *n, *m, *اگلا;

	list_for_each_محفوظ(n, اگلا, new_list) {
		بنية file_element *ne = list_اندراج(n, بنية file_element, file_list);
		حرف *nn = ul_basename(ne->filename);
		صحيح duplicate = 0;

		list_for_each(m, main_list) {
			بنية file_element *me = list_اندراج(m, بنية file_element, file_list);
			حرف *mn = ul_basename(me->filename);

			إذا (strcoll(mn, nn) == 0) {
				duplicate = 1;
				اكسر;
			}
		}

		إذا (!duplicate) {
			list_del_init(n);
			list_جوڑو_پوچھ(n, main_list);
		}
	}

	list_ترتیب_دیو(main_list, config_cmp, عدم);
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
	بنية list_سر لگاتار_list;
	بنية list_سر usr_list;
	صحيح ret;

	INIT_LIST_سر(file_list);

	إذا (!confname)
		أرجع -ENOTEMPTY;

	/* Default is /etc */
	إذا (!etcdir)
		etcdir = رستہ_SYSCONFDIR;
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
	INIT_LIST_سر(&لگاتار_list);
	INIT_LIST_سر(&usr_list);

#إذا defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	إذا (ret == -ENOMEM)
		اذهب انت;

	ret = read_dir(&لگاتار_list, project, rundir, confname, suffix);
	إذا (ret == -ENOMEM)
		اذهب انت;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	إذا (ret == -ENOMEM)
		اذهب انت;
#جے_انت

	/* Merge drop-in directories in priority order (high to low) */
	إذا ((ret = config_ملاؤ_list(file_list, &etc_list)) < 0 ||
	    (ret = config_ملاؤ_list(file_list, &لگاتار_list)) < 0 ||
	    (ret = config_ملاؤ_list(file_list, &usr_list)) < 0)
		اذهب انت;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	إذا (main_file != عدم) {
		ret = configs_refer_filename(file_list, main_file, 1);
		إذا (ret < 0) {
			حرر(main_file);
			اذهب انت;
		}
	}

انت:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&لگاتار_list);
	ul_configs_free_list(&usr_list);

	أرجع ret < 0 ? ret : (صحيح) list_گنتی_entries(file_list);
}

فراغ ul_configs_free_list(بنية list_سر *file_list)
{
	list_free(file_list, بنية file_element,  file_list, free_list_اندراج);
}

صحيح ul_configs_اگلا_filename(بنية list_سر *file_list,
			     بنية list_سر **current_اندراج,
			     حرف **name)
{
	بنية file_element *element = عدم;

	إذا (list_خالی_تھاواں(file_list) || *current_اندراج == file_list)
		أرجع 1;

	إذا (*current_اندراج == عدم)
		*current_اندراج = file_list->اگلا;

	element = list_اندراج(*current_اندراج, بنية file_element, file_list);
	*name = element->filename;
	*current_اندراج = (*current_اندراج)->اگلا;

	أرجع 0;
}

#جے_تعریف_شدہ TEST_پروگرام_CONFIGS
# include <getopt.h>

صحيح مکھ(صحيح argc, حرف *argv[])
{
	بنية list_سر file_list;
	بنية list_سر *current = عدم;
	حرف *name = عدم;
	ثابت حرف *etc_رستہ = عدم;
	ثابت حرف *لگاتار_رستہ = عدم;
	ثابت حرف *usr_رستہ = عدم;
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
	صحيح ch, گنتی;

	طالما ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, عدم)) != -1) {
		بدل (ch) {
		حالة 'e':
			etc_رستہ = optarg;
			اكسر;
		حالة 'r':
			لگاتار_رستہ = optarg;
			اكسر;
		حالة 'u':
			usr_رستہ = optarg;
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
			لکھو("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				پروگرام_invocation_short_name);
			أرجع EXIT_SUCCESS;
		افتراضي:
			أرجع EXIT_FAILURE;
		}
	}

	إذا (!config_name) {
		fprintf(stderr, "config name is required (use --name)\n");
		أرجع EXIT_FAILURE;
	}

	گنتی = ul_configs_file_list(&file_list, project, etc_رستہ, لگاتار_رستہ, usr_رستہ,
				     config_name, config_suffix);
	إذا (گنتی < 0) {
		fprintf(stderr, "failed to get config file list: %d\n", گنتی);
		أرجع EXIT_FAILURE;
	}

	لکھو("Found %d configuration file(s):\n", گنتی);
	طالما (ul_configs_اگلا_filename(&file_list, &current, &name) == 0)
		لکھو("  %s\n", name);

	ul_configs_free_list(&file_list);
	أرجع EXIT_SUCCESS;
}
#جے_انت /* TEST_PROGRAM_CONFIGS */
