/*
 * configs_file.c instantiates functions defined and described in configs_file.h
 */
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <sys/syslog.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಯದಿ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
#ಅನ್ತರ್ಭಾವ <dirent.h>
#endif

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "configs.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"
#ಅನ್ತರ್ಭಾವ "list.h"
#ಅನ್ತರ್ಭಾವ "fileutils.h"

ಸಂರಚನಾ file_element {
	ಸಂರಚನಾ list_head file_list;
	ವರ್ಣ *filename;
};

ಸ್ಥಿರ __attribute__ ((__format__ (__printf__, 2, 0)))
	ವರ್ಣ *config_mk_path(ಪೂರ್ಣಾಙ್ಕ types, ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
	va_list ap;
	ವರ್ಣ *path = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ rc;

	errno = 0;

	va_start(ap, fmt);
	rc = vasprintf(&path, fmt, ap);
	va_end(ap);

	ಯದಿ (rc > 0 && types) {
		ಸಂರಚನಾ stat st;

		ಯದಿ (stat(path, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			ಗಚ್ಛ fail;

	}

	ಪ್ರತ್ಯಾಗಮ path;
fail:
	ಮುಕ್ತ(path);
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
ಸ್ಥಿರ ವರ್ಣ *main_configs(ಧ್ರುವ ವರ್ಣ *ಬೇರು,
			  ಧ್ರುವ ವರ್ಣ *project,
			  ಧ್ರುವ ವರ್ಣ *confname,
			  ಧ್ರುವ ವರ್ಣ *suffix)
{
	ವರ್ಣ *path = ರಿಕ್ತ;

	ಯದಿ (suffix)
		path = config_mk_path(S_IFREG, "%s/%s/%s.%s",
				ಬೇರು, project, confname, suffix);
	ಯದಿ (!path)
		path = config_mk_path(S_IFREG, "%s/%s/%s",
				ಬೇರು, project, confname);
	ಪ್ರತ್ಯಾಗಮ path;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ configs_refer_filename(ಸಂರಚನಾ list_head *list, ವರ್ಣ *filename, ಪೂರ್ಣಾಙ್ಕ head)
{
	ಸಂರಚನಾ file_element *e;

	e = calloc(1, ಪರಿಮಾಣ(*e));
	ಯದಿ (e == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ -ENOMEM;

	INIT_LIST_HEAD(&e->file_list);
	e->filename = filename;
	ಯದಿ (head)
		list_add(&e->file_list, list);
	ಅನ್ಯಥಾ
		list_add_tail(&e->file_list, list);
	ಪ್ರತ್ಯಾಗಮ 0;
}

#ಯದಿ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ filter(ಧ್ರುವ ಸಂರಚನಾ dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	ಯದಿ (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		ಪ್ರತ್ಯಾಗಮ 0;
#endif
	ಯದಿ (is_dotdir_dirent(d))
		ಪ್ರತ್ಯಾಗಮ 0;

	ಪ್ರತ್ಯಾಗಮ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ read_dir(ಸಂರಚನಾ list_head *file_list,
		    ಧ್ರುವ ವರ್ಣ *project,
		    ಧ್ರುವ ವರ್ಣ *ಬೇರು,
		    ಧ್ರುವ ವರ್ಣ *confname,
		    ಧ್ರುವ ವರ್ಣ *suffix)
{
	ವರ್ಣ *dirname = ರಿಕ್ತ;
	ವರ್ಣ *filename = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ dd = -1, nfiles = 0, i;
	ಪೂರ್ಣಾಙ್ಕ ret = 0;
	ಸಂರಚನಾ dirent **namelist = ರಿಕ್ತ;

	ಯದಿ (suffix)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.%s.d",
				ಬೇರು, project, confname, suffix);
	ಯದಿ (!dirname)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.d",
				ಬೇರು, project, confname);
	ಯದಿ (!dirname)
		ಪ್ರತ್ಯಾಗಮ errno == ENOMEM ? -ENOMEM : 0;

	dd = open(dirname, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	ಯದಿ (dd < 0)
		ಗಚ್ಛ finish;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	ಯದಿ (nfiles <= 0)
		ಗಚ್ಛ finish;

	ಪ್ರತಿ (i = 0; i < nfiles; i++) {
		ಸಂರಚನಾ dirent *d = namelist[i];

		ಯದಿ (suffix) {
			ಧ್ರುವ ವರ್ಣ *p = ul_endswith(d->d_name, suffix);

			ಯದಿ (!p || p == d->d_name || *(p - 1) != '.')
				ಅನುವರ್ತ;
		}

		ಯದಿ (asprintf(&filename, "%s/%s", dirname, d->d_name) < 0) {
			ret = -ENOMEM;
			ಭಙ್ಗ;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		ಯದಿ (ret < 0) {
			ಮುಕ್ತ(filename);
			ಭಙ್ಗ;
		}
	}

finish:
	ಪ್ರತಿ (i = 0; i < nfiles; i++)
		ಮುಕ್ತ(namelist[i]);
	ಮುಕ್ತ(namelist);
	ಮುಕ್ತ(dirname);
	ಯದಿ (dd >= 0)
		close(dd);
	ಪ್ರತ್ಯಾಗಮ ret;
}

#endif /* HAVE_SCANDIRAT */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ config_cmp(ಸಂರಚನಾ list_head *a, ಸಂರಚನಾ list_head *b,
		      ಶೂನ್ಯ *data __attribute__((__unused__)))
{
	ಸಂರಚನಾ file_element *ea = list_entry(a, ಸಂರಚನಾ file_element, file_list);
	ಸಂರಚನಾ file_element *eb = list_entry(b, ಸಂರಚನಾ file_element, file_list);
	ವರ್ಣ *na = ul_basename(ea->filename);
	ವರ್ಣ *nb = ul_basename(eb->filename);

	ಪ್ರತ್ಯಾಗಮ strcoll(na, nb);
}

ಸ್ಥಿರ ಶೂನ್ಯ free_list_entry(ಸಂರಚನಾ file_element *element)
{
	ಮುಕ್ತ(element->filename);
	ಮುಕ್ತ(element);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ config_merge_list(ಸಂರಚನಾ list_head *main_list,
			      ಸಂರಚನಾ list_head *new_list)
{
	ಸಂರಚನಾ list_head *n, *m, *next;

	list_for_each_safe(n, next, new_list) {
		ಸಂರಚನಾ file_element *ne = list_entry(n, ಸಂರಚನಾ file_element, file_list);
		ವರ್ಣ *nn = ul_basename(ne->filename);
		ಪೂರ್ಣಾಙ್ಕ duplicate = 0;

		list_for_each(m, main_list) {
			ಸಂರಚನಾ file_element *me = list_entry(m, ಸಂರಚನಾ file_element, file_list);
			ವರ್ಣ *mn = ul_basename(me->filename);

			ಯದಿ (strcoll(mn, nn) == 0) {
				duplicate = 1;
				ಭಙ್ಗ;
			}
		}

		ಯದಿ (!duplicate) {
			list_del_init(n);
			list_add_tail(n, main_list);
		}
	}

	list_sort(main_list, config_cmp, ರಿಕ್ತ);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ ul_configs_file_list(ಸಂರಚನಾ list_head *file_list,
			 ಧ್ರುವ ವರ್ಣ *project,
			 ಧ್ರುವ ವರ್ಣ *etcdir,
			 ಧ್ರುವ ವರ್ಣ *rundir,
			 ಧ್ರುವ ವರ್ಣ *usrdir,
			 ಧ್ರುವ ವರ್ಣ *confname,
			 ಧ್ರುವ ವರ್ಣ *suffix)
{
	ವರ್ಣ *main_file = ರಿಕ್ತ;
	ಸಂರಚನಾ list_head etc_list;
	ಸಂರಚನಾ list_head run_list;
	ಸಂರಚನಾ list_head usr_list;
	ಪೂರ್ಣಾಙ್ಕ ret;

	INIT_LIST_HEAD(file_list);

	ಯದಿ (!confname)
		ಪ್ರತ್ಯಾಗಮ -ENOTEMPTY;

	/* Default is /etc */
	ಯದಿ (!etcdir)
		etcdir = _PATH_SYSCONFDIR;
	ಯದಿ (!rundir)
		rundir = "";
	ಯದಿ (!usrdir)
		usrdir = "";

	ಯದಿ (!project)
		project = "";

	/* Find "main" config file (but don't add to list yet) */
	/* Search order: /etc /run /usr */
	main_file = main_configs(etcdir, project, confname, suffix);
	ಯದಿ (main_file == ರಿಕ್ತ)
		main_file = main_configs(rundir, project, confname, suffix);
	ಯದಿ (main_file == ರಿಕ್ತ)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_HEAD(&etc_list);
	INIT_LIST_HEAD(&run_list);
	INIT_LIST_HEAD(&usr_list);

#ಯದಿ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	ಯದಿ (ret == -ENOMEM)
		ಗಚ್ಛ finish;

	ret = read_dir(&run_list, project, rundir, confname, suffix);
	ಯದಿ (ret == -ENOMEM)
		ಗಚ್ಛ finish;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	ಯದಿ (ret == -ENOMEM)
		ಗಚ್ಛ finish;
#endif

	/* Merge drop-in directories in priority order (high to low) */
	ಯದಿ ((ret = config_merge_list(file_list, &etc_list)) < 0 ||
	    (ret = config_merge_list(file_list, &run_list)) < 0 ||
	    (ret = config_merge_list(file_list, &usr_list)) < 0)
		ಗಚ್ಛ finish;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	ಯದಿ (main_file != ರಿಕ್ತ) {
		ret = configs_refer_filename(file_list, main_file, 1);
		ಯದಿ (ret < 0) {
			ಮುಕ್ತ(main_file);
			ಗಚ್ಛ finish;
		}
	}

finish:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&run_list);
	ul_configs_free_list(&usr_list);

	ಪ್ರತ್ಯಾಗಮ ret < 0 ? ret : (ಪೂರ್ಣಾಙ್ಕ) list_ಎಣಿಕೆ_entries(file_list);
}

ಶೂನ್ಯ ul_configs_free_list(ಸಂರಚನಾ list_head *file_list)
{
	list_free(file_list, ಸಂರಚನಾ file_element,  file_list, free_list_entry);
}

ಪೂರ್ಣಾಙ್ಕ ul_configs_next_filename(ಸಂರಚನಾ list_head *file_list,
			     ಸಂರಚನಾ list_head **current_entry,
			     ವರ್ಣ **name)
{
	ಸಂರಚನಾ file_element *element = ರಿಕ್ತ;

	ಯದಿ (list_empty(file_list) || *current_entry == file_list)
		ಪ್ರತ್ಯಾಗಮ 1;

	ಯದಿ (*current_entry == ರಿಕ್ತ)
		*current_entry = file_list->next;

	element = list_entry(*current_entry, ಸಂರಚನಾ file_element, file_list);
	*name = element->filename;
	*current_entry = (*current_entry)->next;

	ಪ್ರತ್ಯಾಗಮ 0;
}

#ifdef TEST_PROGRAM_CONFIGS
# include <getopt.h>

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ಸಂರಚನಾ list_head file_list;
	ಸಂರಚನಾ list_head *current = ರಿಕ್ತ;
	ವರ್ಣ *name = ರಿಕ್ತ;
	ಧ್ರುವ ವರ್ಣ *etc_path = ರಿಕ್ತ;
	ಧ್ರುವ ವರ್ಣ *run_path = ರಿಕ್ತ;
	ಧ್ರುವ ವರ್ಣ *usr_path = ರಿಕ್ತ;
	ಧ್ರುವ ವರ್ಣ *project = ರಿಕ್ತ;
	ಧ್ರುವ ವರ್ಣ *config_name = ರಿಕ್ತ;
	ಧ್ರುವ ವರ್ಣ *config_suffix = ರಿಕ್ತ;
	ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ option longopts[] = {
		{ "etc", required_argument, ರಿಕ್ತ, 'e' },
		{ "run", required_argument, ರಿಕ್ತ, 'r' },
		{ "usr", required_argument, ರಿಕ್ತ, 'u' },
		{ "project", required_argument, ರಿಕ್ತ, 'p' },
		{ "name", required_argument, ರಿಕ್ತ, 'n' },
		{ "suffix", required_argument, ರಿಕ್ತ, 's' },
		{ "help", no_argument, ರಿಕ್ತ, 'h' },
		{ ರಿಕ್ತ, 0, ರಿಕ್ತ, 0 }
	};
	ಪೂರ್ಣಾಙ್ಕ ch, ಎಣಿಕೆ;

	ಯಾವತ್ ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, ರಿಕ್ತ)) != -1) {
		ವಿಕಲ್ಪ (ch) {
		ಪಕ್ಷ 'e':
			etc_path = optarg;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'r':
			run_path = optarg;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'u':
			usr_path = optarg;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'p':
			project = optarg;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'n':
			config_name = optarg;
			ಭಙ್ಗ;
		ಪಕ್ಷ 's':
			config_suffix = optarg;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'h':
			printf("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				program_invocation_short_name);
			ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
		ಸ್ವಭಾವ:
			ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
		}
	}

	ಯದಿ (!config_name) {
		fprintf(stderr, "config name is required (use --name)\n");
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
	}

	ಎಣಿಕೆ = ul_configs_file_list(&file_list, project, etc_path, run_path, usr_path,
				     config_name, config_suffix);
	ಯದಿ (ಎಣಿಕೆ < 0) {
		fprintf(stderr, "failed to get config file list: %d\n", ಎಣಿಕೆ);
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
	}

	printf("Found %d configuration file(s):\n", ಎಣಿಕೆ);
	ಯಾವತ್ (ul_configs_next_filename(&file_list, &current, &name) == 0)
		printf("  %s\n", name);

	ul_configs_free_list(&file_list);
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_CONFIGS */
