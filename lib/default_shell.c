/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#অন্তর্ভাৱ <paths.h>

#অন্তর্ভাৱ "c.h"
#অন্তর্ভাৱ "default_shell.h"

ধ্রুৱ ৱর্ণ *ul_default_shell(পূর্ণাঙ্ক flags, ধ্রুৱ সংরচনা passwd *pw)
{
	ধ্রুৱ ৱর্ণ *shell = রিক্ত;

	যদি (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		যদি (shell && *shell)
			ঘূৰাই shell;
	}
	যদি (!(flags & UL_SHELL_NOPWD)) {
		যদি (!pw)
			pw = getpwuid(getuid());
		যদি (pw)
			shell = pw->pw_shell;
		যদি (shell && *shell)
			ঘূৰাই shell;
	}

	ঘূৰাই _PATH_BSHELL;
}
