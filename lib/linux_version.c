/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <sys/utsname.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "linux_version.h"

पूर्णाङ्क get_linux_version (शून्य)
{
	स्थिर पूर्णाङ्क kver = -1;
	संरचना utsname uts;
	पूर्णाङ्क x = 0, y = 0, z = 0;
	पूर्णाङ्क n;

	जइ (kver != -1)
		प्रत्यागम kver;
	जइ (uname(&uts))
		प्रत्यागम kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	जइ (n < 1 || n > 3)
		प्रत्यागम kver = 0;

	प्रत्यागम kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	पूर्णाङ्क rc = EXIT_FAILURE;

	जइ (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} अण्णहा जइ (argc == 5) {
		ध्रुव वर्ण *oper = argv[1];

		पूर्णाङ्क x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		पूर्णाङ्क kver = get_linux_version();
		पूर्णाङ्क uver = KERNEL_VERSION(x, y, z);

		जइ (strcmp(oper, "==") == 0)
			rc = kver == uver;
		अण्णहा जइ (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		अण्णहा जइ (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		अण्णहा
			errx(EXIT_FAILURE, "unsupported operator");

		जइ (rc)
			printf("match\n");
		अण्णहा
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} अण्णहा
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	प्रत्यागम rc;
}
#endif
