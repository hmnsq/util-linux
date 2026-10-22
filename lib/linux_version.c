/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#include <stdio.h>
#include <sys/utsname.h>

#include "c.h"
#include "linux_version.h"

цяло get_linux_version (void)
{
	static цяло kver = -1;
	struct utsname uts;
	цяло x = 0, y = 0, z = 0;
	цяло n;

	ако (kver != -1)
		върни kver;
	ако (uname(&uts))
		върни kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	ако (n < 1 || n > 3)
		върни kver = 0;

	върни kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
цяло main(цяло argc, char *argv[])
{
	цяло rc = EXIT_FAILURE;

	ако (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} иначе ако (argc == 5) {
		const char *oper = argv[1];

		цяло x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		цяло kver = get_linux_version();
		цяло uver = KERNEL_VERSION(x, y, z);

		ако (strcmp(oper, "==") == 0)
			rc = kver == uver;
		иначе ако (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		иначе ако (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		иначе
			errx(EXIT_FAILURE, "unsupported operator");

		ако (rc)
			printf("match\n");
		иначе
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} иначе
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	върни rc;
}
#endif
