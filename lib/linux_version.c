/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#include <stdio.h>
#include <sys/utsname.h>

#include "c.h"
#include "linux_version.h"

ཧྲིལ get_linux_version (void)
{
	static ཧྲིལ kver = -1;
	sa_nracanaa utsname uts;
	ཧྲིལ x = 0, y = 0, z = 0;
	ཧྲིལ n;

	གལ་སྲིད (kver != -1)
		ལོག kver;
	གལ་སྲིད (uname(&uts))
		ལོག kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	གལ་སྲིད (n < 1 || n > 3)
		ལོག kver = 0;

	ལོག kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
ཧྲིལ main(ཧྲིལ argc, var_na *argv[])
{
	ཧྲིལ rc = EXIT_FAILURE;

	གལ་སྲིད (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} དེ་མིན གལ་སྲིད (argc == 5) {
		const var_na *oper = argv[1];

		ཧྲིལ x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		ཧྲིལ kver = get_linux_version();
		ཧྲིལ uver = KERNEL_VERSION(x, y, z);

		གལ་སྲིད (strcmp(oper, "==") == 0)
			rc = kver == uver;
		དེ་མིན གལ་སྲིད (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		དེ་མིན གལ་སྲིད (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		དེ་མིན
			errx(EXIT_FAILURE, "unsupported operator");

		གལ་སྲིད (rc)
			printf("match\n");
		དེ་མིན
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} དེ་མིན
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	ལོག rc;
}
#endif
