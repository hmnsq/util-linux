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
ಪೂರ್ಣಾಂಕ ul_parse_pid_str(ಅಕ್ಷರ *pidstr, pid_t *pid_ಅಂಕೆ, uint64_t *pfd_ino, ಪೂರ್ಣಾಂಕ flags)
{
	ಪೂರ್ಣಾಂಕ rc;
	ಅಕ್ಷರ *ಕೊನೆ = ಖಾಲಿ;
	int64_t ಅಂಕೆ = 0;

	ಆದರೆ (!pidstr || !*pidstr || !pid_ಅಂಕೆ)
		ಹಿಂತಿರುಗಿ -(errno = EINVAL);

	ಅಂಕೆ = strtoimax(pidstr, &ಕೊನೆ, 10);
	ಆದರೆ (ಅಂಕೆ == 0 && ಕೊನೆ == pidstr)
		ಹಿಂತಿರುಗಿ -(errno = EINVAL);

	ಆದರೆ (errno == ERANGE || ಅಂಕೆ < SINT_MIN(pid_t) || ಅಂಕೆ > SINT_MAX(pid_t))
		ಹಿಂತಿರುಗಿ -(errno = ERANGE);

	ಆದರೆ (ಅಂಕೆ == 0 && !(flags & UL_PID_ZERO))
		ಹಿಂತಿರುಗಿ -(errno = ERANGE);

	ಆದರೆ (ಅಂಕೆ < 0 && !(flags & UL_PID_NEGATIVE))
		ಹಿಂತಿರುಗಿ -(errno = ERANGE);

	*pid_ಅಂಕೆ = (pid_t) ಅಂಕೆ;

	ಆದರೆ (*ಕೊನೆ == ':' && pfd_ino && ಅಂಕೆ > 0) {
		rc = ul_strtou64(++ಕೊನೆ, pfd_ino, 10);
		ಆದರೆ (rc < 0)
			ಹಿಂತಿರುಗಿ rc;

		ಆದರೆ (*pfd_ino == 0)
			ಹಿಂತಿರುಗಿ -(errno = ERANGE);
		*ಕೊನೆ = '\0';
	}

	ಆದರೆ (ಕೊನೆ && *ಕೊನೆ != '\0')
		ಹಿಂತಿರುಗಿ -(errno = EINVAL);
	ಹಿಂತಿರುಗಿ 0;
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
ಶೂನ್ಯ ul_parse_pid_str_or_err(ಅಕ್ಷರ *pidstr, pid_t *pid_ಅಂಕೆ, uint64_t *pfd_ino, ಪೂರ್ಣಾಂಕ flags)
{
	ಆದರೆ (ul_parse_pid_str(pidstr, pid_ಅಂಕೆ, pfd_ino, flags) < 0) {
		err(EXIT_FAILURE, _("failed to parse PID argument '%s'"), pidstr);
	}
}


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_PARSEPID

#ಅನ್ತರ್ಭಾವ <getopt.h>

ಸ್ಥಿರ ಶೂನ್ಯ __attribute__((__noreturn__)) usage(ಶೂನ್ಯ)
{
	fprintf(stdout, " %s [options] <pidstr>\n\n", ಕಾರ್ಯಕ್ರಮ_invocation_short_name);
	fputs(" -z, --zero         allow zero (0) as value for <pidstr>\n", stdout);
	fputs(" -n, --negative     allow negative number as value for <pidstr>\n", stdout);

	ನಿರ್ಗಮಿಸು(EXIT_SUCCESS);
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ c, flags = 0, rc = 0;
	uint64_t pidfd_ino = 0;
	pid_t pid_ಅಂಕೆ = 0;
	ಅಕ್ಷರ *str = ಖಾಲಿ;

	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[] = {
		{ "zero",	0, ಖಾಲಿ, 'z' },
		{ "negative",	0, ಖಾಲಿ, 'n' },
		{ "help",       0, ಖಾಲಿ, 'h' },
		{ ಖಾಲಿ, 0, ಖಾಲಿ, 0 },
	};

	ಆಗ((c = getopt_long(argc, argv, "nhz", longopts, ಖಾಲಿ)) != -1) {
		ಆಯ್ಕೆ(c) {
		ಸ್ಥಿತಿ 'n':
			flags |= UL_PID_NEGATIVE;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'z':
			flags |= UL_PID_ZERO;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'h':
			usage();
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			err(EXIT_FAILURE, "try --help");
		}
	}

	ಆದರೆ (optind == argc)
		errx(EXIT_FAILURE, "missing <pidstr> argument");
	str = argv[optind];

	rc = ul_parse_pid_str(str, &pid_ಅಂಕೆ, &pidfd_ino, flags);
	ಆದರೆ (rc)
		err(EXIT_FAILURE, "failed to parse PID '%s'", str);

	ಬರೆ("PID: %d\n", pid_ಅಂಕೆ);
	ಆದರೆ (pidfd_ino)
		ಬರೆ("inode: %"PRIu64"\n", pidfd_ino);

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

#ಆದರೆ_ಅಂತ್ಯ /* TEST_PROGRAM_PARSE_PID */
