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
পূর্ণসংখ্যা ul_parse_pid_str(অক্ষর *pidstr, pid_t *pid_অঙ্ক, uint64_t *pfd_ino, পূর্ণসংখ্যা flags)
{
	পূর্ণসংখ্যা rc;
	অক্ষর *end = ফাঁকা;
	int64_t অঙ্ক = 0;

	যদি (!pidstr || !*pidstr || !pid_অঙ্ক)
		ফেরত -(errno = EINVAL);

	অঙ্ক = strtoimax(pidstr, &end, 10);
	যদি (অঙ্ক == 0 && end == pidstr)
		ফেরত -(errno = EINVAL);

	যদি (errno == ERANGE || অঙ্ক < SINT_MIN(pid_t) || অঙ্ক > SINT_MAX(pid_t))
		ফেরত -(errno = ERANGE);

	যদি (অঙ্ক == 0 && !(flags & UL_PID_ZERO))
		ফেরত -(errno = ERANGE);

	যদি (অঙ্ক < 0 && !(flags & UL_PID_NEGATIVE))
		ফেরত -(errno = ERANGE);

	*pid_অঙ্ক = (pid_t) অঙ্ক;

	যদি (*end == ':' && pfd_ino && অঙ্ক > 0) {
		rc = ul_strtou64(++end, pfd_ino, 10);
		যদি (rc < 0)
			ফেরত rc;

		যদি (*pfd_ino == 0)
			ফেরত -(errno = ERANGE);
		*end = '\0';
	}

	যদি (end && *end != '\0')
		ফেরত -(errno = EINVAL);
	ফেরত 0;
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
শূন্য ul_parse_pid_str_or_err(অক্ষর *pidstr, pid_t *pid_অঙ্ক, uint64_t *pfd_ino, পূর্ণসংখ্যা flags)
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

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর **argv)
{
	পূর্ণসংখ্যা c, flags = 0, rc = 0;
	uint64_t pidfd_ino = 0;
	pid_t pid_অঙ্ক = 0;
	অক্ষর *str = ফাঁকা;

	স্থির ধ্রুবক কাঠামো option longopts[] = {
		{ "zero",	0, ফাঁকা, 'z' },
		{ "negative",	0, ফাঁকা, 'n' },
		{ "help",       0, ফাঁকা, 'h' },
		{ ফাঁকা, 0, ফাঁকা, 0 },
	};

	যখন((c = getopt_long(argc, argv, "nhz", longopts, ফাঁকা)) != -1) {
		নির্বাচন(c) {
		ক্ষেত্র 'n':
			flags |= UL_PID_NEGATIVE;
			বিরতি;
		ক্ষেত্র 'z':
			flags |= UL_PID_ZERO;
			বিরতি;
		ক্ষেত্র 'h':
			usage();
			বিরতি;
		পূর্বনির্ধারিত:
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

	ফেরত EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_PARSE_PID */
