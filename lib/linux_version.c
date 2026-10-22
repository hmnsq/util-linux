/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#include <stdio.h>
#include <sys/utsname.h>

#include "c.h"
#include "linux_version.h"

정수 get_linux_version (void)
{
	static 정수 kver = -1;
	struct utsname uts;
	정수 x = 0, y = 0, z = 0;
	정수 n;

	만약 (kver != -1)
		반환 kver;
	만약 (uname(&uts))
		반환 kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	만약 (n < 1 || n > 3)
		반환 kver = 0;

	반환 kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
정수 main(정수 argc, char *argv[])
{
	정수 rc = EXIT_FAILURE;

	만약 (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} 아니면 만약 (argc == 5) {
		const char *oper = argv[1];

		정수 x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		정수 kver = get_linux_version();
		정수 uver = KERNEL_VERSION(x, y, z);

		만약 (strcmp(oper, "==") == 0)
			rc = kver == uver;
		아니면 만약 (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		아니면 만약 (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		아니면
			errx(EXIT_FAILURE, "unsupported operator");

		만약 (rc)
			printf("match\n");
		아니면
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} 아니면
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	반환 rc;
}
#endif
