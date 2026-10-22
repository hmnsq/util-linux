/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <sys/utsname.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "linux_version.h"

পূর্ণাঙ্ক get_linux_version (শূন্য)
{
	স্থির পূর্ণাঙ্ক kver = -1;
	সংরচনা utsname uts;
	পূর্ণাঙ্ক x = 0, y = 0, z = 0;
	পূর্ণাঙ্ক n;

	যদি (kver != -1)
		প্রত্যাগম kver;
	যদি (uname(&uts))
		প্রত্যাগম kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	যদি (n < 1 || n > 3)
		প্রত্যাগম kver = 0;

	প্রত্যাগম kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ *argv[])
{
	পূর্ণাঙ্ক rc = EXIT_FAILURE;

	যদি (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} অন্যথা যদি (argc == 5) {
		ধ্রুব বর্ণ *oper = argv[1];

		পূর্ণাঙ্ক x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		পূর্ণাঙ্ক kver = get_linux_version();
		পূর্ণাঙ্ক uver = KERNEL_VERSION(x, y, z);

		যদি (strcmp(oper, "==") == 0)
			rc = kver == uver;
		অন্যথা যদি (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		অন্যথা যদি (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		অন্যথা
			errx(EXIT_FAILURE, "unsupported operator");

		যদি (rc)
			printf("match\n");
		অন্যথা
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} অন্যথা
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	প্রত্যাগম rc;
}
#endif
