/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const char *ul_default_shell(整数 flags, const struct passwd *pw)
{
	const char *shell = NULL;

	もし (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		もし (shell && *shell)
			戻る shell;
	}
	もし (!(flags & UL_SHELL_NOPWD)) {
		もし (!pw)
			pw = getpwuid(getuid());
		もし (pw)
			shell = pw->pw_shell;
		もし (shell && *shell)
			戻る shell;
	}

	戻る _PATH_BSHELL;
}
