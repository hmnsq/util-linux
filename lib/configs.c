/*
 * configs_file.c instantiates functions defined and described in configs_file.h
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/syslog.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਜੇ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
#ਅਨ੍ਤਰ੍ਭਾਵ <dirent.h>
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "configs.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "strutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "list.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "fileutils.h"

ਬਣਤਰ file_element {
	ਬਣਤਰ list_ਸਿਰ file_list;
	ਅੱਖਰ *filename;
};

ਸਥਿਰ __attribute__ ((__format__ (__printf__, 2, 0)))
	ਅੱਖਰ *config_mk_ਰਸਤਾ(ਪੂਰਨ types, ਅਚਲ ਅੱਖਰ *fmt, ...)
{
	va_list ap;
	ਅੱਖਰ *ਰਸਤਾ = ਨਿਰਾ;
	ਪੂਰਨ rc;

	errno = 0;

	va_start(ap, fmt);
	rc = vasprintf(&ਰਸਤਾ, fmt, ap);
	va_end(ap);

	ਜੇ (rc > 0 && types) {
		ਬਣਤਰ stat st;

		ਜੇ (stat(ਰਸਤਾ, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			ਜਾਓ fail;

	}

	ਵਾਪਸ ਰਸਤਾ;
fail:
	ਮੁਕਤ(ਰਸਤਾ);
	ਵਾਪਸ ਨਿਰਾ;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
ਸਥਿਰ ਅੱਖਰ *main_configs(ਅਚਲ ਅੱਖਰ *ਜੜ੍ਹ,
			  ਅਚਲ ਅੱਖਰ *project,
			  ਅਚਲ ਅੱਖਰ *confname,
			  ਅਚਲ ਅੱਖਰ *suffix)
{
	ਅੱਖਰ *ਰਸਤਾ = ਨਿਰਾ;

	ਜੇ (suffix)
		ਰਸਤਾ = config_mk_ਰਸਤਾ(S_IFREG, "%s/%s/%s.%s",
				ਜੜ੍ਹ, project, confname, suffix);
	ਜੇ (!ਰਸਤਾ)
		ਰਸਤਾ = config_mk_ਰਸਤਾ(S_IFREG, "%s/%s/%s",
				ਜੜ੍ਹ, project, confname);
	ਵਾਪਸ ਰਸਤਾ;
}

ਸਥਿਰ ਪੂਰਨ configs_refer_filename(ਬਣਤਰ list_ਸਿਰ *list, ਅੱਖਰ *filename, ਪੂਰਨ ਸਿਰ)
{
	ਬਣਤਰ file_element *e;

	e = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*e));
	ਜੇ (e == ਨਿਰਾ)
		ਵਾਪਸ -ENOMEM;

	INIT_LIST_ਸਿਰ(&e->file_list);
	e->filename = filename;
	ਜੇ (ਸਿਰ)
		list_ਜੋੜੋ(&e->file_list, list);
	ਨਹੀਂਤਾਂ
		list_ਜੋੜੋ_ਪੂਛ(&e->file_list, list);
	ਵਾਪਸ 0;
}

#ਜੇ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

ਸਥਿਰ ਪੂਰਨ filter(ਅਚਲ ਬਣਤਰ dirent *d)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _DIRENT_HAVE_D_TYPE
	ਜੇ (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
	ਜੇ (is_dotdir_dirent(d))
		ਵਾਪਸ 0;

	ਵਾਪਸ 1;
}

ਸਥਿਰ ਪੂਰਨ read_dir(ਬਣਤਰ list_ਸਿਰ *file_list,
		    ਅਚਲ ਅੱਖਰ *project,
		    ਅਚਲ ਅੱਖਰ *ਜੜ੍ਹ,
		    ਅਚਲ ਅੱਖਰ *confname,
		    ਅਚਲ ਅੱਖਰ *suffix)
{
	ਅੱਖਰ *dirname = ਨਿਰਾ;
	ਅੱਖਰ *filename = ਨਿਰਾ;
	ਪੂਰਨ dd = -1, nfiles = 0, i;
	ਪੂਰਨ ret = 0;
	ਬਣਤਰ dirent **namelist = ਨਿਰਾ;

	ਜੇ (suffix)
		dirname = config_mk_ਰਸਤਾ(S_IFDIR, "%s/%s/%s.%s.d",
				ਜੜ੍ਹ, project, confname, suffix);
	ਜੇ (!dirname)
		dirname = config_mk_ਰਸਤਾ(S_IFDIR, "%s/%s/%s.d",
				ਜੜ੍ਹ, project, confname);
	ਜੇ (!dirname)
		ਵਾਪਸ errno == ENOMEM ? -ENOMEM : 0;

	dd = open(dirname, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	ਜੇ (dd < 0)
		ਜਾਓ ਅੰਤ;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	ਜੇ (nfiles <= 0)
		ਜਾਓ ਅੰਤ;

	ਲਈ (i = 0; i < nfiles; i++) {
		ਬਣਤਰ dirent *d = namelist[i];

		ਜੇ (suffix) {
			ਅਚਲ ਅੱਖਰ *p = ul_endswith(d->d_name, suffix);

			ਜੇ (!p || p == d->d_name || *(p - 1) != '.')
				ਜਾਰੀ;
		}

		ਜੇ (asprintf(&filename, "%s/%s", dirname, d->d_name) < 0) {
			ret = -ENOMEM;
			ਰੋਕੋ;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		ਜੇ (ret < 0) {
			ਮੁਕਤ(filename);
			ਰੋਕੋ;
		}
	}

ਅੰਤ:
	ਲਈ (i = 0; i < nfiles; i++)
		ਮੁਕਤ(namelist[i]);
	ਮੁਕਤ(namelist);
	ਮੁਕਤ(dirname);
	ਜੇ (dd >= 0)
		close(dd);
	ਵਾਪਸ ret;
}

#ਜੇ_ਅੰਤ /* HAVE_SCANDIRAT */

ਸਥਿਰ ਪੂਰਨ config_cmp(ਬਣਤਰ list_ਸਿਰ *a, ਬਣਤਰ list_ਸਿਰ *b,
		      ਖਾਲੀ *ਅੰਕੜੇ __attribute__((__unused__)))
{
	ਬਣਤਰ file_element *ea = list_ਇੰਦਰਾਜ(a, ਬਣਤਰ file_element, file_list);
	ਬਣਤਰ file_element *eb = list_ਇੰਦਰਾਜ(b, ਬਣਤਰ file_element, file_list);
	ਅੱਖਰ *na = ul_basename(ea->filename);
	ਅੱਖਰ *nb = ul_basename(eb->filename);

	ਵਾਪਸ strcoll(na, nb);
}

ਸਥਿਰ ਖਾਲੀ free_list_ਇੰਦਰਾਜ(ਬਣਤਰ file_element *element)
{
	ਮੁਕਤ(element->filename);
	ਮੁਕਤ(element);
}

ਸਥਿਰ ਪੂਰਨ config_ਮਿਲਾਓ_list(ਬਣਤਰ list_ਸਿਰ *main_list,
			      ਬਣਤਰ list_ਸਿਰ *new_list)
{
	ਬਣਤਰ list_ਸਿਰ *n, *m, *ਅਗਲਾ;

	list_for_each_ਸੁਰੱਖਿਅਤ(n, ਅਗਲਾ, new_list) {
		ਬਣਤਰ file_element *ne = list_ਇੰਦਰਾਜ(n, ਬਣਤਰ file_element, file_list);
		ਅੱਖਰ *nn = ul_basename(ne->filename);
		ਪੂਰਨ duplicate = 0;

		list_for_each(m, main_list) {
			ਬਣਤਰ file_element *me = list_ਇੰਦਰਾਜ(m, ਬਣਤਰ file_element, file_list);
			ਅੱਖਰ *mn = ul_basename(me->filename);

			ਜੇ (strcoll(mn, nn) == 0) {
				duplicate = 1;
				ਰੋਕੋ;
			}
		}

		ਜੇ (!duplicate) {
			list_del_init(n);
			list_ਜੋੜੋ_ਪੂਛ(n, main_list);
		}
	}

	list_ਛਾਂਟੋ(main_list, config_cmp, ਨਿਰਾ);
	ਵਾਪਸ 0;
}

ਪੂਰਨ ul_configs_file_list(ਬਣਤਰ list_ਸਿਰ *file_list,
			 ਅਚਲ ਅੱਖਰ *project,
			 ਅਚਲ ਅੱਖਰ *etcdir,
			 ਅਚਲ ਅੱਖਰ *rundir,
			 ਅਚਲ ਅੱਖਰ *usrdir,
			 ਅਚਲ ਅੱਖਰ *confname,
			 ਅਚਲ ਅੱਖਰ *suffix)
{
	ਅੱਖਰ *main_file = ਨਿਰਾ;
	ਬਣਤਰ list_ਸਿਰ etc_list;
	ਬਣਤਰ list_ਸਿਰ ਲੜੀਵਾਰ_list;
	ਬਣਤਰ list_ਸਿਰ usr_list;
	ਪੂਰਨ ret;

	INIT_LIST_ਸਿਰ(file_list);

	ਜੇ (!confname)
		ਵਾਪਸ -ENOTEMPTY;

	/* Default is /etc */
	ਜੇ (!etcdir)
		etcdir = ਰਸਤਾ_SYSCONFDIR;
	ਜੇ (!rundir)
		rundir = "";
	ਜੇ (!usrdir)
		usrdir = "";

	ਜੇ (!project)
		project = "";

	/* Find "main" config file (but don't add to list yet) */
	/* Search order: /etc /run /usr */
	main_file = main_configs(etcdir, project, confname, suffix);
	ਜੇ (main_file == ਨਿਰਾ)
		main_file = main_configs(rundir, project, confname, suffix);
	ਜੇ (main_file == ਨਿਰਾ)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_ਸਿਰ(&etc_list);
	INIT_LIST_ਸਿਰ(&ਲੜੀਵਾਰ_list);
	INIT_LIST_ਸਿਰ(&usr_list);

#ਜੇ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	ਜੇ (ret == -ENOMEM)
		ਜਾਓ ਅੰਤ;

	ret = read_dir(&ਲੜੀਵਾਰ_list, project, rundir, confname, suffix);
	ਜੇ (ret == -ENOMEM)
		ਜਾਓ ਅੰਤ;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	ਜੇ (ret == -ENOMEM)
		ਜਾਓ ਅੰਤ;
#ਜੇ_ਅੰਤ

	/* Merge drop-in directories in priority order (high to low) */
	ਜੇ ((ret = config_ਮਿਲਾਓ_list(file_list, &etc_list)) < 0 ||
	    (ret = config_ਮਿਲਾਓ_list(file_list, &ਲੜੀਵਾਰ_list)) < 0 ||
	    (ret = config_ਮਿਲਾਓ_list(file_list, &usr_list)) < 0)
		ਜਾਓ ਅੰਤ;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	ਜੇ (main_file != ਨਿਰਾ) {
		ret = configs_refer_filename(file_list, main_file, 1);
		ਜੇ (ret < 0) {
			ਮੁਕਤ(main_file);
			ਜਾਓ ਅੰਤ;
		}
	}

ਅੰਤ:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&ਲੜੀਵਾਰ_list);
	ul_configs_free_list(&usr_list);

	ਵਾਪਸ ret < 0 ? ret : (ਪੂਰਨ) list_ਗਿਣਤੀ_entries(file_list);
}

ਖਾਲੀ ul_configs_free_list(ਬਣਤਰ list_ਸਿਰ *file_list)
{
	list_free(file_list, ਬਣਤਰ file_element,  file_list, free_list_ਇੰਦਰਾਜ);
}

ਪੂਰਨ ul_configs_ਅਗਲਾ_filename(ਬਣਤਰ list_ਸਿਰ *file_list,
			     ਬਣਤਰ list_ਸਿਰ **current_ਇੰਦਰਾਜ,
			     ਅੱਖਰ **name)
{
	ਬਣਤਰ file_element *element = ਨਿਰਾ;

	ਜੇ (list_ਖਾਲੀ_ਥਾਵਾਂ(file_list) || *current_ਇੰਦਰਾਜ == file_list)
		ਵਾਪਸ 1;

	ਜੇ (*current_ਇੰਦਰਾਜ == ਨਿਰਾ)
		*current_ਇੰਦਰਾਜ = file_list->ਅਗਲਾ;

	element = list_ਇੰਦਰਾਜ(*current_ਇੰਦਰਾਜ, ਬਣਤਰ file_element, file_list);
	*name = element->filename;
	*current_ਇੰਦਰਾਜ = (*current_ਇੰਦਰਾਜ)->ਅਗਲਾ;

	ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_CONFIGS
# include <getopt.h>

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਬਣਤਰ list_ਸਿਰ file_list;
	ਬਣਤਰ list_ਸਿਰ *current = ਨਿਰਾ;
	ਅੱਖਰ *name = ਨਿਰਾ;
	ਅਚਲ ਅੱਖਰ *etc_ਰਸਤਾ = ਨਿਰਾ;
	ਅਚਲ ਅੱਖਰ *ਲੜੀਵਾਰ_ਰਸਤਾ = ਨਿਰਾ;
	ਅਚਲ ਅੱਖਰ *usr_ਰਸਤਾ = ਨਿਰਾ;
	ਅਚਲ ਅੱਖਰ *project = ਨਿਰਾ;
	ਅਚਲ ਅੱਖਰ *config_name = ਨਿਰਾ;
	ਅਚਲ ਅੱਖਰ *config_suffix = ਨਿਰਾ;
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ option longopts[] = {
		{ "etc", required_argument, ਨਿਰਾ, 'e' },
		{ "run", required_argument, ਨਿਰਾ, 'r' },
		{ "usr", required_argument, ਨਿਰਾ, 'u' },
		{ "project", required_argument, ਨਿਰਾ, 'p' },
		{ "name", required_argument, ਨਿਰਾ, 'n' },
		{ "suffix", required_argument, ਨਿਰਾ, 's' },
		{ "help", no_argument, ਨਿਰਾ, 'h' },
		{ ਨਿਰਾ, 0, ਨਿਰਾ, 0 }
	};
	ਪੂਰਨ ch, ਗਿਣਤੀ_2;

	ਜਦੋਂ ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, ਨਿਰਾ)) != -1) {
		ਚੋਣ (ch) {
		ਹਾਲਤ 'e':
			etc_ਰਸਤਾ = optarg;
			ਰੋਕੋ;
		ਹਾਲਤ 'r':
			ਲੜੀਵਾਰ_ਰਸਤਾ = optarg;
			ਰੋਕੋ;
		ਹਾਲਤ 'u':
			usr_ਰਸਤਾ = optarg;
			ਰੋਕੋ;
		ਹਾਲਤ 'p':
			project = optarg;
			ਰੋਕੋ;
		ਹਾਲਤ 'n':
			config_name = optarg;
			ਰੋਕੋ;
		ਹਾਲਤ 's':
			config_suffix = optarg;
			ਰੋਕੋ;
		ਹਾਲਤ 'h':
			ਲਿਖੋ("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				ਪ੍ਰੋਗਰਾਮ_invocation_short_name);
			ਵਾਪਸ EXIT_SUCCESS;
		ਮੂਲ:
			ਵਾਪਸ EXIT_FAILURE;
		}
	}

	ਜੇ (!config_name) {
		fprintf(stderr, "config name is required (use --name)\n");
		ਵਾਪਸ EXIT_FAILURE;
	}

	ਗਿਣਤੀ_2 = ul_configs_file_list(&file_list, project, etc_ਰਸਤਾ, ਲੜੀਵਾਰ_ਰਸਤਾ, usr_ਰਸਤਾ,
				     config_name, config_suffix);
	ਜੇ (ਗਿਣਤੀ_2 < 0) {
		fprintf(stderr, "failed to get config file list: %d\n", ਗਿਣਤੀ_2);
		ਵਾਪਸ EXIT_FAILURE;
	}

	ਲਿਖੋ("Found %d configuration file(s):\n", ਗਿਣਤੀ_2);
	ਜਦੋਂ (ul_configs_ਅਗਲਾ_filename(&file_list, &current, &name) == 0)
		ਲਿਖੋ("  %s\n", name);

	ul_configs_free_list(&file_list);
	ਵਾਪਸ EXIT_SUCCESS;
}
#ਜੇ_ਅੰਤ /* TEST_PROGRAM_CONFIGS */
