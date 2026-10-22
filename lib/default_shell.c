/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const char *ul_default_shell(цяло flags, const struct passwd *pw)
{
	const char *shell = NULL;

	ако (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		ако (shell && *shell)
			върни shell;
	}
	ако (!(flags & UL_SHELL_NOPWD)) {
		ако (!pw)
			pw = getpwuid(getuid());
		ако (pw)
			shell = pw->pw_shell;
		ако (shell && *shell)
			върни shell;
	}

	върни _PATH_BSHELL;
}
