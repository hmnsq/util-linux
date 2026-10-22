/*
 * configs_file.c instantiates functions defined and described in configs_file.h
 */
#అన్తర్భావ <errno.h>
#అన్తర్భావ <sys/syslog.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdio.h>
#ఒకవేళ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
#అన్తర్భావ <dirent.h>
#అయితే_ముగింపు

#అన్తర్భావ "c.h"
#అన్తర్భావ "configs.h"
#అన్తర్భావ "strutils.h"
#అన్తర్భావ "list.h"
#అన్తర్భావ "fileutils.h"

నిర్మాణం file_element {
	నిర్మాణం list_తల file_list;
	అక్షరం *filename;
};

స్థిర __attribute__ ((__format__ (__printf__, 2, 0)))
	అక్షరం *config_mk_మార్గం(పూర్ణాంకం types, స్థిరరాశి అక్షరం *fmt, ...)
{
	va_list ap;
	అక్షరం *మార్గం = ఖాళీ;
	పూర్ణాంకం rc;

	errno = 0;

	va_start(ap, fmt);
	rc = vasprintf(&మార్గం, fmt, ap);
	va_end(ap);

	ఒకవేళ (rc > 0 && types) {
		నిర్మాణం stat st;

		ఒకవేళ (stat(మార్గం, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			వెళ్ళు fail;

	}

	తిరిగి మార్గం;
fail:
	విడుదల(మార్గం);
	తిరిగి ఖాళీ;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
స్థిర అక్షరం *main_configs(స్థిరరాశి అక్షరం *మూలం,
			  స్థిరరాశి అక్షరం *project,
			  స్థిరరాశి అక్షరం *confname,
			  స్థిరరాశి అక్షరం *suffix)
{
	అక్షరం *మార్గం = ఖాళీ;

	ఒకవేళ (suffix)
		మార్గం = config_mk_మార్గం(S_IFREG, "%s/%s/%s.%s",
				మూలం, project, confname, suffix);
	ఒకవేళ (!మార్గం)
		మార్గం = config_mk_మార్గం(S_IFREG, "%s/%s/%s",
				మూలం, project, confname);
	తిరిగి మార్గం;
}

స్థిర పూర్ణాంకం configs_refer_filename(నిర్మాణం list_తల *list, అక్షరం *filename, పూర్ణాంకం తల)
{
	నిర్మాణం file_element *e;

	e = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*e));
	ఒకవేళ (e == ఖాళీ)
		తిరిగి -ENOMEM;

	INIT_LIST_తల(&e->file_list);
	e->filename = filename;
	ఒకవేళ (తల)
		list_కూడు(&e->file_list, list);
	లేకపోతే
		list_కూడు_తోక(&e->file_list, list);
	తిరిగి 0;
}

#ఒకవేళ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

స్థిర పూర్ణాంకం filter(స్థిరరాశి నిర్మాణం dirent *d)
{
#నిర్వచితమైతే _DIRENT_HAVE_D_TYPE
	ఒకవేళ (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		తిరిగి 0;
#అయితే_ముగింపు
	ఒకవేళ (is_dotdir_dirent(d))
		తిరిగి 0;

	తిరిగి 1;
}

స్థిర పూర్ణాంకం read_dir(నిర్మాణం list_తల *file_list,
		    స్థిరరాశి అక్షరం *project,
		    స్థిరరాశి అక్షరం *మూలం,
		    స్థిరరాశి అక్షరం *confname,
		    స్థిరరాశి అక్షరం *suffix)
{
	అక్షరం *dirname = ఖాళీ;
	అక్షరం *filename = ఖాళీ;
	పూర్ణాంకం dd = -1, nfiles = 0, i;
	పూర్ణాంకం ret = 0;
	నిర్మాణం dirent **namelist = ఖాళీ;

	ఒకవేళ (suffix)
		dirname = config_mk_మార్గం(S_IFDIR, "%s/%s/%s.%s.d",
				మూలం, project, confname, suffix);
	ఒకవేళ (!dirname)
		dirname = config_mk_మార్గం(S_IFDIR, "%s/%s/%s.d",
				మూలం, project, confname);
	ఒకవేళ (!dirname)
		తిరిగి errno == ENOMEM ? -ENOMEM : 0;

	dd = open(dirname, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	ఒకవేళ (dd < 0)
		వెళ్ళు ముగింపు;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	ఒకవేళ (nfiles <= 0)
		వెళ్ళు ముగింపు;

	కొరకు (i = 0; i < nfiles; i++) {
		నిర్మాణం dirent *d = namelist[i];

		ఒకవేళ (suffix) {
			స్థిరరాశి అక్షరం *p = ul_endswith(d->d_name, suffix);

			ఒకవేళ (!p || p == d->d_name || *(p - 1) != '.')
				కొనసాగు;
		}

		ఒకవేళ (asprintf(&filename, "%s/%s", dirname, d->d_name) < 0) {
			ret = -ENOMEM;
			ఆపు;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		ఒకవేళ (ret < 0) {
			విడుదల(filename);
			ఆపు;
		}
	}

ముగింపు:
	కొరకు (i = 0; i < nfiles; i++)
		విడుదల(namelist[i]);
	విడుదల(namelist);
	విడుదల(dirname);
	ఒకవేళ (dd >= 0)
		close(dd);
	తిరిగి ret;
}

#అయితే_ముగింపు /* HAVE_SCANDIRAT */

స్థిర పూర్ణాంకం config_cmp(నిర్మాణం list_తల *a, నిర్మాణం list_తల *b,
		      శూన్య *దత్తాంశం __attribute__((__unused__)))
{
	నిర్మాణం file_element *ea = list_నమోదు(a, నిర్మాణం file_element, file_list);
	నిర్మాణం file_element *eb = list_నమోదు(b, నిర్మాణం file_element, file_list);
	అక్షరం *na = ul_basename(ea->filename);
	అక్షరం *nb = ul_basename(eb->filename);

	తిరిగి strcoll(na, nb);
}

స్థిర శూన్య free_list_నమోదు(నిర్మాణం file_element *element)
{
	విడుదల(element->filename);
	విడుదల(element);
}

స్థిర పూర్ణాంకం config_విలీనం_list(నిర్మాణం list_తల *main_list,
			      నిర్మాణం list_తల *new_list)
{
	నిర్మాణం list_తల *n, *m, *తదుపరి;

	list_for_each_సురక్షిత(n, తదుపరి, new_list) {
		నిర్మాణం file_element *ne = list_నమోదు(n, నిర్మాణం file_element, file_list);
		అక్షరం *nn = ul_basename(ne->filename);
		పూర్ణాంకం duplicate = 0;

		list_for_each(m, main_list) {
			నిర్మాణం file_element *me = list_నమోదు(m, నిర్మాణం file_element, file_list);
			అక్షరం *mn = ul_basename(me->filename);

			ఒకవేళ (strcoll(mn, nn) == 0) {
				duplicate = 1;
				ఆపు;
			}
		}

		ఒకవేళ (!duplicate) {
			list_del_init(n);
			list_కూడు_తోక(n, main_list);
		}
	}

	list_క్రమీకరించు(main_list, config_cmp, ఖాళీ);
	తిరిగి 0;
}

పూర్ణాంకం ul_configs_file_list(నిర్మాణం list_తల *file_list,
			 స్థిరరాశి అక్షరం *project,
			 స్థిరరాశి అక్షరం *etcdir,
			 స్థిరరాశి అక్షరం *rundir,
			 స్థిరరాశి అక్షరం *usrdir,
			 స్థిరరాశి అక్షరం *confname,
			 స్థిరరాశి అక్షరం *suffix)
{
	అక్షరం *main_file = ఖాళీ;
	నిర్మాణం list_తల etc_list;
	నిర్మాణం list_తల నిరంతరం_list;
	నిర్మాణం list_తల usr_list;
	పూర్ణాంకం ret;

	INIT_LIST_తల(file_list);

	ఒకవేళ (!confname)
		తిరిగి -ENOTEMPTY;

	/* Default is /etc */
	ఒకవేళ (!etcdir)
		etcdir = మార్గం_SYSCONFDIR;
	ఒకవేళ (!rundir)
		rundir = "";
	ఒకవేళ (!usrdir)
		usrdir = "";

	ఒకవేళ (!project)
		project = "";

	/* Find "main" config file (but don't add to list yet) */
	/* Search order: /etc /run /usr */
	main_file = main_configs(etcdir, project, confname, suffix);
	ఒకవేళ (main_file == ఖాళీ)
		main_file = main_configs(rundir, project, confname, suffix);
	ఒకవేళ (main_file == ఖాళీ)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_తల(&etc_list);
	INIT_LIST_తల(&నిరంతరం_list);
	INIT_LIST_తల(&usr_list);

#ఒకవేళ defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	ఒకవేళ (ret == -ENOMEM)
		వెళ్ళు ముగింపు;

	ret = read_dir(&నిరంతరం_list, project, rundir, confname, suffix);
	ఒకవేళ (ret == -ENOMEM)
		వెళ్ళు ముగింపు;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	ఒకవేళ (ret == -ENOMEM)
		వెళ్ళు ముగింపు;
#అయితే_ముగింపు

	/* Merge drop-in directories in priority order (high to low) */
	ఒకవేళ ((ret = config_విలీనం_list(file_list, &etc_list)) < 0 ||
	    (ret = config_విలీనం_list(file_list, &నిరంతరం_list)) < 0 ||
	    (ret = config_విలీనం_list(file_list, &usr_list)) < 0)
		వెళ్ళు ముగింపు;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	ఒకవేళ (main_file != ఖాళీ) {
		ret = configs_refer_filename(file_list, main_file, 1);
		ఒకవేళ (ret < 0) {
			విడుదల(main_file);
			వెళ్ళు ముగింపు;
		}
	}

ముగింపు:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&నిరంతరం_list);
	ul_configs_free_list(&usr_list);

	తిరిగి ret < 0 ? ret : (పూర్ణాంకం) list_లెక్క_entries(file_list);
}

శూన్య ul_configs_free_list(నిర్మాణం list_తల *file_list)
{
	list_free(file_list, నిర్మాణం file_element,  file_list, free_list_నమోదు);
}

పూర్ణాంకం ul_configs_తదుపరి_filename(నిర్మాణం list_తల *file_list,
			     నిర్మాణం list_తల **current_నమోదు,
			     అక్షరం **name)
{
	నిర్మాణం file_element *element = ఖాళీ;

	ఒకవేళ (list_ఖాళీలు(file_list) || *current_నమోదు == file_list)
		తిరిగి 1;

	ఒకవేళ (*current_నమోదు == ఖాళీ)
		*current_నమోదు = file_list->తదుపరి;

	element = list_నమోదు(*current_నమోదు, నిర్మాణం file_element, file_list);
	*name = element->filename;
	*current_నమోదు = (*current_నమోదు)->తదుపరి;

	తిరిగి 0;
}

#నిర్వచితమైతే TEST_కార్యక్రమం_CONFIGS
# include <getopt.h>

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[])
{
	నిర్మాణం list_తల file_list;
	నిర్మాణం list_తల *current = ఖాళీ;
	అక్షరం *name = ఖాళీ;
	స్థిరరాశి అక్షరం *etc_మార్గం = ఖాళీ;
	స్థిరరాశి అక్షరం *నిరంతరం_మార్గం = ఖాళీ;
	స్థిరరాశి అక్షరం *usr_మార్గం = ఖాళీ;
	స్థిరరాశి అక్షరం *project = ఖాళీ;
	స్థిరరాశి అక్షరం *config_name = ఖాళీ;
	స్థిరరాశి అక్షరం *config_suffix = ఖాళీ;
	స్థిర స్థిరరాశి నిర్మాణం option longopts[] = {
		{ "etc", required_argument, ఖాళీ, 'e' },
		{ "run", required_argument, ఖాళీ, 'r' },
		{ "usr", required_argument, ఖాళీ, 'u' },
		{ "project", required_argument, ఖాళీ, 'p' },
		{ "name", required_argument, ఖాళీ, 'n' },
		{ "suffix", required_argument, ఖాళీ, 's' },
		{ "help", no_argument, ఖాళీ, 'h' },
		{ ఖాళీ, 0, ఖాళీ, 0 }
	};
	పూర్ణాంకం ch, లెక్క;

	అయితే ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, ఖాళీ)) != -1) {
		ఎంపిక (ch) {
		స్థితి 'e':
			etc_మార్గం = optarg;
			ఆపు;
		స్థితి 'r':
			నిరంతరం_మార్గం = optarg;
			ఆపు;
		స్థితి 'u':
			usr_మార్గం = optarg;
			ఆపు;
		స్థితి 'p':
			project = optarg;
			ఆపు;
		స్థితి 'n':
			config_name = optarg;
			ఆపు;
		స్థితి 's':
			config_suffix = optarg;
			ఆపు;
		స్థితి 'h':
			రాయి("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				కార్యక్రమం_invocation_short_name);
			తిరిగి EXIT_SUCCESS;
		అప్రమేయ:
			తిరిగి EXIT_FAILURE;
		}
	}

	ఒకవేళ (!config_name) {
		fprintf(stderr, "config name is required (use --name)\n");
		తిరిగి EXIT_FAILURE;
	}

	లెక్క = ul_configs_file_list(&file_list, project, etc_మార్గం, నిరంతరం_మార్గం, usr_మార్గం,
				     config_name, config_suffix);
	ఒకవేళ (లెక్క < 0) {
		fprintf(stderr, "failed to get config file list: %d\n", లెక్క);
		తిరిగి EXIT_FAILURE;
	}

	రాయి("Found %d configuration file(s):\n", లెక్క);
	అయితే (ul_configs_తదుపరి_filename(&file_list, &current, &name) == 0)
		రాయి("  %s\n", name);

	ul_configs_free_list(&file_list);
	తిరిగి EXIT_SUCCESS;
}
#అయితే_ముగింపు /* TEST_PROGRAM_CONFIGS */
