/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ضمن <stdio.h>
#ضمن <sys/utsname.h>

#ضمن "c.h"
#ضمن "linux_version.h"

صحيح get_linux_نسخه (فراغ)
{
	ساكن صحيح kver = -1;
	بنية utsname uts;
	صحيح x = 0, y = 0, z = 0;
	صحيح n;

	إذا (kver != -1)
		أرجع kver;
	إذا (uname(&uts))
		أرجع kver = 0;

	n = از_رشته_بخوان(uts.release, "%d.%d.%d", &x, &y, &z);
	إذا (n < 1 || n > 3)
		أرجع kver = 0;

	أرجع kver = KERNEL_نسخه(x, y, z);
}

#اگر_تعریف_شده TEST_برنامه_LINUXVERSION
# include <stdlib.h>
صحيح اصلی(صحيح argc, حرف *argv[])
{
	صحيح rc = EXIT_FAILURE;

	إذا (argc == 1) {
		چاپ_کن("Linux version: %d\n", get_linux_نسخه());
		rc = EXIT_SUCCESS;

	} وإلا إذا (argc == 5) {
		ثابت حرف *oper = argv[1];

		صحيح x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		صحيح kver = get_linux_نسخه();
		صحيح uver = KERNEL_نسخه(x, y, z);

		إذا (strcmp(oper, "==") == 0)
			rc = kver == uver;
		وإلا إذا (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		وإلا إذا (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		وإلا
			errx(EXIT_FAILURE, "unsupported operator");

		إذا (rc)
			چاپ_کن("match\n");
		وإلا
			چاپ_کن("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} وإلا
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 برنامه_invocation_short_name);

	أرجع rc;
}
#پایان_اگر
