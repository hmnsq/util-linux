/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <errno.h>

#அந்தர்பாவ "nls.h"
#அந்தர்பாவ "strutils.h"
#அந்தர்பாவ "pidutils.h"

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
முழு ul_parse_pid_str(எழுத்து *pidstr, pid_t *pid_இலக்கம், uint64_t *pfd_ino, முழு flags)
{
	முழு rc;
	எழுத்து *end = வெற்று;
	int64_t இலக்கம் = 0;

	எனில் (!pidstr || !*pidstr || !pid_இலக்கம்)
		திரும்பு -(errno = EINVAL);

	இலக்கம் = strtoimax(pidstr, &end, 10);
	எனில் (இலக்கம் == 0 && end == pidstr)
		திரும்பு -(errno = EINVAL);

	எனில் (errno == ERANGE || இலக்கம் < SINT_MIN(pid_t) || இலக்கம் > SINT_MAX(pid_t))
		திரும்பு -(errno = ERANGE);

	எனில் (இலக்கம் == 0 && !(flags & UL_PID_ZERO))
		திரும்பு -(errno = ERANGE);

	எனில் (இலக்கம் < 0 && !(flags & UL_PID_NEGATIVE))
		திரும்பு -(errno = ERANGE);

	*pid_இலக்கம் = (pid_t) இலக்கம்;

	எனில் (*end == ':' && pfd_ino && இலக்கம் > 0) {
		rc = ul_strtou64(++end, pfd_ino, 10);
		எனில் (rc < 0)
			திரும்பு rc;

		எனில் (*pfd_ino == 0)
			திரும்பு -(errno = ERANGE);
		*end = '\0';
	}

	எனில் (end && *end != '\0')
		திரும்பு -(errno = EINVAL);
	திரும்பு 0;
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
வெறுமை ul_parse_pid_str_or_err(எழுத்து *pidstr, pid_t *pid_இலக்கம், uint64_t *pfd_ino, முழு flags)
{
	எனில் (ul_parse_pid_str(pidstr, pid_இலக்கம், pfd_ino, flags) < 0) {
		err(EXIT_FAILURE, _("failed to parse PID argument '%s'"), pidstr);
	}
}


#ifdef TEST_PROGRAM_PARSEPID

#அந்தர்பாவ <getopt.h>

நிலையான வெறுமை __attribute__((__noreturn__)) usage(வெறுமை)
{
	fprintf(stdout, " %s [options] <pidstr>\n\n", program_invocation_short_name);
	fputs(" -z, --zero         allow zero (0) as value for <pidstr>\n", stdout);
	fputs(" -n, --negative     allow negative number as value for <pidstr>\n", stdout);

	exit(EXIT_SUCCESS);
}

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	முழு c, flags = 0, rc = 0;
	uint64_t pidfd_ino = 0;
	pid_t pid_இலக்கம் = 0;
	எழுத்து *str = வெற்று;

	நிலையான மாறிலி கட்டமைப்பு option longopts[] = {
		{ "zero",	0, வெற்று, 'z' },
		{ "negative",	0, வெற்று, 'n' },
		{ "help",       0, வெற்று, 'h' },
		{ வெற்று, 0, வெற்று, 0 },
	};

	போது((c = getopt_long(argc, argv, "nhz", longopts, வெற்று)) != -1) {
		தேர்வு(c) {
		நிலை 'n':
			flags |= UL_PID_NEGATIVE;
			நிறுத்து;
		நிலை 'z':
			flags |= UL_PID_ZERO;
			நிறுத்து;
		நிலை 'h':
			usage();
			நிறுத்து;
		இயல்பு:
			err(EXIT_FAILURE, "try --help");
		}
	}

	எனில் (optind == argc)
		errx(EXIT_FAILURE, "missing <pidstr> argument");
	str = argv[optind];

	rc = ul_parse_pid_str(str, &pid_இலக்கம், &pidfd_ino, flags);
	எனில் (rc)
		err(EXIT_FAILURE, "failed to parse PID '%s'", str);

	printf("PID: %d\n", pid_இலக்கம்);
	எனில் (pidfd_ino)
		printf("inode: %"PRIu64"\n", pidfd_ino);

	திரும்பு EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_PARSE_PID */
