/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const var_na *ul_default_shell(puur_naa_nka flags, const sa_nracanaa passwd *pw)
{
	const var_na *shell = NULL;

	ᮚᮦᮔ᮪ (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		ᮚᮦᮔ᮪ (shell && *shell)
			ᮝᮜᮤ shell;
	}
	ᮚᮦᮔ᮪ (!(flags & UL_SHELL_NOPWD)) {
		ᮚᮦᮔ᮪ (!pw)
			pw = getpwuid(getuid());
		ᮚᮦᮔ᮪ (pw)
			shell = pw->pw_shell;
		ᮚᮦᮔ᮪ (shell && *shell)
			ᮝᮜᮤ shell;
	}

	ᮝᮜᮤ _PATH_BSHELL;
}
