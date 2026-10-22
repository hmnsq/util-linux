/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <sys/utsname.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "linux_version.h"

పూర్ణాంకం get_linux_సంస్కరణ (శూన్య)
{
	స్థిర పూర్ణాంకం kver = -1;
	నిర్మాణం utsname uts;
	పూర్ణాంకం x = 0, y = 0, z = 0;
	పూర్ణాంకం n;

	ఒకవేళ (kver != -1)
		తిరిగి kver;
	ఒకవేళ (uname(&uts))
		తిరిగి kver = 0;

	n = అక్షరశ్రేణి_చదువు(uts.release, "%d.%d.%d", &x, &y, &z);
	ఒకవేళ (n < 1 || n > 3)
		తిరిగి kver = 0;

	తిరిగి kver = KERNEL_సంస్కరణ(x, y, z);
}

#నిర్వచితమైతే TEST_కార్యక్రమం_LINUXVERSION
# include <stdlib.h>
పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[])
{
	పూర్ణాంకం rc = EXIT_FAILURE;

	ఒకవేళ (argc == 1) {
		రాయి("Linux version: %d\n", get_linux_సంస్కరణ());
		rc = EXIT_SUCCESS;

	} లేకపోతే ఒకవేళ (argc == 5) {
		స్థిరరాశి అక్షరం *oper = argv[1];

		పూర్ణాంకం x = atoi(argv[2]),
		    y = atoi(argv[3]),
		    z = atoi(argv[4]);
		పూర్ణాంకం kver = get_linux_సంస్కరణ();
		పూర్ణాంకం uver = KERNEL_సంస్కరణ(x, y, z);

		ఒకవేళ (strcmp(oper, "==") == 0)
			rc = kver == uver;
		లేకపోతే ఒకవేళ (strcmp(oper, "<=") == 0)
			rc = kver <= uver;
		లేకపోతే ఒకవేళ (strcmp(oper, ">=") == 0)
			rc = kver >= uver;
		లేకపోతే
			errx(EXIT_FAILURE, "unsupported operator");

		ఒకవేళ (rc)
			రాయి("match\n");
		లేకపోతే
			రాయి("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? EXIT_SUCCESS : EXIT_FAILURE;

	} లేకపోతే
		 fprintf(stderr, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 కార్యక్రమం_invocation_short_name);

	తిరిగి rc;
}
#అయితే_ముగింపు
