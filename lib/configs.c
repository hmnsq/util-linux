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
#endif

#شامل "c.h"
#شامل "configs.h"
#شامل "strutils.h"
#شامل "list.h"
#شامل "fileutils.h"

ساختار file_element {
	ساختار list_head file_list;
	نویسه *filename;
};

ایستا __attribute__ ((__format__ (__printf__, 2, 0)))
	نویسه *config_mk_path(صحیح types, ثابت نویسه *fmt, ...)
{
	va_list ap;
	نویسه *path = پوچ;
	صحیح rc;

	errno = 0;

	va_start(ap, fmt);
	rc = vasprintf(&path, fmt, ap);
	va_end(ap);

	اگر (rc > 0 && types) {
		ساختار stat st;

		اگر (stat(path, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			برو fail;

	}

	بازگشت path;
fail:
	آزاد(path);
	بازگشت پوچ;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
ایستا نویسه *main_configs(ثابت نویسه *ریشه,
			  ثابت نویسه *project,
			  ثابت نویسه *confname,
			  ثابت نویسه *suffix)
{
	نویسه *path = پوچ;

	اگر (suffix)
		path = config_mk_path(S_IFREG, "%s/%s/%s.%s",
				ریشه, project, confname, suffix);
	اگر (!path)
		path = config_mk_path(S_IFREG, "%s/%s/%s",
				ریشه, project, confname);
	بازگشت path;
}

ایستا صحیح configs_refer_filename(ساختار list_head *list, نویسه *filename, صحیح head)
{
	ساختار file_element *e;

	e = calloc(1, اندازه(*e));
	اگر (e == پوچ)
		بازگشت -ENOMEM;

	INIT_LIST_HEAD(&e->file_list);
	e->filename = filename;
	اگر (head)
		list_add(&e->file_list, list);
	وگرنه
		list_add_tail(&e->file_list, list);
	بازگشت 0;
}

#اگر defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

ایستا صحیح filter(ثابت ساختار dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	اگر (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		بازگشت 0;
#endif
	اگر (is_dotdir_dirent(d))
		بازگشت 0;

	بازگشت 1;
}

ایستا صحیح read_dir(ساختار list_head *file_list,
		    ثابت نویسه *project,
		    ثابت نویسه *ریشه,
		    ثابت نویسه *confname,
		    ثابت نویسه *suffix)
{
	نویسه *dirname = پوچ;
	نویسه *filename = پوچ;
	صحیح dd = -1, nfiles = 0, i;
	صحیح ret = 0;
	ساختار dirent **namelist = پوچ;

	اگر (suffix)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.%s.d",
				ریشه, project, confname, suffix);
	اگر (!dirname)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.d",
				ریشه, project, confname);
	اگر (!dirname)
		بازگشت errno == ENOMEM ? -ENOMEM : 0;

	dd = open(dirname, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	اگر (dd < 0)
		برو finish;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	اگر (nfiles <= 0)
		برو finish;

	برای (i = 0; i < nfiles; i++) {
		ساختار dirent *d = namelist[i];

		اگر (suffix) {
			ثابت نویسه *p = ul_endswith(d->d_name, suffix);

			اگر (!p || p == d->d_name || *(p - 1) != '.')
				ادامه;
		}

		اگر (asprintf(&filename, "%s/%s", dirname, d->d_name) < 0) {
			ret = -ENOMEM;
			بشکن;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		اگر (ret < 0) {
			آزاد(filename);
			بشکن;
		}
	}

finish:
	برای (i = 0; i < nfiles; i++)
		آزاد(namelist[i]);
	آزاد(namelist);
	آزاد(dirname);
	اگر (dd >= 0)
		close(dd);
	بازگشت ret;
}

#endif /* HAVE_SCANDIRAT */

ایستا صحیح config_cmp(ساختار list_head *a, ساختار list_head *b,
		      تهی *data __attribute__((__unused__)))
{
	ساختار file_element *ea = list_entry(a, ساختار file_element, file_list);
	ساختار file_element *eb = list_entry(b, ساختار file_element, file_list);
	نویسه *na = ul_basename(ea->filename);
	نویسه *nb = ul_basename(eb->filename);

	بازگشت strcoll(na, nb);
}

ایستا تهی free_list_entry(ساختار file_element *element)
{
	آزاد(element->filename);
	آزاد(element);
}

ایستا صحیح config_merge_list(ساختار list_head *main_list,
			      ساختار list_head *new_list)
{
	ساختار list_head *n, *m, *next;

	list_for_each_safe(n, next, new_list) {
		ساختار file_element *ne = list_entry(n, ساختار file_element, file_list);
		نویسه *nn = ul_basename(ne->filename);
		صحیح duplicate = 0;

		list_for_each(m, main_list) {
			ساختار file_element *me = list_entry(m, ساختار file_element, file_list);
			نویسه *mn = ul_basename(me->filename);

			اگر (strcoll(mn, nn) == 0) {
				duplicate = 1;
				بشکن;
			}
		}

		اگر (!duplicate) {
			list_del_init(n);
			list_add_tail(n, main_list);
		}
	}

	list_sort(main_list, config_cmp, پوچ);
	بازگشت 0;
}

صحیح ul_configs_file_list(ساختار list_head *file_list,
			 ثابت نویسه *project,
			 ثابت نویسه *etcdir,
			 ثابت نویسه *rundir,
			 ثابت نویسه *usrdir,
			 ثابت نویسه *confname,
			 ثابت نویسه *suffix)
{
	نویسه *main_file = پوچ;
	ساختار list_head etc_list;
	ساختار list_head run_list;
	ساختار list_head usr_list;
	صحیح ret;

	INIT_LIST_HEAD(file_list);

	اگر (!confname)
		بازگشت -ENOTEMPTY;

	/* Default is /etc */
	اگر (!etcdir)
		etcdir = _PATH_SYSCONFDIR;
	اگر (!rundir)
		rundir = "";
	اگر (!usrdir)
		usrdir = "";

	اگر (!project)
		project = "";

	/* Find "main" config file (but don't add to list yet) */
	/* Search order: /etc /run /usr */
	main_file = main_configs(etcdir, project, confname, suffix);
	اگر (main_file == پوچ)
		main_file = main_configs(rundir, project, confname, suffix);
	اگر (main_file == پوچ)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_HEAD(&etc_list);
	INIT_LIST_HEAD(&run_list);
	INIT_LIST_HEAD(&usr_list);

#اگر defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	اگر (ret == -ENOMEM)
		برو finish;

	ret = read_dir(&run_list, project, rundir, confname, suffix);
	اگر (ret == -ENOMEM)
		برو finish;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	اگر (ret == -ENOMEM)
		برو finish;
#endif

	/* Merge drop-in directories in priority order (high to low) */
	اگر ((ret = config_merge_list(file_list, &etc_list)) < 0 ||
	    (ret = config_merge_list(file_list, &run_list)) < 0 ||
	    (ret = config_merge_list(file_list, &usr_list)) < 0)
		برو finish;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	اگر (main_file != پوچ) {
		ret = configs_refer_filename(file_list, main_file, 1);
		اگر (ret < 0) {
			آزاد(main_file);
			برو finish;
		}
	}

finish:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&run_list);
	ul_configs_free_list(&usr_list);

	بازگشت ret < 0 ? ret : (صحیح) list_شمار_entries(file_list);
}

تهی ul_configs_free_list(ساختار list_head *file_list)
{
	list_free(file_list, ساختار file_element,  file_list, free_list_entry);
}

صحیح ul_configs_next_filename(ساختار list_head *file_list,
			     ساختار list_head **current_entry,
			     نویسه **name)
{
	ساختار file_element *element = پوچ;

	اگر (list_empty(file_list) || *current_entry == file_list)
		بازگشت 1;

	اگر (*current_entry == پوچ)
		*current_entry = file_list->next;

	element = list_entry(*current_entry, ساختار file_element, file_list);
	*name = element->filename;
	*current_entry = (*current_entry)->next;

	بازگشت 0;
}

#ifdef TEST_PROGRAM_CONFIGS
# include <getopt.h>

صحیح اصلی(صحیح argc, نویسه *argv[])
{
	ساختار list_head file_list;
	ساختار list_head *current = پوچ;
	نویسه *name = پوچ;
	ثابت نویسه *etc_path = پوچ;
	ثابت نویسه *run_path = پوچ;
	ثابت نویسه *usr_path = پوچ;
	ثابت نویسه *project = پوچ;
	ثابت نویسه *config_name = پوچ;
	ثابت نویسه *config_suffix = پوچ;
	ایستا ثابت ساختار option longopts[] = {
		{ "etc", required_argument, پوچ, 'e' },
		{ "run", required_argument, پوچ, 'r' },
		{ "usr", required_argument, پوچ, 'u' },
		{ "project", required_argument, پوچ, 'p' },
		{ "name", required_argument, پوچ, 'n' },
		{ "suffix", required_argument, پوچ, 's' },
		{ "help", no_argument, پوچ, 'h' },
		{ پوچ, 0, پوچ, 0 }
	};
	صحیح ch, شمار;

	وقتی ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, پوچ)) != -1) {
		انتخاب (ch) {
		حالت 'e':
			etc_path = optarg;
			بشکن;
		حالت 'r':
			run_path = optarg;
			بشکن;
		حالت 'u':
			usr_path = optarg;
			بشکن;
		حالت 'p':
			project = optarg;
			بشکن;
		حالت 'n':
			config_name = optarg;
			بشکن;
		حالت 's':
			config_suffix = optarg;
			بشکن;
		حالت 'h':
			printf("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				program_invocation_short_name);
			بازگشت EXIT_SUCCESS;
		پیشفرض:
			بازگشت EXIT_FAILURE;
		}
	}

	اگر (!config_name) {
		fprintf(stderr, "config name is required (use --name)\n");
		بازگشت EXIT_FAILURE;
	}

	شمار = ul_configs_file_list(&file_list, project, etc_path, run_path, usr_path,
				     config_name, config_suffix);
	اگر (شمار < 0) {
		fprintf(stderr, "failed to get config file list: %d\n", شمار);
		بازگشت EXIT_FAILURE;
	}

	printf("Found %d configuration file(s):\n", شمار);
	وقتی (ul_configs_next_filename(&file_list, &current, &name) == 0)
		printf("  %s\n", name);

	ul_configs_free_list(&file_list);
	بازگشت EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_CONFIGS */
