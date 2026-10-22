/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <errno.h>

#അന്തര്ഭാവ "nls.h"
#അന്തര്ഭാവ "strutils.h"
#അന്തര്ഭാവ "pidutils.h"

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
പൂർണ്ണസംഖ്യ ul_parse_pid_str(അക്ഷരം *pidstr, pid_t *pid_അക്കം, uint64_t *pfd_ino, പൂർണ്ണസംഖ്യ flags)
{
	പൂർണ്ണസംഖ്യ rc;
	അക്ഷരം *end = ശൂന്യ;
	int64_t അക്കം = 0;

	എങ്കിൽ (!pidstr || !*pidstr || !pid_അക്കം)
		തിരികെ -(errno = EINVAL);

	അക്കം = strtoimax(pidstr, &end, 10);
	എങ്കിൽ (അക്കം == 0 && end == pidstr)
		തിരികെ -(errno = EINVAL);

	എങ്കിൽ (errno == ERANGE || അക്കം < SINT_MIN(pid_t) || അക്കം > SINT_MAX(pid_t))
		തിരികെ -(errno = ERANGE);

	എങ്കിൽ (അക്കം == 0 && !(flags & UL_PID_ZERO))
		തിരികെ -(errno = ERANGE);

	എങ്കിൽ (അക്കം < 0 && !(flags & UL_PID_NEGATIVE))
		തിരികെ -(errno = ERANGE);

	*pid_അക്കം = (pid_t) അക്കം;

	എങ്കിൽ (*end == ':' && pfd_ino && അക്കം > 0) {
		rc = ul_strtou64(++end, pfd_ino, 10);
		എങ്കിൽ (rc < 0)
			തിരികെ rc;

		എങ്കിൽ (*pfd_ino == 0)
			തിരികെ -(errno = ERANGE);
		*end = '\0';
	}

	എങ്കിൽ (end && *end != '\0')
		തിരികെ -(errno = EINVAL);
	തിരികെ 0;
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
ശൂന്യം ul_parse_pid_str_or_err(അക്ഷരം *pidstr, pid_t *pid_അക്കം, uint64_t *pfd_ino, പൂർണ്ണസംഖ്യ flags)
{
	എങ്കിൽ (ul_parse_pid_str(pidstr, pid_അക്കം, pfd_ino, flags) < 0) {
		err(EXIT_FAILURE, _("failed to parse PID argument '%s'"), pidstr);
	}
}


#ifdef TEST_PROGRAM_PARSEPID

#അന്തര്ഭാവ <getopt.h>

സ്ഥിതം ശൂന്യം __attribute__((__noreturn__)) usage(ശൂന്യം)
{
	fprintf(stdout, " %s [options] <pidstr>\n\n", program_invocation_short_name);
	fputs(" -z, --zero         allow zero (0) as value for <pidstr>\n", stdout);
	fputs(" -n, --negative     allow negative number as value for <pidstr>\n", stdout);

	exit(EXIT_SUCCESS);
}

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ c, flags = 0, rc = 0;
	uint64_t pidfd_ino = 0;
	pid_t pid_അക്കം = 0;
	അക്ഷരം *str = ശൂന്യ;

	സ്ഥിതം സ്ഥിരം ഘടന option longopts[] = {
		{ "zero",	0, ശൂന്യ, 'z' },
		{ "negative",	0, ശൂന്യ, 'n' },
		{ "help",       0, ശൂന്യ, 'h' },
		{ ശൂന്യ, 0, ശൂന്യ, 0 },
	};

	ആകുമ്പോൾ((c = getopt_long(argc, argv, "nhz", longopts, ശൂന്യ)) != -1) {
		തിരഞ്ഞെടുപ്പ്(c) {
		അവസ്ഥ 'n':
			flags |= UL_PID_NEGATIVE;
			നിർത്തുക;
		അവസ്ഥ 'z':
			flags |= UL_PID_ZERO;
			നിർത്തുക;
		അവസ്ഥ 'h':
			usage();
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			err(EXIT_FAILURE, "try --help");
		}
	}

	എങ്കിൽ (optind == argc)
		errx(EXIT_FAILURE, "missing <pidstr> argument");
	str = argv[optind];

	rc = ul_parse_pid_str(str, &pid_അക്കം, &pidfd_ino, flags);
	എങ്കിൽ (rc)
		err(EXIT_FAILURE, "failed to parse PID '%s'", str);

	printf("PID: %d\n", pid_അക്കം);
	എങ്കിൽ (pidfd_ino)
		printf("inode: %"PRIu64"\n", pidfd_ino);

	തിരികെ EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM_PARSE_PID */
