/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#अन्तर्भाव <paths.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "default_shell.h"

स्थिरांक अक्षर *ul_default_shell(पूर्णांक flags, स्थिरांक संरचना passwd *pw)
{
	स्थिरांक अक्षर *shell = रिक्त;

	जर (!(flags & UL_SHELL_NOENV)) {
		shell = दो_पर्या("SHELL");
		जर (shell && *shell)
			परत shell;
	}
	जर (!(flags & UL_SHELL_NOPWD)) {
		जर (!pw)
			pw = getpwuid(getuid());
		जर (pw)
			shell = pw->pw_shell;
		जर (shell && *shell)
			परत shell;
	}

	परत _PATH_BSHELL;
}
