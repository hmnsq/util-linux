/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <sys/utsname.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "linux_version.h"

पूर्णाङ्क get_linux_संस्करणम् (शून्य)
{
	स्थिर पूर्णाङ्क kver = -1;
	संरचना utsname uts;
	पूर्णाङ्क x = 0, y = 0, z = 0;
	पूर्णाङ्क n;

	यदि (kver != -1)
		प्रत्यागम kver;
	यदि (uname(&uts))
		प्रत्यागम kver = 0;

	n = वाक्यात्_पठ(uts.release, "%d.%d.%d", &x, &y, &z);
	यदि (n < 1 || n > 3)
		प्रत्यागम kver = 0;

	प्रत्यागम kver = KERNEL_संस्करणम्(x, y, z);
}

#यदि_परिभाषितम् TEST_क्रमादेशः_LINUXVERSION
# include <stdlib.h>
पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण *argv[])
{
	पूर्णाङ्क rc = EXIT_FAILURE;

	यदि (argc == 1) {
		प्रारूप_लिख("Linux version: %d\n", get_linux_संस्करणम्());
		rc = EXIT_SUCCESS;

	} अन्यथा यदि (argc == 5) {
		ध्रुव वर्ण *oper = argv[1];

		पूर्णाङ्क x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		पूर्णाङ्क kver = get_linux_संस्करणम्();
		पूर्णाङ्क uver = KERNEL_संस्करणम्(x, y, z);

		यदि (वाक्यं_तुलय(oper, "==") == 0)
			rc = kver == uver;
		अन्यथा यदि (वाक्यं_तुलय(oper, "<=") == 0)
			rc = kver <= uver;
		अन्यथा यदि (वाक्यं_तुलय(oper, ">=") == 0)
			rc = kver >= uver;
		अन्यथा
			errx(EXIT_FAILURE, "unsupported operator");

		यदि (rc)
			प्रारूप_लिख("match\n");
		अन्यथा
			प्रारूप_लिख("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} अन्यथा
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 क्रमादेशः_invocation_short_name);

	प्रत्यागम rc;
}
#यदि_अन्तः
