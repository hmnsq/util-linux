/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ضمن <paths.h>

#ضمن "c.h"
#ضمن "default_shell.h"

ثابت حرف *ul_default_shell(صحيح flags, ثابت بنية passwd *pw)
{
	ثابت حرف *shell = عدم;

	إذا (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		إذا (shell && *shell)
			أرجع shell;
	}
	إذا (!(flags & UL_SHELL_NOPWD)) {
		إذا (!pw)
			pw = getpwuid(getuid());
		إذا (pw)
			shell = pw->pw_shell;
		إذا (shell && *shell)
			أرجع shell;
	}

	أرجع _PATH_BSHELL;
}
