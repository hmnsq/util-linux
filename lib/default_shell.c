/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#अन्तर्भाव <paths.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "default_shell.h"

ध्रुव वर्ण *ul_default_shell(पूर्णाङ्क flags, ध्रुव संरचना passwd *pw)
{
	ध्रुव वर्ण *shell = रिक्त;

	यदि (!(flags & UL_SHELL_NOENV)) {
		shell = दो_पर्या("SHELL");
		यदि (shell && *shell)
			प्रत्यागम shell;
	}
	यदि (!(flags & UL_SHELL_NOPWD)) {
		यदि (!pw)
			pw = getpwuid(getuid());
		यदि (pw)
			shell = pw->pw_shell;
		यदि (shell && *shell)
			प्रत्यागम shell;
	}

	प्रत्यागम _PATH_BSHELL;
}
