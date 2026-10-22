/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <sys/utsname.h>

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "linux_version.h"

പൂർണ്ണസംഖ്യ get_linux_version (ശൂന്യം)
{
	സ്ഥിതം പൂർണ്ണസംഖ്യ kver = -1;
	ഘടന utsname uts;
	പൂർണ്ണസംഖ്യ x = 0, y = 0, z = 0;
	പൂർണ്ണസംഖ്യ n;

	എങ്കിൽ (kver != -1)
		തിരികെ kver;
	എങ്കിൽ (uname(&uts))
		തിരികെ kver = 0;

	n = sscanf(uts.release, "%d.%d.%d", &x, &y, &z);
	എങ്കിൽ (n < 1 || n > 3)
		തിരികെ kver = 0;

	തിരികെ kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	പൂർണ്ണസംഖ്യ rc = EXIT_FAILURE;

	എങ്കിൽ (argc == 1) {
		printf("Linux version: %d\n", get_linux_version());
		rc = EXIT_SUCCESS;

	} അല്ലെങ്കിൽ എങ്കിൽ (argc == 5) {
		സ്ഥിരം അക്ഷരം *oper = argv[1];

		പൂർണ്ണസംഖ്യ x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		പൂർണ്ണസംഖ്യ kver = get_linux_version();
		പൂർണ്ണസംഖ്യ uver = KERNEL_VERSION(x, y, z);

		എങ്കിൽ (strcmp(oper, "==") == 0)
			rc = kver == uver;
		അല്ലെങ്കിൽ എങ്കിൽ (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		അല്ലെങ്കിൽ എങ്കിൽ (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		അല്ലെങ്കിൽ
			errx(EXIT_FAILURE, "unsupported operator");

		എങ്കിൽ (rc)
			printf("match\n");
		അല്ലെങ്കിൽ
			printf("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} അല്ലെങ്കിൽ
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	തിരികെ rc;
}
#endif
