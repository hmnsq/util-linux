/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#شامل <stdio.h>
#شامل <sys/utsname.h>

#شامل "c.h"
#شامل "linux_version.h"

صحیح get_linux_version (تهی)
{
	ایستا صحیح kver = -1;
	ساختار utsname uts;
	صحیح x = 0, y = 0, z = 0;
	صحیح n;

	اگر (kver != -1)
		بازگشت kver;
	اگر (uname(&uts))
		بازگشت kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	اگر (n < 1 || n > 3)
		بازگشت kver = 0;

	بازگشت kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
صحیح اصلی(صحیح argc, نویسه *argv[])
{
	صحیح rc = EXIT_FAILURE;

	اگر (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} وگرنه اگر (argc == 5) {
		ثابت نویسه *oper = argv[1];

		صحیح x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		صحیح kver = get_linux_version();
		صحیح uver = KERNEL_VERSION(x, y, z);

		اگر (strcmp(oper, "==") == 0)
			rc = kver == uver;
		وگرنه اگر (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		وگرنه اگر (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		وگرنه
			errx(EXIT_FAILURE, "unsupported operator");

		اگر (rc)
			printf("match\n");
		وگرنه
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} وگرنه
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	بازگشت rc;
}
#endif
