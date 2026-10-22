/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <sys/utsname.h>

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "linux_version.h"

ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_linux_version (ଶୂନ୍ୟ)
{
	ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kver = -1;
	ସଂରଚନା utsname uts;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x = 0, y = 0, z = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;

	ଯଦି (kver != -1)
		ଫେରନ୍ତୁ kver;
	ଯଦି (uname(&uts))
		ଫେରନ୍ତୁ kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	ଯଦି (n < 1 || n > 3)
		ଫେରନ୍ତୁ kver = 0;

	ଫେରନ୍ତୁ kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = EXIT_FAILURE;

	ଯଦି (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} ଅନ୍ୟଥା ଯଦି (argc == 5) {
		ଅଚଳ ଅକ୍ଷର *oper = argv[1];

		ପୂର୍ଣ୍ଣସଂଖ୍ୟା x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା kver = get_linux_version();
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା uver = KERNEL_VERSION(x, y, z);

		ଯଦି (strcmp(oper, "==") == 0)
			rc = kver == uver;
		ଅନ୍ୟଥା ଯଦି (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		ଅନ୍ୟଥା ଯଦି (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		ଅନ୍ୟଥା
			errx(EXIT_FAILURE, "unsupported operator");

		ଯଦି (rc)
			printf("match\n");
		ଅନ୍ୟଥା
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} ଅନ୍ୟଥା
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	ଫେରନ୍ତୁ rc;
}
#endif
