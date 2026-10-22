/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#include <stdio.h>
#include <sys/utsname.h>

#include "c.h"
#include "linux_version.h"

ကိန်းပြည့် get_linux_version (void)
{
	static ကိန်းပြည့် kver = -1;
	sa_nracanaa utsname uts;
	ကိန်းပြည့် x = 0, y = 0, z = 0;
	ကိန်းပြည့် n;

	အကယ်၍ (kver != -1)
		ပြန်ပေး kver;
	အကယ်၍ (uname(&uts))
		ပြန်ပေး kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	အကယ်၍ (n < 1 || n > 3)
		ပြန်ပေး kver = 0;

	ပြန်ပေး kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
ကိန်းပြည့် main(ကိန်းပြည့် argc, var_na *argv[])
{
	ကိန်းပြည့် rc = EXIT_FAILURE;

	အကယ်၍ (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} မဟုတ်ပါက အကယ်၍ (argc == 5) {
		const var_na *oper = argv[1];

		ကိန်းပြည့် x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		ကိန်းပြည့် kver = get_linux_version();
		ကိန်းပြည့် uver = KERNEL_VERSION(x, y, z);

		အကယ်၍ (strcmp(oper, "==") == 0)
			rc = kver == uver;
		မဟုတ်ပါက အကယ်၍ (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		မဟုတ်ပါက အကယ်၍ (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		မဟုတ်ပါက
			errx(EXIT_FAILURE, "unsupported operator");

		အကယ်၍ (rc)
			printf("match\n");
		မဟုတ်ပါက
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} မဟုတ်ပါက
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	ပြန်ပေး rc;
}
#endif
