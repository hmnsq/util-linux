/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ಅನ್ತರ್ಭಾವ <paths.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "default_shell.h"

ಧ್ರುವ ವರ್ಣ *ul_default_shell(ಪೂರ್ಣಾಙ್ಕ flags, ಧ್ರುವ ಸಂರಚನಾ passwd *pw)
{
	ಧ್ರುವ ವರ್ಣ *shell = ರಿಕ್ತ;

	ಯದಿ (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		ಯದಿ (shell && *shell)
			ಪ್ರತ್ಯಾಗಮ shell;
	}
	ಯದಿ (!(flags & UL_SHELL_NOPWD)) {
		ಯದಿ (!pw)
			pw = getpwuid(getuid());
		ಯದಿ (pw)
			shell = pw->pw_shell;
		ಯದಿ (shell && *shell)
			ಪ್ರತ್ಯಾಗಮ shell;
	}

	ಪ್ರತ್ಯಾಗಮ _PATH_BSHELL;
}
