/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#समावेश <paths.h>

#समावेश "c.h"
#समावेश "default_shell.h"

स्थिर अक्षर *ul_default_shell(पूर्णांक flags, स्थिर काष्ठा passwd *pw)
{
	स्थिर अक्षर *shell = शून्य;

	अगर (!(flags & UL_SHELL_NOENV)) {
		shell = दो_पर्या("SHELL");
		अगर (shell && *shell)
			वापस shell;
	}
	अगर (!(flags & UL_SHELL_NOPWD)) {
		अगर (!pw)
			pw = getpwuid(getuid());
		अगर (pw)
			shell = pw->pw_shell;
		अगर (shell && *shell)
			वापस shell;
	}

	वापस _PATH_BSHELL;
}
