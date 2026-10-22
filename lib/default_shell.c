/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const char *ul_default_shell(số_nguyên flags, const struct passwd *pw)
{
	const char *shell = NULL;

	nếu (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		nếu (shell && *shell)
			trả_về shell;
	}
	nếu (!(flags & UL_SHELL_NOPWD)) {
		nếu (!pw)
			pw = getpwuid(getuid());
		nếu (pw)
			shell = pw->pw_shell;
		nếu (shell && *shell)
			trả_về shell;
	}

	trả_về _PATH_BSHELL;
}
