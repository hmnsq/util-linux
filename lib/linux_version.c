/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/utsname.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "linux_version.h"

ਪੂਰਨ get_linux_ਸੰਸਕਰਣ (ਖਾਲੀ)
{
	ਸਥਿਰ ਪੂਰਨ kver = -1;
	ਬਣਤਰ utsname uts;
	ਪੂਰਨ x = 0, y = 0, z = 0;
	ਪੂਰਨ n;

	ਜੇ (kver != -1)
		ਵਾਪਸ kver;
	ਜੇ (uname(&uts))
		ਵਾਪਸ kver = 0;

	n = ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(uts.release, "%d.%d.%d", &x, &y, &z);
	ਜੇ (n < 1 || n > 3)
		ਵਾਪਸ kver = 0;

	ਵਾਪਸ kver = KERNEL_ਸੰਸਕਰਣ(x, y, z);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_LINUXVERSION
# include <stdlib.h>
ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਪੂਰਨ rc = EXIT_FAILURE;

	ਜੇ (argc == 1) {
		ਲਿਖੋ("Linux version: %d\n", get_linux_ਸੰਸਕਰਣ());
		rc = EXIT_SUCCESS;

	} ਨਹੀਂਤਾਂ ਜੇ (argc == 5) {
		ਅਚਲ ਅੱਖਰ *oper = argv[1];

		ਪੂਰਨ x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		ਪੂਰਨ kver = get_linux_ਸੰਸਕਰਣ();
		ਪੂਰਨ uver = KERNEL_ਸੰਸਕਰਣ(x, y, z);

		ਜੇ (strcmp(oper, "==") == 0)
			rc = kver == uver;
		ਨਹੀਂਤਾਂ ਜੇ (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		ਨਹੀਂਤਾਂ ਜੇ (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		ਨਹੀਂਤਾਂ
			errx(EXIT_FAILURE, "unsupported operator");

		ਜੇ (rc)
			ਲਿਖੋ("match\n");
		ਨਹੀਂਤਾਂ
			ਲਿਖੋ("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} ਨਹੀਂਤਾਂ
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 ਪ੍ਰੋਗਰਾਮ_invocation_short_name);

	ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ
