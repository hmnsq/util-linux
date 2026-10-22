/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <errno.h>

#అన్తర్భావ "nls.h"
#అన్తర్భావ "strutils.h"
#అన్తర్భావ "pidutils.h"

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
పూర్ణాంకం ul_parse_pid_str(అక్షరం *pidstr, pid_t *pid_అంకె, uint64_t *pfd_ino, పూర్ణాంకం flags)
{
	పూర్ణాంకం rc;
	అక్షరం *చివర = ఖాళీ;
	int64_t అంకె = 0;

	ఒకవేళ (!pidstr || !*pidstr || !pid_అంకె)
		తిరిగి -(errno = EINVAL);

	అంకె = strtoimax(pidstr, &చివర, 10);
	ఒకవేళ (అంకె == 0 && చివర == pidstr)
		తిరిగి -(errno = EINVAL);

	ఒకవేళ (errno == ERANGE || అంకె < SINT_MIN(pid_t) || అంకె > SINT_MAX(pid_t))
		తిరిగి -(errno = ERANGE);

	ఒకవేళ (అంకె == 0 && !(flags & UL_PID_ZERO))
		తిరిగి -(errno = ERANGE);

	ఒకవేళ (అంకె < 0 && !(flags & UL_PID_NEGATIVE))
		తిరిగి -(errno = ERANGE);

	*pid_అంకె = (pid_t) అంకె;

	ఒకవేళ (*చివర == ':' && pfd_ino && అంకె > 0) {
		rc = ul_strtou64(++చివర, pfd_ino, 10);
		ఒకవేళ (rc < 0)
			తిరిగి rc;

		ఒకవేళ (*pfd_ino == 0)
			తిరిగి -(errno = ERANGE);
		*చివర = '\0';
	}

	ఒకవేళ (చివర && *చివర != '\0')
		తిరిగి -(errno = EINVAL);
	తిరిగి 0;
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
శూన్య ul_parse_pid_str_or_err(అక్షరం *pidstr, pid_t *pid_అంకె, uint64_t *pfd_ino, పూర్ణాంకం flags)
{
	ఒకవేళ (ul_parse_pid_str(pidstr, pid_అంకె, pfd_ino, flags) < 0) {
		err(EXIT_FAILURE, _("failed to parse PID argument '%s'"), pidstr);
	}
}


#నిర్వచితమైతే TEST_కార్యక్రమం_PARSEPID

#అన్తర్భావ <getopt.h>

స్థిర శూన్య __attribute__((__noreturn__)) usage(శూన్య)
{
	fprintf(stdout, " %s [options] <pidstr>\n\n", కార్యక్రమం_invocation_short_name);
	fputs(" -z, --zero         allow zero (0) as value for <pidstr>\n", stdout);
	fputs(" -n, --negative     allow negative number as value for <pidstr>\n", stdout);

	నిష్క్రమించు(EXIT_SUCCESS);
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	పూర్ణాంకం c, flags = 0, rc = 0;
	uint64_t pidfd_ino = 0;
	pid_t pid_అంకె = 0;
	అక్షరం *str = ఖాళీ;

	స్థిర స్థిరరాశి నిర్మాణం option longopts[] = {
		{ "zero",	0, ఖాళీ, 'z' },
		{ "negative",	0, ఖాళీ, 'n' },
		{ "help",       0, ఖాళీ, 'h' },
		{ ఖాళీ, 0, ఖాళీ, 0 },
	};

	అయితే((c = getopt_long(argc, argv, "nhz", longopts, ఖాళీ)) != -1) {
		ఎంపిక(c) {
		స్థితి 'n':
			flags |= UL_PID_NEGATIVE;
			ఆపు;
		స్థితి 'z':
			flags |= UL_PID_ZERO;
			ఆపు;
		స్థితి 'h':
			usage();
			ఆపు;
		అప్రమేయ:
			err(EXIT_FAILURE, "try --help");
		}
	}

	ఒకవేళ (optind == argc)
		errx(EXIT_FAILURE, "missing <pidstr> argument");
	str = argv[optind];

	rc = ul_parse_pid_str(str, &pid_అంకె, &pidfd_ino, flags);
	ఒకవేళ (rc)
		err(EXIT_FAILURE, "failed to parse PID '%s'", str);

	రాయి("PID: %d\n", pid_అంకె);
	ఒకవేళ (pidfd_ino)
		రాయి("inode: %"PRIu64"\n", pidfd_ino);

	తిరిగి EXIT_SUCCESS;
}

#అయితే_ముగింపు /* TEST_PROGRAM_PARSE_PID */
