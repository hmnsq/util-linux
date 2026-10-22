/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <sys/utsname.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "linux_version.h"

ಪೂರ್ಣಾಂಕ get_linux_ಆವೃತ್ತಿ (ಶೂನ್ಯ)
{
	ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ kver = -1;
	ರಚನೆ utsname uts;
	ಪೂರ್ಣಾಂಕ x = 0, y = 0, z = 0;
	ಪೂರ್ಣಾಂಕ n;

	ಆದರೆ (kver != -1)
		ಹಿಂತಿರುಗಿ kver;
	ಆದರೆ (uname(&uts))
		ಹಿಂತಿರುಗಿ kver = 0;

	n = ಅಕ್ಷರಸರಣಿ_ಓದು(uts.release, "%d.%d.%d", &x, &y, &z);
	ಆದರೆ (n < 1 || n > 3)
		ಹಿಂತಿರುಗಿ kver = 0;

	ಹಿಂತಿರುಗಿ kver = KERNEL_ಆವೃತ್ತಿ(x, y, z);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_LINUXVERSION
# include <stdlib.h>
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ಪೂರ್ಣಾಂಕ rc = EXIT_FAILURE;

	ಆದರೆ (argc == 1) {
		ಬರೆ("Linux version: %d\n", get_linux_ಆವೃತ್ತಿ());
		rc = EXIT_SUCCESS;

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argc == 5) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *oper = argv[1];

		ಪೂರ್ಣಾಂಕ x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		ಪೂರ್ಣಾಂಕ kver = get_linux_ಆವೃತ್ತಿ();
		ಪೂರ್ಣಾಂಕ uver = KERNEL_ಆವೃತ್ತಿ(x, y, z);

		ಆದರೆ (strcmp(oper, "==") == 0)
			rc = kver == uver;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		ಇಲ್ಲದಿದ್ದರೆ
			errx(EXIT_FAILURE, "unsupported operator");

		ಆದರೆ (rc)
			ಬರೆ("match\n");
		ಇಲ್ಲದಿದ್ದರೆ
			ಬರೆ("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} ಇಲ್ಲದಿದ್ದರೆ
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 ಕಾರ್ಯಕ್ರಮ_invocation_short_name);

	ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ
