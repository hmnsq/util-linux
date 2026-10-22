/*
 * configs_file.c instantiates functions defined and described in configs_file.h
 */
#अन्तर्भाव <errno.h>
#अन्तर्भाव <sys/syslog.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdio.h>
#यदि defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
#अन्तर्भाव <dirent.h>
#यदि_अन्तः

#अन्तर्भाव "c.h"
#अन्तर्भाव "configs.h"
#अन्तर्भाव "strutils.h"
#अन्तर्भाव "list.h"
#अन्तर्भाव "fileutils.h"

संरचना file_element {
	संरचना list_शीर्ष file_list;
	वर्ण *filename;
};

स्थिर __attribute__ ((__format__ (__printf__, 2, 0)))
	वर्ण *config_mk_मार्गः(पूर्णाङ्क types, ध्रुव वर्ण *fmt, ...)
{
	va_list ap;
	वर्ण *मार्गः = रिक्त;
	पूर्णाङ्क rc;

	errno = 0;

	va_start(ap, fmt);
	rc = vasprintf(&मार्गः, fmt, ap);
	va_end(ap);

	यदि (rc > 0 && types) {
		संरचना stat st;

		यदि (stat(मार्गः, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			गच्छ fail;

	}

	प्रत्यागम मार्गः;
fail:
	मुक्त(मार्गः);
	प्रत्यागम रिक्त;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
स्थिर वर्ण *main_configs(ध्रुव वर्ण *मूलम्,
			  ध्रुव वर्ण *project,
			  ध्रुव वर्ण *confname,
			  ध्रुव वर्ण *suffix)
{
	वर्ण *मार्गः = रिक्त;

	यदि (suffix)
		मार्गः = config_mk_मार्गः(S_IFREG, "%s/%s/%s.%s",
				मूलम्, project, confname, suffix);
	यदि (!मार्गः)
		मार्गः = config_mk_मार्गः(S_IFREG, "%s/%s/%s",
				मूलम्, project, confname);
	प्रत्यागम मार्गः;
}

स्थिर पूर्णाङ्क configs_refer_filename(संरचना list_शीर्ष *list, वर्ण *filename, पूर्णाङ्क शीर्ष)
{
	संरचना file_element *e;

	e = शून्यस्मृतिं_देहि(1, परिमाण(*e));
	यदि (e == रिक्त)
		प्रत्यागम -ENOMEM;

	INIT_LIST_शीर्ष(&e->file_list);
	e->filename = filename;
	यदि (शीर्ष)
		list_सङ्कलय(&e->file_list, list);
	अन्यथा
		list_सङ्कलय_पुच्छ(&e->file_list, list);
	प्रत्यागम 0;
}

#यदि defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

स्थिर पूर्णाङ्क filter(ध्रुव संरचना dirent *d)
{
#यदि_परिभाषितम् _DIRENT_HAVE_D_TYPE
	यदि (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		प्रत्यागम 0;
#यदि_अन्तः
	यदि (is_dotdir_dirent(d))
		प्रत्यागम 0;

	प्रत्यागम 1;
}

स्थिर पूर्णाङ्क read_dir(संरचना list_शीर्ष *file_list,
		    ध्रुव वर्ण *project,
		    ध्रुव वर्ण *मूलम्,
		    ध्रुव वर्ण *confname,
		    ध्रुव वर्ण *suffix)
{
	वर्ण *dirname = रिक्त;
	वर्ण *filename = रिक्त;
	पूर्णाङ्क dd = -1, nfiles = 0, i;
	पूर्णाङ्क ret = 0;
	संरचना dirent **namelist = रिक्त;

	यदि (suffix)
		dirname = config_mk_मार्गः(S_IFDIR, "%s/%s/%s.%s.d",
				मूलम्, project, confname, suffix);
	यदि (!dirname)
		dirname = config_mk_मार्गः(S_IFDIR, "%s/%s/%s.d",
				मूलम्, project, confname);
	यदि (!dirname)
		प्रत्यागम errno == ENOMEM ? -ENOMEM : 0;

	dd = उद्घाटय(dirname, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	यदि (dd < 0)
		गच्छ finish;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	यदि (nfiles <= 0)
		गच्छ finish;

	प्रति (i = 0; i < nfiles; i++) {
		संरचना dirent *d = namelist[i];

		यदि (suffix) {
			ध्रुव वर्ण *p = ul_endswith(d->d_name, suffix);

			यदि (!p || p == d->d_name || *(p - 1) != '.')
				अनुवर्त;
		}

		यदि (asprintf(&filename, "%s/%s", dirname, d->d_name) < 0) {
			ret = -ENOMEM;
			भङ्ग;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		यदि (ret < 0) {
			मुक्त(filename);
			भङ्ग;
		}
	}

finish:
	प्रति (i = 0; i < nfiles; i++)
		मुक्त(namelist[i]);
	मुक्त(namelist);
	मुक्त(dirname);
	यदि (dd >= 0)
		close(dd);
	प्रत्यागम ret;
}

#यदि_अन्तः /* HAVE_SCANDIRAT */

स्थिर पूर्णाङ्क config_cmp(संरचना list_शीर्ष *a, संरचना list_शीर्ष *b,
		      शून्य *दत्त __attribute__((__unused__)))
{
	संरचना file_element *ea = list_प्रविष्टिः(a, संरचना file_element, file_list);
	संरचना file_element *eb = list_प्रविष्टिः(b, संरचना file_element, file_list);
	वर्ण *na = ul_basename(ea->filename);
	वर्ण *nb = ul_basename(eb->filename);

	प्रत्यागम strcoll(na, nb);
}

स्थिर शून्य free_list_प्रविष्टिः(संरचना file_element *element)
{
	मुक्त(element->filename);
	मुक्त(element);
}

स्थिर पूर्णाङ्क config_merge_list(संरचना list_शीर्ष *main_list,
			      संरचना list_शीर्ष *new_list)
{
	संरचना list_शीर्ष *n, *m, *अग्र;

	list_for_each_safe(n, अग्र, new_list) {
		संरचना file_element *ne = list_प्रविष्टिः(n, संरचना file_element, file_list);
		वर्ण *nn = ul_basename(ne->filename);
		पूर्णाङ्क duplicate = 0;

		list_for_each(m, main_list) {
			संरचना file_element *me = list_प्रविष्टिः(m, संरचना file_element, file_list);
			वर्ण *mn = ul_basename(me->filename);

			यदि (strcoll(mn, nn) == 0) {
				duplicate = 1;
				भङ्ग;
			}
		}

		यदि (!duplicate) {
			list_del_init(n);
			list_सङ्कलय_पुच्छ(n, main_list);
		}
	}

	list_क्रमय(main_list, config_cmp, रिक्त);
	प्रत्यागम 0;
}

पूर्णाङ्क ul_configs_file_list(संरचना list_शीर्ष *file_list,
			 ध्रुव वर्ण *project,
			 ध्रुव वर्ण *etcdir,
			 ध्रुव वर्ण *rundir,
			 ध्रुव वर्ण *usrdir,
			 ध्रुव वर्ण *confname,
			 ध्रुव वर्ण *suffix)
{
	वर्ण *main_file = रिक्त;
	संरचना list_शीर्ष etc_list;
	संरचना list_शीर्ष सततम्_list;
	संरचना list_शीर्ष usr_list;
	पूर्णाङ्क ret;

	INIT_LIST_शीर्ष(file_list);

	यदि (!confname)
		प्रत्यागम -ENOTEMPTY;

	/* Default is /etc */
	यदि (!etcdir)
		etcdir = मार्गः_SYSCONFDIR;
	यदि (!rundir)
		rundir = "";
	यदि (!usrdir)
		usrdir = "";

	यदि (!project)
		project = "";

	/* Find "main" config file (but don't add to list yet) */
	/* Search order: /etc /run /usr */
	main_file = main_configs(etcdir, project, confname, suffix);
	यदि (main_file == रिक्त)
		main_file = main_configs(rundir, project, confname, suffix);
	यदि (main_file == रिक्त)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_शीर्ष(&etc_list);
	INIT_LIST_शीर्ष(&सततम्_list);
	INIT_LIST_शीर्ष(&usr_list);

#यदि defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	यदि (ret == -ENOMEM)
		गच्छ finish;

	ret = read_dir(&सततम्_list, project, rundir, confname, suffix);
	यदि (ret == -ENOMEM)
		गच्छ finish;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	यदि (ret == -ENOMEM)
		गच्छ finish;
#यदि_अन्तः

	/* Merge drop-in directories in priority order (high to low) */
	यदि ((ret = config_merge_list(file_list, &etc_list)) < 0 ||
	    (ret = config_merge_list(file_list, &सततम्_list)) < 0 ||
	    (ret = config_merge_list(file_list, &usr_list)) < 0)
		गच्छ finish;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	यदि (main_file != रिक्त) {
		ret = configs_refer_filename(file_list, main_file, 1);
		यदि (ret < 0) {
			मुक्त(main_file);
			गच्छ finish;
		}
	}

finish:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&सततम्_list);
	ul_configs_free_list(&usr_list);

	प्रत्यागम ret < 0 ? ret : (पूर्णाङ्क) list_गणनम्_entries(file_list);
}

शून्य ul_configs_free_list(संरचना list_शीर्ष *file_list)
{
	list_free(file_list, संरचना file_element,  file_list, free_list_प्रविष्टिः);
}

पूर्णाङ्क ul_configs_अग्र_filename(संरचना list_शीर्ष *file_list,
			     संरचना list_शीर्ष **current_प्रविष्टिः,
			     वर्ण **name)
{
	संरचना file_element *element = रिक्त;

	यदि (list_रिक्तानि(file_list) || *current_प्रविष्टिः == file_list)
		प्रत्यागम 1;

	यदि (*current_प्रविष्टिः == रिक्त)
		*current_प्रविष्टिः = file_list->अग्र;

	element = list_प्रविष्टिः(*current_प्रविष्टिः, संरचना file_element, file_list);
	*name = element->filename;
	*current_प्रविष्टिः = (*current_प्रविष्टिः)->अग्र;

	प्रत्यागम 0;
}

#यदि_परिभाषितम् TEST_क्रमादेशः_CONFIGS
# include <getopt.h>

पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण *argv[])
{
	संरचना list_शीर्ष file_list;
	संरचना list_शीर्ष *current = रिक्त;
	वर्ण *name = रिक्त;
	ध्रुव वर्ण *etc_मार्गः = रिक्त;
	ध्रुव वर्ण *सततम्_मार्गः = रिक्त;
	ध्रुव वर्ण *usr_मार्गः = रिक्त;
	ध्रुव वर्ण *project = रिक्त;
	ध्रुव वर्ण *config_name = रिक्त;
	ध्रुव वर्ण *config_suffix = रिक्त;
	स्थिर ध्रुव संरचना option longopts[] = {
		{ "etc", required_argument, रिक्त, 'e' },
		{ "run", required_argument, रिक्त, 'r' },
		{ "usr", required_argument, रिक्त, 'u' },
		{ "project", required_argument, रिक्त, 'p' },
		{ "name", required_argument, रिक्त, 'n' },
		{ "suffix", required_argument, रिक्त, 's' },
		{ "help", no_argument, रिक्त, 'h' },
		{ रिक्त, 0, रिक्त, 0 }
	};
	पूर्णाङ्क ch, गणनम्;

	यावत् ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, रिक्त)) != -1) {
		विकल्प (ch) {
		पक्ष 'e':
			etc_मार्गः = optarg;
			भङ्ग;
		पक्ष 'r':
			सततम्_मार्गः = optarg;
			भङ्ग;
		पक्ष 'u':
			usr_मार्गः = optarg;
			भङ्ग;
		पक्ष 'p':
			project = optarg;
			भङ्ग;
		पक्ष 'n':
			config_name = optarg;
			भङ्ग;
		पक्ष 's':
			config_suffix = optarg;
			भङ्ग;
		पक्ष 'h':
			प्रारूप_लिख("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				क्रमादेशः_invocation_short_name);
			प्रत्यागम EXIT_SUCCESS;
		स्वभाव:
			प्रत्यागम EXIT_FAILURE;
		}
	}

	यदि (!config_name) {
		fprintf(stderr, "config name is required (use --name)\n");
		प्रत्यागम EXIT_FAILURE;
	}

	गणनम् = ul_configs_file_list(&file_list, project, etc_मार्गः, सततम्_मार्गः, usr_मार्गः,
				     config_name, config_suffix);
	यदि (गणनम् < 0) {
		fprintf(stderr, "failed to get config file list: %d\n", गणनम्);
		प्रत्यागम EXIT_FAILURE;
	}

	प्रारूप_लिख("Found %d configuration file(s):\n", गणनम्);
	यावत् (ul_configs_अग्र_filename(&file_list, &current, &name) == 0)
		प्रारूप_लिख("  %s\n", name);

	ul_configs_free_list(&file_list);
	प्रत्यागम EXIT_SUCCESS;
}
#यदि_अन्तः /* TEST_PROGRAM_CONFIGS */
