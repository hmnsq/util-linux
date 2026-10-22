/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const char *ul_default_shell(정수 flags, const struct passwd *pw)
{
	const char *shell = NULL;

	만약 (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		만약 (shell && *shell)
			반환 shell;
	}
	만약 (!(flags & UL_SHELL_NOPWD)) {
		만약 (!pw)
			pw = getpwuid(getuid());
		만약 (pw)
			shell = pw->pw_shell;
		만약 (shell && *shell)
			반환 shell;
	}

	반환 _PATH_BSHELL;
}
