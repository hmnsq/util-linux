/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#include <stdio.h>
#include <sys/utsname.h>

#include "c.h"
#include "linux_version.h"

整数 get_linux_version (void)
{
	static 整数 kver = -1;
	struct utsname uts;
	整数 x = 0, y = 0, z = 0;
	整数 n;

	如果 (kver != -1)
		返回 kver;
	如果 (uname(&uts))
		返回 kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	如果 (n < 1 || n > 3)
		返回 kver = 0;

	返回 kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
整数 主要(整数 argc, char *argv[])
{
	整数 rc = EXIT_FAILURE;

	如果 (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} 否则 如果 (argc == 5) {
		const char *oper = argv[1];

		整数 x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		整数 kver = get_linux_version();
		整数 uver = KERNEL_VERSION(x, y, z);

		如果 (strcmp(oper, "==") == 0)
			rc = kver == uver;
		否则 如果 (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		否则 如果 (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		否则
			errx(EXIT_FAILURE, "unsupported operator");

		如果 (rc)
			printf("match\n");
		否则
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} 否则
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	返回 rc;
}
#endif
