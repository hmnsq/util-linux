/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#شامل <stdlib.h>
#شامل <sys/stat.h>
#شامل <errno.h>

#شامل "nls.h"
#شامل "strutils.h"
#شامل "pidutils.h"

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
صحیح ul_parse_pid_str(نویسه *pidstr, pid_t *pid_رقم, uint64_t *pfd_ino, صحیح flags)
{
	صحیح rc;
	نویسه *end = پوچ;
	int64_t رقم = 0;

	اگر (!pidstr || !*pidstr || !pid_رقم)
		بازگشت -(errno = EINVAL);

	رقم = strtoimax(pidstr, &end, 10);
	اگر (رقم == 0 && end == pidstr)
		بازگشت -(errno = EINVAL);

	اگر (errno == ERANGE || رقم < SINT_MIN(pid_t) || رقم > SINT_MAX(pid_t))
		بازگشت -(errno = ERANGE);

	اگر (رقم == 0 && !(flags & UL_PID_ZERO))
		بازگشت -(errno = ERANGE);

	اگر (رقم < 0 && !(flags & UL_PID_NEGATIVE))
		بازگشت -(errno = ERANGE);

	*pid_رقم = (pid_t) رقم;

	اگر (*end == ':' && pfd_ino && رقم > 0) {
		rc = ul_strtou64(++end, pfd_ino, 10);
		اگر (rc < 0)
			بازگشت rc;

		اگر (*pfd_ino == 0)
			بازگشت -(errno = ERANGE);
		*end = '\0';
	}

	اگر (end && *end != '\0')
		بازگشت -(errno = EINVAL);
	بازگشت 0;
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
تهی ul_parse_pid_str_or_err(نویسه *pidstr, pid_t *pid_رقم, uint64_t *pfd_ino, صحیح flags)
{
	اگر (ul_parse_pid_str(pidstr, pid_رقم, pfd_ino, flags) < 0) {
		err(EXIT_FAILURE, _("failed to parse PID argument '%s'"), pidstr);
	}
}


#ifdef TEST_PROGRAM_PARSEPID

#شامل <getopt.h>

ایستا تهی __attribute__((__noreturn__)) usage(تهی)
{
	fprintf(stdout, " %s [options] <pidstr>\n\n", program_invocation_short_name);
	fputs(" -z, --zero         allow zero (0) as value for <pidstr>\n", stdout);
	fputs(" -n, --negative     allow negative number as value for <pidstr>\n", stdout);

	exit(EXIT_SUCCESS);
}

صحیح اصلی(صحیح argc, نویسه **argv)
{
	صحیح c, flags = 0, rc = 0;
	uint64_t pidfd_ino = 0;
	pid_t pid_رقم = 0;
	نویسه *str = پوچ;

	ایستا ثابت ساختار option longopts[] = {
		{ "zero",	0, پوچ, 'z' },
		{ "negative",	0, پوچ, 'n' },
		{ "help",       0, پوچ, 'h' },
		{ پوچ, 0, پوچ, 0 },
	};

	وقتی((c = getopt_long(argc, argv, "nhz", longopts, پوچ)) != -1) {
		انتخاب(c) {
		حالت 'n':
			flags |= UL_PID_NEGATIVE;
			بشکن;
		حالت 'z':
			flags |= UL_PID_ZERO;
			بشکن;
		حالت 'h':
			usage();
			بشکن;
		پیشفرض:
			err(EXIT_FAILURE, "try --help");
		}
	}

	اگر (optind == argc)
		errx(EXIT_FAILURE, "missing <pidstr> argument");
	str = argv[optind];

	rc = ul_parse_pid_str(str, &pid_رقم, &pidfd_ino, flags);
	اگر (rc)
		err(EXIT_FAILURE, "failed to parse PID '%s'", str);

	printf("PID: %d\n", pid_رقم);
	اگر (pidfd_ino)
		printf("inode: %"PRIu64"\n", pidfd_ino);

	بازگشت EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_PARSE_PID */
