/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <sys/utsname.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "linux_version.h"

पूर्णांक get_linux_version (शून्य)
{
	स्थिर पूर्णांक kver = -1;
	संरचना utsname uts;
	पूर्णांक x = 0, y = 0, z = 0;
	पूर्णांक n;

	जर (kver != -1)
		परत kver;
	जर (uname(&uts))
		परत kver = 0;

	n = माला_पूछो(uts.release, "%d.%d.%d", &x, &y, &z);
	जर (n < 1 || n > 3)
		परत kver = 0;

	परत kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
पूर्णांक main(पूर्णांक argc, अक्षर *argv[])
{
	पूर्णांक rc = निकास_त्रुटि;

	जर (argc == 1) {
		म_लिखो("Linux version: %d\n", get_linux_version());
		rc = निकास_सफल;

	} अन्यथा जर (argc == 5) {
		स्थिरांक अक्षर *oper = argv[1];

		पूर्णांक x = म_से_प(argv[2]),
		    y = म_से_प(argv[3]),
		    z = म_से_प(argv[4]);
		पूर्णांक kver = get_linux_version();
		पूर्णांक uver = KERNEL_VERSION(x, y, z);

		जर (म_भेद(oper, "==") == 0)
			rc = kver == uver;
		अन्यथा जर (म_भेद(oper, "<=") == 0)
			rc = kver <= uver;
		अन्यथा जर (म_भेद(oper, ">=") == 0)
			rc = kver >= uver;
		अन्यथा
			errx(निकास_त्रुटि, "unsupported operator");

		जर (rc)
			म_लिखो("match\n");
		अन्यथा
			म_लिखो("not-match [%d %s %d, x.y.z: %d.%d.%d]\n",
					kver, oper, uver, x, y, z);

		rc = rc ? निकास_सफल : निकास_त्रुटि;

	} अन्यथा
		 ख_लिखो(मानक_त्रुटि, "Usage:\n"
				 "   %s [<oper> <x> <y> <z>]\n"
				 "supported operators:\n"
				 "   ==, <=, >=\n",
				 program_invocation_short_name);

	परत rc;
}
#endif
