/*
 * exec_shell() - launch a shell, else exit!
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://gnu.org/licenses/>.
 */

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <libgen.h>

#अन्तर्भाव "nls.h"
#अन्तर्भाव "c.h"
#अन्तर्भाव "xalloc.h"

#अन्तर्भाव "exec_shell.h"
#अन्तर्भाव "default_shell.h"

शून्य __attribute__((__noreturn__)) exec_shell(शून्य)
{
	ध्रुव वर्ण *shell = ul_default_shell(0, रिक्त);
	वर्ण *shellc;
	ध्रुव वर्ण *shell_basename;
	वर्ण *arg0;

	shellc = xstrdup(shell);
	shell_basename = basename(shellc);
	xasprintf(&arg0, "-%s", shell_basename);

	execl(shell, arg0, (वर्ण *)रिक्त);
	errexec(shell);
}
