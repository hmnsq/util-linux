/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#अन्तर्भाव <paths.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "default_shell.h"

ध्रुव वर्ण *ul_default_shell(पूर्णाङ्क flags, ध्रुव संरचना passwd *pw)
{
	ध्रुव वर्ण *shell = रिक्त;

	सचे (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		सचे (shell && *shell)
			पटिनिवत्त shell;
	}
	सचे (!(flags & UL_SHELL_NOPWD)) {
		सचे (!pw)
			pw = getpwuid(getuid());
		सचे (pw)
			shell = pw->pw_shell;
		सचे (shell && *shell)
			पटिनिवत्त shell;
	}

	पटिनिवत्त _PATH_BSHELL;
}
