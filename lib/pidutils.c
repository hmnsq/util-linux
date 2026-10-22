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
පූර්ණ_සංඛ්‍යා ul_parse_pid_str(var_na *pidstr, pid_t *pid_num, uint64_t *pfd_ino, පූර්ණ_සංඛ්‍යා flags)
{
	පූර්ණ_සංඛ්‍යා rc;
	var_na *end = NULL;
	int64_t num = 0;

	නම් (!pidstr || !*pidstr || !pid_num)
		ආපසු -(errno = EINVAL);

	num = strtoimax(pidstr, &end, 10);
	නම් (num == 0 && end == pidstr)
		ආපසු -(errno = EINVAL);

	නම් (errno == ERANGE || num < SINT_MIN(pid_t) || num > SINT_MAX(pid_t))
		ආපසු -(errno = ERANGE);

	නම් (num == 0 && !(flags & UL_PID_ZERO))
		ආපසු -(errno = ERANGE);

	නම් (num < 0 && !(flags & UL_PID_NEGATIVE))
		ආපසු -(errno = ERANGE);

	*pid_num = (pid_t) num;

	නම් (*end == ':' && pfd_ino && num > 0) {
		rc = ul_strtou64(++end, pfd_ino, 10);
		නම් (rc < 0)
			ආපසු rc;

		නම් (*pfd_ino == 0)
			ආපසු -(errno = ERANGE);
		*end = '\0';
	}

	නම් (end && *end != '\0')
		ආපසු -(errno = EINVAL);
	ආපසු 0;
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
void ul_parse_pid_str_or_err(var_na *pidstr, pid_t *pid_num, uint64_t *pfd_ino, පූර්ණ_සංඛ්‍යා flags)
{
	නම් (ul_parse_pid_str(pidstr, pid_num, pfd_ino, flags) < 0) {
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

පූර්ණ_සංඛ්‍යා main(පූර්ණ_සංඛ්‍යා argc, var_na **argv)
{
	පූර්ණ_සංඛ්‍යා c, flags = 0, rc = 0;
	uint64_t pidfd_ino = 0;
	pid_t pid_num = 0;
	var_na *str = NULL;

	static const sa_nracanaa option longopts[] = {
		{ "zero",	0, NULL, 'z' },
		{ "negative",	0, NULL, 'n' },
		{ "help",       0, NULL, 'h' },
		{ NULL, 0, NULL, 0 },
	};

	අතරතුර((c = getopt_long(argc, argv, "nhz", longopts, NULL)) != -1) {
		switch(c) {
		pak_sa 'n':
			flags |= UL_PID_NEGATIVE;
			bha_nga;
		pak_sa 'z':
			flags |= UL_PID_ZERO;
			bha_nga;
		pak_sa 'h':
			usage();
			bha_nga;
		default:
			err(EXIT_FAILURE, "try --help");
		}
	}

	නම් (optind == argc)
		errx(EXIT_FAILURE, "missing <pidstr> argument");
	str = argv[optind];

	rc = ul_parse_pid_str(str, &pid_num, &pidfd_ino, flags);
	නම් (rc)
		err(EXIT_FAILURE, "failed to parse PID '%s'", str);

	printf("PID: %d\n", pid_num);
	නම් (pidfd_ino)
		printf("inode: %"PRIu64"\n", pidfd_ino);

	ආපසු EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_PARSE_PID */
