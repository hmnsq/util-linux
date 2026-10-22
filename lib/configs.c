/*
 * configs_file.c instantiates functions defined and described in configs_file.h
 */
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <sys/syslog.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <stdarg.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଯଦି defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
#ଅନ୍ତର୍ଭାଵ <dirent.h>
#endif

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "configs.h"
#ଅନ୍ତର୍ଭାଵ "strutils.h"
#ଅନ୍ତର୍ଭାଵ "list.h"
#ଅନ୍ତର୍ଭାଵ "fileutils.h"

ସଂରଚନା file_element {
	ସଂରଚନା list_head file_list;
	ଅକ୍ଷର *filename;
};

ସ୍ଥିର __attribute__ ((__format__ (__printf__, 2, 0)))
	ଅକ୍ଷର *config_mk_path(ପୂର୍ଣ୍ଣସଂଖ୍ୟା types, ଅଚଳ ଅକ୍ଷର *fmt, ...)
{
	va_list ap;
	ଅକ୍ଷର *path = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	errno = 0;

	va_start(ap, fmt);
	rc = vasprintf(&path, fmt, ap);
	va_end(ap);

	ଯଦି (rc > 0 && types) {
		ସଂରଚନା stat st;

		ଯଦି (stat(path, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			ଯାଅ fail;

	}

	ଫେରନ୍ତୁ path;
fail:
	ମୁକ୍ତ(path);
	ଫେରନ୍ତୁ ଖାଲି;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
ସ୍ଥିର ଅକ୍ଷର *main_configs(ଅଚଳ ଅକ୍ଷର *ମୂଳ,
			  ଅଚଳ ଅକ୍ଷର *project,
			  ଅଚଳ ଅକ୍ଷର *confname,
			  ଅଚଳ ଅକ୍ଷର *suffix)
{
	ଅକ୍ଷର *path = ଖାଲି;

	ଯଦି (suffix)
		path = config_mk_path(S_IFREG, "%s/%s/%s.%s",
				ମୂଳ, project, confname, suffix);
	ଯଦି (!path)
		path = config_mk_path(S_IFREG, "%s/%s/%s",
				ମୂଳ, project, confname);
	ଫେରନ୍ତୁ path;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା configs_refer_filename(ସଂରଚନା list_head *list, ଅକ୍ଷର *filename, ପୂର୍ଣ୍ଣସଂଖ୍ୟା head)
{
	ସଂରଚନା file_element *e;

	e = calloc(1, ଆକାର(*e));
	ଯଦି (e == ଖାଲି)
		ଫେରନ୍ତୁ -ENOMEM;

	INIT_LIST_HEAD(&e->file_list);
	e->filename = filename;
	ଯଦି (head)
		list_add(&e->file_list, list);
	ଅନ୍ୟଥା
		list_add_tail(&e->file_list, list);
	ଫେରନ୍ତୁ 0;
}

#ଯଦି defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା filter(ଅଚଳ ସଂରଚନା dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	ଯଦି (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		ଫେରନ୍ତୁ 0;
#endif
	ଯଦି (is_dotdir_dirent(d))
		ଫେରନ୍ତୁ 0;

	ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା read_dir(ସଂରଚନା list_head *file_list,
		    ଅଚଳ ଅକ୍ଷର *project,
		    ଅଚଳ ଅକ୍ଷର *ମୂଳ,
		    ଅଚଳ ଅକ୍ଷର *confname,
		    ଅଚଳ ଅକ୍ଷର *suffix)
{
	ଅକ୍ଷର *dirname = ଖାଲି;
	ଅକ୍ଷର *filename = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dd = -1, nfiles = 0, i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;
	ସଂରଚନା dirent **namelist = ଖାଲି;

	ଯଦି (suffix)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.%s.d",
				ମୂଳ, project, confname, suffix);
	ଯଦି (!dirname)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.d",
				ମୂଳ, project, confname);
	ଯଦି (!dirname)
		ଫେରନ୍ତୁ errno == ENOMEM ? -ENOMEM : 0;

	dd = open(dirname, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	ଯଦି (dd < 0)
		ଯାଅ finish;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	ଯଦି (nfiles <= 0)
		ଯାଅ finish;

	ପାଇଁ (i = 0; i < nfiles; i++) {
		ସଂରଚନା dirent *d = namelist[i];

		ଯଦି (suffix) {
			ଅଚଳ ଅକ୍ଷର *p = ul_endswith(d->d_name, suffix);

			ଯଦି (!p || p == d->d_name || *(p - 1) != '.')
				ଜାରି;
		}

		ଯଦି (asprintf(&filename, "%s/%s", dirname, d->d_name) < 0) {
			ret = -ENOMEM;
			ଭାଙ୍ଗ;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		ଯଦି (ret < 0) {
			ମୁକ୍ତ(filename);
			ଭାଙ୍ଗ;
		}
	}

finish:
	ପାଇଁ (i = 0; i < nfiles; i++)
		ମୁକ୍ତ(namelist[i]);
	ମୁକ୍ତ(namelist);
	ମୁକ୍ତ(dirname);
	ଯଦି (dd >= 0)
		close(dd);
	ଫେରନ୍ତୁ ret;
}

#endif /* HAVE_SCANDIRAT */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା config_cmp(ସଂରଚନା list_head *a, ସଂରଚନା list_head *b,
		      ଶୂନ୍ୟ *data __attribute__((__unused__)))
{
	ସଂରଚନା file_element *ea = list_entry(a, ସଂରଚନା file_element, file_list);
	ସଂରଚନା file_element *eb = list_entry(b, ସଂରଚନା file_element, file_list);
	ଅକ୍ଷର *na = ul_basename(ea->filename);
	ଅକ୍ଷର *nb = ul_basename(eb->filename);

	ଫେରନ୍ତୁ strcoll(na, nb);
}

ସ୍ଥିର ଶୂନ୍ୟ free_list_entry(ସଂରଚନା file_element *element)
{
	ମୁକ୍ତ(element->filename);
	ମୁକ୍ତ(element);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା config_merge_list(ସଂରଚନା list_head *main_list,
			      ସଂରଚନା list_head *new_list)
{
	ସଂରଚନା list_head *n, *m, *next;

	list_for_each_safe(n, next, new_list) {
		ସଂରଚନା file_element *ne = list_entry(n, ସଂରଚନା file_element, file_list);
		ଅକ୍ଷର *nn = ul_basename(ne->filename);
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା duplicate = 0;

		list_for_each(m, main_list) {
			ସଂରଚନା file_element *me = list_entry(m, ସଂରଚନା file_element, file_list);
			ଅକ୍ଷର *mn = ul_basename(me->filename);

			ଯଦି (strcoll(mn, nn) == 0) {
				duplicate = 1;
				ଭାଙ୍ଗ;
			}
		}

		ଯଦି (!duplicate) {
			list_del_init(n);
			list_add_tail(n, main_list);
		}
	}

	list_sort(main_list, config_cmp, ଖାଲି);
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_configs_file_list(ସଂରଚନା list_head *file_list,
			 ଅଚଳ ଅକ୍ଷର *project,
			 ଅଚଳ ଅକ୍ଷର *etcdir,
			 ଅଚଳ ଅକ୍ଷର *rundir,
			 ଅଚଳ ଅକ୍ଷର *usrdir,
			 ଅଚଳ ଅକ୍ଷର *confname,
			 ଅଚଳ ଅକ୍ଷର *suffix)
{
	ଅକ୍ଷର *main_file = ଖାଲି;
	ସଂରଚନା list_head etc_list;
	ସଂରଚନା list_head run_list;
	ସଂରଚନା list_head usr_list;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	INIT_LIST_HEAD(file_list);

	ଯଦି (!confname)
		ଫେରନ୍ତୁ -ENOTEMPTY;

	/* Default is /etc */
	ଯଦି (!etcdir)
		etcdir = _PATH_SYSCONFDIR;
	ଯଦି (!rundir)
		rundir = "";
	ଯଦି (!usrdir)
		usrdir = "";

	ଯଦି (!project)
		project = "";

	/* Find "main" config file (but don't add to list yet) */
	/* Search order: /etc /run /usr */
	main_file = main_configs(etcdir, project, confname, suffix);
	ଯଦି (main_file == ଖାଲି)
		main_file = main_configs(rundir, project, confname, suffix);
	ଯଦି (main_file == ଖାଲି)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_HEAD(&etc_list);
	INIT_LIST_HEAD(&run_list);
	INIT_LIST_HEAD(&usr_list);

#ଯଦି defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	ଯଦି (ret == -ENOMEM)
		ଯାଅ finish;

	ret = read_dir(&run_list, project, rundir, confname, suffix);
	ଯଦି (ret == -ENOMEM)
		ଯାଅ finish;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	ଯଦି (ret == -ENOMEM)
		ଯାଅ finish;
#endif

	/* Merge drop-in directories in priority order (high to low) */
	ଯଦି ((ret = config_merge_list(file_list, &etc_list)) < 0 ||
	    (ret = config_merge_list(file_list, &run_list)) < 0 ||
	    (ret = config_merge_list(file_list, &usr_list)) < 0)
		ଯାଅ finish;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	ଯଦି (main_file != ଖାଲି) {
		ret = configs_refer_filename(file_list, main_file, 1);
		ଯଦି (ret < 0) {
			ମୁକ୍ତ(main_file);
			ଯାଅ finish;
		}
	}

finish:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&run_list);
	ul_configs_free_list(&usr_list);

	ଫେରନ୍ତୁ ret < 0 ? ret : (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) list_ଗଣନା_entries(file_list);
}

ଶୂନ୍ୟ ul_configs_free_list(ସଂରଚନା list_head *file_list)
{
	list_free(file_list, ସଂରଚନା file_element,  file_list, free_list_entry);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_configs_next_filename(ସଂରଚନା list_head *file_list,
			     ସଂରଚନା list_head **current_entry,
			     ଅକ୍ଷର **name)
{
	ସଂରଚନା file_element *element = ଖାଲି;

	ଯଦି (list_empty(file_list) || *current_entry == file_list)
		ଫେରନ୍ତୁ 1;

	ଯଦି (*current_entry == ଖାଲି)
		*current_entry = file_list->next;

	element = list_entry(*current_entry, ସଂରଚନା file_element, file_list);
	*name = element->filename;
	*current_entry = (*current_entry)->next;

	ଫେରନ୍ତୁ 0;
}

#ifdef TEST_PROGRAM_CONFIGS
# include <getopt.h>

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ସଂରଚନା list_head file_list;
	ସଂରଚନା list_head *current = ଖାଲି;
	ଅକ୍ଷର *name = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *etc_path = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *run_path = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *usr_path = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *project = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *config_name = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *config_suffix = ଖାଲି;
	ସ୍ଥିର ଅଚଳ ସଂରଚନା option longopts[] = {
		{ "etc", required_argument, ଖାଲି, 'e' },
		{ "run", required_argument, ଖାଲି, 'r' },
		{ "usr", required_argument, ଖାଲି, 'u' },
		{ "project", required_argument, ଖାଲି, 'p' },
		{ "name", required_argument, ଖାଲି, 'n' },
		{ "suffix", required_argument, ଖାଲି, 's' },
		{ "help", no_argument, ଖାଲି, 'h' },
		{ ଖାଲି, 0, ଖାଲି, 0 }
	};
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch, ଗଣନା_2;

	ଯେତେବେଳେ ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, ଖାଲି)) != -1) {
		ବାଛ (ch) {
		ସ୍ଥିତି 'e':
			etc_path = optarg;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'r':
			run_path = optarg;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'u':
			usr_path = optarg;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'p':
			project = optarg;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'n':
			config_name = optarg;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 's':
			config_suffix = optarg;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'h':
			printf("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				program_invocation_short_name);
			ଫେରନ୍ତୁ EXIT_SUCCESS;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ଫେରନ୍ତୁ EXIT_FAILURE;
		}
	}

	ଯଦି (!config_name) {
		fprintf(stderr, "config name is required (use --name)\n");
		ଫେରନ୍ତୁ EXIT_FAILURE;
	}

	ଗଣନା_2 = ul_configs_file_list(&file_list, project, etc_path, run_path, usr_path,
				     config_name, config_suffix);
	ଯଦି (ଗଣନା_2 < 0) {
		fprintf(stderr, "failed to get config file list: %d\n", ଗଣନା_2);
		ଫେରନ୍ତୁ EXIT_FAILURE;
	}

	printf("Found %d configuration file(s):\n", ଗଣନା_2);
	ଯେତେବେଳେ (ul_configs_next_filename(&file_list, &current, &name) == 0)
		printf("  %s\n", name);

	ul_configs_free_list(&file_list);
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_CONFIGS */
