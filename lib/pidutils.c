/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <errno.h>

#ಅನ್ತರ್ಭಾವ "nls.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"
#ಅನ್ತರ್ಭಾವ "pidutils.h"

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
ಪೂರ್ಣಾಙ್ಕ ul_parse_pid_str(ವರ್ಣ *pidstr, pid_t *pid_ಅಂಕೆ, uint64_t *pfd_ino, ಪೂರ್ಣಾಙ್ಕ flags)
{
	ಪೂರ್ಣಾಙ್ಕ rc;
	ವರ್ಣ *end = ರಿಕ್ತ;
	int64_t ಅಂಕೆ = 0;

	ಯದಿ (!pidstr || !*pidstr || !pid_ಅಂಕೆ)
		ಪ್ರತ್ಯಾಗಮ -(errno = EINVAL);

	ಅಂಕೆ = strtoimax(pidstr, &end, 10);
	ಯದಿ (ಅಂಕೆ == 0 && end == pidstr)
		ಪ್ರತ್ಯಾಗಮ -(errno = EINVAL);

	ಯದಿ (errno == ERANGE || ಅಂಕೆ < SINT_MIN(pid_t) || ಅಂಕೆ > SINT_MAX(pid_t))
		ಪ್ರತ್ಯಾಗಮ -(errno = ERANGE);

	ಯದಿ (ಅಂಕೆ == 0 && !(flags & UL_PID_ZERO))
		ಪ್ರತ್ಯಾಗಮ -(errno = ERANGE);

	ಯದಿ (ಅಂಕೆ < 0 && !(flags & UL_PID_NEGATIVE))
		ಪ್ರತ್ಯಾಗಮ -(errno = ERANGE);

	*pid_ಅಂಕೆ = (pid_t) ಅಂಕೆ;

	ಯದಿ (*end == ':' && pfd_ino && ಅಂಕೆ > 0) {
		rc = ul_strtou64(++end, pfd_ino, 10);
		ಯದಿ (rc < 0)
			ಪ್ರತ್ಯಾಗಮ rc;

		ಯದಿ (*pfd_ino == 0)
			ಪ್ರತ್ಯಾಗಮ -(errno = ERANGE);
		*end = '\0';
	}

	ಯದಿ (end && *end != '\0')
		ಪ್ರತ್ಯಾಗಮ -(errno = EINVAL);
	ಪ್ರತ್ಯಾಗಮ 0;
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
ಶೂನ್ಯ ul_parse_pid_str_or_err(ವರ್ಣ *pidstr, pid_t *pid_ಅಂಕೆ, uint64_t *pfd_ino, ಪೂರ್ಣಾಙ್ಕ flags)
{
	ಯದಿ (ul_parse_pid_str(pidstr, pid_ಅಂಕೆ, pfd_ino, flags) < 0) {
		err(EXIT_FAILURE, _("failed to parse PID argument '%s'"), pidstr);
	}
}


#ifdef TEST_PROGRAM_PARSEPID

#ಅನ್ತರ್ಭಾವ <getopt.h>

ಸ್ಥಿರ ಶೂನ್ಯ __attribute__((__noreturn__)) usage(ಶೂನ್ಯ)
{
	fprintf(stdout, " %s [options] <pidstr>\n\n", program_invocation_short_name);
	fputs(" -z, --zero         allow zero (0) as value for <pidstr>\n", stdout);
	fputs(" -n, --negative     allow negative number as value for <pidstr>\n", stdout);

	exit(EXIT_SUCCESS);
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	ಪೂರ್ಣಾಙ್ಕ c, flags = 0, rc = 0;
	uint64_t pidfd_ino = 0;
	pid_t pid_ಅಂಕೆ = 0;
	ವರ್ಣ *str = ರಿಕ್ತ;

	ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ option longopts[] = {
		{ "zero",	0, ರಿಕ್ತ, 'z' },
		{ "negative",	0, ರಿಕ್ತ, 'n' },
		{ "help",       0, ರಿಕ್ತ, 'h' },
		{ ರಿಕ್ತ, 0, ರಿಕ್ತ, 0 },
	};

	ಯಾವತ್((c = getopt_long(argc, argv, "nhz", longopts, ರಿಕ್ತ)) != -1) {
		ವಿಕಲ್ಪ(c) {
		ಪಕ್ಷ 'n':
			flags |= UL_PID_NEGATIVE;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'z':
			flags |= UL_PID_ZERO;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'h':
			usage();
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			err(EXIT_FAILURE, "try --help");
		}
	}

	ಯದಿ (optind == argc)
		errx(EXIT_FAILURE, "missing <pidstr> argument");
	str = argv[optind];

	rc = ul_parse_pid_str(str, &pid_ಅಂಕೆ, &pidfd_ino, flags);
	ಯದಿ (rc)
		err(EXIT_FAILURE, "failed to parse PID '%s'", str);

	printf("PID: %d\n", pid_ಅಂಕೆ);
	ಯದಿ (pidfd_ino)
		printf("inode: %"PRIu64"\n", pidfd_ino);

	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_PARSE_PID */
