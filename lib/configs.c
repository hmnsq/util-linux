/*
 * configs_file.c instantiates functions defined and described in configs_file.h
 */
#समावेश <errno.h>
#समावेश <sys/syslog.h>
#समावेश <sys/stat.h>
#समावेश <stdarg.h>
#समावेश <stdbool.h>
#समावेश <string.h>
#समावेश <stdlib.h>
#समावेश <stdio.h>
#अगर defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
#समावेश <dirent.h>
#पूर्ण_अगर

#समावेश "c.h"
#समावेश "configs.h"
#समावेश "strutils.h"
#समावेश "list.h"
#समावेश "fileutils.h"

काष्ठा file_element {
	काष्ठा list_शीर्ष file_list;
	अक्षर *filename;
};

अटल __attribute__ ((__format__ (__printf__, 2, 0)))
	अक्षर *config_mk_path(पूर्णांक types, स्थिर अक्षर *fmt, ...)
{
	बहु_सूची ap;
	अक्षर *path = शून्य;
	पूर्णांक rc;

	त्रुटि_सं = 0;

	बहु_शुरू(ap, fmt);
	rc = vasprintf(&path, fmt, ap);
	बहु_पूर्ण(ap);

	अगर (rc > 0 && types) {
		काष्ठा stat st;

		अगर (stat(path, &st) != 0 ||
		    !((st.st_mode & S_IFMT) & types))
			जाओ fail;

	}

	वापस path;
fail:
	मुक्त(path);
	वापस शून्य;
}

/* Checking for main configuration file
 *
 * Returning absolute path or NULL if not found The return value has to be
 * freed by the caller.
 */
अटल अक्षर *main_configs(स्थिर अक्षर *मूल,
			  स्थिर अक्षर *project,
			  स्थिर अक्षर *confname,
			  स्थिर अक्षर *suffix)
{
	अक्षर *path = शून्य;

	अगर (suffix)
		path = config_mk_path(S_IFREG, "%s/%s/%s.%s",
				मूल, project, confname, suffix);
	अगर (!path)
		path = config_mk_path(S_IFREG, "%s/%s/%s",
				मूल, project, confname);
	वापस path;
}

अटल पूर्णांक configs_refer_filename(काष्ठा list_शीर्ष *list, अक्षर *filename, पूर्णांक शीर्ष)
{
	काष्ठा file_element *e;

	e = सुस्मृति(1, माप(*e));
	अगर (e == शून्य)
		वापस -ENOMEM;

	INIT_LIST_शीर्ष(&e->file_list);
	e->filename = filename;
	अगर (शीर्ष)
		list_add(&e->file_list, list);
	अन्यथा
		list_add_पुच्छ(&e->file_list, list);
	वापस 0;
}

#अगर defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)

अटल पूर्णांक filter(स्थिर काष्ठा dirent *d)
{
#अगर_घोषित _DIRENT_HAVE_D_TYPE
	अगर (d->d_type != DT_UNKNOWN && d->d_type != DT_REG &&
	    d->d_type != DT_LNK)
		वापस 0;
#पूर्ण_अगर
	अगर (is_dotdir_dirent(d))
		वापस 0;

	वापस 1;
}

अटल पूर्णांक read_dir(काष्ठा list_शीर्ष *file_list,
		    स्थिर अक्षर *project,
		    स्थिर अक्षर *मूल,
		    स्थिर अक्षर *confname,
		    स्थिर अक्षर *suffix)
{
	अक्षर *स_नाम = शून्य;
	अक्षर *filename = शून्य;
	पूर्णांक dd = -1, nfiles = 0, i;
	पूर्णांक ret = 0;
	काष्ठा dirent **namelist = शून्य;

	अगर (suffix)
		स_नाम = config_mk_path(S_IFDIR, "%s/%s/%s.%s.d",
				मूल, project, confname, suffix);
	अगर (!स_नाम)
		स_नाम = config_mk_path(S_IFDIR, "%s/%s/%s.d",
				मूल, project, confname);
	अगर (!स_नाम)
		वापस त्रुटि_सं == ENOMEM ? -ENOMEM : 0;

	dd = खोलो(स_नाम, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
	अगर (dd < 0)
		जाओ finish;

	nfiles = scandirat(dd, ".", &namelist, filter, alphasort);
	अगर (nfiles <= 0)
		जाओ finish;

	क्रम (i = 0; i < nfiles; i++) {
		काष्ठा dirent *d = namelist[i];

		अगर (suffix) {
			स्थिर अक्षर *p = ul_endswith(d->d_name, suffix);

			अगर (!p || p == d->d_name || *(p - 1) != '.')
				जारी;
		}

		अगर (asprintf(&filename, "%s/%s", स_नाम, d->d_name) < 0) {
			ret = -ENOMEM;
			अवरोध;
		}

		ret = configs_refer_filename(file_list, filename, 0);
		अगर (ret < 0) {
			मुक्त(filename);
			अवरोध;
		}
	}

finish:
	क्रम (i = 0; i < nfiles; i++)
		मुक्त(namelist[i]);
	मुक्त(namelist);
	मुक्त(स_नाम);
	अगर (dd >= 0)
		बंद(dd);
	वापस ret;
}

#पूर्ण_अगर /* HAVE_SCANDIRAT */

अटल पूर्णांक config_cmp(काष्ठा list_शीर्ष *a, काष्ठा list_शीर्ष *b,
		      व्योम *तथ्य __attribute__((__unused__)))
{
	काष्ठा file_element *ea = list_entry(a, काष्ठा file_element, file_list);
	काष्ठा file_element *eb = list_entry(b, काष्ठा file_element, file_list);
	अक्षर *na = ul_basename(ea->filename);
	अक्षर *nb = ul_basename(eb->filename);

	वापस strcoll(na, nb);
}

अटल व्योम free_list_entry(काष्ठा file_element *element)
{
	मुक्त(element->filename);
	मुक्त(element);
}

अटल पूर्णांक config_merge_list(काष्ठा list_शीर्ष *main_list,
			      काष्ठा list_शीर्ष *new_list)
{
	काष्ठा list_शीर्ष *n, *m, *अग्र;

	list_for_each_safe(n, अग्र, new_list) {
		काष्ठा file_element *ne = list_entry(n, काष्ठा file_element, file_list);
		अक्षर *nn = ul_basename(ne->filename);
		पूर्णांक duplicate = 0;

		list_for_each(m, main_list) {
			काष्ठा file_element *me = list_entry(m, काष्ठा file_element, file_list);
			अक्षर *mn = ul_basename(me->filename);

			अगर (strcoll(mn, nn) == 0) {
				duplicate = 1;
				अवरोध;
			}
		}

		अगर (!duplicate) {
			list_del_init(n);
			list_add_पुच्छ(n, main_list);
		}
	}

	list_sort(main_list, config_cmp, शून्य);
	वापस 0;
}

पूर्णांक ul_configs_file_list(काष्ठा list_शीर्ष *file_list,
			 स्थिर अक्षर *project,
			 स्थिर अक्षर *etcdir,
			 स्थिर अक्षर *rundir,
			 स्थिर अक्षर *usrdir,
			 स्थिर अक्षर *confname,
			 स्थिर अक्षर *suffix)
{
	अक्षर *main_file = शून्य;
	काष्ठा list_शीर्ष etc_list;
	काष्ठा list_शीर्ष run_list;
	काष्ठा list_शीर्ष usr_list;
	पूर्णांक ret;

	INIT_LIST_शीर्ष(file_list);

	अगर (!confname)
		वापस -ENOTEMPTY;

	/* Default is /etc */
	अगर (!etcdir)
		etcdir = _PATH_SYSCONFDIR;
	अगर (!rundir)
		rundir = "";
	अगर (!usrdir)
		usrdir = "";

	अगर (!project)
		project = "";

	/* Find "main" config file (but don't add to list yet) */
	/* Search order: /etc /run /usr */
	main_file = main_configs(etcdir, project, confname, suffix);
	अगर (main_file == शून्य)
		main_file = main_configs(rundir, project, confname, suffix);
	अगर (main_file == शून्य)
		main_file = main_configs(usrdir, project, confname, suffix);

	INIT_LIST_शीर्ष(&etc_list);
	INIT_LIST_शीर्ष(&run_list);
	INIT_LIST_शीर्ष(&usr_list);

#अगर defined(HAVE_SCANDIRAT) && defined(HAVE_OPENAT)
	ret = read_dir(&etc_list, project, etcdir, confname, suffix);
	अगर (ret == -ENOMEM)
		जाओ finish;

	ret = read_dir(&run_list, project, rundir, confname, suffix);
	अगर (ret == -ENOMEM)
		जाओ finish;

	ret = read_dir(&usr_list, project, usrdir, confname, suffix);
	अगर (ret == -ENOMEM)
		जाओ finish;
#पूर्ण_अगर

	/* Merge drop-in directories in priority order (high to low) */
	अगर ((ret = config_merge_list(file_list, &etc_list)) < 0 ||
	    (ret = config_merge_list(file_list, &run_list)) < 0 ||
	    (ret = config_merge_list(file_list, &usr_list)) < 0)
		जाओ finish;

	/* Add main config file at the beginning; the list is in parsing
	 * order, so the main file is parsed first and the drop-ins which
	 * follow it overwrite its settings. */
	अगर (main_file != शून्य) {
		ret = configs_refer_filename(file_list, main_file, 1);
		अगर (ret < 0) {
			मुक्त(main_file);
			जाओ finish;
		}
	}

finish:
	ul_configs_free_list(&etc_list);
	ul_configs_free_list(&run_list);
	ul_configs_free_list(&usr_list);

	वापस ret < 0 ? ret : (पूर्णांक) list_गिनती_entries(file_list);
}

व्योम ul_configs_free_list(काष्ठा list_शीर्ष *file_list)
{
	list_free(file_list, काष्ठा file_element,  file_list, free_list_entry);
}

पूर्णांक ul_configs_अग्र_filename(काष्ठा list_शीर्ष *file_list,
			     काष्ठा list_शीर्ष **current_entry,
			     अक्षर **name)
{
	काष्ठा file_element *element = शून्य;

	अगर (list_empty(file_list) || *current_entry == file_list)
		वापस 1;

	अगर (*current_entry == शून्य)
		*current_entry = file_list->अग्र;

	element = list_entry(*current_entry, काष्ठा file_element, file_list);
	*name = element->filename;
	*current_entry = (*current_entry)->अग्र;

	वापस 0;
}

#अगर_घोषित TEST_PROGRAM_CONFIGS
# include <getopt.h>

पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[])
{
	काष्ठा list_शीर्ष file_list;
	काष्ठा list_शीर्ष *current = शून्य;
	अक्षर *name = शून्य;
	स्थिर अक्षर *etc_path = शून्य;
	स्थिर अक्षर *run_path = शून्य;
	स्थिर अक्षर *usr_path = शून्य;
	स्थिर अक्षर *project = शून्य;
	स्थिर अक्षर *config_name = शून्य;
	स्थिर अक्षर *config_suffix = शून्य;
	अटल स्थिर काष्ठा option longopts[] = {
		{ "etc", required_argument, शून्य, 'e' },
		{ "run", required_argument, शून्य, 'r' },
		{ "usr", required_argument, शून्य, 'u' },
		{ "project", required_argument, शून्य, 'p' },
		{ "name", required_argument, शून्य, 'n' },
		{ "suffix", required_argument, शून्य, 's' },
		{ "help", no_argument, शून्य, 'h' },
		{ शून्य, 0, शून्य, 0 }
	};
	पूर्णांक ch, गिनती;

	जबतक ((ch = getopt_long(argc, argv, "e:r:u:p:n:s:h", longopts, शून्य)) != -1) {
		चयन (ch) {
		हाल 'e':
			etc_path = optarg;
			अवरोध;
		हाल 'r':
			run_path = optarg;
			अवरोध;
		हाल 'u':
			usr_path = optarg;
			अवरोध;
		हाल 'p':
			project = optarg;
			अवरोध;
		हाल 'n':
			config_name = optarg;
			अवरोध;
		हाल 's':
			config_suffix = optarg;
			अवरोध;
		हाल 'h':
			म_लिखो("usage: %s [options]\n"
				" -e, --etc <path>      path to /etc directory\n"
				" -r, --run <path>      path to /run directory\n"
				" -u, --usr <path>      path to /usr directory\n"
				" -p, --project <name>  project name subdirectory\n"
				" -n, --name <name>     config file base name\n"
				" -s, --suffix <suffix> config file suffix\n"
				" -h, --help            display this help\n",
				program_invocation_short_name);
			वापस निकास_सफल;
		शेष:
			वापस निकास_त्रुटि;
		}
	}

	अगर (!config_name) {
		ख_लिखो(मानक_त्रुटि, "config name is required (use --name)\n");
		वापस निकास_त्रुटि;
	}

	गिनती = ul_configs_file_list(&file_list, project, etc_path, run_path, usr_path,
				     config_name, config_suffix);
	अगर (गिनती < 0) {
		ख_लिखो(मानक_त्रुटि, "failed to get config file list: %d\n", गिनती);
		वापस निकास_त्रुटि;
	}

	म_लिखो("Found %d configuration file(s):\n", गिनती);
	जबतक (ul_configs_अग्र_filename(&file_list, &current, &name) == 0)
		म_लिखो("  %s\n", name);

	ul_configs_free_list(&file_list);
	वापस निकास_सफल;
}
#पूर्ण_अगर /* TEST_PROGRAM_CONFIGS */
