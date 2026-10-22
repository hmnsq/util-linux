/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#شامل <stdio.h>
#شامل <sys/utsname.h>

#شامل "c.h"
#شامل "linux_version.h"

صحیح get_linux_نسخہ (خالی)
{
	ساکن صحیح kver = -1;
	ساخت utsname uts;
	صحیح x = 0, y = 0, z = 0;
	صحیح n;

	اگر (kver != -1)
		واپس kver;
	اگر (uname(&uts))
		واپس kver = 0;

	n = لڑی_سے_پڑھو(uts.release, "%d.%d.%d", &x, &y, &z);
	اگر (n < 1 || n > 3)
		واپس kver = 0;

	واپس kver = KERNEL_نسخہ(x, y, z);
}

#اگر_تعریف_شدہ TEST_پروگرام_LINUXVERSION
# include <stdlib.h>
صحیح مین(صحیح argc, حرف *argv[])
{
	صحیح rc = EXIT_FAILURE;

	اگر (argc == 1) {
		لکھو("Linux version: %d\n", get_linux_نسخہ());
		rc = EXIT_SUCCESS;

	} ورنہ اگر (argc == 5) {
		ثابت حرف *oper = argv[1];

		صحیح x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		صحیح kver = get_linux_نسخہ();
		صحیح uver = KERNEL_نسخہ(x, y, z);

		اگر (strcmp(oper, "==") == 0)
			rc = kver == uver;
		ورنہ اگر (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		ورنہ اگر (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		ورنہ
			errx(EXIT_FAILURE, "unsupported operator");

		اگر (rc)
			لکھو("match\n");
		ورنہ
			لکھو("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} ورنہ
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 پروگرام_invocation_short_name);

	واپس rc;
}
#اگر_ختم
