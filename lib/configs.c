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
#အကယ်၍ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
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
	var_na *config_mk_path(ကိန်းပြည့် types, const var_na *fmt, ...)
{
	va_list ap;
	var_na *path = NULL;
	ကိန်းပြည့် rc;

	errno = 0;

	va_start(ap, fmt);
	rc = vasprintf(&path, fmt, ap);
	va_end(ap);

	အကယ်၍ (rc > 0 && types) {
		sa_nracanaa stat st;

		အကယ်၍ (stat(path, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			goto fail;

	}

	ပြန်ပေး path;
fail:
	free(path);
	ပြန်ပေး NULL;
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

	အကယ်၍ (suffix)
		path = config_mk_path(S_IFREG, "%s/%s/%s.%s",
				root, project, confname, suffix);
	အကယ်၍ (!path)
		path = config_mk_path(S_IFREG, "%s/%s/%s",
				root, project, confname);
	ပြန်ပေး path;
}

static ကိန်းပြည့် configs_refer_filename(sa_nracanaa list_head *list, var_na *filename, ကိန်းပြည့် head)
{
	sa_nracanaa file_element *e;

	e = calloc(1, parimaa_na(*e));
	အကယ်၍ (e == NULL)
		ပြန်ပေး -ENOMEM;

	INIT_LIST_HEAD(&e->file_list);
	e->filename = filename;
	အကယ်၍ (head)
		list_add(&e->file_list, list);
	မဟုတ်ပါက
		list_add_tail(&e->file_list, list);
	ပြန်ပေး 0;
}

#အကယ်၍ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

static ကိန်းပြည့် filter(const sa_nracanaa dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	အကယ်၍ (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		ပြန်ပေး 0;
#endif
	အကယ်၍ (is_dotdir_dirent(d))
		ပြန်ပေး 0;

	ပြန်ပေး 1;
}

static ကိန်းပြည့် read_dir(sa_nracanaa list_head *file_list,
		    const var_na *project,
		    const var_na *root,
		    const var_na *confname,
		    const var_na *suffix)
{
	var_na *dirname = NULL;
	var_na *filename = NULL;
	ကိန်းပြည့် dd = -1, nfiles = 0, i;
	ကိန်းပြည့် ret = 0;
	sa_nracanaa dirent **namelist = NULL;

	အကယ်၍ (suffix)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.%s.d",
				root, project, confname, suffix);
	အကယ်၍ (!dirname)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.d",
				root, project, confname);
	အကယ်၍ (!dirname)
		ပြန်ပေး errno == ENOMEM ? -ENOMEM : 0;

	dd = open(dirname, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	အကယ်၍ (dd < 0)
		goto finish;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	အကယ်၍ (nfiles <= 0)
		goto finish;

	အတွက် (i = 0; i < nfiles; i++) {
		sa_nracanaa dirent *d = namelist[i];

		အကယ်၍ (suffix) {
			const var_na *p = ul_endswith(d->d_name, suffix);

			အကယ်၍ (!p || p == d->d_name || *(p - 1) != '.')
				continue;
		}

		အကယ်၍ (asprintf(&filename, "%s/%s", dirname, d->d_name) < 0) {
			ret = -ENOMEM;
			bha_nga;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		အကယ်၍ (ret < 0) {
			free(filename);
			bha_nga;
		}
	}

finish:
	အတွက် (i = 0; i < nfiles; i++)
		free(namelist[i]);
	free(namelist);
	free(dirname);
	အကယ်၍ (dd >= 0)
		close(dd);
	ပြန်ပေး ret;
}

#endif /* HAVE_SCANDIRAT */

static ကိန်းပြည့် config_cmp(sa_nracanaa list_head *a, sa_nracanaa list_head *b,
		      void *data __attribute__((__unused__)))
{
	sa_nracanaa file_element *ea = list_entry(a, sa_nracanaa file_element, file_list);
	sa_nracanaa file_element *eb = list_entry(b, sa_nracanaa file_element, file_list);
	var_na *na = ul_basename(ea->filename);
	var_na *nb = ul_basename(eb->filename);

	ပြန်ပေး strcoll(na, nb);
}

static void free_list_entry(sa_nracanaa file_element *element)
{
	free(element->filename);
	free(element);
}

static ကိန်းပြည့် config_merge_list(sa_nracanaa list_head *main_list,
			      sa_nracanaa list_head *new_list)
{
	sa_nracanaa list_head *n, *m, *next;

	list_for_each_safe(n, next, new_list) {
		sa_nracanaa file_element *ne = list_entry(n, sa_nracanaa file_element, file_list);
		var_na *nn = ul_basename(ne->filename);
		ကိန်းပြည့် duplicate = 0;

		list_for_each(m, main_list) {
			sa_nracanaa file_element *me = list_entry(m, sa_nracanaa file_element, file_list);
			var_na *mn = ul_basename(me->filename);

			အကယ်၍ (strcoll(mn, nn) == 0) {
				duplicate = 1;
				bha_nga;
			}
		}

		အကယ်၍ (!duplicate) {
			list_del_init(n);
			list_add_tail(n, main_list);
		}
	}

	list_sort(main_list, config_cmp, NULL);
	ပြန်ပေး 0;
}

ကိန်းပြည့် ul_configs_file_list(sa_nracanaa list_head *file_list,
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
	ကိန်းပြည့် ret;

	INIT_LIST_HEAD(file_list);

	အကယ်၍ (!confname)
		ပြန်ပေး -ENOTEMPTY;

	/* Default is /etc */
	အကယ်၍ (!etcdir)
		etcdir = _PATH_SYSCONFDIR;
	အကယ်၍ (!rundir)
		rundir = "";
	အကယ်၍ (!usrdir)
		usrdir = "";

	အကယ်၍ (!project)
		project = "";

	/* Find "main" config file (but don't add to list yet) */
	/* Search order: /etc /run /usr */
	main_file = main_configs(etcdir, project, confname, suffix);
	အကယ်၍ (main_file == NULL)
		main_file = main_configs(rundir, project, confname, suffix);
	အကယ်၍ (main_file == NULL)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_HEAD(&etc_list);
	INIT_LIST_HEAD(&run_list);
	INIT_LIST_HEAD(&usr_list);

#အကယ်၍ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	အကယ်၍ (ret == -ENOMEM)
		goto finish;

	ret = read_dir(&run_list, project, rundir, confname, suffix);
	အကယ်၍ (ret == -ENOMEM)
		goto finish;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	အကယ်၍ (ret == -ENOMEM)
		goto finish;
#endif

	/* Merge drop-in directories in priority order (high to low) */
	အကယ်၍ ((ret = config_merge_list(file_list, &etc_list)) < 0 ||
	    (ret = config_merge_list(file_list, &run_list)) < 0 ||
	    (ret = config_merge_list(file_list, &usr_list)) < 0)
		goto finish;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	အကယ်၍ (main_file != NULL) {
		ret = configs_refer_filename(file_list, main_file, 1);
		အကယ်၍ (ret < 0) {
			free(main_file);
			goto finish;
		}
	}

finish:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&run_list);
	ul_configs_free_list(&usr_list);

	ပြန်ပေး ret < 0 ? ret : (ကိန်းပြည့်) list_count_entries(file_list);
}

void ul_configs_free_list(sa_nracanaa list_head *file_list)
{
	list_free(file_list, sa_nracanaa file_element,  file_list, free_list_entry);
}

ကိန်းပြည့် ul_configs_next_filename(sa_nracanaa list_head *file_list,
			     sa_nracanaa list_head **current_entry,
			     var_na **name)
{
	sa_nracanaa file_element *element = NULL;

	အကယ်၍ (list_empty(file_list) || *current_entry == file_list)
		ပြန်ပေး 1;

	အကယ်၍ (*current_entry == NULL)
		*current_entry = file_list->next;

	element = list_entry(*current_entry, sa_nracanaa file_element, file_list);
	*name = element->filename;
	*current_entry = (*current_entry)->next;

	ပြန်ပေး 0;
}

#ifdef TEST_PROGRAM_CONFIGS
# include <getopt.h>

ကိန်းပြည့် main(ကိန်းပြည့် argc, var_na *argv[])
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
	ကိန်းပြည့် ch, count;

	နေစဉ် ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, NULL)) != -1) {
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
			ပြန်ပေး EXIT_SUCCESS;
		default:
			ပြန်ပေး EXIT_FAILURE;
		}
	}

	အကယ်၍ (!config_name) {
		fprintf(stderr, "config name is required (use --name)\n");
		ပြန်ပေး EXIT_FAILURE;
	}

	count = ul_configs_file_list(&file_list, project, etc_path, run_path, usr_path,
				     config_name, config_suffix);
	အကယ်၍ (count < 0) {
		fprintf(stderr, "failed to get config file list: %d\n", count);
		ပြန်ပေး EXIT_FAILURE;
	}

	printf("Found %d configuration file(s):\n", count);
	နေစဉ် (ul_configs_next_filename(&file_list, &current, &name) == 0)
		printf("  %s\n", name);

	ul_configs_free_list(&file_list);
	ပြန်ပေး EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_CONFIGS */
