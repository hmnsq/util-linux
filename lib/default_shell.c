/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const var_na *ul_default_shell(පූර්ණ_සංඛ්‍යා flags, const sa_nracanaa passwd *pw)
{
	const var_na *shell = NULL;

	නම් (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		නම් (shell && *shell)
			ආපසු shell;
	}
	නම් (!(flags & UL_SHELL_NOPWD)) {
		නම් (!pw)
			pw = getpwuid(getuid());
		නම් (pw)
			shell = pw->pw_shell;
		නම් (shell && *shell)
			ආපසු shell;
	}

	ආපසු _PATH_BSHELL;
}
