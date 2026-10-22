/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ضمن <stdio.h>
#ضمن <sys/utsname.h>

#ضمن "c.h"
#ضمن "linux_version.h"

صحيح get_linux_نسخو (فراغ)
{
	ساكن صحيح kver = -1;
	بنية utsname uts;
	صحيح x = 0, y = 0, z = 0;
	صحيح n;

	إذا (kver != -1)
		أرجع kver;
	إذا (uname(&uts))
		أرجع kver = 0;

	n = لڙي_مان_پڙهو(uts.release, "%d.%d.%d", &x, &y, &z);
	إذا (n < 1 || n > 3)
		أرجع kver = 0;

	أرجع kver = KERNEL_نسخو(x, y, z);
}

#جيڪڏهن_وصف_ٿيل TEST_پروگرام_LINUXVERSION
# include <stdlib.h>
صحيح مکيه(صحيح argc, حرف *argv[])
{
	صحيح rc = EXIT_FAILURE;

	إذا (argc == 1) {
		لکو("Linux version: %d\n", get_linux_نسخو());
		rc = EXIT_SUCCESS;

	} وإلا إذا (argc == 5) {
		ثابت حرف *oper = argv[1];

		صحيح x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		صحيح kver = get_linux_نسخو();
		صحيح uver = KERNEL_نسخو(x, y, z);

		إذا (strcmp(oper, "==") == 0)
			rc = kver == uver;
		وإلا إذا (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		وإلا إذا (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		وإلا
			errx(EXIT_FAILURE, "unsupported operator");

		إذا (rc)
			لکو("match\n");
		وإلا
			لکو("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} وإلا
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 پروگرام_invocation_short_name);

	أرجع rc;
}
#جيڪڏهن_پورو
