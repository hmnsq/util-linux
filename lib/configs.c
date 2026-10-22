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

সংরচনা file_element {
	সংরচনা list_head file_list;
	বর্ণ *filename;
};

স্থির __attribute__ ((__format__ (__printf__, 2, 0)))
	বর্ণ *config_mk_path(পূর্ণাঙ্ক types, ধ্রুব বর্ণ *fmt, ...)
{
	va_list ap;
	বর্ণ *path = রিক্ত;
	পূর্ণাঙ্ক rc;

	errno = 0;

	va_start(ap, fmt);
	rc = vasprintf(&path, fmt, ap);
	va_end(ap);

	যদি (rc > 0 && types) {
		সংরচনা stat st;

		যদি (stat(path, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			গচ্ছ fail;

	}

	প্রত্যাগম path;
fail:
	মুক্ত(path);
	প্রত্যাগম রিক্ত;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
স্থির বর্ণ *main_configs(ধ্রুব বর্ণ *মূল,
			  ধ্রুব বর্ণ *project,
			  ধ্রুব বর্ণ *confname,
			  ধ্রুব বর্ণ *suffix)
{
	বর্ণ *path = রিক্ত;

	যদি (suffix)
		path = config_mk_path(S_IFREG, "%s/%s/%s.%s",
				মূল, project, confname, suffix);
	যদি (!path)
		path = config_mk_path(S_IFREG, "%s/%s/%s",
				মূল, project, confname);
	প্রত্যাগম path;
}

স্থির পূর্ণাঙ্ক configs_refer_filename(সংরচনা list_head *list, বর্ণ *filename, পূর্ণাঙ্ক head)
{
	সংরচনা file_element *e;

	e = calloc(1, পরিমাণ(*e));
	যদি (e == রিক্ত)
		প্রত্যাগম -ENOMEM;

	INIT_LIST_HEAD(&e->file_list);
	e->filename = filename;
	যদি (head)
		list_add(&e->file_list, list);
	অন্যথা
		list_add_tail(&e->file_list, list);
	প্রত্যাগম 0;
}

#যদি defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

স্থির পূর্ণাঙ্ক filter(ধ্রুব সংরচনা dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	যদি (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		প্রত্যাগম 0;
#endif
	যদি (is_dotdir_dirent(d))
		প্রত্যাগম 0;

	প্রত্যাগম 1;
}

স্থির পূর্ণাঙ্ক read_dir(সংরচনা list_head *file_list,
		    ধ্রুব বর্ণ *project,
		    ধ্রুব বর্ণ *মূল,
		    ধ্রুব বর্ণ *confname,
		    ধ্রুব বর্ণ *suffix)
{
	বর্ণ *dirname = রিক্ত;
	বর্ণ *filename = রিক্ত;
	পূর্ণাঙ্ক dd = -1, nfiles = 0, i;
	পূর্ণাঙ্ক ret = 0;
	সংরচনা dirent **namelist = রিক্ত;

	যদি (suffix)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.%s.d",
				মূল, project, confname, suffix);
	যদি (!dirname)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.d",
				মূল, project, confname);
	যদি (!dirname)
		প্রত্যাগম errno == ENOMEM ? -ENOMEM : 0;

	dd = open(dirname, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	যদি (dd < 0)
		গচ্ছ finish;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	যদি (nfiles <= 0)
		গচ্ছ finish;

	প্রতি (i = 0; i < nfiles; i++) {
		সংরচনা dirent *d = namelist[i];

		যদি (suffix) {
			ধ্রুব বর্ণ *p = ul_endswith(d->d_name, suffix);

			যদি (!p || p == d->d_name || *(p - 1) != '.')
				অনুবর্ত;
		}

		যদি (asprintf(&filename, "%s/%s", dirname, d->d_name) < 0) {
			ret = -ENOMEM;
			ভঙ্গ;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		যদি (ret < 0) {
			মুক্ত(filename);
			ভঙ্গ;
		}
	}

finish:
	প্রতি (i = 0; i < nfiles; i++)
		মুক্ত(namelist[i]);
	মুক্ত(namelist);
	মুক্ত(dirname);
	যদি (dd >= 0)
		close(dd);
	প্রত্যাগম ret;
}

#endif /* HAVE_SCANDIRAT */

স্থির পূর্ণাঙ্ক config_cmp(সংরচনা list_head *a, সংরচনা list_head *b,
		      শূন্য *data __attribute__((__unused__)))
{
	সংরচনা file_element *ea = list_entry(a, সংরচনা file_element, file_list);
	সংরচনা file_element *eb = list_entry(b, সংরচনা file_element, file_list);
	বর্ণ *na = ul_basename(ea->filename);
	বর্ণ *nb = ul_basename(eb->filename);

	প্রত্যাগম strcoll(na, nb);
}

স্থির শূন্য free_list_entry(সংরচনা file_element *element)
{
	মুক্ত(element->filename);
	মুক্ত(element);
}

স্থির পূর্ণাঙ্ক config_merge_list(সংরচনা list_head *main_list,
			      সংরচনা list_head *new_list)
{
	সংরচনা list_head *n, *m, *next;

	list_for_each_safe(n, next, new_list) {
		সংরচনা file_element *ne = list_entry(n, সংরচনা file_element, file_list);
		বর্ণ *nn = ul_basename(ne->filename);
		পূর্ণাঙ্ক duplicate = 0;

		list_for_each(m, main_list) {
			সংরচনা file_element *me = list_entry(m, সংরচনা file_element, file_list);
			বর্ণ *mn = ul_basename(me->filename);

			যদি (strcoll(mn, nn) == 0) {
				duplicate = 1;
				ভঙ্গ;
			}
		}

		যদি (!duplicate) {
			list_del_init(n);
			list_add_tail(n, main_list);
		}
	}

	list_sort(main_list, config_cmp, রিক্ত);
	প্রত্যাগম 0;
}

পূর্ণাঙ্ক ul_configs_file_list(সংরচনা list_head *file_list,
			 ধ্রুব বর্ণ *project,
			 ধ্রুব বর্ণ *etcdir,
			 ধ্রুব বর্ণ *rundir,
			 ধ্রুব বর্ণ *usrdir,
			 ধ্রুব বর্ণ *confname,
			 ধ্রুব বর্ণ *suffix)
{
	বর্ণ *main_file = রিক্ত;
	সংরচনা list_head etc_list;
	সংরচনা list_head run_list;
	সংরচনা list_head usr_list;
	পূর্ণাঙ্ক ret;

	INIT_LIST_HEAD(file_list);

	যদি (!confname)
		প্রত্যাগম -ENOTEMPTY;

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
	যদি (main_file == রিক্ত)
		main_file = main_configs(rundir, project, confname, suffix);
	যদি (main_file == রিক্ত)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_HEAD(&etc_list);
	INIT_LIST_HEAD(&run_list);
	INIT_LIST_HEAD(&usr_list);

#যদি defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	যদি (ret == -ENOMEM)
		গচ্ছ finish;

	ret = read_dir(&run_list, project, rundir, confname, suffix);
	যদি (ret == -ENOMEM)
		গচ্ছ finish;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	যদি (ret == -ENOMEM)
		গচ্ছ finish;
#endif

	/* Merge drop-in directories in priority order (high to low) */
	যদি ((ret = config_merge_list(file_list, &etc_list)) < 0 ||
	    (ret = config_merge_list(file_list, &run_list)) < 0 ||
	    (ret = config_merge_list(file_list, &usr_list)) < 0)
		গচ্ছ finish;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	যদি (main_file != রিক্ত) {
		ret = configs_refer_filename(file_list, main_file, 1);
		যদি (ret < 0) {
			মুক্ত(main_file);
			গচ্ছ finish;
		}
	}

finish:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&run_list);
	ul_configs_free_list(&usr_list);

	প্রত্যাগম ret < 0 ? ret : (পূর্ণাঙ্ক) list_গুনতি_entries(file_list);
}

শূন্য ul_configs_free_list(সংরচনা list_head *file_list)
{
	list_free(file_list, সংরচনা file_element,  file_list, free_list_entry);
}

পূর্ণাঙ্ক ul_configs_next_filename(সংরচনা list_head *file_list,
			     সংরচনা list_head **current_entry,
			     বর্ণ **name)
{
	সংরচনা file_element *element = রিক্ত;

	যদি (list_empty(file_list) || *current_entry == file_list)
		প্রত্যাগম 1;

	যদি (*current_entry == রিক্ত)
		*current_entry = file_list->next;

	element = list_entry(*current_entry, সংরচনা file_element, file_list);
	*name = element->filename;
	*current_entry = (*current_entry)->next;

	প্রত্যাগম 0;
}

#ifdef TEST_PROGRAM_CONFIGS
# include <getopt.h>

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ *argv[])
{
	সংরচনা list_head file_list;
	সংরচনা list_head *current = রিক্ত;
	বর্ণ *name = রিক্ত;
	ধ্রুব বর্ণ *etc_path = রিক্ত;
	ধ্রুব বর্ণ *run_path = রিক্ত;
	ধ্রুব বর্ণ *usr_path = রিক্ত;
	ধ্রুব বর্ণ *project = রিক্ত;
	ধ্রুব বর্ণ *config_name = রিক্ত;
	ধ্রুব বর্ণ *config_suffix = রিক্ত;
	স্থির ধ্রুব সংরচনা option longopts[] = {
		{ "etc", required_argument, রিক্ত, 'e' },
		{ "run", required_argument, রিক্ত, 'r' },
		{ "usr", required_argument, রিক্ত, 'u' },
		{ "project", required_argument, রিক্ত, 'p' },
		{ "name", required_argument, রিক্ত, 'n' },
		{ "suffix", required_argument, রিক্ত, 's' },
		{ "help", no_argument, রিক্ত, 'h' },
		{ রিক্ত, 0, রিক্ত, 0 }
	};
	পূর্ণাঙ্ক ch, গুনতি;

	যাবত্ ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, রিক্ত)) != -1) {
		বিকল্প (ch) {
		পক্ষ 'e':
			etc_path = optarg;
			ভঙ্গ;
		পক্ষ 'r':
			run_path = optarg;
			ভঙ্গ;
		পক্ষ 'u':
			usr_path = optarg;
			ভঙ্গ;
		পক্ষ 'p':
			project = optarg;
			ভঙ্গ;
		পক্ষ 'n':
			config_name = optarg;
			ভঙ্গ;
		পক্ষ 's':
			config_suffix = optarg;
			ভঙ্গ;
		পক্ষ 'h':
			printf("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				program_invocation_short_name);
			প্রত্যাগম EXIT_SUCCESS;
		স্বভাব:
			প্রত্যাগম EXIT_FAILURE;
		}
	}

	যদি (!config_name) {
		fprintf(stderr, "config name is required (use --name)\n");
		প্রত্যাগম EXIT_FAILURE;
	}

	গুনতি = ul_configs_file_list(&file_list, project, etc_path, run_path, usr_path,
				     config_name, config_suffix);
	যদি (গুনতি < 0) {
		fprintf(stderr, "failed to get config file list: %d\n", গুনতি);
		প্রত্যাগম EXIT_FAILURE;
	}

	printf("Found %d configuration file(s):\n", গুনতি);
	যাবত্ (ul_configs_next_filename(&file_list, &current, &name) == 0)
		printf("  %s\n", name);

	ul_configs_free_list(&file_list);
	প্রত্যাগম EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_CONFIGS */
