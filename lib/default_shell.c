/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ଅନ୍ତର୍ଭାଵ <paths.h>

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "default_shell.h"

ଅଚଳ ଅକ୍ଷର *ul_default_shell(ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ଅଚଳ ସଂରଚନା passwd *pw)
{
	ଅଚଳ ଅକ୍ଷର *shell = ଖାଲି;

	ଯଦି (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		ଯଦି (shell && *shell)
			ଫେରନ୍ତୁ shell;
	}
	ଯଦି (!(flags & UL_SHELL_NOPWD)) {
		ଯଦି (!pw)
			pw = getpwuid(getuid());
		ଯଦି (pw)
			shell = pw->pw_shell;
		ଯଦି (shell && *shell)
			ଫେରନ୍ତୁ shell;
	}

	ଫେରନ୍ତୁ _PATH_BSHELL;
}
