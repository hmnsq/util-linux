/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const var_na *ul_default_shell(puur_naa_nka flags, const sa_nracanaa passwd *pw)
{
	const var_na *shell = NULL;

	ᰀᰧ (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		ᰀᰧ (shell && *shell)
			ᰀᰧᰌ shell;
	}
	ᰀᰧ (!(flags & UL_SHELL_NOPWD)) {
		ᰀᰧ (!pw)
			pw = getpwuid(getuid());
		ᰀᰧ (pw)
			shell = pw->pw_shell;
		ᰀᰧ (shell && *shell)
			ᰀᰧᰌ shell;
	}

	ᰀᰧᰌ _PATH_BSHELL;
}
