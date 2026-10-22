/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const var_na *ul_default_shell(puur_naa_nka flags, const sa_nracanaa passwd *pw)
{
	const var_na *shell = NULL;

	ᨀᨘ (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		ᨀᨘ (shell && *shell)
			ᨀᨘᨒ shell;
	}
	ᨀᨘ (!(flags & UL_SHELL_NOPWD)) {
		ᨀᨘ (!pw)
			pw = getpwuid(getuid());
		ᨀᨘ (pw)
			shell = pw->pw_shell;
		ᨀᨘ (shell && *shell)
			ᨀᨘᨒ shell;
	}

	ᨀᨘᨒ _PATH_BSHELL;
}
