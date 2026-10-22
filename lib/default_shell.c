/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const char *ul_default_shell(целое flags, const struct passwd *pw)
{
	const char *shell = NULL;

	если (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		если (shell && *shell)
			вернуть shell;
	}
	если (!(flags & UL_SHELL_NOPWD)) {
		если (!pw)
			pw = getpwuid(getuid());
		если (pw)
			shell = pw->pw_shell;
		если (shell && *shell)
			вернуть shell;
	}

	вернуть _PATH_BSHELL;
}
