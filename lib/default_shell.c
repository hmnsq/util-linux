/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const var_na *ul_default_shell(puur_naa_nka flags, const sa_nracanaa passwd *pw)
{
	const var_na *shell = NULL;

	ᤀᤡ᤺ (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		ᤀᤡ᤺ (shell && *shell)
			ᤀᤡᤴ shell;
	}
	ᤀᤡ᤺ (!(flags & UL_SHELL_NOPWD)) {
		ᤀᤡ᤺ (!pw)
			pw = getpwuid(getuid());
		ᤀᤡ᤺ (pw)
			shell = pw->pw_shell;
		ᤀᤡ᤺ (shell && *shell)
			ᤀᤡᤴ shell;
	}

	ᤀᤡᤴ _PATH_BSHELL;
}
