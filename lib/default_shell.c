/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const var_na *ul_default_shell(ཧྲིལ flags, const sa_nracanaa passwd *pw)
{
	const var_na *shell = NULL;

	གལ་སྲིད (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		གལ་སྲིད (shell && *shell)
			ལོག shell;
	}
	གལ་སྲིད (!(flags & UL_SHELL_NOPWD)) {
		གལ་སྲིད (!pw)
			pw = getpwuid(getuid());
		གལ་སྲིད (pw)
			shell = pw->pw_shell;
		གལ་སྲིད (shell && *shell)
			ལོག shell;
	}

	ལོག _PATH_BSHELL;
}
