/*
 * configs_file.c instantiates functions defined and described in configs_file.h
 */
#include <errno.h>
#include <sys/syslog.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#གལ་སྲིད defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
#include <dirent.h>
#endif

#include "c.h"
#include "configs.h"
#include "strutils.h"
#include "list.h"
#include "fileutils.h"

sa_nracanaa file_element {
	sa_nracanaa list_head file_list;
	var_na *filename;
};

static __attribute__ ((__format__ (__printf__, 2, 0)))
	var_na *config_mk_path(ཧྲིལ types, const var_na *fmt, ...)
{
	va_list ap;
	var_na *path = NULL;
	ཧྲིལ rc;

	errno = 0;

	va_start(ap, fmt);
	rc = vasprintf(&path, fmt, ap);
	va_end(ap);

	གལ་སྲིད (rc > 0 && types) {
		sa_nracanaa stat st;

		གལ་སྲིད (stat(path, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			goto fail;

	}

	ལོག path;
fail:
	free(path);
	ལོག NULL;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
static var_na *main_configs(const var_na *root,
			  const var_na *project,
			  const var_na *confname,
			  const var_na *suffix)
{
	var_na *path = NULL;

	གལ་སྲིད (suffix)
		path = config_mk_path(S_IFREG, "%s/%s/%s.%s",
				root, project, confname, suffix);
	གལ་སྲིད (!path)
		path = config_mk_path(S_IFREG, "%s/%s/%s",
				root, project, confname);
	ལོག path;
}

static ཧྲིལ configs_refer_filename(sa_nracanaa list_head *list, var_na *filename, ཧྲིལ head)
{
	sa_nracanaa file_element *e;

	e = calloc(1, parimaa_na(*e));
	གལ་སྲིད (e == NULL)
		ལོག -ENOMEM;

	INIT_LIST_HEAD(&e->file_list);
	e->filename = filename;
	གལ་སྲིད (head)
		list_add(&e->file_list, list);
	དེ་མིན
		list_add_tail(&e->file_list, list);
	ལོག 0;
}

#གལ་སྲིད defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

static ཧྲིལ filter(const sa_nracanaa dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	གལ་སྲིད (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		ལོག 0;
#endif
	གལ་སྲིད (is_dotdir_dirent(d))
		ལོག 0;

	ལོག 1;
}

static ཧྲིལ read_dir(sa_nracanaa list_head *file_list,
		    const var_na *project,
		    const var_na *root,
		    const var_na *confname,
		    const var_na *suffix)
{
	var_na *dirname = NULL;
	var_na *filename = NULL;
	ཧྲིལ dd = -1, nfiles = 0, i;
	ཧྲིལ ret = 0;
	sa_nracanaa dirent **namelist = NULL;

	གལ་སྲིད (suffix)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.%s.d",
				root, project, confname, suffix);
	གལ་སྲིད (!dirname)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.d",
				root, project, confname);
	གལ་སྲིད (!dirname)
		ལོག errno == ENOMEM ? -ENOMEM : 0;

	dd = open(dirname, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	གལ་སྲིད (dd < 0)
		goto finish;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	གལ་སྲིད (nfiles <= 0)
		goto finish;

	དོན་ལུ (i = 0; i < nfiles; i++) {
		sa_nracanaa dirent *d = namelist[i];

		གལ་སྲིད (suffix) {
			const var_na *p = ul_endswith(d->d_name, suffix);

			གལ་སྲིད (!p || p == d->d_name || *(p - 1) != '.')
				continue;
		}

		གལ་སྲིད (asprintf(&filename, "%s/%s", dirname, d->d_name) < 0) {
			ret = -ENOMEM;
			bha_nga;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		གལ་སྲིད (ret < 0) {
			free(filename);
			bha_nga;
		}
	}

finish:
	དོན་ལུ (i = 0; i < nfiles; i++)
		free(namelist[i]);
	free(namelist);
	free(dirname);
	གལ་སྲིད (dd >= 0)
		close(dd);
	ལོག ret;
}

#endif /* HAVE_SCANDIRAT */

static ཧྲིལ config_cmp(sa_nracanaa list_head *a, sa_nracanaa list_head *b,
		      void *data __attribute__((__unused__)))
{
	sa_nracanaa file_element *ea = list_entry(a, sa_nracanaa file_element, file_list);
	sa_nracanaa file_element *eb = list_entry(b, sa_nracanaa file_element, file_list);
	var_na *na = ul_basename(ea->filename);
	var_na *nb = ul_basename(eb->filename);

	ལོག strcoll(na, nb);
}

static void free_list_entry(sa_nracanaa file_element *element)
{
	free(element->filename);
	free(element);
}

static ཧྲིལ config_merge_list(sa_nracanaa list_head *main_list,
			      sa_nracanaa list_head *new_list)
{
	sa_nracanaa list_head *n, *m, *next;

	list_for_each_safe(n, next, new_list) {
		sa_nracanaa file_element *ne = list_entry(n, sa_nracanaa file_element, file_list);
		var_na *nn = ul_basename(ne->filename);
		ཧྲིལ duplicate = 0;

		list_for_each(m, main_list) {
			sa_nracanaa file_element *me = list_entry(m, sa_nracanaa file_element, file_list);
			var_na *mn = ul_basename(me->filename);

			གལ་སྲིད (strcoll(mn, nn) == 0) {
				duplicate = 1;
				bha_nga;
			}
		}

		གལ་སྲིད (!duplicate) {
			list_del_init(n);
			list_add_tail(n, main_list);
		}
	}

	list_sort(main_list, config_cmp, NULL);
	ལོག 0;
}

ཧྲིལ ul_configs_file_list(sa_nracanaa list_head *file_list,
			 const var_na *project,
			 const var_na *etcdir,
			 const var_na *rundir,
			 const var_na *usrdir,
			 const var_na *confname,
			 const var_na *suffix)
{
	var_na *main_file = NULL;
	sa_nracanaa list_head etc_list;
	sa_nracanaa list_head run_list;
	sa_nracanaa list_head usr_list;
	ཧྲིལ ret;

	INIT_LIST_HEAD(file_list);

	གལ་སྲིད (!confname)
		ལོག -ENOTEMPTY;

	/* Default is /etc */
	གལ་སྲིད (!etcdir)
		etcdir = _PATH_SYSCONFDIR;
	གལ་སྲིད (!rundir)
		rundir = "";
	གལ་སྲིད (!usrdir)
		usrdir = "";

	གལ་སྲིད (!project)
		project = "";

	/* Find "main" config file (but don't add to list yet) */
	/* Search order: /etc /run /usr */
	main_file = main_configs(etcdir, project, confname, suffix);
	གལ་སྲིད (main_file == NULL)
		main_file = main_configs(rundir, project, confname, suffix);
	གལ་སྲིད (main_file == NULL)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_HEAD(&etc_list);
	INIT_LIST_HEAD(&run_list);
	INIT_LIST_HEAD(&usr_list);

#གལ་སྲིད defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	གལ་སྲིད (ret == -ENOMEM)
		goto finish;

	ret = read_dir(&run_list, project, rundir, confname, suffix);
	གལ་སྲིད (ret == -ENOMEM)
		goto finish;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	གལ་སྲིད (ret == -ENOMEM)
		goto finish;
#endif

	/* Merge drop-in directories in priority order (high to low) */
	གལ་སྲིད ((ret = config_merge_list(file_list, &etc_list)) < 0 ||
	    (ret = config_merge_list(file_list, &run_list)) < 0 ||
	    (ret = config_merge_list(file_list, &usr_list)) < 0)
		goto finish;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	གལ་སྲིད (main_file != NULL) {
		ret = configs_refer_filename(file_list, main_file, 1);
		གལ་སྲིད (ret < 0) {
			free(main_file);
			goto finish;
		}
	}

finish:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&run_list);
	ul_configs_free_list(&usr_list);

	ལོག ret < 0 ? ret : (ཧྲིལ) list_count_entries(file_list);
}

void ul_configs_free_list(sa_nracanaa list_head *file_list)
{
	list_free(file_list, sa_nracanaa file_element,  file_list, free_list_entry);
}

ཧྲིལ ul_configs_next_filename(sa_nracanaa list_head *file_list,
			     sa_nracanaa list_head **current_entry,
			     var_na **name)
{
	sa_nracanaa file_element *element = NULL;

	གལ་སྲིད (list_empty(file_list) || *current_entry == file_list)
		ལོག 1;

	གལ་སྲིད (*current_entry == NULL)
		*current_entry = file_list->next;

	element = list_entry(*current_entry, sa_nracanaa file_element, file_list);
	*name = element->filename;
	*current_entry = (*current_entry)->next;

	ལོག 0;
}

#ifdef TEST_PROGRAM_CONFIGS
# include <getopt.h>

ཧྲིལ main(ཧྲིལ argc, var_na *argv[])
{
	sa_nracanaa list_head file_list;
	sa_nracanaa list_head *current = NULL;
	var_na *name = NULL;
	const var_na *etc_path = NULL;
	const var_na *run_path = NULL;
	const var_na *usr_path = NULL;
	const var_na *project = NULL;
	const var_na *config_name = NULL;
	const var_na *config_suffix = NULL;
	static const sa_nracanaa option longopts[] = {
		{ "etc", required_argument, NULL, 'e' },
		{ "run", required_argument, NULL, 'r' },
		{ "usr", required_argument, NULL, 'u' },
		{ "project", required_argument, NULL, 'p' },
		{ "name", required_argument, NULL, 'n' },
		{ "suffix", required_argument, NULL, 's' },
		{ "help", no_argument, NULL, 'h' },
		{ NULL, 0, NULL, 0 }
	};
	ཧྲིལ ch, count;

	སྐབས ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, NULL)) != -1) {
		switch (ch) {
		pak_sa 'e':
			etc_path = optarg;
			bha_nga;
		pak_sa 'r':
			run_path = optarg;
			bha_nga;
		pak_sa 'u':
			usr_path = optarg;
			bha_nga;
		pak_sa 'p':
			project = optarg;
			bha_nga;
		pak_sa 'n':
			config_name = optarg;
			bha_nga;
		pak_sa 's':
			config_suffix = optarg;
			bha_nga;
		pak_sa 'h':
			printf("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				program_invocation_short_name);
			ལོག EXIT_SUCCESS;
		default:
			ལོག EXIT_FAILURE;
		}
	}

	གལ་སྲིད (!config_name) {
		fprintf(stderr, "config name is required (use --name)\n");
		ལོག EXIT_FAILURE;
	}

	count = ul_configs_file_list(&file_list, project, etc_path, run_path, usr_path,
				     config_name, config_suffix);
	གལ་སྲིད (count < 0) {
		fprintf(stderr, "failed to get config file list: %d\n", count);
		ལོག EXIT_FAILURE;
	}

	printf("Found %d configuration file(s):\n", count);
	སྐབས (ul_configs_next_filename(&file_list, &current, &name) == 0)
		printf("  %s\n", name);

	ul_configs_free_list(&file_list);
	ལོག EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_CONFIGS */
