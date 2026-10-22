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
#ܐܢ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
#ضمن <dirent.h>
#endif

#ضمن "c.h"
#ضمن "configs.h"
#ضمن "strutils.h"
#ضمن "list.h"
#ضمن "fileutils.h"

بنية file_element {
	بنية list_head file_list;
	حرف *filename;
};

ساكن __attribute__ ((__format__ (__printf__, 2, 0)))
	حرف *config_mk_path(صحيح types, ثابت حرف *fmt, ...)
{
	va_list ap;
	حرف *path = عدم;
	صحيح rc;

	errno = 0;

	va_start(ap, fmt);
	rc = vasprintf(&path, fmt, ap);
	va_end(ap);

	ܐܢ (rc > 0 && types) {
		بنية stat st;

		ܐܢ (stat(path, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			اذهب fail;

	}

	ܦܢܐ path;
fail:
	حرر(path);
	ܦܢܐ عدم;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
ساكن حرف *main_configs(ثابت حرف *root,
			  ثابت حرف *project,
			  ثابت حرف *confname,
			  ثابت حرف *suffix)
{
	حرف *path = عدم;

	ܐܢ (suffix)
		path = config_mk_path(S_IFREG, "%s/%s/%s.%s",
				root, project, confname, suffix);
	ܐܢ (!path)
		path = config_mk_path(S_IFREG, "%s/%s/%s",
				root, project, confname);
	ܦܢܐ path;
}

ساكن صحيح configs_refer_filename(بنية list_head *list, حرف *filename, صحيح head)
{
	بنية file_element *e;

	e = calloc(1, حجم(*e));
	ܐܢ (e == عدم)
		ܦܢܐ -ENOMEM;

	INIT_LIST_HEAD(&e->file_list);
	e->filename = filename;
	ܐܢ (head)
		list_add(&e->file_list, list);
	ܐܚܪܢܐ
		list_add_tail(&e->file_list, list);
	ܦܢܐ 0;
}

#ܐܢ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

ساكن صحيح filter(ثابت بنية dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	ܐܢ (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		ܦܢܐ 0;
#endif
	ܐܢ (is_dotdir_dirent(d))
		ܦܢܐ 0;

	ܦܢܐ 1;
}

ساكن صحيح read_dir(بنية list_head *file_list,
		    ثابت حرف *project,
		    ثابت حرف *root,
		    ثابت حرف *confname,
		    ثابت حرف *suffix)
{
	حرف *dirname = عدم;
	حرف *filename = عدم;
	صحيح dd = -1, nfiles = 0, i;
	صحيح ret = 0;
	بنية dirent **namelist = عدم;

	ܐܢ (suffix)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.%s.d",
				root, project, confname, suffix);
	ܐܢ (!dirname)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.d",
				root, project, confname);
	ܐܢ (!dirname)
		ܦܢܐ errno == ENOMEM ? -ENOMEM : 0;

	dd = open(dirname, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	ܐܢ (dd < 0)
		اذهب finish;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	ܐܢ (nfiles <= 0)
		اذهب finish;

	لكل (i = 0; i < nfiles; i++) {
		بنية dirent *d = namelist[i];

		ܐܢ (suffix) {
			ثابت حرف *p = ul_endswith(d->d_name, suffix);

			ܐܢ (!p || p == d->d_name || *(p - 1) != '.')
				استمر;
		}

		ܐܢ (asprintf(&filename, "%s/%s", dirname, d->d_name) < 0) {
			ret = -ENOMEM;
			اكسر;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		ܐܢ (ret < 0) {
			حرر(filename);
			اكسر;
		}
	}

finish:
	لكل (i = 0; i < nfiles; i++)
		حرر(namelist[i]);
	حرر(namelist);
	حرر(dirname);
	ܐܢ (dd >= 0)
		close(dd);
	ܦܢܐ ret;
}

#endif /* HAVE_SCANDIRAT */

ساكن صحيح config_cmp(بنية list_head *a, بنية list_head *b,
		      فراغ *data __attribute__((__unused__)))
{
	بنية file_element *ea = list_entry(a, بنية file_element, file_list);
	بنية file_element *eb = list_entry(b, بنية file_element, file_list);
	حرف *na = ul_basename(ea->filename);
	حرف *nb = ul_basename(eb->filename);

	ܦܢܐ strcoll(na, nb);
}

ساكن فراغ free_list_entry(بنية file_element *element)
{
	حرر(element->filename);
	حرر(element);
}

ساكن صحيح config_merge_list(بنية list_head *main_list,
			      بنية list_head *new_list)
{
	بنية list_head *n, *m, *next;

	list_for_each_safe(n, next, new_list) {
		بنية file_element *ne = list_entry(n, بنية file_element, file_list);
		حرف *nn = ul_basename(ne->filename);
		صحيح duplicate = 0;

		list_for_each(m, main_list) {
			بنية file_element *me = list_entry(m, بنية file_element, file_list);
			حرف *mn = ul_basename(me->filename);

			ܐܢ (strcoll(mn, nn) == 0) {
				duplicate = 1;
				اكسر;
			}
		}

		ܐܢ (!duplicate) {
			list_del_init(n);
			list_add_tail(n, main_list);
		}
	}

	list_sort(main_list, config_cmp, عدم);
	ܦܢܐ 0;
}

صحيح ul_configs_file_list(بنية list_head *file_list,
			 ثابت حرف *project,
			 ثابت حرف *etcdir,
			 ثابت حرف *rundir,
			 ثابت حرف *usrdir,
			 ثابت حرف *confname,
			 ثابت حرف *suffix)
{
	حرف *main_file = عدم;
	بنية list_head etc_list;
	بنية list_head run_list;
	بنية list_head usr_list;
	صحيح ret;

	INIT_LIST_HEAD(file_list);

	ܐܢ (!confname)
		ܦܢܐ -ENOTEMPTY;

	/* Default is /etc */
	ܐܢ (!etcdir)
		etcdir = _PATH_SYSCONFDIR;
	ܐܢ (!rundir)
		rundir = "";
	ܐܢ (!usrdir)
		usrdir = "";

	ܐܢ (!project)
		project = "";

	/* Find "main" config file (but don't add to list yet) */
	/* Search order: /etc /run /usr */
	main_file = main_configs(etcdir, project, confname, suffix);
	ܐܢ (main_file == عدم)
		main_file = main_configs(rundir, project, confname, suffix);
	ܐܢ (main_file == عدم)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_HEAD(&etc_list);
	INIT_LIST_HEAD(&run_list);
	INIT_LIST_HEAD(&usr_list);

#ܐܢ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	ܐܢ (ret == -ENOMEM)
		اذهب finish;

	ret = read_dir(&run_list, project, rundir, confname, suffix);
	ܐܢ (ret == -ENOMEM)
		اذهب finish;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	ܐܢ (ret == -ENOMEM)
		اذهب finish;
#endif

	/* Merge drop-in directories in priority order (high to low) */
	ܐܢ ((ret = config_merge_list(file_list, &etc_list)) < 0 ||
	    (ret = config_merge_list(file_list, &run_list)) < 0 ||
	    (ret = config_merge_list(file_list, &usr_list)) < 0)
		اذهب finish;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	ܐܢ (main_file != عدم) {
		ret = configs_refer_filename(file_list, main_file, 1);
		ܐܢ (ret < 0) {
			حرر(main_file);
			اذهب finish;
		}
	}

finish:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&run_list);
	ul_configs_free_list(&usr_list);

	ܦܢܐ ret < 0 ? ret : (صحيح) list_count_entries(file_list);
}

فراغ ul_configs_free_list(بنية list_head *file_list)
{
	list_free(file_list, بنية file_element,  file_list, free_list_entry);
}

صحيح ul_configs_next_filename(بنية list_head *file_list,
			     بنية list_head **current_entry,
			     حرف **name)
{
	بنية file_element *element = عدم;

	ܐܢ (list_empty(file_list) || *current_entry == file_list)
		ܦܢܐ 1;

	ܐܢ (*current_entry == عدم)
		*current_entry = file_list->next;

	element = list_entry(*current_entry, بنية file_element, file_list);
	*name = element->filename;
	*current_entry = (*current_entry)->next;

	ܦܢܐ 0;
}

#ifdef TEST_PROGRAM_CONFIGS
# include <getopt.h>

صحيح main(صحيح argc, حرف *argv[])
{
	بنية list_head file_list;
	بنية list_head *current = عدم;
	حرف *name = عدم;
	ثابت حرف *etc_path = عدم;
	ثابت حرف *run_path = عدم;
	ثابت حرف *usr_path = عدم;
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
	صحيح ch, count;

	ܟܡܐ ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, عدم)) != -1) {
		بدل (ch) {
		حالة 'e':
			etc_path = optarg;
			اكسر;
		حالة 'r':
			run_path = optarg;
			اكسر;
		حالة 'u':
			usr_path = optarg;
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
			printf("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				program_invocation_short_name);
			ܦܢܐ EXIT_SUCCESS;
		افتراضي:
			ܦܢܐ EXIT_FAILURE;
		}
	}

	ܐܢ (!config_name) {
		fprintf(stderr, "config name is required (use --name)\n");
		ܦܢܐ EXIT_FAILURE;
	}

	count = ul_configs_file_list(&file_list, project, etc_path, run_path, usr_path,
				     config_name, config_suffix);
	ܐܢ (count < 0) {
		fprintf(stderr, "failed to get config file list: %d\n", count);
		ܦܢܐ EXIT_FAILURE;
	}

	printf("Found %d configuration file(s):\n", count);
	ܟܡܐ (ul_configs_next_filename(&file_list, &current, &name) == 0)
		printf("  %s\n", name);

	ul_configs_free_list(&file_list);
	ܦܢܐ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_CONFIGS */
