/*
 * configs_file.c instantiates functions defined and described in configs_file.h
 */
#अन्तर्भाव <errno.h>
#अन्तर्भाव <sys/syslog.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdio.h>
#यदि defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
#अन्तर्भाव <dirent.h>
#endif

#अन्तर्भाव "c.h"
#अन्तर्भाव "configs.h"
#अन्तर्भाव "strutils.h"
#अन्तर्भाव "list.h"
#अन्तर्भाव "fileutils.h"

संरचना file_element {
	संरचना list_head file_list;
	वर्ण *filename;
};

स्थिर __attribute__ ((__format__ (__printf__, 2, 0)))
	वर्ण *config_mk_path(पूर्णाङ्क types, ध्रुव वर्ण *fmt, ...)
{
	va_list ap;
	वर्ण *path = रिक्त;
	पूर्णाङ्क rc;

	errno = 0;

	va_start(ap, fmt);
	rc = vasprintf(&path, fmt, ap);
	va_end(ap);

	यदि (rc > 0 && types) {
		संरचना stat st;

		यदि (stat(path, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			गच्छ fail;

	}

	प्रत्यागम path;
fail:
	मुक्त(path);
	प्रत्यागम रिक्त;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
स्थिर वर्ण *main_configs(ध्रुव वर्ण *मूळ,
			  ध्रुव वर्ण *project,
			  ध्रुव वर्ण *confname,
			  ध्रुव वर्ण *suffix)
{
	वर्ण *path = रिक्त;

	यदि (suffix)
		path = config_mk_path(S_IFREG, "%s/%s/%s.%s",
				मूळ, project, confname, suffix);
	यदि (!path)
		path = config_mk_path(S_IFREG, "%s/%s/%s",
				मूळ, project, confname);
	प्रत्यागम path;
}

स्थिर पूर्णाङ्क configs_refer_filename(संरचना list_head *list, वर्ण *filename, पूर्णाङ्क head)
{
	संरचना file_element *e;

	e = calloc(1, परिमाण(*e));
	यदि (e == रिक्त)
		प्रत्यागम -ENOMEM;

	INIT_LIST_HEAD(&e->file_list);
	e->filename = filename;
	यदि (head)
		list_add(&e->file_list, list);
	अन्यथा
		list_add_tail(&e->file_list, list);
	प्रत्यागम 0;
}

#यदि defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

स्थिर पूर्णाङ्क filter(ध्रुव संरचना dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	यदि (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		प्रत्यागम 0;
#endif
	यदि (is_dotdir_dirent(d))
		प्रत्यागम 0;

	प्रत्यागम 1;
}

स्थिर पूर्णाङ्क read_dir(संरचना list_head *file_list,
		    ध्रुव वर्ण *project,
		    ध्रुव वर्ण *मूळ,
		    ध्रुव वर्ण *confname,
		    ध्रुव वर्ण *suffix)
{
	वर्ण *dirname = रिक्त;
	वर्ण *filename = रिक्त;
	पूर्णाङ्क dd = -1, nfiles = 0, i;
	पूर्णाङ्क ret = 0;
	संरचना dirent **namelist = रिक्त;

	यदि (suffix)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.%s.d",
				मूळ, project, confname, suffix);
	यदि (!dirname)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.d",
				मूळ, project, confname);
	यदि (!dirname)
		प्रत्यागम errno == ENOMEM ? -ENOMEM : 0;

	dd = open(dirname, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	यदि (dd < 0)
		गच्छ finish;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	यदि (nfiles <= 0)
		गच्छ finish;

	प्रति (i = 0; i < nfiles; i++) {
		संरचना dirent *d = namelist[i];

		यदि (suffix) {
			ध्रुव वर्ण *p = ul_endswith(d->d_name, suffix);

			यदि (!p || p == d->d_name || *(p - 1) != '.')
				अनुवर्त;
		}

		यदि (asprintf(&filename, "%s/%s", dirname, d->d_name) < 0) {
			ret = -ENOMEM;
			भङ्ग;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		यदि (ret < 0) {
			मुक्त(filename);
			भङ्ग;
		}
	}

finish:
	प्रति (i = 0; i < nfiles; i++)
		मुक्त(namelist[i]);
	मुक्त(namelist);
	मुक्त(dirname);
	यदि (dd >= 0)
		close(dd);
	प्रत्यागम ret;
}

#endif /* HAVE_SCANDIRAT */

स्थिर पूर्णाङ्क config_cmp(संरचना list_head *a, संरचना list_head *b,
		      शून्य *data __attribute__((__unused__)))
{
	संरचना file_element *ea = list_entry(a, संरचना file_element, file_list);
	संरचना file_element *eb = list_entry(b, संरचना file_element, file_list);
	वर्ण *na = ul_basename(ea->filename);
	वर्ण *nb = ul_basename(eb->filename);

	प्रत्यागम strcoll(na, nb);
}

स्थिर शून्य free_list_entry(संरचना file_element *element)
{
	मुक्त(element->filename);
	मुक्त(element);
}

स्थिर पूर्णाङ्क config_merge_list(संरचना list_head *main_list,
			      संरचना list_head *new_list)
{
	संरचना list_head *n, *m, *next;

	list_for_each_safe(n, next, new_list) {
		संरचना file_element *ne = list_entry(n, संरचना file_element, file_list);
		वर्ण *nn = ul_basename(ne->filename);
		पूर्णाङ्क duplicate = 0;

		list_for_each(m, main_list) {
			संरचना file_element *me = list_entry(m, संरचना file_element, file_list);
			वर्ण *mn = ul_basename(me->filename);

			यदि (strcoll(mn, nn) == 0) {
				duplicate = 1;
				भङ्ग;
			}
		}

		यदि (!duplicate) {
			list_del_init(n);
			list_add_tail(n, main_list);
		}
	}

	list_sort(main_list, config_cmp, रिक्त);
	प्रत्यागम 0;
}

पूर्णाङ्क ul_configs_file_list(संरचना list_head *file_list,
			 ध्रुव वर्ण *project,
			 ध्रुव वर्ण *etcdir,
			 ध्रुव वर्ण *rundir,
			 ध्रुव वर्ण *usrdir,
			 ध्रुव वर्ण *confname,
			 ध्रुव वर्ण *suffix)
{
	वर्ण *main_file = रिक्त;
	संरचना list_head etc_list;
	संरचना list_head run_list;
	संरचना list_head usr_list;
	पूर्णाङ्क ret;

	INIT_LIST_HEAD(file_list);

	यदि (!confname)
		प्रत्यागम -ENOTEMPTY;

	/* Default is /etc */
	यदि (!etcdir)
		etcdir = _PATH_SYSCONFDIR;
	यदि (!rundir)
		rundir = "";
	यदि (!usrdir)
		usrdir = "";

	यदि (!project)
		project = "";

	/* Find "main" config file (but don't add to list yet) */
	/* Search order: /etc /run /usr */
	main_file = main_configs(etcdir, project, confname, suffix);
	यदि (main_file == रिक्त)
		main_file = main_configs(rundir, project, confname, suffix);
	यदि (main_file == रिक्त)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_HEAD(&etc_list);
	INIT_LIST_HEAD(&run_list);
	INIT_LIST_HEAD(&usr_list);

#यदि defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	यदि (ret == -ENOMEM)
		गच्छ finish;

	ret = read_dir(&run_list, project, rundir, confname, suffix);
	यदि (ret == -ENOMEM)
		गच्छ finish;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	यदि (ret == -ENOMEM)
		गच्छ finish;
#endif

	/* Merge drop-in directories in priority order (high to low) */
	यदि ((ret = config_merge_list(file_list, &etc_list)) < 0 ||
	    (ret = config_merge_list(file_list, &run_list)) < 0 ||
	    (ret = config_merge_list(file_list, &usr_list)) < 0)
		गच्छ finish;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	यदि (main_file != रिक्त) {
		ret = configs_refer_filename(file_list, main_file, 1);
		यदि (ret < 0) {
			मुक्त(main_file);
			गच्छ finish;
		}
	}

finish:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&run_list);
	ul_configs_free_list(&usr_list);

	प्रत्यागम ret < 0 ? ret : (पूर्णाङ्क) list_मोजणी_entries(file_list);
}

शून्य ul_configs_free_list(संरचना list_head *file_list)
{
	list_free(file_list, संरचना file_element,  file_list, free_list_entry);
}

पूर्णाङ्क ul_configs_next_filename(संरचना list_head *file_list,
			     संरचना list_head **current_entry,
			     वर्ण **name)
{
	संरचना file_element *element = रिक्त;

	यदि (list_empty(file_list) || *current_entry == file_list)
		प्रत्यागम 1;

	यदि (*current_entry == रिक्त)
		*current_entry = file_list->next;

	element = list_entry(*current_entry, संरचना file_element, file_list);
	*name = element->filename;
	*current_entry = (*current_entry)->next;

	प्रत्यागम 0;
}

#ifdef TEST_PROGRAM_CONFIGS
# include <getopt.h>

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	संरचना list_head file_list;
	संरचना list_head *current = रिक्त;
	वर्ण *name = रिक्त;
	ध्रुव वर्ण *etc_path = रिक्त;
	ध्रुव वर्ण *run_path = रिक्त;
	ध्रुव वर्ण *usr_path = रिक्त;
	ध्रुव वर्ण *project = रिक्त;
	ध्रुव वर्ण *config_name = रिक्त;
	ध्रुव वर्ण *config_suffix = रिक्त;
	स्थिर ध्रुव संरचना option longopts[] = {
		{ "etc", required_argument, रिक्त, 'e' },
		{ "run", required_argument, रिक्त, 'r' },
		{ "usr", required_argument, रिक्त, 'u' },
		{ "project", required_argument, रिक्त, 'p' },
		{ "name", required_argument, रिक्त, 'n' },
		{ "suffix", required_argument, रिक्त, 's' },
		{ "help", no_argument, रिक्त, 'h' },
		{ रिक्त, 0, रिक्त, 0 }
	};
	पूर्णाङ्क ch, मोजणी;

	यावत् ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, रिक्त)) != -1) {
		विकल्प (ch) {
		पक्ष 'e':
			etc_path = optarg;
			भङ्ग;
		पक्ष 'r':
			run_path = optarg;
			भङ्ग;
		पक्ष 'u':
			usr_path = optarg;
			भङ्ग;
		पक्ष 'p':
			project = optarg;
			भङ्ग;
		पक्ष 'n':
			config_name = optarg;
			भङ्ग;
		पक्ष 's':
			config_suffix = optarg;
			भङ्ग;
		पक्ष 'h':
			printf("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				program_invocation_short_name);
			प्रत्यागम EXIT_SUCCESS;
		स्वभाव:
			प्रत्यागम EXIT_FAILURE;
		}
	}

	यदि (!config_name) {
		fprintf(stderr, "config name is required (use --name)\n");
		प्रत्यागम EXIT_FAILURE;
	}

	मोजणी = ul_configs_file_list(&file_list, project, etc_path, run_path, usr_path,
				     config_name, config_suffix);
	यदि (मोजणी < 0) {
		fprintf(stderr, "failed to get config file list: %d\n", मोजणी);
		प्रत्यागम EXIT_FAILURE;
	}

	printf("Found %d configuration file(s):\n", मोजणी);
	यावत् (ul_configs_next_filename(&file_list, &current, &name) == 0)
		printf("  %s\n", name);

	ul_configs_free_list(&file_list);
	प्रत्यागम EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_CONFIGS */
