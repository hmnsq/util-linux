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
#endif

#अन्तर्भाव "c.h"
#अन्तर्भाव "configs.h"
#अन्तर्भाव "strutils.h"
#अन्तर्भाव "list.h"
#अन्तर्भाव "fileutils.h"

संरचना file_element {
	संरचना list_head file_list;
	वर्ण *filename;
};

स्थिर __attribute__ ((__format__ (__printf__, 2, 0)))
	वर्ण *config_mk_path(पूर्णाङ्क types, अचर वर्ण *fmt, ...)
{
	बहु_सूची ap;
	वर्ण *path = रिक्त;
	पूर्णाङ्क rc;

	त्रुटि_सं = 0;

	बहु_शुरू(ap, fmt);
	rc = vasprintf(&path, fmt, ap);
	बहु_पूर्ण(ap);

	यदि (rc > 0 && types) {
		संरचना stat st;

		यदि (stat(path, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			गच्छ fail;

	}

	फर्काऊ path;
fail:
	मुक्त(path);
	फर्काऊ रिक्त;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
स्थिर वर्ण *main_configs(अचर वर्ण *जरा,
			  अचर वर्ण *project,
			  अचर वर्ण *confname,
			  अचर वर्ण *suffix)
{
	वर्ण *path = रिक्त;

	यदि (suffix)
		path = config_mk_path(S_IFREG, "%s/%s/%s.%s",
				जरा, project, confname, suffix);
	यदि (!path)
		path = config_mk_path(S_IFREG, "%s/%s/%s",
				जरा, project, confname);
	फर्काऊ path;
}

स्थिर पूर्णाङ्क configs_refer_filename(संरचना list_head *list, वर्ण *filename, पूर्णाङ्क head)
{
	संरचना file_element *e;

	e = सुस्मृति(1, परिमाण(*e));
	यदि (e == रिक्त)
		फर्काऊ -ENOMEM;

	INIT_LIST_HEAD(&e->file_list);
	e->filename = filename;
	यदि (head)
		list_add(&e->file_list, list);
	अन्यथा
		list_add_tail(&e->file_list, list);
	फर्काऊ 0;
}

#यदि defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

स्थिर पूर्णाङ्क filter(अचर संरचना dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	यदि (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		फर्काऊ 0;
#endif
	यदि (is_dotdir_dirent(d))
		फर्काऊ 0;

	फर्काऊ 1;
}

स्थिर पूर्णाङ्क read_dir(संरचना list_head *file_list,
		    अचर वर्ण *project,
		    अचर वर्ण *जरा,
		    अचर वर्ण *confname,
		    अचर वर्ण *suffix)
{
	वर्ण *स_नाम = रिक्त;
	वर्ण *filename = रिक्त;
	पूर्णाङ्क dd = -1, nfiles = 0, i;
	पूर्णाङ्क ret = 0;
	संरचना dirent **namelist = रिक्त;

	यदि (suffix)
		स_नाम = config_mk_path(S_IFDIR, "%s/%s/%s.%s.d",
				जरा, project, confname, suffix);
	यदि (!स_नाम)
		स_नाम = config_mk_path(S_IFDIR, "%s/%s/%s.d",
				जरा, project, confname);
	यदि (!स_नाम)
		फर्काऊ त्रुटि_सं == ENOMEM ? -ENOMEM : 0;

	dd = खोलो(स_नाम, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	यदि (dd < 0)
		गच्छ finish;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	यदि (nfiles <= 0)
		गच्छ finish;

	लागि (i = 0; i < nfiles; i++) {
		संरचना dirent *d = namelist[i];

		यदि (suffix) {
			अचर वर्ण *p = ul_endswith(d->d_name, suffix);

			यदि (!p || p == d->d_name || *(p - 1) != '.')
				जारी;
		}

		यदि (asprintf(&filename, "%s/%s", स_नाम, d->d_name) < 0) {
			ret = -ENOMEM;
			रोक;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		यदि (ret < 0) {
			मुक्त(filename);
			रोक;
		}
	}

finish:
	लागि (i = 0; i < nfiles; i++)
		मुक्त(namelist[i]);
	मुक्त(namelist);
	मुक्त(स_नाम);
	यदि (dd >= 0)
		बंद(dd);
	फर्काऊ ret;
}

#endif /* HAVE_SCANDIRAT */

स्थिर पूर्णाङ्क config_cmp(संरचना list_head *a, संरचना list_head *b,
		      शून्य *data __attribute__((__unused__)))
{
	संरचना file_element *ea = list_entry(a, संरचना file_element, file_list);
	संरचना file_element *eb = list_entry(b, संरचना file_element, file_list);
	वर्ण *na = ul_basename(ea->filename);
	वर्ण *nb = ul_basename(eb->filename);

	फर्काऊ strcoll(na, nb);
}

स्थिर शून्य free_list_entry(संरचना file_element *element)
{
	मुक्त(element->filename);
	मुक्त(element);
}

स्थिर पूर्णाङ्क config_merge_list(संरचना list_head *main_list,
			      संरचना list_head *new_list)
{
	संरचना list_head *n, *m, *next;

	list_for_each_safe(n, next, new_list) {
		संरचना file_element *ne = list_entry(n, संरचना file_element, file_list);
		वर्ण *nn = ul_basename(ne->filename);
		पूर्णाङ्क duplicate = 0;

		list_for_each(m, main_list) {
			संरचना file_element *me = list_entry(m, संरचना file_element, file_list);
			वर्ण *mn = ul_basename(me->filename);

			यदि (strcoll(mn, nn) == 0) {
				duplicate = 1;
				रोक;
			}
		}

		यदि (!duplicate) {
			list_del_init(n);
			list_add_tail(n, main_list);
		}
	}

	list_sort(main_list, config_cmp, रिक्त);
	फर्काऊ 0;
}

पूर्णाङ्क ul_configs_file_list(संरचना list_head *file_list,
			 अचर वर्ण *project,
			 अचर वर्ण *etcdir,
			 अचर वर्ण *rundir,
			 अचर वर्ण *usrdir,
			 अचर वर्ण *confname,
			 अचर वर्ण *suffix)
{
	वर्ण *main_file = रिक्त;
	संरचना list_head etc_list;
	संरचना list_head run_list;
	संरचना list_head usr_list;
	पूर्णाङ्क ret;

	INIT_LIST_HEAD(file_list);

	यदि (!confname)
		फर्काऊ -ENOTEMPTY;

	/* Default is /etc */
	यदि (!etcdir)
		etcdir = _PATH_SYSCONFDIR;
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

	INIT_LIST_HEAD(&etc_list);
	INIT_LIST_HEAD(&run_list);
	INIT_LIST_HEAD(&usr_list);

#यदि defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	यदि (ret == -ENOMEM)
		गच्छ finish;

	ret = read_dir(&run_list, project, rundir, confname, suffix);
	यदि (ret == -ENOMEM)
		गच्छ finish;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	यदि (ret == -ENOMEM)
		गच्छ finish;
#endif

	/* Merge drop-in directories in priority order (high to low) */
	यदि ((ret = config_merge_list(file_list, &etc_list)) < 0 ||
	    (ret = config_merge_list(file_list, &run_list)) < 0 ||
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
	ul_configs_free_list(&run_list);
	ul_configs_free_list(&usr_list);

	फर्काऊ ret < 0 ? ret : (पूर्णाङ्क) list_गन्ती_entries(file_list);
}

शून्य ul_configs_free_list(संरचना list_head *file_list)
{
	list_free(file_list, संरचना file_element,  file_list, free_list_entry);
}

पूर्णाङ्क ul_configs_next_filename(संरचना list_head *file_list,
			     संरचना list_head **current_entry,
			     वर्ण **name)
{
	संरचना file_element *element = रिक्त;

	यदि (list_empty(file_list) || *current_entry == file_list)
		फर्काऊ 1;

	यदि (*current_entry == रिक्त)
		*current_entry = file_list->next;

	element = list_entry(*current_entry, संरचना file_element, file_list);
	*name = element->filename;
	*current_entry = (*current_entry)->next;

	फर्काऊ 0;
}

#ifdef TEST_PROGRAM_CONFIGS
# include <getopt.h>

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	संरचना list_head file_list;
	संरचना list_head *current = रिक्त;
	वर्ण *name = रिक्त;
	अचर वर्ण *etc_path = रिक्त;
	अचर वर्ण *run_path = रिक्त;
	अचर वर्ण *usr_path = रिक्त;
	अचर वर्ण *project = रिक्त;
	अचर वर्ण *config_name = रिक्त;
	अचर वर्ण *config_suffix = रिक्त;
	स्थिर अचर संरचना option longopts[] = {
		{ "etc", required_argument, रिक्त, 'e' },
		{ "run", required_argument, रिक्त, 'r' },
		{ "usr", required_argument, रिक्त, 'u' },
		{ "project", required_argument, रिक्त, 'p' },
		{ "name", required_argument, रिक्त, 'n' },
		{ "suffix", required_argument, रिक्त, 's' },
		{ "help", no_argument, रिक्त, 'h' },
		{ रिक्त, 0, रिक्त, 0 }
	};
	पूर्णाङ्क ch, गन्ती;

	जबसम्म ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, रिक्त)) != -1) {
		विकल्प (ch) {
		पक्ष 'e':
			etc_path = optarg;
			रोक;
		पक्ष 'r':
			run_path = optarg;
			रोक;
		पक्ष 'u':
			usr_path = optarg;
			रोक;
		पक्ष 'p':
			project = optarg;
			रोक;
		पक्ष 'n':
			config_name = optarg;
			रोक;
		पक्ष 's':
			config_suffix = optarg;
			रोक;
		पक्ष 'h':
			म_लिखो("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				program_invocation_short_name);
			फर्काऊ निकास_सफल;
		स्वभाव:
			फर्काऊ निकास_त्रुटि;
		}
	}

	यदि (!config_name) {
		ख_लिखो(मानक_त्रुटि, "config name is required (use --name)\n");
		फर्काऊ निकास_त्रुटि;
	}

	गन्ती = ul_configs_file_list(&file_list, project, etc_path, run_path, usr_path,
				     config_name, config_suffix);
	यदि (गन्ती < 0) {
		ख_लिखो(मानक_त्रुटि, "failed to get config file list: %d\n", गन्ती);
		फर्काऊ निकास_त्रुटि;
	}

	म_लिखो("Found %d configuration file(s):\n", गन्ती);
	जबसम्म (ul_configs_next_filename(&file_list, &current, &name) == 0)
		म_लिखो("  %s\n", name);

	ul_configs_free_list(&file_list);
	फर्काऊ निकास_सफल;
}
#endif /* TEST_PROGRAM_CONFIGS */
