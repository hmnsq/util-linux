/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#அந்தர்பாவ <paths.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "default_shell.h"

மாறிலி எழுத்து *ul_default_shell(முழு flags, மாறிலி கட்டமைப்பு passwd *pw)
{
	மாறிலி எழுத்து *shell = வெற்று;

	எனில் (!(flags & UL_SHELL_NOENV)) {
		shell = getenv("SHELL");
		எனில் (shell && *shell)
			திரும்பு shell;
	}
	எனில் (!(flags & UL_SHELL_NOPWD)) {
		எனில் (!pw)
			pw = getpwuid(getuid());
		எனில் (pw)
			shell = pw->pw_shell;
		எனில் (shell && *shell)
			திரும்பு shell;
	}

	திரும்பு _PATH_BSHELL;
}
