/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#अन्तर्भाव <paths.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "default_shell.h"

अचर वर्ण *ul_default_shell(पूर्णाङ्क flags, अचर संरचना passwd *pw)
{
	अचर वर्ण *shell = रिक्त;

	यदि (!(flags & UL_SHELL_NOENV)) {
		shell = दो_पर्या("SHELL");
		यदि (shell && *shell)
			फर्काऊ shell;
	}
	यदि (!(flags & UL_SHELL_NOPWD)) {
		यदि (!pw)
			pw = getpwuid(getuid());
		यदि (pw)
			shell = pw->pw_shell;
		यदि (shell && *shell)
			फर्काऊ shell;
	}

	फर्काऊ _PATH_BSHELL;
}
