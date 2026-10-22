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
#ಆದರೆ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
#ಅನ್ತರ್ಭಾವ <dirent.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "configs.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"
#ಅನ್ತರ್ಭಾವ "list.h"
#ಅನ್ತರ್ಭಾವ "fileutils.h"

ರಚನೆ file_element {
	ರಚನೆ list_ತಲೆ file_list;
	ಅಕ್ಷರ *filename;
};

ಸ್ಥಿರ __attribute__ ((__format__ (__printf__, 2, 0)))
	ಅಕ್ಷರ *config_mk_ಮಾರ್ಗ(ಪೂರ್ಣಾಂಕ types, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
	va_list ap;
	ಅಕ್ಷರ *ಮಾರ್ಗ = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ rc;

	errno = 0;

	va_start(ap, fmt);
	rc = vasprintf(&ಮಾರ್ಗ, fmt, ap);
	va_end(ap);

	ಆದರೆ (rc > 0 && types) {
		ರಚನೆ stat st;

		ಆದರೆ (stat(ಮಾರ್ಗ, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			ಹೋಗು fail;

	}

	ಹಿಂತಿರುಗಿ ಮಾರ್ಗ;
fail:
	ಬಿಡುಗಡೆ(ಮಾರ್ಗ);
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
ಸ್ಥಿರ ಅಕ್ಷರ *main_configs(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬೇರು,
			  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *project,
			  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *confname,
			  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *suffix)
{
	ಅಕ್ಷರ *ಮಾರ್ಗ = ಖಾಲಿ;

	ಆದರೆ (suffix)
		ಮಾರ್ಗ = config_mk_ಮಾರ್ಗ(S_IFREG, "%s/%s/%s.%s",
				ಬೇರು, project, confname, suffix);
	ಆದರೆ (!ಮಾರ್ಗ)
		ಮಾರ್ಗ = config_mk_ಮಾರ್ಗ(S_IFREG, "%s/%s/%s",
				ಬೇರು, project, confname);
	ಹಿಂತಿರುಗಿ ಮಾರ್ಗ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ configs_refer_filename(ರಚನೆ list_ತಲೆ *list, ಅಕ್ಷರ *filename, ಪೂರ್ಣಾಂಕ ತಲೆ)
{
	ರಚನೆ file_element *e;

	e = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(*e));
	ಆದರೆ (e == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ -ENOMEM;

	INIT_LIST_ತಲೆ(&e->file_list);
	e->filename = filename;
	ಆದರೆ (ತಲೆ)
		list_ಕೂಡಿಸು(&e->file_list, list);
	ಇಲ್ಲದಿದ್ದರೆ
		list_ಕೂಡಿಸು_ಬಾಲ(&e->file_list, list);
	ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ filter(ಸ್ಥಿರಾಂಕ ರಚನೆ dirent *d)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _DIRENT_HAVE_D_TYPE
	ಆದರೆ (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (is_dotdir_dirent(d))
		ಹಿಂತಿರುಗಿ 0;

	ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ read_dir(ರಚನೆ list_ತಲೆ *file_list,
		    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *project,
		    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬೇರು,
		    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *confname,
		    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *suffix)
{
	ಅಕ್ಷರ *dirname = ಖಾಲಿ;
	ಅಕ್ಷರ *filename = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ dd = -1, nfiles = 0, i;
	ಪೂರ್ಣಾಂಕ ret = 0;
	ರಚನೆ dirent **namelist = ಖಾಲಿ;

	ಆದರೆ (suffix)
		dirname = config_mk_ಮಾರ್ಗ(S_IFDIR, "%s/%s/%s.%s.d",
				ಬೇರು, project, confname, suffix);
	ಆದರೆ (!dirname)
		dirname = config_mk_ಮಾರ್ಗ(S_IFDIR, "%s/%s/%s.d",
				ಬೇರು, project, confname);
	ಆದರೆ (!dirname)
		ಹಿಂತಿರುಗಿ errno == ENOMEM ? -ENOMEM : 0;

	dd = open(dirname, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	ಆದರೆ (dd < 0)
		ಹೋಗು ಮುಕ್ತಾಯ;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	ಆದರೆ (nfiles <= 0)
		ಹೋಗು ಮುಕ್ತಾಯ;

	ಗಾಗಿ (i = 0; i < nfiles; i++) {
		ರಚನೆ dirent *d = namelist[i];

		ಆದರೆ (suffix) {
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = ul_endswith(d->d_name, suffix);

			ಆದರೆ (!p || p == d->d_name || *(p - 1) != '.')
				ಮುಂದುವರಿ;
		}

		ಆದರೆ (asprintf(&filename, "%s/%s", dirname, d->d_name) < 0) {
			ret = -ENOMEM;
			ನಿಲ್ಲಿಸು;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		ಆದರೆ (ret < 0) {
			ಬಿಡುಗಡೆ(filename);
			ನಿಲ್ಲಿಸು;
		}
	}

ಮುಕ್ತಾಯ:
	ಗಾಗಿ (i = 0; i < nfiles; i++)
		ಬಿಡುಗಡೆ(namelist[i]);
	ಬಿಡುಗಡೆ(namelist);
	ಬಿಡುಗಡೆ(dirname);
	ಆದರೆ (dd >= 0)
		close(dd);
	ಹಿಂತಿರುಗಿ ret;
}

#ಆದರೆ_ಅಂತ್ಯ /* HAVE_SCANDIRAT */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ config_cmp(ರಚನೆ list_ತಲೆ *a, ರಚನೆ list_ತಲೆ *b,
		      ಶೂನ್ಯ *ದತ್ತಾಂಶ __attribute__((__unused__)))
{
	ರಚನೆ file_element *ea = list_ನಮೂದು(a, ರಚನೆ file_element, file_list);
	ರಚನೆ file_element *eb = list_ನಮೂದು(b, ರಚನೆ file_element, file_list);
	ಅಕ್ಷರ *na = ul_basename(ea->filename);
	ಅಕ್ಷರ *nb = ul_basename(eb->filename);

	ಹಿಂತಿರುಗಿ strcoll(na, nb);
}

ಸ್ಥಿರ ಶೂನ್ಯ free_list_ನಮೂದು(ರಚನೆ file_element *element)
{
	ಬಿಡುಗಡೆ(element->filename);
	ಬಿಡುಗಡೆ(element);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ config_ವಿಲೀನ_list(ರಚನೆ list_ತಲೆ *main_list,
			      ರಚನೆ list_ತಲೆ *new_list)
{
	ರಚನೆ list_ತಲೆ *n, *m, *ಮುಂದಿನ;

	list_for_each_ಸುರಕ್ಷಿತ(n, ಮುಂದಿನ, new_list) {
		ರಚನೆ file_element *ne = list_ನಮೂದು(n, ರಚನೆ file_element, file_list);
		ಅಕ್ಷರ *nn = ul_basename(ne->filename);
		ಪೂರ್ಣಾಂಕ duplicate = 0;

		list_for_each(m, main_list) {
			ರಚನೆ file_element *me = list_ನಮೂದು(m, ರಚನೆ file_element, file_list);
			ಅಕ್ಷರ *mn = ul_basename(me->filename);

			ಆದರೆ (strcoll(mn, nn) == 0) {
				duplicate = 1;
				ನಿಲ್ಲಿಸು;
			}
		}

		ಆದರೆ (!duplicate) {
			list_del_init(n);
			list_ಕೂಡಿಸು_ಬಾಲ(n, main_list);
		}
	}

	list_ವಿಂಗಡಿಸು(main_list, config_cmp, ಖಾಲಿ);
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ ul_configs_file_list(ರಚನೆ list_ತಲೆ *file_list,
			 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *project,
			 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *etcdir,
			 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *rundir,
			 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *usrdir,
			 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *confname,
			 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *suffix)
{
	ಅಕ್ಷರ *main_file = ಖಾಲಿ;
	ರಚನೆ list_ತಲೆ etc_list;
	ರಚನೆ list_ತಲೆ ಸತತ_list;
	ರಚನೆ list_ತಲೆ usr_list;
	ಪೂರ್ಣಾಂಕ ret;

	INIT_LIST_ತಲೆ(file_list);

	ಆದರೆ (!confname)
		ಹಿಂತಿರುಗಿ -ENOTEMPTY;

	/* Default is /etc */
	ಆದರೆ (!etcdir)
		etcdir = ಮಾರ್ಗ_SYSCONFDIR;
	ಆದರೆ (!rundir)
		rundir = "";
	ಆದರೆ (!usrdir)
		usrdir = "";

	ಆದರೆ (!project)
		project = "";

	/* Find "main" config file (but don't add to list yet) */
	/* Search order: /etc /run /usr */
	main_file = main_configs(etcdir, project, confname, suffix);
	ಆದರೆ (main_file == ಖಾಲಿ)
		main_file = main_configs(rundir, project, confname, suffix);
	ಆದರೆ (main_file == ಖಾಲಿ)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_ತಲೆ(&etc_list);
	INIT_LIST_ತಲೆ(&ಸತತ_list);
	INIT_LIST_ತಲೆ(&usr_list);

#ಆದರೆ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	ಆದರೆ (ret == -ENOMEM)
		ಹೋಗು ಮುಕ್ತಾಯ;

	ret = read_dir(&ಸತತ_list, project, rundir, confname, suffix);
	ಆದರೆ (ret == -ENOMEM)
		ಹೋಗು ಮುಕ್ತಾಯ;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	ಆದರೆ (ret == -ENOMEM)
		ಹೋಗು ಮುಕ್ತಾಯ;
#ಆದರೆ_ಅಂತ್ಯ

	/* Merge drop-in directories in priority order (high to low) */
	ಆದರೆ ((ret = config_ವಿಲೀನ_list(file_list, &etc_list)) < 0 ||
	    (ret = config_ವಿಲೀನ_list(file_list, &ಸತತ_list)) < 0 ||
	    (ret = config_ವಿಲೀನ_list(file_list, &usr_list)) < 0)
		ಹೋಗು ಮುಕ್ತಾಯ;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	ಆದರೆ (main_file != ಖಾಲಿ) {
		ret = configs_refer_filename(file_list, main_file, 1);
		ಆದರೆ (ret < 0) {
			ಬಿಡುಗಡೆ(main_file);
			ಹೋಗು ಮುಕ್ತಾಯ;
		}
	}

ಮುಕ್ತಾಯ:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&ಸತತ_list);
	ul_configs_free_list(&usr_list);

	ಹಿಂತಿರುಗಿ ret < 0 ? ret : (ಪೂರ್ಣಾಂಕ) list_ಎಣಿಕೆ_entries(file_list);
}

ಶೂನ್ಯ ul_configs_free_list(ರಚನೆ list_ತಲೆ *file_list)
{
	list_free(file_list, ರಚನೆ file_element,  file_list, free_list_ನಮೂದು);
}

ಪೂರ್ಣಾಂಕ ul_configs_ಮುಂದಿನ_filename(ರಚನೆ list_ತಲೆ *file_list,
			     ರಚನೆ list_ತಲೆ **current_ನಮೂದು,
			     ಅಕ್ಷರ **name)
{
	ರಚನೆ file_element *element = ಖಾಲಿ;

	ಆದರೆ (list_ಖಾಲಿಗಳು(file_list) || *current_ನಮೂದು == file_list)
		ಹಿಂತಿರುಗಿ 1;

	ಆದರೆ (*current_ನಮೂದು == ಖಾಲಿ)
		*current_ನಮೂದು = file_list->ಮುಂದಿನ;

	element = list_ನಮೂದು(*current_ನಮೂದು, ರಚನೆ file_element, file_list);
	*name = element->filename;
	*current_ನಮೂದು = (*current_ನಮೂದು)->ಮುಂದಿನ;

	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_CONFIGS
# include <getopt.h>

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ರಚನೆ list_ತಲೆ file_list;
	ರಚನೆ list_ತಲೆ *current = ಖಾಲಿ;
	ಅಕ್ಷರ *name = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *etc_ಮಾರ್ಗ = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸತತ_ಮಾರ್ಗ = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *usr_ಮಾರ್ಗ = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *project = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *config_name = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *config_suffix = ಖಾಲಿ;
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[] = {
		{ "etc", required_argument, ಖಾಲಿ, 'e' },
		{ "run", required_argument, ಖಾಲಿ, 'r' },
		{ "usr", required_argument, ಖಾಲಿ, 'u' },
		{ "project", required_argument, ಖಾಲಿ, 'p' },
		{ "name", required_argument, ಖಾಲಿ, 'n' },
		{ "suffix", required_argument, ಖಾಲಿ, 's' },
		{ "help", no_argument, ಖಾಲಿ, 'h' },
		{ ಖಾಲಿ, 0, ಖಾಲಿ, 0 }
	};
	ಪೂರ್ಣಾಂಕ ch, ಎಣಿಕೆ_2;

	ಆಗ ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, ಖಾಲಿ)) != -1) {
		ಆಯ್ಕೆ (ch) {
		ಸ್ಥಿತಿ 'e':
			etc_ಮಾರ್ಗ = optarg;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'r':
			ಸತತ_ಮಾರ್ಗ = optarg;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'u':
			usr_ಮಾರ್ಗ = optarg;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'p':
			project = optarg;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'n':
			config_name = optarg;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 's':
			config_suffix = optarg;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'h':
			ಬರೆ("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				ಕಾರ್ಯಕ್ರಮ_invocation_short_name);
			ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ಹಿಂತಿರುಗಿ EXIT_FAILURE;
		}
	}

	ಆದರೆ (!config_name) {
		fprintf(stderr, "config name is required (use --name)\n");
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	}

	ಎಣಿಕೆ_2 = ul_configs_file_list(&file_list, project, etc_ಮಾರ್ಗ, ಸತತ_ಮಾರ್ಗ, usr_ಮಾರ್ಗ,
				     config_name, config_suffix);
	ಆದರೆ (ಎಣಿಕೆ_2 < 0) {
		fprintf(stderr, "failed to get config file list: %d\n", ಎಣಿಕೆ_2);
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	}

	ಬರೆ("Found %d configuration file(s):\n", ಎಣಿಕೆ_2);
	ಆಗ (ul_configs_ಮುಂದಿನ_filename(&file_list, &current, &name) == 0)
		ಬರೆ("  %s\n", name);

	ul_configs_free_list(&file_list);
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
#ಆದರೆ_ಅಂತ್ಯ /* TEST_PROGRAM_CONFIGS */
