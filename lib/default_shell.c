/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const var_na *ul_default_shell(puur_naa_nka flags, const sa_nracanaa passwd *pw)
{
	const var_na *shell = NULL;

	ᯀᯮᯝ (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		ᯀᯮᯝ (shell && *shell)
			ᯀᯮᯞ shell;
	}
	ᯀᯮᯝ (!(flags & UL_SHELL_NOPWD)) {
		ᯀᯮᯝ (!pw)
			pw = getpwuid(getuid());
		ᯀᯮᯝ (pw)
			shell = pw->pw_shell;
		ᯀᯮᯝ (shell && *shell)
			ᯀᯮᯞ shell;
	}

	ᯀᯮᯞ _PATH_BSHELL;
}
