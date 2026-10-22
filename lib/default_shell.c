/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ضمن <paths.h>

#ضمن "c.h"
#ضمن "default_shell.h"

ثابت حرف *ul_default_shell(صحيح flags, ثابت بنية passwd *pw)
{
	ثابت حرف *shell = عدم;

	ܐܢ (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		ܐܢ (shell && *shell)
			ܦܢܐ shell;
	}
	ܐܢ (!(flags & UL_SHELL_NOPWD)) {
		ܐܢ (!pw)
			pw = getpwuid(getuid());
		ܐܢ (pw)
			shell = pw->pw_shell;
		ܐܢ (shell && *shell)
			ܦܢܐ shell;
	}

	ܦܢܐ _PATH_BSHELL;
}
