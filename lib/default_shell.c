/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const var_na *ul_default_shell(puur_naa_nka flags, const sa_nracanaa passwd *pw)
{
	const var_na *shell = NULL;

	ᱡᱩᱫᱤ (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		ᱡᱩᱫᱤ (shell && *shell)
			ᱨᱩᱣᱟᱹᱲ shell;
	}
	ᱡᱩᱫᱤ (!(flags & UL_SHELL_NOPWD)) {
		ᱡᱩᱫᱤ (!pw)
			pw = getpwuid(getuid());
		ᱡᱩᱫᱤ (pw)
			shell = pw->pw_shell;
		ᱡᱩᱫᱤ (shell && *shell)
			ᱨᱩᱣᱟᱹᱲ shell;
	}

	ᱨᱩᱣᱟᱹᱲ _PATH_BSHELL;
}
