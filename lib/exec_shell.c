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

#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <unistd.h>
#समावेश <sys/types.h>
#समावेश <libgen.h>

#समावेश "nls.h"
#समावेश "c.h"
#समावेश "xalloc.h"

#समावेश "exec_shell.h"
#समावेश "default_shell.h"

व्योम __attribute__((__noreturn__)) exec_shell(व्योम)
{
	स्थिर अक्षर *shell = ul_default_shell(0, शून्य);
	अक्षर *shellc;
	स्थिर अक्षर *shell_basename;
	अक्षर *arg0;

	shellc = xstrdup(shell);
	shell_basename = basename(shellc);
	xasprintf(&arg0, "-%s", shell_basename);

	execl(shell, arg0, (अक्षर *)शून्य);
	errexec(shell);
}
