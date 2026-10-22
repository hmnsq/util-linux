/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#include "nls.h"
#include "strutils.h"
#include "pidutils.h"

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
цяло ul_parse_pid_str(char *pidstr, pid_t *pid_num, uint64_t *pfd_ino, цяло flags)
{
	цяло rc;
	char *end = NULL;
	int64_t num = 0;

	ако (!pidstr || !*pidstr || !pid_num)
		върни -(errno = EINVAL);

	num = strtoimax(pidstr, &end, 10);
	ако (num == 0 && end == pidstr)
		върни -(errno = EINVAL);

	ако (errno == ERANGE || num < SINT_MIN(pid_t) || num > SINT_MAX(pid_t))
		върни -(errno = ERANGE);

	ако (num == 0 && !(flags & UL_PID_ZERO))
		върни -(errno = ERANGE);

	ако (num < 0 && !(flags & UL_PID_NEGATIVE))
		върни -(errno = ERANGE);

	*pid_num = (pid_t) num;

	ако (*end == ':' && pfd_ino && num > 0) {
		rc = ul_strtou64(++end, pfd_ino, 10);
		ако (rc < 0)
			върни rc;

		ако (*pfd_ino == 0)
			върни -(errno = ERANGE);
		*end = '\0';
	}

	ако (end && *end != '\0')
		върни -(errno = EINVAL);
	върни 0;
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
void ul_parse_pid_str_or_err(char *pidstr, pid_t *pid_num, uint64_t *pfd_ino, цяло flags)
{
	ако (ul_parse_pid_str(pidstr, pid_num, pfd_ino, flags) < 0) {
		err(EXIT_FAILURE, _("failed to parse PID argument '%s'"), pidstr);
	}
}


#ifdef TEST_PROGRAM_PARSEPID

#include <getopt.h>

static void __attribute__((__noreturn__)) usage(void)
{
	fprintf(stdout, " %s [options] <pidstr>\n\n", program_invocation_short_name);
	fputs(" -z, --zero         allow zero (0) as value for <pidstr>\n", stdout);
	fputs(" -n, --negative     allow negative number as value for <pidstr>\n", stdout);

	exit(EXIT_SUCCESS);
}

цяло main(цяло argc, char **argv)
{
	цяло c, flags = 0, rc = 0;
	uint64_t pidfd_ino = 0;
	pid_t pid_num = 0;
	char *str = NULL;

	static const struct option longopts[] = {
		{ "zero",	0, NULL, 'z' },
		{ "negative",	0, NULL, 'n' },
		{ "help",       0, NULL, 'h' },
		{ NULL, 0, NULL, 0 },
	};

	докато((c = getopt_long(argc, argv, "nhz", longopts, NULL)) != -1) {
		switch(c) {
		case 'n':
			flags |= UL_PID_NEGATIVE;
			break;
		case 'z':
			flags |= UL_PID_ZERO;
			break;
		case 'h':
			usage();
			break;
		default:
			err(EXIT_FAILURE, "try --help");
		}
	}

	ако (optind == argc)
		errx(EXIT_FAILURE, "missing <pidstr> argument");
	str = argv[optind];

	rc = ul_parse_pid_str(str, &pid_num, &pidfd_ino, flags);
	ако (rc)
		err(EXIT_FAILURE, "failed to parse PID '%s'", str);

	printf("PID: %d\n", pid_num);
	ако (pidfd_ino)
		printf("inode: %"PRIu64"\n", pidfd_ino);

	върни EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_PARSE_PID */
