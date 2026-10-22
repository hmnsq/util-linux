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
صحیح ul_parse_pid_str(حرف *pidstr, pid_t *pid_عدد, uint64_t *pfd_ino, صحیح flags)
{
	صحیح rc;
	حرف *انتہا = تہی;
	int64_t عدد = 0;

	اگر (!pidstr || !*pidstr || !pid_عدد)
		واپس -(errno = EINVAL);

	عدد = strtoimax(pidstr, &انتہا, 10);
	اگر (عدد == 0 && انتہا == pidstr)
		واپس -(errno = EINVAL);

	اگر (errno == ERANGE || عدد < SINT_MIN(pid_t) || عدد > SINT_MAX(pid_t))
		واپس -(errno = ERANGE);

	اگر (عدد == 0 && !(flags & UL_PID_ZERO))
		واپس -(errno = ERANGE);

	اگر (عدد < 0 && !(flags & UL_PID_NEGATIVE))
		واپس -(errno = ERANGE);

	*pid_عدد = (pid_t) عدد;

	اگر (*انتہا == ':' && pfd_ino && عدد > 0) {
		rc = ul_strtou64(++انتہا, pfd_ino, 10);
		اگر (rc < 0)
			واپس rc;

		اگر (*pfd_ino == 0)
			واپس -(errno = ERANGE);
		*انتہا = '\0';
	}

	اگر (انتہا && *انتہا != '\0')
		واپس -(errno = EINVAL);
	واپس 0;
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
خالی ul_parse_pid_str_or_err(حرف *pidstr, pid_t *pid_عدد, uint64_t *pfd_ino, صحیح flags)
{
	اگر (ul_parse_pid_str(pidstr, pid_عدد, pfd_ino, flags) < 0) {
		err(EXIT_FAILURE, _("failed to parse PID argument '%s'"), pidstr);
	}
}


#اگر_تعریف_شدہ TEST_پروگرام_PARSEPID

#شامل <getopt.h>

ساکن خالی __attribute__((__noreturn__)) usage(خالی)
{
	fprintf(stdout, " %s [options] <pidstr>\n\n", پروگرام_invocation_short_name);
	fputs(" -z, --zero         allow zero (0) as value for <pidstr>\n", stdout);
	fputs(" -n, --negative     allow negative number as value for <pidstr>\n", stdout);

	باہر_نکلو(EXIT_SUCCESS);
}

صحیح مین(صحیح argc, حرف **argv)
{
	صحیح c, flags = 0, rc = 0;
	uint64_t pidfd_ino = 0;
	pid_t pid_عدد = 0;
	حرف *str = تہی;

	ساکن ثابت ساخت option longopts[] = {
		{ "zero",	0, تہی, 'z' },
		{ "negative",	0, تہی, 'n' },
		{ "help",       0, تہی, 'h' },
		{ تہی, 0, تہی, 0 },
	};

	جب((c = getopt_long(argc, argv, "nhz", longopts, تہی)) != -1) {
		انتخاب(c) {
		حالت 'n':
			flags |= UL_PID_NEGATIVE;
			توڑو;
		حالت 'z':
			flags |= UL_PID_ZERO;
			توڑو;
		حالت 'h':
			usage();
			توڑو;
		طےشدہ:
			err(EXIT_FAILURE, "try --help");
		}
	}

	اگر (optind == argc)
		errx(EXIT_FAILURE, "missing <pidstr> argument");
	str = argv[optind];

	rc = ul_parse_pid_str(str, &pid_عدد, &pidfd_ino, flags);
	اگر (rc)
		err(EXIT_FAILURE, "failed to parse PID '%s'", str);

	لکھو("PID: %d\n", pid_عدد);
	اگر (pidfd_ino)
		لکھو("inode: %"PRIu64"\n", pidfd_ino);

	واپس EXIT_SUCCESS;
}

#اگر_ختم /* TEST_PROGRAM_PARSE_PID */
