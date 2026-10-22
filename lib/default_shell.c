/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#شامل <paths.h>

#شامل "c.h"
#شامل "default_shell.h"

ثابت نویسه *ul_default_shell(صحیح flags, ثابت ساختار passwd *pw)
{
	ثابت نویسه *shell = پوچ;

	اگر (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		اگر (shell && *shell)
			بازگشت shell;
	}
	اگر (!(flags & UL_SHELL_NOPWD)) {
		اگر (!pw)
			pw = getpwuid(getuid());
		اگر (pw)
			shell = pw->pw_shell;
		اگر (shell && *shell)
			بازگشت shell;
	}

	بازگشت _PATH_BSHELL;
}
