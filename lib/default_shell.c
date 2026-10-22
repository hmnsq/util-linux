/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ಅನ್ತರ್ಭಾವ <paths.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "default_shell.h"

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ul_default_shell(ಪೂರ್ಣಾಂಕ flags, ಸ್ಥಿರಾಂಕ ರಚನೆ passwd *pw)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *shell = ಖಾಲಿ;

	ಆದರೆ (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		ಆದರೆ (shell && *shell)
			ಹಿಂತಿರುಗಿ shell;
	}
	ಆದರೆ (!(flags & UL_SHELL_NOPWD)) {
		ಆದರೆ (!pw)
			pw = getpwuid(getuid());
		ಆದರೆ (pw)
			shell = pw->pw_shell;
		ಆದರೆ (shell && *shell)
			ಹಿಂತಿರುಗಿ shell;
	}

	ಹಿಂತಿರುಗಿ _PATH_BSHELL;
}
