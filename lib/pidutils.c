/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>

#ଅନ୍ତର୍ଭାଵ "nls.h"
#ଅନ୍ତର୍ଭାଵ "strutils.h"
#ଅନ୍ତର୍ଭାଵ "pidutils.h"

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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_parse_pid_str(ଅକ୍ଷର *pidstr, pid_t *pid_ଅଙ୍କ, uint64_t *pfd_ino, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	ଅକ୍ଷର *end = ଖାଲି;
	int64_t ଅଙ୍କ = 0;

	ଯଦି (!pidstr || !*pidstr || !pid_ଅଙ୍କ)
		ଫେରନ୍ତୁ -(errno = EINVAL);

	ଅଙ୍କ = strtoimax(pidstr, &end, 10);
	ଯଦି (ଅଙ୍କ == 0 && end == pidstr)
		ଫେରନ୍ତୁ -(errno = EINVAL);

	ଯଦି (errno == ERANGE || ଅଙ୍କ < SINT_MIN(pid_t) || ଅଙ୍କ > SINT_MAX(pid_t))
		ଫେରନ୍ତୁ -(errno = ERANGE);

	ଯଦି (ଅଙ୍କ == 0 && !(flags & UL_PID_ZERO))
		ଫେରନ୍ତୁ -(errno = ERANGE);

	ଯଦି (ଅଙ୍କ < 0 && !(flags & UL_PID_NEGATIVE))
		ଫେରନ୍ତୁ -(errno = ERANGE);

	*pid_ଅଙ୍କ = (pid_t) ଅଙ୍କ;

	ଯଦି (*end == ':' && pfd_ino && ଅଙ୍କ > 0) {
		rc = ul_strtou64(++end, pfd_ino, 10);
		ଯଦି (rc < 0)
			ଫେରନ୍ତୁ rc;

		ଯଦି (*pfd_ino == 0)
			ଫେରନ୍ତୁ -(errno = ERANGE);
		*end = '\0';
	}

	ଯଦି (end && *end != '\0')
		ଫେରନ୍ତୁ -(errno = EINVAL);
	ଫେରନ୍ତୁ 0;
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
ଶୂନ୍ୟ ul_parse_pid_str_or_err(ଅକ୍ଷର *pidstr, pid_t *pid_ଅଙ୍କ, uint64_t *pfd_ino, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	ଯଦି (ul_parse_pid_str(pidstr, pid_ଅଙ୍କ, pfd_ino, flags) < 0) {
		err(EXIT_FAILURE, _("failed to parse PID argument '%s'"), pidstr);
	}
}


#ifdef TEST_PROGRAM_PARSEPID

#ଅନ୍ତର୍ଭାଵ <getopt.h>

ସ୍ଥିର ଶୂନ୍ୟ __attribute__((__noreturn__)) usage(ଶୂନ୍ୟ)
{
	fprintf(stdout, " %s [options] <pidstr>\n\n", program_invocation_short_name);
	fputs(" -z, --zero         allow zero (0) as value for <pidstr>\n", stdout);
	fputs(" -n, --negative     allow negative number as value for <pidstr>\n", stdout);

	exit(EXIT_SUCCESS);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା c, flags = 0, rc = 0;
	uint64_t pidfd_ino = 0;
	pid_t pid_ଅଙ୍କ = 0;
	ଅକ୍ଷର *str = ଖାଲି;

	ସ୍ଥିର ଅଚଳ ସଂରଚନା option longopts[] = {
		{ "zero",	0, ଖାଲି, 'z' },
		{ "negative",	0, ଖାଲି, 'n' },
		{ "help",       0, ଖାଲି, 'h' },
		{ ଖାଲି, 0, ଖାଲି, 0 },
	};

	ଯେତେବେଳେ((c = getopt_long(argc, argv, "nhz", longopts, ଖାଲି)) != -1) {
		ବାଛ(c) {
		ସ୍ଥିତି 'n':
			flags |= UL_PID_NEGATIVE;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'z':
			flags |= UL_PID_ZERO;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'h':
			usage();
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			err(EXIT_FAILURE, "try --help");
		}
	}

	ଯଦି (optind == argc)
		errx(EXIT_FAILURE, "missing <pidstr> argument");
	str = argv[optind];

	rc = ul_parse_pid_str(str, &pid_ଅଙ୍କ, &pidfd_ino, flags);
	ଯଦି (rc)
		err(EXIT_FAILURE, "failed to parse PID '%s'", str);

	printf("PID: %d\n", pid_ଅଙ୍କ);
	ଯଦି (pidfd_ino)
		printf("inode: %"PRIu64"\n", pidfd_ino);

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_PARSE_PID */
