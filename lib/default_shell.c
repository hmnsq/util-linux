/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ضمن <paths.h>

#ضمن "c.h"
#ضمن "default_shell.h"

ثابت حرف *ul_default_shell(صحيح flags, ثابت بنية passwd *pw)
{
	ثابت حرف *shell = عدم;

	אם (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		אם (shell && *shell)
			החזר shell;
	}
	אם (!(flags & UL_SHELL_NOPWD)) {
		אם (!pw)
			pw = getpwuid(getuid());
		אם (pw)
			shell = pw->pw_shell;
		אם (shell && *shell)
			החזר shell;
	}

	החזר _PATH_BSHELL;
}
