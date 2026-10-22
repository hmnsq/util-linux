/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#include <stdio.h>
#include <sys/utsname.h>

#include "c.h"
#include "linux_version.h"

целое get_linux_version (void)
{
	static целое kver = -1;
	struct utsname uts;
	целое x = 0, y = 0, z = 0;
	целое n;

	если (kver != -1)
		вернуть kver;
	если (uname(&uts))
		вернуть kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	если (n < 1 || n > 3)
		вернуть kver = 0;

	вернуть kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
целое основной(целое argc, char *argv[])
{
	целое rc = EXIT_FAILURE;

	если (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} иначе если (argc == 5) {
		const char *oper = argv[1];

		целое x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		целое kver = get_linux_version();
		целое uver = KERNEL_VERSION(x, y, z);

		если (strcmp(oper, "==") == 0)
			rc = kver == uver;
		иначе если (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		иначе если (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		иначе
			errx(EXIT_FAILURE, "unsupported operator");

		если (rc)
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

	вернуть rc;
}
#endif
