/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#અન્તર્ભાવ <paths.h>

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "default_shell.h"

અચલ અક્ષર *ul_default_shell(પૂર્ણાંક flags, અચલ સંરચના passwd *pw)
{
	અચલ અક્ષર *shell = ખાલી;

	જો (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		જો (shell && *shell)
			પાછા shell;
	}
	જો (!(flags & UL_SHELL_NOPWD)) {
		જો (!pw)
			pw = getpwuid(getuid());
		જો (pw)
			shell = pw->pw_shell;
		જો (shell && *shell)
			પાછા shell;
	}

	પાછા _PATH_BSHELL;
}
