/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <sys/utsname.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "linux_version.h"

ಪೂರ್ಣಾಙ್ಕ get_linux_version (ಶೂನ್ಯ)
{
	ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kver = -1;
	ಸಂರಚನಾ utsname uts;
	ಪೂರ್ಣಾಙ್ಕ x = 0, y = 0, z = 0;
	ಪೂರ್ಣಾಙ್ಕ n;

	ಯದಿ (kver != -1)
		ಪ್ರತ್ಯಾಗಮ kver;
	ಯದಿ (uname(&uts))
		ಪ್ರತ್ಯಾಗಮ kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	ಯದಿ (n < 1 || n > 3)
		ಪ್ರತ್ಯಾಗಮ kver = 0;

	ಪ್ರತ್ಯಾಗಮ kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ಪೂರ್ಣಾಙ್ಕ rc = EXIT_FAILURE;

	ಯದಿ (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} ಅನ್ಯಥಾ ಯದಿ (argc == 5) {
		ಧ್ರುವ ವರ್ಣ *oper = argv[1];

		ಪೂರ್ಣಾಙ್ಕ x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		ಪೂರ್ಣಾಙ್ಕ kver = get_linux_version();
		ಪೂರ್ಣಾಙ್ಕ uver = KERNEL_VERSION(x, y, z);

		ಯದಿ (strcmp(oper, "==") == 0)
			rc = kver == uver;
		ಅನ್ಯಥಾ ಯದಿ (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		ಅನ್ಯಥಾ ಯದಿ (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		ಅನ್ಯಥಾ
			errx(EXIT_FAILURE, "unsupported operator");

		ಯದಿ (rc)
			printf("match\n");
		ಅನ್ಯಥಾ
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} ಅನ್ಯಥಾ
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	ಪ್ರತ್ಯಾಗಮ rc;
}
#endif
