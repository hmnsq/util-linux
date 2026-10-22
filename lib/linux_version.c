/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ضمن <stdio.h>
#ضمن <sys/utsname.h>

#ضمن "c.h"
#ضمن "linux_version.h"

صحيح get_linux_version (فراغ)
{
	ساكن صحيح kver = -1;
	بنية utsname uts;
	صحيح x = 0, y = 0, z = 0;
	صحيح n;

	ܐܢ (kver != -1)
		ܦܢܐ kver;
	ܐܢ (uname(&uts))
		ܦܢܐ kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	ܐܢ (n < 1 || n > 3)
		ܦܢܐ kver = 0;

	ܦܢܐ kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
صحيح main(صحيح argc, حرف *argv[])
{
	صحيح rc = EXIT_FAILURE;

	ܐܢ (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} ܐܚܪܢܐ ܐܢ (argc == 5) {
		ثابت حرف *oper = argv[1];

		صحيح x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		صحيح kver = get_linux_version();
		صحيح uver = KERNEL_VERSION(x, y, z);

		ܐܢ (strcmp(oper, "==") == 0)
			rc = kver == uver;
		ܐܚܪܢܐ ܐܢ (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		ܐܚܪܢܐ ܐܢ (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		ܐܚܪܢܐ
			errx(EXIT_FAILURE, "unsupported operator");

		ܐܢ (rc)
			printf("match\n");
		ܐܚܪܢܐ
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} ܐܚܪܢܐ
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	ܦܢܐ rc;
}
#endif
