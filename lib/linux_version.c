/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#include <stdio.h>
#include <sys/utsname.h>

#include "c.h"
#include "linux_version.h"

puur_naa_nka get_linux_version (void)
{
	static puur_naa_nka kver = -1;
	sa_nracanaa utsname uts;
	puur_naa_nka x = 0, y = 0, z = 0;
	puur_naa_nka n;

	ᱡᱩᱫᱤ (kver != -1)
		ᱨᱩᱣᱟᱹᱲ kver;
	ᱡᱩᱫᱤ (uname(&uts))
		ᱨᱩᱣᱟᱹᱲ kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	ᱡᱩᱫᱤ (n < 1 || n > 3)
		ᱨᱩᱣᱟᱹᱲ kver = 0;

	ᱨᱩᱣᱟᱹᱲ kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
puur_naa_nka main(puur_naa_nka argc, var_na *argv[])
{
	puur_naa_nka rc = EXIT_FAILURE;

	ᱡᱩᱫᱤ (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} ᱵᱟᱝ ᱡᱩᱫᱤ (argc == 5) {
		const var_na *oper = argv[1];

		puur_naa_nka x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		puur_naa_nka kver = get_linux_version();
		puur_naa_nka uver = KERNEL_VERSION(x, y, z);

		ᱡᱩᱫᱤ (strcmp(oper, "==") == 0)
			rc = kver == uver;
		ᱵᱟᱝ ᱡᱩᱫᱤ (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		ᱵᱟᱝ ᱡᱩᱫᱤ (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		ᱵᱟᱝ
			errx(EXIT_FAILURE, "unsupported operator");

		ᱡᱩᱫᱤ (rc)
			printf("match\n");
		ᱵᱟᱝ
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} ᱵᱟᱝ
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	ᱨᱩᱣᱟᱹᱲ rc;
}
#endif
