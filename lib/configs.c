/*
 * configs_file.c instantiates functions defined and described in configs_file.h
 */
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <sys/syslog.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdio.h>
#எனில் defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
#அந்தர்பாவ <dirent.h>
#endif

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "configs.h"
#அந்தர்பாவ "strutils.h"
#அந்தர்பாவ "list.h"
#அந்தர்பாவ "fileutils.h"

கட்டமைப்பு file_element {
	கட்டமைப்பு list_head file_list;
	எழுத்து *filename;
};

நிலையான __attribute__ ((__format__ (__printf__, 2, 0)))
	எழுத்து *config_mk_path(முழு types, மாறிலி எழுத்து *fmt, ...)
{
	va_list ap;
	எழுத்து *path = வெற்று;
	முழு rc;

	errno = 0;

	va_start(ap, fmt);
	rc = vasprintf(&path, fmt, ap);
	va_end(ap);

	எனில் (rc > 0 && types) {
		கட்டமைப்பு stat st;

		எனில் (stat(path, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			செல் fail;

	}

	திரும்பு path;
fail:
	விடுவி(path);
	திரும்பு வெற்று;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
நிலையான எழுத்து *main_configs(மாறிலி எழுத்து *வேர்,
			  மாறிலி எழுத்து *project,
			  மாறிலி எழுத்து *confname,
			  மாறிலி எழுத்து *suffix)
{
	எழுத்து *path = வெற்று;

	எனில் (suffix)
		path = config_mk_path(S_IFREG, "%s/%s/%s.%s",
				வேர், project, confname, suffix);
	எனில் (!path)
		path = config_mk_path(S_IFREG, "%s/%s/%s",
				வேர், project, confname);
	திரும்பு path;
}

நிலையான முழு configs_refer_filename(கட்டமைப்பு list_head *list, எழுத்து *filename, முழு head)
{
	கட்டமைப்பு file_element *e;

	e = calloc(1, அளவு(*e));
	எனில் (e == வெற்று)
		திரும்பு -ENOMEM;

	INIT_LIST_HEAD(&e->file_list);
	e->filename = filename;
	எனில் (head)
		list_add(&e->file_list, list);
	இல்லையெனில்
		list_add_tail(&e->file_list, list);
	திரும்பு 0;
}

#எனில் defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

நிலையான முழு filter(மாறிலி கட்டமைப்பு dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	எனில் (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		திரும்பு 0;
#endif
	எனில் (is_dotdir_dirent(d))
		திரும்பு 0;

	திரும்பு 1;
}

நிலையான முழு read_dir(கட்டமைப்பு list_head *file_list,
		    மாறிலி எழுத்து *project,
		    மாறிலி எழுத்து *வேர்,
		    மாறிலி எழுத்து *confname,
		    மாறிலி எழுத்து *suffix)
{
	எழுத்து *dirname = வெற்று;
	எழுத்து *filename = வெற்று;
	முழு dd = -1, nfiles = 0, i;
	முழு ret = 0;
	கட்டமைப்பு dirent **namelist = வெற்று;

	எனில் (suffix)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.%s.d",
				வேர், project, confname, suffix);
	எனில் (!dirname)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.d",
				வேர், project, confname);
	எனில் (!dirname)
		திரும்பு errno == ENOMEM ? -ENOMEM : 0;

	dd = open(dirname, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	எனில் (dd < 0)
		செல் finish;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	எனில் (nfiles <= 0)
		செல் finish;

	க்கு (i = 0; i < nfiles; i++) {
		கட்டமைப்பு dirent *d = namelist[i];

		எனில் (suffix) {
			மாறிலி எழுத்து *p = ul_endswith(d->d_name, suffix);

			எனில் (!p || p == d->d_name || *(p - 1) != '.')
				தொடர்;
		}

		எனில் (asprintf(&filename, "%s/%s", dirname, d->d_name) < 0) {
			ret = -ENOMEM;
			நிறுத்து;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		எனில் (ret < 0) {
			விடுவி(filename);
			நிறுத்து;
		}
	}

finish:
	க்கு (i = 0; i < nfiles; i++)
		விடுவி(namelist[i]);
	விடுவி(namelist);
	விடுவி(dirname);
	எனில் (dd >= 0)
		close(dd);
	திரும்பு ret;
}

#endif /* HAVE_SCANDIRAT */

நிலையான முழு config_cmp(கட்டமைப்பு list_head *a, கட்டமைப்பு list_head *b,
		      வெறுமை *data __attribute__((__unused__)))
{
	கட்டமைப்பு file_element *ea = list_entry(a, கட்டமைப்பு file_element, file_list);
	கட்டமைப்பு file_element *eb = list_entry(b, கட்டமைப்பு file_element, file_list);
	எழுத்து *na = ul_basename(ea->filename);
	எழுத்து *nb = ul_basename(eb->filename);

	திரும்பு strcoll(na, nb);
}

நிலையான வெறுமை free_list_entry(கட்டமைப்பு file_element *element)
{
	விடுவி(element->filename);
	விடுவி(element);
}

நிலையான முழு config_merge_list(கட்டமைப்பு list_head *main_list,
			      கட்டமைப்பு list_head *new_list)
{
	கட்டமைப்பு list_head *n, *m, *next;

	list_for_each_safe(n, next, new_list) {
		கட்டமைப்பு file_element *ne = list_entry(n, கட்டமைப்பு file_element, file_list);
		எழுத்து *nn = ul_basename(ne->filename);
		முழு duplicate = 0;

		list_for_each(m, main_list) {
			கட்டமைப்பு file_element *me = list_entry(m, கட்டமைப்பு file_element, file_list);
			எழுத்து *mn = ul_basename(me->filename);

			எனில் (strcoll(mn, nn) == 0) {
				duplicate = 1;
				நிறுத்து;
			}
		}

		எனில் (!duplicate) {
			list_del_init(n);
			list_add_tail(n, main_list);
		}
	}

	list_sort(main_list, config_cmp, வெற்று);
	திரும்பு 0;
}

முழு ul_configs_file_list(கட்டமைப்பு list_head *file_list,
			 மாறிலி எழுத்து *project,
			 மாறிலி எழுத்து *etcdir,
			 மாறிலி எழுத்து *rundir,
			 மாறிலி எழுத்து *usrdir,
			 மாறிலி எழுத்து *confname,
			 மாறிலி எழுத்து *suffix)
{
	எழுத்து *main_file = வெற்று;
	கட்டமைப்பு list_head etc_list;
	கட்டமைப்பு list_head run_list;
	கட்டமைப்பு list_head usr_list;
	முழு ret;

	INIT_LIST_HEAD(file_list);

	எனில் (!confname)
		திரும்பு -ENOTEMPTY;

	/* Default is /etc */
	எனில் (!etcdir)
		etcdir = _PATH_SYSCONFDIR;
	எனில் (!rundir)
		rundir = "";
	எனில் (!usrdir)
		usrdir = "";

	எனில் (!project)
		project = "";

	/* Find "main" config file (but don't add to list yet) */
	/* Search order: /etc /run /usr */
	main_file = main_configs(etcdir, project, confname, suffix);
	எனில் (main_file == வெற்று)
		main_file = main_configs(rundir, project, confname, suffix);
	எனில் (main_file == வெற்று)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_HEAD(&etc_list);
	INIT_LIST_HEAD(&run_list);
	INIT_LIST_HEAD(&usr_list);

#எனில் defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	எனில் (ret == -ENOMEM)
		செல் finish;

	ret = read_dir(&run_list, project, rundir, confname, suffix);
	எனில் (ret == -ENOMEM)
		செல் finish;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	எனில் (ret == -ENOMEM)
		செல் finish;
#endif

	/* Merge drop-in directories in priority order (high to low) */
	எனில் ((ret = config_merge_list(file_list, &etc_list)) < 0 ||
	    (ret = config_merge_list(file_list, &run_list)) < 0 ||
	    (ret = config_merge_list(file_list, &usr_list)) < 0)
		செல் finish;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	எனில் (main_file != வெற்று) {
		ret = configs_refer_filename(file_list, main_file, 1);
		எனில் (ret < 0) {
			விடுவி(main_file);
			செல் finish;
		}
	}

finish:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&run_list);
	ul_configs_free_list(&usr_list);

	திரும்பு ret < 0 ? ret : (முழு) list_மொத்தம்_entries(file_list);
}

வெறுமை ul_configs_free_list(கட்டமைப்பு list_head *file_list)
{
	list_free(file_list, கட்டமைப்பு file_element,  file_list, free_list_entry);
}

முழு ul_configs_next_filename(கட்டமைப்பு list_head *file_list,
			     கட்டமைப்பு list_head **current_entry,
			     எழுத்து **name)
{
	கட்டமைப்பு file_element *element = வெற்று;

	எனில் (list_empty(file_list) || *current_entry == file_list)
		திரும்பு 1;

	எனில் (*current_entry == வெற்று)
		*current_entry = file_list->next;

	element = list_entry(*current_entry, கட்டமைப்பு file_element, file_list);
	*name = element->filename;
	*current_entry = (*current_entry)->next;

	திரும்பு 0;
}

#ifdef TEST_PROGRAM_CONFIGS
# include <getopt.h>

முழு முதன்மை(முழு argc, எழுத்து *argv[])
{
	கட்டமைப்பு list_head file_list;
	கட்டமைப்பு list_head *current = வெற்று;
	எழுத்து *name = வெற்று;
	மாறிலி எழுத்து *etc_path = வெற்று;
	மாறிலி எழுத்து *run_path = வெற்று;
	மாறிலி எழுத்து *usr_path = வெற்று;
	மாறிலி எழுத்து *project = வெற்று;
	மாறிலி எழுத்து *config_name = வெற்று;
	மாறிலி எழுத்து *config_suffix = வெற்று;
	நிலையான மாறிலி கட்டமைப்பு option longopts[] = {
		{ "etc", required_argument, வெற்று, 'e' },
		{ "run", required_argument, வெற்று, 'r' },
		{ "usr", required_argument, வெற்று, 'u' },
		{ "project", required_argument, வெற்று, 'p' },
		{ "name", required_argument, வெற்று, 'n' },
		{ "suffix", required_argument, வெற்று, 's' },
		{ "help", no_argument, வெற்று, 'h' },
		{ வெற்று, 0, வெற்று, 0 }
	};
	முழு ch, மொத்தம்;

	போது ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, வெற்று)) != -1) {
		தேர்வு (ch) {
		நிலை 'e':
			etc_path = optarg;
			நிறுத்து;
		நிலை 'r':
			run_path = optarg;
			நிறுத்து;
		நிலை 'u':
			usr_path = optarg;
			நிறுத்து;
		நிலை 'p':
			project = optarg;
			நிறுத்து;
		நிலை 'n':
			config_name = optarg;
			நிறுத்து;
		நிலை 's':
			config_suffix = optarg;
			நிறுத்து;
		நிலை 'h':
			printf("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				program_invocation_short_name);
			திரும்பு EXIT_SUCCESS;
		இயல்பு:
			திரும்பு EXIT_FAILURE;
		}
	}

	எனில் (!config_name) {
		fprintf(stderr, "config name is required (use --name)\n");
		திரும்பு EXIT_FAILURE;
	}

	மொத்தம் = ul_configs_file_list(&file_list, project, etc_path, run_path, usr_path,
				     config_name, config_suffix);
	எனில் (மொத்தம் < 0) {
		fprintf(stderr, "failed to get config file list: %d\n", மொத்தம்);
		திரும்பு EXIT_FAILURE;
	}

	printf("Found %d configuration file(s):\n", மொத்தம்);
	போது (ul_configs_next_filename(&file_list, &current, &name) == 0)
		printf("  %s\n", name);

	ul_configs_free_list(&file_list);
	திரும்பு EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_CONFIGS */
