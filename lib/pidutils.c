/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <errno.h>

#અન્તર્ભાવ "nls.h"
#અન્તર્ભાવ "strutils.h"
#અન્તર્ભાવ "pidutils.h"

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
પૂર્ણાંક ul_parse_pid_str(અક્ષર *pidstr, pid_t *pid_અંક, uint64_t *pfd_ino, પૂર્ણાંક flags)
{
	પૂર્ણાંક rc;
	અક્ષર *end = ખાલી;
	int64_t અંક = 0;

	જો (!pidstr || !*pidstr || !pid_અંક)
		પાછા -(errno = EINVAL);

	અંક = strtoimax(pidstr, &end, 10);
	જો (અંક == 0 && end == pidstr)
		પાછા -(errno = EINVAL);

	જો (errno == ERANGE || અંક < SINT_MIN(pid_t) || અંક > SINT_MAX(pid_t))
		પાછા -(errno = ERANGE);

	જો (અંક == 0 && !(flags & UL_PID_ZERO))
		પાછા -(errno = ERANGE);

	જો (અંક < 0 && !(flags & UL_PID_NEGATIVE))
		પાછા -(errno = ERANGE);

	*pid_અંક = (pid_t) અંક;

	જો (*end == ':' && pfd_ino && અંક > 0) {
		rc = ul_strtou64(++end, pfd_ino, 10);
		જો (rc < 0)
			પાછા rc;

		જો (*pfd_ino == 0)
			પાછા -(errno = ERANGE);
		*end = '\0';
	}

	જો (end && *end != '\0')
		પાછા -(errno = EINVAL);
	પાછા 0;
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
શૂન્ય ul_parse_pid_str_or_err(અક્ષર *pidstr, pid_t *pid_અંક, uint64_t *pfd_ino, પૂર્ણાંક flags)
{
	જો (ul_parse_pid_str(pidstr, pid_અંક, pfd_ino, flags) < 0) {
		err(EXIT_FAILURE, _("failed to parse PID argument '%s'"), pidstr);
	}
}


#ifdef TEST_PROGRAM_PARSEPID

#અન્તર્ભાવ <getopt.h>

સ્થિર શૂન્ય __attribute__((__noreturn__)) usage(શૂન્ય)
{
	fprintf(stdout, " %s [options] <pidstr>\n\n", program_invocation_short_name);
	fputs(" -z, --zero         allow zero (0) as value for <pidstr>\n", stdout);
	fputs(" -n, --negative     allow negative number as value for <pidstr>\n", stdout);

	exit(EXIT_SUCCESS);
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	પૂર્ણાંક c, flags = 0, rc = 0;
	uint64_t pidfd_ino = 0;
	pid_t pid_અંક = 0;
	અક્ષર *str = ખાલી;

	સ્થિર અચલ સંરચના option longopts[] = {
		{ "zero",	0, ખાલી, 'z' },
		{ "negative",	0, ખાલી, 'n' },
		{ "help",       0, ખાલી, 'h' },
		{ ખાલી, 0, ખાલી, 0 },
	};

	જ્યારે((c = getopt_long(argc, argv, "nhz", longopts, ખાલી)) != -1) {
		પસંદગી(c) {
		સ્થિતિ 'n':
			flags |= UL_PID_NEGATIVE;
			તોડો;
		સ્થિતિ 'z':
			flags |= UL_PID_ZERO;
			તોડો;
		સ્થિતિ 'h':
			usage();
			તોડો;
		મૂળભૂત:
			err(EXIT_FAILURE, "try --help");
		}
	}

	જો (optind == argc)
		errx(EXIT_FAILURE, "missing <pidstr> argument");
	str = argv[optind];

	rc = ul_parse_pid_str(str, &pid_અંક, &pidfd_ino, flags);
	જો (rc)
		err(EXIT_FAILURE, "failed to parse PID '%s'", str);

	printf("PID: %d\n", pid_અંક);
	જો (pidfd_ino)
		printf("inode: %"PRIu64"\n", pidfd_ino);

	પાછા EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_PARSE_PID */
