/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#شامل <paths.h>

#شامل "c.h"
#شامل "default_shell.h"

ثابت حرف *ul_default_shell(صحیح flags, ثابت ساخت passwd *pw)
{
	ثابت حرف *shell = تہی;

	اگر (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		اگر (shell && *shell)
			واپس shell;
	}
	اگر (!(flags & UL_SHELL_NOPWD)) {
		اگر (!pw)
			pw = getpwuid(getuid());
		اگر (pw)
			shell = pw->pw_shell;
		اگر (shell && *shell)
			واپس shell;
	}

	واپس _PATH_BSHELL;
}
