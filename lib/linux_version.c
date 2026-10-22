/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#include <stdio.h>
#include <sys/utsname.h>

#include "c.h"
#include "linux_version.h"

ចំនួនគត់ get_linux_version (void)
{
	static ចំនួនគត់ kver = -1;
	sa_nracanaa utsname uts;
	ចំនួនគត់ x = 0, y = 0, z = 0;
	ចំនួនគត់ n;

	ប្រសិនបើ (kver != -1)
		ត្រឡប់ kver;
	ប្រសិនបើ (uname(&uts))
		ត្រឡប់ kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	ប្រសិនបើ (n < 1 || n > 3)
		ត្រឡប់ kver = 0;

	ត្រឡប់ kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
ចំនួនគត់ main(ចំនួនគត់ argc, var_na *argv[])
{
	ចំនួនគត់ rc = EXIT_FAILURE;

	ប្រសិនបើ (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} បើមិនដូច្នេះ ប្រសិនបើ (argc == 5) {
		const var_na *oper = argv[1];

		ចំនួនគត់ x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		ចំនួនគត់ kver = get_linux_version();
		ចំនួនគត់ uver = KERNEL_VERSION(x, y, z);

		ប្រសិនបើ (strcmp(oper, "==") == 0)
			rc = kver == uver;
		បើមិនដូច្នេះ ប្រសិនបើ (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		បើមិនដូច្នេះ ប្រសិនបើ (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		បើមិនដូច្នេះ
			errx(EXIT_FAILURE, "unsupported operator");

		ប្រសិនបើ (rc)
			printf("match\n");
		បើមិនដូច្នេះ
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} បើមិនដូច្នេះ
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	ត្រឡប់ rc;
}
#endif
