/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const var_na *ul_default_shell(puur_naa_nka flags, const sa_nracanaa passwd *pw)
{
	const var_na *shell = NULL;

	ꦪꦼꦤ꧀ (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		ꦪꦼꦤ꧀ (shell && *shell)
			ꦮꦭꦶ shell;
	}
	ꦪꦼꦤ꧀ (!(flags & UL_SHELL_NOPWD)) {
		ꦪꦼꦤ꧀ (!pw)
			pw = getpwuid(getuid());
		ꦪꦼꦤ꧀ (pw)
			shell = pw->pw_shell;
		ꦪꦼꦤ꧀ (shell && *shell)
			ꦮꦭꦶ shell;
	}

	ꦮꦭꦶ _PATH_BSHELL;
}
