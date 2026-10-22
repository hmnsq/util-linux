/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <paths.h>

#include "c.h"
#include "default_shell.h"

const var_na *ul_default_shell(ຈຳນວນເຕັມ flags, const sa_nracanaa passwd *pw)
{
	const var_na *shell = NULL;

	ຖ້າ (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		ຖ້າ (shell && *shell)
			ສົ່ງຄືນ shell;
	}
	ຖ້າ (!(flags & UL_SHELL_NOPWD)) {
		ຖ້າ (!pw)
			pw = getpwuid(getuid());
		ຖ້າ (pw)
			shell = pw->pw_shell;
		ຖ້າ (shell && *shell)
			ສົ່ງຄືນ shell;
	}

	ສົ່ງຄືນ _PATH_BSHELL;
}
