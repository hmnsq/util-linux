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

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <libgen.h>

#ಅನ್ತರ್ಭಾವ "nls.h"
#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "xalloc.h"

#ಅನ್ತರ್ಭಾವ "exec_shell.h"
#ಅನ್ತರ್ಭಾವ "default_shell.h"

ಶೂನ್ಯ __attribute__((__noreturn__)) exec_shell(ಶೂನ್ಯ)
{
	ಧ್ರುವ ವರ್ಣ *shell = ul_default_shell(0, ರಿಕ್ತ);
	ವರ್ಣ *shellc;
	ಧ್ರುವ ವರ್ಣ *shell_basename;
	ವರ್ಣ *arg0;

	shellc = xstrdup(shell);
	shell_basename = basename(shellc);
	xasprintf(&arg0, "-%s", shell_basename);

	execl(shell, arg0, (ವರ್ಣ *)ರಿಕ್ತ);
	errexec(shell);
}
