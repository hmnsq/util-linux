/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#ضمن <stdlib.h>
#ضمن <sys/stat.h>
#ضمن <errno.h>

#ضمن "nls.h"
#ضمن "strutils.h"
#ضمن "pidutils.h"

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
صحيح ul_parse_pid_str(حرف *pidstr, pid_t *pid_رقم, uint64_t *pfd_ino, صحيح flags)
{
	صحيح rc;
	حرف *end = عدم;
	int64_t رقم = 0;

	إذا (!pidstr || !*pidstr || !pid_رقم)
		أرجع -(errno = EINVAL);

	رقم = strtoimax(pidstr, &end, 10);
	إذا (رقم == 0 && end == pidstr)
		أرجع -(errno = EINVAL);

	إذا (errno == ERANGE || رقم < SINT_MIN(pid_t) || رقم > SINT_MAX(pid_t))
		أرجع -(errno = ERANGE);

	إذا (رقم == 0 && !(flags & UL_PID_ZERO))
		أرجع -(errno = ERANGE);

	إذا (رقم < 0 && !(flags & UL_PID_NEGATIVE))
		أرجع -(errno = ERANGE);

	*pid_رقم = (pid_t) رقم;

	إذا (*end == ':' && pfd_ino && رقم > 0) {
		rc = ul_strtou64(++end, pfd_ino, 10);
		إذا (rc < 0)
			أرجع rc;

		إذا (*pfd_ino == 0)
			أرجع -(errno = ERANGE);
		*end = '\0';
	}

	إذا (end && *end != '\0')
		أرجع -(errno = EINVAL);
	أرجع 0;
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
فراغ ul_parse_pid_str_or_err(حرف *pidstr, pid_t *pid_رقم, uint64_t *pfd_ino, صحيح flags)
{
	إذا (ul_parse_pid_str(pidstr, pid_رقم, pfd_ino, flags) < 0) {
		err(EXIT_FAILURE, _("failed to parse PID argument '%s'"), pidstr);
	}
}


#ifdef TEST_PROGRAM_PARSEPID

#ضمن <getopt.h>

ساكن فراغ __attribute__((__noreturn__)) usage(فراغ)
{
	fprintf(stdout, " %s [options] <pidstr>\n\n", program_invocation_short_name);
	fputs(" -z, --zero         allow zero (0) as value for <pidstr>\n", stdout);
	fputs(" -n, --negative     allow negative number as value for <pidstr>\n", stdout);

	exit(EXIT_SUCCESS);
}

صحيح main(صحيح argc, حرف **argv)
{
	صحيح c, flags = 0, rc = 0;
	uint64_t pidfd_ino = 0;
	pid_t pid_رقم = 0;
	حرف *str = عدم;

	ساكن ثابت بنية option longopts[] = {
		{ "zero",	0, عدم, 'z' },
		{ "negative",	0, عدم, 'n' },
		{ "help",       0, عدم, 'h' },
		{ عدم, 0, عدم, 0 },
	};

	طالما((c = getopt_long(argc, argv, "nhz", longopts, عدم)) != -1) {
		بدل(c) {
		حالة 'n':
			flags |= UL_PID_NEGATIVE;
			اكسر;
		حالة 'z':
			flags |= UL_PID_ZERO;
			اكسر;
		حالة 'h':
			usage();
			اكسر;
		افتراضي:
			err(EXIT_FAILURE, "try --help");
		}
	}

	إذا (optind == argc)
		errx(EXIT_FAILURE, "missing <pidstr> argument");
	str = argv[optind];

	rc = ul_parse_pid_str(str, &pid_رقم, &pidfd_ino, flags);
	إذا (rc)
		err(EXIT_FAILURE, "failed to parse PID '%s'", str);

	printf("PID: %d\n", pid_رقم);
	إذا (pidfd_ino)
		printf("inode: %"PRIu64"\n", pidfd_ino);

	أرجع EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_PARSE_PID */
