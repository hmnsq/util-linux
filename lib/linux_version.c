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

	यदि (kver != -1)
		प्रत्यागम kver;
	यदि (uname(&uts))
		प्रत्यागम kver = 0;

	n = माला_पूछो(uts.release, "%d.%d.%d", &x, &y, &z);
	यदि (n < 1 || n > 3)
		प्रत्यागम kver = 0;

	प्रत्यागम kver = KERNEL_VERSION(x, y, z);
}

#ifdef TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	पूर्णाङ्क rc = निकास_त्रुटि;

	यदि (argc == 1) {
		म_लिखो("Linux version: %d\n", get_linux_version());
		rc = निकास_सफल;

	} अन्यथा यदि (argc == 5) {
		ध्रुव वर्ण *oper = argv[1];

		पूर्णाङ्क x = म_से_प(argv[2]),
		    y = म_से_प(argv[3]),
		    z = म_से_प(argv[4]);
		पूर्णाङ्क kver = get_linux_version();
		पूर्णाङ्क uver = KERNEL_VERSION(x, y, z);

		यदि (म_भेद(oper, "==") == 0)
			rc = kver == uver;
		अन्यथा यदि (म_भेद(oper, "<=") == 0)
			rc = kver <= uver;
		अन्यथा यदि (म_भेद(oper, ">=") == 0)
			rc = kver >= uver;
		अन्यथा
			errx(निकास_त्रुटि, "unsupported operator");

		यदि (rc)
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

	प्रत्यागम rc;
}
#endif
