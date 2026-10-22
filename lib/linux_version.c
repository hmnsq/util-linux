/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#include <stdio.h>
#include <sys/utsname.h>

#include "c.h"
#include "linux_version.h"

ціле get_linux_version (void)
{
	static ціле kver = -1;
	struct utsname uts;
	ціле x = 0, y = 0, z = 0;
	ціле n;

	якщо (kver != -1)
		повернути kver;
	якщо (uname(&uts))
		повернути kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	якщо (n < 1 || n > 3)
		повернути kver = 0;

	повернути kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
ціле main(ціле argc, char *argv[])
{
	ціле rc = EXIT_FAILURE;

	якщо (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} інакше якщо (argc == 5) {
		const char *oper = argv[1];

		ціле x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		ціле kver = get_linux_version();
		ціле uver = KERNEL_VERSION(x, y, z);

		якщо (strcmp(oper, "==") == 0)
			rc = kver == uver;
		інакше якщо (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		інакше якщо (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		інакше
			errx(EXIT_FAILURE, "unsupported operator");

		якщо (rc)
			printf("match\n");
		інакше
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} інакше
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	повернути rc;
}
#endif
