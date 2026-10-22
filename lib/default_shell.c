/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <paths.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "default_shell.h"

ਅਚਲ ਅੱਖਰ *ul_default_shell(ਪੂਰਨ flags, ਅਚਲ ਬਣਤਰ passwd *pw)
{
	ਅਚਲ ਅੱਖਰ *shell = ਨਿਰਾ;

	ਜੇ (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		ਜੇ (shell && *shell)
			ਵਾਪਸ shell;
	}
	ਜੇ (!(flags & UL_SHELL_NOPWD)) {
		ਜੇ (!pw)
			pw = getpwuid(getuid());
		ਜੇ (pw)
			shell = pw->pw_shell;
		ਜੇ (shell && *shell)
			ਵਾਪਸ shell;
	}

	ਵਾਪਸ _PATH_BSHELL;
}
