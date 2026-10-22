/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <errno.h>

#अन्तर्भाव "nls.h"
#अन्तर्भाव "strutils.h"
#अन्तर्भाव "pidutils.h"

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
पूर्णाङ्क ul_parse_pid_str(वर्ण *pidstr, pid_t *pid_अङ्कः, uint64_t *pfd_ino, पूर्णाङ्क flags)
{
	पूर्णाङ्क rc;
	वर्ण *end = रिक्त;
	int64_t अङ्कः = 0;

	सचे (!pidstr || !*pidstr || !pid_अङ्कः)
		पटिनिवत्त -(errno = EINVAL);

	अङ्कः = strtoimax(pidstr, &end, 10);
	सचे (अङ्कः == 0 && end == pidstr)
		पटिनिवत्त -(errno = EINVAL);

	सचे (errno == ERANGE || अङ्कः < SINT_MIN(pid_t) || अङ्कः > SINT_MAX(pid_t))
		पटिनिवत्त -(errno = ERANGE);

	सचे (अङ्कः == 0 && !(flags & UL_PID_ZERO))
		पटिनिवत्त -(errno = ERANGE);

	सचे (अङ्कः < 0 && !(flags & UL_PID_NEGATIVE))
		पटिनिवत्त -(errno = ERANGE);

	*pid_अङ्कः = (pid_t) अङ्कः;

	सचे (*end == ':' && pfd_ino && अङ्कः > 0) {
		rc = ul_strtou64(++end, pfd_ino, 10);
		सचे (rc < 0)
			पटिनिवत्त rc;

		सचे (*pfd_ino == 0)
			पटिनिवत्त -(errno = ERANGE);
		*end = '\0';
	}

	सचे (end && *end != '\0')
		पटिनिवत्त -(errno = EINVAL);
	पटिनिवत्त 0;
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
शून्य ul_parse_pid_str_or_err(वर्ण *pidstr, pid_t *pid_अङ्कः, uint64_t *pfd_ino, पूर्णाङ्क flags)
{
	सचे (ul_parse_pid_str(pidstr, pid_अङ्कः, pfd_ino, flags) < 0) {
		err(EXIT_FAILURE, _("failed to parse PID argument '%s'"), pidstr);
	}
}


#ifdef TEST_PROGRAM_PARSEPID

#अन्तर्भाव <getopt.h>

स्थिर शून्य __attribute__((__noreturn__)) usage(शून्य)
{
	fprintf(stdout, " %s [options] <pidstr>\n\n", program_invocation_short_name);
	fputs(" -z, --zero         allow zero (0) as value for <pidstr>\n", stdout);
	fputs(" -n, --negative     allow negative number as value for <pidstr>\n", stdout);

	exit(EXIT_SUCCESS);
}

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	पूर्णाङ्क c, flags = 0, rc = 0;
	uint64_t pidfd_ino = 0;
	pid_t pid_अङ्कः = 0;
	वर्ण *str = रिक्त;

	स्थिर ध्रुव संरचना option longopts[] = {
		{ "zero",	0, रिक्त, 'z' },
		{ "negative",	0, रिक्त, 'n' },
		{ "help",       0, रिक्त, 'h' },
		{ रिक्त, 0, रिक्त, 0 },
	};

	याव((c = getopt_long(argc, argv, "nhz", longopts, रिक्त)) != -1) {
		विकल्प(c) {
		पक्ष 'n':
			flags |= UL_PID_NEGATIVE;
			भङ्ग;
		पक्ष 'z':
			flags |= UL_PID_ZERO;
			भङ्ग;
		पक्ष 'h':
			usage();
			भङ्ग;
		स्वभाव:
			err(EXIT_FAILURE, "try --help");
		}
	}

	सचे (optind == argc)
		errx(EXIT_FAILURE, "missing <pidstr> argument");
	str = argv[optind];

	rc = ul_parse_pid_str(str, &pid_अङ्कः, &pidfd_ino, flags);
	सचे (rc)
		err(EXIT_FAILURE, "failed to parse PID '%s'", str);

	printf("PID: %d\n", pid_अङ्कः);
	सचे (pidfd_ino)
		printf("inode: %"PRIu64"\n", pidfd_ino);

	पटिनिवत्त EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_PARSE_PID */
