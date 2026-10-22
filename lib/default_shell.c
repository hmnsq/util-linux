/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const char *ul_default_shell(整数 flags, const struct passwd *pw)
{
	const char *shell = NULL;

	如果 (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		如果 (shell && *shell)
			返回 shell;
	}
	如果 (!(flags & UL_SHELL_NOPWD)) {
		如果 (!pw)
			pw = getpwuid(getuid());
		如果 (pw)
			shell = pw->pw_shell;
		如果 (shell && *shell)
			返回 shell;
	}

	返回 _PATH_BSHELL;
}
