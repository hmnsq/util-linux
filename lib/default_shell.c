/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#അന്തര്ഭാവ <paths.h>

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "default_shell.h"

സ്ഥിരം അക്ഷരം *ul_default_shell(പൂർണ്ണസംഖ്യ flags, സ്ഥിരം ഘടന passwd *pw)
{
	സ്ഥിരം അക്ഷരം *shell = ശൂന്യ;

	എങ്കിൽ (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		എങ്കിൽ (shell && *shell)
			തിരികെ shell;
	}
	എങ്കിൽ (!(flags & UL_SHELL_NOPWD)) {
		എങ്കിൽ (!pw)
			pw = getpwuid(getuid());
		എങ്കിൽ (pw)
			shell = pw->pw_shell;
		എങ്കിൽ (shell && *shell)
			തിരികെ shell;
	}

	തിരികെ _PATH_BSHELL;
}
