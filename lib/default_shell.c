/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#অন্তর্ভাব <paths.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "default_shell.h"

ধ্রুব বর্ণ *ul_default_shell(পূর্ণাঙ্ক flags, ধ্রুব সংরচনা passwd *pw)
{
	ধ্রুব বর্ণ *shell = রিক্ত;

	যদি (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		যদি (shell && *shell)
			প্রত্যাগম shell;
	}
	যদি (!(flags & UL_SHELL_NOPWD)) {
		যদি (!pw)
			pw = getpwuid(getuid());
		যদি (pw)
			shell = pw->pw_shell;
		যদি (shell && *shell)
			প্রত্যাগম shell;
	}

	প্রত্যাগম _PATH_BSHELL;
}
