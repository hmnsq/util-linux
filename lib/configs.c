/*
 * configs_file.c instantiates functions defined and described in configs_file.h
 */
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <sys/syslog.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <stdarg.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <stdio.h>
#എങ്കിൽ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
#അന്തര്ഭാവ <dirent.h>
#endif

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "configs.h"
#അന്തര്ഭാവ "strutils.h"
#അന്തര്ഭാവ "list.h"
#അന്തര്ഭാവ "fileutils.h"

ഘടന file_element {
	ഘടന list_head file_list;
	അക്ഷരം *filename;
};

സ്ഥിതം __attribute__ ((__format__ (__printf__, 2, 0)))
	അക്ഷരം *config_mk_path(പൂർണ്ണസംഖ്യ types, സ്ഥിരം അക്ഷരം *fmt, ...)
{
	va_list ap;
	അക്ഷരം *path = ശൂന്യ;
	പൂർണ്ണസംഖ്യ rc;

	errno = 0;

	va_start(ap, fmt);
	rc = vasprintf(&path, fmt, ap);
	va_end(ap);

	എങ്കിൽ (rc > 0 && types) {
		ഘടന stat st;

		എങ്കിൽ (stat(path, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			പോകുക fail;

	}

	തിരികെ path;
fail:
	സ്വതന്ത്രം(path);
	തിരികെ ശൂന്യ;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
സ്ഥിതം അക്ഷരം *main_configs(സ്ഥിരം അക്ഷരം *വേര്,
			  സ്ഥിരം അക്ഷരം *project,
			  സ്ഥിരം അക്ഷരം *confname,
			  സ്ഥിരം അക്ഷരം *suffix)
{
	അക്ഷരം *path = ശൂന്യ;

	എങ്കിൽ (suffix)
		path = config_mk_path(S_IFREG, "%s/%s/%s.%s",
				വേര്, project, confname, suffix);
	എങ്കിൽ (!path)
		path = config_mk_path(S_IFREG, "%s/%s/%s",
				വേര്, project, confname);
	തിരികെ path;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ configs_refer_filename(ഘടന list_head *list, അക്ഷരം *filename, പൂർണ്ണസംഖ്യ head)
{
	ഘടന file_element *e;

	e = calloc(1, വലുപ്പം(*e));
	എങ്കിൽ (e == ശൂന്യ)
		തിരികെ -ENOMEM;

	INIT_LIST_HEAD(&e->file_list);
	e->filename = filename;
	എങ്കിൽ (head)
		list_add(&e->file_list, list);
	അല്ലെങ്കിൽ
		list_add_tail(&e->file_list, list);
	തിരികെ 0;
}

#എങ്കിൽ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

സ്ഥിതം പൂർണ്ണസംഖ്യ filter(സ്ഥിരം ഘടന dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	എങ്കിൽ (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		തിരികെ 0;
#endif
	എങ്കിൽ (is_dotdir_dirent(d))
		തിരികെ 0;

	തിരികെ 1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ read_dir(ഘടന list_head *file_list,
		    സ്ഥിരം അക്ഷരം *project,
		    സ്ഥിരം അക്ഷരം *വേര്,
		    സ്ഥിരം അക്ഷരം *confname,
		    സ്ഥിരം അക്ഷരം *suffix)
{
	അക്ഷരം *dirname = ശൂന്യ;
	അക്ഷരം *filename = ശൂന്യ;
	പൂർണ്ണസംഖ്യ dd = -1, nfiles = 0, i;
	പൂർണ്ണസംഖ്യ ret = 0;
	ഘടന dirent **namelist = ശൂന്യ;

	എങ്കിൽ (suffix)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.%s.d",
				വേര്, project, confname, suffix);
	എങ്കിൽ (!dirname)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.d",
				വേര്, project, confname);
	എങ്കിൽ (!dirname)
		തിരികെ errno == ENOMEM ? -ENOMEM : 0;

	dd = open(dirname, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	എങ്കിൽ (dd < 0)
		പോകുക finish;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	എങ്കിൽ (nfiles <= 0)
		പോകുക finish;

	വേണ്ടി (i = 0; i < nfiles; i++) {
		ഘടന dirent *d = namelist[i];

		എങ്കിൽ (suffix) {
			സ്ഥിരം അക്ഷരം *p = ul_endswith(d->d_name, suffix);

			എങ്കിൽ (!p || p == d->d_name || *(p - 1) != '.')
				തുടരുക;
		}

		എങ്കിൽ (asprintf(&filename, "%s/%s", dirname, d->d_name) < 0) {
			ret = -ENOMEM;
			നിർത്തുക;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		എങ്കിൽ (ret < 0) {
			സ്വതന്ത്രം(filename);
			നിർത്തുക;
		}
	}

finish:
	വേണ്ടി (i = 0; i < nfiles; i++)
		സ്വതന്ത്രം(namelist[i]);
	സ്വതന്ത്രം(namelist);
	സ്വതന്ത്രം(dirname);
	എങ്കിൽ (dd >= 0)
		close(dd);
	തിരികെ ret;
}

#endif /* HAVE_SCANDIRAT */

സ്ഥിതം പൂർണ്ണസംഖ്യ config_cmp(ഘടന list_head *a, ഘടന list_head *b,
		      ശൂന്യം *data __attribute__((__unused__)))
{
	ഘടന file_element *ea = list_entry(a, ഘടന file_element, file_list);
	ഘടന file_element *eb = list_entry(b, ഘടന file_element, file_list);
	അക്ഷരം *na = ul_basename(ea->filename);
	അക്ഷരം *nb = ul_basename(eb->filename);

	തിരികെ strcoll(na, nb);
}

സ്ഥിതം ശൂന്യം free_list_entry(ഘടന file_element *element)
{
	സ്വതന്ത്രം(element->filename);
	സ്വതന്ത്രം(element);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ config_merge_list(ഘടന list_head *main_list,
			      ഘടന list_head *new_list)
{
	ഘടന list_head *n, *m, *next;

	list_for_each_safe(n, next, new_list) {
		ഘടന file_element *ne = list_entry(n, ഘടന file_element, file_list);
		അക്ഷരം *nn = ul_basename(ne->filename);
		പൂർണ്ണസംഖ്യ duplicate = 0;

		list_for_each(m, main_list) {
			ഘടന file_element *me = list_entry(m, ഘടന file_element, file_list);
			അക്ഷരം *mn = ul_basename(me->filename);

			എങ്കിൽ (strcoll(mn, nn) == 0) {
				duplicate = 1;
				നിർത്തുക;
			}
		}

		എങ്കിൽ (!duplicate) {
			list_del_init(n);
			list_add_tail(n, main_list);
		}
	}

	list_sort(main_list, config_cmp, ശൂന്യ);
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ ul_configs_file_list(ഘടന list_head *file_list,
			 സ്ഥിരം അക്ഷരം *project,
			 സ്ഥിരം അക്ഷരം *etcdir,
			 സ്ഥിരം അക്ഷരം *rundir,
			 സ്ഥിരം അക്ഷരം *usrdir,
			 സ്ഥിരം അക്ഷരം *confname,
			 സ്ഥിരം അക്ഷരം *suffix)
{
	അക്ഷരം *main_file = ശൂന്യ;
	ഘടന list_head etc_list;
	ഘടന list_head run_list;
	ഘടന list_head usr_list;
	പൂർണ്ണസംഖ്യ ret;

	INIT_LIST_HEAD(file_list);

	എങ്കിൽ (!confname)
		തിരികെ -ENOTEMPTY;

	/* Default is /etc */
	എങ്കിൽ (!etcdir)
		etcdir = _PATH_SYSCONFDIR;
	എങ്കിൽ (!rundir)
		rundir = "";
	എങ്കിൽ (!usrdir)
		usrdir = "";

	എങ്കിൽ (!project)
		project = "";

	/* Find "main" config file (but don't add to list yet) */
	/* Search order: /etc /run /usr */
	main_file = main_configs(etcdir, project, confname, suffix);
	എങ്കിൽ (main_file == ശൂന്യ)
		main_file = main_configs(rundir, project, confname, suffix);
	എങ്കിൽ (main_file == ശൂന്യ)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_HEAD(&etc_list);
	INIT_LIST_HEAD(&run_list);
	INIT_LIST_HEAD(&usr_list);

#എങ്കിൽ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	എങ്കിൽ (ret == -ENOMEM)
		പോകുക finish;

	ret = read_dir(&run_list, project, rundir, confname, suffix);
	എങ്കിൽ (ret == -ENOMEM)
		പോകുക finish;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	എങ്കിൽ (ret == -ENOMEM)
		പോകുക finish;
#endif

	/* Merge drop-in directories in priority order (high to low) */
	എങ്കിൽ ((ret = config_merge_list(file_list, &etc_list)) < 0 ||
	    (ret = config_merge_list(file_list, &run_list)) < 0 ||
	    (ret = config_merge_list(file_list, &usr_list)) < 0)
		പോകുക finish;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	എങ്കിൽ (main_file != ശൂന്യ) {
		ret = configs_refer_filename(file_list, main_file, 1);
		എങ്കിൽ (ret < 0) {
			സ്വതന്ത്രം(main_file);
			പോകുക finish;
		}
	}

finish:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&run_list);
	ul_configs_free_list(&usr_list);

	തിരികെ ret < 0 ? ret : (പൂർണ്ണസംഖ്യ) list_എണ്ണം_entries(file_list);
}

ശൂന്യം ul_configs_free_list(ഘടന list_head *file_list)
{
	list_free(file_list, ഘടന file_element,  file_list, free_list_entry);
}

പൂർണ്ണസംഖ്യ ul_configs_next_filename(ഘടന list_head *file_list,
			     ഘടന list_head **current_entry,
			     അക്ഷരം **name)
{
	ഘടന file_element *element = ശൂന്യ;

	എങ്കിൽ (list_empty(file_list) || *current_entry == file_list)
		തിരികെ 1;

	എങ്കിൽ (*current_entry == ശൂന്യ)
		*current_entry = file_list->next;

	element = list_entry(*current_entry, ഘടന file_element, file_list);
	*name = element->filename;
	*current_entry = (*current_entry)->next;

	തിരികെ 0;
}

#ifdef TEST_PROGRAM_CONFIGS
# include <getopt.h>

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	ഘടന list_head file_list;
	ഘടന list_head *current = ശൂന്യ;
	അക്ഷരം *name = ശൂന്യ;
	സ്ഥിരം അക്ഷരം *etc_path = ശൂന്യ;
	സ്ഥിരം അക്ഷരം *run_path = ശൂന്യ;
	സ്ഥിരം അക്ഷരം *usr_path = ശൂന്യ;
	സ്ഥിരം അക്ഷരം *project = ശൂന്യ;
	സ്ഥിരം അക്ഷരം *config_name = ശൂന്യ;
	സ്ഥിരം അക്ഷരം *config_suffix = ശൂന്യ;
	സ്ഥിതം സ്ഥിരം ഘടന option longopts[] = {
		{ "etc", required_argument, ശൂന്യ, 'e' },
		{ "run", required_argument, ശൂന്യ, 'r' },
		{ "usr", required_argument, ശൂന്യ, 'u' },
		{ "project", required_argument, ശൂന്യ, 'p' },
		{ "name", required_argument, ശൂന്യ, 'n' },
		{ "suffix", required_argument, ശൂന്യ, 's' },
		{ "help", no_argument, ശൂന്യ, 'h' },
		{ ശൂന്യ, 0, ശൂന്യ, 0 }
	};
	പൂർണ്ണസംഖ്യ ch, എണ്ണം;

	ആകുമ്പോൾ ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, ശൂന്യ)) != -1) {
		തിരഞ്ഞെടുപ്പ് (ch) {
		അവസ്ഥ 'e':
			etc_path = optarg;
			നിർത്തുക;
		അവസ്ഥ 'r':
			run_path = optarg;
			നിർത്തുക;
		അവസ്ഥ 'u':
			usr_path = optarg;
			നിർത്തുക;
		അവസ്ഥ 'p':
			project = optarg;
			നിർത്തുക;
		അവസ്ഥ 'n':
			config_name = optarg;
			നിർത്തുക;
		അവസ്ഥ 's':
			config_suffix = optarg;
			നിർത്തുക;
		അവസ്ഥ 'h':
			printf("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				program_invocation_short_name);
			തിരികെ EXIT_SUCCESS;
		സ്ഥിരസ്ഥിതി:
			തിരികെ EXIT_FAILURE;
		}
	}

	എങ്കിൽ (!config_name) {
		fprintf(stderr, "config name is required (use --name)\n");
		തിരികെ EXIT_FAILURE;
	}

	എണ്ണം = ul_configs_file_list(&file_list, project, etc_path, run_path, usr_path,
				     config_name, config_suffix);
	എങ്കിൽ (എണ്ണം < 0) {
		fprintf(stderr, "failed to get config file list: %d\n", എണ്ണം);
		തിരികെ EXIT_FAILURE;
	}

	printf("Found %d configuration file(s):\n", എണ്ണം);
	ആകുമ്പോൾ (ul_configs_next_filename(&file_list, &current, &name) == 0)
		printf("  %s\n", name);

	ul_configs_free_list(&file_list);
	തിരികെ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_CONFIGS */
