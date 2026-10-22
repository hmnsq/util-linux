/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <sys/utsname.h>

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "linux_version.h"

પૂર્ણાંક get_linux_version (શૂન્ય)
{
	સ્થિર પૂર્ણાંક kver = -1;
	સંરચના utsname uts;
	પૂર્ણાંક x = 0, y = 0, z = 0;
	પૂર્ણાંક n;

	જો (kver != -1)
		પાછા kver;
	જો (uname(&uts))
		પાછા kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	જો (n < 1 || n > 3)
		પાછા kver = 0;

	પાછા kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[])
{
	પૂર્ણાંક rc = EXIT_FAILURE;

	જો (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} નહીંતર જો (argc == 5) {
		અચલ અક્ષર *oper = argv[1];

		પૂર્ણાંક x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		પૂર્ણાંક kver = get_linux_version();
		પૂર્ણાંક uver = KERNEL_VERSION(x, y, z);

		જો (strcmp(oper, "==") == 0)
			rc = kver == uver;
		નહીંતર જો (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		નહીંતર જો (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		નહીંતર
			errx(EXIT_FAILURE, "unsupported operator");

		જો (rc)
			printf("match\n");
		નહીંતર
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} નહીંતર
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	પાછા rc;
}
#endif
