/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#include <stdio.h>
#include <sys/utsname.h>

#include "c.h"
#include "linux_version.h"

ຈຳນວນເຕັມ get_linux_version (void)
{
	static ຈຳນວນເຕັມ kver = -1;
	sa_nracanaa utsname uts;
	ຈຳນວນເຕັມ x = 0, y = 0, z = 0;
	ຈຳນວນເຕັມ n;

	ຖ້າ (kver != -1)
		ສົ່ງຄືນ kver;
	ຖ້າ (uname(&uts))
		ສົ່ງຄືນ kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	ຖ້າ (n < 1 || n > 3)
		ສົ່ງຄືນ kver = 0;

	ສົ່ງຄືນ kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
ຈຳນວນເຕັມ main(ຈຳນວນເຕັມ argc, var_na *argv[])
{
	ຈຳນວນເຕັມ rc = EXIT_FAILURE;

	ຖ້າ (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} ບໍ່ດັ່ງນັ້ນ ຖ້າ (argc == 5) {
		const var_na *oper = argv[1];

		ຈຳນວນເຕັມ x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		ຈຳນວນເຕັມ kver = get_linux_version();
		ຈຳນວນເຕັມ uver = KERNEL_VERSION(x, y, z);

		ຖ້າ (strcmp(oper, "==") == 0)
			rc = kver == uver;
		ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		ບໍ່ດັ່ງນັ້ນ
			errx(EXIT_FAILURE, "unsupported operator");

		ຖ້າ (rc)
			printf("match\n");
		ບໍ່ດັ່ງນັ້ນ
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} ບໍ່ດັ່ງນັ້ນ
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	ສົ່ງຄືນ rc;
}
#endif
