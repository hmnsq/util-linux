/*
 * configs_file.c instantiates functions defined and described in configs_file.h
 */
#شامل <errno.h>
#شامل <sys/syslog.h>
#شامل <sys/stat.h>
#شامل <stdarg.h>
#شامل <stdbool.h>
#شامل <string.h>
#شامل <stdlib.h>
#شامل <stdio.h>
#اگر defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
#شامل <dirent.h>
#اگر_ختم

#شامل "c.h"
#شامل "configs.h"
#شامل "strutils.h"
#شامل "list.h"
#شامل "fileutils.h"

ساخت file_element {
	ساخت list_سر file_list;
	حرف *filename;
};

ساکن __attribute__ ((__format__ (__printf__, 2, 0)))
	حرف *config_mk_راستہ(صحیح types, ثابت حرف *fmt, ...)
{
	va_list ap;
	حرف *راستہ = تہی;
	صحیح rc;

	errno = 0;

	va_start(ap, fmt);
	rc = vasprintf(&راستہ, fmt, ap);
	va_end(ap);

	اگر (rc > 0 && types) {
		ساخت stat st;

		اگر (stat(راستہ, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			جاؤ fail;

	}

	واپس راستہ;
fail:
	آزاد(راستہ);
	واپس تہی;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
ساکن حرف *main_configs(ثابت حرف *جڑ,
			  ثابت حرف *project,
			  ثابت حرف *confname,
			  ثابت حرف *suffix)
{
	حرف *راستہ = تہی;

	اگر (suffix)
		راستہ = config_mk_راستہ(S_IFREG, "%s/%s/%s.%s",
				جڑ, project, confname, suffix);
	اگر (!راستہ)
		راستہ = config_mk_راستہ(S_IFREG, "%s/%s/%s",
				جڑ, project, confname);
	واپس راستہ;
}

ساکن صحیح configs_refer_filename(ساخت list_سر *list, حرف *filename, صحیح سر)
{
	ساخت file_element *e;

	e = صفر_حافظہ_دو(1, حجم(*e));
	اگر (e == تہی)
		واپس -ENOMEM;

	INIT_LIST_سر(&e->file_list);
	e->filename = filename;
	اگر (سر)
		list_جوڑو(&e->file_list, list);
	ورنہ
		list_جوڑو_دم(&e->file_list, list);
	واپس 0;
}

#اگر defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

ساکن صحیح filter(ثابت ساخت dirent *d)
{
#اگر_تعریف_شدہ _DIRENT_HAVE_D_TYPE
	اگر (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		واپس 0;
#اگر_ختم
	اگر (is_dotdir_dirent(d))
		واپس 0;

	واپس 1;
}

ساکن صحیح read_dir(ساخت list_سر *file_list,
		    ثابت حرف *project,
		    ثابت حرف *جڑ,
		    ثابت حرف *confname,
		    ثابت حرف *suffix)
{
	حرف *dirname = تہی;
	حرف *filename = تہی;
	صحیح dd = -1, nfiles = 0, i;
	صحیح ret = 0;
	ساخت dirent **namelist = تہی;

	اگر (suffix)
		dirname = config_mk_راستہ(S_IFDIR, "%s/%s/%s.%s.d",
				جڑ, project, confname, suffix);
	اگر (!dirname)
		dirname = config_mk_راستہ(S_IFDIR, "%s/%s/%s.d",
				جڑ, project, confname);
	اگر (!dirname)
		واپس errno == ENOMEM ? -ENOMEM : 0;

	dd = open(dirname, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	اگر (dd < 0)
		جاؤ اختتام;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	اگر (nfiles <= 0)
		جاؤ اختتام;

	برائے (i = 0; i < nfiles; i++) {
		ساخت dirent *d = namelist[i];

		اگر (suffix) {
			ثابت حرف *p = ul_endswith(d->d_name, suffix);

			اگر (!p || p == d->d_name || *(p - 1) != '.')
				جاری;
		}

		اگر (asprintf(&filename, "%s/%s", dirname, d->d_name) < 0) {
			ret = -ENOMEM;
			توڑو;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		اگر (ret < 0) {
			آزاد(filename);
			توڑو;
		}
	}

اختتام:
	برائے (i = 0; i < nfiles; i++)
		آزاد(namelist[i]);
	آزاد(namelist);
	آزاد(dirname);
	اگر (dd >= 0)
		close(dd);
	واپس ret;
}

#اگر_ختم /* HAVE_SCANDIRAT */

ساکن صحیح config_cmp(ساخت list_سر *a, ساخت list_سر *b,
		      خالی *ڈیٹا __attribute__((__unused__)))
{
	ساخت file_element *ea = list_اندراج(a, ساخت file_element, file_list);
	ساخت file_element *eb = list_اندراج(b, ساخت file_element, file_list);
	حرف *na = ul_basename(ea->filename);
	حرف *nb = ul_basename(eb->filename);

	واپس strcoll(na, nb);
}

ساکن خالی free_list_اندراج(ساخت file_element *element)
{
	آزاد(element->filename);
	آزاد(element);
}

ساکن صحیح config_ملاؤ_list(ساخت list_سر *main_list,
			      ساخت list_سر *new_list)
{
	ساخت list_سر *n, *m, *اگلا;

	list_for_each_محفوظ(n, اگلا, new_list) {
		ساخت file_element *ne = list_اندراج(n, ساخت file_element, file_list);
		حرف *nn = ul_basename(ne->filename);
		صحیح duplicate = 0;

		list_for_each(m, main_list) {
			ساخت file_element *me = list_اندراج(m, ساخت file_element, file_list);
			حرف *mn = ul_basename(me->filename);

			اگر (strcoll(mn, nn) == 0) {
				duplicate = 1;
				توڑو;
			}
		}

		اگر (!duplicate) {
			list_del_init(n);
			list_جوڑو_دم(n, main_list);
		}
	}

	list_ترتیب_دو(main_list, config_cmp, تہی);
	واپس 0;
}

صحیح ul_configs_file_list(ساخت list_سر *file_list,
			 ثابت حرف *project,
			 ثابت حرف *etcdir,
			 ثابت حرف *rundir,
			 ثابت حرف *usrdir,
			 ثابت حرف *confname,
			 ثابت حرف *suffix)
{
	حرف *main_file = تہی;
	ساخت list_سر etc_list;
	ساخت list_سر تسلسل_list;
	ساخت list_سر usr_list;
	صحیح ret;

	INIT_LIST_سر(file_list);

	اگر (!confname)
		واپس -ENOTEMPTY;

	/* Default is /etc */
	اگر (!etcdir)
		etcdir = راستہ_SYSCONFDIR;
	اگر (!rundir)
		rundir = "";
	اگر (!usrdir)
		usrdir = "";

	اگر (!project)
		project = "";

	/* Find "main" config file (but don't add to list yet) */
	/* Search order: /etc /run /usr */
	main_file = main_configs(etcdir, project, confname, suffix);
	اگر (main_file == تہی)
		main_file = main_configs(rundir, project, confname, suffix);
	اگر (main_file == تہی)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_سر(&etc_list);
	INIT_LIST_سر(&تسلسل_list);
	INIT_LIST_سر(&usr_list);

#اگر defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	اگر (ret == -ENOMEM)
		جاؤ اختتام;

	ret = read_dir(&تسلسل_list, project, rundir, confname, suffix);
	اگر (ret == -ENOMEM)
		جاؤ اختتام;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	اگر (ret == -ENOMEM)
		جاؤ اختتام;
#اگر_ختم

	/* Merge drop-in directories in priority order (high to low) */
	اگر ((ret = config_ملاؤ_list(file_list, &etc_list)) < 0 ||
	    (ret = config_ملاؤ_list(file_list, &تسلسل_list)) < 0 ||
	    (ret = config_ملاؤ_list(file_list, &usr_list)) < 0)
		جاؤ اختتام;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	اگر (main_file != تہی) {
		ret = configs_refer_filename(file_list, main_file, 1);
		اگر (ret < 0) {
			آزاد(main_file);
			جاؤ اختتام;
		}
	}

اختتام:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&تسلسل_list);
	ul_configs_free_list(&usr_list);

	واپس ret < 0 ? ret : (صحیح) list_گنتی_entries(file_list);
}

خالی ul_configs_free_list(ساخت list_سر *file_list)
{
	list_free(file_list, ساخت file_element,  file_list, free_list_اندراج);
}

صحیح ul_configs_اگلا_filename(ساخت list_سر *file_list,
			     ساخت list_سر **current_اندراج,
			     حرف **name)
{
	ساخت file_element *element = تہی;

	اگر (list_خالی_خانے(file_list) || *current_اندراج == file_list)
		واپس 1;

	اگر (*current_اندراج == تہی)
		*current_اندراج = file_list->اگلا;

	element = list_اندراج(*current_اندراج, ساخت file_element, file_list);
	*name = element->filename;
	*current_اندراج = (*current_اندراج)->اگلا;

	واپس 0;
}

#اگر_تعریف_شدہ TEST_پروگرام_CONFIGS
# include <getopt.h>

صحیح مین(صحیح argc, حرف *argv[])
{
	ساخت list_سر file_list;
	ساخت list_سر *current = تہی;
	حرف *name = تہی;
	ثابت حرف *etc_راستہ = تہی;
	ثابت حرف *تسلسل_راستہ = تہی;
	ثابت حرف *usr_راستہ = تہی;
	ثابت حرف *project = تہی;
	ثابت حرف *config_name = تہی;
	ثابت حرف *config_suffix = تہی;
	ساکن ثابت ساخت option longopts[] = {
		{ "etc", required_argument, تہی, 'e' },
		{ "run", required_argument, تہی, 'r' },
		{ "usr", required_argument, تہی, 'u' },
		{ "project", required_argument, تہی, 'p' },
		{ "name", required_argument, تہی, 'n' },
		{ "suffix", required_argument, تہی, 's' },
		{ "help", no_argument, تہی, 'h' },
		{ تہی, 0, تہی, 0 }
	};
	صحیح ch, گنتی;

	جب ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, تہی)) != -1) {
		انتخاب (ch) {
		حالت 'e':
			etc_راستہ = optarg;
			توڑو;
		حالت 'r':
			تسلسل_راستہ = optarg;
			توڑو;
		حالت 'u':
			usr_راستہ = optarg;
			توڑو;
		حالت 'p':
			project = optarg;
			توڑو;
		حالت 'n':
			config_name = optarg;
			توڑو;
		حالت 's':
			config_suffix = optarg;
			توڑو;
		حالت 'h':
			لکھو("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				پروگرام_invocation_short_name);
			واپس EXIT_SUCCESS;
		طےشدہ:
			واپس EXIT_FAILURE;
		}
	}

	اگر (!config_name) {
		fprintf(stderr, "config name is required (use --name)\n");
		واپس EXIT_FAILURE;
	}

	گنتی = ul_configs_file_list(&file_list, project, etc_راستہ, تسلسل_راستہ, usr_راستہ,
				     config_name, config_suffix);
	اگر (گنتی < 0) {
		fprintf(stderr, "failed to get config file list: %d\n", گنتی);
		واپس EXIT_FAILURE;
	}

	لکھو("Found %d configuration file(s):\n", گنتی);
	جب (ul_configs_اگلا_filename(&file_list, &current, &name) == 0)
		لکھو("  %s\n", name);

	ul_configs_free_list(&file_list);
	واپس EXIT_SUCCESS;
}
#اگر_ختم /* TEST_PROGRAM_CONFIGS */
