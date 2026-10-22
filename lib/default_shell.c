/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#অন্তর্ভাব <paths.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "default_shell.h"

ধ্রুবক অক্ষর *ul_default_shell(পূর্ণসংখ্যা flags, ধ্রুবক কাঠামো passwd *pw)
{
	ধ্রুবক অক্ষর *shell = ফাঁকা;

	যদি (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		যদি (shell && *shell)
			ফেরত shell;
	}
	যদি (!(flags & UL_SHELL_NOPWD)) {
		যদি (!pw)
			pw = getpwuid(getuid());
		যদি (pw)
			shell = pw->pw_shell;
		যদি (shell && *shell)
			ফেরত shell;
	}

	ফেরত _PATH_BSHELL;
}
