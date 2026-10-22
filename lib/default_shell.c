/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const char *ul_default_shell(ціле flags, const struct passwd *pw)
{
	const char *shell = NULL;

	якщо (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		якщо (shell && *shell)
			повернути shell;
	}
	якщо (!(flags & UL_SHELL_NOPWD)) {
		якщо (!pw)
			pw = getpwuid(getuid());
		якщо (pw)
			shell = pw->pw_shell;
		якщо (shell && *shell)
			повернути shell;
	}

	повернути _PATH_BSHELL;
}
