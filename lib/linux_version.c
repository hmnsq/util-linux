/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#समावेश <stdio.h>
#समावेश <sys/utsname.h>

#समावेश "c.h"
#समावेश "linux_version.h"

पूर्णांक get_linux_version (व्योम)
{
	अटल पूर्णांक kver = -1;
	काष्ठा utsname uts;
	पूर्णांक x = 0, y = 0, z = 0;
	पूर्णांक n;

	अगर (kver != -1)
		वापस kver;
	अगर (uname(&uts))
		वापस kver = 0;

	n = माला_पूछो(uts.release, "%d.%d.%d", &x, &y, &z);
	अगर (n < 1 || n > 3)
		वापस kver = 0;

	वापस kver = KERNEL_VERSION(x, y, z);
}

#अगर_घोषित TEST_PROGRAM_LINUXVERSION
# include <stdlib.h>
पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[])
{
	पूर्णांक rc = निकास_त्रुटि;

	अगर (argc == 1) {
		म_लिखो("Linux version: %d\n", get_linux_version());
		rc = निकास_सफल;

	} अन्यथा अगर (argc == 5) {
		स्थिर अक्षर *oper = argv[1];

		पूर्णांक x = म_से_प(argv[2]),
		    y = म_से_प(argv[3]),
		    z = म_से_प(argv[4]);
		पूर्णांक kver = get_linux_version();
		पूर्णांक uver = KERNEL_VERSION(x, y, z);

		अगर (म_भेद(oper, "==") == 0)
			rc = kver == uver;
		अन्यथा अगर (म_भेद(oper, "<=") == 0)
			rc = kver <= uver;
		अन्यथा अगर (म_भेद(oper, ">=") == 0)
			rc = kver >= uver;
		अन्यथा
			errx(निकास_त्रुटि, "unsupported operator");

		अगर (rc)
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

	वापस rc;
}
#पूर्ण_अगर
