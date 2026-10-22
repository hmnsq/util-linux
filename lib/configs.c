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
#якщо defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
#include <dirent.h>
#endif

#include "c.h"
#include "configs.h"
#include "strutils.h"
#include "list.h"
#include "fileutils.h"

struct file_element {
	struct list_head file_list;
	char *filename;
};

static __attribute__ ((__format__ (__printf__, 2, 0)))
	char *config_mk_path(ціле types, const char *fmt, ...)
{
	va_list ap;
	char *path = NULL;
	ціле rc;

	errno = 0;

	va_start(ap, fmt);
	rc = vasprintf(&path, fmt, ap);
	va_end(ap);

	якщо (rc > 0 && types) {
		struct stat st;

		якщо (stat(path, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			goto fail;

	}

	повернути path;
fail:
	free(path);
	повернути NULL;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
static char *main_configs(const char *root,
			  const char *project,
			  const char *confname,
			  const char *suffix)
{
	char *path = NULL;

	якщо (suffix)
		path = config_mk_path(S_IFREG, "%s/%s/%s.%s",
				root, project, confname, suffix);
	якщо (!path)
		path = config_mk_path(S_IFREG, "%s/%s/%s",
				root, project, confname);
	повернути path;
}

static ціле configs_refer_filename(struct list_head *list, char *filename, ціле head)
{
	struct file_element *e;

	e = calloc(1, sizeof(*e));
	якщо (e == NULL)
		повернути -ENOMEM;

	INIT_LIST_HEAD(&e->file_list);
	e->filename = filename;
	якщо (head)
		list_add(&e->file_list, list);
	інакше
		list_add_tail(&e->file_list, list);
	повернути 0;
}

#якщо defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

static ціле filter(const struct dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	якщо (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		повернути 0;
#endif
	якщо (is_dotdir_dirent(d))
		повернути 0;

	повернути 1;
}

static ціле read_dir(struct list_head *file_list,
		    const char *project,
		    const char *root,
		    const char *confname,
		    const char *suffix)
{
	char *dirname = NULL;
	char *filename = NULL;
	ціле dd = -1, nfiles = 0, i;
	ціле ret = 0;
	struct dirent **namelist = NULL;

	якщо (suffix)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.%s.d",
				root, project, confname, suffix);
	якщо (!dirname)
		dirname = config_mk_path(S_IFDIR, "%s/%s/%s.d",
				root, project, confname);
	якщо (!dirname)
		повернути errno == ENOMEM ? -ENOMEM : 0;

	dd = open(dirname, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	якщо (dd < 0)
		goto finish;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	якщо (nfiles <= 0)
		goto finish;

	для (i = 0; i < nfiles; i++) {
		struct dirent *d = namelist[i];

		якщо (suffix) {
			const char *p = ul_endswith(d->d_name, suffix);

			якщо (!p || p == d->d_name || *(p - 1) != '.')
				continue;
		}

		якщо (asprintf(&filename, "%s/%s", dirname, d->d_name) < 0) {
			ret = -ENOMEM;
			break;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		якщо (ret < 0) {
			free(filename);
			break;
		}
	}

finish:
	для (i = 0; i < nfiles; i++)
		free(namelist[i]);
	free(namelist);
	free(dirname);
	якщо (dd >= 0)
		close(dd);
	повернути ret;
}

#endif /* HAVE_SCANDIRAT */

static ціле config_cmp(struct list_head *a, struct list_head *b,
		      void *data __attribute__((__unused__)))
{
	struct file_element *ea = list_entry(a, struct file_element, file_list);
	struct file_element *eb = list_entry(b, struct file_element, file_list);
	char *na = ul_basename(ea->filename);
	char *nb = ul_basename(eb->filename);

	повернути strcoll(na, nb);
}

static void free_list_entry(struct file_element *element)
{
	free(element->filename);
	free(element);
}

static ціле config_merge_list(struct list_head *main_list,
			      struct list_head *new_list)
{
	struct list_head *n, *m, *next;

	list_for_each_safe(n, next, new_list) {
		struct file_element *ne = list_entry(n, struct file_element, file_list);
		char *nn = ul_basename(ne->filename);
		ціле duplicate = 0;

		list_for_each(m, main_list) {
			struct file_element *me = list_entry(m, struct file_element, file_list);
			char *mn = ul_basename(me->filename);

			якщо (strcoll(mn, nn) == 0) {
				duplicate = 1;
				break;
			}
		}

		якщо (!duplicate) {
			list_del_init(n);
			list_add_tail(n, main_list);
		}
	}

	list_sort(main_list, config_cmp, NULL);
	повернути 0;
}

ціле ul_configs_file_list(struct list_head *file_list,
			 const char *project,
			 const char *etcdir,
			 const char *rundir,
			 const char *usrdir,
			 const char *confname,
			 const char *suffix)
{
	char *main_file = NULL;
	struct list_head etc_list;
	struct list_head run_list;
	struct list_head usr_list;
	ціле ret;

	INIT_LIST_HEAD(file_list);

	якщо (!confname)
		повернути -ENOTEMPTY;

	/* Default is /etc */
	якщо (!etcdir)
		etcdir = _PATH_SYSCONFDIR;
	якщо (!rundir)
		rundir = "";
	якщо (!usrdir)
		usrdir = "";

	якщо (!project)
		project = "";

	/* Find "main" config file (but don't add to list yet) */
	/* Search order: /etc /run /usr */
	main_file = main_configs(etcdir, project, confname, suffix);
	якщо (main_file == NULL)
		main_file = main_configs(rundir, project, confname, suffix);
	якщо (main_file == NULL)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_HEAD(&etc_list);
	INIT_LIST_HEAD(&run_list);
	INIT_LIST_HEAD(&usr_list);

#якщо defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	якщо (ret == -ENOMEM)
		goto finish;

	ret = read_dir(&run_list, project, rundir, confname, suffix);
	якщо (ret == -ENOMEM)
		goto finish;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	якщо (ret == -ENOMEM)
		goto finish;
#endif

	/* Merge drop-in directories in priority order (high to low) */
	якщо ((ret = config_merge_list(file_list, &etc_list)) < 0 ||
	    (ret = config_merge_list(file_list, &run_list)) < 0 ||
	    (ret = config_merge_list(file_list, &usr_list)) < 0)
		goto finish;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	якщо (main_file != NULL) {
		ret = configs_refer_filename(file_list, main_file, 1);
		якщо (ret < 0) {
			free(main_file);
			goto finish;
		}
	}

finish:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&run_list);
	ul_configs_free_list(&usr_list);

	повернути ret < 0 ? ret : (ціле) list_count_entries(file_list);
}

void ul_configs_free_list(struct list_head *file_list)
{
	list_free(file_list, struct file_element,  file_list, free_list_entry);
}

ціле ul_configs_next_filename(struct list_head *file_list,
			     struct list_head **current_entry,
			     char **name)
{
	struct file_element *element = NULL;

	якщо (list_empty(file_list) || *current_entry == file_list)
		повернути 1;

	якщо (*current_entry == NULL)
		*current_entry = file_list->next;

	element = list_entry(*current_entry, struct file_element, file_list);
	*name = element->filename;
	*current_entry = (*current_entry)->next;

	повернути 0;
}

#ifdef TEST_PROGRAM_CONFIGS
# include <getopt.h>

ціле main(ціле argc, char *argv[])
{
	struct list_head file_list;
	struct list_head *current = NULL;
	char *name = NULL;
	const char *etc_path = NULL;
	const char *run_path = NULL;
	const char *usr_path = NULL;
	const char *project = NULL;
	const char *config_name = NULL;
	const char *config_suffix = NULL;
	static const struct option longopts[] = {
		{ "etc", required_argument, NULL, 'e' },
		{ "run", required_argument, NULL, 'r' },
		{ "usr", required_argument, NULL, 'u' },
		{ "project", required_argument, NULL, 'p' },
		{ "name", required_argument, NULL, 'n' },
		{ "suffix", required_argument, NULL, 's' },
		{ "help", no_argument, NULL, 'h' },
		{ NULL, 0, NULL, 0 }
	};
	ціле ch, count;

	поки ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, NULL)) != -1) {
		switch (ch) {
		case 'e':
			etc_path = optarg;
			break;
		case 'r':
			run_path = optarg;
			break;
		case 'u':
			usr_path = optarg;
			break;
		case 'p':
			project = optarg;
			break;
		case 'n':
			config_name = optarg;
			break;
		case 's':
			config_suffix = optarg;
			break;
		case 'h':
			printf("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				program_invocation_short_name);
			повернути EXIT_SUCCESS;
		default:
			повернути EXIT_FAILURE;
		}
	}

	якщо (!config_name) {
		fprintf(stderr, "config name is required (use --name)\n");
		повернути EXIT_FAILURE;
	}

	count = ul_configs_file_list(&file_list, project, etc_path, run_path, usr_path,
				     config_name, config_suffix);
	якщо (count < 0) {
		fprintf(stderr, "failed to get config file list: %d\n", count);
		повернути EXIT_FAILURE;
	}

	printf("Found %d configuration file(s):\n", count);
	поки (ul_configs_next_filename(&file_list, &current, &name) == 0)
		printf("  %s\n", name);

	ul_configs_free_list(&file_list);
	повернути EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_CONFIGS */
