/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#समावेश <stdlib.h>
#समावेश <sys/stat.h>
#समावेश <errno.h>

#समावेश "nls.h"
#समावेश "strutils.h"
#समावेश "pidutils.h"

/*
 * ul_parse_pid_str() - Parse a string and store the found pid and pidfd inode.
 *
 * @pidstr:  	string in format `pid[:pidfd_inode]` that is to be parsed
 * @pid_num: 	stores pid number
 * @pfd_ino: 	stores pidfd inode number
 * @flags:	uncommon values that are accepted as PIDs
 * 		(e.g.: zero = UL_PID_ZERO, negative = UL_PID_NEGATIVE)
 *		the flag values can be ORed
 *
 * If @pfd_ino is not destined to be set, pass it as NULL.
 *
 * Return: On success, 0 is returned.
 *         On failure, a negative errno number is returned
 *         and errno is set to indicate the issue.
 */
पूर्णांक ul_parse_pid_str(अक्षर *pidstr, pid_t *pid_अंक, uint64_t *pfd_ino, पूर्णांक flags)
{
	पूर्णांक rc;
	अक्षर *end = शून्य;
	int64_t अंक = 0;

	अगर (!pidstr || !*pidstr || !pid_अंक)
		वापस -(त्रुटि_सं = EINVAL);

	अंक = strtoimax(pidstr, &end, 10);
	अगर (अंक == 0 && end == pidstr)
		वापस -(त्रुटि_सं = EINVAL);

	अगर (त्रुटि_सं == दुस्फल || अंक < SINT_MIN(pid_t) || अंक > SINT_MAX(pid_t))
		वापस -(त्रुटि_सं = दुस्फल);

	अगर (अंक == 0 && !(flags & UL_PID_ZERO))
		वापस -(त्रुटि_सं = दुस्फल);

	अगर (अंक < 0 && !(flags & UL_PID_NEGATIVE))
		वापस -(त्रुटि_सं = दुस्फल);

	*pid_अंक = (pid_t) अंक;

	अगर (*end == ':' && pfd_ino && अंक > 0) {
		rc = ul_strtou64(++end, pfd_ino, 10);
		अगर (rc < 0)
			वापस rc;

		अगर (*pfd_ino == 0)
			वापस -(त्रुटि_सं = दुस्फल);
		*end = '\0';
	}

	अगर (end && *end != '\0')
		वापस -(त्रुटि_सं = EINVAL);
	वापस 0;
}

/*
 * ul_parse_pid_str_or_err() - Parse a string and store the found pid
 *                             and pidfd inode, or exit on error.
 *
 * @pidstr:  string in format `pid[:pidfd_inode]` that is to be parsed
 * @pid_num: stores pid number
 * @pfd_ino: stores pidfd inode number
 * @flags:	uncommon values that are accepted as PIDs
 * 		(e.g.: zero = UL_PID_ZERO, negative = UL_PID_NEGATIVE)
 *		the flag values can be ORed
 *
 * If @pfd_ino is not destined to be set, pass it as NULL.
 *
 * On failure, err() is called with an error message to indicate the issue.
 */
व्योम ul_parse_pid_str_or_err(अक्षर *pidstr, pid_t *pid_अंक, uint64_t *pfd_ino, पूर्णांक flags)
{
	अगर (ul_parse_pid_str(pidstr, pid_अंक, pfd_ino, flags) < 0) {
		err(निकास_त्रुटि, _("failed to parse PID argument '%s'"), pidstr);
	}
}


#अगर_घोषित TEST_PROGRAM_PARSEPID

#समावेश <getopt.h>

अटल व्योम __attribute__((__noreturn__)) usage(व्योम)
{
	ख_लिखो(मानक_निकास, " %s [options] <pidstr>\n\n", program_invocation_short_name);
	ख_माला_दो(" -z, --zero         allow zero (0) as value for <pidstr>\n", मानक_निकास);
	ख_माला_दो(" -n, --negative     allow negative number as value for <pidstr>\n", मानक_निकास);

	निकास(निकास_सफल);
}

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	पूर्णांक c, flags = 0, rc = 0;
	uint64_t pidfd_ino = 0;
	pid_t pid_अंक = 0;
	अक्षर *str = शून्य;

	अटल स्थिर काष्ठा option longopts[] = {
		{ "zero",	0, शून्य, 'z' },
		{ "negative",	0, शून्य, 'n' },
		{ "help",       0, शून्य, 'h' },
		{ शून्य, 0, शून्य, 0 },
	};

	जबतक((c = getopt_long(argc, argv, "nhz", longopts, शून्य)) != -1) {
		चयन(c) {
		हाल 'n':
			flags |= UL_PID_NEGATIVE;
			अवरोध;
		हाल 'z':
			flags |= UL_PID_ZERO;
			अवरोध;
		हाल 'h':
			usage();
			अवरोध;
		शेष:
			err(निकास_त्रुटि, "try --help");
		}
	}

	अगर (optind == argc)
		errx(निकास_त्रुटि, "missing <pidstr> argument");
	str = argv[optind];

	rc = ul_parse_pid_str(str, &pid_अंक, &pidfd_ino, flags);
	अगर (rc)
		err(निकास_त्रुटि, "failed to parse PID '%s'", str);

	म_लिखो("PID: %d\n", pid_अंक);
	अगर (pidfd_ino)
		म_लिखो("inode: %"PRIu64"\n", pidfd_ino);

	वापस निकास_सफल;
}

#पूर्ण_अगर /* TEST_PROGRAM_PARSE_PID */
