/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "nls.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "strutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "pidutils.h"

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
ਪੂਰਨ ul_parse_pid_str(ਅੱਖਰ *pidstr, pid_t *pid_ਅੰਕ, uint64_t *pfd_ino, ਪੂਰਨ flags)
{
	ਪੂਰਨ rc;
	ਅੱਖਰ *ਸਿਰਾ = ਨਿਰਾ;
	int64_t ਅੰਕ = 0;

	ਜੇ (!pidstr || !*pidstr || !pid_ਅੰਕ)
		ਵਾਪਸ -(errno = EINVAL);

	ਅੰਕ = strtoimax(pidstr, &ਸਿਰਾ, 10);
	ਜੇ (ਅੰਕ == 0 && ਸਿਰਾ == pidstr)
		ਵਾਪਸ -(errno = EINVAL);

	ਜੇ (errno == ERANGE || ਅੰਕ < SINT_MIN(pid_t) || ਅੰਕ > SINT_MAX(pid_t))
		ਵਾਪਸ -(errno = ERANGE);

	ਜੇ (ਅੰਕ == 0 && !(flags & UL_PID_ZERO))
		ਵਾਪਸ -(errno = ERANGE);

	ਜੇ (ਅੰਕ < 0 && !(flags & UL_PID_NEGATIVE))
		ਵਾਪਸ -(errno = ERANGE);

	*pid_ਅੰਕ = (pid_t) ਅੰਕ;

	ਜੇ (*ਸਿਰਾ == ':' && pfd_ino && ਅੰਕ > 0) {
		rc = ul_strtou64(++ਸਿਰਾ, pfd_ino, 10);
		ਜੇ (rc < 0)
			ਵਾਪਸ rc;

		ਜੇ (*pfd_ino == 0)
			ਵਾਪਸ -(errno = ERANGE);
		*ਸਿਰਾ = '\0';
	}

	ਜੇ (ਸਿਰਾ && *ਸਿਰਾ != '\0')
		ਵਾਪਸ -(errno = EINVAL);
	ਵਾਪਸ 0;
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
ਖਾਲੀ ul_parse_pid_str_or_err(ਅੱਖਰ *pidstr, pid_t *pid_ਅੰਕ, uint64_t *pfd_ino, ਪੂਰਨ flags)
{
	ਜੇ (ul_parse_pid_str(pidstr, pid_ਅੰਕ, pfd_ino, flags) < 0) {
		err(EXIT_FAILURE, _("failed to parse PID argument '%s'"), pidstr);
	}
}


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_PARSEPID

#ਅਨ੍ਤਰ੍ਭਾਵ <getopt.h>

ਸਥਿਰ ਖਾਲੀ __attribute__((__noreturn__)) usage(ਖਾਲੀ)
{
	fprintf(stdout, " %s [options] <pidstr>\n\n", ਪ੍ਰੋਗਰਾਮ_invocation_short_name);
	fputs(" -z, --zero         allow zero (0) as value for <pidstr>\n", stdout);
	fputs(" -n, --negative     allow negative number as value for <pidstr>\n", stdout);

	ਬਾਹਰ_ਨਿਕਲੋ(EXIT_SUCCESS);
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਪੂਰਨ c, flags = 0, rc = 0;
	uint64_t pidfd_ino = 0;
	pid_t pid_ਅੰਕ = 0;
	ਅੱਖਰ *str = ਨਿਰਾ;

	ਸਥਿਰ ਅਚਲ ਬਣਤਰ option longopts[] = {
		{ "zero",	0, ਨਿਰਾ, 'z' },
		{ "negative",	0, ਨਿਰਾ, 'n' },
		{ "help",       0, ਨਿਰਾ, 'h' },
		{ ਨਿਰਾ, 0, ਨਿਰਾ, 0 },
	};

	ਜਦੋਂ((c = getopt_long(argc, argv, "nhz", longopts, ਨਿਰਾ)) != -1) {
		ਚੋਣ(c) {
		ਹਾਲਤ 'n':
			flags |= UL_PID_NEGATIVE;
			ਰੋਕੋ;
		ਹਾਲਤ 'z':
			flags |= UL_PID_ZERO;
			ਰੋਕੋ;
		ਹਾਲਤ 'h':
			usage();
			ਰੋਕੋ;
		ਮੂਲ:
			err(EXIT_FAILURE, "try --help");
		}
	}

	ਜੇ (optind == argc)
		errx(EXIT_FAILURE, "missing <pidstr> argument");
	str = argv[optind];

	rc = ul_parse_pid_str(str, &pid_ਅੰਕ, &pidfd_ino, flags);
	ਜੇ (rc)
		err(EXIT_FAILURE, "failed to parse PID '%s'", str);

	ਲਿਖੋ("PID: %d\n", pid_ਅੰਕ);
	ਜੇ (pidfd_ino)
		ਲਿਖੋ("inode: %"PRIu64"\n", pidfd_ino);

	ਵਾਪਸ EXIT_SUCCESS;
}

#ਜੇ_ਅੰਤ /* TEST_PROGRAM_PARSE_PID */
