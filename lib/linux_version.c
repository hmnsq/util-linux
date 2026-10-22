/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#include <stdio.h>
#include <sys/utsname.h>

#include "c.h"
#include "linux_version.h"

цео get_linux_version (void)
{
	static цео kver = -1;
	struct utsname uts;
	цео x = 0, y = 0, z = 0;
	цео n;

	ако (kver != -1)
		врати kver;
	ако (uname(&uts))
		врати kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	ако (n < 1 || n > 3)
		врати kver = 0;

	врати kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
цео main(цео argc, char *argv[])
{
	цео rc = EXIT_FAILURE;

	ако (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} инако ако (argc == 5) {
		const char *oper = argv[1];

		цео x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		цео kver = get_linux_version();
		цео uver = KERNEL_VERSION(x, y, z);

		ако (strcmp(oper, "==") == 0)
			rc = kver == uver;
		инако ако (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		инако ако (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		инако
			errx(EXIT_FAILURE, "unsupported operator");

		ако (rc)
			printf("match\n");
		инако
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} инако
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	врати rc;
}
#endif
