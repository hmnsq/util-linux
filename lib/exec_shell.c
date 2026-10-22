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

#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <unistd.h>
#ضمن <sys/types.h>
#ضمن <libgen.h>

#ضمن "nls.h"
#ضمن "c.h"
#ضمن "xalloc.h"

#ضمن "exec_shell.h"
#ضمن "default_shell.h"

فراغ __attribute__((__noreturn__)) exec_shell(فراغ)
{
	ثابت حرف *shell = ul_default_shell(0, عدم);
	حرف *shellc;
	ثابت حرف *shell_basename;
	حرف *arg0;

	shellc = xstrdup(shell);
	shell_basename = basename(shellc);
	xasprintf(&arg0, "-%s", shell_basename);

	execl(shell, arg0, (حرف *)عدم);
	errexec(shell);
}
