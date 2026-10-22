/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const var_na *ul_default_shell(ཧྲིལ flags, const sa_nracanaa passwd *pw)
{
	const var_na *shell = NULL;

	གལ་ཏེ (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		གལ་ཏེ (shell && *shell)
			ཕྱིར shell;
	}
	གལ་ཏེ (!(flags & UL_SHELL_NOPWD)) {
		གལ་ཏེ (!pw)
			pw = getpwuid(getuid());
		གལ་ཏེ (pw)
			shell = pw->pw_shell;
		གལ་ཏེ (shell && *shell)
			ཕྱིར shell;
	}

	ཕྱིར _PATH_BSHELL;
}
