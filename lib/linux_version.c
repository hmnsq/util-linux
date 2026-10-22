/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <sys/utsname.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "linux_version.h"

முழு get_linux_version (வெறுமை)
{
	நிலையான முழு kver = -1;
	கட்டமைப்பு utsname uts;
	முழு x = 0, y = 0, z = 0;
	முழு n;

	எனில் (kver != -1)
		திரும்பு kver;
	எனில் (uname(&uts))
		திரும்பு kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	எனில் (n < 1 || n > 3)
		திரும்பு kver = 0;

	திரும்பு kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
முழு முதன்மை(முழு argc, எழுத்து *argv[])
{
	முழு rc = EXIT_FAILURE;

	எனில் (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} இல்லையெனில் எனில் (argc == 5) {
		மாறிலி எழுத்து *oper = argv[1];

		முழு x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		முழு kver = get_linux_version();
		முழு uver = KERNEL_VERSION(x, y, z);

		எனில் (strcmp(oper, "==") == 0)
			rc = kver == uver;
		இல்லையெனில் எனில் (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		இல்லையெனில் எனில் (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		இல்லையெனில்
			errx(EXIT_FAILURE, "unsupported operator");

		எனில் (rc)
			printf("match\n");
		இல்லையெனில்
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} இல்லையெனில்
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	திரும்பு rc;
}
#endif
