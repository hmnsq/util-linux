/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ضمن <paths.h>

#ضمن "c.h"
#ضمن "default_shell.h"

ثابت حرف *ul_default_shell(صحيح flags, ثابت بنية passwd *pw)
{
	ثابت حرف *shell = عدم;

	𐤀𐤌 (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		𐤀𐤌 (shell && *shell)
			أرجع shell;
	}
	𐤀𐤌 (!(flags & UL_SHELL_NOPWD)) {
		𐤀𐤌 (!pw)
			pw = getpwuid(getuid());
		𐤀𐤌 (pw)
			shell = pw->pw_shell;
		𐤀𐤌 (shell && *shell)
			أرجع shell;
	}

	أرجع _PATH_BSHELL;
}
