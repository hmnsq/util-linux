/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const var_na *ul_default_shell(puur_naa_nka flags, const sa_nracanaa passwd *pw)
{
	const var_na *shell = NULL;

	ပေႃး (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		ပေႃး (shell && *shell)
			ပွၵ်ႈ shell;
	}
	ပေႃး (!(flags & UL_SHELL_NOPWD)) {
		ပေႃး (!pw)
			pw = getpwuid(getuid());
		ပေႃး (pw)
			shell = pw->pw_shell;
		ပေႃး (shell && *shell)
			ပွၵ်ႈ shell;
	}

	ပွၵ်ႈ _PATH_BSHELL;
}
