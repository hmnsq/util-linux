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

#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <libgen.h>

#અન્તર્ભાવ "nls.h"
#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "xalloc.h"

#અન્તર્ભાવ "exec_shell.h"
#અન્તર્ભાવ "default_shell.h"

શૂન્ય __attribute__((__noreturn__)) exec_shell(શૂન્ય)
{
	અચલ અક્ષર *shell = ul_default_shell(0, ખાલી);
	અક્ષર *shellc;
	અચલ અક્ષર *shell_basename;
	અક્ષર *arg0;

	shellc = xstrdup(shell);
	shell_basename = basename(shellc);
	xasprintf(&arg0, "-%s", shell_basename);

	execl(shell, arg0, (અક્ષર *)ખાલી);
	errexec(shell);
}
