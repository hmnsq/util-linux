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

	もし (kver != -1)
		戻る kver;
	もし (uname(&uts))
		戻る kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	もし (n < 1 || n > 3)
		戻る kver = 0;

	戻る kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
整数 メインページ(整数 argc, char *argv[])
{
	整数 rc = EXIT_FAILURE;

	もし (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} 否则 もし (argc == 5) {
		const char *oper = argv[1];

		整数 x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		整数 kver = get_linux_version();
		整数 uver = KERNEL_VERSION(x, y, z);

		もし (strcmp(oper, "==") == 0)
			rc = kver == uver;
		否则 もし (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		否则 もし (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		否则
			errx(EXIT_FAILURE, "unsupported operator");

		もし (rc)
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

	戻る rc;
}
#endif
