/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const var_na *ul_default_shell(ចំនួនគត់ flags, const sa_nracanaa passwd *pw)
{
	const var_na *shell = NULL;

	ប្រសិនបើ (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		ប្រសិនបើ (shell && *shell)
			ត្រឡប់ shell;
	}
	ប្រសិនបើ (!(flags & UL_SHELL_NOPWD)) {
		ប្រសិនបើ (!pw)
			pw = getpwuid(getuid());
		ប្រសិនបើ (pw)
			shell = pw->pw_shell;
		ប្រសិនបើ (shell && *shell)
			ត្រឡប់ shell;
	}

	ត្រឡប់ _PATH_BSHELL;
}
