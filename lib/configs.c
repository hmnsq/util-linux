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
#जर defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
#अन्तर्भाव <dirent.h>
#endif

#अन्तर्भाव "c.h"
#अन्तर्भाव "configs.h"
#अन्तर्भाव "strutils.h"
#अन्तर्भाव "list.h"
#अन्तर्भाव "fileutils.h"

संरचना file_element {
	संरचना list_head file_list;
	अक्षर *filename;
};

स्थिर __attribute__ ((__format__ (__printf__, 2, 0)))
	अक्षर *config_mk_path(पूर्णांक types, स्थिरांक अक्षर *fmt, ...)
{
	बहु_सूची ap;
	अक्षर *path = रिक्त;
	पूर्णांक rc;

	त्रुटि_सं = 0;

	बहु_शुरू(ap, fmt);
	rc = vasprintf(&path, fmt, ap);
	बहु_पूर्ण(ap);

	जर (rc > 0 && types) {
		संरचना stat st;

		जर (stat(path, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			जा fail;

	}

	परत path;
fail:
	मुक्त(path);
	परत रिक्त;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
स्थिर अक्षर *main_configs(स्थिरांक अक्षर *मूळ,
			  स्थिरांक अक्षर *project,
			  स्थिरांक अक्षर *confname,
			  स्थिरांक अक्षर *suffix)
{
	अक्षर *path = रिक्त;

	जर (suffix)
		path = config_mk_path(S_IFREG, "%s/%s/%s.%s",
				मूळ, project, confname, suffix);
	जर (!path)
		path = config_mk_path(S_IFREG, "%s/%s/%s",
				मूळ, project, confname);
	परत path;
}

स्थिर पूर्णांक configs_refer_filename(संरचना list_head *list, अक्षर *filename, पूर्णांक head)
{
	संरचना file_element *e;

	e = सुस्मृति(1, आकार(*e));
	जर (e == रिक्त)
		परत -ENOMEM;

	INIT_LIST_HEAD(&e->file_list);
	e->filename = filename;
	जर (head)
		list_add(&e->file_list, list);
	अन्यथा
		list_add_tail(&e->file_list, list);
	परत 0;
}

#जर defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

स्थिर पूर्णांक filter(स्थिरांक संरचना dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	जर (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		परत 0;
#endif
	जर (is_dotdir_dirent(d))
		परत 0;

	परत 1;
}

स्थिर पूर्णांक read_dir(संरचना list_head *file_list,
		    स्थिरांक अक्षर *project,
		    स्थिरांक अक्षर *मूळ,
		    स्थिरांक अक्षर *confname,
		    स्थिरांक अक्षर *suffix)
{
	अक्षर *स_नाम = रिक्त;
	अक्षर *filename = रिक्त;
	पूर्णांक dd = -1, nfiles = 0, i;
	पूर्णांक ret = 0;
	संरचना dirent **namelist = रिक्त;

	जर (suffix)
		स_नाम = config_mk_path(S_IFDIR, "%s/%s/%s.%s.d",
				मूळ, project, confname, suffix);
	जर (!स_नाम)
		स_नाम = config_mk_path(S_IFDIR, "%s/%s/%s.d",
				मूळ, project, confname);
	जर (!स_नाम)
		परत त्रुटि_सं == ENOMEM ? -ENOMEM : 0;

	dd = खोलो(स_नाम, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	जर (dd < 0)
		जा finish;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	जर (nfiles <= 0)
		जा finish;

	साठी (i = 0; i < nfiles; i++) {
		संरचना dirent *d = namelist[i];

		जर (suffix) {
			स्थिरांक अक्षर *p = ul_endswith(d->d_name, suffix);

			जर (!p || p == d->d_name || *(p - 1) != '.')
				सुरूठेवा;
		}

		जर (asprintf(&filename, "%s/%s", स_नाम, d->d_name) < 0) {
			ret = -ENOMEM;
			थांबा;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		जर (ret < 0) {
			मुक्त(filename);
			थांबा;
		}
	}

finish:
	साठी (i = 0; i < nfiles; i++)
		मुक्त(namelist[i]);
	मुक्त(namelist);
	मुक्त(स_नाम);
	जर (dd >= 0)
		बंद(dd);
	परत ret;
}

#endif /* HAVE_SCANDIRAT */

स्थिर पूर्णांक config_cmp(संरचना list_head *a, संरचना list_head *b,
		      शून्य *data __attribute__((__unused__)))
{
	संरचना file_element *ea = list_entry(a, संरचना file_element, file_list);
	संरचना file_element *eb = list_entry(b, संरचना file_element, file_list);
	अक्षर *na = ul_basename(ea->filename);
	अक्षर *nb = ul_basename(eb->filename);

	परत strcoll(na, nb);
}

स्थिर शून्य free_list_entry(संरचना file_element *element)
{
	मुक्त(element->filename);
	मुक्त(element);
}

स्थिर पूर्णांक config_merge_list(संरचना list_head *main_list,
			      संरचना list_head *new_list)
{
	संरचना list_head *n, *m, *next;

	list_for_each_safe(n, next, new_list) {
		संरचना file_element *ne = list_entry(n, संरचना file_element, file_list);
		अक्षर *nn = ul_basename(ne->filename);
		पूर्णांक duplicate = 0;

		list_for_each(m, main_list) {
			संरचना file_element *me = list_entry(m, संरचना file_element, file_list);
			अक्षर *mn = ul_basename(me->filename);

			जर (strcoll(mn, nn) == 0) {
				duplicate = 1;
				थांबा;
			}
		}

		जर (!duplicate) {
			list_del_init(n);
			list_add_tail(n, main_list);
		}
	}

	list_sort(main_list, config_cmp, रिक्त);
	परत 0;
}

पूर्णांक ul_configs_file_list(संरचना list_head *file_list,
			 स्थिरांक अक्षर *project,
			 स्थिरांक अक्षर *etcdir,
			 स्थिरांक अक्षर *rundir,
			 स्थिरांक अक्षर *usrdir,
			 स्थिरांक अक्षर *confname,
			 स्थिरांक अक्षर *suffix)
{
	अक्षर *main_file = रिक्त;
	संरचना list_head etc_list;
	संरचना list_head run_list;
	संरचना list_head usr_list;
	पूर्णांक ret;

	INIT_LIST_HEAD(file_list);

	जर (!confname)
		परत -ENOTEMPTY;

	/* Default is /etc */
	जर (!etcdir)
		etcdir = _PATH_SYSCONFDIR;
	जर (!rundir)
		rundir = "";
	जर (!usrdir)
		usrdir = "";

	जर (!project)
		project = "";

	/* Find "main" config file (but don't add to list yet) */
	/* Search order: /etc /run /usr */
	main_file = main_configs(etcdir, project, confname, suffix);
	जर (main_file == रिक्त)
		main_file = main_configs(rundir, project, confname, suffix);
	जर (main_file == रिक्त)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_HEAD(&etc_list);
	INIT_LIST_HEAD(&run_list);
	INIT_LIST_HEAD(&usr_list);

#जर defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	जर (ret == -ENOMEM)
		जा finish;

	ret = read_dir(&run_list, project, rundir, confname, suffix);
	जर (ret == -ENOMEM)
		जा finish;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	जर (ret == -ENOMEM)
		जा finish;
#endif

	/* Merge drop-in directories in priority order (high to low) */
	जर ((ret = config_merge_list(file_list, &etc_list)) < 0 ||
	    (ret = config_merge_list(file_list, &run_list)) < 0 ||
	    (ret = config_merge_list(file_list, &usr_list)) < 0)
		जा finish;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	जर (main_file != रिक्त) {
		ret = configs_refer_filename(file_list, main_file, 1);
		जर (ret < 0) {
			मुक्त(main_file);
			जा finish;
		}
	}

finish:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&run_list);
	ul_configs_free_list(&usr_list);

	परत ret < 0 ? ret : (पूर्णांक) list_मोजणी_entries(file_list);
}

शून्य ul_configs_free_list(संरचना list_head *file_list)
{
	list_free(file_list, संरचना file_element,  file_list, free_list_entry);
}

पूर्णांक ul_configs_next_filename(संरचना list_head *file_list,
			     संरचना list_head **current_entry,
			     अक्षर **name)
{
	संरचना file_element *element = रिक्त;

	जर (list_empty(file_list) || *current_entry == file_list)
		परत 1;

	जर (*current_entry == रिक्त)
		*current_entry = file_list->next;

	element = list_entry(*current_entry, संरचना file_element, file_list);
	*name = element->filename;
	*current_entry = (*current_entry)->next;

	परत 0;
}

#ifdef TEST_PROGRAM_CONFIGS
# include <getopt.h>

पूर्णांक main(पूर्णांक argc, अक्षर *argv[])
{
	संरचना list_head file_list;
	संरचना list_head *current = रिक्त;
	अक्षर *name = रिक्त;
	स्थिरांक अक्षर *etc_path = रिक्त;
	स्थिरांक अक्षर *run_path = रिक्त;
	स्थिरांक अक्षर *usr_path = रिक्त;
	स्थिरांक अक्षर *project = रिक्त;
	स्थिरांक अक्षर *config_name = रिक्त;
	स्थिरांक अक्षर *config_suffix = रिक्त;
	स्थिर स्थिरांक संरचना option longopts[] = {
		{ "etc", required_argument, रिक्त, 'e' },
		{ "run", required_argument, रिक्त, 'r' },
		{ "usr", required_argument, रिक्त, 'u' },
		{ "project", required_argument, रिक्त, 'p' },
		{ "name", required_argument, रिक्त, 'n' },
		{ "suffix", required_argument, रिक्त, 's' },
		{ "help", no_argument, रिक्त, 'h' },
		{ रिक्त, 0, रिक्त, 0 }
	};
	पूर्णांक ch, मोजणी;

	पर्यंत ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, रिक्त)) != -1) {
		निवड (ch) {
		स्थिती 'e':
			etc_path = optarg;
			थांबा;
		स्थिती 'r':
			run_path = optarg;
			थांबा;
		स्थिती 'u':
			usr_path = optarg;
			थांबा;
		स्थिती 'p':
			project = optarg;
			थांबा;
		स्थिती 'n':
			config_name = optarg;
			थांबा;
		स्थिती 's':
			config_suffix = optarg;
			थांबा;
		स्थिती 'h':
			म_लिखो("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				program_invocation_short_name);
			परत निकास_सफल;
		पूर्वनिर्धारित:
			परत निकास_त्रुटि;
		}
	}

	जर (!config_name) {
		ख_लिखो(मानक_त्रुटि, "config name is required (use --name)\n");
		परत निकास_त्रुटि;
	}

	मोजणी = ul_configs_file_list(&file_list, project, etc_path, run_path, usr_path,
				     config_name, config_suffix);
	जर (मोजणी < 0) {
		ख_लिखो(मानक_त्रुटि, "failed to get config file list: %d\n", मोजणी);
		परत निकास_त्रुटि;
	}

	म_लिखो("Found %d configuration file(s):\n", मोजणी);
	पर्यंत (ul_configs_next_filename(&file_list, &current, &name) == 0)
		म_लिखो("  %s\n", name);

	ul_configs_free_list(&file_list);
	परत निकास_सफल;
}
#endif /* TEST_PROGRAM_CONFIGS */
