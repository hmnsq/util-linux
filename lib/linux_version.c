/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#include <stdio.h>
#include <sys/utsname.h>

#include "c.h"
#include "linux_version.h"

số_nguyên get_linux_version (void)
{
	static số_nguyên kver = -1;
	struct utsname uts;
	số_nguyên x = 0, y = 0, z = 0;
	số_nguyên n;

	nếu (kver != -1)
		trả_về kver;
	nếu (uname(&uts))
		trả_về kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	nếu (n < 1 || n > 3)
		trả_về kver = 0;

	trả_về kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
số_nguyên main(số_nguyên argc, char *argv[])
{
	số_nguyên rc = EXIT_FAILURE;

	nếu (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} không_thì nếu (argc == 5) {
		const char *oper = argv[1];

		số_nguyên x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		số_nguyên kver = get_linux_version();
		số_nguyên uver = KERNEL_VERSION(x, y, z);

		nếu (strcmp(oper, "==") == 0)
			rc = kver == uver;
		không_thì nếu (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		không_thì nếu (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		không_thì
			errx(EXIT_FAILURE, "unsupported operator");

		nếu (rc)
			printf("match\n");
		không_thì
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} không_thì
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	trả_về rc;
}
#endif
