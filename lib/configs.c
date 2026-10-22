/*
 * configs_file.c instantiates functions defined and described in configs_file.h
 */
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <sys/syslog.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdio.h>
#যদি defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
#অন্তর্ভাব <dirent.h>
#endif

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "configs.h"
#অন্তর্ভাব "strutils.h"
#অন্তর্ভাব "list.h"
#অন্তর্ভাব "fileutils.h"

কাঠামো file_element {
	কাঠামো list_শীর্ষ file_list;
	অক্ষর *filename;
};

স্থির __attribute__ ((__format__ (__printf__, 2, 0)))
	অক্ষর *config_mk_path(পূর্ণসংখ্যা types, ধ্রুবক অক্ষর *fmt, ...)
{
	va_list ap;
	অক্ষর *path = ফাঁকা;
	পূর্ণসংখ্যা rc;

	errno = 0;

	va_start(ap, fmt);
	rc = vasprintf(&path, fmt, ap);
	va_end(ap);

	যদি (rc > 0 && types) {
		কাঠামো stat st;

		যদি (stat(path, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			যাও fail;

	}

	ফেরত path;
fail:
	মুক্ত(path);
	ফেরত ফাঁকা;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
স্থির অক্ষর *main_configs(ধ্রুবক অক্ষর *মূল,
			  ধ্রুবক অক্ষর *project,
			  ধ্রুবক অক্ষর *confname,
			  ধ্রুবক অক্ষর *suffix)
{
	অক্ষর *path = ফাঁকা;

	যদি (suffix)
		path = config_mk_path(S_IFREG, "%s/%s/%s.%s",
				মূল, project, confname, suffix);
	যদি (!path)
		path = config_mk_path(S_IFREG, "%s/%s/%s",
				মূল, project, confname);
	ফেরত path;
}

স্থির পূর্ণসংখ্যা configs_refer_filename(কাঠামো list_শীর্ষ *list, অক্ষর *filename, পূর্ণসংখ্যা শীর্ষ)
{
	কাঠামো file_element *e;

	e = calloc(1, আকার(*e));
	যদি (e == ফাঁকা)
		ফেরত -ENOMEM;

	INIT_LIST_শীর্ষ(&e->file_list);
	e->filename = filename;
	যদি (শীর্ষ)
		list_add(&e->file_list, list);
	অন্যথা
		list_add_লেজ(&e->file_list, list);
	ফেরত 0;
}

#যদি defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

স্থির পূর্ণসংখ্যা filter(ধ্রুবক কাঠামো dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	যদি (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		ফেরত 0;
#endif
	যদি (is_dotdir_dirent(d))
		ফেরত 0;

	ফেরত 1;
}

স্থির পূর্ণসংখ্যা read_dir(কাঠামো list_শীর্ষ *file_list,
		    ধ্রুবক অক্ষর *project,
		    ধ্রুবক অক্ষর *মূল,
		    ধ্রুবক অক্ষর *confname,
		    ধ্রুবক অক্ষর *suffix)
{
	অক্ষর *dirname = ফাঁকা;
	অক্ষর *filename = ফাঁকা;
	পূর্ণসংখ্যা dd = -1, nfiles = 0, i;
	পূর্ণসংখ্যা ret = 0;
	কাঠামো dirent **namelist = ফাঁকা;

	যদি (suffix)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.%s.d",
				মূল, project, confname, suffix);
	যদি (!dirname)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.d",
				মূল, project, confname);
	যদি (!dirname)
		ফেরত errno == ENOMEM ? -ENOMEM : 0;

	dd = open(dirname, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	যদি (dd < 0)
		যাও finish;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	যদি (nfiles <= 0)
		যাও finish;

	জন্য (i = 0; i < nfiles; i++) {
		কাঠামো dirent *d = namelist[i];

		যদি (suffix) {
			ধ্রুবক অক্ষর *p = ul_endswith(d->d_name, suffix);

			যদি (!p || p == d->d_name || *(p - 1) != '.')
				চালু;
		}

		যদি (asprintf(&filename, "%s/%s", dirname, d->d_name) < 0) {
			ret = -ENOMEM;
			বিরতি;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		যদি (ret < 0) {
			মুক্ত(filename);
			বিরতি;
		}
	}

finish:
	জন্য (i = 0; i < nfiles; i++)
		মুক্ত(namelist[i]);
	মুক্ত(namelist);
	মুক্ত(dirname);
	যদি (dd >= 0)
		close(dd);
	ফেরত ret;
}

#endif /* HAVE_SCANDIRAT */

স্থির পূর্ণসংখ্যা config_cmp(কাঠামো list_শীর্ষ *a, কাঠামো list_শীর্ষ *b,
		      শূন্য *তথ্য __attribute__((__unused__)))
{
	কাঠামো file_element *ea = list_entry(a, কাঠামো file_element, file_list);
	কাঠামো file_element *eb = list_entry(b, কাঠামো file_element, file_list);
	অক্ষর *na = ul_basename(ea->filename);
	অক্ষর *nb = ul_basename(eb->filename);

	ফেরত strcoll(na, nb);
}

স্থির শূন্য free_list_entry(কাঠামো file_element *element)
{
	মুক্ত(element->filename);
	মুক্ত(element);
}

স্থির পূর্ণসংখ্যা config_merge_list(কাঠামো list_শীর্ষ *main_list,
			      কাঠামো list_শীর্ষ *new_list)
{
	কাঠামো list_শীর্ষ *n, *m, *পরবর্তী;

	list_for_each_safe(n, পরবর্তী, new_list) {
		কাঠামো file_element *ne = list_entry(n, কাঠামো file_element, file_list);
		অক্ষর *nn = ul_basename(ne->filename);
		পূর্ণসংখ্যা duplicate = 0;

		list_for_each(m, main_list) {
			কাঠামো file_element *me = list_entry(m, কাঠামো file_element, file_list);
			অক্ষর *mn = ul_basename(me->filename);

			যদি (strcoll(mn, nn) == 0) {
				duplicate = 1;
				বিরতি;
			}
		}

		যদি (!duplicate) {
			list_del_init(n);
			list_add_লেজ(n, main_list);
		}
	}

	list_sort(main_list, config_cmp, ফাঁকা);
	ফেরত 0;
}

পূর্ণসংখ্যা ul_configs_file_list(কাঠামো list_শীর্ষ *file_list,
			 ধ্রুবক অক্ষর *project,
			 ধ্রুবক অক্ষর *etcdir,
			 ধ্রুবক অক্ষর *rundir,
			 ধ্রুবক অক্ষর *usrdir,
			 ধ্রুবক অক্ষর *confname,
			 ধ্রুবক অক্ষর *suffix)
{
	অক্ষর *main_file = ফাঁকা;
	কাঠামো list_শীর্ষ etc_list;
	কাঠামো list_শীর্ষ run_list;
	কাঠামো list_শীর্ষ usr_list;
	পূর্ণসংখ্যা ret;

	INIT_LIST_শীর্ষ(file_list);

	যদি (!confname)
		ফেরত -ENOTEMPTY;

	/* Default is /etc */
	যদি (!etcdir)
		etcdir = _PATH_SYSCONFDIR;
	যদি (!rundir)
		rundir = "";
	যদি (!usrdir)
		usrdir = "";

	যদি (!project)
		project = "";

	/* Find "main" config file (but don't add to list yet) */
	/* Search order: /etc /run /usr */
	main_file = main_configs(etcdir, project, confname, suffix);
	যদি (main_file == ফাঁকা)
		main_file = main_configs(rundir, project, confname, suffix);
	যদি (main_file == ফাঁকা)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_শীর্ষ(&etc_list);
	INIT_LIST_শীর্ষ(&run_list);
	INIT_LIST_শীর্ষ(&usr_list);

#যদি defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	যদি (ret == -ENOMEM)
		যাও finish;

	ret = read_dir(&run_list, project, rundir, confname, suffix);
	যদি (ret == -ENOMEM)
		যাও finish;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	যদি (ret == -ENOMEM)
		যাও finish;
#endif

	/* Merge drop-in directories in priority order (high to low) */
	যদি ((ret = config_merge_list(file_list, &etc_list)) < 0 ||
	    (ret = config_merge_list(file_list, &run_list)) < 0 ||
	    (ret = config_merge_list(file_list, &usr_list)) < 0)
		যাও finish;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	যদি (main_file != ফাঁকা) {
		ret = configs_refer_filename(file_list, main_file, 1);
		যদি (ret < 0) {
			মুক্ত(main_file);
			যাও finish;
		}
	}

finish:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&run_list);
	ul_configs_free_list(&usr_list);

	ফেরত ret < 0 ? ret : (পূর্ণসংখ্যা) list_গুনতি_entries(file_list);
}

শূন্য ul_configs_free_list(কাঠামো list_শীর্ষ *file_list)
{
	list_free(file_list, কাঠামো file_element,  file_list, free_list_entry);
}

পূর্ণসংখ্যা ul_configs_পরবর্তী_filename(কাঠামো list_শীর্ষ *file_list,
			     কাঠামো list_শীর্ষ **current_entry,
			     অক্ষর **name)
{
	কাঠামো file_element *element = ফাঁকা;

	যদি (list_empty(file_list) || *current_entry == file_list)
		ফেরত 1;

	যদি (*current_entry == ফাঁকা)
		*current_entry = file_list->পরবর্তী;

	element = list_entry(*current_entry, কাঠামো file_element, file_list);
	*name = element->filename;
	*current_entry = (*current_entry)->পরবর্তী;

	ফেরত 0;
}

#ifdef TEST_PROGRAM_CONFIGS
# include <getopt.h>

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	কাঠামো list_শীর্ষ file_list;
	কাঠামো list_শীর্ষ *current = ফাঁকা;
	অক্ষর *name = ফাঁকা;
	ধ্রুবক অক্ষর *etc_path = ফাঁকা;
	ধ্রুবক অক্ষর *run_path = ফাঁকা;
	ধ্রুবক অক্ষর *usr_path = ফাঁকা;
	ধ্রুবক অক্ষর *project = ফাঁকা;
	ধ্রুবক অক্ষর *config_name = ফাঁকা;
	ধ্রুবক অক্ষর *config_suffix = ফাঁকা;
	স্থির ধ্রুবক কাঠামো option longopts[] = {
		{ "etc", required_argument, ফাঁকা, 'e' },
		{ "run", required_argument, ফাঁকা, 'r' },
		{ "usr", required_argument, ফাঁকা, 'u' },
		{ "project", required_argument, ফাঁকা, 'p' },
		{ "name", required_argument, ফাঁকা, 'n' },
		{ "suffix", required_argument, ফাঁকা, 's' },
		{ "help", no_argument, ফাঁকা, 'h' },
		{ ফাঁকা, 0, ফাঁকা, 0 }
	};
	পূর্ণসংখ্যা ch, গুনতি;

	যখন ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, ফাঁকা)) != -1) {
		নির্বাচন (ch) {
		ক্ষেত্র 'e':
			etc_path = optarg;
			বিরতি;
		ক্ষেত্র 'r':
			run_path = optarg;
			বিরতি;
		ক্ষেত্র 'u':
			usr_path = optarg;
			বিরতি;
		ক্ষেত্র 'p':
			project = optarg;
			বিরতি;
		ক্ষেত্র 'n':
			config_name = optarg;
			বিরতি;
		ক্ষেত্র 's':
			config_suffix = optarg;
			বিরতি;
		ক্ষেত্র 'h':
			printf("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				program_invocation_short_name);
			ফেরত EXIT_SUCCESS;
		পূর্বনির্ধারিত:
			ফেরত EXIT_FAILURE;
		}
	}

	যদি (!config_name) {
		fprintf(stderr, "config name is required (use --name)\n");
		ফেরত EXIT_FAILURE;
	}

	গুনতি = ul_configs_file_list(&file_list, project, etc_path, run_path, usr_path,
				     config_name, config_suffix);
	যদি (গুনতি < 0) {
		fprintf(stderr, "failed to get config file list: %d\n", গুনতি);
		ফেরত EXIT_FAILURE;
	}

	printf("Found %d configuration file(s):\n", গুনতি);
	যখন (ul_configs_পরবর্তী_filename(&file_list, &current, &name) == 0)
		printf("  %s\n", name);

	ul_configs_free_list(&file_list);
	ফেরত EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_CONFIGS */
