/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#include <stdio.h>
#include <sys/utsname.h>

#include "c.h"
#include "linux_version.h"

පූර්ණ_සංඛ්‍යා get_linux_version (void)
{
	static පූර්ණ_සංඛ්‍යා kver = -1;
	sa_nracanaa utsname uts;
	පූර්ණ_සංඛ්‍යා x = 0, y = 0, z = 0;
	පූර්ණ_සංඛ්‍යා n;

	නම් (kver != -1)
		ආපසු kver;
	නම් (uname(&uts))
		ආපසු kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	නම් (n < 1 || n > 3)
		ආපසු kver = 0;

	ආපසු kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
පූර්ණ_සංඛ්‍යා main(පූර්ණ_සංඛ්‍යා argc, var_na *argv[])
{
	පූර්ණ_සංඛ්‍යා rc = EXIT_FAILURE;

	නම් (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} නැතහොත් නම් (argc == 5) {
		const var_na *oper = argv[1];

		පූර්ණ_සංඛ්‍යා x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		පූර්ණ_සංඛ්‍යා kver = get_linux_version();
		පූර්ණ_සංඛ්‍යා uver = KERNEL_VERSION(x, y, z);

		නම් (strcmp(oper, "==") == 0)
			rc = kver == uver;
		නැතහොත් නම් (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		නැතහොත් නම් (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		නැතහොත්
			errx(EXIT_FAILURE, "unsupported operator");

		නම් (rc)
			printf("match\n");
		නැතහොත්
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} නැතහොත්
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	ආපසු rc;
}
#endif
