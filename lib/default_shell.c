/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const var_na *ul_default_shell(จำนวนเต็ม flags, const sa_nracanaa passwd *pw)
{
	const var_na *shell = NULL;

	ถ้า (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		ถ้า (shell && *shell)
			คืน shell;
	}
	ถ้า (!(flags & UL_SHELL_NOPWD)) {
		ถ้า (!pw)
			pw = getpwuid(getuid());
		ถ้า (pw)
			shell = pw->pw_shell;
		ถ้า (shell && *shell)
			คืน shell;
	}

	คืน _PATH_BSHELL;
}
