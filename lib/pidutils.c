/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <errno.h>

#অন্তর্ভাব "nls.h"
#অন্তর্ভাব "strutils.h"
#অন্তর্ভাব "pidutils.h"

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
পূর্ণাঙ্ক ul_parse_pid_str(বর্ণ *pidstr, pid_t *pid_অঙ্ক, uint64_t *pfd_ino, পূর্ণাঙ্ক flags)
{
	পূর্ণাঙ্ক rc;
	বর্ণ *end = রিক্ত;
	int64_t অঙ্ক = 0;

	যদি (!pidstr || !*pidstr || !pid_অঙ্ক)
		প্রত্যাগম -(errno = EINVAL);

	অঙ্ক = strtoimax(pidstr, &end, 10);
	যদি (অঙ্ক == 0 && end == pidstr)
		প্রত্যাগম -(errno = EINVAL);

	যদি (errno == ERANGE || অঙ্ক < SINT_MIN(pid_t) || অঙ্ক > SINT_MAX(pid_t))
		প্রত্যাগম -(errno = ERANGE);

	যদি (অঙ্ক == 0 && !(flags & UL_PID_ZERO))
		প্রত্যাগম -(errno = ERANGE);

	যদি (অঙ্ক < 0 && !(flags & UL_PID_NEGATIVE))
		প্রত্যাগম -(errno = ERANGE);

	*pid_অঙ্ক = (pid_t) অঙ্ক;

	যদি (*end == ':' && pfd_ino && অঙ্ক > 0) {
		rc = ul_strtou64(++end, pfd_ino, 10);
		যদি (rc < 0)
			প্রত্যাগম rc;

		যদি (*pfd_ino == 0)
			প্রত্যাগম -(errno = ERANGE);
		*end = '\0';
	}

	যদি (end && *end != '\0')
		প্রত্যাগম -(errno = EINVAL);
	প্রত্যাগম 0;
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
শূন্য ul_parse_pid_str_or_err(বর্ণ *pidstr, pid_t *pid_অঙ্ক, uint64_t *pfd_ino, পূর্ণাঙ্ক flags)
{
	যদি (ul_parse_pid_str(pidstr, pid_অঙ্ক, pfd_ino, flags) < 0) {
		err(EXIT_FAILURE, _("failed to parse PID argument '%s'"), pidstr);
	}
}


#ifdef TEST_PROGRAM_PARSEPID

#অন্তর্ভাব <getopt.h>

স্থির শূন্য __attribute__((__noreturn__)) usage(শূন্য)
{
	fprintf(stdout, " %s [options] <pidstr>\n\n", program_invocation_short_name);
	fputs(" -z, --zero         allow zero (0) as value for <pidstr>\n", stdout);
	fputs(" -n, --negative     allow negative number as value for <pidstr>\n", stdout);

	exit(EXIT_SUCCESS);
}

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ **argv)
{
	পূর্ণাঙ্ক c, flags = 0, rc = 0;
	uint64_t pidfd_ino = 0;
	pid_t pid_অঙ্ক = 0;
	বর্ণ *str = রিক্ত;

	স্থির ধ্রুব সংরচনা option longopts[] = {
		{ "zero",	0, রিক্ত, 'z' },
		{ "negative",	0, রিক্ত, 'n' },
		{ "help",       0, রিক্ত, 'h' },
		{ রিক্ত, 0, রিক্ত, 0 },
	};

	যাবত্((c = getopt_long(argc, argv, "nhz", longopts, রিক্ত)) != -1) {
		বিকল্প(c) {
		পক্ষ 'n':
			flags |= UL_PID_NEGATIVE;
			ভঙ্গ;
		পক্ষ 'z':
			flags |= UL_PID_ZERO;
			ভঙ্গ;
		পক্ষ 'h':
			usage();
			ভঙ্গ;
		স্বভাব:
			err(EXIT_FAILURE, "try --help");
		}
	}

	যদি (optind == argc)
		errx(EXIT_FAILURE, "missing <pidstr> argument");
	str = argv[optind];

	rc = ul_parse_pid_str(str, &pid_অঙ্ক, &pidfd_ino, flags);
	যদি (rc)
		err(EXIT_FAILURE, "failed to parse PID '%s'", str);

	printf("PID: %d\n", pid_অঙ্ক);
	যদি (pidfd_ino)
		printf("inode: %"PRIu64"\n", pidfd_ino);

	প্রত্যাগম EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_PARSE_PID */
