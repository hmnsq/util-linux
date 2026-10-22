/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#అన్తర్భావ <paths.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "default_shell.h"

స్థిరరాశి అక్షరం *ul_default_shell(పూర్ణాంకం flags, స్థిరరాశి నిర్మాణం passwd *pw)
{
	స్థిరరాశి అక్షరం *shell = ఖాళీ;

	ఒకవేళ (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		ఒకవేళ (shell && *shell)
			తిరిగి shell;
	}
	ఒకవేళ (!(flags & UL_SHELL_NOPWD)) {
		ఒకవేళ (!pw)
			pw = getpwuid(getuid());
		ఒకవేళ (pw)
			shell = pw->pw_shell;
		ఒకవేళ (shell && *shell)
			తిరిగి shell;
	}

	తిరిగి _PATH_BSHELL;
}
