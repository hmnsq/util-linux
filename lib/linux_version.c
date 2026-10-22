/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#include <stdio.h>
#include <sys/utsname.h>

#include "c.h"
#include "linux_version.h"

จำนวนเต็ม get_linux_version (void)
{
	static จำนวนเต็ม kver = -1;
	sa_nracanaa utsname uts;
	จำนวนเต็ม x = 0, y = 0, z = 0;
	จำนวนเต็ม n;

	ถ้า (kver != -1)
		คืน kver;
	ถ้า (uname(&uts))
		คืน kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	ถ้า (n < 1 || n > 3)
		คืน kver = 0;

	คืน kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
จำนวนเต็ม หลัก(จำนวนเต็ม argc, var_na *argv[])
{
	จำนวนเต็ม rc = EXIT_FAILURE;

	ถ้า (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} มิฉะนั้น ถ้า (argc == 5) {
		const var_na *oper = argv[1];

		จำนวนเต็ม x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		จำนวนเต็ม kver = get_linux_version();
		จำนวนเต็ม uver = KERNEL_VERSION(x, y, z);

		ถ้า (strcmp(oper, "==") == 0)
			rc = kver == uver;
		มิฉะนั้น ถ้า (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		มิฉะนั้น ถ้า (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		มิฉะนั้น
			errx(EXIT_FAILURE, "unsupported operator");

		ถ้า (rc)
			printf("match\n");
		มิฉะนั้น
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} มิฉะนั้น
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	คืน rc;
}
#endif
