/*
 * configs_file.c instantiates functions defined and described in configs_file.h
 */
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <sys/syslog.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <stdio.h>
#જો defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
#અન્તર્ભાવ <dirent.h>
#endif

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "configs.h"
#અન્તર્ભાવ "strutils.h"
#અન્તર્ભાવ "list.h"
#અન્તર્ભાવ "fileutils.h"

સંરચના file_element {
	સંરચના list_head file_list;
	અક્ષર *filename;
};

સ્થિર __attribute__ ((__format__ (__printf__, 2, 0)))
	અક્ષર *config_mk_path(પૂર્ણાંક types, અચલ અક્ષર *fmt, ...)
{
	va_list ap;
	અક્ષર *path = ખાલી;
	પૂર્ણાંક rc;

	errno = 0;

	va_start(ap, fmt);
	rc = vasprintf(&path, fmt, ap);
	va_end(ap);

	જો (rc > 0 && types) {
		સંરચના stat st;

		જો (stat(path, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			જાઓ fail;

	}

	પાછા path;
fail:
	મુક્ત(path);
	પાછા ખાલી;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
સ્થિર અક્ષર *main_configs(અચલ અક્ષર *મૂળ,
			  અચલ અક્ષર *project,
			  અચલ અક્ષર *confname,
			  અચલ અક્ષર *suffix)
{
	અક્ષર *path = ખાલી;

	જો (suffix)
		path = config_mk_path(S_IFREG, "%s/%s/%s.%s",
				મૂળ, project, confname, suffix);
	જો (!path)
		path = config_mk_path(S_IFREG, "%s/%s/%s",
				મૂળ, project, confname);
	પાછા path;
}

સ્થિર પૂર્ણાંક configs_refer_filename(સંરચના list_head *list, અક્ષર *filename, પૂર્ણાંક head)
{
	સંરચના file_element *e;

	e = calloc(1, કદ(*e));
	જો (e == ખાલી)
		પાછા -ENOMEM;

	INIT_LIST_HEAD(&e->file_list);
	e->filename = filename;
	જો (head)
		list_add(&e->file_list, list);
	નહીંતર
		list_add_tail(&e->file_list, list);
	પાછા 0;
}

#જો defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

સ્થિર પૂર્ણાંક filter(અચલ સંરચના dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	જો (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		પાછા 0;
#endif
	જો (is_dotdir_dirent(d))
		પાછા 0;

	પાછા 1;
}

સ્થિર પૂર્ણાંક read_dir(સંરચના list_head *file_list,
		    અચલ અક્ષર *project,
		    અચલ અક્ષર *મૂળ,
		    અચલ અક્ષર *confname,
		    અચલ અક્ષર *suffix)
{
	અક્ષર *dirname = ખાલી;
	અક્ષર *filename = ખાલી;
	પૂર્ણાંક dd = -1, nfiles = 0, i;
	પૂર્ણાંક ret = 0;
	સંરચના dirent **namelist = ખાલી;

	જો (suffix)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.%s.d",
				મૂળ, project, confname, suffix);
	જો (!dirname)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.d",
				મૂળ, project, confname);
	જો (!dirname)
		પાછા errno == ENOMEM ? -ENOMEM : 0;

	dd = open(dirname, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	જો (dd < 0)
		જાઓ finish;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	જો (nfiles <= 0)
		જાઓ finish;

	માટે (i = 0; i < nfiles; i++) {
		સંરચના dirent *d = namelist[i];

		જો (suffix) {
			અચલ અક્ષર *p = ul_endswith(d->d_name, suffix);

			જો (!p || p == d->d_name || *(p - 1) != '.')
				ચાલુ;
		}

		જો (asprintf(&filename, "%s/%s", dirname, d->d_name) < 0) {
			ret = -ENOMEM;
			તોડો;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		જો (ret < 0) {
			મુક્ત(filename);
			તોડો;
		}
	}

finish:
	માટે (i = 0; i < nfiles; i++)
		મુક્ત(namelist[i]);
	મુક્ત(namelist);
	મુક્ત(dirname);
	જો (dd >= 0)
		close(dd);
	પાછા ret;
}

#endif /* HAVE_SCANDIRAT */

સ્થિર પૂર્ણાંક config_cmp(સંરચના list_head *a, સંરચના list_head *b,
		      શૂન્ય *data __attribute__((__unused__)))
{
	સંરચના file_element *ea = list_entry(a, સંરચના file_element, file_list);
	સંરચના file_element *eb = list_entry(b, સંરચના file_element, file_list);
	અક્ષર *na = ul_basename(ea->filename);
	અક્ષર *nb = ul_basename(eb->filename);

	પાછા strcoll(na, nb);
}

સ્થિર શૂન્ય free_list_entry(સંરચના file_element *element)
{
	મુક્ત(element->filename);
	મુક્ત(element);
}

સ્થિર પૂર્ણાંક config_merge_list(સંરચના list_head *main_list,
			      સંરચના list_head *new_list)
{
	સંરચના list_head *n, *m, *next;

	list_for_each_safe(n, next, new_list) {
		સંરચના file_element *ne = list_entry(n, સંરચના file_element, file_list);
		અક્ષર *nn = ul_basename(ne->filename);
		પૂર્ણાંક duplicate = 0;

		list_for_each(m, main_list) {
			સંરચના file_element *me = list_entry(m, સંરચના file_element, file_list);
			અક્ષર *mn = ul_basename(me->filename);

			જો (strcoll(mn, nn) == 0) {
				duplicate = 1;
				તોડો;
			}
		}

		જો (!duplicate) {
			list_del_init(n);
			list_add_tail(n, main_list);
		}
	}

	list_sort(main_list, config_cmp, ખાલી);
	પાછા 0;
}

પૂર્ણાંક ul_configs_file_list(સંરચના list_head *file_list,
			 અચલ અક્ષર *project,
			 અચલ અક્ષર *etcdir,
			 અચલ અક્ષર *rundir,
			 અચલ અક્ષર *usrdir,
			 અચલ અક્ષર *confname,
			 અચલ અક્ષર *suffix)
{
	અક્ષર *main_file = ખાલી;
	સંરચના list_head etc_list;
	સંરચના list_head run_list;
	સંરચના list_head usr_list;
	પૂર્ણાંક ret;

	INIT_LIST_HEAD(file_list);

	જો (!confname)
		પાછા -ENOTEMPTY;

	/* Default is /etc */
	જો (!etcdir)
		etcdir = _PATH_SYSCONFDIR;
	જો (!rundir)
		rundir = "";
	જો (!usrdir)
		usrdir = "";

	જો (!project)
		project = "";

	/* Find "main" config file (but don't add to list yet) */
	/* Search order: /etc /run /usr */
	main_file = main_configs(etcdir, project, confname, suffix);
	જો (main_file == ખાલી)
		main_file = main_configs(rundir, project, confname, suffix);
	જો (main_file == ખાલી)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_HEAD(&etc_list);
	INIT_LIST_HEAD(&run_list);
	INIT_LIST_HEAD(&usr_list);

#જો defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	જો (ret == -ENOMEM)
		જાઓ finish;

	ret = read_dir(&run_list, project, rundir, confname, suffix);
	જો (ret == -ENOMEM)
		જાઓ finish;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	જો (ret == -ENOMEM)
		જાઓ finish;
#endif

	/* Merge drop-in directories in priority order (high to low) */
	જો ((ret = config_merge_list(file_list, &etc_list)) < 0 ||
	    (ret = config_merge_list(file_list, &run_list)) < 0 ||
	    (ret = config_merge_list(file_list, &usr_list)) < 0)
		જાઓ finish;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	જો (main_file != ખાલી) {
		ret = configs_refer_filename(file_list, main_file, 1);
		જો (ret < 0) {
			મુક્ત(main_file);
			જાઓ finish;
		}
	}

finish:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&run_list);
	ul_configs_free_list(&usr_list);

	પાછા ret < 0 ? ret : (પૂર્ણાંક) list_ગણતરી_entries(file_list);
}

શૂન્ય ul_configs_free_list(સંરચના list_head *file_list)
{
	list_free(file_list, સંરચના file_element,  file_list, free_list_entry);
}

પૂર્ણાંક ul_configs_next_filename(સંરચના list_head *file_list,
			     સંરચના list_head **current_entry,
			     અક્ષર **name)
{
	સંરચના file_element *element = ખાલી;

	જો (list_empty(file_list) || *current_entry == file_list)
		પાછા 1;

	જો (*current_entry == ખાલી)
		*current_entry = file_list->next;

	element = list_entry(*current_entry, સંરચના file_element, file_list);
	*name = element->filename;
	*current_entry = (*current_entry)->next;

	પાછા 0;
}

#ifdef TEST_PROGRAM_CONFIGS
# include <getopt.h>

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[])
{
	સંરચના list_head file_list;
	સંરચના list_head *current = ખાલી;
	અક્ષર *name = ખાલી;
	અચલ અક્ષર *etc_path = ખાલી;
	અચલ અક્ષર *run_path = ખાલી;
	અચલ અક્ષર *usr_path = ખાલી;
	અચલ અક્ષર *project = ખાલી;
	અચલ અક્ષર *config_name = ખાલી;
	અચલ અક્ષર *config_suffix = ખાલી;
	સ્થિર અચલ સંરચના option longopts[] = {
		{ "etc", required_argument, ખાલી, 'e' },
		{ "run", required_argument, ખાલી, 'r' },
		{ "usr", required_argument, ખાલી, 'u' },
		{ "project", required_argument, ખાલી, 'p' },
		{ "name", required_argument, ખાલી, 'n' },
		{ "suffix", required_argument, ખાલી, 's' },
		{ "help", no_argument, ખાલી, 'h' },
		{ ખાલી, 0, ખાલી, 0 }
	};
	પૂર્ણાંક ch, ગણતરી;

	જ્યારે ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, ખાલી)) != -1) {
		પસંદગી (ch) {
		સ્થિતિ 'e':
			etc_path = optarg;
			તોડો;
		સ્થિતિ 'r':
			run_path = optarg;
			તોડો;
		સ્થિતિ 'u':
			usr_path = optarg;
			તોડો;
		સ્થિતિ 'p':
			project = optarg;
			તોડો;
		સ્થિતિ 'n':
			config_name = optarg;
			તોડો;
		સ્થિતિ 's':
			config_suffix = optarg;
			તોડો;
		સ્થિતિ 'h':
			printf("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				program_invocation_short_name);
			પાછા EXIT_SUCCESS;
		મૂળભૂત:
			પાછા EXIT_FAILURE;
		}
	}

	જો (!config_name) {
		fprintf(stderr, "config name is required (use --name)\n");
		પાછા EXIT_FAILURE;
	}

	ગણતરી = ul_configs_file_list(&file_list, project, etc_path, run_path, usr_path,
				     config_name, config_suffix);
	જો (ગણતરી < 0) {
		fprintf(stderr, "failed to get config file list: %d\n", ગણતરી);
		પાછા EXIT_FAILURE;
	}

	printf("Found %d configuration file(s):\n", ગણતરી);
	જ્યારે (ul_configs_next_filename(&file_list, &current, &name) == 0)
		printf("  %s\n", name);

	ul_configs_free_list(&file_list);
	પાછા EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_CONFIGS */
