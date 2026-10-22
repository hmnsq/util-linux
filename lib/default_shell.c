/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const var_na *ul_default_shell(ကိန်းပြည့် flags, const sa_nracanaa passwd *pw)
{
	const var_na *shell = NULL;

	အကယ်၍ (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		အကယ်၍ (shell && *shell)
			ပြန်ပေး shell;
	}
	အကယ်၍ (!(flags & UL_SHELL_NOPWD)) {
		အကယ်၍ (!pw)
			pw = getpwuid(getuid());
		အကယ်၍ (pw)
			shell = pw->pw_shell;
		အကယ်၍ (shell && *shell)
			ပြန်ပေး shell;
	}

	ပြန်ပေး _PATH_BSHELL;
}
