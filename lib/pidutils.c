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
ціле ul_parse_pid_str(char *pidstr, pid_t *pid_num, uint64_t *pfd_ino, ціле flags)
{
	ціле rc;
	char *end = NULL;
	int64_t num = 0;

	якщо (!pidstr || !*pidstr || !pid_num)
		повернути -(errno = EINVAL);

	num = strtoimax(pidstr, &end, 10);
	якщо (num == 0 && end == pidstr)
		повернути -(errno = EINVAL);

	якщо (errno == ERANGE || num < SINT_MIN(pid_t) || num > SINT_MAX(pid_t))
		повернути -(errno = ERANGE);

	якщо (num == 0 && !(flags & UL_PID_ZERO))
		повернути -(errno = ERANGE);

	якщо (num < 0 && !(flags & UL_PID_NEGATIVE))
		повернути -(errno = ERANGE);

	*pid_num = (pid_t) num;

	якщо (*end == ':' && pfd_ino && num > 0) {
		rc = ul_strtou64(++end, pfd_ino, 10);
		якщо (rc < 0)
			повернути rc;

		якщо (*pfd_ino == 0)
			повернути -(errno = ERANGE);
		*end = '\0';
	}

	якщо (end && *end != '\0')
		повернути -(errno = EINVAL);
	повернути 0;
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
void ul_parse_pid_str_or_err(char *pidstr, pid_t *pid_num, uint64_t *pfd_ino, ціле flags)
{
	якщо (ul_parse_pid_str(pidstr, pid_num, pfd_ino, flags) < 0) {
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

ціле main(ціле argc, char **argv)
{
	ціле c, flags = 0, rc = 0;
	uint64_t pidfd_ino = 0;
	pid_t pid_num = 0;
	char *str = NULL;

	static const struct option longopts[] = {
		{ "zero",	0, NULL, 'z' },
		{ "negative",	0, NULL, 'n' },
		{ "help",       0, NULL, 'h' },
		{ NULL, 0, NULL, 0 },
	};

	поки((c = getopt_long(argc, argv, "nhz", longopts, NULL)) != -1) {
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

	якщо (optind == argc)
		errx(EXIT_FAILURE, "missing <pidstr> argument");
	str = argv[optind];

	rc = ul_parse_pid_str(str, &pid_num, &pidfd_ino, flags);
	якщо (rc)
		err(EXIT_FAILURE, "failed to parse PID '%s'", str);

	printf("PID: %d\n", pid_num);
	якщо (pidfd_ino)
		printf("inode: %"PRIu64"\n", pidfd_ino);

	повернути EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_PARSE_PID */
