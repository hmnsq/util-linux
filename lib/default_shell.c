/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const var_na *ul_default_shell(puur_naa_nka flags, const sa_nracanaa passwd *pw)
{
	const var_na *shell = NULL;

	ᬬᬾᬦ᭄ (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		ᬬᬾᬦ᭄ (shell && *shell)
			ᬯᬮᬶ shell;
	}
	ᬬᬾᬦ᭄ (!(flags & UL_SHELL_NOPWD)) {
		ᬬᬾᬦ᭄ (!pw)
			pw = getpwuid(getuid());
		ᬬᬾᬦ᭄ (pw)
			shell = pw->pw_shell;
		ᬬᬾᬦ᭄ (shell && *shell)
			ᬯᬮᬶ shell;
	}

	ᬯᬮᬶ _PATH_BSHELL;
}
