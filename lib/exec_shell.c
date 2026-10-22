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

#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <libgen.h>

#അന്തര്ഭാവ "nls.h"
#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "xalloc.h"

#അന്തര്ഭാവ "exec_shell.h"
#അന്തര്ഭാവ "default_shell.h"

ശൂന്യം __attribute__((__noreturn__)) exec_shell(ശൂന്യം)
{
	സ്ഥിരം അക്ഷരം *shell = ul_default_shell(0, ശൂന്യ);
	അക്ഷരം *shellc;
	സ്ഥിരം അക്ഷരം *shell_basename;
	അക്ഷരം *arg0;

	shellc = xstrdup(shell);
	shell_basename = basename(shellc);
	xasprintf(&arg0, "-%s", shell_basename);

	execl(shell, arg0, (അക്ഷരം *)ശൂന്യ);
	errexec(shell);
}
