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
पूर्णाङ्क ul_parse_pid_str(वर्ण *pidstr, pid_t *pid_अंक, uint64_t *pfd_ino, पूर्णाङ्क flags)
{
	पूर्णाङ्क rc;
	वर्ण *end = रिक्त;
	int64_t अंक = 0;

	यदि (!pidstr || !*pidstr || !pid_अंक)
		प्रत्यागम -(त्रुटि_सं = EINVAL);

	अंक = strtoimax(pidstr, &end, 10);
	यदि (अंक == 0 && end == pidstr)
		प्रत्यागम -(त्रुटि_सं = EINVAL);

	यदि (त्रुटि_सं == दुस्फल || अंक < SINT_MIN(pid_t) || अंक > SINT_MAX(pid_t))
		प्रत्यागम -(त्रुटि_सं = दुस्फल);

	यदि (अंक == 0 && !(flags & UL_PID_ZERO))
		प्रत्यागम -(त्रुटि_सं = दुस्फल);

	यदि (अंक < 0 && !(flags & UL_PID_NEGATIVE))
		प्रत्यागम -(त्रुटि_सं = दुस्फल);

	*pid_अंक = (pid_t) अंक;

	यदि (*end == ':' && pfd_ino && अंक > 0) {
		rc = ul_strtou64(++end, pfd_ino, 10);
		यदि (rc < 0)
			प्रत्यागम rc;

		यदि (*pfd_ino == 0)
			प्रत्यागम -(त्रुटि_सं = दुस्फल);
		*end = '\0';
	}

	यदि (end && *end != '\0')
		प्रत्यागम -(त्रुटि_सं = EINVAL);
	प्रत्यागम 0;
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
शून्य ul_parse_pid_str_or_err(वर्ण *pidstr, pid_t *pid_अंक, uint64_t *pfd_ino, पूर्णाङ्क flags)
{
	यदि (ul_parse_pid_str(pidstr, pid_अंक, pfd_ino, flags) < 0) {
		err(निकास_त्रुटि, _("failed to parse PID argument '%s'"), pidstr);
	}
}


#ifdef TEST_PROGRAM_PARSEPID

#अन्तर्भाव <getopt.h>

स्थिर शून्य __attribute__((__noreturn__)) usage(शून्य)
{
	ख_लिखो(मानक_निकास, " %s [options] <pidstr>\n\n", program_invocation_short_name);
	ख_माला_दो(" -z, --zero         allow zero (0) as value for <pidstr>\n", मानक_निकास);
	ख_माला_दो(" -n, --negative     allow negative number as value for <pidstr>\n", मानक_निकास);

	निकास(निकास_सफल);
}

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	पूर्णाङ्क c, flags = 0, rc = 0;
	uint64_t pidfd_ino = 0;
	pid_t pid_अंक = 0;
	वर्ण *str = रिक्त;

	स्थिर ध्रुव संरचना option longopts[] = {
		{ "zero",	0, रिक्त, 'z' },
		{ "negative",	0, रिक्त, 'n' },
		{ "help",       0, रिक्त, 'h' },
		{ रिक्त, 0, रिक्त, 0 },
	};

	यावत्((c = getopt_long(argc, argv, "nhz", longopts, रिक्त)) != -1) {
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
			err(निकास_त्रुटि, "try --help");
		}
	}

	यदि (optind == argc)
		errx(निकास_त्रुटि, "missing <pidstr> argument");
	str = argv[optind];

	rc = ul_parse_pid_str(str, &pid_अंक, &pidfd_ino, flags);
	यदि (rc)
		err(निकास_त्रुटि, "failed to parse PID '%s'", str);

	म_लिखो("PID: %d\n", pid_अंक);
	यदि (pidfd_ino)
		म_लिखो("inode: %"PRIu64"\n", pidfd_ino);

	प्रत्यागम निकास_सफल;
}

#endif /* TEST_PROGRAM_PARSE_PID */
